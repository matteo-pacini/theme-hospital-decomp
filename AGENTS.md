# Agent notes

`src/` and `include/` are the decompiled original: read them, never edit them.
`enhanced/` is a C++20 port of it (SDL3, ImGui for dev tools only), ported
function by function with the same RNG draw order. Read `enhanced/README.md`.

## Multi-agent work (herdr)
Multi-agent work runs under herdr by default: one lead orchestrates, workers
each do a brief in their own pane.
- Before any herdr operation (panes, starting agents, messaging, reading
  their screens), run `herdr --skill` and follow what it prints.
- The lead is Claude Code on `claude-opus-5-5`, medium effort (high for a
  hard task); workers are `claude-sonnet-5-5`, medium effort.
- The default is the main workspace: `th-lead` on the repository's
  checked-out branch, workers `th-agent-NN`. A big unrelated task can get its
  own herdr worktree workspace (`th-lead-<tag>`, `th-agent-NN-<tag>`), to be
  merged back when done; if unclear, ask the user which.
- Leads talk to their workers, and to each other to coordinate, through
  herdr. Workers do not message or reply through herdr unless their own
  workspace's lead asks them to.
- Before prompting any agent, check its pane's prompt cache: cold with a
  large context means `/clear`, `/caveman`, then a self-contained prompt
  (docs/agents/herdr.md, "Lead").
- With `HERDR_ENV=1`, the lead and the workers also read
  `docs/agents/herdr.md`.

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
  test in `enhanced/tests/drive/`; `enhanced/tools/drive_tests.sh` runs
  them all and lists the header lines (`# Level N`, `# NoLevel`, `# Size`,
  `# Seed`). Fix interface bugs with one of these first, failing.

## Replay check
`enhanced/tools/replay/orig_trace.sh` runs an instrumented original headless in
DOSBox-X (its work folder is `$REPLAY_DIR`, default `build/replay`; use your
own when running in parallel). `th_replay` runs the same script in the port,
and `compare.py` diffs the two traces turn by turn, random draws included
(`th_replay --rand`, `TRACE_RAND=1` on the original's side). The original
skips rendering its view and copying the screen unless `TRACE_DRAW=1`; the
trace cache keys on it. The usual SKIP
list is empty: the computer hospitals (`computer_action`), rats
(`manage_rats`) and disasters (`generate_disaster`) are on in every replay.
`SKIP=computer_action` (th_replay `--skip`) still leaves the computer
players out on both sides. How each piece works, the
trace cache and the script format: enhanced/README.md.

Run every script (or the ones named) with the decompilation built (`ninja`)
and `build/enhanced` built; the decompilation's dev shell has python3 and
dosbox-x:

    env -u LD_LIBRARY_PATH nix develop path:./nix -c sh -c \
      'unset LD_LIBRARY_PATH; enhanced/tools/replay/replay_all.sh [SCRIPT ...]'

- `replay_all.sh` compares both sides and checks each script's `# Expect`
  lines, prints one line per script and a summary, and exits non-zero on any
  disagreement or failed expectation. It runs each script to its `# Turns N`
  (else 18300) on its `# Level N`; the defaults (SKIP, TURNS, RAND, the
  watchdog) live in `lib.sh`. It keeps a failing pair gzipped with its path
  printed and deletes the rest. Its settings (`BUILD`, `JOBS`, `KEEP`,
  `WATCHDOG`, `REPLAY_BASE`, ...) are listed at the top of the script.
- The original's traces are cached locally, so DOSBox runs only for a new or
  changed script, setting or decompilation. A cached trace is the reference:
  don't force fresh runs (`NO_CACHE=1` only when the cache is suspected
  stale), and never edit or hand-place its files.
- A script's header states what it exercises with `# Expect <text> [<op> <n>]`
  and `# Expect-not <text>`; when the generator moves its numbers,
  `expect_candidates.py SCRIPT TRACE` lists the events of the same shape.
  Never weaken an Expect line to make it pass. Common script traps (forced
  cures, wrong headers, dead commands): docs/agents/herdr.md, "Workers".
- Every script in `enhanced/tools/replay/scripts/` must agree to its turns.
  A worker's gate is split: its own new or changed scripts with fresh
  originals (`replay_all.sh NAME ...`), plus `CACHED_ONLY=1 replay_all.sh` over
  all the others, which compares the port with cached originals only, never
  starts DOSBox (seconds) and lists scripts without a cache entry as "no
  cache" and then exits 3 (1 if anything disagreed), so it never reads as a
  full pass. After a change to `instrument.py` or the decompilation every key
  is new and nothing is cached: fresh originals are needed then. The lead
  runs the full cold run once at integration.
- Whoever ports a function adds its trace events on both sides:
  `trace_event` in the port, the `instrument.py` EVENTS entry, and a line in
  `trace.h`.
- The trace names each random draw after the function that made it
  (`std::source_location`). A helper that draws must pass the original
  function's name, or the replays diverge.
- Player actions go through `Game::process_turn`, so they land in
  `Game::command_log` and scripts can drive them on both sides.
- Cover every branch with its own script, and report event counts.
  `enhanced/tools/coverage.sh` shows what no script reaches yet.
- New World, Game or Thing fields go in `core/world/debug_dump.cpp` and
  `core/world/snapshot.cpp`; size `static_assert`s enforce the snapshot.

## Sharing the machine
Several agents can run at once:
- DOSBox runs in one of `TH_REPLAY_SLOTS` (default 10) shared slots; a run
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
- Disk: `/tmp` is on the root filesystem, and a `--rand` replay trace is
  35-70 MB per side. Delete both traces once they agree; keep only diverging
  pairs (gzip them) until the divergence is fixed. Put large temporary output
  under `build/` or your scratch folder, not loose in `/tmp`.
- When your task is done, remove what you made: your traces and temporary
  files, your `REPLAY_DIR` (`build/replay-<you>*`, and `build/replay-all-<pid>*`
  the runner leaves) and any extra build folders. Keep `build/replay-cache`,
  `build/slots` and anything still in use by someone else.

## Debugging
The user files bugs as debug reports (F12's Debug report tab, or F10):
`debug-reports/<time>/` (in the repository when the data came from the development folder,
else in the state folder `ThemeHospitalEnhanced/debug-reports/` under
`$XDG_STATE_HOME`; see enhanced/README.md) holds a screenshot, `state.txt`, the note,
`snapshot.bin` and `commands.txt`. Load the snapshot with
`build/enhanced/th_replay --load` to reproduce what the user saw. F12's
Inspect shows a person's or object's fields live, with states named.
Details: enhanced/README.md.

## Style
Match the surrounding code, keep diffs minimal, and write comments about the
code as it stands, never about the change.
