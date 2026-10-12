# Theme Hospital decompilation

A matching decompilation of the DOS release of Theme Hospital (Bullfrog, 1997):
C++ source that, compiled with the original toolchain, reproduces the code in
`HOSPITAL.EXE` function for function. Progress is measured with
[objdiff](https://github.com/encounter/objdiff).

No game files are included. You need your own copy.

This branch, `decomp`, is the matching decompilation and its tooling. The
playable port built from it, Theme Hospital Enhanced, lives on the
[`master`](https://github.com/matteo-pacini/theme-hospital-decomp/tree/master)
branch. Fixes to `src/` and `include/` are made here and merged into `master`,
never the other way.

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
EXE's data.

## Two builds of the EXE

The same compiled units make two executables:

| | Byte-identical build | wlink build |
| --- | --- | --- |
| Output | `build/hospital.exe` | `build/link/hosp.exe` |
| Made by | `tools/lewrite.py --strict` (after `ninja`) | `ninja link` (`tools/link.py`, Open Watcom's wlink) |
| Layout | every function and global at its original address | chosen by wlink |
| Result | byte for byte the original `HOSPITAL.EXE` | a working game, not identical to the original |
| Needs `orig/HOSPITAL.EXE` | yes: the build checks the placement against it and compares the result | no: the repository's objects alone |
| Use it to | prove the decompilation exact | change the code |

The byte-identical build is the decompilation's result: `lewrite.py`
places every unit where `tools/relink.py` checks it belongs, fills the gaps
as the original linker did and writes the headers, fixup tables and the
DOS/4GW stub of the Watcom 10.5 toolchain itself, so the file compares
equal to the original. Because everything sits at a fixed address, a
function that compiles to a different size no longer fits.

The wlink build is for changing the code. wlink links the units like any
other program, so functions and data may grow, shrink or be added. No code,
data or layout comes from the original EXE: the data the units do not
define is laid out from `config/` as zeroed storage, and the stack is a
segment of its own. It runs in DOSBox-X; check changes there, since only
the byte-identical build is compared with the original.

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
contains, which the matching build is compared against, is:

| | |
| --- | --- |
| `HOSPITAL.EXE` SHA-256 | `2bd86130c9c44bb9c76f993c3c0365e032414cb873f5740dc0aff9cf0a08bade` |
| `HOSPITAL.EXE` SHA-1 | `d108d34d4e435ce4ad1e9c9b76ab3ad6f000d6ae` |
| `HOSPITAL.EXE` MD5 | `a4a934ec18b66d131c53bdc71a429f93` |

You also need Nix with flakes enabled. The flake lives in `nix/` and is used
as `path:./nix`, from the repository root: a `path:` flake copies only that
directory into the Nix store, where the repository root as a flake would copy
the whole tree on every run. The first `nix develop path:./nix` downloads the
Watcom 10.5 and 10.6 CD images from WinWorld for their runtime libraries
(about 600 MB); everything else comes from nixpkgs.

## Unpacking

```sh
nix run path:./nix#gog-unpack -- 'PATH/setup_theme_hospital_v3_(28027).exe'
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
nix run path:./nix                 # DOSBox Staging
nix run path:./nix -- --dosbox-x   # or DOSBox-X, or --dosbox
nix run path:./nix -- --wlink      # play the wlink build (for changed code) instead
nix run path:./nix -- --486        # at the minimum spec (486DX2-66); --pentium for a Pentium 133
```

This builds every unit, rebuilds `HOSPITAL.EXE` from them with
`tools/lewrite.py --strict`, checks it is byte-identical to `orig/`'s, links
`build/link/hosp.exe`, and starts the rebuilt game in DOSBox from a run
directory `build/run` (game data linked from `orig/`; its own
`HOSPITAL.CFG`, set to English on first run, and its saves). Run it from the
repository root.

`--wlink` builds and plays only the wlink build (see [Two builds of the
EXE](#two-builds-of-the-exe)), without the byte-identical rebuild and its
check, so it needs only the game's files, not `orig/HOSPITAL.EXE`:
`DOS4GW.EXE`, `HOSPITAL.CFG` and the data directories (`DATA` and the others
above).

## Developing

Enter the dev shell and build:

```sh
nix develop path:./nix
python3 configure.py     # writes build.ninja and objdiff.json; rerun after adding a source file
ninja                    # target and base objects (without orig/HOSPITAL.EXE: base objects only)
ninja build/report.json  # objdiff report and progress summary (plain `ninja` does not rebuild it)
ninja link               # the wlink build, for changed code; unresolved symbols in build/link/report.txt
```

`objdiff` (GUI) can open the project root directly; it reads `objdiff.json`.

The matching side (`build/target`, split from `orig/HOSPITAL.EXE` by
`tools/split.py`; `ninja all`, `ninja report`, objdiff, `tools/check.py`,
`tools/lewrite.py`) needs the original EXE. Compiling the units and linking
them with wlink (`ninja link`) does not: without the EXE,
`configure.py` says so, and the matching targets fail with that message if
built.

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

Named constants live in `include/enums.h`. Enumerators only replace
literals: fields, parameters and variables keep their integer types, since
an enum-typed declaration changes layout and mangled names. Bit masks are
`#define`s, because Watcom compiles `field & ENUMERATOR` differently from
`field & 8`. The data-only table files start with `// clang-format off` to
keep their hand-laid-out initialisers.

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
| Watcom C/C++ 10.5 and 10.6 runtime libraries (`$WATCOM_LIBS_10_5`, `$WATCOM_LIBS_10_6`) | `LIB386` of each CD (downloaded from WinWorld, 245 + 349 MB, on first `nix develop path:./nix`); several runtime units are built from their modules |
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
| `nix/` | the flake (toolchain, dev shell, `nix run` apps), its play and unpack scripts, the owdemangle filter |

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

[docs/BUGS.md](docs/BUGS.md) lists bugs in the original code that the
decompilation keeps.

## Legal

This is an unofficial, non-commercial fan project, not affiliated with or
endorsed by Electronic Arts, Bullfrog Productions, RAD Game Tools or
Sybase. Theme Hospital is (C) 1997 Bullfrog Productions Ltd / Electronic
Arts Inc.; trademarks belong to their owners. The repository holds no game
assets and no original binaries: you need your own copy of the game (the
GOG release) to build it. Please don't open issues or pull requests with
game files or leaked material.

The tools, build system and documentation are GPLv3 ([COPYING](COPYING)).
The reconstructed code in `src/` and `include/`, including the Miles Sound
System, Smacker and Watcom runtime code linked into the game, is not ours to
license; see [LICENSE](LICENSE). If you hold rights to any of it and object,
open an issue and it will be taken down.
