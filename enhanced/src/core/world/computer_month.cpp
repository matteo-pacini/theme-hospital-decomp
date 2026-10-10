// The computer players' money and reputation (src/computer.cpp, and the
// parts of src/sim.cpp and src/finance.cpp that reach hospitals 1-3).

#include "core/world/arrivals.h"
#include "core/world/computer.h"
#include "core/world/reputation.h"
#include "core/world/trace.h"
#include "core/world/world.h"
#include "core/world/wrap32.h"

#include <algorithm>

namespace th
{

namespace
{

// Months in the buckets' ring.
constexpr uint32_t kBucketMonths = 12;
// change_balance's ceiling (src/finance.cpp).
constexpr int32_t kMaxBalance = 9999999;
// The day interval of computer_day.
constexpr int kBucketDays = 5;

// Whether the game set the hospital up as a computer player's
// (allhosps.player_type 2).
bool is_computer_player(const RivalHospital &rival)
{
    return rival.player_type == 2;
}

// change_balance (src/finance.cpp) for a hospital that is not the player's:
// no statement, only the ceiling.
void rival_change_balance(RivalHospital &rival, int32_t amount)
{
    rival.balance = std::min(rival.balance + amount, kMaxBalance);
}

// The town table's interest rate for level 0: a computer player's level is 0
// (init_player runs before the level is stored in the hospital).
int32_t rival_interest_rate(const Balance &settings)
{
    return settings.towns[0].interest_rate;
}

} // namespace

void rival_reputation_affected(RivalHospital &rival, HospitalMarket &market, const Balance &settings, uint8_t month,
                               uint8_t treatment, RepEvent event)
{
    const RepChange change = reputation_change(market, treatment, event, 0);
    ComputerBucket &bucket = rival.buckets[month % kBucketMonths];
    if (treatment) {
        bucket.room_rep_points[treatment] += change.delta;
        bucket.room_rep_events[treatment] += change.count;
    }
    bucket.rep_points += change.delta;
    bucket.rep_events += change.count;
    if (treatment)
        update_single_cure_cost(market, settings, treatment);
}

void get_computer_bucket(World &world, ArrivalState &arrivals, uint8_t owner)
{
    RivalHospital &rival = world.rivals[owner];
    HospitalMarket &market = arrivals.hospitals[owner];
    ComputerBucket &bucket = rival.buckets[arrivals.game_month % kBucketMonths];
    rival.balance += bucket.income;
    rival.cures = uint16_t(rival.cures + bucket.cures);
    rival.deaths = uint16_t(rival.deaths + bucket.deaths);
    market.hospital_rep_points += bucket.rep_points;
    market.hospital_rep_events += bucket.rep_events;
    evaluate_reputation(market, 0);
    rival.patient_load = std::max(rival.patient_load - bucket.patients_done, 0);
    for (size_t i = 0; i < kRivalIllnesses; i++) {
        market.rep_points[i] += bucket.room_rep_points[i];
        market.rep_events[i] += bucket.room_rep_events[i];
        evaluate_reputation(market, uint8_t(i));
    }
    trace_event("computer_bucket %u balance=%d cures=%d deaths=%d rep=%d load=%d", unsigned(owner), int(rival.balance),
                int(rival.cures), int(rival.deaths), int(market.hospital_reputation), int(rival.patient_load));
    bucket = {};
}

void computer_day(World &world, ArrivalState &arrivals, int day)
{
    if (day % kBucketDays != 0)
        return;
    for (size_t owner = 1; owner < world.rivals.size(); owner++)
        if (is_computer_player(world.rivals[owner]))
            get_computer_bucket(world, arrivals, uint8_t(owner));
}

void computer_month(World &world, ArrivalState &arrivals)
{
    for (size_t owner = 1; owner < world.rivals.size(); owner++) {
        RivalHospital &rival = world.rivals[owner];
        if (!is_computer_player(rival))
            continue;
        get_computer_bucket(world, arrivals, uint8_t(owner));
        rival.balance -= rival.wage_bill;
        trace_event("computer_month %u balance=%d wage_bill=%d", unsigned(owner), int(rival.balance),
                    int(rival.wage_bill));
    }
}

void rival_month_end(World &world, ArrivalState &arrivals, const Balance &settings, int heating_price)
{
    const int32_t interest = rival_interest_rate(settings);
    for (size_t owner = 1; owner < world.rivals.size(); owner++) {
        RivalHospital &rival = world.rivals[owner];
        const int32_t heating = rival.radiator_count * rival.heating_level * heating_price / 100;
        if (heating != 0)
            rival_change_balance(rival, -heating);
        if (rival.loan != 0)
            rival_change_balance(rival, -(interest * rival.loan / 10000 / 12));
        if (rival.balance < 0)
            rival_change_balance(rival, mul32(mul32(interest, rival.balance), 2) / 10000 / 12);
        set_price_level(arrivals.hospitals[owner], settings, rival.cure_available);
    }
}

void rival_market_share(World &world, const ArrivalState &arrivals)
{
    int32_t total = 0;
    for (const HospitalMarket &hosp : arrivals.hospitals)
        total += int32_t(hosp.month_visitors);
    for (size_t owner = 1; owner < world.rivals.size(); owner++) {
        const int32_t visitors = int32_t(arrivals.hospitals[owner].month_visitors);
        world.rivals[owner].market_share = uint16_t(total != 0 ? visitors * 100 / total : 0);
    }
}

void rival_year_start(World &world, const ArrivalState &arrivals)
{
    for (size_t owner = 1; owner < world.rivals.size(); owner++) {
        world.rivals[owner].year_start_balance = world.rivals[owner].balance;
        world.rivals[owner].year_start_visitors = int32_t(arrivals.hospitals[owner].visitors);
    }
}

} // namespace th
