#pragma once

#include "core/data/anims.h"
#include "core/world/world.h"

#include <cstdint>

namespace th
{

// Patients dying, as the game's treat.cpp (person_dies, person_dead) and
// peeps.cpp (person_flys), and the grim reaper who may come for them
// (src/reaper.cpp, reaper_goto_hell in src/goto.cpp). The action sequences
// run come from World::person_complexes.

// Patient states (the game's PS_*): dead, floating away, gone into the
// gates of hell.
constexpr uint8_t kPsDead = 0x32;
constexpr uint8_t kPsFlys = 0x34;
constexpr uint8_t kPsAtHellGate = 0x42;

// Thing.person_flags: the patient has died (the game's PF_DYING).
constexpr uint32_t kPfDying = 0x40000;

// The grim reaper's states (the game's REAPER_*).
constexpr uint8_t kReaperPlayAnim = 2;
constexpr uint8_t kReaperWaits = 4;
constexpr uint8_t kReaperRemove = 6;
constexpr uint8_t kReaperGotoHell = 8;

// The gates of hell's object kind and state as they rise or sink (the
// game's OBJ_GATES_OF_HELL and OS_HELL).
constexpr uint8_t kObjGatesOfHell = 48;
constexpr uint8_t kObjectHell = 6;

// person_dies (0x3ccc8): patient `id`, out of its queue, walks (complex
// 110) to the first free cell near it it can reach with a free cell beside
// it to fall on (the game's dying_place; its own if none), to lie there
// dead (PS_DEAD). Counted as a death of its illness and of its hospital
// (hospital 0's: HospitalSettings illnesses[].deaths, Finance deaths and
// deaths_this_month), with the
// reputation lost (kRepDied) and the statistics (record_person_as_patient);
// its player calls (kinds 8 and 4) are dropped. Its status icon is not
// ported.
void person_dies(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

// person_dead (0x3cf5c), PS_DEAD's turn for patient `id`: just fallen
// (substate below 0x1e), it turns to a free side (a draw from world.seed)
// and plays its death (complex 320; 640 for an alien, who becomes itself
// again), then floats away (substate 0x2f) or, for the kinds the reaper
// takes, half the time (another draw) calls him (0x30); then it floats
// away (complex 460 to PS_FLYS), or the reaper comes (summon_the_reaper),
// which counts as a cure of its illness (cure_illness, as the game), or,
// if he cannot, it floats away.
void person_dead(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

// person_flys (0x17cb0), PS_FLYS's turn: patient `id` rises by its speed
// a turn (Thing.countdown, the game's fly_height) and is removed
// (remove_person) once 400 up.
void person_flys(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

// summon_the_reaper (0x5c230): for dead patient `victim`, raises the gates
// of hell on the first cell of the game's gate_arrive round it that it can
// reach, that takes them (can_place_object, corridor_object_nav_check, and
// place_object's checks: the rooms left reachable, no one in the way) and
// from which it can reach their use cell 1, and puts the reaper on the
// first cell of reap_arrive round them from which he can reach their use
// cell 0, linked to the victim (Thing.slave both ways), to play his
// arrival and walk there (REAPER_GOTO_HELL). Returns 0 if he is already
// coming or no place was found (the gates taken down again), else 1, also
// when no thing was free for him. The advisor's word when the gates would
// block the rooms is not given.
uint8_t summon_the_reaper(World &world, const AnimData &anims, uint16_t victim);

// One game turn of the grim reaper `id`, as process_things for a
// THING_REAPER: reaper_play_anim (0x5c6f8) plays his animation, then
// enters reaper_next_state with next_sprite, or leaves; reaper_waits
// (0x5c8e0) swings at the victim come to the gates, removes it once gone
// in (PS_AT_HELL_GATE), and leaves after 1500 swings or if it will not come;
// remove_reaper (0x5cbe4) removes him and sinks the gates (OS_HELL, removed
// at the animation's end); reaper_goto_hell (0x7724c) walks him to the
// gates, where he calls the victim to them (reaper_calls_victim, 0x5c7b4).
void reaper_turn(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

// object_hell (0x3751c), OS_HELL's turn for object `id`: its animation
// plays, and at its end goes on to aux_sprite, or the object is removed
// (remove_object) if remove_after_anim.
void object_hell(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

} // namespace th
