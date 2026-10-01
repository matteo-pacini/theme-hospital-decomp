# Theme Hospital decompilation

A matching decompilation of the DOS release of Theme Hospital (Bullfrog, 1997):
C++ source that, compiled with the original toolchain, reproduces the code in
`HOSPITAL.EXE` function for function. Progress is measured with
[objdiff](https://github.com/encounter/objdiff).

No game files are included. You need your own copy.

## Status

As of 2026-09-30, `ninja report` counts all 2938 functions matched (100% of
code bytes): 1381 in the game, 813 in Bullfrog's libraries and 744 in the
runtime (Watcom C library, Miles, Smacker). Of these:

- 53 are assembly stand-ins (`*.standin.s`, listed in
  [docs/MATCHING.md](docs/MATCHING.md)): functions whose C has not yet been
  made to match (mostly register or stack-slot choices), after flag sweeps
  and source attempts. They count as matched, not as decompiled;
  `STANDIN_OFF=1` builds their C instead.
- 79, in 15 runtime modules, are built from Watcom's own 10.5/10.6 runtime
  libraries (`lib` entries in `config/cflags.txt`), which the EXE links.

The rest are compiled from the C++/C in `src/`, which also defines the
EXE's data. `tools/lewrite.py --strict` writes `build/hospital.exe` from
the built units alone, placed as `tools/relink.py` checks them, with the
DOS/4GW stub from the Watcom 10.5 toolchain: it is byte-identical to the
original `HOSPITAL.EXE`. `ninja link` also links the units with Open
Watcom's wlink into `build/link/hosp.exe` (see `tools/link.py`), which runs
in DOSBox-X but does not reproduce the original's layout.

## What you need

The game's files come from GOG's offline installer for Theme Hospital, from
your GOG library (Theme Hospital → offline backup installers). Only this
build is supported:

| | |
| --- | --- |
| File | `setup_theme_hospital_v3_(28027).exe` |
| Size | 174,951,008 bytes |
| SHA-256 | `3475c2bb4b262156af4d92e6b1e9e20075f1788a8776835b534e2917c7921e93` |
| SHA-1 | `9e26a9eafc59ba6a85894a6365391843ccdb42ca` |
| MD5 | `cb1a467f48532d3607fa5b9dd571a342` |

Check it with `sha256sum` (Linux), `shasum -a 256` (macOS) or
`certutil -hashfile <file> SHA256` (Windows). The game executable it
contains, which every build is compared against, is:

| | |
| --- | --- |
| `HOSPITAL.EXE` SHA-256 | `2bd86130c9c44bb9c76f993c3c0365e032414cb873f5740dc0aff9cf0a08bade` |
| `HOSPITAL.EXE` SHA-1 | `d108d34d4e435ce4ad1e9c9b76ab3ad6f000d6ae` |
| `HOSPITAL.EXE` MD5 | `a4a934ec18b66d131c53bdc71a429f93` |

You also need Nix with flakes enabled. The first `nix develop` downloads the
Watcom 10.5 and 10.6 CD images from WinWorld for their runtime libraries
(about 600 MB); everything else comes from nixpkgs.

## Unpacking

```sh
nix run .#gog-unpack -- 'PATH/setup_theme_hospital_v3_(28027).exe'
```

This checks the installer's name and SHA-256, extracts it with innoextract,
checks `HOSPITAL.EXE`'s SHA-1, and copies the game into `orig/`:
`HOSPITAL.EXE`, `DOS4GW.EXE`, `HOSPITAL.CFG`, `MODEM.INI` and the `ANIMS`,
`DATA`, `DATAM`, `INTRO`, `LEVELS`, `QDATA`, `QDATAM` and `SOUND`
directories. The installer holds one `HOSPITAL.CFG` per language; the
English one is extracted (`LANGUAGE=ENG`; edit it for another of FRE, GER,
ITA, SPA, SWE). Other files already in `orig/` are left alone. `orig/` is
gitignored: never commit game files.

## Playing

```sh
nix run .                # DOSBox Staging
nix run . -- --dosbox-x  # or DOSBox-X, or --dosbox for DOSBox 0.74
nix run . -- --wlink     # play the wlink-linked build/link/hosp.exe instead
```

This builds every unit, rebuilds `HOSPITAL.EXE` from them with
`tools/lewrite.py --strict`, checks it is byte-identical to `orig/`'s, links
`build/link/hosp.exe`, and starts the rebuilt game in DOSBox from a run
directory `build/run` (game data linked from `orig/`; its own
`HOSPITAL.CFG`, set to English on first run, and its saves). Run it from the
repository root.

## Developing

Enter the dev shell and build:

```sh
nix develop
python3 configure.py     # writes build.ninja and objdiff.json; rerun after adding a source file
ninja                    # target and base objects
ninja build/report.json  # objdiff report and progress summary (plain `ninja` does not rebuild it)
ninja link               # trial link with wlink; unresolved symbols in build/link/report.txt
```

`objdiff` (GUI) can open the project root directly; it reads `objdiff.json`.

Editors: `configure.py` also writes `compile_commands.json`, which lets
clangd and clang-tidy parse the sources (clang flags approximating Watcom's
dialect, with `tools/clangd/compat.h` pre-included). A few Watcom-only
constructs still show as errors (ordered comparisons of a pointer with 0,
one callback type). For Neovim, `.nvim.lua` (read with `exrc` on) turns
format-on-save off here, since plain clang-format rewrites Watcom pragmas;
use `tools/format.py` instead.

Before committing, check that everything still matches:

```sh
ninja build/report.json                       # every function matched (build/report.json measures)
python3 tools/lewrite.py --strict             # rebuild HOSPITAL.EXE from the units alone
cmp build/hospital.exe orig/HOSPITAL.EXE      # byte-identical
```

Day-to-day tools (see [Matching a function](#matching-a-function) and
[docs/MATCHING.md](docs/MATCHING.md)):

- `tools/fn.py 0xADDR`: disassembly of a function with relocations named,
  then rz-ghidra pseudo-C.
- `tools/check.py src/FILE.cpp`: builds one file's unit in isolation and
  shows each function's match; safe to run while other builds run.
- `tools/variants.py src/FILE.cpp 0xADDR`: searches source rewrites
  (operand and declaration order, block scopes, zero tests, variable reuse)
  for a near-miss.
- `tools/setfn.py 0xADDR NAME`: rename or add one entry in
  `config/functions.txt`.
- `tools/format.py FILE...`: clang-format (`.clang-format`) with Watcom
  `#pragma`s left exactly as written; `--check` lists files that would
  change. Formatting never changes the build, but check it as above.

Renaming is safe as long as the build still matches: locals and parameters
can be renamed freely; functions and globals need their entries in
`config/functions.txt` / `config/symbols.txt` changed with them; never
reorder declarations, since order decides stack slots and (for C files)
the layout of statics, whose names also decide their order (see "Static
data layout" in docs/MATCHING.md).

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
| `config/sections.txt` | a module's own data segments (compiler-generated tables, string literals, the data of data-only modules), emitted into its target object |
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
