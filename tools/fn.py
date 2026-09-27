#!/usr/bin/env python3
"""Show a function from HOSPITAL.EXE as a starting point for decompiling it.

Prints the disassembly with relocated operands named the way source must name
them (fn_XXXXXXXX_, _D_XXXXXXXX), followed by rz-ghidra's pseudo-C.

Usage: tools/fn.py ADDRESS [--no-ghidra]
"""

import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(__file__))
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from demangle import demangle  # noqa: E402
from lefile import LEFile  # noqa: E402
from project import EXE, Project  # noqa: E402
from x86scan import scan  # noqa: E402


def name_for(proj, le, addr):
    if le.code.contains(addr):
        fn = proj.function_at(addr)
        if fn is None:
            return f"_C_{addr:08x}"
        label = fn.name if addr == fn.addr else f"{fn.name}+{addr - fn.addr:#x}"
        return f"{label} ({fn.demo_name})" if fn.demo_name and addr == fn.addr else label
    ref = proj.data_ref(addr)
    demo = proj.demo_data.get(addr)
    return f"{ref} ({demo})" if demo else ref


def main(addr, *flags):
    addr = int(addr, 16)
    proj = Project()
    le = LEFile(EXE)
    fn = proj.function_at(addr)
    if fn is None or fn.addr != addr:
        sys.exit(f"no function starts at {addr:#x}")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    print(f"; {fn.name}  {fn.addr:#x}..{fn.end:#x} ({fn.size} bytes)")
    if fn.demo_name:
        print(f"; demo build: {demangle([fn.demo_name])[fn.demo_name]}")
        print(f";   symbol {fn.demo_name} in {fn.source} ({fn.category})")
    for it in scan(le, fn.addr, fn.end):
        raw = le.read(it.addr, it.size)
        if it.mnemonic is None:
            fx = le.fixups[it.addr]
            text = f".long {name_for(proj, le, fx.target)}"
        else:
            ins = next(md.disasm(raw, it.addr))
            text = f"{ins.mnemonic} {ins.op_str}"
            refs = [
                name_for(proj, le, le.fixups[a].target)
                for a in range(it.addr, it.end)
                if a in le.fixups and le.fixups[a].target is not None
            ]
            if it.branch_target is not None:
                refs.append(name_for(proj, le, it.branch_target))
            if refs:
                text = f"{text:<40} ; {', '.join(refs)}"
        print(f"{it.addr:08x}  {raw.hex(' '):<30} {text}")

    if "--no-ghidra" not in flags:
        print()
        out = subprocess.run(
            ["rizin", "-q", "-e", "scr.color=0", "-e", "bin.relocs.apply=true",
             "-c", f"af @ {addr:#x}; pdg @ {addr:#x}", EXE],
            capture_output=True, text=True,
        )
        print(out.stdout)


if __name__ == "__main__":
    main(*sys.argv[1:])
