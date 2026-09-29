# Theme Hospital decompilation

A matching decompilation of the DOS release of Theme Hospital (Bullfrog, 1997):
C++ source that, compiled with the original toolchain, reproduces the code in
`HOSPITAL.EXE` function for function. Progress is measured with
[objdiff](https://github.com/encounter/objdiff).

No game files are included. You need your own copy.

## Status

As of 2026-09-29, `ninja report` counts all 2938 functions matched (100% of
code bytes): 1385 in the game, 800 in Bullfrog's libraries and 753 in the
runtime (Watcom C library, Miles, Smacker). Of these:

- 62 are assembly stand-ins (`*.standin.s`, listed in
  [docs/MATCHING.md](docs/MATCHING.md)): functions whose C has not yet been
  made to match (mostly register or stack-slot choices), after flag sweeps
  and source attempts. They count as matched, not as decompiled;
  `STANDIN_OFF=1` builds their C instead.
- 79, in 15 runtime modules, are built from Watcom's own 10.5/10.6 runtime
  libraries (`lib` entries in `config/cflags.txt`), which the EXE links.

The rest are compiled from the C++/C in `src/`. The executable is not yet
relinked from these objects (`ninja link` reports what does not resolve).

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
   ninja link       # trial link; unresolved symbols in build/link/report.txt
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
| Watcom C/C++ 10.0a (`$WATCOM_10_0A`) | built the C runtime's printf/stdio modules |
| Watcom C/C++ 10.5 and 10.6 runtime libraries (`$WATCOM_LIBS_10_5`, `$WATCOM_LIBS_10_6`) | `LIB386` of each CD (downloaded from WinWorld, 245 + 349 MB, on first `nix develop`); several runtime units are built from their modules |
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
| `config/functions.txt` | every function: address, size, symbol (from `tools/discover.py`; edit single entries with `tools/setfn.py`) |
| `config/cflags.txt` | compiler, version and flags for each source file |
| `config/demo_map.txt`, `config/demo_data_map.txt` | retail functions and globals paired with the demo's names |
| `config/symbols.txt` | named data symbols |
| `config/relocs.txt` | relocations named after a symbol other than the one containing their target |
| `config/sections.txt` | a module's own data segments (compiler-generated tables), emitted into its target object |
| `src/` | decompiled C++ |
| `src/**/*.standin.s` | assembly stand-ins for functions of the source file of the same name (see `tools/standin.py`) |
| `include/` | shared headers |
| `tools/` | LE reader, splitter, compiler wrapper, OMF-to-ELF converter, matching aids |

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
5. For a near-miss, `tools/variants.py src/<file>.cpp 0xADDR` searches
   operand orders, declaration orders and similar rewrites for a better
   match.

See [docs/MATCHING.md](docs/MATCHING.md) for compiler behaviour and the
source idioms that reproduce it.
