#!/usr/bin/env python3
"""Place demo library functions whose code is byte-identical in the retail game.

Bullfrog's libraries and the runtime were built the same way for the demo
and the retail game, so a module's functions keep their relative layout and
their bytes. A demo function is placed at `demo address + delta` when one of
the deltas already known for its module (from mapped neighbours) lands on
identical code: every byte equal outside relocated fields, and the relocated
fields at the same offsets.

Run as a script, it applies these placements to config/demo_map.txt: each
replaces any other pairing of the same demo or retail function. Placements
at addresses that are not yet function starts are left to tools/discover.py.

Usage: tools/demoident.py
"""

import collections
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
import demosyms  # noqa: E402
from lefile import LEFile  # noqa: E402
from project import EXE, ROOT, Project  # noqa: E402
from x86scan import trimmed_end  # noqa: E402


def _identical(demo_le, retail_le, d, r, size):
    if not (retail_le.code.contains(r) and retail_le.code.contains(r + size - 1)):
        return False
    a, b = demo_le.read(d, size), retail_le.read(r, size)
    dfix = {x - d for x in range(d - 3, d + size) if x in demo_le.fixups}
    rfix = {x - r for x in range(r - 3, r + size) if x in retail_le.fixups}
    if dfix != rfix:
        return False
    masked = {o + k for o in dfix for k in range(4)}
    return all(a[i] == b[i] for i in range(size) if i not in masked)


def placements(demo_le, retail_le, symbols, known):
    """{demo address: (retail address, size)} for byte-identical functions.

    symbols: demo code symbols (demosyms.DemoSymbol) outside game modules.
    known: {demo address: retail address} from an existing mapping; its
    per-module deltas are the only offsets tried.
    """
    starts = sorted({s.addr for s in symbols})
    nxt = dict(zip(starts, starts[1:]))
    modules = collections.defaultdict(set)
    for s in symbols:
        modules[s.module].add(s.addr)
    found = {}
    for addrs in modules.values():
        votes = collections.Counter(known[a] - a for a in addrs if a in known)
        for d in sorted(addrs):
            end = nxt.get(d, d + 0x400)
            size = trimmed_end(demo_le, d, end) - d
            if size < 1:
                continue
            for delta, _ in votes.most_common():
                if _identical(demo_le, retail_le, d, d + delta, size):
                    found[d] = (d + delta, size)
                    break
    return found


def library_symbols(retail_le):
    """Demo code symbols outside game modules, within the retail code object."""
    code = retail_le.code
    _, syms = demosyms.load()
    return [s for s in syms if s.code and s.category != "game" and code.base <= s.addr < code.base + code.vsize]


def main():
    retail_le = LEFile(EXE)
    library = library_symbols(retail_le)
    by_addr = {}
    for s in library:
        by_addr.setdefault(s.addr, s)

    path = os.path.join(ROOT, "config", "demo_map.txt")
    lines = open(path).read().splitlines()
    header, rows = lines[0], [line.split() for line in lines[1:] if line.strip()]
    known = {int(r[1], 16): int(r[0], 16) for r in rows if r[3] != "conflict"}
    starts = {f.addr for f in Project().functions}
    found = placements(LEFile(demosyms.DEMO_EXE), retail_le, library, known)
    placed = {d: r for d, (r, _) in found.items() if r in starts}

    changed = {d: r for d, r in placed.items() if known.get(d) != r}
    taken_demo, taken_retail = set(changed), set(changed.values())
    rows = [r for r in rows if int(r[1], 16) not in taken_demo and int(r[0], 16) not in taken_retail]
    for d, r in changed.items():
        s = by_addr[d]
        rows.append([f"{r:08x}", f"{d:08x}", "bytes", "confirmed", s.category, s.source, s.name])
    rows.sort(key=lambda r: int(r[0], 16))
    with open(path, "w") as f:
        f.write(header + "\n")
        for r in rows:
            f.write(f"{r[0]} {r[1]} {r[2]:<7} {r[3]:<10} {r[4]:<7} {r[5]:<14} {r[6]}\n")
    print(f"{len(placed)} identical, {len(changed)} pairings added or corrected", file=sys.stderr)


if __name__ == "__main__":
    main()
