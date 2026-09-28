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

## Names from the 1997 demo

The March 1997 demo of the game (`HOSPITAL.EXE`, sha1
`d37aaea89e22be35467b3050280394c74a5e0b07`, on archive.org as `HOSP_zip`) was
built from nearly the same source and ships with Watcom debug info: 437 source
modules and about 4,000 named functions and globals. It was built with
optimisation, so its code differs from retail; `tools/mapdemo.py` and
`tools/mapdata.py` match retail functions and globals to it by strings, call
graphs, link order and struct offsets. The results are committed as
`config/demo_map.txt` and `config/demo_data_map.txt`, so the demo is only
needed to regenerate them (extract it to `orig/demo-1997-03/`, then
`wdump -Dx HOSPITAL.EXE > HOSPITAL.debug.txt`).

## Toolchain

The game code was built with Watcom C/C++ 10.x (`wpp386`), unoptimised, with
each function in its own segment. The flake provides:

| Tool | Purpose |
| --- | --- |
| Watcom C/C++ 10.6 (`$WATCOM_10_6`) | period compiler, Win32 host binaries |
| wibo 1.2.0 | runs the Win32 compiler on Linux |
| objdiff-cli 3.8.1 | diffing and progress reports |
| rizin + rz-ghidra | disassembly and decompiler output (`pdg`) |
| Open Watcom v2 | `wdis` for inspecting OMF objects |

Compiler flags for the game: `wpp386 -s -ot -d2 -zm`, in that order (`-d2`: full
debug info, which also disables optimisation; the earlier `-ot` still steers
register allocation). Library flags are in `config/cflags.txt`.

## Layout

| Path | Contents |
| --- | --- |
| `config/functions.txt` | every function: address, size, symbol (from `tools/discover.py`) |
| `config/symbols.txt` | named data symbols |
| `src/` | decompiled C++ |
| `include/` | shared headers |
| `tools/` | LE reader, splitter, compiler wrapper |

so they count towards the total.

## Matching a function

1. Pick a function from `config/functions.txt`; view it with
   `tools/fn.py 0xADDR`.
2. Write it in a source file under `src/`, declared `extern "C"` and named
   `fn_XXXXXXXX` (the trailing `_` in the symbol comes from Watcom's register
   calling convention). Reference unnamed globals as `D_XXXXXXXX`.
3. Put `// FUNCTION: 0xXXXXXXXX` on the line above it (a new file needs
   `python3 configure.py` once).
4. `tools/check.py src/<file>.cpp` for per-function results, `ninja report`
   for overall progress, or diff interactively:
   `objdiff-cli diff -p . -u game/<unit> fn_XXXXXXXX_`.

See [docs/MATCHING.md](docs/MATCHING.md) for compiler behaviour and the
source idioms that reproduce it.
