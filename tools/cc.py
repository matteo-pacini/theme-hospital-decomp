#!/usr/bin/env python3
"""Compile one source file with Watcom C++ 10.6 into an ELF object objdiff can read.

wpp386 (run under wibo) -> OMF .obj -> objconv -> ELF .o -> fixsyms.

Usage: tools/cc.py SOURCE OUTPUT.o [extra wpp386 flags...]
Requires $WATCOM_10_6 (set by the flake devShell).
"""

import os
import shutil
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(__file__))
import fixsyms  # noqa: E402

CFLAGS = ["-zq", "-s", "-od", "-zm", "-i=include"]


def dos_path(path):
    # Watcom parses '/' as an option prefix.
    return path.replace("/", "\\")


def main(source, output, *extra):
    watcom = os.environ["WATCOM_10_6"]
    base = os.path.splitext(output)[0]
    omf = base + ".obj"
    subprocess.run(
        ["wibo", f"{watcom}/binnt/wpp386.exe", *CFLAGS, *extra, f"-fo={dos_path(omf)}", dos_path(source)],
        check=True,
    )
    # objconv 2.54.1 aborts (fortify buffer overflow) on long output basenames.
    with tempfile.TemporaryDirectory() as tmpdir:
        tmp = os.path.join(tmpdir, "o.o")
        subprocess.run(["objconv", "-felf32", "-wd1211", omf, tmp], check=True, stdout=subprocess.DEVNULL)
        fixsyms.main(tmp)
        shutil.move(tmp, output)


if __name__ == "__main__":
    main(*sys.argv[1:])
