#!/usr/bin/env python3
"""Compare two replay traces (core/world/trace.h) and report the first turn
where they differ: the turn, the field (a header field, or a thing's) and
both values. Exits 0 if they agree over the turns both hold, 1 otherwise.

Usage: compare.py ORIGINAL ENHANCED [--all]
  --all  list every difference of the first differing turn, not just the first
"""

import sys


def turns(path):
    """Yield (turn, header fields, {thing id: fields}) per turn of a trace."""
    turn = header = things = None
    with open(path, encoding="ascii", errors="replace", newline=None) as f:
        for line in f:
            line = line.rstrip("\r\n")
            if line.startswith("T "):
                if turn is not None:
                    yield turn, header, things
                head, *fields = line[2:].split()
                turn, header, things = int(head), dict(kv.split("=", 1) for kv in fields), {}
            elif line.startswith("H ") and turn is not None:
                hosp, *fields = line[2:].split()
                header.update((f"hospital {hosp} {k}", v) for k, v in (kv.split("=", 1) for kv in fields))
            elif line.startswith(" ") and turn is not None:
                tid, *fields = line.split()
                things[int(tid)] = dict(kv.split("=", 1) for kv in fields)
    if turn is not None:
        yield turn, header, things


def differences(a_head, a_things, b_head, b_things):
    for key in dict.fromkeys([*a_head, *b_head]):
        if a_head.get(key) != b_head.get(key):
            yield key, a_head.get(key, "-"), b_head.get(key, "-")
    for tid in sorted(a_things.keys() | b_things.keys()):
        a, b = a_things.get(tid), b_things.get(tid)
        if a is None or b is None:
            yield f"thing {tid}", fmt(a), fmt(b)
            continue
        for key in a:
            if a[key] != b.get(key):
                yield f"thing {tid} {key}", f"{a[key]} ({fmt(a)})", f"{b.get(key, '-')} ({fmt(b)})"


def fmt(fields):
    return "free" if fields is None else " ".join(f"{k}={v}" for k, v in fields.items())


def main(argv):
    show_all = "--all" in argv
    paths = [a for a in argv if a != "--all"]
    if len(paths) != 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2
    b_iter = turns(paths[1])
    count = 0
    for (a_turn, a_head, a_things), b in zip(turns(paths[0]), b_iter):
        b_turn, b_head, b_things = b
        if a_turn != b_turn:
            print(f"turn numbers differ: {a_turn} (original) and {b_turn} (enhanced)")
            return 1
        diffs = list(differences(a_head, a_things, b_head, b_things))
        if diffs:
            print(f"first divergence at turn {a_turn} (after {count} turns in lockstep):")
            for field, a, b in diffs if show_all else diffs[:1]:
                print(f"  {field}\n    original: {a}\n    enhanced: {b}")
            if not show_all and len(diffs) > 1:
                print(f"  ({len(diffs) - 1} more; --all lists them)")
            return 1
        count += 1
    print(f"{count} turns agree")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
