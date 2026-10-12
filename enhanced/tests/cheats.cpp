// The cheats (src/utils.cpp's cheat, the keypad's codes, src/rats.cpp's
// try_bonus_level with debug.force_bonus_level): what each does, that none
// does anything with cheats off, and that command_log replays them.

#include "common.h"

#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/research.h"
#include "core/world/snapshot.h"
#include "core/world/year_end.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace
{

constexpr uint32_t kMakeAvailable = 2;
constexpr uint32_t kEveryObject = 4;
constexpr uint32_t kAddCash = 8;
constexpr uint32_t kTakeCash = 0x10;

// Objects 1 to 0x3d that cannot be bought.
std::vector<int> unavailable(const th::Game &g)
{
    std::vector<int> kinds;
    for (int i = 1; i < 0x3e; i++)
        if (!th::equipment_available(g.world, i))
            kinds.push_back(i);
    return kinds;
}

// Without cheats_enabled every call does nothing; the Shift+1 command is the
// interface's.
void test_cheats_off(th::Game &g)
{
    const int balance = g.balance;
    const auto locked = unavailable(g);
    CHECK(!g.cheats_enabled);
    CHECK(!g.cheat(kAddCash) && !g.cheat(kEveryObject) && !g.cheat_rat() && !g.cheat_scroll());
    CHECK(g.balance == balance && unavailable(g) == locked);
}

// cheat(8) and cheat(0x10) move 1000 as kTxCheatMoney, which the month's
// income and expenditure leave out; cheat(2) opens the level's objects and
// cheat(4) every object.
void test_cheats_effects(th::Game &g)
{
    g.set_cheats_enabled(true);
    const int balance = g.balance;
    const int income = g.finance.income_this_month;
    const int spent = g.finance.expenditure_this_month;
    CHECK(g.cheat(kAddCash) && g.balance == balance + 1000);
    CHECK(g.cheat(kTakeCash) && g.balance == balance);
    CHECK(g.finance.income_this_month == income && g.finance.expenditure_this_month == spent);
    CHECK(g.finance.entry_count >= 1 && g.finance.entries[g.finance.entry_count - 1].type == th::kTxCheatMoney);

    const auto locked = unavailable(g);
    CHECK(!locked.empty());
    CHECK(g.cheat(kMakeAvailable));
    const auto after_level = unavailable(g);
    // Only what the level has was opened.
    for (int kind : locked) {
        const bool in_level = g.settings().objects[size_t(kind)].available_for_level;
        CHECK(in_level == !std::count(after_level.begin(), after_level.end(), kind));
    }
    CHECK(g.cheat(kEveryObject));
    CHECK(unavailable(g).empty());
    for (size_t i = 1; i < 0x3e; i++)
        CHECK(g.settings().objects[i].available_for_level == 1);

    g.world.scroll_min = 5;
    g.world.scroll_max = 100;
    CHECK(g.cheat_scroll() && g.world.scroll_min == 0 && g.world.scroll_max == 0x3fff);
}

// The keypad's second code forces the bonus level once: try_bonus_level
// draws it, clears the flag, and a level just started inherits it.
void test_force_bonus_level(th::Game &g, th::Game &next)
{
    th::try_bonus_level(g);
    CHECK(g.progress.bonus_level == 0);
    g.set_force_bonus_level(true);
    th::try_bonus_level(g);
    CHECK((g.progress.bonus_level == 13 || g.progress.bonus_level == 14) && !g.force_bonus_level);
    g.progress.bonus_level = 0;
    th::try_bonus_level(g);
    CHECK(g.progress.bonus_level == 0);
    CHECK(!next.force_bonus_level);
    next.carry_force_bonus_level(true);
    CHECK(next.force_bonus_level);
}

// A command_log's cheat lines, replayed on a fresh game, give its balance and
// objects.
void test_cheats_replay(const char *dir, const th::Game &g, int balance_after)
{
    QueueFixture f;
    CHECK(f.start(dir));
    if (!f.game)
        return;
    th::Game &copy = *f.game;
    std::string error;
    for (const std::string &line : g.command_log) {
        if (line.empty() || line[0] == '#')
            continue;
        const size_t at = line.find(' ');
        if (at == std::string::npos || line[at + 1] != '@')
            continue;
        CHECK(copy.replay_input(line.substr(at + 1), &error));
    }
    CHECK(copy.cheats_enabled && copy.balance == balance_after);
    CHECK(unavailable(copy).empty() && copy.world.scroll_max == 0x3fff);
}

// Both flags are in the snapshot: a debug report reproduces try_bonus_level.
void test_cheats_snapshot(const QueueFixture &f)
{
    std::vector<uint8_t> bytes;
    CHECK(th::save_snapshot(*f.game, bytes));
    auto copy = std::make_unique<th::Game>();
    std::string error;
    CHECK(th::load_snapshot(*copy, bytes, f.gd, &error));
    CHECK(copy->cheats_enabled == f.game->cheats_enabled && copy->force_bonus_level == f.game->force_bonus_level);
}

} // namespace

void test_cheats(const char *dir)
{
    QueueFixture f;
    QueueFixture other;
    CHECK(f.start(dir) && other.start(dir));
    if (!f.game || !other.game)
        return;
    th::Game &g = *f.game;
    test_cheats_off(g);
    test_cheats_effects(g);
    const int balance = g.balance;
    test_force_bonus_level(g, *other.game);
    test_cheats_replay(dir, g, balance);
    g.set_force_bonus_level(true);
    test_cheats_snapshot(f);
}
