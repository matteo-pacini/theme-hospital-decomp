# Theme Hospital Enhanced

A new engine for Theme Hospital, written in C++20 with SDL3 and Dear ImGui,
for Windows, macOS and Linux. It reads the data of your own copy of the game
and draws it at any resolution and aspect ratio. The decompilation in `../src`
is its reference: behaviour is ported from it function by function.

It contains no game files. At start-up it looks for the game data and, if it
finds none, says what it needs and lets you pick the folder.

## Version and changelog

The version is the `VERSION` of `project()` in `CMakeLists.txt`, the one
source: CMake generates `th_version.h` from it, which the about screen, the
debug report's `version:` line and the browser page (`web/shell.html`, through
the configured copy in the build folder) show. `CHANGELOG.md` is embedded the
same way (`th_changelog.h`) and shown by the about screen; edit it when the
version changes. Its entries are `## vX.Y.Z` headings with `- ` bullets.

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
  draws in order; a rival hospital's patient is made, treated at once by
  its computer player (process_computer_peep, with the game's has_drinks
  quirk) and dropped;
- the computer players (the three rival hospitals): their script of
  decisions (computer_action, computer_decision, the notes, hiring, building
  and buying objects, the recheck of hires), their monthly and yearly
  figures (buckets, wages, value, score, the year-end charts and awards with
  their real figures), all with the game's draws in order. The replay check
  compares them through the events `computer_action`, `computer_decision`,
  `rival_open`, `computer_employ`, `computer_build`, `computer_place_object`,
  `computer_loan`, `computer_land`, `computer_note`, `computer_peep`,
  `computer_bucket`, `computer_month` and `computer_year` (trace.h); the
  `rival_*` scripts exercise them;
- the patients' action sequences (DATA/P_COMP.LEX and the step table): with
  no reception desk, a patient goes home through an entrance, as in the game;
- the game's path finding (wall following, its four-waypoint plans and its
  limits), walking, and the sliding entrance doors;
- a path finding tool in the tools window (F12): people's planned routes
  drawn over the view.

Milestone 4 so far, the game's interface:

- the start-up flow: the intro, the title picture, an about screen, main
  menu, difficulty and tutorial choice, and the level's area movie (Smacker
  movies with their sound); `--level N` skips it and starts level N at once.
  The about screen takes the place of the original's name screen: after two
  seconds of the bare title picture (a key or click brings it at once) a glass
  box shows the credits and the version, with CHANGELOG and CONTINUE buttons
  (Enter or Space continue, C opens the changelog, Escape does nothing). The
  changelog, the text of `CHANGELOG.md` embedded at build time, opens in a
  second box over the first and scrolls by whole lines (the wheel, the arrow
  buttons and keys, Page Up and Down); Escape or CLOSE closes it. There is no
  name entry: the player is always called as the language's default (HORZA in
  English) for the hospital, the saves, the `.hdt` file and the hall of fame;
