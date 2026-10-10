#pragma once

#include "core/world/world.h"

#include <cstdint>

namespace th
{

// The statistics pass of process_things (src/environ.cpp), on game turns
// that are multiples of 32 (othervar.every_32_turns): reset_all_count before
// the things' turns, person_count, staff_count and object_count for each
// thing in its turn, final_count after them. Hospital 0's figures go into
// World::counts (the game's adcounts), its emergency patients indoors into
// World::Emergency::present. On other turns they do nothing.

// reset_all_count (0x725dc): World::counts back to its start (keeping
// AdCounts::has_patients) and no emergency patients present.
void reset_all_count(World &world, uint32_t game_turn);

// person_count (0x72880): patient `id`, of any hospital, makes
// AdCounts::has_patients true. Indoors, one of hospital 0's adds to the
// patients' counts and sums (impatience, waiting time, happiness,
// refreshment, warmth, bladder, its illness) and, the first time, to this
// month's visitors in the graphs' history (Finance::history, kept by
// kPfCountedVisitor); an emergency patient of hospital 0 counts as present.
// Hospital 0's contagious patients add to World::epidemic.contagious_count, and one outdoors resets its timer.
void person_count(World &world, uint16_t id, uint32_t game_turn);

// staff_count (0x72684): hospital 0's member of staff `id`, not an
// applicant: its kind's and the staff's tiredness, happiness and warmth
// sums, the doctors' and nurses' count, its qualifications, the most tired,
// and whether it is idle (wandering, or in a room other than the training
// room with no patient queueing or coming).
void staff_count(World &world, uint16_t id, uint32_t game_turn);

// object_count (0x72ba4): machine `id`: hospital 0's count and wear sum
// (times used * 512 / (strength * 3)), and the machine of the most wear
// past its strength.
void object_count(World &world, uint16_t id, uint32_t game_turn);

// final_count (0x72b20): the pass is over (a `counts` trace event). The
// average happinesses the game stores here are read by nothing ported and
// are not kept.
void final_count(const World &world, uint32_t game_turn);

// process_things run again for thing `id` alone within a turn (a complex
// step of action 50, person_fills waking a doctor): on a counting turn the
// pass starts over (reset_all_count) and counts the thing alone
// (person_count or staff_count) before its turn; final_count after it ends
// that pass, the rest of the things then counting on from there.
void process_things_alone_begin(World &world, uint16_t id, uint32_t game_turn);

// Thing.person_flags: counted in this month's visitors (PF_COUNTED_VISITOR).
constexpr uint32_t kPfCountedVisitor = 0x200000;

} // namespace th
