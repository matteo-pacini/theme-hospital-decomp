#pragma once

#include "core/data/anims.h"
#include "core/data/balance.h"
#include "core/world/events.h"
#include "core/world/world.h"

#include <array>
#include <cstdint>

namespace th
{

struct ArrivalState;
struct Date;
class Game;
struct PeepSetup;

// Emergencies and earthquakes (src/emerg.cpp, src/quake.cpp): the level's
// Balance::emergency_control and quake_control entries picked in turn, at
// random turns within their months; the emergencies offered to the player
// by a call, accepted or refused, their helicopter and patients, and their
// end with a bonus or a loss of reputation (hospital 0's, World::emergency).
// The earthquakes themselves (generate_quake) are not ported.
struct Disasters {
    // The next emergency (gamevar.next_emergency_turn and on): the turn it
    // is due, its illness, patients, cure percentage to win and bonus per
    // patient; pending while one is scheduled.
    uint32_t next_emergency_turn = 0;
    uint8_t emergency_illness = 0;
    uint8_t emergency_patients = 0;
    uint8_t emergency_win_percent = 0;
    uint16_t emergency_bonus = 0;
    bool emergency_pending = false;
    // The emergency_control entries taken (gamevar.emergency_used).
    std::array<uint8_t, 20> emergency_used{};
    // The offers made to the player (gamevar.emergency_calls): the call, its
    // terms, and whether the slot is used; and the offers outstanding
    // (gamevar.emergencies_active). Only accepting one frees its slot and
    // counts it off; an offer refused or expired keeps both, as in the game,
    // so that after four of those no emergency is offered again.
    struct Offer {
        uint16_t call = 0;
        uint16_t bonus = 0;
        uint8_t used = 0;
        uint8_t illness = 0;
        uint8_t count = 0;
        uint8_t win_percent = 0;
    };
    std::array<Offer, 4> offers{};
    uint8_t emergencies_active = 0;
    // The id of the emergency call last taken off the panel to be answered
    // (popup_store[0][2].id, save_popup), which accepting takes on.
    uint16_t popup_call = 0;
    // The next earthquake (gamevar.next_quake_turn and on) and the
    // quake_control entries taken.
    uint32_t next_quake_turn = 0;
    uint8_t quake_severity = 0;
    bool quake_pending = false;
    std::array<uint8_t, 16> quake_used{};
};

// Helicopter states (the game's HELI_*): coming down, waiting landed while
// the patients leave it, going up and away.
constexpr uint8_t kHeliLands = 2;
constexpr uint8_t kHeliWaits = 4;
constexpr uint8_t kHeliLeaves = 6;

// The player calls about an emergency: the offer, and its result
// (subject 1).
constexpr uint16_t kCallEmergency = 2;
constexpr uint16_t kCallEmergencyOver = 5;

// The fax answers to an emergency offer (EV_CHOICE_RESULT values, the
// game's CH_EMERGENCY_REFUSE and CH_EMERGENCY_ACCEPT).
constexpr uint16_t kChoiceEmergencyRefuse = 0x1e;
constexpr uint16_t kChoiceEmergencyAccept = 0x1f;

// setup_next_emergency (0xaf2c0): schedules the first emergency_control
// entry not yet taken that ends after the month of `date`: its turn (1500 a
// month, drawn from world.seed within its months), illness, patients (drawn
// between its Min and Max), and terms. Taken entries that end too soon are
// passed over; with none left, none is pending.
void setup_next_emergency(Disasters &d, World &world, const Balance &settings, const Date &date);

// setup_next_quake (0x43d88): likewise the next quake_control entry, its
// turn drawn from world.seed.
void setup_next_quake(Disasters &d, World &world, const Balance &settings, const Date &date);

// try_to_generate_emergency (0xaeec0), daily (daily_admin): an emergency
// accepted and counted down to state 5 gets its helicopter (summon_vehicle:
// a thing landing a row below World::Emergency::heli_pad_tile, with its
// shadow; state 3, or 0 with no free thing). Then, once the scheduled
// emergency is due (check_emergency_criteria: the hospital open, no
// epidemic call waiting, no emergency of its own, fewer than four offers
// outstanding), it is offered by a call (kCallEmergency, expiring after 20
// days; setup_emergency_stats) if hospital 0 has discovered its illness,
// and the next one is scheduled. Last, an accepted emergency's state above
// 5 falls by one. Epidemics are not ported.
void try_to_generate_emergency(Game &game);

// daily_admin's end of an emergency (src/finance.cpp): once its time is up
// (state 2) and the last statistics pass found none of its patients
// indoors, it is over (state 1): with at least its win percentage of the
// patients cured, the bonus per patient cured is paid (kTxEmergencyBonus)
// and the reputation rises (kRepEmergencySuccess), else it falls by 4% of
// the hospital's events per patient not cured (kRepEmergencyFailed); then
// the result is announced by a call (kCallEmergencyOver, subject 1,
// expiring after 20 days, which ends the emergency's state as answering it
// does). An emergency of no patients counts as failed.
void daily_admin_emergency(Game &game);

// twice_daily_admin's countdown of an emergency (src/finance.cpp): the
// timer falls by one; at 0 the emergency's state falls by one (from 3, the
// helicopter sent, to 2: time is up). Plays no clock.
void twice_daily_admin_emergency(World &world);

// create_a_peep's emergency patient (src/peepmake.cpp), while hospital 0's
// helicopter waits landed with patients left: made on the helicopter pad
// (make_a_peep) knowing its illness, quoted its treatment's price or, if
// it will not pay, given none, and sent straight for treatment
// (PS_GO_FOR_TREATMENT, kPfEmergency), counted a visitor; the helicopter
// leaves after the last patient, made or not. Status icons are not ported.
void create_a_peep_emergency(World &world, ArrivalState &state, const PeepSetup &setup);

// modify_person's emergency patient (src/environ.cpp): for patient `id`
// with kPfEmergency, once its emergency's time is up (state 2) it loses
// all its health (change_health) and is an emergency patient no longer;
// else only the health held back while in a room is applied (change_health
// of 0). Returns whether it was an emergency patient, which modify_person
// then leaves at that.
bool modify_person_emergency(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

// One turn of helicopter `id` (process_things' THING_HELICOPTER): landing
// (helicopter_lands) it sinks by its speed a turn, its animation running,
// until it waits landed (kHeliWaits), setting World::Emergency::heli_landed;
// waiting it animates; leaving (helicopter_leaves) it rises to its height
// at summoning, then it and its shadow are removed and the emergency's
// helicopter cleared. The saucer that abducts (kind 1) is not ported.
void helicopter_turn(World &world, const AnimData &anims, uint16_t id);

// The player's commands about emergencies (process_turn, src/io.cpp):
// - EV_ANSWER_PLAYERCALL kCallEmergency: the head emergency offer, if
//   any, is taken off the panel to be answered (save_popup) and its fax
//   shown (Game::choice_screen kFaxEmergency);
// - EV_CHOICE_RESULT kChoiceEmergencyAccept: the offer taken off the panel
//   becomes hospital 0's emergency (give_emergency_stats: its illness,
//   patients, bonus and win percentage; the offer's slot freed), which
//   starts as EV_START_EMERGENCY does; nothing happens if the offer is not
//   found (the game reads past its table). Nothing stops an emergency
//   accepted while another runs (the game's fax disables the button);
// - EV_CHOICE_RESULT kChoiceEmergencyRefuse: the offer is dropped;
// - EV_START_EMERGENCY: hospital 0's emergency starts (state 10, timer 100,
//   none cured);
// - EV_ANSWER_PLAYERCALL kCallEmergencyOver: the first call of that kind
//   in the calls table is answered (get_playercall); for the emergency's result (subject 1)
//   the emergency's state ends (0) and its fax is shown (kFaxEmergencyOver,
//   closing itself after 400 turns).
// Returns whether `event` was one of these.
bool apply_emergency_event(Game &game, const Event &event);

} // namespace th
