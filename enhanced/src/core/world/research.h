#pragma once

#include "core/world/object_tables.h"

#include <array>
#include <cstdint>

namespace th
{

struct World;
struct Thing;
struct Date;

// Research (src/research.cpp, src/drugs.cpp): the points researchers earn,
// spent each day on what the player's allocation names, and what they buy:
// new machines, drugs, better drugs and machine upgrades. Only hospital 0
// (the player's) is kept; the rivals earn no points, as their researchers
// (the computer players) are not ported.

// The research categories, research_alloc's indices (the game's RC_*): new
// treatment machines, new diagnosis machines, drugs, improvements and the
// illness concentrated on (gamevar.research_illness).
constexpr int kResearchCategories = 5;
constexpr uint8_t kRcCure = 0;
constexpr uint8_t kRcDiagnosis = 1;
constexpr uint8_t kRcDrugs = 2;
constexpr uint8_t kRcImprovements = 3;
constexpr uint8_t kRcSpecialisation = 4;

// The advisor's announcements (gamevar.announce's kinds, the game's ANN_*)
// research makes.
constexpr uint8_t kAnnAutopsyDiscovered = 2;
constexpr uint8_t kAnnCategoryDone = 3;
constexpr uint8_t kAnnObjectResearched = 5;
constexpr uint8_t kAnnNewDrug = 6;
constexpr uint8_t kAnnDrugImproved = 7;
constexpr uint8_t kAnnObjectImproved = 8;
constexpr uint8_t kAnnObjectAvailable = 9;
// gamevar.announce's kinds (ANN_STUCK 1 to ANN_DOCTOR_TO_CONSULTANT 0xd,
// entry 0 unused).
constexpr int kAnnounceKinds = 14;

// One object kind for a hospital (the game's ObjectStats, allhosps[].object_stats,
// with the Equipment bytes it shares): its price, the month it becomes
// available by itself (year * 12 + month, 0 for none), the strength new ones
// get, whether it can be bought (equipment[].available) and its research
// upgrades (equipment[].upgrade_level[0], +2 each, up to 40).
struct ObjectStats {
    int32_t cost = 0;
    uint32_t available = 0;
    uint16_t strength = 0;
    uint8_t equip_available = 0;
    uint8_t upgrade_level = 0;
};

// Hospital 0's research (allhosps[0]'s fields and award_counts').
struct Research {
    // Points earned and not yet spent (research_points).
    int32_t points = 0;
    // Points banked per illness (illness_research): a machine's are its first
    // illness's.
    std::array<int32_t, 47> illness_research{};
    // Per category, the cost of what it last worked towards (research_cost),
    // which the research screen's bars measure against; 0 until then.
    std::array<int32_t, kResearchCategories> cost{};
    // Per object kind (object_stats).
    std::array<ObjectStats, kObjectKinds> objects{};
    // The points earned and spent since the level started
    // (award_counts.research_earned, research_spent).
    int32_t earned = 0;
    int32_t spent = 0;
    // The categories that ran out since the interface last looked
    // (othervar.research_exhausted); each plays SFX_RESEARCH_EXHAUSTED.
    uint8_t exhausted = 0;
};

// init_player's objects (0x910a0 in src/start.cpp) for hospital 0: each kind's
// price and strength from the level (StartCost, StartStrength) and, if the
// level has it, whether it can be bought from the start (StartAvail) and the
// month it becomes so (WhenAvail). No points, nothing banked.
void start_research(World &world);

// announce_this (0x60b54 in src/advisor.cpp): queues `value` for the
// advisor's announcement of `kind` (World::announce), up to four waiting,
// once each.
void announce_this(World &world, uint8_t kind, uint32_t value);

// done_announcement (0x60bf4 in src/advisor.cpp): drops the oldest
// announcement of `kind`, moving the rest down.
void done_announcement(World &world, uint8_t kind);

// The player can buy objects of `kind`: researched or available from the
// start (equipment[].available), and in the level (AvailableForLevel).
bool equipment_available(const World &world, int kind);

// The price hospital 0 pays for a new object of `kind`
// (object_stats[].cost), and the strength it gets (object_stats[].strength).
int32_t object_cost(const World &world, int kind);
uint16_t object_strength(const World &world, int kind);

// do_research (0x6cf80), daily: hospital 0's points shared out by the
// policy's allocation (World::hospital's research_alloc); each category's
// share goes to its cheapest open item (choose_next_research_illness), the
// specialisation's to the illness concentrated on. A category with nothing
// left keeps its share; cures and diagnosis are then closed
// (research_category_exhausted).
void do_research(World &world);

// auto_available (0x6d8fc), at month end: each object kind whose month has
// come (object_stats[].available) can be bought, announced.
void auto_available(World &world, const Date &date);

// research_contribution (0x6da40): researcher `staff`'s work, `amount` plus
// their ability (a half when tired, a quarter when very tired), added to
// their hospital's points; a crazy doctor takes away half of it instead.
// Points never go below 0.
void research_contribution(World &world, const Thing &staff, int16_t amount);

// The functions do_research works through, kept for the research screen and
// tests. Each cost is -1 when the illness has nothing of that kind to
// research for hospital 0:
//   find_new_cure_cost (0x6e1c8)      its treatment machine, not yet bought;
//   find_new_diag_cost (0x6e27c)      its diagnosis machine, not yet bought;
//   find_new_drug_cost (0x6e330)      its drug, known and with no pharmacy;
//   find_drug_improve_cost (0x6e3e0)  its drug's next improvement;
//   find_hw_improve_cost (0x6e4dc)    its machine's next upgrade.
enum class ResearchCost : uint8_t { NewCure, NewDiag, NewDrug, DrugImprove, HwImprove };
int32_t research_cost(const World &world, ResearchCost kind, uint8_t illness);

// choose_next_research_illness (0x6e624): the illness (0-0x2e) with the
// lowest positive cost of `kind`, the highest numbered of those tied; 0 if
// none has one.
uint8_t choose_next_research_illness(const World &world, ResearchCost kind);

// do_illness_research (0x6e720): `amount` points for `illness` in
// `category` (a machine's go to the first illness using it), and once its
// cost is banked, the next thing for it: the machine (item_researched), the
// drug (drug_researched), a better drug (drug_improved) or a machine
// upgrade (item_upgraded).
void do_illness_research(World &world, uint8_t illness, uint32_t amount, uint8_t category);

// emptiest_research_room (0x6dc64): hospital `owner`'s research room with
// the most places to work (research computers, atom analysers, autopsy
// machines, desks and filing cabinets) less the staff in it, the first of
// those tied; 0 if none.
uint16_t emptiest_research_room(const World &world, uint8_t owner);

// find_rand_research_room (0x6ddf0): one of hospital `owner`'s research
// rooms but `exclude` (among the first 30), drawn from world.seed; 0 if
// none, without a draw.
uint16_t find_rand_research_room(World &world, uint8_t owner, uint16_t exclude);

// person_autopsied (0x6dfe8): patient `patient` autopsied. Its illness gains
// gbv.autopsy_rsch_percent of its research cost (do_illness_research,
// kRcSpecialisation). Then a draw from world.seed: hospital 0's reputation
// falls (kRepAutopsy) and the advisor announces it (kAnnAutopsyDiscovered)
// with a chance of 2 in 3 when its staff's average happiness (of the staff
// not applicants, in 1/256ths) is under 25, else 1 in 3.
void person_autopsied(World &world, uint16_t patient);

} // namespace th
