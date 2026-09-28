#!/usr/bin/env python3
"""Write GNU as (Intel syntax) source for functions that were hand-written
assembly in the original (modules named *.asm in the demo's debug info).

Instructions are emitted as mnemonics with local labels for branch targets
and symbols for relocated operands. The function is assembled and compared
with the original bytes (relocated fields excluded); wherever as chooses a
different encoding (e.g. a short jump for a near one, or the other ModRM form
of a register move), that instruction is emitted as bytes instead, with its
mnemonic kept as a comment, until the whole function matches.

Usage: tools/asm2src.py OUTPUT.s ADDRESS...
"""

import os
import re
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(__file__))
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from lefile import LEFile  # noqa: E402
from project import EXE, Project  # noqa: E402
from split import q  # noqa: E402
from x86scan import scan  # noqa: E402

_md = Cs(CS_ARCH_X86, CS_MODE_32)


def symbolic(text, raw, field_offsets, exprs):
    """Replace the literal value of each relocated field in capstone's text."""
    for off, expr in zip(field_offsets, exprs):
        value = int.from_bytes(raw[off : off + 4], "little")
        for lit in (f"0x{value:x}", str(value)):
            pattern = rf"(?<![0-9a-fx]){re.escape(lit)}(?![0-9a-f])"
            if re.search(pattern, text):
                text = re.sub(pattern, expr.replace("\\", "\\\\"), text, count=1)
                break
        else:
            return None
    return text


def code_ref(proj, addr):
    target = proj.function_at(addr)
    off = addr - target.addr
    return f"{q(target.name)}+{off:#x}" if off else q(target.name)


def byte_form(le, proj, fn, addr, ins, raw, label, known_labels=None):
    """The instruction as bytes, keeping relocated fields and branch
    displacements symbolic."""
    def ref(t):
        if fn.addr <= t < fn.end:
            return label(t)
        return proj.data_ref(t) if not le.code.contains(t) else code_ref(proj, t)

    if ins.mnemonic.startswith(("j", "call", "loop")) and ins.op_str.startswith("0x") and not any(
        addr + k in le.fixups for k in range(len(raw))
    ):
        disp = 4 if ins.size >= 5 else 1
        dest = int(ins.op_str, 16)
        inside = fn.addr <= dest < fn.end
        if (disp == 1 and not inside) or (inside and label(dest) not in known_labels):
            # A short branch into another function carries no relocation.
            return "; ".join(f".byte {b:#04x}" for b in raw)
        head = raw[: len(raw) - disp]
        directive = ".long" if disp == 4 else ".byte"
        return "; ".join([f".byte {b:#04x}" for b in head] + [f"{directive} {ref(dest)}-.-{disp}"])
    parts, j = [], 0
    while j < len(raw):
        a = addr + j
        if a in le.fixups:
            parts.append(f".long {ref(le.fixups[a].target)}")
            j += 4
        else:
            parts.append(f".byte {raw[j]:#04x}")
            j += 1
    return "; ".join(parts)


def first_misplaced(alts, form, offsets, base):
    """Index of the first instruction assembled to a different length than
    the original (the next instruction starts at the wrong offset)."""
    line = 0
    starts = []
    for _, _, _, is_label in alts:
        line += is_label
        starts.append(line)
        line += 1
    for i in range(len(alts) - 1):
        want = alts[i + 1][0] - base
        if offsets.get(starts[i + 1]) != want:
            return i if offsets.get(starts[i]) == alts[i][0] - base else None
    return None


def line_owner(alts, line):
    """Index of the instruction rendered on the given 0-based output line."""
    n = 0
    for i, (_, _, _, is_label) in enumerate(alts):
        n += 1 + is_label
        if n > line:
            return i
    return None


# Forms tried for each instruction, in order: as written, with the operand
# direction forced either way (the original assembler preferred the "load"
# ModRM form for register-register operations), then raw bytes.
FORMS = ("", "{load} ", "{store} ", None)


def render(alts, form, label):
    lines = []
    for (addr, text, raw_form, is_label), f in zip(alts, form):
        if is_label:
            lines.append(f"{label(addr)}:")
        prefix = FORMS[f]
        lines.append(f"    {raw_form}" if prefix is None or text is None or text.startswith(".") else f"    {prefix}{text}")
    return lines


