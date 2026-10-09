#include "common.h"

#include "core/data/level.h"
#include "core/world/complex.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/object_tables.h"
#include "core/world/year_end.h"

#include <algorithm>
#include <cstdint>

// The year-end and win/lose simulation (core/world/year_end.cpp): the criteria
// a level is won or lost by, the score and the yearly figures, against level
// 1's own thresholds.

namespace
{

// Makes every win criterion of `game` met by setting the figures it measures.
void satisfy_win_criteria(th::Game &game)
{
    const th::Balance &settings = game.settings();
    const th::HospitalMarket &m = game.arrivals.hospitals[0];
    for (const th::Balance::WinLose &c : settings.win_criteria) {
        switch (c.criteria) {
        case 1:
            game.arrivals.hospitals[0].hospital_reputation = c.value;
            break;
        case 2:
            game.balance = c.value + game.finance.loan;
            break;
        case 3:
            game.arrivals.hospitals[0].visitors = 1000;
            game.arrivals.hospitals[0].diagnosed = uint32_t(c.value * 10);
            break;
        case 4:
            game.finance.cures = uint16_t(c.value);
            break;
        case 6:
            game.finance.hospital_value = c.value;
            break;
        default:
            break;
        }
    }
    (void)m;
}

} // namespace

void test_yearend_criteria(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::Game &game = *f.game;

    // init_player's: score 10000, the criteria copied from the level.
    CHECK(game.progress.score == 10000 && game.progress.start_score == 10000);
    CHECK(game.progress.market_share == 25);
    for (size_t i = 0; i < 10; i++) {
        CHECK(game.progress.win_criteria[i].criterion == f.settings.win_criteria[i].criteria);
        CHECK(game.progress.win_criteria[i].value == f.settings.win_criteria[i].value);
        CHECK(game.progress.lose_criteria[i].criterion == f.settings.lose_criteria[i].criteria);
    }

    // Everything met: the criteria are won (100) and check_game_over offers
    // the next level.
    satisfy_win_criteria(game);
    CHECK(th::evaluate_criteria(game, 0) >= 100);
    CHECK(game.progress.win_progress >= 100);
    th::check_game_over(game);
    CHECK(game.progress.winlose_result == 1 && game.progress.level_won == 1);
    CHECK(game.progress.year_end_type == 5);
    CHECK(game.progress.next_level_offer > 0 && game.progress.next_level_offer <= 200000000);

    // A low balance and reputation: a lose criterion is met.
    game.balance = -100000;
    game.arrivals.hospitals[0].hospital_reputation = 0;
    CHECK(th::check_criteria(game, false, 0) != 0);
}

void test_yearend_score(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::Game &game = *f.game;
    satisfy_win_criteria(game);
    th::evaluate_criteria(game, 0);
    const int32_t before = game.progress.score;
    th::calculate_score(game, 0);
    CHECK(game.progress.score >= before);
}

void test_yearend_year(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::Game &game = *f.game;
    game.arrivals.hospitals[0].visitors = 42;
    game.finance.cures = 7;
    game.finance.deaths = 3;
    game.date.month = 12; // the year wraps
    th::year_end_sim(game);
    CHECK(game.date.month == 0 && game.date.year == 1);
    CHECK(game.progress.year_start_visitors == 42);
    CHECK(game.progress.year_start_cures == 7 && game.progress.year_start_deaths == 3);
    CHECK(game.progress.skip_things); // the year-end turn's things do not run
}

// A year end that pays a trophy keeps it in progress.trophies, which the
// year-end screen copies as the original's display_awards does (the original
// clears only its trophies_won/awards_won display arrays).
void test_yearend_trophy_kept(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::Game &game = *f.game;
    for (int32_t &visitors : game.finance.history[th::kHistoryVisitors])
        visitors = 1; // over 5 for the year, so the trophies are checked
    game.finance.cures = 0;
    game.arrivals.hospitals[0].visitors = 2; // not the cures trophy
    game.finance.deaths = 0;
    game.progress.prev_year_deaths = 0; // no deaths again: trophy 4
    th::year_end_sim(game);
    CHECK(game.progress.trophies[4].id == 4 && game.progress.trophies[4].text_index != 0);
    bool visible = false;
    for (const th::Trophy &t : game.progress.trophies)
        visible = visible || t.text_index != 0;
    CHECK(visible);
}

// A drink counts a can for the drinks trophy (complex_success, action 2), and
// pays the hospital's drinks income.
void test_yearend_drinks_counted(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::Game &game = *f.game;
    uint16_t id = 0;
    for (size_t i = 1; i < game.world.things.size(); i++)
        if (game.world.things[i].type == th::kThingFree) {
            id = uint16_t(i);
            break;
        }
    CHECK(id != 0);
    th::Thing &p = game.world.things[id];
    p.type = th::kThingPatient;
    p.owner = 0;
    p.cash = 100;
    int32_t income = game.world.drinks_income;
    th::complex_success(game.world, id, 2);
    CHECK(game.progress.award_counts.cans_of_coke == 1);
    CHECK(game.world.drinks_income == income + 20);
}