- the special levels: the training level (5: three consultants placed by
  `training_builder`) and the two rat bonus levels (13 and 14, which `--level`
  also starts: litter, rat holes and handymen from `rat_fest_builder`, the
  rat panel, shooting with the left button, the level's end); a win accepted
  after a year whose rat shots beat the awards' targets (`try_bonus_level`)
  plays one before the next level, and its end goes back to the level it
  interrupted;
- the pull-down menu bar and the requester windows, with the original art;
  quitting asks first; Options speed, sound and volume, autosave (slot 8 on
  day 28), Display shadows. The player's options, jukebox playlist and the
  advisor's one-off advice are kept in `<name>.hdt` beside the saves, in the
  original's 0x5c-byte layout, written after each save and read at start;
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
  answers carried out; the keypad (its numbers dial through; 2 4 3 2 8
  switches the cheats on and off and 7 2 8 7 forces the next bonus rat
  level), and with the cheats on Ctrl+C and Ctrl+Shift+C make objects
  available, Shift+C adds 1000 cash, Shift+R makes a rat, Ctrl+Alt+D lets
  the view scroll over the whole map and Ctrl+Y and Ctrl+M end the year and
  the month (Shift+1 sends a command the game carries out as nothing);
- the statistics pass of every 32nd turn (the patients' and staff's
  happiness, thirst, warmth, bladders and waits, the idle staff, the
  machines' wear, this month's visitors) and the advisor's advice from it:
  thirst, warmth, toilets, unhappy patients and those gone home fed up,
  long waits, too few or too many doctors and nurses, worn machines, litter,
  plants, benches, the first death and cure, deaths near the level's limit,
  the rooms open, a room just opened (and what its illness still lacks), a
  treatment not paid for, a doctor cracked up; the staff promoted, new staff
  put down where they are not needed, the earthquake and its strength, how
  close the level is to being won or lost, and the mayor's visit; Shift+A
  switches the advisor off and on;
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
  hair restorer, fracture clinic and electrolysis cures (hairyitis patients
  treated, waiting for a doctor, and left with no room); the jelly vat and
  decontamination rooms work as the slicer's but are not checked against
  the game yet;
- autopsies: a patient with no treatment sent to the research room (the fax's
  button, EV_PATIENT_TO_RESEARCH), its queue, the researcher calling it, the
  autopsy giving its illness research points and, by the staff's mood, costing
  reputation with the advisor's remark, the autopsy gone with the last
  machine;
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
- the tutorial: the advisor's steps from furnishing the corridor to the
  doctor placed, which ends it, with the flashing of the panel icon or
  requester button each step points at;
- the year end: the hall of fame, the charts, the awards and trophies and
  the year's score; winning or losing at the year end, and a win offered at
  a quarter's end and declined; accepting the win of level 12 ends the
  game with the WIN_GAME movie, the scrolling credits and the main menu;
- saving and loading: File > Save and Load, the quick save and load (Alt+S,
  Alt+L), the Continue slot written on quitting, and the load game screen;
- the game's other single-player keys: Alt+F1-F8 store and Ctrl+F1-F8 recall
  view positions, W held runs turns faster (Ctrl+W does not), Shift+Ctrl+Esc
  or Shift+Esc with Tab held quits at once, the numpad 8/2/4/6 work in the
  casebook and the staff list, and C on the briefing picks any level by a
  click on its area.
- the interface's cosmetics: the palette fades between screens (the old
  screen to black in 10 steps, or 5 on leaving a screen for the hospital, the
  new one up in 10; a movie sets its own palette), the loading screen at the
  start and around a level's start, the game standing still and its input
  kept for after a fade; Alt+C saves the screen as scrshots/hospNNNN.png
  beside the saves (downloaded in the browser), and J opens the jukebox over a
  chart too (it shows when the chart is left, and Escape closes it with the
  chart.

Not yet: the DNA fixer.
Route planning defaults to A*; the game's wall following stays selectable
in the tools window.

## Game data and the user's folders

The data folder is the one holding `DATA`, `LEVELS` and `QDATA` (file names
are matched case-insensitively). Where it is looked for:

- The data comes from the first of these that holds every required file:
  `--data <folder>`; the development folder, `../orig` (the repository's
  `orig/`, where `nix run path:./nix#gog-unpack` puts the GOG release;
  builds for others turn it off with `-DTH_DEV_DATA_DIR=`); the game folder
  below, only when it holds `install.ok` (which an install leaves). If none
  is complete, the setup screen installs it: from GOG's installer (any file name; size and sha256 are checked) or
  from a game folder, chosen with the buttons or dropped on the window, on a
  worker thread with a progress bar and Cancel. "Verify files" re-hashes an
  existing install. `--install-from <installer or folder>` does the same
  before the window opens (progress on stderr, exit 1 with the error if it
  fails).
- `--debug` turns the F12 tools on (off otherwise; F10 reports work in every
  mode). `--drive` and `--control` imply it. `nix run path:./nix#enhanced`
  passes it.
- `--user-data` skips the development folder, so a development build looks
  only in the game folder as a normal start does and can show the setup
  screen; `--install-from` implies it. `--drop PATH` hands PATH to the setup screen as a file dropped on the window at start.
  A driven run waits, its commands paused, while an install runs.

The per-user folders (`ThemeHospitalEnhanced`, made on first use; src/app/
paths.cpp):

| | settings.ini | saves, hiscore.dat | game data | debug reports |
|---|---|---|---|---|
| Linux | `$XDG_CONFIG_HOME` (`~/.config`) | `$XDG_DATA_HOME` (`~/.local/share`) `/save` | `$XDG_DATA_HOME/.../game` | `$XDG_STATE_HOME` (`~/.local/state`) `/debug-reports` |
| macOS | `~/Library/Application Support/` | `.../save` | `.../game` | `.../debug-reports` |
| Windows | `%APPDATA%` | `%USERPROFILE%\Saved Games` | `%LOCALAPPDATA%\...\game` | `%LOCALAPPDATA%\...\debug-reports` |

An unset or relative XDG variable falls back to its default under `$HOME`.
On the first start settings.ini and `save/` are copied from the old folder
(`SDL_GetPrefPath("ThemeHospitalDecomp", "Enhanced")`), and hiscore.dat from
`save/` of the data folder, when missing in the new ones; nothing is
overwritten or deleted. The old `data_dir` setting is ignored.

## Upscaling

The setup screen (an install from the installer or a folder) offers to
upscale the graphics: a checkbox, the algorithm (a table: xBRZ only, in
src/core/render/upscale/scaler.cpp) and the factor, 2x to 6x. The choice is
the `upscale` setting (`off` or `xbrz:4`); `--upscale off|xbrz:N` overrides
it for a run and preselects the setup screen's choice. Nothing in the
simulation changes: replays and snapshots are not involved.

- **What is upscaled.** The map's sheets (blocks, things and people, the money
  bag) are scaled once at install, in the install thread after the files are
  in place (about 1 s at any factor, plus writing), then snapped back to the
  game's 256 colours so that ghosting, the wall mask, the pause shade and the
  palette fades work on indices as before. The result is in the user's
  `upscale` folder (`$XDG_DATA_HOME/ThemeHospitalEnhanced/upscale`, the
  browser's `/persist/upscale`) as `<algorithm>-<factor>.thup`, one file,
  deflated, valid for the data it was made from (a hash of the sheets and
  the palette), the algorithm's version and the factor. Missing or stale, the
  game builds it at start (and writes it); a factor that cannot be had in
  memory (a probe allocation fails) gives the next lower one. About 40 MB of
  indices at 4x and a 7 MB file; the shaded blocks the game derives at
  load come on top.
- **How it is drawn.** The map surface holds `factor` x `factor` indices per
  game pixel (`Surface::scale`); a sprite with an upscaled version copies
  its runs, any other (fonts, the wobbling buffer) replicates its pixels, so
  every drawing function takes game coordinates as before and clicks land on
  the same cells. The window shows that surface at the view's zoom (linear
  filter unless the ratio is whole).
