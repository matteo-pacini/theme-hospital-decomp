#!/usr/bin/env python3
"""Add, rename or remove one entry in config/functions.txt.

Only the entry and the one before it are re-sized (padding before the next
start trimmed, as tools/discover.py does), so several people can edit
different parts of the file without rewriting the rest of it.

Usage: tools/setfn.py ADDRESS NAME     add a start, or rename an existing one
       tools/setfn.py ADDRESS --remove  merge a start into the previous entry
"""

import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from lefile import LEFile  # noqa: E402
from project import EXE, ROOT  # noqa: E402
from x86scan import leading_table, trimmed_end  # noqa: E402

PATH = os.path.join(ROOT, "config", "functions.txt")


def main(addr, name):
    addr = int(addr, 16)
    lines = open(PATH).read().splitlines()
    header, rows = lines[0], [line.split() for line in lines[1:] if line.strip()]
    entries = {int(a, 16): [int(s, 16), n] for a, s, n in rows}
    if name == "--remove":
        del entries[addr]
    else:
        entries.setdefault(addr, [0, name])[1] = name

    le = LEFile(EXE)
    hi = le.code.base + le.code.vsize
    starts = sorted(entries)
    i = starts.index(addr) if addr in entries else next(k for k, s in enumerate(starts) if s > addr)
    for k in range(max(i - 1, 0), min(i + 1, len(starts))):
        s = starts[k]
        nxt = starts[k + 1] if k + 1 < len(starts) else hi
        if k + 1 < len(starts):
            nxt = leading_table(le, nxt, starts[k + 2] if k + 2 < len(starts) else hi) or nxt
        entries[s][0] = trimmed_end(le, s, nxt) - s

    with open(PATH, "w") as f:
        f.write(header + "\n")
        for s in starts:
            size, n = entries[s]
            f.write(f"{s:08x} {size:6x} {n}\n")


if __name__ == "__main__":
    main(*sys.argv[1:])
