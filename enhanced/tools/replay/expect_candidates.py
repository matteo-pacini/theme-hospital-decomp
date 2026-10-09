#!/usr/bin/env python3
"""For each "# Expect" line of a replay script, list the trace events that
share its non-numeric text (its shape), with their numeric fields, so an
Expect line whose numbers drift with the generator can be re-derived.

Usage: expect_candidates.py SCRIPT TRACE
"""

import re
import sys

NUM = re.compile(r"^-?\d+$")


def parse_expects(path):
    out = []
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.strip()
            if line.startswith("# Expect-not "):
                out.append(("not", line[len("# Expect-not ") :].strip()))
            elif line.startswith("# Expect "):
                rest = line[len("# Expect ") :].strip()
                words = rest.split()
                if len(words) >= 3 and words[-2] in (">=", "==", "<=") and NUM.match(words[-1]):
                    rest = " ".join(words[:-2])
                out.append(("count", rest))
    return out


def shape(text):
    return tuple("*" if NUM.match(t) else t for t in text.split())


def events(path):
    out = []
    with open(path, encoding="ascii", errors="replace") as f:
        for line in f:
            if line.startswith("E "):
                out.append(line[2:].rstrip("\r\n"))
    return out


def main(argv):
    script, trace = argv[0], argv[1]
    evs = events(trace)
    shapes = {}
    for e in evs:
        shapes.setdefault(shape(e), set()).add(e)
    for kind, text in parse_expects(script):
        if kind == "not":
            print(f"Expect-not {text}")
            continue
        sh = shape(text)
        cands = shapes.get(sh)
        if cands:
            print(f"Expect {text}  ->")
            for c in sorted(cands):
                print(f"    {c}")
            continue
        head = text.split()[0]
        near = {}
        for e in evs:
            if e.split()[0] == head:
                near[e] = near.get(e, 0) + 1
        if near:
            print(f"Expect {text}  ->  shape differs; events named {head}:")
            for e, n in sorted(near.items()):
                print(f"    {e} x{n}")
        else:
            print(f"Expect {text}  ->  NO EVENT NAMED {head}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))