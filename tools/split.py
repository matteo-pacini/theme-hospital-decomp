#!/usr/bin/env python3
"""Write one GNU as source per unit reproducing its functions from HOSPITAL.EXE.

Each function goes in its own _TEXT section, mirroring wpp386 -zm output.
LE fixups become R_386_32 relocations; rel32 branches leaving the function,
and recursive calls, become R_386_PC32 relocations, as the compiler emits for
any call across -zm segments.

Usage: tools/split.py OUTDIR [UNIT...]   (default: every unit)
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(__file__))
from lefile import SRC_OFFSET32, LEFile  # noqa: E402
from project import EXE, Project  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from x86scan import leading_table, scan  # noqa: E402

_md = Cs(CS_ARCH_X86, CS_MODE_32)
_SAR = re.compile(r"(e[a-d]x|e[sd]i), 0x(10|18)$")


def q(sym):
    """Quote a symbol for GNU as; mangled C++ names contain ?, $ and ()."""
    return f'"{sym}"'


def code_ref(proj, fn, section_label, section_start, addr):
    """Expression for a code address, as referenced from inside `fn`, whose
    section starts at `section_start`."""
    if addr == fn.addr:
        # Taking a function's own address names its symbol.
        return q(fn.name)
    if section_start <= addr < fn.end:
        return f"{section_label}+{addr - section_start:#x}"
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


def emit_function(le, proj, fn, n, out):
    label = f".Lsec{n}"
    start = leading_table(le, fn.addr, fn.end) or fn.addr
    out.append(f'\n.section _TEXT,"ax",@progbits,unique,{n}\n{label}:')

    items = scan(le, fn.addr, fn.end)
    sign_reads = sign_extending_reads(le, items)

    fields = {}  # field address -> (size, expression)
    for it in items:
        if it.branch_target is None:
            continue
        if not fn.addr <= it.branch_target < fn.end:
            fields[it.branch_field] = (4, f"{code_ref(proj, fn, label, start, it.branch_target)}-.-4")
        elif it.mnemonic == "call" and it.branch_target == fn.addr:
            # A recursive call is relocated against the function's own symbol;
            # as would resolve `sym-.-4` itself, so the relocation is explicit.
            fields[it.branch_field] = (4, f'-4\n.reloc .-4, R_386_PC32, {q(fn.name)}')
    for site in range(start, fn.end):
        fx = le.fixups.get(site)
        if fx is None:
            continue
        if fx.kind != SRC_OFFSET32:
            raise NotImplementedError(f"{fn.name}: fixup kind {fx.kind} at {site:#x}")
        if site in proj.reloc_overrides:
            name, off = proj.reloc_overrides[site]
            expr = f"{q(name)}{off:+#x}" if off else q(name)
        elif le.code.contains(fx.target):
            expr = code_ref(proj, fn, label, start, fx.target)
        elif site in sign_reads:
            expr = sign_read_ref(proj, fx.target, sign_reads[site])
        else:
            expr = proj.data_ref(fx.target)
        fields[site] = (4, expr)

    data = le.read(start, fn.end - start)
    pending = []

    def flush():
        if pending:
            out.append(".byte " + ",".join(f"{b:#04x}" for b in pending))
            pending.clear()

    a = start
    while a < fn.end:
        if a == fn.addr:
            flush()
            out.append(f".globl {q(fn.name)}\n.type {q(fn.name)},@function\n{q(fn.name)}:")
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
    out.append(f".size {q(fn.name)},.-{q(fn.name)}")


def write_unit(le, proj, unit, path):
    out = [".intel_syntax noprefix"]
    for n, fn in enumerate(unit.functions):
        emit_function(le, proj, fn, n, out)
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
