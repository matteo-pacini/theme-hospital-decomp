#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace th
{

// The game's tables that balance files (levels/*.sam) set, keyed as the
// game's balance_variables (src/vloader.cpp): a key "towns.StartCash" is
// towns[i].start_cash, "gbv.SalaryAdd" gbv.salary_add[i], "visuals" visuals[i].
// Field names are the keys' members in snake_case; each field has the width
// and signedness the game writes it with (but for the two noted), so a value
// the game truncates is truncated here too (expertise.StartPrice takes up to
// 1000000 into 16 bits).
// Arrays are as long as the game's tables. A default Balance holds the game's
// static values (src/tables.cpp, src/array.cpp, src/comtable.cpp; zero for
// visuals_available, emergency_control and quake_control).
struct Balance {
    Balance();

    struct Expertise {
        uint16_t start_price = 0;
        uint8_t cont_rate = 0;
        uint32_t rsch_reqd = 0;
        uint16_t max_diag_diff = 0;
        uint8_t known = 0;
    };
    std::array<Expertise, 47> expertise{};
    struct Object {
        // Unsigned in the balance table; signed where the game reads it.
        int32_t start_cost = 0;
        uint8_t start_avail = 0;
        uint16_t when_avail = 0;
        uint8_t start_strength = 0;
        uint8_t available_for_level = 0;
    };
    std::array<Object, 63> objects{};
    struct Town {
        uint32_t start_cash = 0;
        // The game writes 4 bytes here (clearing the 3 after it) and reads 1.
        uint8_t ill_rate = 0;
        uint16_t interest_rate = 0;
        uint16_t inflation_rate = 0;
    };
    std::array<Town, 16> towns{};
    struct Room {
        int32_t cost = 0;
    };
    std::array<Room, 31> rooms{};
    struct Staff {
        uint16_t min_salary = 0;
    };
    std::array<Staff, 10> staff{};
    struct Gbv {
        uint16_t land_cost_per_tile = 0;
        std::array<int16_t, 9> salary_add{};
        uint16_t salary_ability_divisor = 0;
        uint16_t research_points_divisor = 0;
        uint16_t start_rating = 0;
        uint16_t start_cost = 0;
        uint16_t min_drug_cost = 0;
        uint16_t how_contagious = 0;
        uint16_t reduce_cont_months = 0;
        uint16_t reduce_cont_peep_count = 0;
        uint16_t reduce_cont_rate = 0;
        uint16_t hold_visual_months = 0;
        uint16_t hold_visual_peep_count = 0;
        uint16_t rest_standing = 0;
        uint16_t rest_sofa = 0;
        uint16_t rest_game = 0;
        uint16_t rest_snooker = 0;
        uint16_t work_light = 0;
        uint16_t modify_freq = 0;
        uint16_t not_tired = 0;
        uint16_t tired = 0;
        uint16_t very_tired = 0;
        uint16_t crack_up_tired = 0;
        uint16_t recovery_factor = 0;
        uint16_t recovery_minimum = 0;
        uint16_t max_object_strength = 0;
        uint16_t research_increment = 0;
        uint16_t q_points = 0;
        uint16_t dist_points = 0;
        uint16_t no_staff_points = 0;
        int16_t litter_drop = 0;
        int16_t leave_max = 0;
        int16_t resign_max = 0;
        int16_t happy = 0;
        int16_t unhappy = 0;
        int16_t very_unhappy = 0;
        int16_t drink_happy = 0;
        int16_t toilet_happy = 0;
        int16_t bowel_full = 0;
        int16_t bowel_overflows = 0;
        int16_t vomit_limit = 0;
        uint16_t litter_random = 0;
        int16_t salary_too_low = 0;
        int16_t salary_too_high = 0;
        uint16_t max_diag_diff = 0;
        uint16_t score_max_inc = 0;
        uint16_t vac_cost = 0;
        std::array<uint16_t, 3> training_value{};
        uint16_t disaster_launch = 0;
        uint16_t epidemic_fine = 0;
        uint16_t epidemic_comp_lo = 0;
        uint16_t epidemic_comp_hi = 0;
        uint16_t minimum_abduct_time = 0;
        uint16_t abductions_per_year = 0;
        uint16_t autopsy_rsch_percent = 0;
        uint16_t autopsy_rep_hit_percent = 0;
        uint8_t alloc_rand = 0;
        uint8_t alloc_total_rep = 0;
        uint8_t alloc_ind_rep = 0;
        uint8_t alloc_delay = 0;
        uint16_t training_rate = 0;
        uint8_t promo_doc = 0;
        uint8_t promo_con = 0;
        uint16_t doctor_threshold = 0;
        uint16_t consultant_threshold = 0;
        uint8_t drug_improve_rate = 0;
        uint8_t contagious_spread_factor = 0;
        uint16_t mayor_launch = 0;
        std::array<uint16_t, 3> ability_threshold{};
        uint8_t rsch_improve_cost_percent = 0;
        uint8_t rsch_improve_increment_percent = 0;
        uint32_t remove_rat_hole_chance = 0;
        // Not a balance key: the game's static value, the length of the opening
        // countdown of a single-player level (init_player, src/start.cpp).
        uint16_t open_countdown = 0;
    };
    Gbv gbv{};
    struct WinLose {
        uint16_t criteria = 0;
        uint16_t max_min = 0;
        int32_t value = 0;
        int32_t bound = 0;
        uint16_t group = 0;
    };
    std::array<WinLose, 10> win_criteria{};
    std::array<WinLose, 10> lose_criteria{};
    struct NetCriterion {
        uint16_t criteria = 0;
        int32_t value = 0;
        uint16_t month = 0;
        uint16_t time_to_do = 0;
    };
    std::array<NetCriterion, 20> net_criteria{};
    struct Population {
        uint16_t month = 0;
        int16_t change = 0;
    };
    std::array<Population, 30> popn{};
    struct StaffLevel {
        uint16_t month = 0;
        uint16_t nurses = 0;
        uint16_t doctors = 0;
        uint16_t handymen = 0;
        uint16_t receptionists = 0;
        uint8_t shrk_rate = 0;
        uint8_t surg_rate = 0;
        uint8_t rsch_rate = 0;
        uint8_t cons_rate = 0;
        uint8_t jr_rate = 0;
        uint32_t seed = 0;
    };
    std::array<StaffLevel, 30> staff_levels{};
    struct Emergency {
        uint16_t start_month = 0;
        uint16_t end_month = 0;
        uint8_t min = 0;
        uint8_t max = 0;
        uint8_t illness = 0;
        uint8_t perc_win = 0;
        uint16_t bonus = 0;
    };
    std::array<Emergency, 20> emergency_control{};
    struct Quake {
        uint16_t start_month = 0;
        uint16_t end_month = 0;
        uint16_t severity = 0;
    };
    std::array<Quake, 16> quake_control{};
    struct Computer {
        uint8_t skill = 0;
        uint8_t staff_levels = 0;
        uint8_t luck = 0;
        uint8_t speed = 0;
        uint8_t comfort = 0;
        uint8_t guess_at = 0;
        uint8_t playing = 0;
    };
    std::array<Computer, 20> computer{};
    struct AwardsTrophies {
        uint8_t rat_kills_absolute = 0;
        uint16_t rat_kills_absolute_bonus = 0;
        uint8_t cans_of_coke = 0;
        uint16_t cans_of_coke_bonus = 0;
        int16_t reputation = 0;
        uint16_t trophy_reputation_bonus = 0;
        uint8_t plant = 0;
        uint8_t plant_bonus = 0;
        uint8_t trophy_staff_happiness = 0;
        uint8_t trophy_staff_happiness_bonus = 0;
        uint8_t rat_kills_percentage = 0;
        uint16_t rat_kills_percentage_bonus = 0;
        uint16_t trophy_cures_bonus = 0;
        uint16_t trophy_death_bonus = 0;
        uint8_t trophy_mayor_bonus = 0;
        uint8_t trophy_mayor = 0;
        uint8_t cures_award = 0;
        uint8_t cures_poor = 0;
        uint8_t deaths_award = 0;
        uint8_t deaths_poor = 0;
        uint8_t population_percentage_award = 0;
        uint8_t population_percentage_poor = 0;
        uint8_t cures_v_deaths_award = 0;
        uint8_t cures_v_deaths_poor = 0;
        uint16_t reputation_award = 0;
        uint16_t reputation_poor = 0;
        uint32_t hosp_value_award = 0;
        uint32_t hosp_value_poor = 0;
        uint8_t cleanliness_award = 0;
        uint8_t cleanliness_poor = 0;
        uint8_t emergency_award = 0;
        uint8_t emergency_poor = 0;
        uint8_t staff_happiness_award = 0;
        uint8_t staff_happiness_poor = 0;
        uint8_t peep_happiness_award = 0;
        uint8_t peep_happiness_poor = 0;
        uint8_t waiting_times_award = 0;
        uint8_t waiting_times_poor = 0;
        uint8_t well_kept_tech_award = 0;
        uint8_t well_kept_tech_poor = 0;
        int16_t cures_bonus = 0;
        int16_t cures_penalty = 0;
        int16_t deaths_bonus = 0;
        int16_t deaths_penalty = 0;
        int8_t population_percentage_bonus = 0;
        int8_t population_percentage_penalty = 0;
        int16_t cures_v_deaths_bonus = 0;
        int16_t cures_v_deaths_penalty = 0;
        int16_t award_reputation_bonus = 0;
        int16_t award_reputation_penalty = 0;
        int16_t hosp_value_bonus = 0;
        int16_t hosp_value_penalty = 0;
        int8_t cleanliness_bonus = 0;
        int8_t cleanliness_penalty = 0;
        int8_t emergency_bonus = 0;
        int8_t emergency_penalty = 0;
        int8_t award_staff_happiness_bonus = 0;
        int8_t award_staff_happiness_penalty = 0;
        int8_t peep_happiness_bonus = 0;
        int8_t peep_happiness_penalty = 0;
        int8_t waiting_times_bonus = 0;
        int8_t waiting_times_penalty = 0;
        int8_t well_kept_tech_bonus = 0;
        int8_t well_kept_tech_penalty = 0;
        uint32_t new_tech_award = 0;
        uint32_t new_tech_poor = 0;
        int8_t research_bonus = 0;
        int8_t research_penalty = 0;
    };
    AwardsTrophies awards_trophies{};
    std::array<uint16_t, 20> non_visuals{};
    std::array<uint16_t, 14> visuals{};
    std::array<uint16_t, 14> visuals_available{};

    // Set by balance_control from the above once a level's files are read:
    // visuals and non_visuals then hold running totals of the weights,
    // visuals_total and non_visuals_total the totals, and illness_kinds the
    // illnesses a patient may get (weighted visual ones, every non-visual one).
    std::vector<uint8_t> illness_kinds;
    uint16_t visuals_total = 0;
    uint16_t non_visuals_total = 0;
};

// The balance id (file prefix) used when none is chosen: "FULL", the
// game's default difficulty. The game also ships "EASY" and "HARD".
inline constexpr std::string_view kDefaultBalanceId = "FULL";

// Applies an unpacked balance file to `out` as the game's read_variable_file
// does: every line holding "#key[index].Member1.Member2 value1 value2 ..."
// (the index may also follow a member, "#gbv.SalaryAdd[3]") sets those
// values; anything else is a comment. Lines are read as the game reads them,
// which assumes CRLF: each line's last character is dropped, and an empty
// line ends the file. An unknown key, an index above the key's bound or a
// value outside its bounds leaves that value unset, finishes the line and
// skips the rest of the file; then it returns false. Indexes the game
// accepts past the end of a table (where it writes into the next one) are
// ignored.
bool read_balance_file(std::span<const uint8_t> text, Balance &out);

// What the game's balance_control does after reading a level's files (see
// Balance::illness_kinds); it also marks the weighted visual illnesses'
// treatment objects available and makes general practice known.
void balance_control(Balance &b);

// Reads into `out` the balance files for campaign level `level` as the
// game's load_balance does: levels/<id>00.sam, then levels/<id><level>.sam
// (two digits at least), then balance_control. An empty `balance_id` is
// kDefaultBalanceId. Values are written over what `out` holds, as the game
// writes over its tables, so values a level's files do not set carry over
// from the previous level. Like the game, a missing file is passed over;
// then it returns false and says which in `error`.
bool load_balance(const std::filesystem::path &dir, int level, std::string_view balance_id, Balance &out,
                  std::string *error = nullptr);

// The name shown for balance file `balance_id` (the game's balance_filename):
// of the first six lines of levels/<id>00.sam, the first starting with '&' and
// the digit of `language`, less those two characters (" Doctor (Medium)" for
// "&0 Doctor (Medium)"); else its first line. Lines are cut at 31 characters,
// as the game cuts them. In the game's code page (437). Nothing if the file
// cannot be read.
std::optional<std::string> balance_description(const std::filesystem::path &dir, std::string_view balance_id,
                                               int language);

} // namespace th
