# Agent notes

`src/` and `include/` are the decompiled original: read them, never edit them.
`enhanced/` is a C++20 port of it (SDL3, ImGui for dev tools only), ported
function by function with the same RNG draw order. Read `enhanced/README.md`.

When `HERDR_ENV=1` and you lead or work in a multi-agent herdr session, also
read `docs/agents/herdr.md`.

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
GUI, so agents don't — drive it headless instead (below).

## Driving the app
`th_enhanced` runs headless with clicks and keys from a script, through the
real interface (commands in `enhanced/src/app/drive.h`):

    env -u LD_LIBRARY_PATH build/enhanced/th_enhanced --level 1 \
      --drive SCRIPT --out DIR        # exit 1 if an expect failed
    ... --control --out DIR           # the same commands on stdin, one answer
                                      # per line on stdout, step by step

- Screen coordinates are the game's 640 x 480 of what is shown; `cell C R`
  and `thing ID` aim at the map. `screenshot NAME` and `dump NAME` (the debug
  report's state text) go to DIR; `expect TEXT` checks the state text.
- The clock is driven and frames run unpaced: a run repeats exactly, in
  seconds. English text unless `--language`; the player's settings are not
  read. `--visible` shows the window, `--size WxH` sets it (default
  1280x720).
- Explore with screenshots to find coordinates, then keep the script as a
  test in `enhanced/tests/drive/` (`# Level N` in its header);
  `enhanced/tools/drive_tests.sh` runs them all. Fix interface bugs with one
  of these first, failing.

## Replay check
`enhanced/tools/replay/orig_trace.sh` runs an instrumented original headless in
DOSBox-X (its work folder is `$REPLAY_DIR`, default `build/replay`; use your
own when running in parallel). `th_replay` runs the same script in the port,
and `compare.py` diffs the two traces turn by turn, random draws included
(`--rand`). The usual SKIP list is
`computer_action,manage_rats,generate_disaster`.

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

## Sharing the machine
Several agents can run at once:
- DOSBox runs in one of `TH_REPLAY_SLOTS` (default 4) shared slots; a run
  waits for a free one. `enhanced/tools/slot.sh --status replay` shows who
  holds them. Never kill another agent's DOSBox.
- The instrumented `HOSPITAL.EXE` is cached in `build/replay-cache`, keyed by
  a hash of everything it is built from, so it's rebuilt only when the decomp
  or `instrument.py` changes.
- Driven runs of `th_enhanced` (`--drive`, `--control`) take one of
  `TH_DRIVE_SLOTS` (default 6) slots themselves, held until they exit;
  `enhanced/tools/slot.sh --status drive` shows them. Quit a `--control`
  session when done: it holds its slot while open.
- Cap your build and tidy jobs: `CMAKE_BUILD_PARALLEL_LEVEL=4`,
  `TIDY_JOBS=4`.

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
