#pragma once

#include "core/data/anims.h"
#include "core/world/events.h"
#include "core/world/world.h"

#include <array>
#include <cstdint>

namespace th
{

struct Date;
class Game;

// VIPs (src/mayor.cpp): the daily chance of a visit, the VIP itself walking
// a hospital's entrances, reception desk and rooms of interest, and the
// verdict it leaves, which pays a cash award and moves the reputation.
// Hospital 0's visit is World::mayor (the game's allhosps[] mayor fields).

// The player call about a VIP's offered visit (the game's popup type 6).
constexpr uint16_t kCallVip = 6;

// The fax answers to a VIP offer (EV_CHOICE_RESULT values, the game's
// CH_VIP_ACCEPT and CH_VIP_REFUSE).
constexpr uint16_t kChoiceVipAccept = 0x14;
constexpr uint16_t kChoiceVipRefuse = 0x15;

// One band of a VIP's verdict (the game's mayor_result, src/tables.cpp): the
// highest score it covers, the reputation event it applies (0 for none) and
// the cash paid.
struct MayorResult {
    int16_t max_score = 0;
    uint8_t reputation_event = 0;
    int16_t cash_bonus = 0;
};
extern const std::array<MayorResult, 15> kMayorResult;

// generate_mayors (0xa0cac), daily (daily_admin): once hospital 0 has waited
// gbv.mayor_launch days with a reception desk, offers it a random VIP by a
// call (kCallVip, expiring after 20 days); an accepted visit (World::Mayor
// state 2) starts at its arrival turn, making the VIP (create_mayor) and,
// for the player, queueing the advisor's advice. Nothing else while a visit
// is outstanding.
void generate_mayors(Game &game);

// The walker a VIP and the health inspector share (src/goto.cpp 0x78640,
// src/mayor.cpp 0xa1af0 and 0xa1754): walking to a stop, the arrival there,
// and leaving the map. mayor_gone clears an inspector's inspection[1] or
// ends a VIP's visit by its type.
void mayor_goto(Game &game, uint16_t id, uint32_t game_turn);
void mayor_inspects(World &world, const AnimData &anims, uint16_t id);
void mayor_gone(World &world, uint16_t id);

// One turn of a VIP `id` (process_things' kThingMayor): walking to its next
// stop, waiting at one, or leaving the map with its verdict.
void mayor_turn(Game &game, uint16_t id, uint32_t game_turn);

// The player's commands about a VIP (process_turn, src/io.cpp):
// - EV_ANSWER_PLAYERCALL kCallVip: the offered visit's call is taken, its
//   fax shown (Game::choice_screen kFaxVip, its subject the VIP's name);
// - EV_ANSWER_PLAYERCALL kCallEmergencyOver with subject 2: the visiting
//   VIP's result call ends the visit and shows its report (kFaxVipResult,
//   closing itself after 400 turns);
// - EV_CHOICE_RESULT kChoiceVipAccept: the visit is scheduled (state 2,
//   arriving in 1500 turns);
// - EV_CHOICE_RESULT kChoiceVipRefuse: the visit is dropped (state 0), after
//   which, past five refusals in a row and while the turn's random
//   percentage is below 10, an offer is accepted anyway.
// Returns whether `event` was one of these.
bool apply_mayor_event(Game &game, const Event &event);

} // namespace th
