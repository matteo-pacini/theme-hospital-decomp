# Bugs in the original code

This is a catalogue of bugs in the 1997 `HOSPITAL.EXE`: in the game, in
Bullfrog's libraries and in the bundled Miles and Smacker code. The
decompilation keeps every one of them. It has to: the C must compile to the
original's bytes, so any fix would break the match. Each entry was checked
against the current source and the original disassembly (`tools/fn.py
0xADDR`). The source matches that disassembly byte for byte, so what the C
does is what the shipped game does.

**Bug** means the code plainly does not do what it was meant to; the entry
gives the effect, which can be "none in practice" when the wrong value is
never used or the code is never reached. **Potential bug** means the code
looks wrong but this could not be settled: it may be deliberate, or whether
it misbehaves depends on data or callers that could not be confirmed. The
entry says which.

Addresses are those of the function in `HOSPITAL.EXE`.

## Game logic

### computer_loan (src/computer.cpp)

**Bug** · `0x4fe78`

The two branches have each other's bodies. The branch taken for a negative
amount checks that the hospital has a loan and at least 5000 cash, which
fits repaying, but then adds 5000 to both the balance and the loan. The
positive branch checks there is room under `max_loan`, which fits borrowing,
then takes 5000 off both. `computer_maintain_loan` (`0x50aa8`) asks for -1
when a computer player has a loan and more than 6000 cash, so a rich AI
hospital borrows another 5000 each time, and nothing caps the loan at
`max_loan`.

### process_computer_peep (src/computer.cpp)

**Bug** · `0x4f15c`

`has_drinks` is set to 1 when the computer player has enough drinks machines
for its patient load, and is never set to 0 otherwise. When there are too
few machines, the simulated patient's drinks purchases (20 each) depend on
whatever was left in that stack slot.

### spread_epidemic (src/epidemic.cpp)

**Bug** · `0xa0040`

