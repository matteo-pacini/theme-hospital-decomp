#pragma once

#include "core/data/anims.h"
#include "core/world/complex.h"
#include "core/world/world.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace th
{

// Staff's action sequences ("complexes"), as the game's
// process_staff_complex (src/complex.cpp), and the interactions between two
// people (src/interact.cpp). They work as the patients'
// (core/world/complex.h): a complex is a run of step bytes in
// StaffComplexes::group, ending at 0, each a kStaffComplex index, named by
// where it starts. A complex is given with set_complex (core/world/complex.h).

// The staff's complexes: DATA/S_COMP.LEX, as the game's staff_complex_group
// (loaded by src/files.cpp).
struct StaffComplexes {
    std::vector<uint8_t> group;

    // The step byte at `pc`; 0 (the end of a complex) past the file's end.
    uint8_t at(uint32_t pc) const { return pc < group.size() ? group[pc] : 0; }
};

// Loads DATA/S_COMP.LEX from the game data folder `dir`. Fails, saying why
// in `error`, if it cannot be read or names a step kStaffComplex lacks.
bool load_staff_complexes(const std::filesystem::path &dir, StaffComplexes &out, std::string *error = nullptr);

// Per staff kind, the start of each of its animations (an AnimData::starts
// index, to which the facing is added; 9999 for none): [0] walking, [5]
// standing. Copied from the game's staff_anim_starts (src/anitable.cpp);
// the columns the game leaves out are 0.
extern const std::array<std::array<uint16_t, 0x8c>, 5> kStaffAnimStarts;

// process_staff_complex (0x3e724): runs the step of member of staff `id`'s
// complex at complex_pc and moves on, starting the queued complex or
// entering next_state at its end; chained steps run at once. Returns 1 after
// a step, 2 if no complex is running, 0, 3 or 4 if the step failed
// (fail_staff_complex, or SS_COMPLEX_RETRY). Draws from world.seed as the
// game does. A step that drives the slave runs the patient's complex with
// `person_complexes`; `game_turn` is the turn being played.
//
// Doors (actions 14, 20 and 22) take the member of staff into or out of a
// room, claiming it on the way in (check_room_mutex_on_entry) and
// answering its page (pager_arrival); complex_success (action 52) adds
// research points, rests staff and counts a use of their object. Actions 60
// and 62 play the surgeon screen changing animations by the room's count of
// surgeons. Not yet ported: autopsies among complex_success's kinds.
uint8_t process_staff_complex(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                              const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn);

// rest_staff (0x2be14): member of staff `s` (of `world`) rests: tiredness
// down by `amount` (by amount * gbv.recovery_factor / tiredness when more
// tired than half gbv.recovery_factor, but at least gbv.recovery_minimum;
// it may go below 0), happiness up by 1 (0-100).
void rest_staff(World &world, Thing &s, int16_t amount);

// fail_staff_complex (0x41c0c): ends member of staff `id`'s failed complex:
// back to resume_state standing, the complex and its queue cleared; with no
// resume_state, SS_COMPLEX_RETRY to retry the step. A handyman going back
// holds no repair page any more.
void fail_staff_complex(World &world, const AnimData &anims, uint16_t id);

// start_new_interaction (0x54dd4): starts interaction sequence `seq` (1 for
// the first of the game's interaction table) between `id` and its slave,
// putting `id` in PS_TWO_PEEPS_INTERACT or SS_TWO_PEEPS_INTERACT and playing
// the first step's animations. At the sequence's end `id`'s complex goes on.
void start_new_interaction(const PersonRun &run, const StaffComplexes &staff_complexes, uint16_t id, uint8_t seq);

// One game turn of member of staff `id` in a state of the complexes, as
// process_things: SS_PLAY_COMPLEX_ANIM (play_complex_staff_anim, 0x4029c),
// SS_TWO_PEEPS_INTERACT (two_peeps_interact, 0x54870),
// SS_WAIT_FOR_COMPLEX_SLAVE (staff_wait_for_complex_slave, 0x403d0) and
// SS_COMPLEX_RETRY (staff_complex_retry, 0x4031c). Returns false, doing
// nothing, for other states.
bool staff_complex_turn(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                        const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn);

} // namespace th
