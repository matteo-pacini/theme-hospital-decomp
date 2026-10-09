#pragma once

#include "core/data/balance.h"

#include <array>
#include <cstdint>

namespace th
{

class LanguageText;
struct ArrivalState;
struct HospitalMarket;
struct Thing;
struct Game;
struct World;
enum RepEvent : uint8_t;

// The computer players' hospitals, as the game's src/computer.cpp, src/comtable.cpp
// and init_player (src/start.cpp) set and use them. A computer player has no
// map, rooms or staff things: its rooms, objects and staff are counts, and
// its patients are simulated at once (process_computer_peep). Its
// reputation, prices and whether it takes patients are in the hospital's
// HospitalMarket (core/world/arrivals.h).

// Hospitals the game has, and the objects and illnesses a hospital counts.
inline constexpr int kHospitalCount = 4;
inline constexpr int kRivalObjects = 62;
inline constexpr int kRivalIllnesses = 47;

// A step of a computer player's script (com_strategy), its pending action
// (allhosps.action) or a postponed one (allhosps.notes).
struct ComputerStrategy {
    // 0 decide again, 1 employ, 2 build, 3 place objects, 4 loan; other
    // values act at once in computer_decision.
    uint16_t type = 0;
    // The staff kind, room, object or amount the action applies to.
    int16_t arg = 0;
    // How many to place, or the skills to employ with.
    int16_t count = 0;
};

// Turns to wait after an action of a type, times the pace / 10
// (com_decide).
struct ComputerDecision {
    uint16_t delay = 0;
};

// One month of a computer player's simulated results (comp_buckets[owner][month]),
// added to its hospital by get_computer_bucket.
struct ComputerBucket {
    // Money taken.
    int32_t income = 0;
    // Reputation points and events from reputation_affected.
    int32_t rep_points = 0;
    int32_t rep_events = 0;
    int32_t cures = 0;
    int32_t deaths = 0;
    // Patients whose visit was simulated; taken off the hospital's patient load.
    int32_t patients_done = 0;
    // rep_points and rep_events per treatment.
    std::array<int32_t, kRivalIllnesses> room_rep_points{};
    std::array<int32_t, kRivalIllnesses> room_rep_events{};
};

// An object kind for a computer player (allhosps.object_stats): its price and
// how many it has bought (inc_hardware_stats).
struct RivalObjectStats {
    int32_t cost = 0;
    uint16_t count = 0;
};

// One computer player's hospital: the fields of allhosps[owner] and
// listhead[owner] that init_player, setup_computer_players and
// src/computer.cpp read or write for a hospital whose player_type is 2.
// World::rivals holds one per owner, indexed by the game's owner number (1-3;
// 0 is the player's and unused).
struct RivalHospital {
    // The turn of the next action (next_action_turn), in game turns.
    uint32_t next_action_turn = 0;
    // allhosps.balance, loan, max_loan and wage_bill (paid each month).
    int32_t balance = 0;
    int32_t loan = 0;
    int32_t max_loan = 0;
    int32_t wage_bill = 0;
    int32_t hospital_value = 0;
    // The price of the objects it has bought (inc_hardware_stats).
    int32_t object_value = 0;
    int32_t year_start_balance = 0;
    int32_t year_start_visitors = 0;
    int32_t score = 0;
    int32_t start_score = 0;
    // Simulated patients arrived and not yet treated; sizes its staff and
    // objects.
    int32_t patient_load = 0;
    // allhosps.rooms_built: rooms built by computer_build.
    uint32_t rooms_built = 0;
    // The pending action, kept whole: computer_staff_levels sets its type and
    // arg only, and a later action reads the count left behind.
    ComputerStrategy action;
    // The postponed actions (allhosps.notes); type 0 marks a free slot.
    std::array<ComputerStrategy, 8> notes{};
    // 1 when the pending action comes from a note.
    uint16_t action_from_note = 0;
    // The com_strategy index of the next step, and where
    // computer_recheck_actions resumes (0 none).
    int16_t strategy_step = 0;
    int16_t recheck_step = 0;
    // Share of the month's patients (percent).
    uint16_t market_share = 0;
    // allhosps.area: tiles of land owned.
    uint16_t area = 0;
    uint16_t cures = 0;
    uint16_t deaths = 0;
    // Staff hired, by kind (staff_by_kind: nurse, doctor, handyman,
    // receptionist), surgeons among the doctors, and in all.
    uint16_t staff_count = 0;
    std::array<int16_t, 4> staff_by_kind{};
    int16_t surgeon_count = 0;
    // Sum of the abilities of the staff hired, by staff kind (the game
    // indexes it by Thing.subtype, 0-7).
    std::array<int16_t, 8> staff_ability_sum{};
    // listhead.staff_by_skill: staff with each skill bit.
    std::array<uint16_t, 16> staff_by_skill{};
    // listhead.rooms: rooms built of each type (a count, unlike the player's
    // room index). Entry 30 is read by com_room_orders; the game shares it with
    // first_call[0], which nothing reads for a computer player.
    std::array<uint16_t, 31> rooms{};
    // listhead.objects: objects placed of each kind (a count).
    std::array<uint16_t, kRivalObjects> objects{};
    // inc_hardware_stats' counts.
    uint16_t extinguisher_count = 0;
    uint16_t radiator_count = 0;
    uint16_t amenity_count = 0;
    uint16_t leisure_count = 0;
    uint8_t object_count = 0;
    uint8_t machine_count = 0;
    // allhosps.heating_level (month_end's heating bill).
    uint8_t heating_level = 0;
    std::array<RivalObjectStats, kRivalObjects> object_stats{};
    // rooms[].drug_cost, as init_player sets it.
    std::array<uint16_t, kRivalIllnesses> drug_cost{};
    // rooms[].cure_available (1 when it can treat the illness; [0] always) and
    // discovered (1 once the illness is known to it).
    std::array<uint8_t, kRivalIllnesses> cure_available{};
    std::array<uint8_t, kRivalIllnesses> discovered{};
    // rooms[].drug_effectiveness: the chance (percent) a drug cures, which
    // never improves for a computer player.
    std::array<uint8_t, kRivalIllnesses> drug_effectiveness{};
    // allhosps.win_progress.
    uint8_t win_progress = 0;
    // 0 unused, 2 a computer player (allhosps.player_type).
    uint8_t player_type = 0;
    // Its entry in Balance::computer, and the script (com_strategy) it follows.
    uint8_t ai_index = 0;
    uint8_t strategy = 0;
    // allhosps.name, its computer_names entry, NUL-terminated.
    std::array<char, 0x29> name{};
    // The results simulated for each month of the year, merged into the
    // hospital by get_computer_bucket.
    std::array<ComputerBucket, 12> buckets{};
};

// com_strategy (src/comtable.cpp): the scripts computer players follow, as
// {type, arg, count}. Strategy 1 starts at the entry holding {5, 1}, strategy 2 at
// {5, 2}, and type 9 marks the end of the table. The types: 1 employ staff
// kind arg, 2 build room arg, 3 place count objects of kind arg, 4 loan,
// 5 start of strategy arg, 6 switch to strategy 2, 7 choose a room, 8
// recheck the hires (arg 0 or 1), 10 open the hospital, 11 staff levels, 12
// object levels, 13 buy land, 14 maintain the loan, 15 restart the strategy, 16
// switch to strategy 2, 17 free build.
inline constexpr std::array<ComputerStrategy, 18> kComStrategy{{
    {5, 1, 0},
    {1, 3, 0},
    {3, 11, 1},
    {3, 4, 4},
    {8, 0, 0},
    {2, 7, 0},
    {10, 0, 0},
    {1, 1, 0},
    {7, 0, 0},
    {8, 1, 0},
    {6, 0, 0},
    {5, 2, 0},
    {11, 0, 0},
    {17, 0, 0},
    {12, 0, 0},
    {14, 0, 0},
    {15, 0, 0},
    {9, 0, 0},
}};

// com_decide (src/comtable.cpp): the delay after an action, by the action's
// type (0 employ or build then decide again; 1 employ; 2 build; 3 place
// objects; 4 loan; 5 none).
inline constexpr std::array<ComputerDecision, 6> kComDecide{{{33}, {25}, {37}, {10}, {50}, {0}}};

// com_room_orders (src/comtable.cpp): the rooms a computer player builds in
// turn, [order][0] their number then their room types. Order 2 is never
// chosen.
inline constexpr std::array<std::array<uint8_t, 14>, 3> kComRoomOrders{{
    {8, 27, 16, 12, 13, 8, 15, 14, 9, 0, 0, 0, 0, 0},
    {12, 11, 8, 9, 10, 17, 23, 20, 21, 19, 24, 30, 18, 0},
    {4, 25, 28, 29, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0},
}};

// staff_max_nbs (src/comtable.cpp): per staff kind (nurse, doctor, handyman,
// receptionist), the patient load per member of staff its StaffLevels column
// (0-9) calls for.
inline constexpr std::array<std::array<uint16_t, 10>, 4> kStaffMaxNbs{{
    {55, 45, 36, 28, 21, 15, 10, 6, 3, 1},
    {55, 45, 36, 28, 21, 15, 10, 6, 3, 1},
    {10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
    {40, 36, 32, 28, 24, 20, 16, 12, 8, 4},
}};

// object_nbs (src/comtable.cpp): per comfort object (drinks machine, bench,
// plant, fire extinguisher), [i][0] its object kind then the patient load per
// object its Comfort column (0-9, at index + 1) calls for.
inline constexpr std::array<std::array<uint16_t, 11>, 4> kObjectNbs{{
    {7, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
    {4, 5, 4, 4, 3, 3, 2, 2, 2, 1, 1},
    {45, 32, 28, 24, 20, 16, 10, 7, 4, 2, 1},
    {43, 32, 28, 24, 20, 16, 10, 7, 4, 2, 1},
}};

// ---- Foundation: tables, level start, setup ----

// find_strategy_start (0x502e0): the kComStrategy index just after the start
// marker of strategy `type`, or 0 if the table ends first.
uint32_t find_strategy_start(uint8_t type);

// init_player (src/start.cpp) for a computer player's hospital: `rival` is
// reset, then given the level's starting cash, loan limit, scores and market
// share, the illnesses it knows, its drugs' start figures and the objects'
// prices. `start_cash` is the level's towns[].start_cash. Its reputation, prices
// and opening are HospitalMarket's (start_arrivals).
void init_rival(RivalHospital &rival, const Balance &settings, int32_t start_cash);

// init_player for hospitals 1-3 of `world` at a level start: World::rivals is
// cleared and each computer player's hospital given init_rival's figures.
void start_rivals(World &world, const Balance &settings, int32_t start_cash);

// setup_computer_players (0x4e760), single player: gives hospitals 1-3 the
// computer players Balance::computer marks as playing, in order (their
// ai_index, strategy, the start of the script and the land `area` of the
// player), and then, if fewer than three play, random unused ones, drawing
// from World::seed (as old_newrand) and `mouse_x + mouse_y`. A rat level
// (towns[].special_kind 1) has none. The names are left empty for
// name_computer_players, but for a random pick that finds no free entry in
// 1000 draws, which is named "DEF 64".
void setup_computer_players(World &world, const Balance &settings, int level_number, uint16_t area, int mouse_x,
                            int mouse_y);

// Names the computer players of `rivals` by their ai_index from the language's
// computer names (text section 4: computer_names[ai_index]), unless
// setup_computer_players named them already.
void name_computer_players(std::array<RivalHospital, kHospitalCount> &rivals, const LanguageText &text);

// ---- Money and reputation (computer_month.cpp) ----

// reputation_affected (0x58370) for a computer player: banks `event` in
// `rival`.buckets[month % 12] (the treatment's entry too, unless `treatment`
// is 0, the hospital as a whole) for get_computer_bucket to merge, and moves
// the price for `treatment` at once (update_single_cure_cost on `market`,
// the rival's HospitalMarket). `month` is the month the visit is simulated
// in. The event's `amount` is 0: a computer player gets none that scale.
// Emits no trace event.
void rival_reputation_affected(RivalHospital &rival, HospitalMarket &market, const Balance &settings, uint8_t month,
                               uint8_t treatment, RepEvent event);

// get_computer_bucket (0x4f900): merges owner's bucket for the current month
// (arrivals.game_month % 12) into its hospital, takes the visits done off
// its patient load, re-evaluates its reputations, and empties the bucket.
// Emits a computer_bucket trace event.
void get_computer_bucket(World &world, ArrivalState &arrivals, uint8_t owner);

// computer_day (0x4ea50): every fifth `day` of the month, get_computer_bucket
// for each computer player.
void computer_day(World &world, ArrivalState &arrivals, int day);

// computer_month (0x4eae0): get_computer_bucket for each computer player, then
// its wage bill leaves the balance (no cap). Emits a computer_month trace
// event per player.
void computer_month(World &world, ArrivalState &arrivals);

// month_end (src/finance.cpp) for the computer players' hospitals: their
// heating, loan interest (at towns[0]'s rate: their level is 0) and
// overdraft interest, then their price level. `heating_price` is the
// player's Finance::heating_price.
void rival_month_end(World &world, ArrivalState &arrivals, const Balance &settings, int heating_price);

// month_end's market shares for hospitals 1-3: each one's month_visitors as a
// percent of the four hospitals', 0 when none had visitors. Run before the
// visitors are cleared.
void rival_market_share(World &world, const ArrivalState &arrivals);

// year_end's year_start_balance and year_start_visitors for hospitals 1-3.
void rival_year_start(World &world, const ArrivalState &arrivals);
// ---- Hiring and building (computer_staff.cpp) ----

// computer_employ (0x4fa50): hires for hospital `owner` the first job applicant
// of staff kind `type` (Thing.subtype) with every skill bit of `skills` (0 for
// any), paying 1.2 times their salary, and frees their thing. Returns false if
// the first such applicant costs more than the balance, or there is none.
bool computer_employ(World &world, uint8_t type, uint16_t skills, uint8_t owner);

// computer_build (0x4fc48): builds a room of type `type` for hospital `owner`
// in the abstract: pays its cost (the level's Balance::rooms) and counts it,
// and buys each compulsory object that the balance still covers. Returns false
// if the room costs more than the balance.
bool computer_build(World &world, const Balance &settings, uint8_t type, uint8_t owner);

// computer_place_object (0x4fd6c): buys object kind `obj` for hospital `owner`
// at its own price, or for nothing if `free`. Returns false if it cannot pay.
bool computer_place_object(World &world, uint8_t obj, bool free, uint8_t owner);

// set_expertise_staff (0x4a2fc) for the computer player `rival` hiring `staff`:
// the treatments their skills complete in rooms and with objects it has
// counted (two surgeons for surgery; a ward for ward treatments).
void set_expertise_staff(RivalHospital &rival, const Thing &staff);

// set_expertise_room (0x4a520) for the computer player `rival` building a room
// of type `type`: the treatments given there whose staff skills it has.
void set_expertise_room(RivalHospital &rival, uint8_t type);
// ---- Rival patients ----

// What process_computer_peep needs of a patient's arrival besides the patient.
struct RivalPeepVisit {
    // The computer player (1-3) the patient came to.
    uint8_t owner = 0;
    // The day of the month (mycontrol->day): the visit's simulated clock starts there.
    int day = 0;
    // The calendar month (mycontrol->month) the visit starts in; it can differ from the month of
    // othervar.game_month after a cheat moves the date.
    int month = 0;
    // has_drinks, which the game leaves unset when the hospital has too few
    // drinks machines, so that it holds what the stack slot did.
    bool stale_drinks = false;
};

// process_computer_peep (0x4f15c): the whole visit of `patient` to computer
// player `visit.owner`'s hospital at once: GP and diagnosis visits until the
// patient is diagnosed, leaves or dies at random, or, with no diagnosis room
// left, a guess if the diagnosis is far enough on; then the cure (or a death,
// when a drug fails its effectiveness roll). The income, cures, deaths and
// reputation changes go to the bucket of the simulated month, which moves on
// as the visit takes time. Draws from World::seed as the game does.
void process_computer_peep(World &world, ArrivalState &state, const Balance &settings, uint16_t patient,
                           const RivalPeepVisit &visit);

// computer_diag_progress (0x507c4): raises `patient`'s diagnosis by a visit
// to the room of its treatment in a hospital with `rival`'s staff: their
// average ability, a random spread (old_normal_rand, 2 draws from `seed`) and
// the room's bonus for the illness. The hospital must have staff of the kind
// the treatment needs.
void computer_diag_progress(uint32_t &seed, const RivalHospital &rival, Thing &patient);
// ---- The AI (computer_ai.cpp) ----

// What the AI reads and changes beside the rival itself: the draws' seed and
// the advisor's announcements (world), whether the rival takes patients
// (arrivals), the level's figures, the game turn (actual_game_turn) and the
// land of the player (myhosp->area).
struct AiContext {
    World &world;
    ArrivalState &arrivals;
    const Balance &settings;
    uint32_t turn;
    uint16_t player_area;
};

// computer_action (0x4eb5c), from twice_daily_admin after rat_fest_control:
// each rival whose next_action_turn has come carries out its pending action
// or takes its next step (computer_decision). Not in the tutorial.
void computer_action(Game &game);

// computer_decision (0x4eebc): takes owner's next step in its script.
void computer_decision(AiContext &ctx, uint8_t owner);

// computer_recheck_actions (0x4ff24); always returns 0.
uint8_t computer_recheck_actions(AiContext &ctx, uint8_t a, uint8_t owner);

// computer_staff_levels (0x5010c), computer_object_levels (0x50c44),
// computer_free_build (0x5090c), computer_maintain_loan (0x50aa8): return 1
// if they set an action.
uint8_t computer_staff_levels(AiContext &ctx, uint8_t owner);
uint8_t computer_object_levels(AiContext &ctx, uint8_t owner);
uint8_t computer_free_build(AiContext &ctx, uint8_t owner);
uint8_t computer_maintain_loan(AiContext &ctx, uint8_t owner);

// computer_choose_room (0x50360): returns the room type chosen, 0 for none.
uint8_t computer_choose_room(AiContext &ctx, uint8_t order, uint8_t owner);

// computer_loan (0x4fe78) and computer_buy_land (0x50de4): return 1 if done.
uint8_t computer_loan(AiContext &ctx, int8_t amount, uint8_t owner);
uint8_t computer_buy_land(AiContext &ctx, uint8_t owner);

// make_a_note (0x50528), find_a_note (0x505d0), remove_a_note (0x50684) and
// declass_a_note (0x506f4) on `rival`'s notes. make_a_note and find_a_note
// return 1 on success.
uint8_t make_a_note(RivalHospital &rival, uint8_t owner, uint16_t type, int16_t arg, int16_t count);
uint8_t find_a_note(RivalHospital &rival, uint16_t type);
void remove_a_note(RivalHospital &rival, uint8_t owner, uint16_t type);
void declass_a_note(RivalHospital &rival, uint8_t owner, uint16_t type);

} // namespace th