When it infects a new victim, it changes the victim's illness but calls
`effect_illness` on the carrier. For an illness that changes a patient's
look, the victim never takes it on, and the carrier gets the change a
second time (bloaty head's, for one, adds 5 to `layers[0]`).

### clear_cell_of_ratholes (src/rats.cpp)

**Bug** · `0x7d2a8`

The loop runs `hole < last` where `last` already points at the final
entry, so the newest rat hole is never checked. A room built over that hole
leaves it in place.

### queue_person_to_new_queue (src/queue.cpp)

**Bug** · `0x4b7e0`

`find_nearest_manned_reception` returns a `u16` thing index, which is
stored in a `u8`. For a desk whose thing index is above 255, a patient moved
to the reception queue walks to the `use_pos` of an unrelated thing, or goes
nowhere if the low byte is 0.

### increase_staff_salaries (src/staff.cpp)

**Bug** · `0x2cb2c`

In the all-staff branch, used when `staff` is not a thing (the caller passes
0), the 90,000 cap is applied to `staff->salary` instead of
`member->salary`. A raise for all staff is never capped, and the cap test
reads, and may write, through the null `staff` pointer.

### find_free_object_anywhere (src/objects.cpp)

**Potential bug** · `0x3800c`

In the room branch, `continue` on an object of the wanted kind that is in
use skips the step to the next object, so the loop never ends and the game
hangs. Both callers look for reception desks. Whether the seeker can ever be
in a room that holds an in-use reception desk was not confirmed.

### free_balance_files (src/name.cpp)

**Bug** · `0x902d0`

It is meant to free the whole ring of balance file entries, but `next` is
only ever 0, so it frees `current_balance_file` and stops. The other entries
leak each time the list is rebuilt.

### load_midi (src/sound.cpp)

**Bug** · `0x89b14`

When `midi/midi.txt` is missing or cannot be read, it clears the sound
effects flags (`statvar.sound_ok`, `myplayer.sfx_on`) instead of the music
ones. Without that file the game runs with sound effects off, while music
stays enabled.

### alter_research (src/rschscrn.cpp)

**Bug** · `0xb2c74`

The research allocation for a category is read into a `u8`, so the
`old >= 0` test meant to protect a closed category (-1) is always true. No
effect in practice: `process_res_button` (`0xb3060`) checks the category is
open before queueing the event.

### cheat (src/utils.cpp)

**Potential bug** · `0x1a238`

Cheat codes typed into the message line are recognised by `cheat_mode`
(`0x86544`), which passes flags 0x20 to 0x4000, but `cheat` acts only on
flags 2, 4, 8 and 0x10. The code is swallowed and nothing happens. This
may be the cheats being disabled on purpose for release.

### drug_improved (src/research.cpp)

**Bug** · `0x6d428`

After raising a drug's effectiveness, one time in seven it lowers the drug's
cost by 1 and then applies `cost > 10 ? cost = 10`, a cap where a floor was
meant (the other branch floors the cost at 10). The drug's cost drops to 10
at once.

### advisor_thirsty_peeps (src/advisor.cpp)

**Bug** · `0x61188`

The "very thirsty" test (`thirst < 10`) comes after `thirst < 0x1e` in an
`else if`, so it is never reached. The advisor's very-thirsty messages
(`advisor_text[5]`, `extra_advice[0x33]`) are never given.

### find_advice (src/advisor.cpp)

**Bug** · `0x5defc`

In the contagion case (0xd), `contagious_count >= 8` is tested after
`>= 4` in an `else if`, so the stronger warning (`extra_advice[2]`) is never
given.

### find_advice, French text (src/advisor.cpp, DATA/LANG-1.DAT)

**Bug** · `0x5defc`

The "junior has become a doctor" advice (case 0x16, announced by
`appraise_staff` in environ.cpp when a junior's ability passes the
threshold) is printed with `sprintf(advisor.text, advisor_text[0x16])`: the
language text is the format and no argument is passed. English's "One of
your JUNIORS has become a DOCTOR." has no conversion, but the French
`LANG-1.DAT` has "Un des médecins est devenu %s.", so `%s` takes whatever
is next on the stack as a string pointer. This is the French version's
well-known crash when a junior is promoted, often first met on level 5;
players reported that the game survived it under real DOS. What the stray
pointer prints there depends on the caller's stack. GOG's `LANG-1.DAT` still has the `%s`.
A corrected `LANG-1.DAT` or turning the advisor off avoids it. Case 0x89
(doctor to consultant) has the same pattern, but its French text has no
conversion.

### advisor_staff_tired (src/advisor.cpp)

**Potential bug** · `0x619bc`

It picks a tired-staff message but writes no text, and `find_advice` (case
0x10) calls it and ignores the result, so the advisor never warns about
tired staff. The empty `if` in `find_advice` suggests the advice was
disabled on purpose.

### create_chart_data (src/annualch.cpp)

**Bug** · `0xb4aac`

`total`, the sum of a hospital's insurance claims, is set to 0 before the
loop over hospitals but not for each hospital. Each hospital's figure in the
first year-end table includes the claims of all hospitals before it, which
favours later hospitals in that ranking.

### mayor_judges_hospital (src/mayor.cpp)

**Bug** · `0xa13a4`

The litter test checks `litter_count > 10` before `> 20`, so the `+2` for
very heavy litter is never applied; a hospital with more than 20 pieces of
litter scores the same as one with 11.

### check_awards_won (src/awards.cpp)

**Bug** · `0xb3430`

The emergency award needs `award_counts.emergencies > 2`, but nothing writes
that field or `emergency_saved_pct` apart from the yearly reset to 0. The
emergency award and its poor counterpart are never given.

### replace_machine (src/objects.cpp)

**Bug** · `0x3a3e0`

It tests whether the local player (`player`) can afford the machine, but
charges the machine's owner, and takes the cost and strength from
`othervar.cur_player`'s stats. In a single-player game these are the same
hospital; in a network game the affordability test uses whichever player is
local on that machine.

### find_next_thing (src/req3.cpp)

**Potential bug** · `0x73bb0`

When nothing matches, it returns `end_things - thing`, an index one past
the last thing, and the info windows store that as the thing to show. This
needs nothing to match, including the thing currently shown, which
normally matches itself; whether that can happen was not confirmed.

### change_lockholder (src/staff.cpp)

**Potential bug** · `0x2c9cc`

In an operating theatre with one lock holder, it overwrites that holder's
slot with the new staff member and does not evacuate the old one, although
the other slot is free. The old surgeon keeps working without holding the
lock. It may have been meant to fill the free slot.

### pay_staff_bonus (src/staff.cpp)

**Bug** · `0x2b998`

For one staff member, the bonus is worked out from `percent` when given, but
the charge is always `-amount`. No effect in practice: every caller passes
`percent` 0.

### remove_object_from_map (src/objects.cpp)

**Bug** · `0x389b8`

It increments `object_stats[kind].count` where it should decrement it, as
it does `object_count` just before. The count only grows, so the town map's
plant and fire extinguisher totals, and computer players' tally of the
objects they own (`computer_object_levels`, `0x50c44`), overstate what is there once
objects are removed or moved.

### manage_sound (src/sound.cpp)

**Potential bug** · `0x89204`

The branch that stops the emergency clock sample (`SourceID ==
SNDSRC_CLOCK`, 0xd) sits inside `SourceID > 0xe` and inside a test that the
source is a thing, so it never runs. Whether the clock is stopped elsewhere
when the emergency timer runs out was not confirmed.

### make_a_peep (src/peepmake.cpp)

**Bug** · `0x70918`

It fills a new patient's `health_history` (an `s8` per entry, in tenths of
health) with the raw health, 200 to 1000, truncated to a byte;
`add_health_to_history` stores `health / 10`. The patient window's health
graph starts from meaningless values, and the first comparison in
`add_health_to_history` uses them.

