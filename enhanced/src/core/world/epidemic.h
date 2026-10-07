#pragma once

#include "core/world/world.h"

#include <cstdint>

namespace th
{

struct AnimData;
struct Event;
class Game;

// Epidemics and the health inspector (src/epidemic.cpp, src/inspect.cpp,
// src/vaccine.cpp): a patient who is diagnosed while contagious raises the
// player's epidemic call, answered by the fax to declare it and pay the
// fine, or to cover it up; a cover-up counts down (World::Epidemic::timer)
// and, when it ends, sends the health inspector to judge it (inspect.cpp).
// The vaccination of a cover-up's contagious patients is the nurse's
// (vaccine.h).

// The player calls about an epidemic: the declaration the fax asks, and the
// result the inspector's verdict raises (subject 3 of the kind's call).
constexpr uint16_t kCallEpidemic = 3;
constexpr uint16_t kCallEpidemicOver = 5;

// The fax answers to the epidemic's declaration (EV_CHOICE_RESULT values,
// the game's CH_EPIDEMIC_DECLARE and CH_EPIDEMIC_COVER_UP).
constexpr uint16_t kChoiceEpidemicDeclare = 0xa;
constexpr uint16_t kChoiceEpidemicCoverUp = 0xb;

// The health inspector's states (the game's MAYOR_*, include/enums.h),
// shared with the mayor, which is not ported: walking to its stop, waiting,
// leaving the map.
constexpr uint8_t kMayorGoto = 2;
constexpr uint8_t kMayorPauses = 4;
constexpr uint8_t kMayorGone = 6;
constexpr uint8_t kMayorInspects = 8;

// The epidemic's sound effects (SFX_* in include/enums.h), played by the
// interface from World::epidemic_sounds.
constexpr uint8_t kSfxEpidemicStart = 0xcd;
constexpr uint8_t kSfxEpidemicEnd = 0xce;
constexpr uint8_t kSfxVaccin = 0xde;
constexpr uint8_t kSfxDeVaccin = 0xdf;

// alert_epidemic (0xa0414): starts an epidemic of `illness` in `owner`'s
// hospital unless one or an emergency is already running: withdraws any
// emergency call (get_playercall 2) and raises the epidemic call (type 3,
// subject 1, expiring after 20 days). The player's advisor flag and the
// announcement are the interface's.
void alert_epidemic(World &world, uint8_t illness, uint8_t owner);

// count_and_flag_contagious_peeps (0xa04e4): goes over `owner`'s patients
// inside the building who carry `illness`: mode 1 flags them contagious
// (curing fed-up ones of it instead) and counts them, mode 2 cleans them all
// and counts those that were flagged. Patients outside lose their contagion.
// Returns the count (0 for other modes).
uint32_t count_and_flag_contagious_peeps(World &world, const AnimData &anims, uint8_t illness, uint8_t mode,
                                         uint8_t owner);

// epidemic_status (0xa0654): counts down `owner`'s epidemic cover-up
// (World::Epidemic::timer): it ends when no contagious patient is left
// (contagious_count) or the timer runs out. On ending, clears the cover-up
// and sets epidemic_over. Returns 1 if it ended this call.
uint32_t epidemic_status(World &world, uint8_t owner);

// epidemic_over's call (0xa073c's display_choice_screen): the result fax for
// the verdict the inspector reached (World::Epidemic::inspection[0]).
void epidemic_over_result(Game &game);

// expire_playercalls' default answer for an epidemic call that ran out
// unanswered (src/peeps.cpp 0x18908): choice_spare_data 0x63 then
// CH_EPIDEMIC_COVER_UP, the cover-up started as the fax's answer does.
void expire_epidemic_cover_up(World &world);

// send_peeps_home (0xa0a30): sends `owner`'s patients inside the building
// home (PF_SENT_HOME_BY_INSPECTOR), all of them or (all zero) only the
// contagious ones, clearing their contagion.
void send_peeps_home(World &world, uint8_t all, uint8_t owner);

// clean_contagious_peep (0xa0ae8): cures patient `id` of contagion: zeroes
// their contagion, clears PF_CONTAGIOUS, their place in the vaccination
// queue and takes one off the contagious count. The status icons are not
// ported.
void clean_contagious_peep(World &world, uint16_t id);

// create_inspector (0xa1c10): creates the health inspector for `owner` at
// one of the hospital's entrances, walking to the reception desk to inspect
// (kMayorGoto to kMayorInspects), or straight back out if there is none.
// Returns its thing index, or 0 if no thing was free. Draws from world.seed
// as the game does.
uint16_t create_inspector(World &world, const AnimData &anims, uint8_t owner);

// generate_inspector (0xa1e3c): sends the health inspector to `owner` after
// an epidemic cover-up, unless one is already on the way.
void generate_inspector(World &world, const AnimData &anims, uint8_t owner);

// One turn of health inspector `id` (process_things' THING_INSPECTOR):
// kMayorGoto walks it to its stop (mayor_goto, core/world/mayor.h),
// kMayorInspects and kMayorPauses wait there and judge (inspector_pauses,
// 0xa1ed4), and kMayorGone removes it. Draws from world.seed as the game
// does.
void inspector_turn(Game &game, uint16_t id, uint32_t game_turn);

// The player's commands about an epidemic (process_turn, src/io.cpp):
// - EV_ANSWER_PLAYERCALL kCallEpidemic: the epidemic call is taken
//   (get_playercall) and, if contagious patients were counted, the
//   declaration fax shown (kFaxEpidemic) with its fine;
// - EV_CHOICE_RESULT kChoiceEpidemicDeclare: the cover-up is cleaned
//   (count_and_flag_contagious_peeps, mode 2), the fine paid
//   (kTxEpidemicFine) and the reputation hit (kRepEpidemicDeclared);
// - EV_CHOICE_RESULT kChoiceEpidemicCoverUp: the contagious are flagged
//   (mode 1), the cover-up starts (active, timer 100).
// Returns whether `event` was one of these.
bool apply_epidemic_event(Game &game, const Event &event);

} // namespace th
