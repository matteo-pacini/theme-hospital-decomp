#!/usr/bin/env python3
"""Compare two replay traces (core/world/trace.h) and report the first turn
where they differ. Within a turn the events (E lines) come first: the first
event missing, extra, out of order or with different values, with the events
round it on both sides; then the state: the field (a header field, or a
thing's) and both values. Exits 0 if they agree over the turns both hold, 1
otherwise.

Usage: compare.py ORIGINAL ENHANCED [--all] [--context N] [--no-events]
                  [--ignore-event KIND]... [--ignore-field NAME]...
  --all           list every state difference of the first differing turn
  --context N     events shown before and after the first differing one (5)
  --no-events     compare the state alone
  --ignore-event  leave out events of KIND (e.g. rand, or one known to differ)
  --ignore-field  leave out the field NAME (key=value) of every event
"""

import sys
from itertools import zip_longest


def turns(path):
    """Yield (turn, header fields, events, {thing id: fields}) per turn of a
    trace; an event is its text after "E "."""
    turn = header = events = things = None
    with open(path, encoding="ascii", errors="replace", newline=None) as f:
        for line in f:
            line = line.rstrip("\r\n")
            if line.startswith("T "):
                if turn is not None:
                    yield turn, header, events, things
                head, *fields = line[2:].split()
                turn, header, events, things = int(head), dict(kv.split("=", 1) for kv in fields), [], {}
            elif line.startswith("E ") and turn is not None:
                events.append(line[2:])
            elif line.startswith("H ") and turn is not None:
                hosp, *fields = line[2:].split()
                header.update((f"hospital {hosp} {k}", v) for k, v in (kv.split("=", 1) for kv in fields))
            elif line.startswith(" ") and turn is not None:
                tid, *fields = line.split()
                things[int(tid)] = dict(kv.split("=", 1) for kv in fields)
    if turn is not None:
        yield turn, header, events, things


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


def filtered(events, kinds, fields):
    out = []
    for e in events:
        words = e.split()
        if words[0] in kinds:
            continue
        out.append(" ".join(w for w in words if w.split("=", 1)[0] not in fields or "=" not in w))
    return out


def event_divergence(a, b):
    """(index of the first difference, what it is) between two turns'
    events, or None if they agree."""
    for i, (x, y) in enumerate(zip(a, b)):
        if x == y:
            continue
        later_a, later_b = x in b[i + 1:], y in a[i + 1:]
        if later_a and later_b:
            return i, "out of order"
        if later_a:
            return i, "extra in enhanced"
        if later_b:
            return i, "missing in enhanced"
        if x.split()[0] == y.split()[0]:
            return i, "different values"
        return i, "different events"
    if len(a) != len(b):
        return min(len(a), len(b)), "missing in enhanced" if len(a) > len(b) else "extra in enhanced"
    return None


def show_events(name, events, at, context):
    print(f"  {name}:")
    lo = max(0, at - context)
    for i in range(lo, min(len(events), at + context + 1)):
        print(f"    {'>' if i == at else ' '} {i:4} {events[i]}")
    if at >= len(events):
        print(f"    > {at:4} (none)")


def main(argv):
    show_all = no_events = False
    context = 5
    kinds, fields, paths = set(), set(), []
    args = iter(argv)
    for arg in args:
        if arg == "--all":
            show_all = True
        elif arg == "--no-events":
            no_events = True
        elif arg == "--context":
            context = int(next(args))
        elif arg == "--ignore-event":
            kinds.add(next(args))
        elif arg == "--ignore-field":
            fields.add(next(args))
        else:
            paths.append(arg)
    if len(paths) != 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2
    count = 0
    missing = object()
    for a, b in zip_longest(turns(paths[0]), turns(paths[1]), fillvalue=missing):
        if a is missing or b is missing:
            longer = "enhanced" if a is missing else "original"
            print(f"the traces differ in length: after {count} turns in lockstep the {longer} one "
                  f"continues, the other ends")
            return 1
        (a_turn, a_head, a_events, a_things), (b_turn, b_head, b_events, b_things) = a, b
        if a_turn != b_turn:
            print(f"turn numbers differ: {a_turn} (original) and {b_turn} (enhanced)")
            return 1
        if not no_events:
            a_ev, b_ev = filtered(a_events, kinds, fields), filtered(b_events, kinds, fields)
            found = event_divergence(a_ev, b_ev)
            if found:
                at, what = found
                print(f"first divergence at turn {a_turn} (after {count} turns in lockstep): event {at}, {what}")
                show_events("original", a_ev, at, context)
                show_events("enhanced", b_ev, at, context)
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