- **What is not.** The interface layers (panel, requesters, menu bar),
  full-screen pictures (when first shown) and the pointers are scaled at
  present, through the same scaler, in RGBA; the layer again only when its
  pixels change. Movies are not scaled.
- **Full-screen pictures** are drawn at a whole-number scale when the window
  is at least 640 x 480, upscaling or not.
- **Check.** `# Compare nearest:2` (tests/drive/upscale_nearest_matches_zoom.txt)
  keeps the machinery honest: the nearest scaler through the Nx surface must
  give the 1x frame zoomed, byte for byte. `th_tests` has the scaler, snapper
  and cache tests; the install tests have a cache build.
- **Licence.** xBRZ is GPL-3.0-only (third_party/xbrz); a binary that contains
  it is conveyed under the GPL version 3, and the web page links the source.

## Building

From the repository root, with Nix:

    nix run path:./nix#enhanced            # build (Release) and run
    nix run path:./nix#enhanced -- --level 5

or by hand (CMake 3.24+, a C++20 compiler; SDL3 is used from the system if
found, otherwise fetched, and Dear ImGui is always fetched):

    env -u LD_LIBRARY_PATH nix develop path:./nix#enhanced
    unset LD_LIBRARY_PATH
    cmake -S enhanced -B build/enhanced -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build/enhanced
    build/enhanced/th_enhanced

`LD_LIBRARY_PATH` is unset outside the shell and again in it: a host's,
pointing at libraries built against a newer glibc, breaks linking.

Tests: `ctest --test-dir build/enhanced`. The tests that need the game data
read it from the development folder and are skipped without it.

The gate is one command from a plain terminal (it enters the dev shells,
unsets `LD_LIBRARY_PATH` and runs at the lowest priority itself):

    enhanced/tools/check.sh [SCRIPT ...]

It builds `build/enhanced` (configuring it if missing; a warning from our
own targets fails), runs `th_tests`, `tools/tidy.sh`, `clang-format
--dry-run` over the sources, `tools/drive_tests.sh`, `tools/install_tests.sh`
and `tools/replay/replay_all.sh` (over the scripts named, else all), prints
one line per step and stops at the first failure. Before the replays it
prints how many scripts have no cached original and stops when there are
more than `FRESH_MAX` (default 10; not with `CACHE=refresh`), saying to fill
the cache first (`trace_cache.sh prefetch`); a script with a stored port
verdict needs no original and is not counted. The replays are cached on the
port's side too (`PORT_CACHE`, "Port verdict cache" below), so a gate on an
unchanged `src/core` repeats no replay. `CACHE` goes on to
`replay_all.sh` (below). The job caps default to those for a machine shared
with other agents (`CMAKE_BUILD_PARALLEL_LEVEL=2`, `TIDY_JOBS=2`, `JOBS=1`);
set them higher when the machine is yours. Its logs are in
`build/check-<pid>/`, removed when the gate passes.

Without a window, `--screenshot OUT.bmp --data FOLDER [--level N] [--size WxH]`
renders a level's start view to a BMP.

