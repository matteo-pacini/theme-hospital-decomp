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
- a tools window (F1) with the UI scale; levels are chosen with `--level N`.

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
- a path finding tool in the tools window (F1): planned routes drawn over
  the view, click a person to select them and a cell to send them there,
  or add a test walker.

Not yet: rooms, staff, building, litter, the menu bar and the game's
screens.

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

## Layout

    src/core/data/    file formats: RNC, sprites, palette, levels, the data folder
    src/core/render/  indexed surface, sprites, frames, shadows, the hospital view, panel
    src/core/world/   the map and its things, level start, the turn, arrivals, action
                      sequences, path finding
    src/app/          SDL3 window and input, data-folder search, settings, ImGui tools
    tests/            ctest tests
