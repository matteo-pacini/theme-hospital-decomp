// The computer players' money and reputation: the buckets
// (get_computer_bucket), computer_day and computer_month, and the rival
// steps of month_end and year_end (src/computer.cpp, src/sim.cpp,
// src/finance.cpp).

#include "common.h"

#include "core/world/arrivals.h"
#include "core/world/computer.h"
#include "core/world/reputation.h"
#include "core/world/world.h"

#include <cstdint>
#include <memory>

namespace
{

using namespace th;

// Three computer players and the arrivals that weigh them, the level's
// start figures.
struct Rivals {
    std::unique_ptr<World> world = std::make_unique<World>();
    ArrivalState arrivals;
    Balance settings;

    explicit Rivals(const Balance &b) : settings(b)
    {
        start_arrivals(arrivals, settings);
        start_rivals(*world, settings, 100000);
        for (size_t owner = 1; owner < 4; owner++)
            world->rivals[owner].player_type = 2;
    }
};

// reputation_affected banks the event in the month's bucket, leaves the
// hospital's totals for the merge and moves the price at once.
void check_rival_reputation(const Balance &b)
{
    Rivals r(b);
    RivalHospital &rival = r.world->rivals[2];
    HospitalMarket &market = r.arrivals.hospitals[2];
    rival.cure_available[3] = 1;
    rival_reputation_affected(rival, market, r.settings, 14, 3, kRepCured);
    const ComputerBucket &bucket = rival.buckets[2];
    CHECK(bucket.rep_points == 1 && bucket.rep_events == 0);
    CHECK(bucket.room_rep_points[3] == 1 && bucket.room_rep_events[3] == 0);
    rival_reputation_affected(rival, market, r.settings, 14, 3, kRepCase);
    CHECK(bucket.rep_events == 1 && bucket.room_rep_events[3] == 1);
    rival_reputation_affected(rival, market, r.settings, 14, 0, kRepDied);
    CHECK(bucket.rep_points == 0 && bucket.room_rep_points[0] == 0 && bucket.room_rep_points[3] == 1);
    // Nothing is merged yet.
    CHECK(market.hospital_rep_points == 20 && market.hospital_rep_events == 20);
    CHECK(market.rep_points[3] == 20 && market.reputation[3] == 500);
    // Other months and hospitals are untouched.
    CHECK(rival.buckets[1].rep_events == 0 && r.world->rivals[1].buckets[2].rep_events == 0);

    // The price follows the share (price_scale) of what patients will pay, at
    // once: halve the scale and the price drops by the 1% threshold's rule.
    const int32_t before = market.price[3];
    market.price_scale[3] = 5000;
    rival_reputation_affected(rival, market, r.settings, 0, 3, kRepGoForTreatment);
    CHECK(market.price[3] == before / 2 || market.price[3] < before);
    const int32_t moved = market.price[3];
    rival_reputation_affected(rival, market, r.settings, 0, 0, kRepGoForTreatment);
    CHECK(market.price[3] == moved);
}

// get_computer_bucket adds the bucket for game_month % 12 to the hospital.
void check_bucket_merge(const Balance &b)
{
    Rivals r(b);
    r.arrivals.game_month = 12 + 5;
    RivalHospital &rival = r.world->rivals[1];
    HospitalMarket &market = r.arrivals.hospitals[1];
    ComputerBucket &bucket = rival.buckets[5];
    bucket.income = 700;
    bucket.cures = 3;
    bucket.deaths = 1;
    bucket.rep_points = 10;
    bucket.rep_events = 20;
    bucket.patients_done = 7;
    bucket.room_rep_points[4] = 6;
    bucket.room_rep_events[4] = 4;
    rival.buckets[6].income = 99;
    rival.patient_load = 5;
    rival.cures = 65535;
    get_computer_bucket(*r.world, r.arrivals, 1);
    CHECK(rival.balance == 100700 && rival.cures == 2 && rival.deaths == 1);
    CHECK(market.hospital_rep_points == 30 && market.hospital_rep_events == 40);
    CHECK(market.hospital_reputation == 30 * 500 / 40);
    CHECK(market.rep_points[4] == 26 && market.rep_events[4] == 24 && market.reputation[4] == 26 * 500 / 24);
    CHECK(market.reputation[5] == 500);
    // The load cannot go below 0.
    CHECK(rival.patient_load == 0);
    // The month's bucket is empty again, the next month's kept.
    CHECK(bucket.income == 0 && bucket.rep_events == 0 && bucket.room_rep_events[4] == 0 && bucket.patients_done == 0);
    CHECK(rival.buckets[6].income == 99);
    // The other hospitals did not change.
    CHECK(r.world->rivals[2].balance == 100000 && r.arrivals.hospitals[2].hospital_rep_events == 20);

    // A load above the visits done keeps the difference.
    rival.patient_load = 9;
    bucket.patients_done = 7;
    get_computer_bucket(*r.world, r.arrivals, 1);
    CHECK(rival.patient_load == 2);

    // Points from a negative total clamp the reputation at 0.
    bucket.rep_points = -1000;
    bucket.rep_events = 1;
    get_computer_bucket(*r.world, r.arrivals, 1);
    CHECK(market.hospital_reputation == 0);
}

// computer_day merges every fifth day; computer_month merges and pays the
// wages, for the computer players only.
void check_day_and_month(const Balance &b)
{
    Rivals r(b);
    r.world->rivals[3].player_type = 0;
    for (size_t owner = 1; owner < 4; owner++) {
        r.world->rivals[owner].buckets[0].income = 50;
        r.world->rivals[owner].wage_bill = 300;
    }
    computer_day(*r.world, r.arrivals, 4);
    CHECK(r.world->rivals[1].balance == 100000);
    computer_day(*r.world, r.arrivals, 10);
    CHECK(r.world->rivals[1].balance == 100050 && r.world->rivals[2].balance == 100050);
    CHECK(r.world->rivals[3].balance == 100000 && r.world->rivals[3].buckets[0].income == 50);
    computer_month(*r.world, r.arrivals);
    CHECK(r.world->rivals[1].balance == 100050 - 300 && r.world->rivals[3].balance == 100000);
    // The wages are not capped, and take a balance below zero.
    r.world->rivals[1].balance = 100;
    computer_month(*r.world, r.arrivals);
    CHECK(r.world->rivals[1].balance == -200);
}

// month_end's money for a computer player: the town 0 rate, whatever the
// player's level, on a loan or an overdraft; heating; the price level.
void check_month_end(Balance b)
{
    b.towns[0].interest_rate = 240;
    b.towns[1].interest_rate = 100;
    Rivals r(b);
    RivalHospital &rival = r.world->rivals[1];
    rival.balance = -12000;
    rival_month_end(*r.world, r.arrivals, r.settings, 750);
    CHECK(rival.balance == -12000 + 240 * -12000 * 2 / 10000 / 12);
    CHECK(r.world->rivals[2].balance == 100000);

    rival.balance = 5000;
    rival.loan = 20000;
    rival.radiator_count = 3;
    rival_month_end(*r.world, r.arrivals, r.settings, 750);
    CHECK(rival.balance == 5000 - 3 * 5 * 750 / 100 - 240 * 20000 / 10000 / 12);

    // A balance so far below zero that the original's 32-bit product wraps: the
    // interest is what the wrapped product gives, with no undefined overflow.
    rival.loan = 0;
    rival.radiator_count = 0;
    rival.balance = -20000000;
    rival_month_end(*r.world, r.arrivals, r.settings, 750);
    const int32_t wrapped = int32_t(uint32_t(240) * uint32_t(-20000000) * 2u);
    CHECK(wrapped / 10000 / 12 != 240 * -20000000LL * 2 / 10000 / 12);
    CHECK(rival.balance == -20000000 + wrapped / 10000 / 12);

    // The ceiling holds a balance that a negative interest would raise.
    rival.loan = 0;
    rival.radiator_count = 0;
    rival.balance = 9999999;
    rival_month_end(*r.world, r.arrivals, r.settings, 750);
    CHECK(rival.balance == 9999999);

    // The price level of a hospital that can only give general practice's
    // base treatment (index 0): prices at the most patients pay.
    r.arrivals.hospitals[1].price_level = 7;
    rival_month_end(*r.world, r.arrivals, r.settings, 750);
    CHECK(r.arrivals.hospitals[1].price_level >= 0 && r.arrivals.hospitals[1].price_level <= 300);
    rival.cure_available.fill(0);
    rival_month_end(*r.world, r.arrivals, r.settings, 750);
    CHECK(r.arrivals.hospitals[1].price_level == 100);
}

// The market shares of all four hospitals, a closed one's 0, and the year's
// starting figures.
void check_shares_and_year(const Balance &b)
{
    Rivals r(b);
    r.arrivals.hospitals[0].month_visitors = 10;
    r.arrivals.hospitals[1].month_visitors = 30;
    r.arrivals.hospitals[2].month_visitors = 0;
    r.arrivals.hospitals[3].month_visitors = 60;
    rival_market_share(*r.world, r.arrivals);
    CHECK(r.world->rivals[1].market_share == 30 && r.world->rivals[2].market_share == 0);
    CHECK(r.world->rivals[3].market_share == 60);
    for (HospitalMarket &h : r.arrivals.hospitals)
        h.month_visitors = 0;
    rival_market_share(*r.world, r.arrivals);
    CHECK(r.world->rivals[1].market_share == 0 && r.world->rivals[3].market_share == 0);

    r.world->rivals[2].balance = 4321;
    r.arrivals.hospitals[2].visitors = 77;
    rival_year_start(*r.world, r.arrivals);
    CHECK(r.world->rivals[2].year_start_balance == 4321 && r.world->rivals[2].year_start_visitors == 77);
    CHECK(r.world->rivals[0].year_start_balance == 0);
}

} // namespace

void test_computer_money(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    const th::Balance &b = f.game->settings();
    check_rival_reputation(b);
    check_bucket_merge(b);
    check_day_and_month(b);
    check_month_end(b);
    check_shares_and_year(b);
}
