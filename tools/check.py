#!/usr/bin/env python3
"""Rebuild one source file and report how well each of its functions matches.

Mismatched instructions are printed target | ours, aligned as objdiff aligns
them.

Usage: tools/check.py src/game/FILE.cpp [-v]   (-v: print every instruction)
"""

import json
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(__file__))
from project import ROOT, Project  # noqa: E402


def fmt(row):
    ins = row.get("instruction")
    return ins["formatted"] if ins else ""


def main(source, *flags):
    verbose = "-v" in flags
    source = os.path.relpath(os.path.abspath(source), ROOT)
    os.chdir(ROOT)
    subprocess.run(["python3", "configure.py"], check=True)
    proj = Project()
    unit = next((u for u in proj.units if u.source == source), None)
    if unit is None:
        sys.exit(f"{source}: no // FUNCTION: markers")
    r = subprocess.run(
        ["ninja", f"build/target/{unit.name}.o", f"build/base/{unit.name}.o"],
        capture_output=True, text=True,
    )
    if r.returncode:
        sys.exit(r.stdout + r.stderr)

    total = 0
    for fn in unit.functions:
        out = subprocess.run(
            ["objdiff-cli", "diff", "-p", ".", "-u", unit.name, "-o", "-", "--format", "json", fn.name],
            capture_output=True, text=True, check=True,
        ).stdout
        diff = json.loads(out)
        left = next(s for s in diff["left"]["symbols"] if s["name"] == fn.name)
        right = next((s for s in diff["right"]["symbols"] if s["name"] == fn.name), None)
        pct = left.get("match_percent", 0.0)
        total += pct == 100.0
        print(f"{pct:7.2f}%  {fn.addr:#010x}  {fn.name}")
        if pct == 100.0 or right is None:
            if right is None:
                print("          (not found in compiled object)")
            continue
        for lrow, rrow in zip(left["instructions"], right["instructions"]):
            differs = lrow.get("diff_kind") or rrow.get("diff_kind")
            if verbose or differs:
                mark = "!" if differs else " "
                print(f"   {mark} {fmt(lrow):<44} | {fmt(rrow)}")
    print(f"{total}/{len(unit.functions)} matched")


if __name__ == "__main__":
    main(*sys.argv[1:])
