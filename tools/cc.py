#!/usr/bin/env python3
"""Compile one source file with Watcom 10.6 into an ELF object objdiff can read.

wpp386/wcc386 (run under wibo) -> OMF .obj -> omf2elf -> ELF .o;
assembly sources go through GNU as.
The compiler and flags for each source come from config/cflags.txt.

Usage: tools/cc.py SOURCE OUTPUT.o [extra wpp386 flags...]
Requires $WATCOM_10_6 (set by the flake devShell).
"""

import fnmatch
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(__file__))
import omf2elf  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# -hd emits any debug info as DWARF segments, which omf2elf drops;
# -zm gives each function its own segment, as in the original; -fr stops
# the compiler writing .err files into the working directory.
COMMON_FLAGS = ["-zq", "-hd", "-zm", "-fr", "-i=include"]


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
    omf2elf.main(omf, output)


if __name__ == "__main__":
    main(*sys.argv[1:])
