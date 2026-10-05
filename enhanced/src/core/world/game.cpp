#include "core/world/game.h"

#include <array>

namespace th
{

namespace
{

constexpr std::array<int, 12> kCalendar = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Object states (the game's OS_*, include/enums.h) handled so far.
constexpr uint8_t kObjectStatic = 2;

} // namespace

int days_in_month(int month)
{
    return kCalendar[size_t(month % 12)];
}

void Game::start(const Level &level, int number, const Balance &settings, const AnimData &anims)
{
    int open_countdown = settings.gbv.open_countdown;
    anims_ = &anims;
    start_level(world, level, anims);
    date = {};
    turn_count = 1;
    paused = false;
    opening_countdown = open_countdown;
    this->open_countdown = open_countdown;
    open = open_countdown == 0;
    balance = size_t(number) < settings.towns.size() ? int(settings.towns[size_t(number)].start_cash) : 0;
    reputation = 500;
}

void Game::turn()
{
    if (paused)
        return;
    turn_count++;
    // The per-turn random values (rand_pct, queue_near_dist,
    // object_near_dist): unused so far, but drawn to keep the generator in
    // step with the game's.
    old_newrand(world.seed);
    old_newrand(world.seed);
    old_newrand(world.seed);

    if (turn_count % kTurnsPerDay == 0) {
        // daily_admin, then month_end and year_end (src/finance.cpp).
        date.day++;
        if (date.day > days_in_month(date.month)) {
            date.day = 1;
            date.month++;
            if (date.month >= 12) {
                date.month = 0;
                date.year++;
            }
        }
    }

    // The game holds the countdown while the advisor's opening message is
    // up (advisor.flags & 1); there is no advisor yet.
    if (!open && --opening_countdown <= 0) {
        opening_countdown = 0;
        open = true;
    }

    // process_things, after the turn starts.
    for (size_t i = 1; i < world.things.size(); i++) {
        Thing &t = world.things[i];
        if (t.type != kThingObject || t.state != kObjectStatic || t.anim_speed == 0)
            continue;
        // object_static (src/objects.cpp).
        if (turn_count % t.anim_speed == 0 && t.sprite < anims_->frames.size())
            t.sprite = anims_->frames[t.sprite].next;
    }
}

} // namespace th
