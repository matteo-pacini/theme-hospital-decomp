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
- scrolling (arrow keys or WASD, the window edges, middle or right drag) and
  zoom (mouse wheel; automatic keeps at least 480 lines);
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
- the game turn: game speeds (keys 1-5 until the menu bar exists, P to
  pause), the date (a day every 50 turns), the opening countdown, the
  per-turn random draws, and objects' animations;
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
- a path finding tool in the tools window (F12): planned routes drawn over
  the view, click a person to select them and a cell to send them there,
  or add a test walker.

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
  the scanner, ultrascan, blood machine and X-ray (these four need
  research, not there yet, to be built in the game) and psychiatry (which
  also cures), the ward's nurse putting patients to bed, the fees, the
  machines' wear and the handymen's repairs; emergencies offered (not
  taken on);
- the pharmacy (drugs, and the cabinet for invisibility and transparency)
  and inflation: patients cured and going home cured, or not; patients
  dying, floating away or taken by the grim reaper into the gates of hell.
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
  them, a right click moves on to the next one.

Not yet: the other treatment rooms (all but the fracture clinic, which
level 8 lets you build, need research), the operating theatre, research,
the year's end awards and trophies, earthquakes and editing rooms.
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

    nix develop path:./nix#enhanced
    cmake -S enhanced -B build/enhanced -G Ninja
    cmake --build build/enhanced
    build/enhanced/th_enhanced

Tests: `ctest --test-dir build/enhanced`. The tests that need the game data
read it from the development folder and are skipped without it.

Without a window, `--screenshot OUT.bmp --data FOLDER [--level N] [--size WxH]`
renders a level's start view to a BMP.

The tools window (F12) has a debug report: Save report
writes `debug-reports/<time>/` under the folder the game was started from,
with `screenshot.png` (the window without the tools), `state.txt` (the
screen, the last command, money, finance and the hospitals, building and
placing, every room, every person and object with all their fields, the map
around the mouse, and the turn in the replay trace format; `dump_game`,
core/world/debug_dump.h), `note.txt` (the text typed in the tool),
`snapshot.bin` (the whole game state, core/world/snapshot.h) and
`commands.txt` (every command and other input since the level started,
`Game::command_log`), for review.

To reproduce a report (with a build of the same snapshot id: a hash of the
snapshotted structures' headers and code, ignoring comments; another build
refuses the snapshot, and the byte lines of commands.txt):

    th_replay --load debug-reports/X/snapshot.bin --turns +N \
        --dump dump.txt --focus CELL --render view.png --camera X,Y

runs N turns on from the moment it was saved, then writes `dump_game` of the
end state (the map around CELL) and the hospital view as the game draws it
(`--camera` from state.txt's camera line, `--size WxH`; a `.ppm` name writes
a PPM). `--script FILE` adds commands, its turns counted from the
snapshot's. `th_replay --script debug-reports/X/commands.txt --turns T`
plays the level again from its start up to turn T instead.

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
The player's commands checked so far are in `tools/replay/scripts/`
(`reception.txt`: a reception desk and receptionist; `gp_office.txt`: a
GP's office and its doctor too; `pick_up_staff.txt`: the GP's office with
the receptionist and the doctor picked up, re-placed and dropped;
`gp_prices.txt`: the GP's office with general practice's price moved up
and down in the casebook; `sack_staff.txt`: the GP's office with its doctor
sacked during a consultation; `general_diag.txt`, `psychiatry.txt` and
`ward.txt`: the GP's office and a diagnosis room with its doctor or nurse;
`cardiogram.txt`, on level 6, where the cardiogram can be built: its own
GP's office, the cardiogram room, its doctor and a handyman to repair it;
`th_replay --level 6` and orig_trace.sh's LEVEL 6 for that one;
`treatment.txt`: general_diag.txt with a pharmacy and its nurse, an
inflation room and its doctor, and a policy guessing cures early, so that
patients are cured and die; `info_windows.txt`: treatment.txt with a
handyman whose priorities are changed and patients sent home and made to
guess a cure from their windows; `staff_room.txt`: ward.txt with a staff
room, a handyman and a second receptionist, the policy's "rest at" low
and then raised, staff picked up and ordered out of their break, and the
receptionist ordered to rest; `staff_room_full.txt`: treatment.txt with a
staff room holding a sofa alone and many tired staff at once;
`staff_room_none.txt`: tired and ordered rests with no staff room;
`toilets.txt`: patients waiting at the reception of a GP's office with no
doctor, in a warm corridor with a drinks machine, using toilets and a sink,
one sent home on the toilet; `toilets_busy.txt`: the same with one toilet;
`toilets_none.txt`: no toilets, patients wetting themselves), compared
with SKIP `computer_action,manage_rats,generate_disaster,generate_mayors`:

    enhanced/tools/replay/orig_trace.sh orig.txt 18000 \
        enhanced/tools/replay/scripts/gp_office.txt \
        computer_action,manage_rats,generate_disaster,generate_mayors
    th_replay --script enhanced/tools/replay/scripts/gp_office.txt \
        --turns 18000 --out enhanced.txt
    enhanced/tools/replay/compare.py orig.txt enhanced.txt


## Layout

    src/core/data/    file formats: RNC, sprites, palette, levels, the data folder
    src/core/render/  indexed surface, sprites, frames, shadows, the hospital view, panel
    src/core/world/   the map and its things, level start, the turn, arrivals, action
                      sequences, path finding
    src/app/          SDL3 window and input, data-folder search, settings, ImGui tools
    tests/            ctest tests
    tools/replay/     the replay check against the original