`--drive SCRIPT --out DIR` plays the whole program headless from a script
of clicks, keys and waits, with screenshots, state dumps and expectations,
on a driven clock so that a run repeats exactly; `--control` takes the same
commands on stdin, answering each on stdout (src/app/drive.h).
`--visible` shows the window while it runs. A driven run starts a player
without a details file at Normal speed (a normal start is at Max, as the
original's default); `--default-speed` keeps the Max. `--real-time` gives a driven
run the real clock and frame lengths of a normal start (`wait` then counts
frames that take milliseconds), to look for what only a real clock shows.
A normal start fades between screens and shows the loading screen as the
game does (the game stands still meanwhile, its input kept for after);
a driven run changes screens at once unless it is given `--fades`. The
scripts in tests/drive are
interface tests: `tools/drive_tests.sh` runs them (all, or the ones named)
from the repository root with the build in `DRIVE_BUILD` (default
build/enhanced), its output in build/drive-tests/<pid>/ (a folder per
run, removed when every script passes). A script's header picks
how it starts: `# Level N` the level (default 1), `# Fades` passes `--fades`,
`# NoLevel` the start-up
screens instead, `# Size WxH` the window size (default 1280x720; the menu
bar is only at the top in the game's 640x480 layout), `# SeedScript NAME`
runs the script NAME first, into a folder of its own, and copies its saves
into the run's save folder (the run order does not matter),
`# Upscale SPEC` runs it with `--upscale SPEC`, `# Compare SPEC FILE...` also
runs it with `--upscale SPEC` and fails unless each named output file is
byte for byte the 1x run's (`nearest:N` at the zoom N matches the 1x
frame), and
`# SeedReplay LEVEL SCRIPT TURNS` plays a replay script to that turn with
th_replay and puts its end state in slot 1. `type TEXT` types into a focused
text field, `key [MOD+]NAME` presses a key with Shift, Ctrl or Alt held
(`keydown` and `keyup` hold one, as for W and Tab; spaces in a name are
written `_`: `Keypad_8`), and `screenshot-ui` captures after the ImGui
layer (the tools and the quick report's note box). `wheel N` turns the
mouse wheel N notches (negative toward the user).

The tools window (F12) has a debug report: Save report
writes `debug-reports/<time>/` (under the folder the game was started from when the data came from the
development folder; otherwise in the user's reports folder, above;
`TH_DEBUG_REPORT_DIR` overrides both) with `screenshot.png` (the window
without the tools), `state.txt` (the screen, the last command, money, finance and the hospitals, building and
placing, every room, every person and object with all their fields, the map
around the mouse, and the turn in the replay trace format; `dump_game`,
core/world/debug_dump.h), `system.txt` (what it ran on: the version, commit,
compiler and SDL, the operating system, processor, memory, displays, audio
and video drivers, the renderer and GPU, upscaling, language and sound
settings; no host or user name; `sysinfo.cpp`, `app_sysinfo.cpp`),
`note.txt` (the text typed in the tool),
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
snapshot's. `--snapshot FILE` writes the end state as a snapshot, to
`--load` again. `th_replay --script debug-reports/X/commands.txt --turns T`
plays the level again from its start up to turn T instead. `--save SAVE`
writes the end state as a saved game that the game's Load reads (its
slots are `<player name>.h<slot>` in the save folder; a save checks the
layout hash, not the snapshot id, so ordinary source edits keep it
loadable). `--astar` plans routes with A* rather than the game's wall
following, from the level's start (a snapshot or command log keeps its own
choice); the replay check needs wall following, the default.

With `--advisor`, th_replay also runs the advisor in each turn, as the
game's frame does (not while a fax is up: its frame has no advisor), and
notes each advice given as a comment line
(`# advice <situation>`), which the instrumented original writes too,
followed by the seed. compare.py ignores them; `replay_all.sh` runs the
port with `--advisor` and compares them with the original's (the seed left
off), a difference failing the script.

