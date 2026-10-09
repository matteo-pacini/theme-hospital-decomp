#include "core/data/balance.h"

#include "core/data/gamedata.h"

#include <cstdio>
#include <cstring>
#include <type_traits>

namespace th
{

namespace
{

// The game's static values (see Balance), as HOSPITAL.EXE holds them.

constexpr std::array<Balance::Expertise, 47> kExpertise{{
    {0, 0, 0, 0, 0},      {200, 0, 0, 100, 1},  {1600, 0, 0, 100, 1}, {1800, 0, 0, 100, 1}, {1600, 0, 0, 100, 1},
    {900, 0, 0, 100, 1},  {900, 0, 0, 100, 1},  {500, 0, 0, 100, 1},  {5000, 0, 0, 100, 1}, {900, 0, 0, 100, 1},
    {900, 0, 0, 100, 1},  {900, 0, 0, 100, 1},  {1300, 0, 0, 100, 1}, {900, 0, 0, 100, 1},  {2000, 0, 0, 100, 1},
    {200, 0, 0, 100, 1},  {200, 0, 0, 100, 1},  {200, 0, 0, 100, 1},  {200, 0, 0, 100, 1},  {8000, 0, 0, 100, 1},
    {8000, 0, 0, 100, 1}, {8000, 0, 0, 100, 1}, {3700, 0, 0, 100, 1}, {3700, 0, 0, 100, 1}, {800, 0, 0, 100, 1},
    {8000, 0, 0, 100, 1}, {200, 0, 0, 100, 1},  {200, 0, 0, 100, 1},  {200, 0, 0, 100, 1},  {200, 0, 0, 100, 1},
    {200, 0, 0, 100, 1},  {200, 0, 0, 100, 1},  {200, 0, 0, 100, 1},  {200, 0, 0, 100, 1},  {8000, 0, 0, 100, 1},
    {6000, 0, 0, 100, 1}, {800, 0, 0, 100, 1},  {400, 0, 0, 100, 1},  {300, 0, 0, 100, 1},  {600, 0, 0, 100, 1},
    {400, 0, 0, 100, 1},  {400, 0, 0, 100, 1},  {400, 0, 0, 100, 1},  {400, 0, 0, 100, 1},  {0, 0, 0, 100, 1},
    {0, 0, 0, 100, 0},    {0, 0, 0, 100, 0},
}};

constexpr std::array<Balance::Object, 63> kObjectSettings{{
    {-99, 0, 0, 0, 0},    {2000, 1, 0, 12, 1},  {200, 1, 0, 12, 1},   {0, 1, 0, 12, 1},     {50, 1, 0, 12, 1},
    {40, 1, 0, 12, 1},    {10, 1, 0, 12, 1},    {50, 1, 0, 12, 1},    {0, 1, 500, 12, 1},   {3000, 0, 24, 12, 1},
    {0, 1, 0, 12, 1},     {0, 1, 0, 12, 1},     {0, 1, 0, 12, 1},     {500, 1, 0, 12, 1},   {5000, 0, 24, 12, 1},
    {100, 1, 0, 12, 1},   {30, 1, 0, 12, 1},    {3000, 1, 0, 12, 1},  {300, 1, 0, 12, 1},   {250, 1, 0, 12, 1},
    {700, 1, 0, 12, 1},   {150, 1, 0, 12, 1},   {1500, 0, 24, 12, 1}, {6000, 0, 24, 12, 1}, {6000, 0, 24, 12, 1},
    {3000, 0, 24, 12, 1}, {2000, 0, 24, 12, 1}, {3000, 0, 24, 12, 1}, {2000, 1, 0, 12, 1},  {0, 1, 500, 12, 1},
    {10000, 1, 0, 12, 1}, {2000, 1, 0, 12, 1},  {200, 1, 0, 12, 1},   {0, 1, 3000, 12, 1},  {0, 1, 2000, 12, 1},
    {0, 1, 100, 12, 1},   {100, 1, 0, 12, 1},   {300, 1, 0, 12, 1},   {0, 1, 500, 12, 1},   {1000, 1, 0, 12, 1},
    {1000, 1, 0, 12, 1},  {1500, 0, 24, 12, 1}, {1000, 0, 24, 12, 1}, {50, 1, 0, 12, 1},    {20, 1, 0, 12, 1},
    {10, 1, 0, 12, 1},    {1000, 0, 24, 12, 1}, {1000, 0, 24, 12, 1}, {0, 1, 0, 12, 1},     {0, 1, 200, 12, 1},
    {200, 1, 0, 12, 1},   {200, 1, 0, 12, 1},   {0, 1, 0, 12, 1},     {0, 1, 0, 12, 1},     {5000, 0, 24, 12, 1},
    {9000, 0, 24, 12, 1}, {1000, 1, 0, 12, 1},  {450, 1, 0, 12, 1},   {1000, 1, 0, 12, 1},  {1000, 1, 0, 12, 1},
    {1000, 1, 0, 12, 1},  {1000, 1, 0, 12, 1},  {-99, 0, 0, 0, 0},
}};

constexpr std::array<Balance::Town, 16> kTowns{{
    {0, 10, 525, 450},
    {50000, 10, 525, 450},
    {50010, 20, 350, 350},
    {50020, 18, 6500, 5000},
    {50030, 24, 125, 100},
    {50040, 24, 125, 100},
    {50050, 24, 125, 100},
    {50060, 24, 125, 100},
    {50070, 24, 125, 100},
    {50080, 24, 125, 100},
    {50090, 24, 125, 100},
    {50100, 24, 125, 100},
    {50110, 24, 125, 100},
    {50110, 24, 125, 100},
    {50110, 24, 125, 100},
    {50120, 24, 125, 100},
}};

constexpr std::array<Balance::Room, 31> kRooms{{
    {0},     {0},    {0},    {0},    {0},    {0},    {0},     {2000},  {2500}, {3000}, {20000},
    {10000}, {6000}, {8000}, {7000}, {7000}, {6000}, {8000},  {20000}, {8000}, {5000}, {5000},
    {2000},  {8000}, {8000}, {1500}, {2500}, {2000}, {10000}, {2000},  {8000},
}};

constexpr std::array<Balance::Staff, 10> kStaff{{
    {150},
    {270},
    {95},
    {80},
    {500},
    {0},
    {0},
    {0},
    {0},
    {0},
}};

constexpr Balance::Gbv kGbv{
    .land_cost_per_tile = 100,
    .salary_add = {0, 0, 0, 0, 80, 150, 120, 200, 50},
    .salary_ability_divisor = 10,
    .research_points_divisor = 16,
    .start_rating = 25,
    .start_cost = 100,
    .min_drug_cost = 50,
    .how_contagious = 20,
    .reduce_cont_months = 0,
    .reduce_cont_peep_count = 0,
    .reduce_cont_rate = 100,
    .hold_visual_months = 0,
    .hold_visual_peep_count = 0,
    .rest_standing = 4,
    .rest_sofa = 10,
    .rest_game = 150,
    .rest_snooker = 260,
    .work_light = 2,
    .modify_freq = 16,
    .not_tired = 400,
    .tired = 800,
    .very_tired = 1200,
    .crack_up_tired = 3000,
    .recovery_factor = 400,
    .recovery_minimum = 5,
    .max_object_strength = 50,
    .research_increment = 5,
    .q_points = 5,
    .dist_points = 1,
    .no_staff_points = 10,
    .litter_drop = 50,
    .leave_max = 100,
    .resign_max = 100,
    .happy = 70,
    .unhappy = 25,
    .very_unhappy = 10,
    .drink_happy = 2,
    .toilet_happy = 1,
    .bowel_full = 50,
    .bowel_overflows = 75,
    .vomit_limit = 100,
    .litter_random = 5000,
    .salary_too_low = -2,
    .salary_too_high = 3,
    .max_diag_diff = 750,
    .score_max_inc = 300,
    .vac_cost = 50,
    .training_value = {10, 15, 20},
    .disaster_launch = 105,
    .epidemic_fine = 100,
    .epidemic_comp_lo = 100,
    .epidemic_comp_hi = 2000,
    .minimum_abduct_time = 1,
    .abductions_per_year = 3,
    .autopsy_rsch_percent = 34,
    .autopsy_rep_hit_percent = 40,
    .alloc_rand = 5,
    .alloc_total_rep = 5,
    .alloc_ind_rep = 10,
    .alloc_delay = 2,
    .training_rate = 25,
    .promo_doc = 6,
    .promo_con = 9,
    .doctor_threshold = 300,
    .consultant_threshold = 700,
    .drug_improve_rate = 1,
    .contagious_spread_factor = 40,
    .mayor_launch = 130,
    .ability_threshold = {150, 100, 175},
    .rsch_improve_cost_percent = 40,
    .rsch_improve_increment_percent = 5,
    .remove_rat_hole_chance = 4000,
    .open_countdown = 5000,
};

constexpr std::array<Balance::WinLose, 10> kWinCriteria{{
    {1, 1, 500, 0, 1},
    {3, 1, 75, 0, 1},
    {4, 1, 100, 0, 1},
    {6, 1, 3000000, 0, 1},
    {1, 1, 800, 0, 2},
    {2, 1, 100000, 0, 3},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
}};

constexpr std::array<Balance::WinLose, 10> kLoseCriteria{{
    {1, 0, -500, 50, 1},
    {5, 1, 100, 0, 1},
    {6, 0, 500, 10000, 1},
    {2, 0, -100000, 0, 2},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
}};

constexpr std::array<Balance::NetCriterion, 20> kNetCriteria{{
    {1, 0, 3, 12}, {2, 0, 3, 12}, {3, 0, 3, 12}, {4, 10, 3, 12}, {5, 10000, 3, 12}, {6, 50000, 3, 12}, {7, 500, 3, 12},
    {8, 0, 3, 12}, {9, 0, 3, 12}, {0, 0, 0, 0},  {0, 0, 0, 0},   {0, 0, 0, 0},      {0, 0, 0, 0},      {0, 0, 0, 0},
    {0, 0, 0, 0},  {0, 0, 0, 0},  {0, 0, 0, 0},  {0, 0, 0, 0},   {0, 0, 0, 0},      {0, 0, 0, 0},
}};

constexpr std::array<Balance::Population, 30> kPopn{{
    {0, 1}, {3, 0}, {5, 1}, {7, 0}, {10, -3}, {11, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},   {0, 0},  {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},   {0, 0},  {0, 0}, {0, 0}, {0, 0}, {0, 0},
}};

constexpr std::array<Balance::StaffLevel, 30> kStaffLevels{{
    {0, 2, 4, 4, 2, 10, 10, 10, 25, 5, 345689}, {5, 8, 8, 8, 8, 10, 10, 10, 25, 5, 356730},
    {9, 3, 3, 3, 3, 10, 10, 10, 25, 5, 563498}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
}};

constexpr std::array<Balance::Emergency, 20> kEmergencyControl{{
    {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0},
}};

constexpr std::array<Balance::Quake, 16> kQuakeControl{{
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
}};

constexpr std::array<Balance::Computer, 20> kComputer{{
    {1, 2, 4, 70, 5, 50, 1, 2, 1},  {5, 5, 1, 20, 5, 70, 1, 2, 1},  {4, 8, 2, 50, 5, 30, 1, 2, 1},
    {1, 7, 4, 30, 5, 45, 1, 2, 1},  {2, 3, 1, 40, 5, 80, 1, 2, 1},  {3, 0, 2, 100, 5, 20, 1, 2, 1},
    {3, 0, 2, 100, 5, 20, 1, 2, 1}, {3, 0, 2, 100, 5, 20, 1, 2, 1}, {3, 0, 2, 100, 5, 20, 1, 2, 1},
    {3, 0, 2, 100, 5, 20, 1, 2, 1}, {3, 0, 2, 100, 5, 20, 1, 2, 1}, {3, 0, 2, 100, 5, 20, 1, 2, 1},
    {3, 0, 2, 100, 5, 20, 1, 2, 1}, {3, 0, 2, 100, 5, 20, 1, 2, 1}, {3, 0, 2, 100, 5, 20, 1, 2, 1},
    {3, 0, 2, 100, 5, 20, 1, 2, 1}, {3, 0, 2, 100, 5, 20, 1, 2, 1}, {3, 0, 2, 100, 5, 20, 1, 2, 1},
    {3, 0, 2, 100, 5, 20, 1, 2, 1}, {3, 0, 2, 100, 5, 20, 1, 2, 1},
}};

constexpr Balance::AwardsTrophies kAwardsTrophies{
    .rat_kills_absolute = 1,
    .rat_kills_absolute_bonus = 1,
    .cans_of_coke = 1,
    .cans_of_coke_bonus = 1,
    .reputation = 1,
    .trophy_reputation_bonus = 1,
    .plant = 1,
    .plant_bonus = 1,
    .trophy_staff_happiness = 1,
    .trophy_staff_happiness_bonus = 1,
    .rat_kills_percentage = 1,
    .rat_kills_percentage_bonus = 1,
    .trophy_cures_bonus = 1,
    .trophy_death_bonus = 1,
    .trophy_mayor_bonus = 1,
    .trophy_mayor = 1,
    .cures_award = 1,
    .cures_poor = 1,
    .deaths_award = 1,
    .deaths_poor = 1,
    .population_percentage_award = 1,
    .population_percentage_poor = 1,
    .cures_v_deaths_award = 1,
    .cures_v_deaths_poor = 1,
    .reputation_award = 1,
    .reputation_poor = 1,
    .hosp_value_award = 1,
    .hosp_value_poor = 1,
    .cleanliness_award = 1,
    .cleanliness_poor = 1,
    .emergency_award = 1,
    .emergency_poor = 1,
    .staff_happiness_award = 1,
    .staff_happiness_poor = 1,
    .peep_happiness_award = 1,
    .peep_happiness_poor = 1,
    .waiting_times_award = 1,
    .waiting_times_poor = 1,
    .well_kept_tech_award = 1,
    .well_kept_tech_poor = 1,
    .cures_bonus = 1,
    .cures_penalty = 1,
    .deaths_bonus = 1,
    .deaths_penalty = 1,
    .population_percentage_bonus = 1,
    .population_percentage_penalty = 1,
    .cures_v_deaths_bonus = 1,
    .cures_v_deaths_penalty = 1,
    .award_reputation_bonus = 1,
    .award_reputation_penalty = 1,
    .hosp_value_bonus = 1,
    .hosp_value_penalty = 1,
    .cleanliness_bonus = 1,
    .cleanliness_penalty = 1,
    .emergency_bonus = 1,
    .emergency_penalty = 1,
    .award_staff_happiness_bonus = 1,
    .award_staff_happiness_penalty = 1,
    .peep_happiness_bonus = 1,
    .peep_happiness_penalty = 1,
    .waiting_times_bonus = 1,
    .waiting_times_penalty = 1,
    .well_kept_tech_bonus = 1,
    .well_kept_tech_penalty = 1,
    .new_tech_award = 1,
    .new_tech_poor = 1,
    .research_bonus = 1,
    .research_penalty = 1,
};

constexpr std::array<uint16_t, 20> kNonVisuals{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

constexpr std::array<uint16_t, 14> kVisuals{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

constexpr std::array<uint16_t, 14> kVisualsAvailable{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Object kind treating each visual illness (expertise[2..15].kind, src/array.cpp).
constexpr std::array<uint8_t, 14> kVisualTreatmentObject = {9, 46, 18, 39, 54, 26, 23, 24, 25, 39, 47, 39, 30, 39};

// One balance_variables entry. flags: the size written (1, 2 or 4 bytes; here
// the field's type) and how the bounds compare, 8 signed or 0x10 unsigned.
// `set` stores element `index`, or nothing if it is past the end of the table.
struct Variable {
    const char *name;
    uint16_t flags;
    uint16_t max_index; // inclusive, as the game checks it
    int32_t min;
    int32_t max;
    void (*set)(Balance &b, unsigned index, uint32_t value);
};

constexpr uint16_t kFlagUnsigned = 0x10;

template <auto Table, auto Member> void set_row(Balance &b, unsigned index, uint32_t value)
{
    auto &table = b.*Table;
    if (index >= table.size())
        return;
    auto &field = table[index].*Member;
    field = static_cast<std::remove_reference_t<decltype(field)>>(value);
}

template <auto Group, auto Member> void set_in(Balance &b, unsigned index, uint32_t value)
{
    if (index != 0)
        return;
    auto &field = b.*Group.*Member;
    field = static_cast<std::remove_reference_t<decltype(field)>>(value);
}

template <auto Group, auto Member> void set_in_array(Balance &b, unsigned index, uint32_t value)
{
    auto &array = b.*Group.*Member;
    if (index >= array.size())
        return;
    array[index] = static_cast<std::remove_reference_t<decltype(array[index])>>(value);
}

template <auto Table> void set_array(Balance &b, unsigned index, uint32_t value)
{
    auto &array = b.*Table;
    if (index >= array.size())
        return;
    array[index] = static_cast<std::remove_reference_t<decltype(array[index])>>(value);
}

// balance_variables, in the game's order.
constexpr Variable kVariables[] = {
    {"expertise.StartPrice", 0x12, 47, 0, 1000000, set_row<&Balance::expertise, &Balance::Expertise::start_price>},
    {"expertise.ContRate", 0x11, 47, 0, 100, set_row<&Balance::expertise, &Balance::Expertise::cont_rate>},
    {"expertise.RschReqd", 0x14, 47, 0, 1000000, set_row<&Balance::expertise, &Balance::Expertise::rsch_reqd>},
    {"expertise.MaxDiagDiff", 0x12, 47, 1, 1000, set_row<&Balance::expertise, &Balance::Expertise::max_diag_diff>},
    {"expertise.Known", 0x11, 47, 0, 1, set_row<&Balance::expertise, &Balance::Expertise::known>},
    {"objects.StartCost", 0x14, 62, 0, 1000000, set_row<&Balance::objects, &Balance::Object::start_cost>},
    {"objects.StartAvail", 0x11, 62, 0, 1, set_row<&Balance::objects, &Balance::Object::start_avail>},
    {"objects.WhenAvail", 0x12, 62, 0, 240, set_row<&Balance::objects, &Balance::Object::when_avail>},
    {"objects.StartStrength", 0x11, 62, 1, 255, set_row<&Balance::objects, &Balance::Object::start_strength>},
    {"objects.AvailableForLevel", 0x11, 62, 0, 1, set_row<&Balance::objects, &Balance::Object::available_for_level>},
    {"towns.StartCash", 0x14, 12, 0, 100000, set_row<&Balance::towns, &Balance::Town::start_cash>},
    {"towns.IllRate", 0x14, 12, 0, 100, set_row<&Balance::towns, &Balance::Town::ill_rate>},
    {"towns.InterestRate", 0x12, 12, 0, 10000, set_row<&Balance::towns, &Balance::Town::interest_rate>},
    {"towns.InflationRate", 0x12, 12, 0, 10000, set_row<&Balance::towns, &Balance::Town::inflation_rate>},
    {"rooms.Cost", 0xc, 31, 0, 50000, set_row<&Balance::rooms, &Balance::Room::cost>},
    {"staff.MinSalary", 0x12, 10, 0, 1000, set_row<&Balance::staff, &Balance::Staff::min_salary>},
    {"gbv.LandCostPerTile", 0x12, 1, 0, 10000, set_in<&Balance::gbv, &Balance::Gbv::land_cost_per_tile>},
    {"gbv.SalaryAdd", 0xa, 9, -1000, 1000, set_in_array<&Balance::gbv, &Balance::Gbv::salary_add>},
    {"gbv.SalaryAbilityDivisor", 0x12, 1, 0, 100, set_in<&Balance::gbv, &Balance::Gbv::salary_ability_divisor>},
    {"gbv.ResearchPointsDivisor", 0x12, 1, 0, 100, set_in<&Balance::gbv, &Balance::Gbv::research_points_divisor>},
    {"gbv.StartRating", 0x12, 1, 1, 100, set_in<&Balance::gbv, &Balance::Gbv::start_rating>},
    {"gbv.StartCost", 0x12, 1, 1, 10000, set_in<&Balance::gbv, &Balance::Gbv::start_cost>},
    {"gbv.MinDrugCost", 0x12, 1, 1, 5000, set_in<&Balance::gbv, &Balance::Gbv::min_drug_cost>},
    {"gbv.HowContagious", 0x12, 1, 1, 1000, set_in<&Balance::gbv, &Balance::Gbv::how_contagious>},
    {"gbv.ReduceContMonths", 0x12, 1, 0, 1000, set_in<&Balance::gbv, &Balance::Gbv::reduce_cont_months>},
    {"gbv.ReduceContPeepCount", 0x12, 1, 0, 1000, set_in<&Balance::gbv, &Balance::Gbv::reduce_cont_peep_count>},
    {"gbv.ReduceContRate", 0x12, 1, 0, 100, set_in<&Balance::gbv, &Balance::Gbv::reduce_cont_rate>},
    {"gbv.HoldVisualMonths", 0x12, 1, 0, 1000, set_in<&Balance::gbv, &Balance::Gbv::hold_visual_months>},
    {"gbv.HoldVisualPeepCount", 0x12, 1, 0, 1000, set_in<&Balance::gbv, &Balance::Gbv::hold_visual_peep_count>},
    {"gbv.RestStanding", 0x12, 1, 1, 5000, set_in<&Balance::gbv, &Balance::Gbv::rest_standing>},
    {"gbv.RestSofa", 0x12, 1, 1, 5000, set_in<&Balance::gbv, &Balance::Gbv::rest_sofa>},
    {"gbv.RestGame", 0x12, 1, 1, 5000, set_in<&Balance::gbv, &Balance::Gbv::rest_game>},
    {"gbv.RestSnooker", 0x12, 1, 1, 5000, set_in<&Balance::gbv, &Balance::Gbv::rest_snooker>},
    {"gbv.WorkLight", 0x12, 1, 1, 5000, set_in<&Balance::gbv, &Balance::Gbv::work_light>},
    {"gbv.ModifyFreq", 0x12, 1, 1, 5000, set_in<&Balance::gbv, &Balance::Gbv::modify_freq>},
    {"gbv.NotTired", 0x12, 1, 1, 10000, set_in<&Balance::gbv, &Balance::Gbv::not_tired>},
    {"gbv.Tired", 0x12, 1, 1, 10000, set_in<&Balance::gbv, &Balance::Gbv::tired>},
    {"gbv.VeryTired", 0x12, 1, 1, 10000, set_in<&Balance::gbv, &Balance::Gbv::very_tired>},
    {"gbv.CrackUpTired", 0x12, 1, 1, 10000, set_in<&Balance::gbv, &Balance::Gbv::crack_up_tired>},
    {"gbv.RecoveryFactor", 0x12, 1, 1, 10000, set_in<&Balance::gbv, &Balance::Gbv::recovery_factor>},
    {"gbv.RecoveryMinimum", 0x12, 1, 1, 10000, set_in<&Balance::gbv, &Balance::Gbv::recovery_minimum>},
    {"gbv.MaxObjectStrength", 0x12, 1, 1, 500, set_in<&Balance::gbv, &Balance::Gbv::max_object_strength>},
    {"gbv.ResearchIncrement", 0x12, 1, 1, 500, set_in<&Balance::gbv, &Balance::Gbv::research_increment>},
    {"gbv.QPoints", 0x12, 1, 0, 50, set_in<&Balance::gbv, &Balance::Gbv::q_points>},
    {"gbv.DistPoints", 0x12, 1, 0, 50, set_in<&Balance::gbv, &Balance::Gbv::dist_points>},
    {"gbv.NoStaffPoints", 0x12, 1, 0, 50, set_in<&Balance::gbv, &Balance::Gbv::no_staff_points>},
    {"gbv.LitterDrop", 0xa, 1, 0, 500, set_in<&Balance::gbv, &Balance::Gbv::litter_drop>},
    {"gbv.LeaveMax", 0xa, 1, 0, 1000, set_in<&Balance::gbv, &Balance::Gbv::leave_max>},
    {"gbv.ResignMax", 0xa, 1, 0, 1000, set_in<&Balance::gbv, &Balance::Gbv::resign_max>},
    {"gbv.Happy", 0xa, 1, 0, 100, set_in<&Balance::gbv, &Balance::Gbv::happy>},
    {"gbv.Unhappy", 0xa, 1, 0, 100, set_in<&Balance::gbv, &Balance::Gbv::unhappy>},
    {"gbv.VeryUnhappy", 0xa, 1, 0, 100, set_in<&Balance::gbv, &Balance::Gbv::very_unhappy>},
    {"gbv.DrinkHappy", 0xa, 1, 0, 50, set_in<&Balance::gbv, &Balance::Gbv::drink_happy>},
    {"gbv.ToiletHappy", 0xa, 1, 0, 50, set_in<&Balance::gbv, &Balance::Gbv::toilet_happy>},
    {"gbv.BowelFull", 0xa, 1, 0, 200, set_in<&Balance::gbv, &Balance::Gbv::bowel_full>},
    {"gbv.BowelOverflows", 0xa, 1, 0, 500, set_in<&Balance::gbv, &Balance::Gbv::bowel_overflows>},
    {"gbv.VomitLimit", 0xa, 1, 0, 500, set_in<&Balance::gbv, &Balance::Gbv::vomit_limit>},
    {"gbv.LitterRandom", 0x12, 1, 0, 60000, set_in<&Balance::gbv, &Balance::Gbv::litter_random>},
    {"gbv.SalaryTooLow", 0xa, 1, -50, 50, set_in<&Balance::gbv, &Balance::Gbv::salary_too_low>},
    {"gbv.SalaryTooHigh", 0xa, 1, 0, 50, set_in<&Balance::gbv, &Balance::Gbv::salary_too_high>},
    {"gbv.MaxDiagDiff", 0x12, 1, 1, 1000, set_in<&Balance::gbv, &Balance::Gbv::max_diag_diff>},
    {"gbv.ScoreMaxInc", 0x12, 1, 1, 10000, set_in<&Balance::gbv, &Balance::Gbv::score_max_inc>},
    {"gbv.VacCost", 0x12, 1, 1, 1000, set_in<&Balance::gbv, &Balance::Gbv::vac_cost>},
    {"gbv.TrainingValue", 0x12, 3, 1, 1000, set_in_array<&Balance::gbv, &Balance::Gbv::training_value>},
    {"gbv.DisasterLaunch", 0x12, 1, 0, 10000, set_in<&Balance::gbv, &Balance::Gbv::disaster_launch>},
    {"gbv.EpidemicFine", 0x12, 1, 1, 20000, set_in<&Balance::gbv, &Balance::Gbv::epidemic_fine>},
    {"gbv.EpidemicCompLo", 0x12, 1, 0, 10000, set_in<&Balance::gbv, &Balance::Gbv::epidemic_comp_lo>},
    {"gbv.EpidemicCompHi", 0x12, 1, 0, 20000, set_in<&Balance::gbv, &Balance::Gbv::epidemic_comp_hi>},
    {"gbv.MinimumAbductTime", 0x12, 1, 0, 500, set_in<&Balance::gbv, &Balance::Gbv::minimum_abduct_time>},
    {"gbv.AbductionsPerYear", 0x12, 1, 0, 500, set_in<&Balance::gbv, &Balance::Gbv::abductions_per_year>},
    {"gbv.AutopsyRschPercent", 0x12, 1, 0, 100, set_in<&Balance::gbv, &Balance::Gbv::autopsy_rsch_percent>},
    {"gbv.AutopsyRepHitPercent", 0x12, 1, 0, 100, set_in<&Balance::gbv, &Balance::Gbv::autopsy_rep_hit_percent>},
    {"gbv.AllocRand", 0x11, 1, 1, 255, set_in<&Balance::gbv, &Balance::Gbv::alloc_rand>},
    {"gbv.AllocTotalRep", 0x11, 1, 1, 255, set_in<&Balance::gbv, &Balance::Gbv::alloc_total_rep>},
    {"gbv.AllocIndRep", 0x11, 1, 1, 255, set_in<&Balance::gbv, &Balance::Gbv::alloc_ind_rep>},
    {"gbv.AllocDelay", 0x11, 1, 0, 255, set_in<&Balance::gbv, &Balance::Gbv::alloc_delay>},
    {"gbv.TrainingRate", 0x12, 1, 0, 1000, set_in<&Balance::gbv, &Balance::Gbv::training_rate>},
    {"gbv.PromoDoc", 0x11, 1, 1, 255, set_in<&Balance::gbv, &Balance::Gbv::promo_doc>},
    {"gbv.PromoCon", 0x11, 1, 1, 255, set_in<&Balance::gbv, &Balance::Gbv::promo_con>},
    {"gbv.DoctorThreshold", 0x12, 1, 0, 1000, set_in<&Balance::gbv, &Balance::Gbv::doctor_threshold>},
    {"gbv.ConsultantThreshold", 0x12, 1, 0, 1000, set_in<&Balance::gbv, &Balance::Gbv::consultant_threshold>},
    {"gbv.DrugImproveRate", 0x11, 1, 1, 100, set_in<&Balance::gbv, &Balance::Gbv::drug_improve_rate>},
    {"gbv.ContagiousSpreadFactor", 0x11, 1, 0, 100, set_in<&Balance::gbv, &Balance::Gbv::contagious_spread_factor>},
    {"gbv.MayorLaunch", 0x12, 1, 1, 500, set_in<&Balance::gbv, &Balance::Gbv::mayor_launch>},
    {"win_criteria.Criteria", 0x12, 10, 0, 50, set_row<&Balance::win_criteria, &Balance::WinLose::criteria>},
    {"win_criteria.MaxMin", 0x12, 10, 0, 1, set_row<&Balance::win_criteria, &Balance::WinLose::max_min>},
    {"win_criteria.Value", 0xc, 10, -9000000, 9000000, set_row<&Balance::win_criteria, &Balance::WinLose::value>},
    {"win_criteria.Bound", 0xc, 10, -9000000, 9000000, set_row<&Balance::win_criteria, &Balance::WinLose::bound>},
    {"win_criteria.Group", 0x12, 10, 0, 50, set_row<&Balance::win_criteria, &Balance::WinLose::group>},
    {"lose_criteria.Criteria", 0x12, 10, 0, 50, set_row<&Balance::lose_criteria, &Balance::WinLose::criteria>},
    {"lose_criteria.MaxMin", 0x12, 10, 0, 1, set_row<&Balance::lose_criteria, &Balance::WinLose::max_min>},
    {"lose_criteria.Value", 0xc, 10, -5000000, 5000000, set_row<&Balance::lose_criteria, &Balance::WinLose::value>},
    {"lose_criteria.Bound", 0xc, 10, -5000000, 5000000, set_row<&Balance::lose_criteria, &Balance::WinLose::bound>},
    {"lose_criteria.Group", 0x12, 10, 0, 50, set_row<&Balance::lose_criteria, &Balance::WinLose::group>},
    {"net_criteria.Criteria", 0x12, 20, 0, 50, set_row<&Balance::net_criteria, &Balance::NetCriterion::criteria>},
    {"net_criteria.Value", 0xc, 20, -5000000, 5000000, set_row<&Balance::net_criteria, &Balance::NetCriterion::value>},
    {"net_criteria.Month", 0x12, 20, 0, 120, set_row<&Balance::net_criteria, &Balance::NetCriterion::month>},
    {"net_criteria.TimeToDo", 0x12, 20, 0, 120, set_row<&Balance::net_criteria, &Balance::NetCriterion::time_to_do>},
    {"popn.Month", 0x12, 30, 0, 500, set_row<&Balance::popn, &Balance::Population::month>},
    {"popn.Change", 0xa, 30, -50, 50, set_row<&Balance::popn, &Balance::Population::change>},
    {"staff_levels.Month", 0x12, 30, 0, 500, set_row<&Balance::staff_levels, &Balance::StaffLevel::month>},
    {"staff_levels.Nurses", 0x12, 30, 0, 30, set_row<&Balance::staff_levels, &Balance::StaffLevel::nurses>},
    {"staff_levels.Doctors", 0x12, 30, 0, 30, set_row<&Balance::staff_levels, &Balance::StaffLevel::doctors>},
    {"staff_levels.Handymen", 0x12, 30, 0, 30, set_row<&Balance::staff_levels, &Balance::StaffLevel::handymen>},
    {"staff_levels.Receptionists", 0x12, 30, 0, 30,
     set_row<&Balance::staff_levels, &Balance::StaffLevel::receptionists>},
    {"staff_levels.ShrkRate", 0x11, 30, 1, 255, set_row<&Balance::staff_levels, &Balance::StaffLevel::shrk_rate>},
    {"staff_levels.SurgRate", 0x11, 30, 0, 255, set_row<&Balance::staff_levels, &Balance::StaffLevel::surg_rate>},
    {"staff_levels.RschRate", 0x11, 30, 0, 255, set_row<&Balance::staff_levels, &Balance::StaffLevel::rsch_rate>},
    {"staff_levels.ConsRate", 0x11, 30, 0, 255, set_row<&Balance::staff_levels, &Balance::StaffLevel::cons_rate>},
    {"staff_levels.JrRate", 0x11, 30, 0, 255, set_row<&Balance::staff_levels, &Balance::StaffLevel::jr_rate>},
    {"staff_levels.Seed", 0x14, 30, 0, 9999999, set_row<&Balance::staff_levels, &Balance::StaffLevel::seed>},
    {"emergency_control.StartMonth", 0x12, 20, 0, 500,
     set_row<&Balance::emergency_control, &Balance::Emergency::start_month>},
    {"emergency_control.EndMonth", 0x12, 20, 0, 500,
     set_row<&Balance::emergency_control, &Balance::Emergency::end_month>},
    {"emergency_control.Min", 0x11, 20, 0, 20, set_row<&Balance::emergency_control, &Balance::Emergency::min>},
    {"emergency_control.Max", 0x11, 20, 0, 20, set_row<&Balance::emergency_control, &Balance::Emergency::max>},
    {"emergency_control.Illness", 0x11, 20, 0, 36, set_row<&Balance::emergency_control, &Balance::Emergency::illness>},
    {"emergency_control.PercWin", 0x11, 20, 0, 100,
     set_row<&Balance::emergency_control, &Balance::Emergency::perc_win>},
    {"emergency_control.Bonus", 0x12, 20, 0, 20000, set_row<&Balance::emergency_control, &Balance::Emergency::bonus>},
    {"quake_control.StartMonth", 0x12, 16, 0, 500, set_row<&Balance::quake_control, &Balance::Quake::start_month>},
    {"quake_control.EndMonth", 0x12, 16, 0, 500, set_row<&Balance::quake_control, &Balance::Quake::end_month>},
    {"quake_control.Severity", 0x12, 16, 0, 20, set_row<&Balance::quake_control, &Balance::Quake::severity>},
    {"computer.Skill", 0x11, 20, 1, 5, set_row<&Balance::computer, &Balance::Computer::skill>},
    {"computer.StaffLevels", 0x11, 20, 0, 9, set_row<&Balance::computer, &Balance::Computer::staff_levels>},
    {"computer.Luck", 0x11, 20, 0, 4, set_row<&Balance::computer, &Balance::Computer::luck>},
    {"computer.Speed", 0x11, 20, 0, 100, set_row<&Balance::computer, &Balance::Computer::speed>},
    {"computer.Comfort", 0x11, 20, 0, 9, set_row<&Balance::computer, &Balance::Computer::comfort>},
    {"computer.GuessAt", 0x11, 20, 0, 100, set_row<&Balance::computer, &Balance::Computer::guess_at>},
    {"computer.Playing", 0x11, 20, 0, 1, set_row<&Balance::computer, &Balance::Computer::playing>},
    {"awards_trophies.RatKillsAbsolute", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::rat_kills_absolute>},
    {"awards_trophies.RatKillsAbsoluteBonus", 0x12, 1, 0, 60000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::rat_kills_absolute_bonus>},
    {"awards_trophies.CansofCoke", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cans_of_coke>},
    {"awards_trophies.CansofCokeBonus", 0x12, 1, 0, 60000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cans_of_coke_bonus>},
    {"awards_trophies.Reputation", 0xa, 1, 0, 1000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::reputation>},
    {"awards_trophies.TrophyReputationBonus", 0x12, 1, 0, 60000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::trophy_reputation_bonus>},
    {"awards_trophies.Plant", 0x11, 1, 0, 100, set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::plant>},
    {"awards_trophies.PlantBonus", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::plant_bonus>},
    {"awards_trophies.TrophyStaffHappiness", 0x11, 1, 0, 100,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::trophy_staff_happiness>},
    {"awards_trophies.TrophyStaffHappinessBonus", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::trophy_staff_happiness_bonus>},
    {"awards_trophies.RatKillsPercentage", 0x11, 1, 0, 100,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::rat_kills_percentage>},
    {"awards_trophies.RatKillsPercentageBonus", 0x12, 1, 0, 60000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::rat_kills_percentage_bonus>},
    {"awards_trophies.TrophyCuresBonus", 0x12, 1, 0, 60000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::trophy_cures_bonus>},
    {"awards_trophies.TrophyDeathBonus", 0x12, 1, 0, 60000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::trophy_death_bonus>},
    {"awards_trophies.TrophyMayorBonus", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::trophy_mayor_bonus>},
    {"awards_trophies.TrophyMayor", 0x11, 1, 0, 100,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::trophy_mayor>},
    {"awards_trophies.CuresAward", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cures_award>},
    {"awards_trophies.CuresPoor", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cures_poor>},
    {"awards_trophies.DeathsAward", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::deaths_award>},
    {"awards_trophies.DeathsPoor", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::deaths_poor>},
    {"awards_trophies.PopulationPercentageAward", 0x11, 1, 0, 100,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::population_percentage_award>},
    {"awards_trophies.PopulationPercentagePoor", 0x11, 1, 0, 100,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::population_percentage_poor>},
    {"awards_trophies.CuresVDeathsAward", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cures_v_deaths_award>},
    {"awards_trophies.CuresVDeathsPoor", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cures_v_deaths_poor>},
    {"awards_trophies.ReputationAward", 0x12, 1, 0, 60000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::reputation_award>},
    {"awards_trophies.ReputationPoor", 0x12, 1, 0, 60000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::reputation_poor>},
    {"awards_trophies.HospValueAward", 0x14, 1, 0, 2000000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::hosp_value_award>},
    {"awards_trophies.HospValuePoor", 0x14, 1, 0, 2000000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::hosp_value_poor>},
    {"awards_trophies.CleanlinessAward", 0x11, 1, 1, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cleanliness_award>},
    {"awards_trophies.CleanlinessPoor", 0x11, 1, 1, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cleanliness_poor>},
    {"awards_trophies.EmergencyAward", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::emergency_award>},
    {"awards_trophies.EmergencyPoor", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::emergency_poor>},
    {"awards_trophies.StaffHappinessAward", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::staff_happiness_award>},
    {"awards_trophies.StaffHappinessPoor", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::staff_happiness_poor>},
    {"awards_trophies.PeepHappinessAward", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::peep_happiness_award>},
    {"awards_trophies.PeepHappinessPoor", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::peep_happiness_poor>},
    {"awards_trophies.WaitingTimesAward", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::waiting_times_award>},
    {"awards_trophies.WaitingTimesPoor", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::waiting_times_poor>},
    {"awards_trophies.WellKeptTechAward", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::well_kept_tech_award>},
    {"awards_trophies.WellKeptTechPoor", 0x11, 1, 0, 255,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::well_kept_tech_poor>},
    {"awards_trophies.CuresBonus", 0xa, 1, -32000, 32000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cures_bonus>},
    {"awards_trophies.CuresPenalty", 0xa, 1, -32000, 32000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cures_penalty>},
    {"awards_trophies.DeathsBonus", 0xa, 1, -32000, 32000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::deaths_bonus>},
    {"awards_trophies.DeathsPenalty", 0xa, 1, -32000, 32000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::deaths_penalty>},
    {"awards_trophies.PopulationPercentageBonus", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::population_percentage_bonus>},
    {"awards_trophies.PopulationPercentagePenalty", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::population_percentage_penalty>},
    {"awards_trophies.CuresVDeathsBonus", 0xa, 1, -32000, 32000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cures_v_deaths_bonus>},
    {"awards_trophies.CuresVDeathsPenalty", 0xa, 1, -32000, 32000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cures_v_deaths_penalty>},
    {"awards_trophies.AwardReputationBonus", 0xa, 1, -32000, 32000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::award_reputation_bonus>},
    {"awards_trophies.AwardReputationPenalty", 0xa, 1, -32000, 32000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::award_reputation_penalty>},
    {"awards_trophies.HospValueBonus", 0xa, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::hosp_value_bonus>},
    {"awards_trophies.HospValuePenalty", 0xa, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::hosp_value_penalty>},
    {"awards_trophies.CleanlinessBonus", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cleanliness_bonus>},
    {"awards_trophies.CleanlinessPenalty", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::cleanliness_penalty>},
    {"awards_trophies.EmergencyBonus", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::emergency_bonus>},
    {"awards_trophies.EmergencyPenalty", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::emergency_penalty>},
    {"awards_trophies.AwardStaffHappinessBonus", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::award_staff_happiness_bonus>},
    {"awards_trophies.AwardStaffHappinessPenalty", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::award_staff_happiness_penalty>},
    {"awards_trophies.PeepHappinessBonus", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::peep_happiness_bonus>},
    {"awards_trophies.PeepHappinessPenalty", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::peep_happiness_penalty>},
    {"awards_trophies.WaitingTimesBonus", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::waiting_times_bonus>},
    {"awards_trophies.WaitingTimesPenalty", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::waiting_times_penalty>},
    {"awards_trophies.WellKeptTechBonus", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::well_kept_tech_bonus>},
    {"awards_trophies.WellKeptTechPenalty", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::well_kept_tech_penalty>},
    {"awards_trophies.NewTechAward", 0x14, 1, 0, 4000000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::new_tech_award>},
    {"awards_trophies.NewTechPoor", 0x14, 1, 0, 4000000,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::new_tech_poor>},
    {"awards_trophies.ResearchBonus", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::research_bonus>},
    {"awards_trophies.ResearchPenalty", 0x9, 1, -127, 127,
     set_in<&Balance::awards_trophies, &Balance::AwardsTrophies::research_penalty>},
    {"non_visuals", 0x12, 20, 0, 10000, set_array<&Balance::non_visuals>},
    {"visuals", 0x12, 14, 0, 10000, set_array<&Balance::visuals>},
    {"visuals_available", 0x12, 14, 0, 1000, set_array<&Balance::visuals_available>},
    {"gbv.AbilityThreshold", 0x12, 3, 0, 1000, set_in_array<&Balance::gbv, &Balance::Gbv::ability_threshold>},
    {"gbv.RschImproveCostPercent", 0x11, 1, 0, 100, set_in<&Balance::gbv, &Balance::Gbv::rsch_improve_cost_percent>},
    {"gbv.RschImproveIncrementPercent", 0x11, 1, 0, 100,
     set_in<&Balance::gbv, &Balance::Gbv::rsch_improve_increment_percent>},
    {"gbv.RemoveRatHoleChance", 0x14, 1, 1, 10000, set_in<&Balance::gbv, &Balance::Gbv::remove_rat_hole_chance>},
};
const Variable *find_variable(std::string_view name)
{
    for (const Variable &v : kVariables)
        if (name == v.name)
            return &v;
    return nullptr;
}

// The game's assign_value: false if the key is unknown or the index or value
// is out of its bounds.
bool assign_value(Balance &b, std::string_view name, uint32_t value, uint16_t index)
{
    const Variable *v = find_variable(name);
    if (!v || index > v->max_index)
        return false;
    bool below_max = v->flags & kFlagUnsigned ? value <= uint32_t(v->max) : int32_t(value) <= v->max;
    if (int32_t(value) < v->min || !below_max)
        return false;
    v->set(b, index, value);
    return true;
}

// Watcom's isspace for the game's characters.
bool is_space(char c)
{
    return c == ' ' || (c >= '\t' && c <= '\r');
}

// Watcom's atol (and atoi): leading spaces, a sign, then digits, wrapping.
uint32_t parse_long(std::string_view s)
{
    size_t i = 0;
    while (i < s.size() && is_space(s[i]))
        i++;
    char sign = i < s.size() ? s[i] : 0;
    if (sign == '+' || sign == '-')
        i++;
    uint32_t value = 0;
    for (; i < s.size() && s[i] >= '0' && s[i] <= '9'; i++)
        value = value * 10 + uint32_t(s[i] - '0');
    return sign == '-' ? 0u - value : value;
}

// The game's read_line_from_buf over a file it ends with 0xff.
class LineReader
{
  public:
    explicit LineReader(std::span<const uint8_t> data) : data_(data) {}

    // Reads up to `max` characters of the next line into `line`, then drops
    // the last one (meant for the '\r') unless the line filled `max` at the
    // end of the file. Unless at the end, the byte after the characters read
    // is skipped: the '\n', or of a longer line, its next character. Returns
    // false, with `line` empty, if no character was read.
    bool next(std::string &line, size_t max)
    {
        line.clear();
        while (!at_end() && data_[pos_] != '\n' && line.size() < max)
            line.push_back(char(data_[pos_++]));
        if (line.empty())
            return false;
        if (!at_end()) {
            line.pop_back();
            pos_++;
        } else if (line.size() != max) {
            line.pop_back();
        }
        // The game handles the line as a C string.
        line.resize(std::strlen(line.c_str()));
        return true;
    }

  private:
    bool at_end() const { return pos_ >= data_.size() || data_[pos_] == 0xff; }

    std::span<const uint8_t> data_;
    size_t pos_ = 0;
};

// The game's array_index: removes the first "[n]" from `name` and returns n,
// or 0 if there is none.
uint16_t take_array_index(std::string &name)
{
    size_t open = name.find('[');
    if (open == std::string::npos)
        return 0;
    size_t close = name.find(']', open + 1);
    if (close == std::string::npos)
        close = name.size();
    uint16_t index = uint16_t(parse_long(std::string_view(name).substr(open + 1, close - open - 1)));
    name.erase(open, close == name.size() ? close - open : close - open + 1);
    return index;
}

// Returns the next run of non-space characters at `pos` and moves past it
// and the spaces after it.
std::string_view next_word(std::string_view line, size_t &pos)
{
    size_t start = pos;
    while (pos < line.size() && !is_space(line[pos]))
        pos++;
    std::string_view word = line.substr(start, pos - start);
    while (pos < line.size() && is_space(line[pos]))
        pos++;
    return word;
}

// Applies one line; false if a value could not be set.
bool read_line(std::string_view line, Balance &out)
{
    size_t hash = line.find('#');
    if (hash == std::string_view::npos)
        return true;
    size_t pos = hash + 1;
    std::string name(next_word(line, pos));
    uint16_t index = take_array_index(name);

    size_t dot = name.find('.');
    if (dot == std::string::npos) {
        // The game's strip_formatting would cut the name at a space or tab,
        // but the name holds none (it scans on past its end in memory).
        return assign_value(out, name, parse_long(next_word(line, pos)), index);
    }
    // "front.A.B" sets front.A and front.B from the values in turn; a value
    // missing from the line is 0.
    std::string_view front = std::string_view(name).substr(0, dot);
    bool ok = true;
    while (dot < name.size()) {
        size_t end = name.find('.', dot + 1);
        if (end == std::string::npos)
            end = name.size();
        std::string full = std::string(front) + name.substr(dot, end - dot);
        if (!assign_value(out, full, parse_long(next_word(line, pos)), index))
            ok = false;
        dot = end;
    }
    return ok;
}

} // namespace

Balance::Balance()
    : expertise(kExpertise), objects(kObjectSettings), towns(kTowns), rooms(kRooms), staff(kStaff), gbv(kGbv),
      win_criteria(kWinCriteria), lose_criteria(kLoseCriteria), net_criteria(kNetCriteria), popn(kPopn),
      staff_levels(kStaffLevels), emergency_control(kEmergencyControl), quake_control(kQuakeControl),
      computer(kComputer), awards_trophies(kAwardsTrophies), non_visuals(kNonVisuals), visuals(kVisuals),
      visuals_available(kVisualsAvailable)
{
}

bool read_balance_file(std::span<const uint8_t> text, Balance &out)
{
    LineReader reader(text);
    std::string line;
    while (reader.next(line, 0x100))
        if (!read_line(line, out))
            return false;
    return true;
}

void balance_control(Balance &b)
{
    b.illness_kinds.clear();
    b.visuals_total = 0;
    for (size_t i = 0; i < b.visuals.size(); i++) {
        if (b.visuals[i]) {
            b.illness_kinds.push_back(uint8_t(i + 2));
            b.objects[kVisualTreatmentObject[i]].available_for_level = 1;
        }
        b.visuals_total = uint16_t(b.visuals_total + b.visuals[i]);
        b.visuals[i] = b.visuals_total;
    }
    bool all = true;
    for (uint16_t w : b.non_visuals)
        if (w)
            all = false;
    b.non_visuals_total = 0;
    for (size_t i = 0; i < b.non_visuals.size(); i++) {
        if (all)
            b.non_visuals[i] = 1;
        b.illness_kinds.push_back(uint8_t(i + 16));
        b.non_visuals_total = uint16_t(b.non_visuals_total + b.non_visuals[i]);
        b.non_visuals[i] = b.non_visuals_total;
    }
    b.expertise[1].known = 1;
}

bool load_balance(const std::filesystem::path &dir, int level, std::string_view balance_id, Balance &out,
                  std::string *error)
{
    std::string id(balance_id.empty() ? kDefaultBalanceId : balance_id);
    char number[16];
    std::snprintf(number, sizeof number, "%2.2d", level);
    bool ok = true;
    for (const std::string &rel : {"levels/" + id + "00.sam", "levels/" + id + number + ".sam"}) {
        std::string why;
        auto text = load_data_file(dir, rel, &why);
        if (!text) {
            if (error)
                *error = ok ? why : *error + "; " + why;
            ok = false;
            continue;
        }
        read_balance_file(*text, out);
    }
    balance_control(out);
    return ok;
}

std::optional<std::string> balance_description(const std::filesystem::path &dir, std::string_view balance_id,
                                               int language)
{
    auto text = load_data_file(dir, "levels/" + std::string(balance_id) + "00.sam");
    if (!text)
        return std::nullopt;
    LineReader reader(*text);
    std::string line;
    auto is_ours = [&] {
        return line.size() > 1 && line[0] == '&' && static_cast<signed char>(line[1]) - '0' == language;
    };
    std::string name;
    reader.next(line, 0x20);
    bool found = is_ours();
    if (!found)
        name = line;
    for (int i = 0; !found && i < 5; i++) {
        reader.next(line, 0x20);
        found = is_ours();
    }
    if (found)
        name = line.substr(2);
    return name;
}

} // namespace th