def assemble_function(lines):
    """Assemble one function's lines in a plain section. Returns (bytes, None,
    offsets) with the offset at which each line starts, or (None, index of the
    first line as rejects, None)."""
    with tempfile.TemporaryDirectory() as tmp:
        src, obj, binf = (os.path.join(tmp, n) for n in ("a.s", "a.o", "a.bin"))
        marked = [f"__line{i}: {line.strip()}" if not line.strip().endswith(":") else line for i, line in enumerate(lines)]
        open(src, "w").write(".intel_syntax noprefix\n.text\n" + "\n".join(marked) + "\n")
        r = subprocess.run(["as", "--32", "-o", obj, src], capture_output=True, text=True)
        if r.returncode:
            m = re.search(r"a\.s:(\d+):", r.stderr)
            return None, (int(m.group(1)) - 3 if m else 0), None
        subprocess.run(["objcopy", "-O", "binary", "-j", ".text", obj, binf], check=True)
        nm = subprocess.run(["nm", obj], capture_output=True, text=True, check=True).stdout
        offsets = {int(m.group(2)): int(m.group(1), 16) for m in re.finditer(r"^([0-9a-f]+) t __line(\d+)$", nm, re.M)}
        return open(binf, "rb").read(), None, offsets


def first_mismatch(got, original, base, masked):
    for i in range(max(len(got), len(original))):
        if base + i in masked:
            continue
        if i >= len(got) or i >= len(original) or got[i] != original[i]:
            return i
    return None


def reachable_items(le, fn, items):
    """Addresses of instructions reachable from the entry point; the rest of
    the function is data (e.g. lookup tables after the final ret)."""
    at = {it.addr: i for i, it in enumerate(items)}
    seen, work = set(), [fn.addr]
    while work:
        a = work.pop()
        while a in at and a not in seen:
            it = items[at[a]]
            seen.add(a)
            if it.mnemonic is None:
                break
            ins = next(_md.disasm(le.read(it.addr, it.size), it.addr))
            if ins.mnemonic.startswith(("j", "loop")) and ins.op_str.startswith("0x"):
                work.append(int(ins.op_str, 16))
            if ins.mnemonic in ("ret", "retf", "iretd", "jmp") :
                break
            a = it.end
    # Jump table entries point at code too.
    for it in items:
        if it.mnemonic is None:
            t = le.fixups[it.addr].target
            if t in at and t not in seen:
                seen |= reachable_items_from(le, items, at, t)
    return seen


def reachable_items_from(le, items, at, start):
    seen, work = set(), [start]
    while work:
        a = work.pop()
        while a in at and a not in seen:
            it = items[at[a]]
            seen.add(a)
            if it.mnemonic is None:
                break
            ins = next(_md.disasm(le.read(it.addr, it.size), it.addr))
            if ins.mnemonic.startswith(("j", "loop")) and ins.op_str.startswith("0x"):
                work.append(int(ins.op_str, 16))
            if ins.mnemonic in ("ret", "retf", "iretd", "jmp"):
                break
            a = it.end
    return seen


