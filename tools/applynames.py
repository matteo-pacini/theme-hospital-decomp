#!/usr/bin/env python3
"""Give functions and globals their demo-build names.

Functions in config/demo_map.txt (except conflicts) take the demo's mangled
name in config/functions.txt, unless source code already uses their current
name (it is defined or referenced by an object under build/base/, so build
first); those are listed with the demo name instead, to be renamed by hand.
Globals in config/demo_data_map.txt are added
to config/symbols.txt under the demo's mangled name, sized by the demo but
never overlapping another entry; an existing entry at the same address is
renamed and keeps its size (those sizes were checked against retail code).

Usage: tools/applynames.py
"""

import os
import struct
import sys

sys.path.insert(0, os.path.dirname(__file__))
from project import ROOT  # noqa: E402

CFG = os.path.join(ROOT, "config")


def rows(name):
    return [l.split() for l in open(os.path.join(CFG, name)) if l.strip() and not l.startswith("#")]


def object_symbols(path):
    """Names in the symbol table of an ELF32 object."""
    b = open(path, "rb").read()
    shoff = struct.unpack_from("<I", b, 0x20)[0]
    shentsize, shnum = struct.unpack_from("<HH", b, 0x2E)
    secs = [struct.unpack_from("<10I", b, shoff + i * shentsize) for i in range(shnum)]
    names = set()
    for s in secs:
        if s[1] != 2:  # SHT_SYMTAB
            continue
        strtab = secs[s[6]][4]
        for i in range(1, s[5] // 16):
            off = strtab + struct.unpack_from("<I", b, s[4] + i * 16)[0]
            names.add(b[off : b.index(b"\0", off)].decode("latin-1"))
    return names


def used_names():
    used = set()
    for dirpath, _, files in os.walk(os.path.join(ROOT, "build", "base")):
        for f in files:
            if f.endswith(".o"):
                used |= object_symbols(os.path.join(dirpath, f))
    return used


def main():
    names = {int(r[0], 16): r[6] for r in rows("demo_map.txt") if r[3] != "conflict"}
    used = used_names()
    current = [(int(addr, 16), int(size, 16), sym) for addr, size, sym in rows("functions.txt")]
    # Names sources use are claimed first, so a demo name cannot displace them.
    taken = {sym: a for a, _, sym in current if sym in used}
    lines = ["# address  size  symbol\n"]
    for a, size, sym in current:
        if sym in used:
            new = sym
            if names.get(a, sym) != sym:
                print(f"{a:08x}: keeping {sym}, used by source; demo name {names[a]}", file=sys.stderr)
        else:
            new = names.get(a, sym if not sym.startswith("W?") else f"fn_{a:08x}_")
            if taken.get(new, a) != a:
                new = f"fn_{a:08x}_"
        taken[new] = a
        lines.append(f"{a:08x} {size:6x} {new}\n")
    open(os.path.join(CFG, "functions.txt"), "w").writelines(lines)

    existing = {int(a, 16): (sym, int(sz, 16)) for a, sym, sz in rows("symbols.txt")}
    demo = {int(r[0], 16): (r[5], int(r[4], 16)) for r in rows("demo_data_map.txt")}
    merged = dict(existing)
    for a, (sym, _) in demo.items():
        if a in merged:
            merged[a] = (sym, merged[a][1])
    starts = sorted(set(merged) | set(demo))
    for i, a in enumerate(starts):
        if a in merged:
            continue
        sym, size = demo[a]
        nxt = starts[i + 1] if i + 1 < len(starts) else a + size
        inside = any(s < a < s + sz for s, (_, sz) in merged.items())
        if not inside:
            merged[a] = (sym, max(1, min(size, nxt - a)))
    with open(os.path.join(CFG, "symbols.txt"), "w") as f:
        f.write("# address  symbol  size\n")
        for a in sorted(merged):
            sym, size = merged[a]
            f.write(f"{a:08x} {sym} {size:x}\n")
    print(f"{len(names)} functions and {len(merged)} data symbols named", file=sys.stderr)


if __name__ == "__main__":
    main()
