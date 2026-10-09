// Rats: manage_rats' hole, the rat it sends between two and the player's
// shot at it (src/rats.cpp, io.cpp's EV_SHOOT_RAT).

#include "common.h"

#include "core/data/anims.h"
#include "core/world/game.h"
#include "core/world/rats.h"
#include "core/world/world.h"

#include <cstdint>
#include <memory>

namespace
{

// The middle of level 1's map, where the test's island of hospital 0's land
// sits (cell column + row * 128).
constexpr int kCentre = 64 * th::kMapSize + 64;

// One cell of hospital 0's land with a wall on each of its four sides (the
// north and west its own, the east the next cell's west wall and the south
// the next row's north wall), so that try_create_rathole finds a wall
// whichever side it walks.
void build_island(th::World &w)
{
    w.map[kCentre].owner_flags = 1;
    w.map[kCentre].wall_n = 1;
    w.map[kCentre].wall_w = 1;
    w.map[kCentre + 1].wall_w = 1;
    w.map[kCentre + th::kMapSize].wall_n = 1;
}

// manage_rats' first draw decides whether to try for a hole (always, the
// litter count being high); try_create_rathole's draw the side. The hole
// lands on the island and, in a north or west wall, replaces its litter.
void test_rats_hole(th::World &world)
{
    world.seed = 0x12345678;
    world.litter_count = 0xffff;
    uint32_t seed = world.seed;
    th::old_newrand(seed);
    auto side = uint8_t(th::old_newrand(seed) & 3);
    world.map[kCentre].litter_shadow = 3;
    world.rats.last_litter_tile = uint16_t(kCentre);
    th::manage_rats(world, th::AnimData{});
    CHECK(world.rats.hole_count == 1 && world.rats.holes[0].tile == kCentre && world.rats.holes[0].side == side);
    if (side == 0 || side == 3)
        CHECK((world.map[kCentre].litter_shadow & 0xf) == (side == 0 ? 7 : 8));
    else
        CHECK((world.map[kCentre].litter_shadow & 0xf) == 3);
}

// A second hole and a rat between them: it walks the cells, its step towards
// the next waypoint, and leaves through a hole or dies. Returns its id.
uint16_t test_rats_move(th::World &world, const th::AnimData &anims)
{
    world.map[kCentre + 1].wall_w = 0;
    world.rats.holes[0].side = 0;
    world.rats.holes[1] = th::RatHole{uint16_t(kCentre + 1), 0};
    world.rats.hole_count = 2;
    th::make_a_rat(world, anims, 0);
    uint16_t rat = 0;
    for (uint16_t id = 1; id < world.things.size(); id++)
        if (world.things[id].type == th::kThingRat)
            rat = id;
    CHECK(rat != 0 && world.things[rat].owner == 0 && world.things[rat].speed == 0x40);
    uint16_t was = world.things[rat].x.tile + (world.things[rat].y.tile << 7);
    for (int i = 0; i < 20 && world.things[rat].type == th::kThingRat; i++)
        th::rat_turn(world, anims, rat);
    CHECK(world.things[rat].type != th::kThingRat || world.things[rat].x.tile + (world.things[rat].y.tile << 7) != was);
    return rat;
}

// The player's shot: a miss counts the shot alone; a hit counts the kill,
// floats the number, leaves the rats' mess and removes the rat.
void test_rats_shot(const th::World &world, uint16_t rat)
{
    if (world.things[rat].type != th::kThingRat)
        return;
    th::Game game;
    game.world = world;
    th::shoot_rat(game, 0, 0, 0);
    CHECK(game.progress.award_counts.rat_shots == 1 && game.progress.award_counts.rats_killed == 0);
    CHECK(game.progress.rat_shots == 1 && game.progress.rats_killed == 0);
    th::shoot_rat(game, rat, 0, 1);
    CHECK(game.progress.award_counts.rat_shots == 2 && game.progress.award_counts.rats_killed == 1);
    CHECK(game.progress.rats_killed == 1 && !game.world.rat_sounds.empty());
    CHECK(game.world.things[rat].type != th::kThingRat);
    bool numfx = false;
    for (const th::Thing &t : game.world.things)
        numfx |= t.type == th::kThingNumfx;
    CHECK(numfx);
}

// LbArcTan's cardinal directions and diagonals, from src/asm/arctan.s: the
// table indexed by the smaller over the larger of dx and -dy, its 257th
// entry when they are equal.
void test_rats_atan()
{
    CHECK(th::lb_arc_tan(0, 0) == 0 && th::lb_arc_tan(0, 1) == 0x400 && th::lb_arc_tan(1, 0) == 0x200);
    CHECK(th::lb_arc_tan(0, -1) == 0 && th::lb_arc_tan(-1, 0) == 0x600);
    CHECK(th::lb_arc_tan(1, 1) == 0x300 && th::lb_arc_tan(-1, -1) == 0x700);
    CHECK(th::lb_arc_tan(1, -1) == 0x100 && th::lb_arc_tan(-1, 1) == 0x500);
}

} // namespace

void test_rats()
{
    test_rats_atan();
    auto world = std::make_unique<th::World>();
    build_island(*world);
    test_rats_hole(*world);
    // One looping frame whose last-frame flag is set, so a rat arriving out
    // of a hole scurries and one leaving goes.
    th::AnimData anims;
    anims.starts.resize(0x3c6);
    anims.frames.push_back(th::AnimFrame{0, 0, 0, 0, 1, 0});
    uint16_t rat = test_rats_move(*world, anims);
    test_rats_shot(*world, rat);
}
