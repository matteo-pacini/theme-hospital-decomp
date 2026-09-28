#!/usr/bin/env python3
"""Generate config/functions.txt: every function in the code object of HOSPITAL.EXE.

Starts are the union of rizin's auto-analysis, rel32 call targets, Watcom
unoptimised prologues that are only reached through pointers or that follow
another function's padding, and every start already in config/functions.txt
(so hand-added entries survive). Existing names are preserved.

Usage: tools/discover.py orig/HOSPITAL.EXE config/functions.txt
"""

import json
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(__file__))
from lefile import LEFile  # noqa: E402
from project import GAME_END, Project  # noqa: E402
from x86scan import is_padding, scan, trimmed_end  # noqa: E402

# Unoptimised Watcom frame: register saves, push ebp; mov ebp,esp; sub esp,imm32
PROLOGUE = re.compile(rb"[\x50-\x57]{1,7}\x89\xe5\x81\xec")


def read_names(path):
    names = {}
    if os.path.exists(path):
        for line in open(path):
            line = line.split("#", 1)[0].split()
            if line:
                names[int(line[0], 16)] = line[2]
    return names


def main(exe, out):
    le = LEFile(exe)
    code = le.code
    lo, hi = code.base, code.base + code.vsize

    afl = subprocess.run(
        ["rizin", "-q", "-e", "bin.relocs.apply=true", "-c", "aaa; aflj", exe],
        check=True, capture_output=True, text=True,
    ).stdout
    starts = {f["offset"] for f in json.loads(afl) if lo <= f["offset"] < hi}

    for it in scan(le, lo, hi):
        if it.mnemonic == "call" and it.branch_target is not None and lo <= it.branch_target < hi:
            starts.add(it.branch_target)

    fixup_targets = {f.target for f in le.fixups.values() if f.target is not None}
    for m in PROLOGUE.finditer(code.data):
        a = lo + m.start()
        if a in fixup_targets and a % 4 == 0:
            starts.add(a)

    # A prologue on a 4-byte boundary after a terminal instruction and padding
    # starts a function nothing calls directly. Module boundaries pad to 16.
    for it in scan(le, lo, hi):
        if not it.terminal:
            continue
        for nxt in range(it.end + (-it.end % 4), it.end + 16, 4):
            if not is_padding(code.data[it.end - lo : nxt - lo]):
                break
            if PROLOGUE.match(code.data, nxt - lo):
                starts.add(nxt)
                break

    # Library code is aligned to 16 bytes: an aligned address reached through
    # padding after a terminal instruction starts a function even when it is
    # only called through a pointer.
    # Hand-written assembly aligns loop bodies the same way, so its routines
    # (modules named *.asm in the demo build) are left whole.
    asm = [(f.addr, f.end) for f in Project().functions if f.source and f.source.endswith(".asm")]
    for it in scan(le, GAME_END, hi):
        if not it.terminal or it.end % 16 == 0:
            continue
        nxt = it.end + (-it.end % 16)
        if nxt < hi and is_padding(code.data[it.end - lo : nxt - lo]) and not any(a < nxt < e for a, e in asm):
            starts.add(nxt)

    names = read_names(out)
    starts |= set(names)
    # Game functions are 4-byte aligned (-zm segments); an unaligned start
    # there is analysis noise inside another function.
    starts = sorted(s for s in starts if s >= GAME_END or s % 4 == 0)
    with open(out, "w") as f:
        f.write("# address  size  symbol\n")
        for i, s in enumerate(starts):
            nxt = starts[i + 1] if i + 1 < len(starts) else hi
            end = trimmed_end(le, s, nxt)
            f.write(f"{s:08x} {end - s:6x} {names.get(s, f'fn_{s:08x}_')}\n")
    print(f"{len(starts)} functions", file=sys.stderr)


if __name__ == "__main__":
    main(*sys.argv[1:])
