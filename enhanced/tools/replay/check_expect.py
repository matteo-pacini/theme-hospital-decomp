#!/usr/bin/env python3
"""Check a replay script's "# Expect" header lines against one or more
replay traces (core/world/trace.h), so that a scenario that stops exercising
what it is for fails the replay check instead of quietly agreeing.

The script's header lines (anywhere in it, so its purpose can be written next
to what it checks):

  # Expect <text> [<op> <n>]   a trace event line whose text after "E "
                               starts with <text> occurs <op> <n> times; <op>
                               is one of >=, == or <=; without it, >= 1
  # Expect-not <text>          no such event line occurs

The advisor's advice given also counts, as `advice 0xNN` (NN the situation in
two hex digits, the `# advice N` comment lines of the traces).

Usage: check_expect.py SCRIPT TRACE [TRACE ...]
  Checks every trace and exits 0 when all of them satisfy the script's
  expectations, 1 when one fails, 2 when a header line looks like an Expect
  line but does not parse (a typo must not drop the check silently). Each
  failed expectation is reported on standard output; --verbose also lists the
  ones that hold, with their counts.
"""

import re
import sys

# Any comment that starts with Expect (whatever its case or spacing) but is
# not one of the two forms above is a malformed header line.
LOOKS_LIKE_EXPECT = re.compile(r"^#\s*[Ee]xpect")


def parse_expectations(path):
    """(expectations, errors) for `path`: each expectation is (kind, text,
    op, n), kind "count" with op and n, or "not" with op and n None; each
    error is the offending header line."""
    out, errors = [], []
    with open(path, encoding="utf-8", errors="replace", newline=None) as f:
        for line in f:
            line = line.strip()
            if line.startswith("# Expect-not "):
                text = line[len("# Expect-not ") :].strip()
                if text:
                    out.append(("not", text, None, None))
                else:
                    errors.append(line)
            elif line.startswith("# Expect "):
                rest = line[len("# Expect ") :].strip()
                if not rest:
                    errors.append(line)
                    continue
                words = rest.split()
                op, n = ">=", 1
                if len(words) >= 3 and words[-2] in (">=", "==", "<=") and words[-1].lstrip("-").isdigit():
                    op, n = words[-2], int(words[-1])
                    rest = " ".join(words[:-2])
                out.append(("count", rest, op, n))
            elif LOOKS_LIKE_EXPECT.match(line):
                errors.append(line)
    return out, errors


ADVICE_LINE = re.compile(r"^# advice (\d+)(?: seed=[0-9a-fA-F]+)?\s*$")


def event_counts(path):
    """{event text: count} over the "E " lines of a trace, and over its
    "# advice N" comment lines (the advisor's situation N given), counted as
    events "advice 0xNN" so that `# Expect advice 0x5a` can check them."""
    counts = {}
    with open(path, encoding="ascii", errors="replace", newline=None) as f:
        for line in f:
            if line.startswith("E "):
                text = line[2:].rstrip("\r\n")
                counts[text] = counts.get(text, 0) + 1
                continue
            m = ADVICE_LINE.match(line)
            if m:
                text = f"advice 0x{int(m.group(1)):02x}"
                counts[text] = counts.get(text, 0) + 1
    return counts


def matches(counts, text):
    """How many events start with `text`."""
    return sum(n for event, n in counts.items() if event.startswith(text))


def check(script, traces, verbose):
    expectations, errors = parse_expectations(script)
    if errors:
        for line in errors:
            print(f"{script}: malformed Expect header line: {line!r}")
        return 2
    failed = False
    for trace in traces:
        counts = event_counts(trace)
        for kind, text, op, want in expectations:
            got = matches(counts, text)
            if kind == "not":
                ok = got == 0
                detail = f"Expect-not {text}: {got} match(es)"
            else:
                ok = {">=": got >= want, "==": got == want, "<=": got <= want}[op]
                detail = f"Expect {text} {op} {want}: {got}"
            if not ok:
                print(f"{trace}: {detail} [FAIL]")
                failed = True
            elif verbose:
                print(f"{trace}: {detail} [ok]")
    return 1 if failed else 0


def main(argv):
    verbose = "--verbose" in argv
    args = [a for a in argv if a != "--verbose"]
    if len(args) < 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2
    return check(args[0], args[1:], verbose)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