Replay check against the original: `tools/replay/orig_trace.sh` relinks an
instrumented HOSPITAL.EXE from the decompilation and runs it in DOSBox-X
without a window, writing a trace of every turn (the random seed, date,
balance, hospitals and things) with the turn's events in order (a patient
made, a complex given, a queue joined, money spent, ...; the list is in
core/world/trace.h); `th_replay` writes the same trace from this engine, and
`tools/replay/compare.py` reports the first turn they differ: the first
differing event, with those round it, before the state. `TRACE_RAND=1
orig_trace.sh ...` and `th_replay --rand` also trace every draw from the
random seed with the function that drew it. The instrumented original leaves
out `draw_main_window` and the frame loop's screen copy (they only put the
view on the screen, and no turn reads it), about a quarter of its run time,
unless `TRACE_DRAW=1`. Whoever ports a function the
game writes an event for, or adds an event, writes it on both sides, at the
same point: in the port (`trace_event`) and in `tools/replay/instrument.py`'s
EVENTS; a port named otherwise than the game's function names it in its
draws (`old_newrand(seed, "name")`). See the scripts for their arguments.
`tools/replay/replay_all.sh` runs every script (or the ones named) through
both sides, checks that each agrees and satisfies the `# Expect` lines in its
header (`# Expect <text> [<op> <n>]`: an event line whose text after `E `
starts with `<text>` occurs `<op> <n>` times, `op` one of `>= == <=`, default
`>= 1`; `# Expect-not <text>`: no such event; the advisor's `# advice N` lines
count as events `advice 0xNN`, NN the situation in two hex digits), and prints
one line per script and a summary; `check_expect.py` does the checking on both
traces. It runs `JOBS` scripts at once (default `TH_REPLAY_SLOTS`, else 10)
with the th_replay of `BUILD` (default build/enhanced). Its settings are `BUILD`,
`JOBS`, `TURNS` (the last turn of a script without `# Turns`), `RAND`,
`TRACE_DRAW`, `CACHE` and `PORT_CACHE` (below), `KEEP` (keep every trace; otherwise a
failing script's two traces are kept, zstd-compressed), `WATCHDOG` and
`REPLAY_BASE`; they are listed at the top of the script. `tools/replay/expect_candidates.py SCRIPT TRACE` lists,
for each of a script's `# Expect` lines, the trace's events of the same
shape (the same text, any numbers) with their numbers, to re-derive an
Expect line whose numbers drift with the generator.

Original trace cache. `build/replay-cache/traces/` holds the original
side of past runs, so `replay_all.sh` runs DOSBox only for a script it
has no entry for. Each entry is one script's trace, compressed with zstd
and named by a key of everything that fixes the trace: the instrumented executable's cache key (a hash of the
decompilation's sources, objects and tools and of instrument.py as a parsed
syntax tree without docstrings, so editing its comments, docstrings or layout
keeps the key while any change to its code, EVENTS tables or the C++ they
insert changes it), the script file's bytes
(not its path), the last turn, the level, the left-out calls (none under
`replay_all.sh`), RAND, `TRACE_DRAW`,
`orig_trace.sh` read without its comment-only and blank lines (everything
else counts: the DOSBox configuration, command line, run folder setup and the text of its
messages; a line after a backslash continuation, a multi-line string or an
unrecognised heredoc form is kept as written) and the original's `HOSPITAL.CFG`,
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
new or edited script, other turns, level, an edited `orig_trace.sh`
or CFG, a decompilation or `instrument.py` change) misses only the affected
entries and refreshes them. `CACHE` picks how `replay_all.sh` uses it:
`use` (the default) hits the cache and runs DOSBox for a miss; `only` never
starts DOSBox and takes no lock: a script without an entry is listed as "no
cache", neither passed nor failed, and the run exits 3 (1 if anything
disagreed), so it never reads as a full pass; `refresh` runs every original
fresh and replaces its entry, only when the cache is suspected stale, not to
check a port change, since a cached original trace is the reference while the
executable key is unchanged. After a decompilation or `instrument.py` change
every key is new, so nothing is cached and fresh originals are needed. The
full run with fresh originals is the lead's at integration. A key edit that
could change a trace must be kept in the key; when unsure, keep it, since a
false hit is worse than a miss. `enhanced/tools/replay/test_cache.sh` tests
the cache with a fake DOSBox. `trace_cache.sh missing [SCRIPT ...]` lists
the scripts without an entry at the `replay_all.sh` defaults. To fill them
in one go, run `enhanced/tools/replay/trace_cache.sh prefetch [SCRIPT ...]`
(from any directory): it runs `replay_all.sh --orig-only` over the missing
ones, so it fills exactly what `replay_all.sh` hits, with its `JOBS` and
watchdog. Entries with RAND on are about 0.5 MB each on average (up to
1.5 MB), so the whole set is some 50-100 MB per executable key until `prune`
drops old keys; at about a minute per script on the DOSBox slots, it is best
left to run unattended. `trace_cache.sh status` prints the entry count, size,
current key and stale count; `trace_cache.sh prune` removes entries whose
executable key is no longer current, and `prune DAYS` also removes entries
older than DAYS days. Never edit or hand-place cache files.

Port verdict cache. The port's side of a replay is cached too:
`build/replay-cache/port/` holds, for each script that passed, a small
record (`<kind>.<script>.<tag>.<key>.rec`: the key, the script's result line
as printed, the `# Expect` counts of the port's trace and the hash of that
trace). Only passes are stored; a failure stores nothing and keeps its
traces as before. The key is a hash of everything that decides the verdict,
so any change is a miss and nothing is ever invalidated by hand:

- the sources `th_replay` is built from (all of `src/core/`,
  `tools/replay/main.cpp`, `CMakeLists.txt`, `cmake/`, and `nix/flake.lock`
  and `nix/flake.nix`, which pin the compiler and the dependencies), by
  their bytes, so the same source in two worktrees is the same key;
