# Theme Hospital decompilation

A matching decompilation of the DOS release of Theme Hospital (Bullfrog, 1997):
C++ source that, compiled with the original toolchain, reproduces the code in
`HOSPITAL.EXE` function for function. Progress is measured with
[objdiff](https://github.com/encounter/objdiff).

No game files are included. You need your own copy.

## Setup

1. Extract the GOG installer into `orig/` (the game's `HOSPITAL.EXE` must end
   up at `orig/HOSPITAL.EXE`, sha1 `d108d34d4e435ce4ad1e9c9b76ab3ad6f000d6ae`):

   ```sh
   nix develop -c innoextract -d orig setup_theme_hospital_v3_(28027).exe
   ```

2. Enter the dev shell and build:

   ```sh
   nix develop
   python3 configure.py
   ninja            # target + base objects
   ninja report     # build/report.json and a progress summary
   ```

   `objdiff` (GUI) can open the project root directly; it reads `objdiff.json`.

## Toolchain

The game code was built with Watcom C/C++ 10.x (`wpp386`), unoptimised, with
each function in its own segment. The flake provides:

| Tool | Purpose |
| --- | --- |
| Watcom C/C++ 10.6 (`$WATCOM_10_6`) | period compiler, Win32 host binaries |
| wibo 1.2.0 | runs the Win32 compiler on Linux |
| objconv | converts Watcom OMF objects to ELF |
| objdiff-cli 3.8.1 | diffing and progress reports |
| rizin + rz-ghidra | disassembly and decompiler output (`pdg`) |
| Open Watcom v2 | `wdis` for inspecting OMF objects |

Compiler flags: `wpp386 -s -od -zm` (see `tools/cc.py`).

## Layout

| Path | Contents |
| --- | --- |
| `config/functions.txt` | every function: address, size, symbol (from `tools/discover.py`) |
| `config/symbols.txt` | named data symbols |
| `config/units.txt` | which source file implements which functions |
| `src/` | decompiled C++ |
| `include/` | shared headers |
| `tools/` | LE reader, splitter, compiler wrapper |

Functions not listed in `config/units.txt` are placed in hidden `auto/` units
so they count towards the total.

## Matching a function

1. Pick a function from `config/functions.txt`; view it with
   `rizin -c 'aaa; pdf @ 0xADDR; pdg @ 0xADDR' orig/HOSPITAL.EXE`.
2. Write it in a source file under `src/`, declared `extern "C"` and named
   `fn_XXXXXXXX` (the trailing `_` in the symbol comes from Watcom's register
   calling convention). Reference unnamed globals as `D_XXXXXXXX`.
3. Add the symbol to that file's line in `config/units.txt`.
4. `ninja report`, or diff interactively:
   `objdiff-cli diff -p . -u game/<unit> fn_XXXXXXXX_`.
