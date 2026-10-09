#pragma once

#include "core/data/balance.h"
#include "core/world/computer.h"
#include "core/world/finance.h"

#include <array>
#include <cstdint>

namespace th
{

struct Game;

// The year-end and win/lose simulation (src/awards.cpp, src/sim.cpp,
// src/finance.cpp's year_end/start_year_end, src/rats.cpp's try_bonus_level):
// the annual figures the year-end charts, awards and trophies are decided
// from, the score and the criteria a level is won or lost by. The state is
// Game::progress; the functions take the Game and act on the player (0).

// One of the level's win or lose criteria as check_criteria_values leaves it
// (src/sim.cpp's WinLose).
struct WinLoseRow {
    uint16_t criterion = 0;
    uint16_t max_min = 0;
    int32_t value = 0;
    int32_t base = 0;
    uint16_t group = 0;
    uint16_t met = 0;
    int32_t current = 0;
    int16_t progress = 0;
};

// One award of the year (src/awards.cpp's Awards, all_awards[]): `direction`
// 1 won, 0xff lost; `text_index` picks the text variant; `reward_kind` the
// bonus text (type - 1).
struct Award {
    uint8_t id = 0;
    uint8_t text_index = 0;
    int8_t direction = 0;
    int32_t value = 0;
    uint8_t reward_kind = 0;
};

// One trophy of the year (src/awards.cpp's Trophies, trophy[]).
struct Trophy {
    uint8_t id = 0;
    uint8_t text_index = 0;
    int32_t value = 0;
    uint8_t reward_kind = 0;
};

// The totals and per-month figures the awards and trophies are decided from
// (src/awards.cpp's AwardCounts). The monthly arrays are written at each
// month end at the game's month index (1-12; index 0 is never written, as in
// the game) and checked by year_check over 0-11.
struct AwardCounts {
    std::array<int32_t, 13> staff_happiness{};
    std::array<int32_t, 13> peep_happiness{};
    std::array<int32_t, 13> plant_health{};
    std::array<int32_t, 13> waiting_times{};
    std::array<int32_t, 13> machine_wear{};
    std::array<int32_t, 13> machines{};
    int32_t cans_of_coke = 0;
    int32_t mayor_score_total = 0;
    int32_t mayor_visits = 0;
    int32_t research_earned = 0;
    int32_t research_spent = 0;
    int32_t rats_killed = 0;
    int32_t rat_shots = 0;
    bool machine_exploded = false;
};

// One entry of a year-end league table (src/annualch.cpp's ChartList).
struct ChartEntry {
    int32_t value = 0;
    uint8_t hosp = 0;
};

// The year-end and win/lose state (Game::progress).
struct YearEnd {
    // allhosps[0].score and start_score, and the win/lose progress
    // (win_progress; lose_progress is the advisor's).
    int32_t score = 0;
    int32_t start_score = 0;
    int32_t win_progress = 0;
    int32_t lose_progress = 0;
    int32_t best_win_group = 0;
    // allhosps[0].market_share (percent of the month's patients).
    int32_t market_share = 25;
    // The figures year_end records as the new year starts (allhosps[0]).
    int32_t year_start_visitors = 0;
    int32_t year_start_cures = 0;
    int32_t year_start_deaths = 0;
    int32_t year_start_staff = 0;
    // gamevar.prev_year_deaths, for the no-deaths trophy.
    int32_t prev_year_deaths = 0;
    // The rat counters (myhosp->rat_shots/rats_killed_u); rats are not ported.
    uint16_t rat_shots = 0;
    uint16_t rats_killed = 0;
    // othervar.level_won, win_declined and winlose_result (0 none, 1 win,
    // 2 lose), and the next level's offered score (othervar.next_level_offer).
    uint8_t level_won = 0;
    uint8_t win_declined = 0;
    uint8_t winlose_result = 0;
    int32_t next_level_offer = 0;
    // othervar.bonus_level / level_after_bonus (try_bonus_level), and the
    // year-end screen's step (year_end_type: 0-4 the screens, 5-7 win/lose).
    uint8_t bonus_level = 0;
    uint8_t level_after_bonus = 0;
    uint8_t year_end_type = 0;
    // The criteria, evaluated each statistics pass and at the year end.
    std::array<WinLoseRow, 10> win_criteria{};
    std::array<WinLoseRow, 10> lose_criteria{};
    // The awards and trophies won this year (all_awards, trophy).
    std::array<Award, 13> awards{};
    std::array<Trophy, 9> trophies{};
    AwardCounts award_counts;
    // The year-end league tables (chart[11][4]).
    std::array<std::array<ChartEntry, 4>, 11> chart{};
    // The awards' text variants (a private saferand: the presentation's
    // safeseed is the app's).
    uint32_t safe_seed = 0x546b2;
    // The turn the year end (or a quarterly win) is reached: the things do
    // not run, as the game pauses for the year-end screens before
    // process_things (cleared by Game::run_things).
    bool skip_things = false;
};

// Sets `state` up as init_player does (src/start.cpp): the win and lose
// criteria copied from the level, score and start_score 10000, market_share
// 25, every criterion evaluated once; award_counts and the awards cleared.
void start_year_end(Game &game);

// monthly_update_of_award_counts (0xb4170): stores this month's ratios from
// the statistics pass (World::counts) in AwardCounts at `month` (the game's
// index, 1-12), for year_check. Called at each month end after the month
// moves on, as the game calls it.
void monthly_update_of_award_counts(Game &game, int month);

// annual_update_of_award_counts (0xb4358): clears the year's totals (not the
// monthly arrays).
void annual_update_of_award_counts(AwardCounts &counts);

// check_criteria_values (0x591d4): measures hospital `owner`'s win (`win`
// true) or lose criteria into state.win_criteria / state.lose_criteria (a
// computer player's too, as the game does), setting each met, current and
// progress (0-200). Returns the highest group number.
uint32_t check_criteria_values(Game &game, bool win, uint8_t owner);

// check_criteria (0x590e0): the first win (`win` true) or lose criterion
// group whose conditions are all met, or 0.
int32_t check_criteria(Game &game, bool win, uint8_t owner);

// evaluate_criteria (0x594d0): hospital `owner`'s progress towards winning
// (the best win group's percent, 100 = won). Stores it in state.win_progress
// and state.best_win_group for the player, in the computer player's
// win_progress otherwise.
int32_t evaluate_criteria(Game &game, uint8_t owner);

// calculate_score (0x588a8): grows hospital `owner`'s score by the rating
// table's rate for its criteria progress, capped at 200000000.
void calculate_score(Game &game, uint8_t owner);

// calculate_value (0x52f7c) for a computer player: its land (area), objects
// and `reputation`, and the base 10000, as hospital_value (not below 0), and
// the loan limit max_loan a third of it plus 10000, within 0 to 100000. It
// owns no rooms.
void calculate_value(RivalHospital &rival, int reputation, const Balance &settings);

// computer_year (0x4e9d8): at the year end, calculate_value and
// calculate_score for each computer player.
void computer_year(Game &game);

// check_win_status (0x72c8c) on a statistics pass: re-evaluates the player's
// criteria and stores the lose progress.
void check_win_status(Game &game);

// check_game_over (0x589f0): ends the level if the player met the win
// criteria (year_end_type 5, winlose_result 1) or a lose criterion (7, 2).
void check_game_over(Game &game);

// check_quarterly_game_over (0x58b00): before level 12, offers the win as
// soon as the criteria are met (year_end_type 5, winlose state 1).
void check_quarterly_game_over(Game &game);

// check_awards_won (0xb3430): the player's 13 awards for the year, paying
// their money or reputation. Does nothing unless the year's visitors exceed
// three times the level.
void check_awards_won(Game &game);

// check_trophies_won (0xb3ce4): the player's 9 trophies for the year, paying
// their bonuses. Does nothing unless the year's visitors exceed 5.
void check_trophies_won(Game &game);

// create_chart_data (0xb4aac): fills state.chart with the four hospitals'
// figures and sorts each table best first (table 4 ascending).
void create_chart_data(Game &game);

// year_end_sim (0x51d98): the simulation part of the year end: values and
// scores the player, totals the year, moves the year on, records the year's
// starting figures, then the league tables, trophies and awards. The caller
// is the month end, at the turn the year wraps; the date's month and year
// have already moved on. (Finance::year_end is the money part alone, kept
// for the finance tests.)
void year_end_sim(Game &game);

// try_bonus_level (0x7e140): unlocks a bonus rat level when the player's rat
// kills beat the level's targets; nothing when they did not.
void try_bonus_level(Game &game);

} // namespace th