- the build folder's configuration (type, flags, compilers, linker flags and
  our own options from its `CMakeCache.txt`, the data folder's path left
  out: the game data is the installed game, as for the original's key);
- the checking tools: `compare.py`, `check_expect.py`, `replay_all.sh` and
  `port_cache.sh` (the last two without comment-only and blank lines);
- the original's trace key for the script (script bytes, turns, level,
  RAND, `TRACE_DRAW`, the instrumented executable and the DOSBox run), so a
  decompilation or `instrument.py` change misses it too.

The wasm results (`web_replays` and `web_cross_load`) are separate records
with the web key: the native key and `web/`, `check.sh` (without
comment-only lines) and the wasm build folder's configuration; Emscripten
and node come from `flake.lock`. A wasm record counts only with the native
record for the same script and key, and with the same trace hash. A
cross-load record is also keyed by the turn. All-or-nothing: no script's
verdict depends on which source file it reaches, so any change under
`src/core` reruns all 158 (8 minutes native, 4 wasm with the native hashes known, at two jobs); a
change to `src/app`, `tools/` outside the list above or the docs reruns
nothing.

`replay_all.sh` and the web phase read a record before running a script
and print its stored line, so the gate reads the same; the summaries count
them: `158 passed (150 cached, 8 run), 0 failed`. A script with a native
record needs no original at all (`trace_cache.sh missing` and the gate's
fresh count skip it). `PORT_CACHE=use` (default) reads records;
`PORT_CACHE=off` runs the port side again (both native and wasm; the
original side still comes from its cache) and a pass rewrites its record;
`CACHE=refresh` also runs both sides and rewrites. A record is stored only
when `th_replay` (`th_replay.js` for wasm) is newer than the files it is
built from, so a stale binary never writes a verdict for newer sources
(`replay_all.sh` says so on standard error); the check is the files'
modification times, so a source restored with an old time is not seen. A
false hit is a key bug: add the missing input to `port_cache.sh`. The web
phase cannot compute the native key (no python3 or DOSBox in its shell), so
the gate's replay phase writes the scripts' native keys
(`trace_cache.sh native-keys`) for it to read.
`trace_cache.sh status` and `prune` cover the records (stale means the
key is not the script's current one, or the script is gone; a record whose
build folder is missing here is counted unknown and kept). The tests are in
`test_cache.sh` too.

The player's commands checked so far are the scripts in
`tools/replay/scripts/`, lines of `turn type value arg` (th_replay --script
and orig_trace.sh's SCRIPT; the EV_* numbers in include/enums.h). Each
script's header comments say what it exercises, and its `# Expect` and
`# Expect-not` lines check it on both traces; `# Level N` picks the level
(th_replay `--level N`, orig_trace.sh's LEVEL), `# Balance ID` the
balance files' prefix (EASY, HARD or NET; the game's `-bID`, default FULL: the
level is then passed to both sides as `N -bID`), `# Turns N` the last
turn and `# Score N` the score the last level left (init_player keeps it
above level 1; both sides read the header line itself). `@advisor_event N`
lines raise advisor.event_flags' bits as the interface's buttons do. The
cheats' lines (`@cheats 0|1`, `@force_bonus 0|1`, `@cheat FLAGS`,
`@cheat_rat`, `@cheat_scroll`) are th_replay's only: the instrumented
original has no stand-in for them, so no script uses them yet. A script
is compared with no call left out (the computer hospitals, rats and
disasters all run) by `replay_all.sh`, or by hand as below; th_replay's
`--skip NAMES` and orig_trace.sh's SKIP leave calls out of one side
(`--skip computer_action` runs without the computer hospitals):

    enhanced/tools/replay/orig_trace.sh orig.txt 18000 \
        enhanced/tools/replay/scripts/gp_office.txt
    build/enhanced/th_replay \
        --script enhanced/tools/replay/scripts/gp_office.txt \
        --turns 18000 --out enhanced.txt
    enhanced/tools/replay/compare.py orig.txt enhanced.txt

`compare.py --ignore-event KIND` leaves an event kind out of a comparison on
both sides, for events the original writes and the engine does not emit yet
(none today: the engine emits every event).

What the scripts leave unexercised: `tools/coverage.sh` (in the dev shell, GCC)
builds th_replay with `-DTH_COVERAGE=ON` in build/coverage, runs every script
there and writes an HTML report and `summary.txt` to build/coverage/report/.

## Web build

The engine also builds to WebAssembly with Emscripten, in its own dev shell
(Emscripten, node, clang-tidy, zstd; the system libraries it builds are
cached in `build/em-cache`):

    env -u LD_LIBRARY_PATH nix develop path:./nix#web
    unset LD_LIBRARY_PATH
    emcmake cmake -S enhanced -B build/web -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build/web

