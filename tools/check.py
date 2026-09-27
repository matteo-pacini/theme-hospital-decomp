#!/usr/bin/env python3
"""Rebuild one source file and report how well each of its functions matches.

Builds the unit's target and base objects in a private temporary directory,
so several checks can run at once alongside `ninja`. Mismatched instructions
are printed target | ours, aligned as objdiff aligns them. A function only
counts as matched if its relocations agree too (objdiff checks at most one
per instruction).

Usage: tools/check.py src/game/FILE.cpp [-v]   (-v: print every instruction)
"""

import json
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(__file__))
from elfrelocs import function_relocs  # noqa: E402
from lefile import LEFile  # noqa: E402
from project import EXE, ROOT, Project  # noqa: E402
from split import write_unit  # noqa: E402


def fmt(row):
    ins = row.get("instruction")
    return ins["formatted"] if ins else ""


def build(unit, tmp):
    target_s = os.path.join(tmp, "target.s")
    target = os.path.join(tmp, "target.o")
    base = os.path.join(tmp, "base.o")
    write_unit(LEFile(EXE), Project(), unit, target_s)
    subprocess.run(["as", "--32", "-o", target, target_s], check=True)
    r = subprocess.run(
        ["python3", os.path.join(ROOT, "tools", "cc.py"), unit.source, base],
        capture_output=True, text=True,
    )
    if r.returncode:
        log = r.stdout + r.stderr
        errors = [l for l in log.splitlines() if "Error!" in l or "Warning!" in l]
        sys.exit("\n".join(errors) or log)
    return target, base


def main(source, *flags):
    verbose = "-v" in flags
    source = os.path.relpath(os.path.abspath(source), ROOT)
    os.chdir(ROOT)
    unit = next((u for u in Project().units if u.source == source), None)
    if unit is None:
        sys.exit(f"{source}: no // FUNCTION: markers")

    total = 0
    with tempfile.TemporaryDirectory() as tmp:
        target, base = build(unit, tmp)
        target_relocs = function_relocs(target)
        base_relocs = function_relocs(base)
        for fn in unit.functions:
            out = subprocess.run(
                ["objdiff-cli", "diff", "-1", target, "-2", base, "-o", "-", "--format", "json", fn.name],
                capture_output=True, text=True, check=True,
            ).stdout
            diff = json.loads(out)
            left = next(s for s in diff["left"]["symbols"] if s["name"] == fn.name)
            right = next((s for s in diff["right"]["symbols"] if s["name"] == fn.name), None)
            pct = left.get("match_percent", 0.0)
            if pct == 100.0 and target_relocs.get(fn.name) != base_relocs.get(fn.name):
                print(f"{pct:7.2f}%  {fn.addr:#010x}  {fn.name}  RELOCATIONS DIFFER (offset, type, symbol, addend):")
                theirs = set(target_relocs.get(fn.name, []))
                ours = set(base_relocs.get(fn.name, []))
                for r in sorted(theirs - ours):
                    print(f"     target {r}")
                for r in sorted(ours - theirs):
                    print(f"     ours   {r}")
                continue
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
