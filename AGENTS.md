# Agent notes

This branch is the matching decompilation of Theme Hospital's `HOSPITAL.EXE`
(DOS, 1997): C++ in `src/` that Watcom 10.6 compiles to the original's code,
function for function. Read `README.md` for the status and the layout, and
`docs/MATCHING.md` for how the compiler behaves and which source idioms
reproduce it. `docs/BUGS.md` lists original bugs that the source keeps.

## Git
- Work on `decomp`. The playable port lives on `master`; this branch is
  merged into it, never the other way.
- Commit only when asked; stage by path; no `commit -a`, no
  `git checkout <file>` to undo, no push unless asked.
- Never commit game files. `orig/` and `build/` are gitignored; keep it so.

## Toolchain
Everything runs in the Nix dev shell, from the repository root:

    nix develop path:./nix
    python3 configure.py     # rerun after adding a source file
    ninja                    # units and base objects
    ninja build/report.json  # objdiff report and progress summary

The shell provides Watcom C/C++ 10.6 (through wibo), objdiff-cli, rizin with
rz-ghidra, and Open Watcom's `wdis`. The first `nix develop` downloads the
Watcom CD images' runtime libraries from archive.org. The game's files come
from `nix run path:./nix#gog-unpack -- 'PATH/setup_theme_hospital_v3_(28027).exe'`
into `orig/`; the matching targets need `orig/HOSPITAL.EXE`, compiling and
`ninja link` do not.

## Matching a function
1. Pick a function from `config/functions.txt`; view it with
   `tools/fn.py 0xADDR` (disassembly with relocations named, then rz-ghidra
   pseudo-C as a hint).
2. Write it under `src/`, `extern "C"` and named `fn_XXXXXXXX` (globals
   `D_XXXXXXXX`), with `// FUNCTION: 0xXXXXXXXX` on the line above. A new
   file needs `python3 configure.py` once.
3. `tools/check.py src/FILE.cpp` builds that file's unit alone and shows each
   function's match. For a diff in objdiff:
   `objdiff-cli diff -p . -u game/<unit> fn_XXXXXXXX_`; the `objdiff` GUI
   opens the project root and reads `objdiff.json`.
4. For a near-miss, `tools/variants.py src/FILE.cpp 0xADDR` searches operand
   orders, declaration orders and similar rewrites. Review `types` results:
   they can change behaviour.
5. A function no compiler variant reproduces becomes an assembly stand-in
   (`tools/standin.py`), listed in `docs/MATCHING.md`.

## Rules
- Compiler flags are `wpp386 -s -ot -d2 -zm`, in that order; per-file flags
  are in `config/cflags.txt`.
- Never reorder declarations: order decides stack slots and the layout of
  statics. Locals and parameters can be renamed freely; functions and globals
  need their `config/functions.txt` / `config/symbols.txt` entries changed
  with them. `tools/setfn.py 0xADDR NAME` renames or adds a function entry.
- Enumerators only replace literals, in `include/enums.h`: fields,
  parameters and variables keep their integer types. Bit masks are
  `#define`s.
- Format with `tools/format.py FILE...` (plain clang-format rewrites Watcom
  pragmas). Formatting must not change the build.
- Keep a bug of the original as it is and list it in `docs/BUGS.md`.

## Done
Work is done when everything still matches and the EXE is byte-identical:

    ninja build/report.json                  # every function matched
    python3 tools/lewrite.py --strict        # rebuild HOSPITAL.EXE from the units
    cmp build/hospital.exe orig/HOSPITAL.EXE

Keep `docs/MATCHING.md` true: a new idiom, flag finding or stand-in goes in it.

## Sharing the machine
The user works on this machine while agents run. Start builds under
`nice -n 19 ionice -c 3`, and put large output under `build/`.
