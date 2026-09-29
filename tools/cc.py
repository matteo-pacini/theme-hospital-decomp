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
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(__file__))
import omf2elf  # noqa: E402
import standin  # noqa: E402

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


# Compiler versions a config/cflags.txt entry can name as `compiler@version`,
# each with the environment variable the flake sets to its installation.
VERSIONS = {"10.6": "WATCOM_10_6", "10.0a": "WATCOM_10_0A"}

# Versions whose CD runtime libraries (its LIB386 directory) the flake
# provides, for config/cflags.txt entries built from a library module
# (`lib@version`), with the environment variable that locates them.
LIBRARIES = {"10.6": "WATCOM_LIBS_10_6"}


def library_path(version, library):
    """Path of a library (e.g. `DOS/CLIB3R.LIB`) of a version's CD."""
    return os.path.join(os.environ[LIBRARIES[version or "10.6"]], library)


def command(compiler, flags):
    """Command line up to the output and source arguments for a
    config/cflags.txt entry's compiler (`wpp386`, `wcc386@10.0a`, ...)."""
    name, _, version = compiler.partition("@")
    watcom = os.environ[VERSIONS[version or "10.6"]]
    common = list(COMMON_FLAGS)
    if "-zm-" in flags:
        # Not a compiler option: the module was built with all its functions
        # in one segment (see tools/split.py).
        flags = [f for f in flags if f != "-zm-"]
        common.remove("-zm")
    if version == "10.0a":
        # 10.0a has no -fr; its error files are removed after the compile.
        common.remove("-fr")
    return ["wibo", f"{watcom}/binnt/{name}.exe", *common, *flags]


def relocate_local_calls(sections, symbols):
    """Give a library module's calls to its own functions relocations: the
    library resolved them, where the compiler (and the target) relocates a
    call to a function's entry even within one segment."""
    for sec in sections:
        if not sec.code:
            continue
        entries = {off: name for name, (s, off, _) in symbols.items() if s is sec}
        relocated = {at + k for at, _, _ in sec.relocs for k in range(4)}
        for op in range(len(sec.data) - 4):
            site = op + 1
            if sec.data[op] != 0xE8 or site in relocated:
                continue
            callee = entries.get(site + 4 + struct.unpack_from("<i", sec.data, site)[0])
            if callee is not None:
                struct.pack_into("<i", sec.data, site, -4)
                sec.relocs.append((site, omf2elf.R_386_PC32, ("sym", callee)))


def main(source, output, *extra):
    base = os.path.splitext(output)[0]
    omf = base + ".obj"
    compiler, flags = compiler_for(os.path.relpath(os.path.abspath(source), ROOT))
    name, _, version = compiler.partition("@")
    if name == "lib":
        # A module linked from Watcom's runtime library: its object is the
        # library's, and the source only documents it.
        library, module = flags
        path = library_path(version, library)
        obj = omf2elf.library_module(path, module)
        # The module as the library has it, for tools/link.py.
        open(omf, "wb").write(obj)
        model = omf2elf.convert(obj)
        relocate_local_calls(*model[:2])
        omf2elf.write_elf(*model, output)
        return
    if compiler == "as":
        # Modules that were hand-written assembly: GNU as emits ELF directly.
        subprocess.run(["as", "--32", *flags, "-o", output, source], check=True)
        return
    try:
        subprocess.run([*command(compiler, flags), *extra, f"-fo={dos_path(omf)}", dos_path(source)], check=True)
    finally:
        err = os.path.splitext(os.path.basename(source))[0] + ".err"
        if "@" in compiler and os.path.exists(err):
            os.remove(err)
    model = omf2elf.convert(open(omf, "rb").read())
    standin.apply(source, model, omf2elf.Section)
    omf2elf.write_elf(*model, output)


if __name__ == "__main__":
    main(*sys.argv[1:])
