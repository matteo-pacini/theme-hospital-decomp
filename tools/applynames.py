#!/usr/bin/env python3
"""Give functions and globals their demo-build names.

Functions in config/demo_map.txt (except conflicts) take the demo's mangled
name in config/functions.txt. Globals in config/demo_data_map.txt are added
to config/symbols.txt under the demo's mangled name, sized by the demo but
never overlapping another entry; an existing entry at the same address is
renamed and keeps its size (those sizes were checked against retail code).

Usage: tools/applynames.py
"""

import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from project import ROOT  # noqa: E402

CFG = os.path.join(ROOT, "config")


def rows(name):
    return [l.split() for l in open(os.path.join(CFG, name)) if l.strip() and not l.startswith("#")]


def main():
    names = {int(r[0], 16): r[6] for r in rows("demo_map.txt") if r[3] != "conflict"}
    taken = {}
    lines = ["# address  size  symbol\n"]
    for addr, size, sym in rows("functions.txt"):
        a = int(addr, 16)
        new = names.get(a, sym if not sym.startswith("W?") else f"fn_{a:08x}_")
        if new in taken:
            new = f"fn_{a:08x}_"
        taken[new] = a
        lines.append(f"{addr} {size} {new}\n")
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
