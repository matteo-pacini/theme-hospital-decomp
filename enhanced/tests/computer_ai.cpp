// The computer players' AI: notes, the script's steps, the pending actions and
// the loan and land (src/computer.cpp).

#include "common.h"

#include "core/world/arrivals.h"
#include "core/world/computer.h"
#include "core/world/research.h"

#include <algorithm>
#include <cstdint>
#include <memory>

namespace
{

// A world with rival 1 playing computer player 0 of `settings`, whose skill is
// 3, speed 70, staff levels 0 and comfort 0.
struct AiFixture {
    std::unique_ptr<th::World> world = std::make_unique<th::World>();
    th::ArrivalState arrivals;
    th::Balance settings;
    uint32_t turn = 25;

    AiFixture()
    {
        settings.computer[0].skill = 3;
        settings.computer[0].speed = 70;
        settings.computer[0].staff_levels = 0;
        settings.computer[0].comfort = 0;
        settings.gbv.land_cost_per_tile = 100;
        world->seed = 0x2468ace0;
        rival().player_type = 2;
        rival().strategy = 1;
        rival().strategy_step = 1;
    }
    th::RivalHospital &rival() { return world->rivals[1]; }
    th::AiContext context() { return {*world, arrivals, settings, turn, 20}; }
};

// The turn an action of `type` is due at the given pace.
uint32_t delay_for(uint32_t turn, uint16_t type, int pace)
{
    return turn + uint32_t(th::kComDecide[type].delay * pace / 10);
}

void check_notes()
{
    using namespace th;
    AiFixture f;
    RivalHospital &r = f.rival();
    for (int i = 0; i < 8; i++)
        CHECK(make_a_note(r, 1, 1, int16_t(i), 5));
    CHECK(!make_a_note(r, 1, 2, 0, 0));
    CHECK(find_a_note(r, 1) && r.action.type == 1 && r.action.arg == 0 && r.action.count == 5);
    CHECK(r.action_from_note == 1);
    CHECK(!find_a_note(r, 2));
    // remove clears the type only.
    remove_a_note(r, 1, 1);
    CHECK(r.notes[0].type == 0 && r.notes[0].arg == 0 && r.notes[1].type == 1);
    // declass moves the first note of its type behind the others, and leaves it
    // as the pending action.
    r.notes = {};
    make_a_note(r, 1, 1, 0, 5);
    make_a_note(r, 1, 2, 5, 0);
    make_a_note(r, 1, 1, 1, 6);
    declass_a_note(r, 1, 1);
    CHECK(r.notes[0].type == 0 && r.notes[1].type == 2 && r.notes[2].arg == 1);
    CHECK(r.notes[7].type == 1 && r.notes[7].arg == 0 && r.notes[7].count == 5);
    CHECK(r.action.arg == 0 && r.action.count == 5);
    // A note not there: the pending action is stored as it stands.
    r.action = {3, 9, 9};
    declass_a_note(r, 1, 6);
    CHECK(r.notes[6].type == 3 && r.notes[6].arg == 9);
}

void check_decision_draws()
{
    using namespace th;
    AiFixture f;
    RivalHospital &r = f.rival();
    AiContext ctx = f.context();
    uint32_t seed = f.world->seed;
    int a = int(old_newrand(seed) % 3);
    int b = int(old_newrand(seed) % 5);
    int pace = std::clamp(70 - a + b, 1, 100);
    // Step 1: employ a receptionist (kind 3).
    computer_decision(ctx, 1);
    CHECK(f.world->seed == seed);
    CHECK(r.strategy_step == 2 && r.action.type == 1 && r.action.arg == 3 && r.action.count == 0);
    CHECK(r.next_action_turn == delay_for(25, 1, pace));
    // Step 6 opens the hospital and announces it, with the two draws and no
    // more.
    r.strategy_step = 6;
    seed = f.world->seed;
    old_newrand(seed);
    old_newrand(seed);
    CHECK(!f.arrivals.hospitals[1].open);
    computer_decision(ctx, 1);
    CHECK(f.arrivals.hospitals[1].open && f.world->announce[kAnnRivalOpened][0] == 1);
    CHECK(f.world->seed == seed && r.strategy_step == 7);
    // Step 10 switches to strategy 2, whose script starts at 12.
    r.strategy_step = 10;
    computer_decision(ctx, 1);
    CHECK(r.strategy == 2 && r.strategy_step == 12);
    // Step 16 restarts the strategy: back to step 12.
    r.strategy_step = 16;
    computer_decision(ctx, 1);
    CHECK(r.strategy_step == 12);
}

// The recheck of the hires: strategy 1 hired a receptionist (step 1) and a
// doctor (step 7).
void check_recheck()
{
    using namespace th;
    AiFixture f;
    RivalHospital &r = f.rival();
    AiContext ctx = f.context();
    // At step 5 (after the first recheck at 4): nobody hired.
    r.strategy_step = 5;
    CHECK(computer_recheck_actions(ctx, 0, 1) == 0);
    CHECK(r.action.type == 1 && r.action.arg == 3 && r.next_action_turn == 25);
    CHECK(r.recheck_step == 2 && r.strategy_step == 4);
    // The receptionist is there: the walk resumes after it and finds nothing.
    r.staff_by_kind[3] = 1;
    r.action = {};
    r.strategy_step = 5;
    computer_recheck_actions(ctx, 0, 1);
    CHECK(r.action.type == 0 && r.recheck_step == 0 && r.strategy_step == 5);
    // The doctor is missing at the second recheck (a = 1): back one step and
    // the position forgotten.
    r.strategy_step = 10;
    computer_recheck_actions(ctx, 1, 1);
    CHECK(r.action.type == 1 && r.action.arg == 1 && r.strategy_step == 9 && r.recheck_step == 0);
    r.staff_by_kind[1] = 1;
    r.action = {};
    r.strategy_step = 10;
    computer_recheck_actions(ctx, 1, 1);
    CHECK(r.action.type == 0 && r.strategy_step == 10);
}

void check_levels()
{
    using namespace th;
    AiFixture f;
    RivalHospital &r = f.rival();
    AiContext ctx = f.context();
    uint32_t seed = f.world->seed;
    // No patients: nothing to do, no draw.
    CHECK(computer_staff_levels(ctx, 1) == 0 && computer_object_levels(ctx, 1) == 0);
    CHECK(f.world->seed == seed);
    // Load 100: 100 / 55 = 1 nurse wanted. The count of the last action stays.
    r.patient_load = 100;
    r.action = {4, 0, 77};
    CHECK(computer_staff_levels(ctx, 1) == 1);
    int pace = std::clamp(70 - int(old_newrand(seed) % 3), 1, 100);
    CHECK(f.world->seed == seed);
    CHECK(r.action.type == 1 && r.action.arg == 0 && r.action.count == 77);
    CHECK(r.next_action_turn == delay_for(25, 1, pace));
    // A noted hire takes precedence and is flagged.
    make_a_note(r, 1, 1, 1, 16);
    CHECK(computer_staff_levels(ctx, 1) == 1);
    CHECK(r.action.arg == 1 && r.action.count == 16 && r.action_from_note == 1);
    // Objects: drinks (7) wanted 100 / 10 = 10, capped at 4 to place.
    r.action_from_note = 0;
    seed = f.world->seed;
    CHECK(computer_object_levels(ctx, 1) == 1);
    pace = std::clamp(70 - int(old_newrand(seed) % 3), 1, 100);
    CHECK(f.world->seed == seed);
    CHECK(r.action.type == 3 && r.action.arg == 7 && r.action.count == 4);
    CHECK(r.next_action_turn == delay_for(25, 3, pace));
    r.object_stats[7].count = 9;
    r.action = {};
    CHECK(computer_object_levels(ctx, 1) == 1 && r.action.arg == 7 && r.action.count == 1);
}

void check_loan_and_land()
{
    using namespace th;
    AiFixture f;
    RivalHospital &r = f.rival();
    AiContext ctx = f.context();
    r.max_loan = 15000;
    r.balance = 10000;
    // The branches keep each other's bodies: "repaying" borrows, with a loan.
    CHECK(computer_loan(ctx, -1, 1) == 0);
    r.loan = 5000;
    CHECK(computer_loan(ctx, -1, 1) == 1 && r.balance == 15000 && r.loan == 10000);
    // "Borrowing" pays back, when 5000 under the limit.
    CHECK(computer_loan(ctx, 1, 1) == 1 && r.balance == 10000 && r.loan == 5000);
    r.loan = 11000;
    CHECK(computer_loan(ctx, 1, 1) == 0 && computer_loan(ctx, 0, 1) == 0);
    // maintain_loan: the loan branch needs a loan and over 6000.
    r.loan = 5000;
    r.balance = 6000;
    CHECK(computer_maintain_loan(ctx, 1) == 0);
    r.balance = 6001;
    CHECK(computer_maintain_loan(ctx, 1) == 1 && r.action.type == 4 && r.action.arg == -1);
    // A note 4 comes first, is removed and runs with its argument (0).
    make_a_note(r, 1, 4, 0, 0);
    CHECK(computer_maintain_loan(ctx, 1) == 1 && r.action.arg == 0 && r.notes[0].type == 0);
    // Land: the player has 20 tiles at 100 each, the rival 15.
    r.area = 15;
    r.balance = 499;
    CHECK(computer_buy_land(ctx, 1) == 0 && r.area == 15 && r.notes[0].type == 4);
    r.balance = 500;
    CHECK(computer_buy_land(ctx, 1) == 1 && r.area == 20 && r.balance == 0);
    CHECK(computer_buy_land(ctx, 1) == 0);
}

// who_owns_peep's bidding with every hospital's reputation for the illness at
// 0: no bid, so no roll against the total and the owner is a draw % 4.
void check_no_bids()
{
    using namespace th;
    constexpr uint8_t kIllness = 16;
    Balance settings;
    settings.gbv.alloc_delay = 0;
    settings.gbv.alloc_rand = 255;
    settings.gbv.alloc_ind_rep = 2;
    settings.expertise[kIllness].start_price = 1000;
    ArrivalState state;
    state.game_month = 12;
    for (HospitalMarket &h : state.hospitals) {
        h.open = true;
        h.reputation[kIllness] = 0;
        h.price[kIllness] = 1000;
        h.price_scale[kIllness] = 10000;
    }
    uint32_t seed = 0x1234567;
    for (;; seed++) {
        uint32_t probe = seed;
        if (old_newrand(probe) % 255 != 0)
            break;
    }
    uint32_t expected = seed;
    old_newrand(expected);
    int owner = int(old_newrand(expected) % 4);
    PeepOwnerPath path;
    CHECK(who_owns_peep(seed, state, settings, kIllness, true, &path) == owner);
    CHECK(seed == expected && path.bidding);
}

} // namespace

void test_computer_ai()
{
    check_notes();
    check_decision_draws();
    check_recheck();
    check_levels();
    check_loan_and_land();
    check_no_bids();
}
