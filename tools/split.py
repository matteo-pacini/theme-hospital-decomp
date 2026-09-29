#!/usr/bin/env python3
"""Write one GNU as source per unit reproducing its functions from HOSPITAL.EXE.

Each function goes in its own _TEXT section, mirroring wpp386 -zm output;
a unit whose source is built without -zm gets one section for all its
functions instead. LE fixups become R_386_32 relocations; rel32 branches
leaving the section, and calls to a function's entry, become R_386_PC32
relocations, as the compiler emits them.

Usage: tools/split.py OUTDIR [UNIT...]   (default: every unit)
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(__file__))
from lefile import SRC_OFFSET32, LEFile  # noqa: E402
from project import EXE, Project  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from cc import compiler_for  # noqa: E402
from x86scan import leading_table, scan  # noqa: E402

_md = Cs(CS_ARCH_X86, CS_MODE_32)
_SAR = re.compile(r"(e[a-d]x|e[sd]i), 0x(10|18)$")


def q(sym):
    """Quote a symbol for GNU as; mangled C++ names contain ?, $ and ()."""
    return f'"{sym}"'


def code_ref(proj, starts, section_label, section, addr):
    """Expression for a code address, as referenced from inside a section
    spanning `section` = (start, end) whose functions start at `starts`
    ({address: function})."""
    if addr in starts:
        # Taking a function's address names its symbol.
        return q(starts[addr].name)
    if section[0] <= addr < section[1]:
        return f"{section_label}+{addr - section[0]:#x}"
    target = proj.function_at(addr)
    if target is None:
        return f"_C_{addr:08x}"
    off = addr - target.addr
    return f"{q(target.name)}+{off:#x}" if off else q(target.name)


def sign_extending_reads(le, items):
    """{fixup site: k} for Watcom's signed narrow reads: an s16 (s8) at g is
    loaded as the dword at g-2 (g-3) and shifted down with sar 16 (24)."""
    reads = {}
    for it, nxt in zip(items, items[1:]):
        if it.mnemonic != "mov" or nxt.mnemonic != "sar":
            continue
        sar = next(_md.disasm(le.read(nxt.addr, nxt.size), nxt.addr))
        m = _SAR.match(sar.op_str)
        if not m:
            continue
        for site in range(it.addr, it.end):
            if site in le.fixups:
                reads[site] = 2 if m.group(2) == "10" else 3
    return reads


def sign_read_ref(proj, target, k):
    """The compiler names such a read after the variable it reads (`g-2`),
    which differs from the symbol containing the loaded address when that
    belongs to the preceding variable."""
    value = target + k
    if proj.data_symbol_at(target) is proj.data_symbol_at(value) is not None:
        return proj.data_ref(target)
    return f"{proj.data_ref(value)}-{k}"


def emit_section(le, proj, fns, n, out, shared):
    """One _TEXT section holding `fns`: a single function (-zm), or every
    function of a module compiled into one segment (`shared`), laid out as
    in the executable."""
    label = f".Lsec{n}"
    start = leading_table(le, fns[0].addr, fns[0].end) or fns[0].addr
    end = fns[-1].end
    starts = {f.addr: f for f in fns}
    # Each function's bytes run to the next one's start in a shared segment:
    # jump tables and padding between them belong to the preceding function.
    ends = [g.addr for g in fns[1:]] + [end] if shared else [end]
    out.append(f'\n.section _TEXT,"ax",@progbits,unique,{n}\n{label}:')

    items = [(fn, it) for fn, e in zip(fns, ends) for it in scan(le, fn.addr, e)]
    sign_reads = sign_extending_reads(le, [it for _, it in items])

    fields = {}  # field address -> (size, expression)
    for fn, it in items:
        t = it.branch_target
        if t is None:
            continue
        if not start <= t < end:
            fields[it.branch_field] = (4, f"{code_ref(proj, starts, label, (start, end), t)}-.-4")
        elif t in starts and (it.mnemonic == "call" or t != fn.addr):
            # Calls, and jumps to another function's entry, are relocated
            # against the target's symbol even within one segment; as would
            # resolve `sym-.-4` itself, so the relocation is explicit.
            fields[it.branch_field] = (4, f'-4\n.reloc .-4, R_386_PC32, {q(starts[t].name)}')
    for site in range(start, end):
        fx = le.fixups.get(site)
        if fx is None:
            continue
        if fx.kind != SRC_OFFSET32:
            raise NotImplementedError(f"{fns[0].name}: fixup kind {fx.kind} at {site:#x}")
        if site in proj.reloc_overrides:
            name, off = proj.reloc_overrides[site]
            expr = f"{q(name)}{off:+#x}" if off else q(name)
        elif le.code.contains(fx.target):
            expr = code_ref(proj, starts, label, (start, end), fx.target)
        elif site in sign_reads:
            expr = sign_read_ref(proj, fx.target, sign_reads[site])
        else:
            expr = proj.data_ref(fx.target)
        fields[site] = (4, expr)

    data = le.read(start, end - start)
    pending = []

    def flush():
        if pending:
            out.append(".byte " + ",".join(f"{b:#04x}" for b in pending))
            pending.clear()

    sizes = {fn.addr: e - fn.addr for fn, e in zip(fns, ends)}
    a = start
    while a < end:
        if a in starts:
            flush()
            name = q(starts[a].name)
            out.append(f".globl {name}\n.type {name},@function\n.size {name},{sizes[a]:#x}\n{name}:")
        if a in fields:
            flush()
            size, expr = fields[a]
            out.append(f".long {expr}")
            a += size
            continue
        pending.append(data[a - start])
        if len(pending) == 16:
            flush()
        a += 1
    flush()


def shared_segment(proj, unit):
    """Whether the unit's source was compiled without -zm (config/cflags.txt),
    all its functions in one segment. Such a unit must implement every
    function in its address range, or the layouts cannot agree."""
    if unit.auto or "-zm-" not in compiler_for(unit.source)[1]:
        return False
    first, last = unit.functions[0], unit.functions[-1]
    mine = {f.addr for f in unit.functions}
    missing = [f for f in proj.functions if first.addr < f.addr < last.addr and f.addr not in mine]
    if missing:
        raise ValueError(f"{unit.source}: built without -zm but does not implement "
                         + ", ".join(f"{f.addr:#x}" for f in missing))
    return True


def write_unit(le, proj, unit, path):
    out = [".intel_syntax noprefix"]
    if shared_segment(proj, unit):
        emit_section(le, proj, unit.functions, 0, out, shared=True)
    else:
        for n, fn in enumerate(unit.functions):
            emit_section(le, proj, [fn], n, out, shared=False)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    text = "\n".join(out) + "\n"
    # Leave unchanged files untouched so ninja only reassembles what moved.
    if not os.path.exists(path) or open(path).read() != text:
        with open(path, "w") as f:
            f.write(text)


def main(outdir, *only):
    proj = Project()
    le = LEFile(EXE)
    for unit in proj.units:
        if only and unit.name not in only:
            continue
        write_unit(le, proj, unit, os.path.join(outdir, unit.name + ".s"))


if __name__ == "__main__":
    main(*sys.argv[1:])
