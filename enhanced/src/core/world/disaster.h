#pragma once

#include "core/data/anims.h"
#include "core/world/world.h"

#include <cstdint>

namespace th
{

class Game;
struct Event;

// Machines blowing up, as the game's src/disaster.cpp (machine_disaster
// and the explosion's debris and damage), and the machine window's
// commands that keep them going (replace_machine and the call for a
// handyman, src/objects.cpp and src/io.cpp).

// The game's animation of an exploding machine's debris (the anim
// machine_disaster passes create_explosion).
constexpr uint16_t kAnimMachineDebris = 0x97;

// modify_machine (0x38f14), for machine `id` worn to 75% of its strength
// or more: its announcement (World::machine_announcements, the player's
// machines) and smoke (Thing::smoke, a puff shown at random, a draw from
// world.seed; from 85% with one shown, the warning sound and a second puff
// beside it, another draw, or from 95% all three; its overlay_frame from
// the start); from 100% it may break down (a draw: one chance in
// strength * 3 / 2 - times_used + 1, surely from 150%), machine_disaster.
// Returns whether it broke down.
bool modify_machine(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

// machine_disaster (0x75010): machine `id` blows up. An explosion
// (create_explosion) from the cell kObjects' explosion offset (one cell up
// and left) from it, the room wrecked round there (blast_effect: the machine
// and every object within 4-7 cells taken off, the wrecks of machines left
// as markers, walls marked damaged), its staff killed (sack_staff mode 2)
// and its patients dying (person_dies), its treatments no longer available
// unless another room of its type is left (remove_expertise_room,
// room_deleted_update_listhead), the room ROOM_DESTROYED and closed
// (RS_CLOSED), a mishap counted (World::mishaps, World::machine_exploded),
// the reputation lost (kRepMachineExploded) and the patients queueing for
// it sent on again (restart_entire_queue). The game counts the mishap for
// the machine's owner read after it is gone, so always hospital 0.
void machine_disaster(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

// create_explosion (0x75484): a burst of debris (kThingDebris things with
// animation `anim`, create_bang) thrown up from `tile` in every direction,
// two draws from world.seed each, and the bomb's sound.
void create_explosion(World &world, const AnimData &anims, uint16_t tile, uint16_t anim);

// process_bang (0x75110), a piece of debris' turn: it moves by its
// velocity, slowed by a quarter a turn and falling, and its animation
// advances every anim_speed turns; it is removed (remove_thing) when it
// lands, its animation ends, or, thrown in a room, it moves past the room's
// first column or row or onto its last.
void process_bang(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

// replace_machine (0x3a3e0): machine `id` replaced by a new one for its
// kind's price (object_cost) if the balance is positive and covers it (charged
// only if the thing's owner is hospital 0): its strength that of a new one (object_strength), no wear or
// smoke, the handyman on his way to repair it called off (interrupt_staff)
// and its repair page cancelled.
void replace_machine(Game &game, uint16_t id);

// Carries out the machine window's commands, as process_turn: EV_REPLACE_MACHINE
// (replace_machine of thing event.value) and EV_THINGPAGE_REQUEST (a
// handyman paged at once, priority 0xff, to repair thing event.value; the machine gets kIconNeedsRepair). Returns
// whether the event was one of them.
bool apply_machine_event(Game &game, const Event &event);

// disaster_peeps (0x75ed0): applies `mode` to every patient of hospital
// `owner` (-1 for all): 0 makes them about to vomit (nausea at
// gbv.vomit_limit + 10), 1 and 2 set and clear PF_QUAKE_SHAKEN for the kinds
// whose abilities allow it, 3 kills them (change_health by all their
// health). Earthquake's scaring (modes 1 and 2) and the sickness disaster
// (mode 0) share it.
void disaster_peeps(World &world, const AnimData &anims, const Balance &settings, uint8_t mode, int8_t owner,
                    uint32_t game_turn);

// generate_disaster (0x75b54): runs hospital `owner`'s random disaster
// (World::disaster_kind). Every gbv.disaster_launch days a new one may
// start (a draw from world.seed, low two bits: 0 nothing, 1 cold, 2 hot,
// 3 sick), so 1 or 2 the
// heating fails cold or hot (heating_level 0 or 10) when the hospital has
// radiators and fewer than one handyman per 8 of them, 3 everyone feels
// sick (nausea) while it has over 10 pieces of litter. A running one counts
// down (faster for the heating with more handymen) and then ends,
// restoring the heating and giving the advisor a forced message at start
// and end. The port only has the single local hospital's fields, so `owner`
// is 0.
void generate_disaster(Game &game, uint8_t owner);

} // namespace th
