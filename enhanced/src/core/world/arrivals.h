#pragma once

#include "core/data/anims.h"
#include "core/data/balance.h"
#include "core/world/world.h"

#include <array>
#include <cstdint>
#include <source_location>

namespace th
{

// Patients arriving, as the game's src/peepmake.cpp, single player only.
//
// The game has four hospitals: the player's (0) and three computer players
// whose patients never reach the map (process_computer_peep, src/computer.cpp,
// which is not ported). Their patients are still made (drawing what the
// game draws for them) and then dropped, but process_computer_peep's own
// draws are missing, so the random generator leaves the game's once a rival
// gets a patient. Emergencies are not made.

// What who_owns_peep weighs for one hospital, per illness (the game's
// allhosps[].rooms[]), and whether it takes patients. Hospital 0's is the
// player's: the casebook shows and sets its prices, reception and the
// rooms charge them.
struct HospitalMarket {
    // allhosps.hospital_open, for the rivals only: hospital 0's is
    // create_a_peep's argument. The game opens a rival when its computer
    // player's script says so (computer_decision); the caller sets it.
    bool open = false;
    // rooms[].reputation, 0-1000, and the totals it is evaluated from
    // (rooms[].rep_points and rep_events; core/world/reputation.h).
    std::array<int32_t, 47> reputation{};
    std::array<int32_t, 47> rep_points{};
    std::array<int32_t, 47> rep_events{};
    // allhosps.reputation, rep_points and rep_events: the hospital's as a
    // whole, which prices general practice and diagnoses.
    int32_t hospital_reputation = 500;
    int32_t hospital_rep_points = 20;
    int32_t hospital_rep_events = 20;
    // rooms[].price and rooms[].price_scale (hundredths of a percent of the
    // most patients will pay, 5000-20000).
    std::array<int32_t, 47> price{};
    std::array<int32_t, 47> price_scale{};
    // allhosps.visitors and month_visitors: patients won, in all and this month.
    uint32_t visitors = 0;
    uint32_t month_visitors = 0;
};

// The arrivals' state: the game's gamevar and othervar fields that
// src/peepmake.cpp reads and writes, and the hospitals it weighs.
struct ArrivalState {
    // Patients a month and the monthly change (calc_visitors_this_month).
    uint16_t visitors_per_month = 0;
    uint16_t visitor_change = 0;
    // Patients come in the first 25 turns of every peep_interval turns; 0 none.
    int16_t peep_interval = 0;
    // Patients made for any hospital.
    uint16_t peeps_arrived = 0;
    // who_owns_peep's round robin.
    uint16_t peep_owner_counter = 0;
    // Months since the level started (othervar.game_month: year * 12 + month),
    // kept by the caller.
    uint16_t game_month = 0;
    std::array<HospitalMarket, 4> hospitals{};
};

// Sets up `state` for a level as setup_new_level and init_player
// (src/start.cpp): cleared, each hospital's prices at the illnesses' start
// prices (price_scale 100%) and reputations at 500 (20 points over 20
// events), rivals open when the level has no opening countdown
// (gbv.open_countdown 0), then calc_visitors_this_month.
void start_arrivals(ArrivalState &state, const Balance &settings);

// max_price_for_treatment (0x58e34) in single player: the most patients
// will pay hospital `hosp` for `treatment` (0-46): its start price scaled
// by the reputation for it over 500 (the hospital's as a whole for general
// practice and from treatment 0x24 on), never below the start price.
int32_t max_price_for_treatment(const HospitalMarket &hosp, const Balance &settings, int treatment);

// update_single_cure_cost (0x143e0): moves hospital `hosp`'s price for
// `treatment` (0-46) to its share (price_scale) of the most patients will
// pay, if that moves it by 1% or more.
void update_single_cure_cost(HospitalMarket &hosp, const Balance &settings, int treatment);

// update_cure_costs (0x1438c): update_single_cure_cost for every hospital
// and treatment.
void update_cure_costs(ArrivalState &state, const Balance &settings);

// calc_visitors_this_month (0x6ff30): takes the month's change from the
// popn entry for state.game_month, if any (else keeps the last), adds it to
// visitors_per_month and sets peep_interval to 1500 / visitors_per_month.
// The game calls it when a level starts and in month_end, before
// game_month moves on to the new month.
void calc_visitors_this_month(ArrivalState &state, const Balance &settings);

// old_normal_rand (0xb0144): a number from `low` to `high` (high - 1 when
// high - low is odd), the sum of two draws from `seed`, traced as the
// caller's.
uint32_t old_normal_rand(uint32_t &seed, uint32_t low, uint32_t high,
                         std::source_location caller = std::source_location::current());

// create_an_illness (0x702c4): a new patient's illness drawn from `seed` by
// the balance's weights (Balance::visuals and non_visuals as balance_control
// leaves them): a visual one (2-15) with odds of 1 in the town's ill_rate,
// unless held back early in the level or not yet available, else a
// non-visual one (16-35). `level_number` is the campaign level.
uint8_t create_an_illness(uint32_t &seed, const ArrivalState &state, const Balance &settings, int level_number);

// who_owns_peep (0x70458): the hospital (0-3) that wins a new patient with
// `illness` (2-35): in turn while game_month is below gbv.alloc_delay, else
// one at random with odds of 1 in gbv.alloc_rand, else drawn by each
// hospital's reputation for it times gbv.alloc_ind_rep, scaled by its
// price against the most patients will pay (50-150%). The weighing first
// updates each hospital's price (update_single_cure_cost). Returns -1 if the
// winner does not take patients (hospital 0 when `hospital_open` is false).
int who_owns_peep(uint32_t &seed, ArrivalState &state, const Balance &settings, uint8_t illness, bool hospital_open);

// transform_peep_who (0x16b30) for a patient: their kind changed by
// kChangePersonWho row `change` (the old one kept in orig_subtype), if the
// row has one for them.
void transform_peep_who(Thing &person, uint8_t change);

// make_a_peep (0x70918): makes a patient of hospital `owner` with `illness`
// on cell `pos` (column + row * 128): looks to suit the illness, its
// effects on them (effect_illness), contagion, health, insurance and cash,
// standing at the cell's middle on the first frame of its walk facing 0,
// heading for its own cell. Hospital 0's is put on the map (nav_init,
// insert_node); a rival's is not. Takes the first free thing but the last
// 50. Returns its id, or 0 if none was free.
uint16_t make_a_peep(World &world, const ArrivalState &state, const Balance &settings, const AnimData &anims,
                     int level_number, uint16_t pos, uint8_t illness, uint8_t owner);

// create_a_peep (0x6ffec) without emergencies: in the first 25 turns of
// every peep_interval turns (by `game_turn`, the game's actual_game_turn),
// makes a patient with a random illness for the hospital that wins them.
// Hospital 0's stands at one of its four entrances (World::entrances[0],
// picked at random) and is returned, to be set going; a rival's is made and
// dropped (as remove_thing, which also clears the list of cell (0, 0), the
// position it never was put at). Returns 0 when no patient reaches the map.
// Nothing comes on rat levels.
// The game calls it from twice_daily_admin, on turns that are multiples of
// 25, after the turn's three draws and before daily_admin and month_end.
uint16_t create_a_peep(World &world, ArrivalState &state, const Balance &settings, const AnimData &anims,
                       int level_number, bool hospital_open, uint32_t game_turn);

} // namespace th
