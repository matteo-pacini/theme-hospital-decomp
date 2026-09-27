#!/usr/bin/env python3
"""List functions an objdiff report counts as matched whose relocations differ.

objdiff compares at most one relocation per x86 instruction; this catches
the rest. Exits non-zero if any are found.

Usage: tools/verify.py build/report.json
"""

import json
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from elfrelocs import function_relocs  # noqa: E402


def main(report_path):
    report = json.load(open(report_path))
    config = {u["name"]: u for u in json.load(open("objdiff.json"))["units"]}
    bad = []
    for unit in report["units"]:
        cfg = config.get(unit["name"])
        if not cfg or "base_path" not in cfg:
            continue
        matched = [f["name"] for f in unit.get("functions", []) if f.get("fuzzy_match_percent") == 100.0]
        if not matched:
            continue
        target = function_relocs(cfg["target_path"])
        base = function_relocs(cfg["base_path"])
        bad += [f"{unit['name']}: {name}" for name in matched if target.get(name) != base.get(name)]
    for line in bad:
        print(f"relocations differ despite 100% match: {line}")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main(*sys.argv[1:])
