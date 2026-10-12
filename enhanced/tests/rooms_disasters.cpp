// The random disasters (generate_disaster, disaster_peeps; src/disaster.cpp)
// and a room object's second part placed with no one in the way.
#include "common.h"
#include "rooms_internal.h"

#include "core/data/balance.h"
#include "core/data/level.h"
#include "core/ui/build_room.h"
#include "core/ui/buy_objects.h"
#include "core/ui/charts.h"
#include "core/ui/town_map.h"
#include "core/world/advisor.h"
#include "core/world/build.h"
#include "core/world/disaster.h"
#include "core/world/events.h"
#include "core/world/finance.h"
#include "core/world/litter.h"
#include "core/world/objects.h"
#include "core/world/quake.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace
{

// A seed whose next old_newrand draw has `kind` as its low two bits.
uint32_t seed_for_disaster(uint32_t kind)
{
    uint32_t x = 1;
    for (;; x++) {
        uint32_t r = x * 0x24a1u + 0x24dfu;
        if ((((r >> 13) | (r << 19)) & 3) == kind)
            return x;
    }
}

// The cold and hot heating disasters: each starts, breaks the heating (the
// player's set heating dropped while it is broken) and ends, restoring it.
void check_heating_disasters(th::Game &g, uint16_t launch, uint8_t initial_heating)
{
    th::World &w = g.world;
    // Kind 1 (cold).
    w.disaster_kind = 0;
    w.disaster_counter = launch;
    w.litter_count = 0;
    w.seed = seed_for_disaster(1);
    th::generate_disaster(g, 0);
    CHECK(w.disaster_kind == 1 && w.heating_broken);
    CHECK(g.finance.heating_level == 0 && w.saved_heating == initial_heating);
    CHECK(g.advisor.forced_advice_id == 0xb);
    CHECK(th::apply_town_event(g, g.land, th::Event{th::EV_SET_HEATING, 9, 0, 0}));
    CHECK(g.finance.heating_level == 0);
    while (w.disaster_kind != 0)
        th::generate_disaster(g, 0);
    CHECK(g.finance.heating_level == initial_heating && !w.heating_broken);
    CHECK(g.advisor.forced_advice_id == 0xc);
    // Kind 2 (hot).
    w.disaster_kind = 0;
    w.disaster_counter = launch;
    w.seed = seed_for_disaster(2);
    th::generate_disaster(g, 0);
    CHECK(w.disaster_kind == 2 && g.finance.heating_level == 10 && w.heating_broken);
    CHECK(g.advisor.forced_advice_id == 0xa);
    while (w.disaster_kind != 0)
        th::generate_disaster(g, 0);
    CHECK(g.finance.heating_level == initial_heating);
}

// A drawn heating disaster with no radiator is rejected.
void check_rejected_disaster(th::Game &g, uint16_t launch, uint8_t initial_heating)
{
    th::World &w = g.world;
    w.things[2].type = th::kThingFree;
    w.disaster_kind = 0;
    w.disaster_counter = launch;
    w.seed = seed_for_disaster(1);
    th::generate_disaster(g, 0);
    CHECK(w.disaster_kind == 0 && !w.heating_broken && g.finance.heating_level == initial_heating);
}

// The sick feeling with over 10 litter: the patients feel sick, then it ends.
void check_sick_disaster(th::Game &g, uint16_t launch)
{
    th::World &w = g.world;
    w.things[2].type = th::kThingObject;
    w.things[1].nausea = 0;
    w.litter_count = 11;
    w.disaster_kind = 0;
    w.disaster_counter = launch;
    w.seed = seed_for_disaster(3);
    th::generate_disaster(g, 0);
    CHECK(w.disaster_kind == 3);
    CHECK(w.things[1].nausea == uint16_t(g.settings().gbv.vomit_limit + 10));
    CHECK(g.advisor.forced_advice_id == 0xd);
    // Litter cleaned to 10 or fewer while it runs: the timer's double
    // decrement instead of the nausea being reapplied.
    w.litter_count = 0;
    int8_t timer = w.disaster_timer;
    th::generate_disaster(g, 0);
    CHECK(w.disaster_kind == 3 && w.disaster_timer == int8_t(timer - 2));
    while (w.disaster_kind != 0)
        th::generate_disaster(g, 0);
    CHECK(g.advisor.forced_advice_id == 0xe);
}

// The kinds that react to an earthquake shake and unshake.
void check_quake_peeps(th::Game &g)
{
    th::World &w = g.world;
    w.things[1].person_flags = 0;
    th::disaster_peeps(w, g.anims(), g.settings(), 1, -1, g.turn_count);
    CHECK(w.things[1].person_flags & th::kPfQuakeShaken);
    th::disaster_peeps(w, g.anims(), g.settings(), 2, -1, g.turn_count);
    CHECK(!(w.things[1].person_flags & th::kPfQuakeShaken));
}

} // namespace

