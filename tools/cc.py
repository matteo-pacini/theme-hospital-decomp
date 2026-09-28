#!/usr/bin/env python3
"""Compile one source file with Watcom 10.6 into an ELF object objdiff can read.

wpp386/wcc386 (run under wibo) -> OMF .obj -> objconv -> ELF .o -> fixsyms;
assembly sources go through GNU as.
The compiler and flags for each source come from config/cflags.txt.

Usage: tools/cc.py SOURCE OUTPUT.o [extra wpp386 flags...]
Requires $WATCOM_10_6 (set by the flake devShell).
"""

import fnmatch
import os
import shutil
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(__file__))
import fixsyms  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# -hd emits any debug info as DWARF, which objconv drops, unlike Watcom's own
# format; -zm gives each function its own segment, as in the original.
COMMON_FLAGS = ["-zq", "-hd", "-zm", "-i=include"]


def compiler_for(source):
    """(compiler, flags) for a source path relative to the repo root."""
    for line in open(os.path.join(ROOT, "config", "cflags.txt")):
        line = line.split("#", 1)[0].split()
        if line and fnmatch.fnmatch(source, line[0]):
            return line[1], line[2:]
    raise KeyError(f"{source}: no entry in config/cflags.txt")


def dos_path(path):
    # Watcom parses '/' as an option prefix.
    return path.replace("/", "\\")


def main(source, output, *extra):
    watcom = os.environ["WATCOM_10_6"]
    base = os.path.splitext(output)[0]
    omf = base + ".obj"
    compiler, flags = compiler_for(os.path.relpath(os.path.abspath(source), ROOT))
    if compiler == "as":
        # Modules that were hand-written assembly: GNU as emits ELF directly.
        subprocess.run(["as", "--32", *flags, "-o", output, source], check=True)
        return
    subprocess.run(
        ["wibo", f"{watcom}/binnt/{compiler}.exe", *COMMON_FLAGS, *flags, *extra, f"-fo={dos_path(omf)}", dos_path(source)],
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
