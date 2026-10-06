# Agent notes

`src/` and `include/` are the decompiled original: read them, never edit them.
`enhanced/` is a C++20 port of it (SDL3, ImGui for dev tools only), ported
function by function with the same RNG draw order. Read `enhanced/README.md`.

## Git
- Enhanced work lives on branch `enhanced` only, never on master.
- Commit only when asked; stage by path; no `commit -a`, no
  `git checkout <file>` to undo, no push unless asked.

## Build and check
Always unset `LD_LIBRARY_PATH` (the host's points at libraries built
against a newer glibc, which breaks linking):

    env -u LD_LIBRARY_PATH nix develop path:./nix#enhanced -c sh -c \
      'unset LD_LIBRARY_PATH; cmake --build build/enhanced &&
       TH_TEST_DATA_DIR=orig ./build/enhanced/th_tests && enhanced/tools/tidy.sh'

Done means all of these are green:
- zero compiler warnings, "all tests passed" (`th_tests <filter>` runs a subset);
- `enhanced/tools/tidy.sh` reports 0 warnings: functions of at most 60 lines,
  50 statements, 15 branches, 6 parameters, nesting 4 and cognitive
  complexity 25. Split long functions into helpers named after the original
  function and the step. No NOLINT;
- `clang-format --dry-run` prints nothing. clang-format and clang-tidy are in
  the dev shell; a missing binary makes the check look clean;
- the replay check agrees (below) for any simulation change.

python3 is not on PATH outside the decomp's dev shell (`nix develop path:./nix`).
Run the game with `nix run path:./nix#enhanced -- --level N`; the user runs the
GUI, so agents don't.

## Replay check
`enhanced/tools/replay/orig_trace.sh` runs an instrumented original headless in
DOSBox-X (its work folder is `$REPLAY_DIR`, default `build/replay`; use your
own when running in parallel). `th_replay` runs the same script in the port,
and `compare.py` diffs the two traces turn by turn, random draws included
(`--rand`). The usual SKIP list is
`computer_action,manage_rats,generate_disaster,generate_mayors`.

- The scripts in `enhanced/tools/replay/scripts/` are lines of
  `turn event value arg`, with `# Level N` in the header. All of them must
  agree to turn 18000. The year end (18250) differs until year-end awards are
  ported.
- Whoever ports a function adds its trace events on both sides:
  `trace_event` in the port, the `instrument.py` EVENTS entry, and a line in
  `trace.h`.
- The trace names each random draw after the function that made it
  (`std::source_location`). A helper that draws must pass the original
  function's name, or the replays diverge.
- Player actions go through `Game::process_turn`, so they land in
  `Game::command_log` and scripts can drive them on both sides.
- Cover every branch with its own script, and report event counts.
- New World, Game or Thing fields go in `core/world/debug_dump.cpp` and
  `core/world/snapshot.cpp`; size `static_assert`s enforce the snapshot.
- The original keeps once-only advisor flags in `run/SAVE/HORZA.HDT`. Clear
  SAVE before comparing `# advice` lines.

## Debugging
F12 opens the tools window:
- Inspect: pick a person or object and see its fields live, with states named.
- Path finding.
- Debug report: saves `debug-reports/<time>/` with a screenshot,
  `state.txt`, a note, `snapshot.bin` and `commands.txt`. Load the snapshot
  with `th_replay --load` to reproduce what the user saw.

## Style
Match the surrounding code, keep diffs minimal, and write comments about the
code as it stands, never about the change.
