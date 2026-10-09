// The computer players' foundation: the script tables, init_player for a
// rival, and setup_computer_players (src/comtable.cpp, src/start.cpp,
// src/computer.cpp).

#include "common.h"

#include "core/world/computer.h"
#include "core/world/game.h"
#include "core/world/world.h"

#include <cstdint>
#include <memory>
#include <string>

namespace
{

// Level 1's computer players: computer[0..2] play in FULL01.SAM.
constexpr std::array<uint8_t, 3> kLevel1Players = {0, 1, 2};

// The tables as src/comtable.cpp lays them out: counts that tell how many
// entries of a row are used, and the strategies' start markers.
void check_tables()
{
    using namespace th;
    CHECK(find_strategy_start(1) == 1);
    CHECK(find_strategy_start(2) == 12);
    CHECK(find_strategy_start(3) == 0);
    CHECK(kComStrategy[17].type == 9);
    // Nothing before the end marker is one, and the scripts are in it.
    for (size_t i = 0; i < 17; i++)
        CHECK(kComStrategy[i].type != 9);
    CHECK(kComStrategy[find_strategy_start(1) + 5].type == 10);
    CHECK(kComDecide[0].delay == 33 && kComDecide[4].delay == 50 && kComDecide[5].delay == 0);
    for (const auto &order : kComRoomOrders)
        CHECK(order[size_t(order[0]) + 1] == 0 || size_t(order[0]) + 1 == order.size());
    CHECK(kComRoomOrders[0][1] == 27 && kComRoomOrders[1][12] == 18 && kComRoomOrders[2][4] == 22);
    CHECK(kStaffMaxNbs[0][0] == 55 && kStaffMaxNbs[3][9] == 4);
    CHECK(kObjectNbs[0][0] == 7 && kObjectNbs[1][0] == 4 && kObjectNbs[2][0] == 45 && kObjectNbs[3][0] == 43);
    CHECK(kObjectNbs[2][10] == 1);
}

// The level files leave the script of every computer player to the table:
// strategy 1, unknown byte 2.
void check_balance_defaults(const th::Balance &b)
{
    for (const th::Balance::Computer &c : b.computer)
        CHECK(c.strategy == 1 && c.unknown7 == 2);
    CHECK(b.computer[0].playing == 1);
}

// init_player for a rival at level 1.
void check_init_rival(const th::Game &g)
{
    using namespace th;
    const Balance &b = g.settings();
    for (size_t owner = 1; owner < g.world.rivals.size(); owner++) {
        const RivalHospital &r = g.world.rivals[owner];
        CHECK(r.balance == int32_t(b.towns[1].start_cash) && r.year_start_balance == r.balance);
        CHECK(r.max_loan == 15000 && r.loan == 0 && r.heating_level == 5);
        CHECK(r.score == 10000 && r.start_score == 10000 && r.market_share == 25);
        CHECK(r.cure_available[0] == 1 && r.cure_available[1] == 0);
        CHECK(r.object_stats[4].cost == b.objects[4].start_cost && r.object_stats[61].cost == b.objects[61].start_cost);
        CHECK(r.object_stats[4].count == 0);
        // Illness 5 is cured with a drug (the pharmacy cabinet), illness 2 not.
        CHECK(r.drug_effectiveness[5] == b.gbv.start_rating && r.drug_cost[5] == b.gbv.start_cost);
        CHECK(r.drug_effectiveness[2] == 0 && r.drug_cost[2] == 0);
        CHECK(r.discovered[1] == b.expertise[1].known && r.discovered[1] == 1);
        CHECK(r.patient_load == 0 && r.staff_count == 0 && r.wage_bill == 0);
        for (const ComputerBucket &bucket : r.buckets)
            CHECK(bucket.income == 0 && bucket.patients_done == 0 && bucket.room_rep_events[3] == 0);
    }
    CHECK(g.world.rivals[0].player_type == 0 && g.world.rivals[0].balance == 0);
}

// setup_computer_players for level 1: the first three computer[].playing
// entries, with no draw.
void check_level1_players(const HireFixture &f)
{
    using namespace th;
    const Game &g = *f.game;
    for (size_t owner = 1; owner < g.world.rivals.size(); owner++) {
        const RivalHospital &r = g.world.rivals[owner];
        CHECK(r.player_type == 2);
        CHECK(r.ai_index == kLevel1Players[owner - 1]);
        CHECK(r.strategy == 1 && r.strategy_step == 1 && r.recheck_step == 0);
        CHECK(r.area == g.finance.area);
        CHECK(r.next_action_turn == 0 && r.action.type == 0);
    }
    CHECK(g.world.rivals[0].player_type == 0);
    // Nothing between the level's start and the emergency set-up drew.
    auto world = std::make_unique<World>();
    world->seed = 0x12345678;
    setup_computer_players(*world, g.settings(), 1, 7, 100, 200);
    CHECK(world->seed == 0x12345678);
    CHECK(world->rivals[1].ai_index == 0 && world->rivals[2].ai_index == 1 && world->rivals[3].ai_index == 2);
    CHECK(world->rivals[3].area == 7);
}

// The random fill of setup_computer_players never runs on the game's own
// data: every FULL level has at least three entries playing (the 12 to 19
// the level files do not mention play by default), so its three
// hospitals take the first three and the draw loop has no one to fill.
void check_default_data_has_three_players(const char *dir)
{
    using namespace th;
    for (int level = 1; level <= 12; level++) {
        Balance b;
        std::string error;
        CHECK(load_balance(dir, level, kDefaultBalanceId, b, &error));
        int playing = 0;
        for (const Balance::Computer &c : b.computer)
            playing += c.playing != 0;
        CHECK(playing >= 3);
        auto world = std::make_unique<World>();
        world->seed = 0x2468ace0;
        setup_computer_players(*world, b, level, 0, 5, 9);
        CHECK(world->seed == 0x2468ace0);
        for (size_t owner = 1; owner < world->rivals.size(); owner++)
            CHECK(world->rivals[owner].player_type == 2 && world->rivals[owner].name[0] == '\0');
    }
}

// The random fill, with the data changed to reach it: one computer player
// playing, so two are drawn, each from (mouse + draw) % 20 until an unused
// entry comes up.
void check_random_fill()
{
    using namespace th;
    Balance b;
    for (Balance::Computer &c : b.computer)
        c.playing = 0;
    b.computer[7].playing = 1;
    b.computer[9].strategy = 2;
    auto world = std::make_unique<World>();
    world->seed = 0x13579bdf;
    uint32_t seed = world->seed;
    std::array<bool, 20> used{};
    used[7] = true;
    std::array<uint8_t, 2> expected{};
    for (uint8_t &pick : expected) {
        for (int tries = 0; tries < 1000; tries++) {
            uint8_t j = uint8_t((uint32_t(30 + 40) + old_newrand(seed)) % 20);
            pick = j;
            if (!used[j]) {
                used[j] = true;
                break;
            }
        }
    }
    setup_computer_players(*world, b, 1, 3, 30, 40);
    CHECK(world->rivals[1].ai_index == 7 && world->rivals[1].name[0] == '\0');
    CHECK(world->rivals[2].ai_index == expected[0] && world->rivals[3].ai_index == expected[1]);
    CHECK(world->rivals[2].player_type == 2 && world->rivals[3].player_type == 2);
    CHECK(world->rivals[2].strategy == b.computer[expected[0]].strategy);
    CHECK(world->rivals[2].strategy_step == 1);
    CHECK(world->seed == seed);
    CHECK(world->rivals[1].area == 3 && world->rivals[2].area == 0 && world->rivals[3].area == 0);

    // Every entry playing: the first three take the hospitals and the rest are
    // not drawn for.
    for (Balance::Computer &c : b.computer)
        c.playing = 1;
    auto crowded = std::make_unique<World>();
    setup_computer_players(*crowded, b, 1, 0, 0, 0);
    CHECK(crowded->rivals[1].ai_index == 0 && crowded->rivals[3].ai_index == 2);
    CHECK(crowded->seed == World{}.seed);

    // A rat level has no computer players.
    auto rats = std::make_unique<World>();
    setup_computer_players(*rats, b, 13, 0, 0, 0);
    CHECK(rats->rivals[1].player_type == 0 && rats->rivals[1].strategy_step == 0);
}

// Names come from the language file's computer names by ai_index.
void check_names(const th::GameData &gd, th::Game &g)
{
    using namespace th;
    name_computer_players(g.world.rivals, gd.text());
    CHECK(gd.text().size(TextSection::ComputerNames) >= 20);
    for (size_t owner = 1; owner < 4; owner++) {
        const RivalHospital &r = g.world.rivals[owner];
        CHECK(r.name[0] != '\0' && gd.text().at(TextSection::ComputerNames, r.ai_index) == r.name.data());
    }
    // A name already there stays.
    g.world.rivals[1].name[0] = 'X';
    name_computer_players(g.world.rivals, gd.text());
    CHECK(g.world.rivals[1].name[0] == 'X');
}

} // namespace

void test_computer_tables()
{
    check_tables();
    th::Balance b;
    check_balance_defaults(b);
    check_random_fill();
}

void test_computer_players(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    check_balance_defaults(f.game->settings());
    check_init_rival(*f.game);
    check_level1_players(f);
    check_names(f.gd, *f.game);
    check_default_data_has_three_players(dir);
}