### process_staff_complex (src/complex.cpp)

**Bug** · `0x3e724`

In case 54 it passes `thing` (the null thing) to `check_frame` instead of the
patient. No effect beyond the error log: `check_frame` uses the thing only
to number the peep in its "Invalid Frame Lookup" message.

### recolour_outside_room (src/build.cpp)

**Bug** · `0x2583c`

The test that skips a wall piece reads the occupant of the room's top row
instead of the tile being recoloured, and for the east wall uses `x` left
over from the previous loop. No effect: the test looks for a thing of type
3, and every thing type (`THING_*`) is even.

### rle_decompress (src/rle.cpp)

**Bug** · `0xbbf10`

If the temporary buffer cannot be allocated, the data is left encoded but
the decoded size is still returned, so the caller goes on to use encoded
data as decoded. Only when memory runs out.

### wallhug_trivial (src/wallhug.cpp)

**Bug** · `0x491d8`

The path finder gives up when it hits one of its fixed limits: 60000 steps
in all, 252 corners for either wall follower (`wallhug_hugstep`, the
route's 0xfc entries) or 250 waypoints in the route. It then returns no
route even when one exists, and `orthonav_init` reports the destination
unreachable: `check_points_connected` and `can_i_get_to` say no, and
`orthonav_whatnext` gives a person sent there no direction (-3). Measured
on generated maps, this misses about 3% of reachable pairs, all in mazes,
dense clutter and rows of dead ends. The wall following also gets routes up
to 1.9 times the shortest, and its smoothing pass (`wallhug_cleanup`, up to
ten rounds of further searches) makes long winding searches take over a
second at 486 speed.

### orthonav_init (src/orthonav.cpp)

**Bug** · `0x55454`

When no route is found it sets `count` to 0 and returns before recording
the destination (`x1`, `y1`). `person_head_for` (src/nav.cpp) replans when
`count` is 0 and again when the destination differs from `x1`, `y1`, so for
an unreachable destination it runs the failing search twice per call.

## UI & drawing

### remove_old_window_image_from_wscreen (src/engine.cpp)

**Bug** · `0xa77dc`

The test for the left and right strips is `y1 > y1`, always false; the
sibling `remove_old_window_image_from_wscreen_replacing_detail`
(`0xa73c0`) has `y1 > y0`. When a view window moves or shrinks, the parts of
its old image to the left and right of the new one are not cleared.

### process_tracking_windows (src/cakewin.cpp)

**Bug** · `0xab610`

For the main view, the scroll-limit test subtracts the layout's
`track_offset_x` from the y coordinate; the scroll it then sets, and the
same test for the other windows, use `track_offset_y`. The limit is checked
at the wrong height, so centring on a thing near the map edge can be
refused (closing the info windows) or allowed when it should not be.

### display_buttons (src/choice.cpp)

**Potential bug** · `0xa3db4`

The fax answer text is centred vertically on the button using the button's
`width` as its height. This matters only if the button sprite is not
square; its size comes from the data files.

### set_help_text (src/helpmsg.cpp)

**Bug** · `0x99e4c`

The clamp that keeps a help box on screen compares its bottom edge with
`physical_screen_width` instead of `physical_screen_height`. At 640x480 a
box can extend up to 160 pixels below the screen. `draw_help_text`
(`0x9a1f8`) also sets the text window's height to the screen width; that
one has no visible effect, as the graphics window still clips.

### set_font_from_cache (src/fcache.cpp)

**Bug** · `0xb714c`

When the font is already cached, it stamps `font_memory[font].used`, indexing
the block records by font number, instead of `font_memory[cfonts[font] -
1]`, the font's own block. The least-recently-used eviction then works from
stale stamps and can throw out a font in constant use, to be reloaded from
disk.

### process_advisor_text (src/advtext.cpp)

**Bug** · `0xb2124`

It increments `no_pieces` after each attempt, including the one that fits,
so `no_pieces` ends one past the width used. `advisor_text_box` (`0xb1e10`)
draws only while `no_pieces <= advisor_max_segments`, so advice that fits
only in the widest bubble is never shown.

### LbDrawLine (src/lib/bf1/gline.cpp)

**Bug** · `0xc9be0`

`clipped` is set only on some clipping paths, so when the line needs no
clipping the return value is whatever was on the stack. No effect: no caller
uses the result.

### LbScreenSwapBoxClear (src/lib/bf1/sscreen.cpp)

**Bug** · `0xbef14`

Meant to clear the source box to `colour` after copying it, it calls
`memset(src, width, colour)` with the value and length swapped, writing
`colour` bytes of the value `width` instead. It also advances the
destination offset by the back buffer's width but the row end by the
screen's, as does `LbScreenSwapBox` (`0xbf1e8`). No effect in this game:
neither function is called.

### screen_remove (src/lib/bf1/smouse.cpp)

**Potential bug** · `0xbe0cc`

It skips restoring the area under the pointer when the mouse has not moved,
but compares the mouse position with the saved sprite position, which is
offset by the pointer's hotspot. With a hotspot set the two rarely agree, so
the pointer is erased and redrawn even when it has not moved. Only a cost
in work, and the comparison may be deliberate.

## Memory & data layout

### read_level_number (src/io.cpp)

**Bug** · `0x229b0`

It reads two bytes from the save file into the one-byte local `level_no`.
No effect: the local occupies a 4-byte stack slot, so the second byte lands
in its padding.

### set_default_player_details (src/details.cpp)

**Bug** · `0xbaa28`

The three `LbMemorySet(dst, size, 0)` calls pass the size as the fill value
and 0 as the length, so `room_help_seen`, `advisor_flags` and
`track_enabled` are not cleared. No effect in practice: `init_game_vars`
(`0x10330`) has already zeroed all of `myplayer`.

### start_year_end (src/annualch.cpp)

**Bug** · `0xb4980`

It copies 10 bytes of the fax state (`choice`) to
`year_end_backup.choice_state`, which starts at offset 2 of the 10-byte
`year_end_backup`. The last two bytes overwrite `numbuttons` and `numtext`,
the globals that follow it. `check_quarterly_game_over` (`0x58b00`) does the
same. `save_display_screen` (`0xa41b8`) copies 3 bytes for the same field.

### staff_count (src/environ.cpp)

**Bug** · `0x72684`

It counts idle staff in `adcounts.idle_staff[employee->subtype]`, a 4-entry
array, but surgeons have subtype 4 (`STAFF_SURGEON`). An idle surgeon is
counted in the unused byte after the array instead of as an idle doctor, as
`staff_by_kind` does elsewhere.

### main (src/main.cpp)

**Potential bug** · `0x10010`

The `-n` option copies its argument into `myplayer` with `conv_string`, with
no length limit. The player name has room for 8 characters and a
terminator; the bytes after it hold `years_played` and the volume settings,
and a name of more than 91 characters runs past the 0x5c-byte `myplayer`.
Whether a later `load_player_details` restores the settings depends on the
start path.

### display_manager_text, manager_help_displayed_ins (src/manager.cpp)

**Potential bug** · `0x7e42c`, `0x7f968`

They read the insurance company as `gbv.ins_control_base[manager.value]` with
a value of 1 to 3, past the end of `gbv` (the relocation is against
`gbv+0x15e`). This happens to read `ins_control[value - 1]`, which the linker
places right after `gbv`, so the right company is shown; it works only
because of that layout.

### CDosHeap::Free (src/dosheap.cpp)

**Bug** · `0xbb5fc`

When `p` is not an allocated block, the error message prints `rec->ptr` with
`rec` null, instead of `p`. Under DOS/4GW the read does not fault, so the
message shows a wrong address.

### LbFileMakeFullPath (src/lib/bf1/gfile.cpp)

**Bug** · `0xbc504`

The trailing-backslash test indexes `directory` by the length of the whole
path so far, so with `current_dir` set it reads past the end of `directory`
and can add or omit the separator wrongly. No effect in this game: its only
caller, `LbErrorLogSetup` (`0xcd1e0`), is never called.

## Network

### IPXHandler::SetupVector (src/lib/bf2/dosipx.cpp)

**Bug** · `0xd422c`

To reuse an interrupt vector that points at a bare IRET, it compares the
byte there, read as `s8`, with 0xcf; a signed byte is never 207. Only unset
(0:0) vectors from 0x60 to 0x65 are used, and IPX setup fails if all six are
set.

### LbNetwork_Init (src/lib/bf1/opt/netint.cpp)

**Bug** · `0xbfc4c`

The loop meant to clear every entry of the player info table clears
`localPlayerInfoPtr->active` and `->name` each time without advancing, so
only the first entry is cleared. No effect in this game: `setup_network`
(`0x850f4`) zeroes the table just before the call.

### ServiceProvider::Send (src/lib/bf2/sp.cpp)

**Bug** · `0xd25ac`

For a type 8 message to the local player it passes the uninitialised
`player_id` to the type 8 callback. No effect in this game:
`LbNetwork_Init` sets that callback to null and nothing sets it.

### ServiceProvider::Receive (src/lib/bf2/sp.cpp)

**Potential bug** · `0xd1e24`

A waiting message of type 3, of an unknown type, or of type 8 without a
callback is never read, so the loop peeks the same message forever and the
game hangs. Theme Hospital does not send such messages; whether one can
arrive (from a corrupt packet, say) was not confirmed.

### ModemSP::Start (src/lib/bf2/modemsp.cpp)

**Bug** · `0xee77c`

The "MODEM RESPONSE CODE = %s" debug lines pass `comm`, the serial handler
object, as the string. No effect: `NetworkDebugPrintf` does nothing in
this build.

### ModemSP::Stop (src/lib/bf2/modemsp.cpp)

**Potential bug** · `0xeedd8`

`ret` is set only when a session is running, so outside one it returns
whatever was on the stack. `ModemSP::Release` (`0xee718`) calls it only in a
session and ignores the result; no other caller that tests it was found.

### IPXServiceProvider::Enumerate (src/lib/bf2/ipxsp.cpp)

**Bug** · `0xd6e2c`

`count` is zeroed only when a reply's player count differs from `total`,
which starts at 32, so a first reply listing exactly 32 players is counted
from an uninitialised value. No effect in this game: its sessions hold at
most four players.

### IPXServiceProvider::AddPlayerToSession (src/lib/bf2/ipxsp.cpp)

**Potential bug** · `0xd852c`

If the slot search finds no free slot, it still broadcasts a BUSA message
with `players[i]`, one past the table. The search runs only when
`num_players < max_players`, so a free slot should exist unless the player
table and the count disagree.

## Sound (Bullfrog sound library and Miles)

### DetermineSoundType (src/lib/bf2/init_sou.cpp)

**Bug** · `0xdd040`

The switch that steps down to the next sound bank type has no `default`, so
for a `SoundType` not in its list a failed allocation leaves the type
unchanged and the loop retries forever. Setting `BF_FORCE_SOUND_TYPE` to
such a value hangs the game at startup when that bank cannot be loaded.

### SetMusicMasterVolume, SetSoundMasterVolume (src/lib/bf2/mast_vol.cpp)

**Bug** · `0xefed0`, `0xeff20`

Both refuse to change the volume while `BF_FORCE_MIDI_MASTER_VOLUME` /
`BF_FORCE_SOUND_MASTER_VOLUME` is set, and `InitMusic` and `InitSound` set
the override by calling them with its value. The forced volume is never
applied; the master volume stays at the driver's default.

### LoadAwe32Soundfont (src/lib/bf2/snd/awe32.cpp)

**Bug** · `0xef830`

It formats `<MusicDataPath>/<prefix>.sbk` into a 20-byte buffer, which a
music path longer than 6 characters (with the "Bullfrog" prefix) overflows,
and every failure after the file is opened returns without closing it. No
effect in this game: Theme Hospital sets `UseCurrentAwe32Soundfont`, so the
function returns at once.

### AIL_API_read_INI (src/lib/ail/ail.c)

**Bug** · `0xdedd0`

It passes `sizeof(line)` (80) as `maxlen` to `AIL_fgets` (`0xded50`), which
stores up to `maxlen` characters and then a terminator. A line of 80 or more
characters in `DIG.INI` or `MDI.INI` writes one byte past `line`.

### AIL_API_set_sample_file (src/lib/ail/ailsfile.c)

**Potential bug** · `0xe1d30`

Unlike `AIL_API_allocate_file_sample`, it installs no end-of-sample
callback, so a VOC file with several voice blocks plays only its first.
This may be intended for a caller-managed sample; the game's samples go
through it, but whether any are multi-block VOCs was not checked.

### AIL_API_quick_load (src/lib/ail/ailqapi.c)

**Bug** · `0xe6e90`

It links the new file in front of the loaded list (`audio->next = audios`)
but never updates `audios`, so the list stays empty and
`AIL_API_quick_shutdown` frees none of the loaded files. No effect in this
game, which does not use the Quick API.

### AIL_file_write, AIL_file_append (src/lib/ail/memfile.c)

**Bug** · `0xdde70`, `0xddf00`

On a short write they return without closing the file, and their second
`written != len` test (for `AIL_DISK_FULL`) can never be reached. No effect
in this game: neither is called.

### AILXMIDI_end (src/lib/ail/ailxmidi.c)

**Bug** · `0xe5320`

It unlocks everything `AILXMIDI_start` locked except the variable `j`,
which stays locked. No visible effect.

### AIL_API_channel_notes (src/lib/ail/ailxmidi.c)

**Bug** · `0xe4c90`

It counts the notes on a channel over `NUM_CHANS` (16) entries of the note
queue, which has `MAX_NOTES` (32), so notes in the second half are missed.
No effect in this game: it is not called.

### AIL_API_create_wave_synthesizer (src/lib/ail/ailxdig.c)

**Bug** · `0xe5e30`

When sample handles run out it frees the synthesizer but leaves the MIDI
driver's event and timbre callbacks hooked and `mdi->system_data[0]`
pointing at the freed memory. No effect in this game: it is not called.

### AIL_digital_configuration, AIL_sequence_position (src/lib/ail/aildebug.c)

**Bug** · `0xc4910`, `0xc67a0`

With `MSS_DEBUG` tracing on, the result line dereferences every output
pointer, although the API allows them to be null.
`AIL_digital_configuration` also passes `*string`, a `char`, for `%s`. Only
with tracing on.

## Smacker

### SmackOpen (src/lib/smk/smackinp.c)

**Bug** · `0xe9c80`

When allocating the `Smack` fails, the `fail_close` path calls
`radclose(smk->handle)` with `smk` null; when the header is bad, the
`fail_free` path frees `smk` and then reads `smk->handle` the same way. The
file is closed through a null or freed pointer, so the wrong handle may be
closed and the file left open.

### SmackColorRemap (src/lib/smk/smackinp.c)

**Bug** · `0xe85b0`

With `numcolors` 0 the search loop never runs and `best` is stored
uninitialised for every colour. No effect in this game: it is not called.

## C runtime

No bugs were found in the Watcom C runtime functions the game links.
