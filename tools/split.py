#!/usr/bin/env python3
"""Write one GNU as source per unit reproducing its functions from HOSPITAL.EXE.

Each function goes in its own _TEXT section, mirroring wpp386 -zm output.
LE fixups become R_386_32 relocations; rel32 branches leaving the function
become R_386_PC32 relocations, as the compiler emits for any call across
-zm segments.

Usage: tools/split.py OUTDIR [UNIT...]   (default: every unit)
"""

import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from lefile import SRC_OFFSET32, LEFile  # noqa: E402
from project import EXE, Project  # noqa: E402
from x86scan import scan  # noqa: E402


def q(sym):
    """Quote a symbol for GNU as; mangled C++ names contain ?, $ and ()."""
    return f'"{sym}"'


def code_ref(proj, fn, section_label, addr):
    """Expression for a code address, as referenced from inside `fn`."""
    if fn.addr <= addr < fn.end:
        return f"{section_label}+{addr - fn.addr:#x}"
    target = proj.function_at(addr)
    if target is None:
        return f"_C_{addr:08x}"
    off = addr - target.addr
    return f"{q(target.name)}+{off:#x}" if off else q(target.name)


def emit_function(le, proj, fn, n, out):
    label = f".Lsec{n}"
    out.append(f'\n.section _TEXT,"ax",@progbits,unique,{n}')
    out.append(f".globl {q(fn.name)}\n.type {q(fn.name)},@function\n{q(fn.name)}:\n{label}:")

    fields = {}  # field address -> (size, expression)
    for it in scan(le, fn.addr, fn.end):
        if it.branch_target is not None and not fn.addr <= it.branch_target < fn.end:
            expr = code_ref(proj, fn, label, it.branch_target)
            fields[it.branch_field] = (4, f"{expr}-.-4")
    for site in range(fn.addr, fn.end):
        fx = le.fixups.get(site)
        if fx is None:
            continue
        if fx.kind != SRC_OFFSET32:
            raise NotImplementedError(f"{fn.name}: fixup kind {fx.kind} at {site:#x}")
        if le.code.contains(fx.target):
            expr = code_ref(proj, fn, label, fx.target)
        else:
            expr = proj.data_ref(fx.target)
        fields[site] = (4, expr)

    data = le.read(fn.addr, fn.size)
    pending = []

    def flush():
        if pending:
            out.append(".byte " + ",".join(f"{b:#04x}" for b in pending))
            pending.clear()

    a = fn.addr
    while a < fn.end:
        if a in fields:
            flush()
            size, expr = fields[a]
            out.append(f".long {expr}")
            a += size
            continue
        pending.append(data[a - fn.addr])
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