// A machine's value is the object_stats cost (which research lowers), added as
// it is placed and taken away as it is removed, and calculate_value counts it.
void test_yearend_object_value(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::Game &game = *f.game;
    int kind = 0;
    for (int k = 1; k < th::kObjectKinds; k++)
        if (th::kObjects[size_t(k)].category == 1) {
            kind = k;
            break;
        }
    CHECK(kind != 0);
    game.world.research.objects[size_t(kind)].cost = 500;
    th::add_object_value(game.world, kind, 1);
    CHECK(game.finance.object_value == 500);
    th::add_object_value(game.world, kind, -1);
    CHECK(game.finance.object_value == 0);

    // calculate_value reads the stored value, not the level's base prices.
    game.finance.object_value = 0;
    th::calculate_value(game.finance, game.world, game.settings(), game.reputation());
    int32_t without = game.finance.hospital_value;
    game.finance.object_value = 1234;
    th::calculate_value(game.finance, game.world, game.settings(), game.reputation());
    CHECK(game.finance.hospital_value == without + 1234);
}

// The computer players' year end: their value, score and criteria, and the
// league tables made from their figures.
void test_yearend_rivals(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::Game &game = *f.game;
    const int32_t land = game.settings().gbv.land_cost_per_tile;

    th::RivalHospital r;
    r.area = 100;
    r.object_value = 5000;
    th::calculate_value(r, 500, game.settings());
    CHECK(r.hospital_value == 100 * land + 5000 + 10 * 1000 + 10000);
    CHECK(r.max_loan == std::clamp(r.hospital_value / 3 + 10000, 0, 100000));
    r.object_value = -100000000;
    th::calculate_value(r, 0, game.settings());
    CHECK(r.hospital_value == 0 && r.max_loan == 10000);
    r.object_value = 100000000;
    th::calculate_value(r, 1000, game.settings());
    CHECK(r.max_loan == 100000);

    // A rival nobody has cured scores nothing and leaves the player's own
    // progress as it was.
    const int32_t player_score = game.progress.score;
    const int32_t player_progress = game.progress.win_progress;
    for (size_t i = 1; i < 4; i++) {
        game.world.rivals[i].player_type = 2;
        game.world.rivals[i].area = uint16_t(10 * i);
        game.arrivals.hospitals[i].hospital_reputation = 500;
    }
    th::computer_year(game);
    CHECK(game.world.rivals[2].hospital_value == 20 * land + 500 / 50 * 1000 + 10000);
    CHECK(game.progress.score == player_score && game.progress.win_progress == player_progress);
    CHECK(game.world.rivals[1].score >= 10000);

    // One that meets every win criterion is at 100 and its score grows;
    // another that is not a computer player is left alone.
    th::RivalHospital &winner = game.world.rivals[3];
    game.arrivals.hospitals[3].hospital_reputation = 1000;
    game.arrivals.hospitals[3].visitors = 1000;
    game.arrivals.hospitals[3].diagnosed = 1000;
    winner.balance = 20000000;
    winner.cures = 60000;
    winner.object_value = 20000000;
    const int32_t before = winner.score;
    game.world.rivals[1].player_type = 0;
    const int32_t idle = game.world.rivals[1].score;
    th::computer_year(game);
    CHECK(winner.win_progress >= 100);
    CHECK(winner.score > before);
    CHECK(game.world.rivals[1].score == idle);
}

// create_chart_data and the awards read each computer player's own figures.
void test_yearend_rival_charts(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::Game &game = *f.game;
    for (size_t i = 1; i < 4; i++) {
        game.world.rivals[i].cures = uint16_t(10 * i);
        game.world.rivals[i].deaths = uint16_t(40 - 10 * i);
        game.world.rivals[i].score = int32_t(1000 * i);
        game.world.rivals[i].staff_count = uint16_t(i);
        game.arrivals.hospitals[i].visitors = uint32_t(100 * i);
    }
    game.finance.cures = 15;
    game.finance.deaths = 0;
    th::create_chart_data(game);
    // Cures best first: rival 3 (30), 2 (20), the player (15), rival 1 (10).
    CHECK(game.progress.chart[3][0].hosp == 3 && game.progress.chart[3][1].hosp == 2);
    CHECK(game.progress.chart[3][2].hosp == 0 && game.progress.chart[3][3].hosp == 1);
    // Deaths fewest first: the player (0), rival 3 (10), 2 (20), 1 (30).
    CHECK(game.progress.chart[4][0].hosp == 0 && game.progress.chart[4][3].hosp == 1);
    CHECK(game.progress.chart[1][0].value == 3000 || game.progress.chart[1][0].value == game.progress.score);
    CHECK(game.progress.chart[9][0].value == 300);

    // The year end keeps each rival's balance and visitors as the new year's.
    game.world.rivals[2].balance = 4242;
    th::year_end_sim(game);
    CHECK(game.world.rivals[2].year_start_balance == 4242);
    CHECK(game.world.rivals[2].year_start_visitors == 200);
}

// No tile of any level, in the file or the starting map, has another
// hospital's owner bit without the player's: the town map's detail panel, which
// names the first owner, never names a computer player in single player.
void test_level_rival_owner_cells(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    for (int n = 1; n <= 12; n++) {
        auto level = gd.load_level(n, &error);
        if (!level)
            continue;
        size_t rival_only = 0;
        const auto map = th::build_start_map(*level);
        for (const th::MapCell &c : map)
            rival_only += (c.owner_flags & 0xf) > 1 && !(c.owner_flags & 1);
        for (const th::MapCell &c : level->tiles)
            rival_only += (c.owner_flags & 0xf) > 1 && !(c.owner_flags & 1);
        CHECK(rival_only == 0);
    }
}