def emit(le, proj, fn, out):
    items = scan(le, fn.addr, fn.end)
    labels = {it.branch_target for it in items if it.branch_target and fn.addr <= it.branch_target < fn.end}
    # Short (rel8) branches have no branch_target from scan; find them too.
    for it in items:
        if it.mnemonic and it.mnemonic.startswith("j") and it.branch_target is None and it.size == 2:
            ins = next(_md.disasm(le.read(it.addr, it.size), it.addr))
            try:
                labels.add(int(ins.op_str, 16))
            except ValueError:
                pass
    for it in items:
        for a in range(it.addr, it.end):
            fx = le.fixups.get(a)
            if fx is not None and fn.addr <= fx.target < fn.end:
                labels.add(fx.target)
    # Only addresses where an item starts can carry a label.
    starts = {it.addr for it in items}
    labels &= starts
    # The function's own start is referenced by its symbol, as the compiler
    # and linker would name it.
    label = lambda a: q(fn.name) if a == fn.addr else f".L{a:08x}"
    labels.discard(fn.addr)
    reachable = reachable_items(le, fn, items)
    known = {label(a) for a in labels}

    out.append(f"\n.section _TEXT,\"ax\",@progbits,unique,{fn.addr}")
    out.append(f".globl {q(fn.name)}\n.type {q(fn.name)},@function\n{q(fn.name)}:")
    pending = []
    for it in items:
        # The scan may decode a final "instruction" running past the end.
        raw = le.read(it.addr, min(it.size, fn.end - it.addr))
        if it.mnemonic is not None and (it.addr not in reachable or it.end > fn.end):
            # Not reached from the entry point: emit as data, keeping any
            # relocated fields symbolic.
            ins = next(_md.disasm(raw, it.addr), None)
            text = byte_form(le, proj, fn, it.addr, ins, raw, label, known) if ins and len(raw) == it.size else (
                "; ".join(f".byte {b:#04x}" for b in raw))
            pending.append((it.addr, text, None, raw))
            continue
        if it.mnemonic is None:
            fx = le.fixups[it.addr]
            target = fx.target
            expr = label(target) if fn.addr <= target < fn.end else (
                proj.data_ref(target) if not le.code.contains(target) else code_ref(proj, target))
            pending.append((it.addr, f".long {expr}", None, raw))
            continue
        ins = next(_md.disasm(raw, it.addr))
        text = f"{ins.mnemonic} {ins.op_str}".strip()
        fields = [a - it.addr for a in range(it.addr, it.end) if a in le.fixups]
        exprs = []
        for a in range(it.addr, it.end):
            if a in le.fixups:
                t = le.fixups[a].target
                exprs.append(label(t) if fn.addr <= t < fn.end else (
                    proj.data_ref(t) if not le.code.contains(t) else code_ref(proj, t)))
        if fields:
            text = symbolic(text, raw, fields, exprs)
        if text is not None and ins.mnemonic.startswith(("j", "call", "loop")) and ins.op_str.startswith("0x"):
            dest = int(ins.op_str, 16)
            if fn.addr <= dest < fn.end:
                text = f"{ins.mnemonic} {label(dest)}" if dest in labels else None
            elif ins.size == 2 or proj.function_at(dest) is None:
                text = None
            else:
                text = f"{ins.mnemonic} {code_ref(proj, dest)}"
        pending.append((it.addr, text, ins, raw))

    alts = []  # per instruction: (address, text form or None, byte form, is label target)
    for addr, text, ins, raw in pending:
        if ins is None:
            alts.append((addr, text, text, addr in labels))
            continue
        comment = f"{ins.mnemonic} {ins.op_str}".strip()
        alts.append((addr, text, byte_form(le, proj, fn, addr, ins, raw, label, known) + f"  # {comment}", addr in labels))

    masked = set()
    for it in items:
        for a in range(it.addr, it.end):
            if a in le.fixups:
                masked |= set(range(a, a + 4))
        if it.branch_field is not None:
            masked |= set(range(it.branch_field, it.branch_field + 4))
    bytes_form = len(FORMS) - 1
    form = [0 if t is not None else bytes_form for _, t, _, _ in alts]
    original = le.read(fn.addr, fn.size)
    for _ in range(len(alts) * len(FORMS) + 1):
        lines = render(alts, form, label)
        got, bad_line, offsets = assemble_function(lines)
        if got is None:
            idx = line_owner(alts, bad_line)
        else:
            bad = first_mismatch(got, original, fn.addr, masked)
            if bad is None:
                break
            idx = first_misplaced(alts, form, offsets, fn.addr)
            if idx is None:
                idx = max((i for i, (a, _, _, _) in enumerate(alts) if a <= fn.addr + bad and form[i] < bytes_form), default=None)
        if idx is None or form[idx] == bytes_form:
            # Fall back to bytes throughout, which always reproduces the
            # original, rather than failing the whole module.
            print(f"{fn.name}: emitted as bytes", file=sys.stderr)
            form = [bytes_form] * len(alts)
            break
        form[idx] += 1
    out.extend(render(alts, form, label))
    out.append(f".size {q(fn.name)},.-{q(fn.name)}")


def main(output, *addresses):
    proj = Project()
    le = LEFile(EXE)
    out = [".intel_syntax noprefix"]
    for a in addresses:
        fn = proj.function_at(int(a, 16))
        out.append(f"\n// FUNCTION: {fn.addr:#010x}")
        emit(le, proj, fn, out)
    open(output, "w").write("\n".join(out) + "\n")


if __name__ == "__main__":
    main(*sys.argv[1:])