Every object, the fetched dependencies included, is built with `-pthread`
(the install runs on a `std::thread`), with memory growth allowed. SDL3 is
always fetched (there is none to find). `th_tests` and `th_replay` link for
node with `-sNODERAWFS=1` and an 8 MB stack, so they read the game data and
the scripts from the host, and run as `node build/web/th_replay.js ARGS`
with the native one's arguments. Node's directory listing does not follow a
link given as the folder, so pass the data folder by its real path
(`--data "$(readlink -f orig)"`, `TH_TEST_DATA_DIR="$(readlink -f orig)"`)
where `orig` is a link, as it is in a worktree.

`WEB=1 enhanced/tools/check.sh` adds a phase after the replays, in the web
shell, stopping at the first failure:

1. `web_build`: configures `build/web` when missing and builds it; a
   `warning:` outside `_deps` fails.
2. `web_th_tests`: `th_tests` under node, with the game data.
3. `web_tidy`: `tidy.sh` (the same checks) over the `.cpp` files with a
   region under `__EMSCRIPTEN__`, using `build/web`'s compile database, the
   `wasm32-unknown-emscripten` target and Emscripten's sysroot (`TIDY_EXTRA`),
   so those regions are the ones analysed. A compiler `error:` fails it too.
4. `web_replays`: every script (or the ones named) through native and wasm
   `th_replay` with `replay_all.sh`'s arguments (`--script`, `--turns`,
   `--level`, `--rand`, `--advisor`), `JOBS` at a time. The two traces must
   be byte-identical (`cmp`); equal ones are deleted, different ones kept
   zstd-compressed in `build/web-replay/<script>/`. The original is not
   involved: the native port already agrees with it. A script with a stored
   wasm pass for the current key (and the native pass for it) is not run;
   with a native pass stored, only the wasm one runs and its trace hash is
   compared with the native trace's ("Port verdict cache" above).
5. `web_cross_load`: one build writes a snapshot at turn N
   (`th_replay --snapshot FILE`, at the end of the run), the other `--load`s
   it with the script's remaining commands (turns counted from N) and must
   continue to the trace of the first build's whole run from turn N + 1;
   native to wasm and wasm to native. `CROSS_LOAD` lists the `SCRIPT:N` items
   (default `rival_two_years:18000`, with rivals and a year end after it,
   `win_level1:9000` and `tutorial:9000`). The `# advice` lines are compared
   too: the snapshot carries the presentation generator (`Game::safeseed`)
   the advisor draws from. Each item's pass is stored too, keyed by the script
   and N.

Without `WEB=1` the gate does not touch `build/web`. At one job
(`CMAKE_BUILD_PARALLEL_LEVEL=1 TIDY_JOBS=1 JOBS=1`, nice 19) a clean
`build/web` takes about 3 minutes (the `th_enhanced` part, SDL3 and ImGui, is
half of it), `web_th_tests` 18 s (native 11 s), `web_replays` 11 minutes for
the 158 scripts (a script takes two to three times as long as natively) and
`web_cross_load` 20 s.

### Deploy

`tools/web_smoke.sh` runs the build in headless Chromium: it installs from
`orig/` through the picker, reloads, starts level 1 and downloads a debug
report. Given a deploy folder (below) it also checks the headers of
`web/vercel.json` as the page received them. It runs with
`?upscale=xbrz:4` (`SMOKE_UPSCALE=off` for none) and waits for the cache.

`tools/web_deploy.sh` deploys the build to Vercel as static files, with the
Vercel CLI (`vercel@63.1.2` through `npx`; log in once with `npx vercel
login`). It brings `build/web` up to date and assembles `build/web/deploy`:
`index.html`, `index.<hash>.js` and `index.<hash>.wasm` (the page finds the
wasm through `Module.locateFile`) and `web/vercel.json`, which sends the
COOP/COEP headers the page needs for threads on every path and caches the
hashed files for good and the page not at all. The folder holds no game data:
the player picks their installer or folder. It goes to production
(`vercel deploy --prod`). `TH_VERCEL_PROJECT` names the Vercel project
(default `theme-hospital-enhanced`). `--dry-run` assembles the folder and
prints the command; nothing is sent.

## CI

`.github/workflows/ci.yml` runs `gate.yml` on every pull request, push to
master and by hand. The jobs (check names `gate / Build`, `gate / Web build`,
`gate / Lint N`, `gate / Tests N`, `gate / Replays N`, and `gate / gate`,
which needs all of them and is the one the master ruleset requires):

- Build: `check.sh` with `STEPS="build format" NO_DATA=1`; the native
  binaries (th_replay, th_enhanced, th_tests) go to the tests and replay
  shards as an artifact, so no other job builds them.
- Web build: `STEPS="web_build web_th_tests web_tidy" NO_DATA=1`; the
  WebAssembly binaries go to the replay shards as an artifact.
