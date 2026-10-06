#!/usr/bin/env python3
"""Check enhanced's help hotspot table against the decompilation's hireshelp.

Parses the `hireshelp` initializer in ../src/coords.cpp and the `kHiresHelp`
initializer in enhanced/src/core/ui/helpmsg_table.cpp and compares them entry
by entry: the four coordinates, the packed text/slot and the callback name.

Usage: python3 enhanced/tools/help/check_table.py [repo_root]
Exits non-zero on any difference.
"""

import re
import sys
from pathlib import Path


def entries(path: Path, decl: str) -> list[str]:
    text = path.read_text()
    start = text.index(decl)
    start = text.index("{", start) + 1
    body = text[start : text.index("};", start)]
    # One initializer per line, whitespace normalised.
    out = []
    for line in body.splitlines():
        line = line.strip().rstrip(",")
        if not line:
            continue
        out.append(re.sub(r"\s+", " ", line))
    return out


def main() -> int:
    root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[3]
    orig = entries(root / "src/coords.cpp", "hireshelp[350]")
    port = entries(root / "enhanced/src/core/ui/helpmsg_table.cpp", "kHiresHelp[]")

    if len(orig) != len(port):
        print(f"count differs: original {len(orig)}, port {len(port)}")
        return 1

    bad = 0
    for i, (a, b) in enumerate(zip(orig, port)):
        # The port wraps the same values in the same braces; the type name is
        # not in the line, so the text must match exactly.
        if a != b:
            print(f"entry {i} differs:\n  original: {a}\n  port:     {b}")
            bad += 1

    if bad:
        print(f"{bad} of {len(orig)} entries differ")
        return 1
    print(f"{len(orig)} entries agree")
    return 0


if __name__ == "__main__":
    sys.exit(main())
