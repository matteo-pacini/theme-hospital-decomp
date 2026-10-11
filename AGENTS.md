# Agent notes

`src/` and `include/` are the decompiled original: read them, never edit them.
`enhanced/` is a C++20 port of it (SDL3, ImGui for dev tools only), ported
function by function with the same RNG draw order. Read `enhanced/README.md`.

## Multi-agent work
Multi-agent work runs under herdr: one lead briefs workers, each in its own
pane. Before any herdr operation run `herdr --skill` and follow what it
prints; with `HERDR_ENV=1`, also read `docs/agents/herdr.md` (models,
layout, briefs, worktrees, messaging).

## Git
- Enhanced work lives on branch `enhanced` only, never on master.
- Commit only when asked; stage by path; no `commit -a`, no
  `git checkout <file>` to undo, no push unless asked.

## Gate
Work is done when the gate passes:

    enhanced/tools/check.sh [SCRIPT ...]

It enters the dev shells itself and stops at the first failure: the build
with zero warnings, `th_tests`, `tidy.sh`, clang-format, the drive tests,
the install tests, then every replay script (or the ones named) against the
original. `CACHE=only` compares with cached originals only and never starts
DOSBox, for checking as you go; it exits 3 when a script has no cache entry,
so it never counts as a pass. Settings and timings: enhanced/README.md.

Code must also meet what tidy enforces: functions of at most 60 lines, 50
statements, 15 branches, 6 parameters, nesting 4 and cognitive complexity
25. Split long functions into helpers named after the original function and
the step. No NOLINT.

For the edit-compile loop (always unset `LD_LIBRARY_PATH`: the host's points
at libraries built against a newer glibc, which breaks linking):

    nice -n 19 ionice -c 3 env -u LD_LIBRARY_PATH \
      nix develop path:./nix#enhanced -c sh -c \
      'unset LD_LIBRARY_PATH; cmake --build build/enhanced &&
       TH_TEST_DATA_DIR=orig ./build/enhanced/th_tests [FILTER]'

python3 and dosbox-x are only in the decompilation's dev shell
(`nix develop path:./nix`). The user runs the GUI (`nix run
path:./nix#enhanced -- --level N`); agents drive it headless instead.

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
  read.
- Explore with screenshots to find coordinates, then keep the script as a
  test in `enhanced/tests/drive/` (header lines: enhanced/README.md). Fix
  interface bugs with one of these first, failing.

## Replays
A replay script drives the same player actions through the instrumented
original in DOSBox-X and through `th_replay`; the traces must agree turn by
turn, random draws and advisor messages included. Scripts live in
`enhanced/tools/replay/scripts/`; the format and tools are in
enhanced/README.md.
- Every simulation change needs scripts that reach it: cover every branch
  with its own script and report the event counts that show it ran.
  `enhanced/tools/coverage.sh` shows what no script reaches yet.
- A script's header states what it exercises (`# Expect <text> [<op> <n>]`,
  `# Expect-not <text>`). Never weaken an Expect line to make it pass;
  `expect_candidates.py` lists events of the same shape when numbers move.
- Whoever ports a function adds its trace events on both sides:
  `trace_event` in the port, the `instrument.py` EVENTS entry, and a line in
  `trace.h`. A helper that draws random numbers passes the original
  function's name (`std::source_location`), or the replays diverge.
- Player actions go through `Game::process_turn`, so they land in
  `Game::command_log` and scripts can drive them on both sides.
- New World, Game or Thing fields go in `core/world/debug_dump.cpp` and
  `core/world/snapshot.cpp`; size `static_assert`s enforce the snapshot.
- The original's traces are cached, so DOSBox runs only for a new or changed
  script, setting or decompilation. A cached trace is the reference: never
  edit its files, and use `CACHE=refresh` only when the cache is suspected
  stale. A change to `src/` or `instrument.py` makes every original run
  fresh (an hour or more): `check.sh` counts the missing ones first and
  stops above `FRESH_MAX` (10); fill the cache with
  `trace_cache.sh prefetch`, or raise `FRESH_MAX`.

## Sharing the machine
The user works on this machine while agents run.
- `check.sh` runs at the lowest priority with job caps for several agents.
  Start anything else you run by hand (builds, tests, replays) under
  `nice -n 19 ionice -c 3`, with `CMAKE_BUILD_PARALLEL_LEVEL=2`, and
  `JOBS=1` for replays.
- DOSBox and driven runs take machine-wide slots (`enhanced/tools/slot.sh
  --status replay|drive` shows who holds them). Never kill another agent's
  DOSBox; quit a `--control` session when done.
- `/tmp` is on the root filesystem and a replay trace is 35-70 MB a side:
  put large output under `build/` or your scratch folder.
- When done, remove what you made (traces, temp files, your `REPLAY_DIR`
  and extra build folders); keep `build/replay-cache`, `build/slots` and
  anything someone else is using.

## Debugging
The user files bugs as debug reports (F12's Debug report tab, or F10):
`debug-reports/<time>/` (in the repository when the data came from the
development folder, else in
`$XDG_STATE_HOME/ThemeHospitalEnhanced/debug-reports/`)
holds a screenshot, `state.txt`, the note, `snapshot.bin` and
`commands.txt`. Load the snapshot with `build/enhanced/th_replay --load` to
reproduce what the user saw. F12's Inspect shows a person's or object's
fields live, with states named. Details: enhanced/README.md.

## Style
Match the surrounding code, keep diffs minimal, and write comments about the
code as it stands, never about the change.
