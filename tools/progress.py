#!/usr/bin/env python3
"""Print a one-screen progress summary from an objdiff report.

Usage: tools/progress.py build/report.json
"""

import json
import sys


def line(name, m):
    total = int(m.get("total_code", 0))
    matched = int(m.get("matched_code", 0))
    fns = f"{m.get('matched_functions', 0)}/{m.get('total_functions', 0)}"
    pct = 100 * matched / total if total else 0
    return f"{name:<10} {pct:6.2f}%  {matched:>8}/{total:<8} bytes  {fns:>11} functions"


def main(path):
    r = json.load(open(path))
    print(line("all", r["measures"]))
    for c in r.get("categories", []):
        print(line(c["id"], c["measures"]))


if __name__ == "__main__":
    main(*sys.argv[1:])
