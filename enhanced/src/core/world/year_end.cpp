#include "core/world/year_end.h"

#include "core/world/arrivals.h"
#include "core/world/counts.h"
#include "core/world/game.h"
#include "core/world/object_tables.h"
#include "core/world/reputation.h"
#include "core/world/research.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"
#include "core/world/wrap32.h"

#include <algorithm>
#include <cstdlib>

namespace th
{

namespace
{

// rating[] (src/tables.cpp): the score growth bands calculate_score uses.
struct Rating {
    int16_t threshold;
    uint16_t growth;
    uint16_t capped_growth;
};
constexpr Rating kRatings[] = {
    {10, 0, 0},       {60, 500, 0},     {70, 600, 0},     {75, 700, 0},      {80, 800, 0},     {90, 900, 200},
    {95, 1000, 300},  {100, 5000, 400}, {105, 5500, 500}, {110, 5600, 500},  {120, 5800, 500}, {130, 6000, 600},
    {150, 6200, 700}, {175, 6400, 700}, {200, 6500, 700}, {1000, 6500, 700},
};

// The player's staff (myhosp->staff_count): hospital 0's hired, not the
// applicants.
int player_staff(const World &world)
{
    int n = 0;
    for (const Thing &t : world.things)
        if (t.type == kThingStaff && t.owner == 0 && t.state != kStaffApplicant)
            n++;
    return n;
}

// The player's current win (mode 0) or lose criterion rows.
std::array<WinLoseRow, 10> &criterion_rows(Game &game, bool win)
{
    return win ? game.progress.win_criteria : game.progress.lose_criteria;
}

// The figure a criterion measures (check_criteria_values) for a computer
// player's hospital.
int32_t rival_criterion_value(const Game &game, uint16_t criterion, bool win, uint8_t owner)
{
    const RivalHospital &rival = game.world.rivals[owner];
    const HospitalMarket &market = game.arrivals.hospitals[owner];
    switch (criterion) {
    case 1:
        return market.hospital_reputation;
    case 2:
        return win ? rival.balance - rival.loan : rival.balance;
    case 3:
        return market.visitors > 10 ? int32_t(market.diagnosed) * 100 / int32_t(market.visitors) : 0;
    case 4:
        return rival.cures;
    case 5:
        return market.visitors > 10 ? int32_t(rival.deaths) * 100 / int32_t(market.visitors) : 0;
    case 6:
        return rival.hospital_value;
    default:
        return 0;
    }
}

// The figure a criterion measures (check_criteria_values) for the player
// (owner 0) or a computer player.
int32_t criterion_value(Game &game, uint16_t criterion, bool win, uint8_t owner)
{
    if (owner != 0)
        return rival_criterion_value(game, criterion, win, owner);
    switch (criterion) {
    case 1:
        return game.reputation();
    case 2:
        return win ? game.balance - game.finance.loan : game.balance;
    case 3:
        return game.arrivals.hospitals[0].visitors > 10
                   ? int32_t(game.arrivals.hospitals[0].diagnosed) * 100 / int32_t(game.arrivals.hospitals[0].visitors)
                   : 0;
    case 4:
        return game.finance.cures;
    case 5:
        return game.arrivals.hospitals[0].visitors > 10
                   ? int32_t(game.finance.deaths) * 100 / int32_t(game.arrivals.hospitals[0].visitors)
                   : 0;
    case 6:
        return game.finance.hospital_value;
    default:
        return 0;
    }
}

// paying with change_balance (src/finance.cpp) at the year end's date (day 1
// of the new January).
void pay(Game &game, uint8_t type, int32_t amount)
{
    change_balance(game.finance, game.balance, 1, 0, {type, amount});
}

// The presentation's generator (saferand). Only the awards' text variants
// read it; the trace does not.
uint16_t safe_rand(Game &game)
{
    game.progress.safe_seed = std::rotr(game.progress.safe_seed * 0x24a1u + 0x24dfu, 13);
    return uint16_t(game.progress.safe_seed);
}

// Computer player `owner`'s value for award's statistic (general_award).
int32_t rival_award_value(const Game &game, int owner, int award)
{
    const RivalHospital &rival = game.world.rivals[size_t(owner)];
    switch (award) {
    case 0:
        return game.arrivals.hospitals[size_t(owner)].hospital_reputation;
    case 1:
        return rival.cures;
    case 2:
        return rival.deaths;
    case 3:
        return rival.market_share;
    case 6:
        return rival.hospital_value;
    default:
        return 0;
    }
}

bool rival_award_is_wide(int award)
{
    return award == 0 || award == 6;
}

// set_award_value (0xb4510): records award won (dir 1) or lost (dir -1) with
// text tier `tier`, paying `value` as reputation (type 1) or money (type 2).
void set_award_value(Game &game, int award, int8_t dir, uint8_t tier, int16_t value, uint8_t type)
{
    Award &a = game.progress.awards[award];
    a.direction = dir;
    a.id = uint8_t(award);
    a.text_index = uint8_t(tier + award * 3 + 1);
    if (type == 2) {
        pay(game, kTxYearEndBonus, value);
        a.value = value;
    } else if (type == 1) {
        int32_t rep = game.reputation();
        reputation_affected(game.world, 0, kRepAward, value);
        if (game.reputation() != rep) {
            a.value = game.reputation() - rep;
        } else {
            if (dir > 0) {
                pay(game, kTxYearEndBonus, 2500);
                a.value = 2500;
            } else if (dir < 0) {
                pay(game, kTxYearEndBonus, -2500);
                a.value = -2500;
            }
            type = 2;
        }
    }
    a.reward_kind = uint8_t(type - 1);
}

// set_trophy_value (0xb43c0): records trophy `trophy_no`, paying `value` as
// reputation (type 1) or money (type 2, TX_TROPHY_BONUS); 5000 when the
// reputation cannot change, with the trophy's id recorded as 2.
void set_trophy_value(Game &game, int trophy_no, int16_t value, uint8_t type)
{
    Trophy &t = game.progress.trophies[size_t(trophy_no)];
    t.id = uint8_t(trophy_no);
    t.text_index = uint8_t(trophy_no * 3 + safe_rand(game) % 3 + 1);
    if (type == 1) {
        int32_t rep = game.reputation();
        reputation_affected(game.world, 0, kRepTrophy, value);
        if (game.reputation() != rep) {
            t.value = game.reputation() - rep;
        } else {
            pay(game, kTxTrophyBonus, 5000);
            t.value = 5000;
            t.id = 2;
        }
    } else if (type == 2) {
        pay(game, kTxTrophyBonus, value);
        t.value = value;
    }
    t.reward_kind = uint8_t(type - 1);
}

// Whether the player's `value` for award beats each of the three rivals:
// `top` true if it beats them all, `bottom` true if none of them is at or
// below it, `total` the sum of those it beats.
void award_against_rivals(const Game &game, int award, int32_t value, int32_t &total, bool &top, bool &bottom)
{
    const bool wide = rival_award_is_wide(award);
    total = 0;
    top = true;
    bottom = true;
    for (int h = 1; h < kHospitalCount; h++) {
        const int32_t other = rival_award_value(game, h, award);
        bool beats = wide ? uint32_t(value) >= uint32_t(other) : value >= other;
        if (!beats) {
            top = false;
        } else {
            total += other;
            bottom = false;
        }
    }
}

// An award's fixed figures for general_award: which award, its good and poor
// thresholds, how it pays (type) and its bonus and penalty.
struct AwardSpec {
    int award = 0;
    int32_t high = 0;
    int32_t low = 0;
    uint8_t type = 0;
    int16_t good = 0;
    int16_t bad = 0;
};

// general_award (0xb4698): gives award from `value` against the other
// hospitals: above `spec.high` good (tripled when it also beats their total,
// doubled when none beats it), below `spec.low` bad.
void general_award(Game &game, int32_t value, const AwardSpec &spec)
{
    int32_t total = 0;
    bool top = false;
    bool bottom = false;
    award_against_rivals(game, spec.award, value, total, top, bottom);
    if (value > spec.high) {
        if (top && value > total && !bottom)
            set_award_value(game, spec.award, 1, 2, int16_t(spec.good * 3), spec.type);
        else if (top)
            set_award_value(game, spec.award, 1, 1, int16_t(spec.good * 2), spec.type);
        else
            set_award_value(game, spec.award, 1, 0, spec.good, spec.type);
    } else if (value < spec.low) {
        if (!bottom)
            set_award_value(game, spec.award, -1, 0, spec.bad, spec.type);
        else
            set_award_value(game, spec.award, -1, 1, int16_t(spec.bad * 2), spec.type);
    }
}

// year_check (0xb40ec): all 12 monthly values at least limit (above) or at
// most limit (else).
bool year_check(const int32_t *values, int32_t limit, bool above)
{
    for (int i = 0; i < 12; i++) {
        if (above ? values[i] < limit : values[i] > limit)
            return false;
    }
    return true;
}

// The context check_awards_won's and check_trophies_won's cases share.
struct AwardsContext {
    Game &game;
    int32_t cures;        // the year's cures (history[6] total)
    int32_t visitors;     // the year's visitors (history[4] total)
    int32_t new_visitors; // myhosp->visitors - year_start_visitors
};

const Balance::AwardsTrophies &trophies(const Game &game)
{
    return game.settings().awards_trophies;
}

// check_awards_won's per-award cases (src/awards.cpp's switch), by award
// number. Each decides from the year's figures and pays.
using AwardCase = void (*)(AwardsContext &);
void award_reputation(AwardsContext &c);
void award_cures(AwardsContext &c);
void award_deaths(AwardsContext &c);
void award_population(AwardsContext &c);
void award_cures_v_deaths(AwardsContext &c);
void award_peep_happiness(AwardsContext &c);
void award_hospital_value(AwardsContext &c);
void award_staff_happiness(AwardsContext &c);
void award_cleanliness(AwardsContext &c);
void award_waiting_times(AwardsContext &c);
void award_emergency(AwardsContext &c);
void award_research(AwardsContext &c);
void award_machines(AwardsContext &c);

constexpr std::array<AwardCase, 13> kAwardCases = {
    award_reputation,     award_cures,          award_deaths,          award_population,  award_cures_v_deaths,
    award_peep_happiness, award_hospital_value, award_staff_happiness, award_cleanliness, award_waiting_times,
    award_emergency,      award_research,       award_machines,
};

// check_trophies_won's per-trophy cases, by trophy number.
using TrophyCase = void (*)(AwardsContext &);
void trophy_cures(AwardsContext &c);
void trophy_reputation(AwardsContext &c);
void trophy_staff_happiness(AwardsContext &c);
void trophy_mayor(AwardsContext &c);
void trophy_deaths(AwardsContext &c);
void trophy_rat_kills(AwardsContext &c);
void trophy_rat_pct(AwardsContext &c);
void trophy_plants(AwardsContext &c);
void trophy_drinks(AwardsContext &c);

constexpr std::array<TrophyCase, 9> kTrophyCases = {
    trophy_cures,     trophy_reputation, trophy_staff_happiness, trophy_mayor,  trophy_deaths,
    trophy_rat_kills, trophy_rat_pct,    trophy_plants,          trophy_drinks,
};

// The monthly figures the awards are checked against, from the statistics
// pass.
void update_monthly_award_counts(YearEnd &state, const World &world, int month, int32_t leave_max)
{
    const AdCounts &c = world.counts;
    AwardCounts &ac = state.award_counts;
    const int staff = player_staff(world);
    ac.staff_happiness[size_t(month)] = staff > 3 ? (int32_t(int16_t(c.staff_happiness_sum)) << 8) / staff : 0;
    if (c.patients > 3) {
        ac.peep_happiness[size_t(month)] = (int32_t(c.happiness_sum) << 8) / c.patients;
        ac.waiting_times[size_t(month)] = (c.waiting_sum << 8) / c.patients;
    } else {
        ac.peep_happiness[size_t(month)] = 0;
        ac.waiting_times[size_t(month)] = leave_max << 8;
    }
    int plants = world.plants;
    int32_t plant_health_sum = world.plant_health_sum;
    // The product wraps in 32 bits, as the original's multiply does.
    ac.plant_health[size_t(month)] = plants > 7 ? int32_t(uint32_t(plant_health_sum) * 0x190000u) / plants : 0;
    ac.machine_wear[size_t(month)] = c.machine_wear_sum;
    ac.machines[size_t(month)] = c.machines;
}

} // namespace

void start_year_end(Game &game)
{
    YearEnd &s = game.progress;
    s = {};
    s.score = 10000;
    s.start_score = 10000;
    s.market_share = 25;
    for (size_t i = 0; i < 10; i++) {
        const Balance::WinLose &w = game.settings().win_criteria[i];
        const Balance::WinLose &l = game.settings().lose_criteria[i];
        s.win_criteria[i] = {w.criteria, w.max_min, w.value, w.bound, w.group, 0, 0, 0};
        s.lose_criteria[i] = {l.criteria, l.max_min, l.value, l.bound, l.group, 0, 0, 0};
    }
}

void monthly_update_of_award_counts(Game &game, int month)
{
    update_monthly_award_counts(game.progress, game.world, month, game.settings().gbv.leave_max);
    game.progress.award_counts.research_earned = game.world.research.earned;
    game.progress.award_counts.research_spent = game.world.research.spent;
}

void annual_update_of_award_counts(AwardCounts &counts)
{
    counts.cans_of_coke = 0;
    counts.mayor_score_total = 0;
    counts.mayor_visits = 0;
    counts.research_earned = 0;
    counts.research_spent = 0;
    counts.rats_killed = 0;
    counts.rat_shots = 0;
}

uint32_t check_criteria_values(Game &game, bool win, uint8_t owner)
{
    if (owner == 0xff)
        return 0;
    auto &rows = criterion_rows(game, win);
    int max = 0;
    for (size_t i = 0; i < rows.size(); i++) {
        WinLoseRow &r = rows[i];
        r.met = 0;
        if (r.group > max)
            max = r.group;
        int32_t val = 0;
        if (r.criterion == 0)
            r.met = 1;
        else
            val = criterion_value(game, r.criterion, win, owner);
        if (r.max_min == 1 ? val >= r.value : val <= r.value)
            r.met = 1;
        int32_t range = r.value - r.base;
        int32_t pct = range != 0 ? (val - r.base) * 100 / range : 0;
        r.progress = int16_t(std::clamp(pct, 0, 200));
        r.current = val;
    }
    if (owner == 0) {
        for (size_t i = 0; i < rows.size(); i++)
            trace_event("crit %u %d c=%u now=%d target=%d met=%u", unsigned(win ? 0 : 1), int(i),
                        unsigned(rows[i].criterion), int(rows[i].current), int(rows[i].value), unsigned(rows[i].met));
    }
    return uint32_t(max);
}

int32_t check_criteria(Game &game, bool win, uint8_t owner)
{
    const uint32_t count = check_criteria_values(game, win, owner);
    const auto &rows = criterion_rows(game, win);
    for (uint32_t crit = 1; crit <= count; crit++) {
        bool all = true, any = false;
        for (const WinLoseRow &r : rows) {
            if (r.group != crit)
                continue;
            any = true;
            if (!r.met) {
                all = false;
                break;
            }
        }
        if (all && any)
            return int32_t(crit);
    }
    return 0;
}

namespace
{

// evaluate_criteria's score for one criterion: 100 when met, else its percent
// of the target, 0-100 (0-200 once the level is won).
int criterion_score(const WinLoseRow &r, int32_t level_won)
{
    if (r.met)
        return 100;
    int32_t pct = 0;
    if (r.max_min == 1) {
        if (r.value != 0) {
            pct = std::abs(r.current * 100 / r.value);
            if (r.criterion == 2 && r.current < 0)
                pct = 0;
        }
    } else if (r.current != 0) {
        pct = std::abs(r.value * 100 / r.current);
    }
    return int(std::clamp(pct, int32_t(0), level_won == 0 ? 100 : 200));
}

// evaluate_criteria's average over the criteria of `group`; 0 for none.
int group_average(const std::array<WinLoseRow, 10> &rows, uint32_t group, int32_t level_won)
{
    int count = 0, sum = 0;
    for (const WinLoseRow &r : rows) {
        if (r.group != group)
            continue;
        count++;
        sum += criterion_score(r, level_won);
    }
    return count ? sum / count : 0;
}

} // namespace

int32_t evaluate_criteria(Game &game, uint8_t owner)
{
    const int32_t level_won = game.progress.level_won;
    const uint32_t groups = check_criteria_values(game, true, owner);
    const auto &rows = game.progress.win_criteria;
    int32_t best = 0, best_group = 0, total = 0;
    for (uint32_t group = 1; group <= groups; group++) {
        const int avg = group_average(rows, group, level_won);
        if (avg > best) {
            best = avg;
            best_group = int32_t(group);
        }
        total += avg;
    }
    game.progress.win_average = total > 0 && groups > 0 ? total / int32_t(groups) : 0;
    if (owner == 0) {
        game.progress.win_progress = best;
        game.progress.best_win_group = best_group;
    } else {
        game.world.rivals[owner].win_progress = uint8_t(best);
    }
    return best;
}

void calculate_score(Game &game, uint8_t owner)
{
    int32_t &score = owner == 0 ? game.progress.score : game.world.rivals[owner].score;
    const int32_t start_score = owner == 0 ? game.progress.start_score : game.world.rivals[owner].start_score;
    const int32_t progress = evaluate_criteria(game, owner);
    for (const Rating &r : kRatings) {
        if (r.threshold == 0 || r.threshold < progress)
            continue;
        if (score > 1000000)
            score = add32(score, mul32(r.capped_growth, score) / 10000);
        else if (mul32(game.settings().gbv.score_max_inc, start_score) / 100 < score)
            score = add32(score, mul32(r.capped_growth, score) / 10000);
        else
            score = add32(score, mul32(r.growth, score) / 10000);
        break;
    }
    if (owner == 0)
        trace_event("score %d", int(score));
    if (score > 200000000)
        score = 200000000;
}

void calculate_value(RivalHospital &rival, int reputation, const Balance &settings)
{
    const int32_t rep_value = reputation / 50 * 1000;
    const int32_t value = rival.area * settings.gbv.land_cost_per_tile + rival.object_value + rep_value + 10000;
    rival.hospital_value = std::max(value, 0);
    rival.max_loan = std::clamp(rival.hospital_value / 3 + 10000, 0, 100000);
}

void computer_year(Game &game)
{
    for (uint8_t owner = 1; owner < kHospitalCount; owner++) {
        RivalHospital &rival = game.world.rivals[owner];
        if (rival.player_type != 2)
            continue;
        calculate_value(rival, game.arrivals.hospitals[owner].hospital_reputation, game.settings());
        calculate_score(game, owner);
        trace_event("computer_year %u value=%d score=%d", unsigned(owner), int(rival.hospital_value), int(rival.score));
    }
}

void check_win_status(Game &game)
{
    if ((game.turn_count & 0x1f) != 0)
        return;
    evaluate_criteria(game, 0);
    check_criteria_values(game, false, 0);
    int32_t total = 0;
    int count = 0;
    for (const WinLoseRow &r : game.progress.lose_criteria) {
        if (r.criterion) {
            total += r.progress;
            count++;
        }
    }
    game.progress.lose_progress = count > 0 ? (total << 8) / count : 0;
}

void check_game_over(Game &game)
{
    YearEnd &s = game.progress;
    const int32_t score = evaluate_criteria(game, 0);
    if (score >= 100) {
        s.level_won = 1;
        int32_t pct = s.win_progress / 2;
        pct += int32_t(old_normal_rand(game.world.seed, 0x37, 0x3c));
        s.next_level_offer = s.score * pct / 100;
        if (s.next_level_offer > 200000000)
            s.next_level_offer = 200000000;
        trace_event("winlose win group=%u offer=%d", unsigned(s.best_win_group), int(s.next_level_offer));
        s.year_end_type = 5;
        s.winlose_result = 1;
        s.win_declined = 0;
        try_bonus_level(game);
    } else if (int32_t group = check_criteria(game, false, 0)) {
        trace_event("winlose lose group=%d", int(group));
        s.year_end_type = 7;
        s.winlose_result = 2;
    }
}

void check_quarterly_game_over(Game &game)
{
    YearEnd &s = game.progress;
    if (game.level_number() >= 12)
        return;
    const int32_t progress = evaluate_criteria(game, 0);
    trace_event("quarterly progress=%d offered=%d", int(progress), progress >= 100 ? 1 : 0);
    if (progress < 100)
        return;
    if (!s.win_declined) {
        s.level_won = 1;
        int32_t pct = s.win_progress / 2;
        pct += int32_t(old_normal_rand(game.world.seed, 0x37, 0x3c));
        s.next_level_offer = s.score * pct / 100;
        if (s.next_level_offer > 200000000)
            s.next_level_offer = 200000000;
        s.year_end_type = 5;
        s.skip_things = true;
        try_bonus_level(game);
    }
    s.win_declined = 0;
}

namespace
{

// One hospital's figures for a year-end league table.
struct ChartFigures {
    int32_t balance = 0, loan = 0, cures = 0, deaths = 0, value = 0, score = 0;
    int32_t staff = 0, visitors = 0;
};

ChartFigures chart_player(const Game &game)
{
    const YearEnd &s = game.progress;
    return {game.balance,
            game.finance.loan,
            game.finance.cures,
            game.finance.deaths,
            game.finance.hospital_value,
            s.score,
            player_staff(game.world),
            int32_t(game.arrivals.hospitals[0].visitors)};
}

ChartFigures chart_rival(const Game &game, size_t owner)
{
    const RivalHospital &r = game.world.rivals[owner];
    return {r.balance,        r.loan,  r.cures,       r.deaths,
            r.hospital_value, r.score, r.staff_count, int32_t(game.arrivals.hospitals[owner].visitors)};
}

// Sorts each league table best first (table 4, the deaths, ascending).
void sort_charts(YearEnd &s)
{
    const auto descend = [](const ChartEntry &a, const ChartEntry &b) { return a.value > b.value; };
    const auto ascend = [](const ChartEntry &a, const ChartEntry &b) { return a.value < b.value; };
    for (int cat : {0, 1, 3, 5, 8, 9, 10})
        std::stable_sort(s.chart[size_t(cat)].begin(), s.chart[size_t(cat)].end(), descend);
    std::stable_sort(s.chart[4].begin(), s.chart[4].end(), ascend);
}

} // namespace

void create_chart_data(Game &game)
{
    YearEnd &s = game.progress;
    // The running claims total is not reset between hospitals; a computer
    // player has none of its own.
    int32_t total = 0;
    if (game.world.hospital) {
        for (size_t j = 0; j < 3; j++)
            for (size_t k = 0; k < 12; k++)
                total += game.world.hospital->insurance_claims[j][k];
    }
    for (size_t i = 0; i < 4; i++) {
        const ChartFigures f = i == 0 ? chart_player(game) : chart_rival(game, i);
        s.chart[0][i] = {f.balance - f.loan + total, uint8_t(i)};
        s.chart[1][i] = {f.score, uint8_t(i)};
        s.chart[3][i] = {f.cures, uint8_t(i)};
        s.chart[4][i] = {f.deaths, uint8_t(i)};
        s.chart[5][i] = {f.cures * 100 / (f.cures + f.deaths + 1), uint8_t(i)};
        s.chart[8][i] = {f.staff, uint8_t(i)};
        s.chart[9][i] = {f.visitors, uint8_t(i)};
        s.chart[10][i] = {f.value, uint8_t(i)};
    }
    sort_charts(s);
}

namespace
{

void award_reputation(AwardsContext &c)
{
    if (c.game.level_number() * 2 > c.cures)
        return;
    const auto &t = trophies(c.game);
    general_award(c.game, c.game.reputation(),
                  {0, t.reputation_award, t.reputation_poor, 2, t.award_reputation_bonus, t.award_reputation_penalty});
}

void award_cures(AwardsContext &c)
{
    if (c.game.arrivals.hospitals[0].visitors <= uint32_t(c.game.progress.year_start_visitors))
        return;
    const auto &t = trophies(c.game);
    general_award(
        c.game, c.game.finance.cures,
        {1, t.cures_award * c.new_visitors, t.cures_poor * c.new_visitors, 2, t.cures_bonus, t.cures_penalty});
}

void award_deaths(AwardsContext &c)
{
    if (c.game.arrivals.hospitals[0].visitors <= uint32_t(c.game.progress.year_start_visitors) ||
        c.game.finance.cures == 0)
        return;
    const auto &t = trophies(c.game);
    general_award(
        c.game, c.game.finance.deaths,
        {2, t.deaths_award * c.new_visitors, t.deaths_poor * c.new_visitors, 2, t.deaths_bonus, t.deaths_penalty});
}

void award_population(AwardsContext &c)
{
    if (c.game.level_number() * 2 > c.cures)
        return;
    const auto &t = trophies(c.game);
    general_award(c.game, c.game.progress.market_share,
                  {3, t.population_percentage_award, t.population_percentage_poor, 1, t.population_percentage_bonus,
                   t.population_percentage_penalty});
}

void award_cures_v_deaths(AwardsContext &c)
{
    const auto &t = trophies(c.game);
    if (c.game.finance.deaths > 0) {
        int32_t a = int32_t(c.game.finance.cures) * 0x6400 / c.game.finance.deaths;
        if (int32_t(t.cures_v_deaths_award) << 8 < a)
            set_award_value(c.game, 4, 1, 0, t.cures_v_deaths_bonus, 2);
        else if (int32_t(t.cures_v_deaths_poor) << 8 > a)
            set_award_value(c.game, 4, -1, 0, t.cures_v_deaths_penalty, 2);
    } else if (c.game.arrivals.hospitals[0].visitors > uint32_t(c.game.progress.year_start_visitors) &&
               t.cures_bonus <= c.cures) {
        set_award_value(c.game, 4, 1, 0, t.cures_v_deaths_bonus, 2);
    }
}

void award_peep_happiness(AwardsContext &c)
{
    const auto &t = trophies(c.game);
    const auto &v = c.game.progress.award_counts.peep_happiness;
    if (year_check(v.data(), t.peep_happiness_award, true))
        set_award_value(c.game, 5, 1, 0, t.peep_happiness_bonus, 1);
    else if (year_check(v.data(), t.peep_happiness_poor, false))
        set_award_value(c.game, 5, -1, 0, t.peep_happiness_penalty, 1);
}

void award_hospital_value(AwardsContext &c)
{
    const auto &t = trophies(c.game);
    general_award(
        c.game, c.game.finance.hospital_value,
        {6, int32_t(t.hosp_value_award), int32_t(t.hosp_value_poor), 1, t.hosp_value_bonus, t.hosp_value_penalty});
}

void award_staff_happiness(AwardsContext &c)
{
    const auto &t = trophies(c.game);
    const auto &v = c.game.progress.award_counts.staff_happiness;
    if (year_check(v.data(), t.staff_happiness_award, true))
        set_award_value(c.game, 7, 1, 0, t.award_staff_happiness_bonus, 1);
    else if (year_check(v.data(), t.staff_happiness_poor, false))
        set_award_value(c.game, 7, -1, 0, t.award_staff_happiness_penalty, 1);
}

void award_cleanliness(AwardsContext &c)
{
    const Finance &f = c.game.finance;
    const auto &t = trophies(c.game);
    if (f.area == 0)
        return;
    int32_t a = (int32_t(c.game.world.litter_count + c.game.world.rats.hole_count * 2) << 14) / f.area;
    if ((int32_t(t.cleanliness_award) << 14) / 100 > a)
        set_award_value(c.game, 8, 1, 0, t.cleanliness_bonus, 1);
    else if (0x4000 / t.cleanliness_poor < a)
        set_award_value(c.game, 8, -1, 0, t.cleanliness_penalty, 1);
}

void award_waiting_times(AwardsContext &c)
{
    const auto &t = trophies(c.game);
    const auto &v = c.game.progress.award_counts.waiting_times;
    if (year_check(v.data(), t.waiting_times_award, false))
        set_award_value(c.game, 9, 1, 0, t.waiting_times_bonus, 1);
    else if (year_check(v.data(), t.waiting_times_poor, true))
        set_award_value(c.game, 9, -1, 0, t.waiting_times_penalty, 1);
}

void award_emergency(AwardsContext &c)
{
    (void)c; // award_counts.emergencies is never written, as the game's is not.
}

void award_research(AwardsContext &c)
{
    (void)c; // research_spent was cleared by the annual update, as in the game.
}

void award_machines(AwardsContext &c)
{
    const AwardCounts &ac = c.game.progress.award_counts;
    const auto &t = trophies(c.game);
    if (year_check(ac.machines.data(), 1, true) && year_check(ac.machine_wear.data(), t.well_kept_tech_award, false))
        set_award_value(c.game, 12, 1, 0, t.well_kept_tech_bonus, 1);
    else if (c.game.world.machine_exploded || (year_check(ac.machines.data(), 1, true) &&
                                               year_check(ac.machine_wear.data(), t.well_kept_tech_poor, true)))
        set_award_value(c.game, 12, -1, 0, t.well_kept_tech_penalty, 1);
}

void trophy_cures(AwardsContext &c)
{
    const auto &t = trophies(c.game);
    const auto &m = c.game.arrivals.hospitals[0];
    if (m.visitors > uint32_t(c.game.progress.year_start_visitors)) {
        if ((int32_t(c.game.finance.cures) - c.game.progress.year_start_cures) * 256 / c.new_visitors > 0xf3) {
            set_trophy_value(c.game, 0, t.trophy_cures_bonus, 2);
            pay(c.game, kTxTrophyBonus, t.trophy_cures_bonus);
        }
    } else if (m.visitors == c.game.finance.cures) {
        set_trophy_value(c.game, 0, t.trophy_cures_bonus, 2);
        pay(c.game, kTxTrophyBonus, int32_t(t.trophy_cures_bonus) * 2);
    }
}

void trophy_reputation(AwardsContext &c)
{
    const auto &t = trophies(c.game);
    if (year_check(c.game.finance.history[kHistoryReputation].data(), t.reputation, true))
        set_trophy_value(c.game, 1, t.trophy_reputation_bonus, 2);
}

void trophy_staff_happiness(AwardsContext &c)
{
    const auto &t = trophies(c.game);
    if (year_check(c.game.progress.award_counts.staff_happiness.data(), t.trophy_staff_happiness, true))
        set_trophy_value(c.game, 2, t.trophy_staff_happiness_bonus, 1);
}

void trophy_mayor(AwardsContext &c)
{
    (void)c; // mayor_visits is never written.
}

void trophy_deaths(AwardsContext &c)
{
    const auto &t = trophies(c.game);
    if (c.game.progress.prev_year_deaths == int32_t(c.game.finance.deaths))
        set_trophy_value(c.game, 4, t.trophy_death_bonus, 2);
    c.game.progress.prev_year_deaths = c.game.finance.deaths;
}

void trophy_rat_kills(AwardsContext &c)
{
    const auto &t = trophies(c.game);
    if (int32_t(c.game.progress.award_counts.rats_killed) > t.rat_kills_absolute)
        set_trophy_value(c.game, 5, t.rat_kills_absolute_bonus, 1);
}

void trophy_rat_pct(AwardsContext &c)
{
    const auto &t = trophies(c.game);
    const int32_t killed = c.game.progress.award_counts.rats_killed;
    const int32_t shots = c.game.progress.award_counts.rat_shots;
    if (shots > 0 && killed > t.rat_kills_absolute / 2 &&
        (uint32_t(killed) << 8) / uint32_t(shots) > (uint32_t(t.rat_kills_percentage) << 8) / 100)
        set_trophy_value(c.game, 6, t.rat_kills_percentage_bonus, 2);
}

void trophy_plants(AwardsContext &c)
{
    const auto &t = trophies(c.game);
    if (year_check(c.game.progress.award_counts.plant_health.data(), int32_t(t.plant) << 14, true))
        set_trophy_value(c.game, 7, t.plant_bonus, 1);
}

void trophy_drinks(AwardsContext &c)
{
    const auto &t = trophies(c.game);
    if (c.game.progress.award_counts.cans_of_coke > t.cans_of_coke)
        set_trophy_value(c.game, 8, t.cans_of_coke_bonus, 2);
}

} // namespace

void check_awards_won(Game &game)
{
    for (Award &a : game.progress.awards)
        a = {};
    int32_t visitors = 0, cures = 0;
    for (int i = 0; i < 12; i++) {
        visitors += game.finance.history[kHistoryVisitors][size_t(i)];
        cures += game.finance.history[kHistoryCures][size_t(i)];
    }
    if (game.level_number() * 3 >= visitors)
        return;
    AwardsContext c{game, cures, visitors,
                    int32_t(game.arrivals.hospitals[0].visitors) - game.progress.year_start_visitors};
    for (AwardCase award_case : kAwardCases)
        award_case(c);
}

void check_trophies_won(Game &game)
{
    game.progress.trophies = {};
    int32_t visitors = 0;
    for (int i = 0; i < 12; i++)
        visitors += game.finance.history[kHistoryVisitors][size_t(i)];
    if (visitors <= 5)
        return;
    AwardsContext c{game, 0, visitors,
                    int32_t(game.arrivals.hospitals[0].visitors) - game.progress.year_start_visitors};
    for (TrophyCase trophy_case : kTrophyCases)
        trophy_case(c);
}

void year_end_sim(Game &game)
{
    // The game pauses for the year-end screens before process_things, so
    // this turn's things do not run.
    game.progress.skip_things = true;
    calculate_value(game.finance, game.world, game.settings(), game.reputation());
    calculate_score(game, 0);
    game.finance.income_last_year = 0;
    game.finance.expenditure_last_year = 0;
    for (int i = 0; i < 12; i++) {
        game.finance.income_last_year += game.finance.history[kHistoryIncome][size_t(i)];
        game.finance.expenditure_last_year += game.finance.history[kHistoryExpenditure][size_t(i)];
    }
    computer_year(game);
    game.date.month = 0;
    if (++game.date.year > 999)
        game.date.year = 0;
    game.finance.entry_count = 0;
    for (size_t i = 1; i < size_t(kHospitalCount); i++) {
        game.world.rivals[i].year_start_balance = game.world.rivals[i].balance;
        game.world.rivals[i].year_start_visitors = int32_t(game.arrivals.hospitals[i].visitors);
    }
    game.progress.year_start_visitors = int32_t(game.arrivals.hospitals[0].visitors);
    game.progress.year_start_cures = game.finance.cures;
    game.progress.year_start_deaths = game.finance.deaths;
    game.progress.year_start_staff = player_staff(game.world);
    game.finance.year_start_balance = game.balance;
    rival_year_start(game.world, game.arrivals);
    game.finance.statement_balance = game.balance;
    create_chart_data(game);
    check_trophies_won(game);
    game.progress.award_counts.research_earned = game.world.research.earned;
    game.progress.award_counts.research_spent = game.world.research.spent;
    annual_update_of_award_counts(game.progress.award_counts);
    // start_year_end: the held member of staff goes back. The original
    // clears only the screens' display arrays (trophies_won/awards_won), not
    // the computed trophy[]/all_awards[]; check_awards_won clears all_awards
    // itself, and the screens copy the kept trophies and awards as
    // display_awards does.
    put_back_held_staff(game);
    check_awards_won(game);
    // The win or loss is detected here, at the turn, as the check agrees
    // (replay-yearend.md); its one draw is thus in this turn's trace.
    check_game_over(game);
    game.finance.statement_balance = game.balance;
    game.world.research.earned = 0;
    game.world.research.spent = 0;
}

void try_bonus_level(Game &game)
{
    const Balance::AwardsTrophies &t = trophies(game);
    if (game.progress.rat_shots > 0 && game.progress.rats_killed > uint32_t(t.rat_kills_absolute) / 2) {
        if ((uint32_t(game.progress.rats_killed) << 8) / game.progress.rat_shots >
            (uint32_t(t.rat_kills_percentage) << 8) / 100) {
            game.progress.bonus_level =
                uint8_t((old_newrand(game.world.seed, std::source_location::current()) & 1) + 13);
        }
    }
    trace_event("bonus_level %u", unsigned(game.progress.bonus_level));
}

} // namespace th
