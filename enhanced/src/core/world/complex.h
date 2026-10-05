#pragma once

#include "core/data/anims.h"
#include "core/world/world.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace th
{

// Patients' action sequences ("complexes"), as the game's complex.cpp, and
// the patient states that run them, as process_things (src/things.cpp) and
// peeps.cpp.
//
// A complex is a run of step bytes in PersonComplexes::group, ending at 0,
// each a kPersonComplex index (core/world/complex_tables.h). Complexes are
// named by where they start; the game's start at multiples of 10. A person
// runs a step when the complex is given, on arrival at its destination
// (PS_GOTO_DESTINATION) or when an animation ends (PS_PLAY_COMPLEX_ANIM).

// Patient states (Thing.state; the game's PS_*, include/enums.h).
constexpr uint8_t kPsWaitsForBusy = 0x04;
constexpr uint8_t kPsGotoDestination = 0x06;
constexpr uint8_t kPsGone = 0x0e;
constexpr uint8_t kPsPlayComplexAnim = 0x10;
constexpr uint8_t kPsInQueue = 0x14;
constexpr uint8_t kPsEntertainsThemself = 0x18;
constexpr uint8_t kPsComplexRetry = 0x2a;
constexpr uint8_t kPsStuck = 0x3c;

// The complex that sends a patient to its destination cell and off the
// map (start_person_going, person_go_home).
constexpr uint16_t kComplexLeave = 20;

// The patients' complexes: DATA/P_COMP.LEX, as the game's
// person_complex_group (loaded by src/files.cpp).
struct PersonComplexes {
    std::vector<uint8_t> group;

    // The step byte at `pc`; 0 (the end of a complex) past the file's end.
    uint8_t at(uint32_t pc) const { return pc < group.size() ? group[pc] : 0; }
};

// Loads DATA/P_COMP.LEX from the game data folder `dir`. Fails, saying why
// in `error`, if it cannot be read or names a step kPersonComplex lacks.
bool load_person_complexes(const std::filesystem::path &dir, PersonComplexes &out, std::string *error = nullptr);

// set_complex (0x417ac): gives patient `id` complex `action` with
// destination cell `arg` (column + row * 128; 0 for none). With no complex
// running it starts at once, its destination set (set_destination_coords),
// or, if a step of action 24 left one to resume, that resumes with `action`
// queued behind it; returns true. Otherwise `action` is queued (in the second
// slot if the first is taken, replacing what it held) and returns false.
// The caller then runs its first step (process_person_complex) if wanted.
bool set_complex(World &world, uint16_t id, uint16_t action, uint16_t arg);

// process_person_complex (0x40524): runs the step of patient `id`'s complex
// at complex_pc and moves on, starting the queued complex or entering
// next_state at its end; chained steps run at once. Returns 1 after a step,
// 2 if no complex is running, 0 or 3 if the step failed
// (fail_person_complex). Draws from world.seed as the game does. `game_turn` is the turn being played
// (the game's actual_game_turn).
//
// Not yet ported: the rooms, doors and staff the steps can involve (there
// are none yet: find_door finds no door, so action 20 fails as in the game
// and 14 does nothing), action 22 (leaving a door, which only follows a
// successful 20) and complex_success (action 52, which only follows using
// an object of a kind not yet placeable).
uint8_t process_person_complex(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                               uint32_t game_turn);

// fail_person_complex (0x41d54): ends patient `id`'s failed complex: back
// to resume_state standing, the complex and its queue cleared; with no
// resume_state, PS_COMPLEX_RETRY to retry the step.
void fail_person_complex(World &world, const AnimData &anims, uint16_t id);

// start_person_going (0x173c0): sends patient `id` on its way: to the
// nearest manned reception desk with room in its queue or, there being
// none, to a random one of its hospital's entrances (World::entrances) and
// off the map (kComplexLeave), and runs the complex's first step. Draws
// from world.seed.
// There are no rooms or staff yet, so it always leaves.
void start_person_going(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                        uint32_t game_turn);

// remove_person (0x16760): takes patient `id` off the map and frees its
// entry: abandons its complex (abandon_complex), then unlinks it from its
// cell (remove_thing). Queues, rooms, vaccinations and the patient's
// window, which do not exist yet, are not handled.
void remove_person(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                   uint32_t game_turn);

// One game turn of patient `id`, as process_things for a THING_PATIENT: its
// state's routine. Returns whether it still exists (PS_GONE removes it).
// Handled: PS_GOTO_DESTINATION (person_goto_destination, as walk_turn: the
// blueprint, needs and reaction checks are not run, which is exact on
// outdoor cells, where the game draws nothing for them; a failed walk goes
// to person_nav_failure, which leads to PS_STUCK), PS_GONE,
// PS_PLAY_COMPLEX_ANIM and PS_COMPLEX_RETRY. Other states do nothing yet,
// and nor do the per-turn counts and status icon.
bool person_turn(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                 uint32_t game_turn);

// One game turn of object `id` if it is an entrance door in one of the
// sliding states (OS_SLIDING_DOOR_*), as process_things: closed, it starts
// opening when a patient or member of staff stands on either cell in front
// of it; it plays its opening animation, stays open until 50 turns after the
// last one has gone, then closes, reopening from where it is if someone
// comes. Does nothing for other objects or states.
void sliding_door_turn(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

} // namespace th
