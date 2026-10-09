# Theme Hospital Enhanced

A new engine for Theme Hospital, written in C++20 with SDL3 and Dear ImGui,
for Windows, macOS and Linux. It reads the data of your own copy of the game
and draws it at any resolution and aspect ratio. The decompilation in `../src`
is its reference: behaviour is ported from it function by function.

It contains no game files. At start-up it looks for the game data and, if it
finds none, says what it needs and lets you pick the folder.

## Status

Milestone 1, the map at any resolution:

- the game's data formats: RNC-compressed files (both CRCs checked), the
  palette, the two sprite codings (map blocks and panel/screens) and level
  maps;
- a level's starting map built as the game builds it (public land, your plot,
  grass, decorations and partition walls), drawn isometrically into an 8-bit
  indexed view the size of the window, then scaled by a whole number for
  sharp pixels;
- the bottom panel at native size, centred;
- scrolling (the arrow keys or numpad 8/2/4/6, the window edges, middle or
  right drag) and zoom (mouse wheel; automatic keeps at least 480 lines);
- a tools window (F12) with the UI scale.

Milestone 2, the level as it starts:

- the composite animations (VSTART/VFRA/VELE/VLIST with the VSPR sprites),
  including mirroring, per-thing layer choices and see-through drawing
  through the in-game blend table (GHOST66);
- the objects a level places when it starts (as recreate_map, adjust_mapwho
  and place_object do: for the campaign levels, the entrance doors), with
  the game's random generator;
- the game's drawing order: floors, then each row's walls and what stands on
  its cells, with things behind a wall drawn through a mask of the walls in
  front of them;
- shadows cast by walls;
- each level's scroll area and the game's mouse pointer;
- the panel and pointer at their own UI scale, apart from the view's zoom.

Milestone 3a, time passing:

- the level settings (LEVELS/*.SAM, the FULL set) read as the game reads
  them;
- the game turn: game speeds (keys 1-5, P to pause), the date (a day every
  50 turns), the opening countdown, the per-turn random draws, and objects'
  animations;
- the panel's balance, date and reputation marker, and the opening
  countdown's watch, in the game's fonts and language (HOSPITAL.CFG's, or
  `--language N`: 0 English, 1 French, 2 German, 3 Italian, 4 Spanish,
  5 Swedish).

Milestone 3b, people:

- patients arriving as the game makes them (create_a_peep, make_a_peep:
  illness, looks, needs, health, insurance, cash), with the game's random
  draws in order; the rival hospitals' patients are made and dropped, but
  the computer players' own draws are not ported yet;
- the patients' action sequences (DATA/P_COMP.LEX and the step table): with
  no reception desk, a patient goes home through an entrance, as in the game;
- the game's path finding (wall following, its four-waypoint plans and its
  limits), walking, and the sliding entrance doors;
- a path finding tool in the tools window (F12): people's planned routes
  drawn over the view.

Milestone 4 so far, the game's interface:

- the start-up flow: the intro, title and name, main menu, difficulty and
  tutorial choice, and the level's area movie (Smacker movies with their
  sound); `--level N` skips it and starts level N at once;
- the pull-down menu bar and the requester windows, with the original art;
  quitting asks first; Options speed, sound and volume, Display shadows;
- the panel's buttons; furnish corridor (buy, place, rotate, cost) and hire
  staff (applicants, faces, placing them) through the game's commands;
- sound effects from the game's sound bank, and the music (the XMI songs on
  an emulated OPL3) with the jukebox (J, or Options > Jukebox);
- staff at work, the advisor, wages and the month's accounts;
- patients at reception: the queues, waiting, drinks, benches, going home
  fed up; and the litter, vomit and puddles they leave;
- the town map (T, or the panel): the land and its plots, buying them, the
  overlays and the heating;
- the full-screen charts (F1-F8, the Charts menu, the panel): statement,
  bank manager and loans, policy, research, graphs, staff list, casebook
  prices and the status screen;
- building rooms (the panel's build room): the requester, the blueprint,
  doors and windows, the walls, the cost and the room's objects;
- the GP's office and the diagnosis rooms in use: the GP sending patients
  on to diagnosis, the doctors' work in general diagnosis, the cardiogram,
  the scanner, ultrascan, blood machine and X-ray (these four once
  researched) and psychiatry (which also cures, the king complex among
  others), the ward's nurse putting patients to bed, the fees, the machines'
  wear and the handymen's repairs;
- the pharmacy (drugs, and the cabinet for invisibility and transparency)
  and inflation: patients cured and going home cured, or not; patients
  dying, floating away or taken by the grim reaper into the gates of hell;
- staff rooms and toilets: tired staff (past the policy's "rest at", or
  ordered to rest) going for a break to the nearest staff room and resting
  there on its sofa, at pool or the video game until no longer tired, then
  back to work; patients with a full bladder leaving their queue for the
  toilets, a free toilet and sink, and wetting themselves with none;
- clicking people on the map (their pointers as the mouse passes over
  them): the patient window (health graph or treatments, happiness, thirst
  and warmth, send home, guess a cure, the casebook) and the staff and
  handyman windows (face, name, wage, qualifications, happiness, tiredness
  and ability, the handyman's priorities, pick up, sack, the staff list),
  each with a view of its person; a click on it centres the hospital on
  them, a right click moves on to the next one;
- emergencies: offered by a call, answered on the fax (EV_ANSWER_PLAYERCALL
  2, then EV_CHOICE_RESULT 31 to accept or 30 to refuse; accepting refused
  while one runs), the helicopter and its shadow coming down on the level's
  pad with the announcer naming the illness, its patients going straight
  for their treatment (or home at once with no room for it), the watch
  above the panel and its clock counting down, and the end: the bonus and
  a better reputation, or a worse one and the patients left dying, and the
  result's fax;
- the player's calls: their fax icons rising from behind the panel and
  sliding into their slots (the emergency's flashing), a click on one (or
  on the patient) opening the fax (the choice screen, full screen on the
  fax machine, the game running behind it): a patient with no treatment
  (send home, wait, or the research department, once autopsies can be
  done) or no diagnosis (send home, guess the cure, once known and curable,
  or wait), a new illness found, the emergency offered and its result; the
  answers carried out; the keypad (its numbers dial through, the cheats do
  nothing);
- the statistics pass of every 32nd turn (the patients' and staff's
  happiness, thirst, warmth, bladders and waits, the idle staff, the
  machines' wear, this month's visitors) and the advisor's advice from it:
  thirst, warmth, toilets, unhappy patients and those gone home fed up,
  long waits, too few or too many doctors and nurses, worn machines, litter,
  plants, benches, the first death and cure, deaths near the level's limit,
  the rooms open, a room just opened, a treatment not paid for, a doctor
  cracked up;
- machines wearing out and blowing up: their smoke drawn and their warning
  sounds and announcements from three quarters of their strength, a
  breakdown likelier the more worn past it; the explosion's debris flying
  and falling, the room wrecked (its objects gone, the machines' wrecks and
  cracked walls left), the staff in it killed and its patients dying, the
  room destroyed and its queue sent elsewhere, the reputation lost; and the
  machine window's commands, replace (EV_REPLACE_MACHINE 0x8a) and call a
  handyman (EV_THINGPAGE_REQUEST 0x76);
- research: researchers at work in the research room (the computer, the
  atom analyser, desks and cabinets), their points, back to research after
  a break; the daily spending by the policy's allocation and the
  casebook's concentration: new treatment and diagnosis machines, drugs,
  better drugs and machine upgrades (stronger, cheaper machines), a
  category running out and closing; objects becoming available in their
  month; what is researched offered in the build and buy requesters; the
  research screen's bars; the advisor's announcements spoken; drugs
  improved again and again once researched. The slack tongue slicer, the
  hair restorer and the fracture clinic cure; the electrolysis, jelly vat
  and decontamination rooms work as the slicer's but are not checked
  against the game yet;
- the operating theatre: two surgeons taking turns at its screen, its
  operating table, lamp, sinks and X-ray viewer, the operation itself, and
  the ward and diagnosis its patients need;
- the training room: a consultant starting the projector, lecturing and
  teaching from the bookcase and skeleton, students on its lecture chairs
  gaining ability and the surgeon, psychiatrist and researcher skills, and
  their breaks and return;
- editing a finished room and picking up objects: a room reselected, its
  objects lifted, put back or sold and reopened;
- the machine window (name, patients seen, strength, close, replace and call
  a handyman) and the queue window (its capacity, front, back, go home, go to
  reception and to another queue);
- epidemics: a contagious patient, the call declared or covered up (a fine
  or a reward), the inspector, vaccinations and the epidemic's end;
- VIP visits and earthquakes: the VIP's visit and verdict, and the
  earthquake's tremor and damage;
- the staff's pay-raise demands (raised and bonused, or refused), the status
  icons over people and machines, the panel's information bar under the
  pointer, plants wilting, watering and dying with the handyman's priorities,
  and the panel watch's open-hospital button;
- calls that time out: a doctor's or nurse's patient who cannot reach the
  room called again, then given up and sent on;
- rats: rat holes made by the litter, rats coming out of them, and shooting
  them (a miss, a hit and the rat killed, its number floating over it);
- disasters: the heating failing cold or hot until its timer runs out
  (sooner with enough handymen), the player's heating ignored meanwhile,
  everyone feeling sick when there is litter, a disaster refused when the
  hospital cannot have it, and the advisor's messages;
- the tutorial: the advisor's steps from furnishing the corridor to opening
  the room building (from step 0x2b on not yet);
- the year end: the hall of fame, the charts, the awards and trophies and
  the year's score; winning or losing at the year end, and a win offered at
  a quarter's end and declined;
- saving and loading: File > Save and Load, the quick save and load (Alt+S,
  Alt+L), the Continue slot written on quitting, and the load game screen.

Not yet: autopsies and the DNA fixer.
Route planning defaults to A*; the game's wall following stays selectable
in the tools window.

## Game data

The data folder is the one holding `DATA`, `LEVELS` and `QDATA` (file names
are matched case-insensitively). It is looked for in this order:

1. `--data <folder>` on the command line;
2. the development folder, `../orig` (the repository's `orig/`, where
   `nix run path:./nix#gog-unpack` puts the GOG release). Builds for others
   should turn it off with `-DTH_DEV_DATA_DIR=`;
3. the folder chosen before, saved in the user's preferences folder;
4. next to the program, then the working directory.

If none holds the game, a screen lists where it looked and what was missing,
with a button to choose the folder.

## Building

From the repository root, with Nix:

    nix run path:./nix#enhanced            # build (Release) and run
    nix run path:./nix#enhanced -- --level 5

or by hand (CMake 3.24+, a C++20 compiler; SDL3 is used from the system if
found, otherwise fetched, and Dear ImGui is always fetched):

    env -u LD_LIBRARY_PATH nix develop path:./nix#enhanced
    unset LD_LIBRARY_PATH
    cmake -S enhanced -B build/enhanced -G Ninja
    cmake --build build/enhanced
    build/enhanced/th_enhanced

`LD_LIBRARY_PATH` is unset outside the shell and again in it: a host's,
pointing at libraries built against a newer glibc, breaks linking.

Tests: `ctest --test-dir build/enhanced`. The tests that need the game data
read it from the development folder and are skipped without it.

Without a window, `--screenshot OUT.bmp --data FOLDER [--level N] [--size WxH]`
renders a level's start view to a BMP.

`--drive SCRIPT --out DIR` plays the whole program headless from a script
of clicks, keys and waits, with screenshots, state dumps and expectations,
on a driven clock so that a run repeats exactly; `--control` takes the same
commands on stdin, answering each on stdout (src/app/drive.h).
`--visible` shows the window while it runs. The scripts in tests/drive are
interface tests: `tools/drive_tests.sh` runs them (all, or the ones named)
from the repository root with the build in `DRIVE_BUILD` (default
build/enhanced), its output in build/drive-tests/. A script's header picks
how it starts: `# Level N` the level (default 1), `# NoLevel` the start-up
screens instead, `# Size WxH` the window size (default 1280x720; the menu
bar is only at the top in the game's 640x480 layout) and `# Seed DIR` saves
copied from DIR into the run's save folder first. `type TEXT` types into a
focused text field, `key [MOD+]NAME` holds Shift, Ctrl or Alt, and
`screenshot-ui` captures after the ImGui layer (the tools and the quick
report's note box).

The tools window (F12) has a debug report: Save report
writes `debug-reports/<time>/` under the folder the game was started from,
with `screenshot.png` (the window without the tools), `state.txt` (the
screen, the last command, money, finance and the hospitals, building and
placing, every room, every person and object with all their fields, the map
around the mouse, and the turn in the replay trace format; `dump_game`,
core/world/debug_dump.h), `note.txt` (the text typed in the tool),
`snapshot.bin` (the whole game state, core/world/snapshot.h) and
`commands.txt` (every command and other input since the level started,
`Game::command_log`), for review. F10 does the same at once, on any screen:
it pauses the game, captures the frame already shown (without the note box)
and its state, and shows a note box (Enter saves, Shift+Enter adds a line,
Escape discards); Shift+F10 saves at once with an empty note.
`TH_DEBUG_REPORT_DIR` picks another folder for the reports.

A snapshot loads only in a build with the same snapshot id
(`snapshot_build_id`: a hash of the snapshotted structures' headers and the
snapshot code, comments ignored, and of the sizes of what is copied as raw
bytes). commands.txt's `@placing`, `@build` and `@hospital` lines are raw
bytes too, so th_replay refuses a command log holding them from another
build. To reproduce a report with the build that saved it:

    th_replay --load debug-reports/X/snapshot.bin --turns +N \
        --dump dump.txt --focus CELL --render view.png --camera X,Y

runs N turns on from the moment it was saved, then writes `dump_game` of the
end state (the map around CELL) and the hospital view as the game draws it
(`--camera` from state.txt's camera line, `--size WxH`; a `.ppm` name writes
a PPM). `--script FILE` adds commands, its turns counted from the
snapshot's. `th_replay --script debug-reports/X/commands.txt --turns T`
plays the level again from its start up to turn T instead. `--save SAVE`
writes the end state as a saved game that the game's Load reads (its
slots are `<player name>.h<slot>` in the save folder; a save checks the
layout hash, not the snapshot id, so ordinary source edits keep it
loadable). `--astar` plans routes with A* rather than the game's wall
following, from the level's start (a snapshot or command log keeps its own
choice); the replay check needs wall following, the default.

With `--advisor`, th_replay also runs the advisor in each turn, as the
game's frame does, and notes each advice given as a comment line
(`# advice <situation>`), which the instrumented original writes too,
followed by the seed. compare.py ignores them; to compare them, diff the
two traces' `grep -o '^# advice [0-9]*'`.

Replay check against the original: `tools/replay/orig_trace.sh` relinks an
instrumented HOSPITAL.EXE from the decompilation and runs it in DOSBox-X
without a window, writing a trace of every turn (the random seed, date,
balance, hospitals and things) with the turn's events in order (a patient
made, a complex given, a queue joined, money spent, ...; the list is in
core/world/trace.h); `th_replay` writes the same trace from this engine, and
`tools/replay/compare.py` reports the first turn they differ: the first
differing event, with those round it, before the state. `TRACE_RAND=1
orig_trace.sh ...` and `th_replay --rand` also trace every draw from the
random seed with the function that drew it. Whoever ports a function the
game writes an event for, or adds an event, writes it on both sides, at the
same point: in the port (`trace_event`) and in `tools/replay/instrument.py`'s
EVENTS; a port named otherwise than the game's function names it in its
draws (`old_newrand(seed, "name")`). See the scripts for their arguments.
`tools/replay/replay_all.sh` runs every script (or the ones named) through
both sides, checks that each agrees and satisfies the `# Expect` lines in its
header (`# Expect <text> [<op> <n>]`: an event line whose text after `E `
starts with `<text>` occurs `<op> <n>` times, `op` one of `>= == <=`, default
`>= 1`; `# Expect-not <text>`: no such event), and prints one line per script
and a summary; `check_expect.py` does the checking on both traces. It runs
`JOBS` scripts at once (default 4) with the th_replay of `BUILD` (default
build/enhanced). `tools/replay/expect_candidates.py SCRIPT TRACE` lists,
for each of a script's `# Expect` lines, the trace's events of the same
shape (the same text, any numbers) with their numbers, to re-derive an
Expect line whose numbers drift with the generator.

Original trace cache. `build/replay-cache/traces/` holds the original
side of past runs, so `replay_all.sh` runs DOSBox only for a script it
has no entry for. Each entry is one script's trace, compressed with zstd
when it is on PATH (gzip otherwise) and named by a key of everything that
fixes the trace: the instrumented executable's cache key (a hash of the
decompilation's sources, objects and instrument.py), the script file's bytes
(not its path), the last turn, the level, the SKIP list, RAND, `orig_trace.sh`
(the DOSBox configuration and run folder) and the original's `HOSPITAL.CFG`,
`MODEM.INI` and `DOS4GW.EXE`, and which dosbox-x runs it; the game data
folders are not hashed. An entry is stored only for a run that finished:
the instrumented game ends its trace with a `# end` line (after the last
turn, or at a win or loss), and a trace without it is refused, though DOSBox
exits 0. `orig_trace.sh` clears the run's `SAVE` first, so the original's
once-only advisor flags never leak between runs. `replay_all.sh` and
`orig_trace.sh`'s cached wrapper (`cached_orig_trace.sh`, which replay_all.sh
calls) consult it; a hit is decompressed in a moment and prints `orig trace:
cache hit`. The cache is machine local and git-ignored, and each machine
fills its own under its own executable key: any change to the key above (a
new or edited script, other turns, level or SKIP, an edited `orig_trace.sh`
or CFG, a decompilation or `instrument.py` change) misses only the affected
entries and refreshes them. `enhanced/tools/replay/test_cache.sh` tests
the cache with a fake DOSBox. To fill it for every script in one go, run
`enhanced/tools/replay/trace_cache.sh prefetch` (from any directory; the
same SKIP, turns, RAND and watchdog as `replay_all.sh`, from `lib.sh`,
so it fills exactly what `replay_all.sh` hits; `PREFETCH_JOBS`, default
4). Entries with RAND on are about 0.5 MB each on average (up to 1.5 MB),
so the whole set is some 50-100 MB per executable key until `prune` drops
old keys; at about a minute per script on the DOSBox slots, it is best
left to run unattended. `NO_CACHE=1` forces a fresh run and refreshes the
entry; use it only when the cache is suspected stale, not to check a port
change, since a cached original trace is the reference while the executable
key is unchanged. `trace_cache.sh status` prints the entry count, size,
current key and stale count; `trace_cache.sh prune` removes entries whose
executable key is no longer current, and `prune DAYS` also removes entries
older than DAYS days. Never edit or hand-place cache files.

The player's commands checked so far are the scripts in
`tools/replay/scripts/`, lines of `turn type value arg` (th_replay --script
and orig_trace.sh's SCRIPT; the EV_* numbers in include/enums.h). Each
script's header comments say what it exercises, and its `# Expect` and
`# Expect-not` lines check it on both traces; `# Level N` picks the level
(th_replay `--level N`, orig_trace.sh's LEVEL) and `# Turns N` the last
turn. A script is compared with the default SKIP, `computer_action` (rats
and disasters stay on), by hand as:

    enhanced/tools/replay/orig_trace.sh orig.txt 18000 \
        enhanced/tools/replay/scripts/gp_office.txt \
        computer_action
    build/enhanced/th_replay \
        --script enhanced/tools/replay/scripts/gp_office.txt \
        --turns 18000 --out enhanced.txt
    enhanced/tools/replay/compare.py orig.txt enhanced.txt

What the scripts leave unexercised: `tools/coverage.sh` (in the dev shell, GCC)
builds th_replay with `-DTH_COVERAGE=ON` in build/coverage, runs every script
there and writes an HTML report and `summary.txt` to build/coverage/report/.

## Bugs in the original

`init_edit_room`'s `u8` overflow for an empty room is in
[docs/BUGS.md](../docs/BUGS.md); the port uses an `int`, so a script that
opens a room before `commit_doors` has finished cannot agree
(`tools/replay/scripts/win_level1.txt` waits for it).

## Layout

    src/core/data/    file formats: RNC, sprites, palette, levels, text, Smacker,
                      the data folder
    src/core/render/  indexed surface, sprites, frames, shadows, the hospital view, panel
    src/core/world/   the map and its things, level start, the turn, arrivals, action
                      sequences, path finding
    src/core/audio/   the sound bank's effects and the XMI music
    src/core/ui/      the game's screens and windows: menus, requesters, charts,
                      fax, town map, help, year end, win and lose
    src/app/          SDL3 window and input, data-folder search, settings, driving
    src/app/ui/       ImGui: the tools window, debug report, path finding tool
                      and the missing-data screen
    src/app/audio/    mixes the sounds and music onto the audio device
    tests/            ctest tests
    tests/drive/      driven interface tests (tools/drive_tests.sh)
    tools/replay/     the replay check against the original
    tools/help/       check_table.py: the help hotspot table against the original
    tools/            drive_tests.sh, coverage.sh, tidy.sh (clang-tidy) and slot.sh
                      (shared slots for DOSBox and driven runs)
