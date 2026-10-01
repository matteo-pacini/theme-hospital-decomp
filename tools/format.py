#!/usr/bin/env python3
"""Format source files with clang-format (.clang-format), leaving every
#pragma exactly as written.

clang-format rewrites the inside of pragmas (Watcom's `#pragma aux f parm
[eax]` becomes `parm[eax]`), so each pragma, with its continuation lines, is
swapped for a placeholder before formatting and put back afterwards.
Formatting must not change the build: check `ninja build/report.json` and
`tools/lewrite.py --strict` after running it.

Usage: tools/format.py [--check] FILE...
  --check  list the files that would change, change nothing; exit 1 if any
"""

import re
import subprocess
import sys

PRAGMA = re.compile(r"^[ \t]*#[ \t]*pragma\b")
PLACEHOLDER = "#pragma clang_format_keep_{}"


def mask(text):
    """The text with each pragma replaced by a numbered placeholder, and the pragmas."""
    lines = text.split("\n")
    out, kept = [], []
    i = 0
    while i < len(lines):
        if PRAGMA.match(lines[i]):
            j = i
            while lines[j].endswith("\\") and j + 1 < len(lines):
                j += 1
            kept.append("\n".join(lines[i : j + 1]))
            out.append(PLACEHOLDER.format(len(kept) - 1))
            i = j + 1
        else:
            out.append(lines[i])
            i += 1
    return "\n".join(out), kept


def unmask(text, kept):
    for n, pragma in enumerate(kept):
        # The placeholder is a whole line; clang-format may only have indented it.
        line = re.compile(rf"^[ \t]*{re.escape(PLACEHOLDER.format(n))}$", re.M)
        text, found = line.subn(lambda _: pragma, text)
        if found != 1:
            raise RuntimeError(f"pragma placeholder {n} lost or duplicated by clang-format")
    return text


def formatted(path):
    with open(path, encoding="latin-1", newline="") as f:
        original = f.read()
    masked, kept = mask(original)
    result = subprocess.run(
        ["clang-format", f"--assume-filename={path}"],
        input=masked.encode("latin-1"),
        capture_output=True,
        check=True,
    )
    return original, unmask(result.stdout.decode("latin-1"), kept)


def main(args):
    check = "--check" in args
    paths = [a for a in args if a != "--check"]
    changed = []
    for path in paths:
        original, new = formatted(path)
        if new != original:
            changed.append(path)
            if not check:
                with open(path, "w", encoding="latin-1", newline="") as f:
                    f.write(new)
    for path in changed:
        print(path)
    return 1 if check and changed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