// With the game data: the random disasters (generate_disaster) and what
// disaster_peeps does, on a level 1 game: a cold and a hot heating disaster
// start and end, restoring the heating; a drawn heating disaster with no
// radiator is rejected; the sick feeling with over 10 litter makes the
// patients about to vomit and ends; and a broken heating ignores the
// player's set heating.
void test_disasters_level1(const char *dir)
{
    ScriptedGame s;
    CHECK(s.start(dir, 1, ""));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    th::World &w = g.world;
    const uint16_t launch = g.settings().gbv.disaster_launch;
    const uint8_t initial_heating = g.finance.heating_level;
    // A patient and a corridor radiator to work on.
    const uint16_t patient = 1, radiator = 2;
    w.things[patient] = th::Thing{};
    w.things[patient].type = th::kThingPatient;
    w.things[patient].owner = 0;
    w.things[patient].health = 100;
    w.things[radiator] = th::Thing{};
    w.things[radiator].type = th::kThingObject;
    w.things[radiator].subtype = 44; // OBJ_RADIATOR
    w.things[radiator].owner = 0;
    check_heating_disasters(g, launch, initial_heating);
    check_rejected_disaster(g, launch, initial_heating);
    check_sick_disaster(g, launch);
    check_quake_peeps(g);
    std::printf("disasters: cold, hot, sick and rejected branches exercised\n");
}

namespace
{

// A cell near level 1's view start where the operating table (facing 0) and
// its lamp both fit; -1 if none.
int table_and_lamp_spot(th::World &w, const th::Level &level, const th::NextPartInfo &part, int table_kind)
{
    const int start = level.camera_start[0];
    for (int c = start - 20 * th::kMapSize; c < start + 20 * th::kMapSize; c++) {
        int at = c + part.offset[0];
        bool in_map = c >= 0 && c < th::kMapCells && at >= 0 && at < th::kMapCells;
        if (in_map && th::can_place_object(w, c, table_kind, 0, 0) &&
            th::can_place_object(w, at, part.kind, part.dir[0], 0))
            return c;
    }
    return -1;
}

} // namespace

// With the game data: a two-part object's second part (the operating table's
// lamp) is placed with it only while no one is in its way
// (place_object's check_no_peeps_in_way, run again for the part by
// place_existing_object); the first part is placed either way.
void test_second_part_people_in_the_way(const char *dir)
{
    constexpr int kTable = 30;
    int spot = -1, lamp = -1;
    for (int in_the_way = 0; in_the_way < 2; in_the_way++) {
        HireFixture f;
        CHECK(f.start(dir));
        if (!f.level)
            return;
        th::World &w = f.game->world;
        const th::NextPartInfo &part = th::kNextParts[size_t(th::kObjects[kTable].next_part)];
        if (!in_the_way) {
            spot = table_and_lamp_spot(w, *f.level, part, kTable);
            lamp = spot + part.offset[0];
        }
        CHECK(spot >= 0);
        if (spot < 0)
            return;
        if (in_the_way)
            CHECK(f.game->add_walker(lamp) != 0);
        uint16_t table = th::place_object(w, f.gd.anims(), {spot, kTable, 0}, 0, false);
        CHECK(table != 0);
        if (!table)
            return;
        if (in_the_way) {
            CHECK(w.things[table].pair == 0);
            CHECK(!th::no_peeps_in_way(w, lamp, part.kind, part.dir[0]));
        } else {
            CHECK(w.things[table].pair != 0 && w.things[w.things[table].pair].subtype == part.kind);
        }
    }
    std::printf("second part: placed with no one in the way, left out with someone\n");
}