- Lint 0..2: build and tidy over a third of the files each (`TIDY_SHARD=I/3`:
  the sorted list, every third file).
- Tests 0..2: the drive tests over a third of the scripts each
  (`DRIVE_SHARD=I/3`) with the game data and Build's binaries; shard 0 also
  runs th_tests and the install tests.
- Replays: the scripts split into shards (longest first); each shard restores
  the trace cache and the port verdicts, runs `STEPS="replays web_replays"`
  and uploads its new original traces and verdicts; shard 0 also runs the
  cross-load test.
- Save the trace cache (master only): the traces entry (about 170 MiB) is
  saved only when a shard recorded a new original trace; the port verdicts
  (a few KiB) are a separate entry, saved when a shard produced a new one. A
  run with neither saves nothing; the job's log says which.
- Pull requests from forks have no secrets and no data: only Build, Web
  build and Lint run, with `NO_DATA=1` (th_tests exit 77 passes).

A warm gate takes about 9 minutes, the length of the Web build plus a replay
shard. A gate runs 9 jobs at once (Build, Web build, 3 Lint, 3 Tests, Shards)
and then the 6 replay shards, so two gates fit in GitHub's 20 concurrent
jobs; a third queues.

`check.sh` takes `STEPS` (a subset of its steps; a phase with none is not
entered), `NO_DATA=1`, `TIDY_SHARD` and `DRIVE_SHARD`; see its header.

The game data is a plain tar.zst of ANIMS DATA DATAM INTRO LEVELS QDATA QDATAM
SOUND, HOSPITAL.CFG, MODEM.INI and DOS4GW.EXE in the private R2 bucket
`theme-hospital-enhanced` (key `theme-hospital-data.tar.zst`). Environment
`ci` holds `R2_ENDPOINT`, `R2_BUCKET`, `ATTIC_CACHE_URL`, `ATTIC_CACHE` (variables)
and `R2_ACCESS_KEY_ID`, `R2_SECRET_ACCESS_KEY` (read-only token), `ATTIC_TOKEN`
(secrets). Upload after the game folder changes:

    nix shell nixpkgs#awscli2 nixpkgs#zstd -c enhanced/tools/ci/data_upload.sh

with `AWS_ACCESS_KEY_ID` and `AWS_SECRET_ACCESS_KEY` of a write token in the
shell; it checks the files against the install manifest and prints their
SHA-256. The files never enter caches, artifacts or logs: the original's
traces (numbers only) go in the Actions cache (saved only by pushes to
master; pull requests restore) and in artifacts, with the text logs of a
failure.

The trace key (`replay/exe_cache_key.sh`) hashes the decompilation's `.o`
files, not the OMF `.obj` ones (those hold the build folder and the sources'
timestamps), and sorts in the C locale, so a runner and a local checkout get
the same key. The Nix dev shells come from the project's Attic cache when
`ATTIC_TOKEN` is set (pushes to master and dispatch runs fill it); without it
the replay shell builds open-watcom from source (about 11 minutes).

`enhanced/tools/ci_run.sh` runs the gate remotely from a checkout
(AGENTS.md, "Remote gate").

## Bugs in the original

`init_edit_room`'s `u8` overflow for an empty room is in
[docs/BUGS.md](../docs/BUGS.md); the port uses an `int`, so a script that
opens a room before `commit_doors` has finished cannot agree
(`tools/replay/scripts/win_level1.txt` waits for it).

## Layout

    src/core/data/    file formats: RNC, sprites, palette, levels, text, Smacker,
                      the data folder
    src/core/render/  indexed surface, sprites, frames, shadows, the hospital view, panel
    src/core/render/upscale/  the upscaler: algorithms, palette snapping, the cache
    third_party/xbrz/ xBRZ, vendored (GPL 3; README there)
    src/core/world/   the map and its things, level start, the turn, arrivals, action
                      sequences, path finding
    src/core/audio/   the sound bank's effects and the XMI music
    src/core/ui/      the game's screens and windows: menus, requesters, charts,
                      fax, town map, help, year end, win and lose
    src/app/          SDL3 window and input, data-folder search, settings, driving
    src/app/ui/       ImGui: the tools window, debug report, path finding tool
                      and the setup screen
    src/app/audio/    mixes the sounds and music onto the audio device
    tests/            ctest tests
    tests/drive/      driven interface tests (tools/drive_tests.sh)
    tests/install/    scripts of tools/install_tests.sh (setup screen, install)
    tools/replay/     the replay check against the original
    tools/help/       check_table.py: the help hotspot table against the original
    tools/            drive_tests.sh, install_tests.sh, coverage.sh, tidy.sh
                      (clang-tidy) and slot.sh (shared slots for DOSBox and
                      driven runs)
