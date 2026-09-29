#!/usr/bin/env python3
"""Check a Watcom runtime library module against the EXE, for building a unit
from the library instead of from C (a `lib` entry in config/cflags.txt).

The module's code is laid over the unit's address range (its first function
at the module's offset 0) and compared byte for byte, relocated fields
aside. Each relocation is then resolved in the EXE, which gives what the
config needs for objdiff to pair the two objects: the library's names for
the unit's functions and for the functions and globals it refers to, and
the addresses of the module's own data segments (config/sections.txt), which
the library refers to section-relative.

Usage: tools/libunit.py SOURCE LIBRARY MODULE [VERSION]
       e.g. tools/libunit.py src/lib/clib/qsort.c DOS/CLIB3R.LIB qsort
"""

import os
import struct
import sys

sys.path.insert(0, os.path.dirname(__file__))
import omf2elf  # noqa: E402
from cc import LIBRARIES  # noqa: E402
from lefile import LEFile  # noqa: E402
from project import EXE, ROOT, Project  # noqa: E402


def check(source, library, module, version="10.6"):
    """Print the differences and the config the module needs; return
    whether its code matches."""
    proj, le = Project(), LEFile(EXE)
    unit = next(u for u in proj.units if u.source == source)
    path = os.path.join(ROOT, LIBRARIES[version], library)
    sections, symbols, _, _ = omf2elf.convert(omf2elf.library_module(path, module))
    (text,) = [s for s in sections if s.code and s.data]
    # Align on a function the project already names as the library does,
    # since a module can start with a table rather than a function.
    base = next((proj.by_name[n].addr - off for n, (sec, off, _) in symbols.items()
                 if sec is text and n in proj.by_name), unit.functions[0].addr)
    exe = le.read(base, len(text.data))
    ok = True

    # Relocated fields: the library holds the addend, the EXE the target.
    reloc_bytes = {r[0] + k for r in text.relocs for k in range(4)}
    diffs = [o for o in range(len(text.data)) if o not in reloc_bytes and text.data[o] != exe[o]]
    if diffs:
        ok = False
        print(f"code differs at {len(diffs)} bytes, first at {base + diffs[0]:#x}")

    renames, data_names, segments = {}, {}, {}
    for name, (sec, off, _) in symbols.items():
        if sec is text:
            f = proj.function_at(base + off)
            if f is None or f.addr != base + off:
                ok = False
                print(f"{name}: no function at {base + off:#x}")
            elif f.name != name:
                renames[f.addr] = (f.name, name)

    for off, rtype, target in text.relocs:
        site = base + off
        addend = struct.unpack_from("<i", text.data, off)[0]
        if rtype == omf2elf.R_386_PC32:
            addr = site + 4 + struct.unpack_from("<i", exe, off)[0] - addend - 4
        else:
            fx = le.fixups.get(site)
            if fx is None or fx.target is None:
                ok = False
                print(f"{site:#x}: relocated in the library, not in the EXE")
                continue
            addr = fx.target - addend
        if target[0] == "sec":
            seg = target[1]
            if seg is text:
                if addr != base:
                    ok = False
                    print(f"{site:#x}: reference into the module's code resolves elsewhere")
                continue
            if segments.setdefault(seg.name, (addr, seg))[0] != addr:
                ok = False
                print(f"{seg.name}: placed at both {segments[seg.name][0]:#x} and {addr:#x}")
            continue
        name = target[1]
        if name in symbols and symbols[name][0] is text:
            if addr != base + symbols[name][1]:
                ok = False
                print(f"{site:#x}: {name} resolves to {addr:#x}")
            continue
        # A global of the module's own or another's: the target names it too.
        if le.code.contains(addr):
            f = proj.function_at(addr)
            if f is None or f.addr != addr:
                ok = False
                print(f"{site:#x}: {name} resolves to {addr:#x}, inside a function")
            elif f.name != name:
                renames[f.addr] = (f.name, name)
        else:
            s = proj.data_symbol_at(addr)
            current = s.name if s is not None and s.addr == addr else None
            if current != name:
                data_names[addr] = (current, name)

    for addr, (old, new) in sorted(renames.items()):
        print(f"function {addr:08x} {old} -> {new}")
    for addr, (old, new) in sorted(data_names.items()):
        print(f"global   {addr:08x} {old or '(none)'} -> {new}")
    for name, (addr, seg) in sorted(segments.items()):
        print(f"segment  {source} {name} {addr:08x} {max(len(seg.data), seg.length):x}")
    print("code matches" if ok else "code does not match")
    return ok


if __name__ == "__main__":
    sys.exit(0 if check(*sys.argv[1:]) else 1)
