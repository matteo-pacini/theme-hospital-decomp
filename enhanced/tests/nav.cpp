#include "common.h"

#include "core/data/gamedata.h"
#include "core/data/level.h"
#include "core/world/nav.h"
#include "core/world/world.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace
{

// Whether (x1, y1) can be reached from (x0, y0) in steps wall_in_way allows.
bool reachable(const th::World &w, int x0, int y0, int x1, int y1)
{
    std::vector<bool> seen(th::kMapCells);
    std::vector<int> queue = {x0 + y0 * th::kMapSize};
    seen[size_t(queue[0])] = true;
    for (size_t i = 0; i < queue.size(); i++) {
        int x = queue[i] % th::kMapSize;
        int y = queue[i] / th::kMapSize;
        if (x == x1 && y == y1)
            return true;
        for (uint32_t d = 0; d < 4; d++) {
            int next = x + th::kAroundXy[d][0] + (y + th::kAroundXy[d][1]) * th::kMapSize;
            if (!th::wall_in_way(w, x, y, d) && !seen[size_t(next)]) {
                seen[size_t(next)] = true;
                queue.push_back(next);
            }
        }
    }
    return false;
}

// A route found by wallhug: it ends at its end and each waypoint is in
// sight of the one before.
bool route_walkable(const th::World &w, const th::WallhugPath &path)
{
    th::Waypoint from = path.start;
    for (int i = 0; i < path.count; i++) {
        if (!th::line_of_sight(w, from, path.route[size_t(i)]))
            return false;
        from = path.route[size_t(i)];
    }
    return path.count > 0 && from.x == path.end.x && from.y == path.end.y;
}

} // namespace

void test_wall_in_way()
{
    auto w = std::make_unique<th::World>();
    CHECK(th::wall_in_way(*w, 5, 0, 0) && th::wall_in_way(*w, 0, 5, 3));
    CHECK(th::wall_in_way(*w, 127, 5, 1) && th::wall_in_way(*w, 5, 127, 2));
    CHECK(!th::wall_in_way(*w, 126, 5, 1) && !th::wall_in_way(*w, 5, 1, 0));
    for (uint32_t d = 0; d < 4; d++)
        CHECK(!th::wall_in_way(*w, 5, 5, d));

    w->map[5 + 5 * 128].wall_n = 1; // between (5, 4) and (5, 5)
    w->map[8 + 5 * 128].wall_w = 1; // between (7, 5) and (8, 5)
    CHECK(th::wall_in_way(*w, 5, 5, 0) && th::wall_in_way(*w, 5, 4, 2) && !th::wall_in_way(*w, 5, 5, 2));
    CHECK(th::wall_in_way(*w, 8, 5, 3) && th::wall_in_way(*w, 7, 5, 1) && !th::wall_in_way(*w, 8, 5, 1));

    // An object's footprint blocks steps onto and off it, unless it is also
    // a use cell (flag 2).
    w->map[10 + 10 * 128].flags = 1;
    CHECK(th::wall_in_way(*w, 10, 10, 2) && th::wall_in_way(*w, 9, 10, 1) && th::wall_in_way(*w, 10, 9, 2));
    w->map[10 + 10 * 128].flags = 3;
    CHECK(!th::wall_in_way(*w, 10, 10, 2) && !th::wall_in_way(*w, 9, 10, 1));

    CHECK(th::free_orthog_line(*w, 3, 5, 7, 5) && !th::free_orthog_line(*w, 9, 5, 3, 5));
    CHECK(th::free_orthog_line(*w, 5, 1, 5, 4) && !th::free_orthog_line(*w, 5, 8, 5, 1));
    CHECK(th::line_of_sight(*w, {1, 1}, {4, 9}) && !th::line_of_sight(*w, {5, 9}, {5, 1}));
}

namespace
{

// A wall down column 64 from the top to row 125, open below: found while
// the wall is plain, but not when every cell along it has a spur: each
// wall follower runs out of its 252 corners before getting round
// (docs/BUGS.md).
void check_wallhug_comb(th::WallhugPath &path)
{
    auto comb = std::make_unique<th::World>();
    for (int y = 0; y <= 125; y++)
        comb->map[size_t(64 + y * 128)].wall_w = 1;
    path.start = {50, 70};
    path.end = {80, 70};
    CHECK(th::wallhug_trivial(*comb, path) != th::kWallhugFailed && route_walkable(*comb, path));
    for (int y = 1; y <= 125; y++) {
        comb->map[size_t(63 + y * 128)].wall_n = 1;
        comb->map[size_t(64 + y * 128)].wall_n = 1;
    }
    CHECK(reachable(*comb, 50, 70, 80, 70));
    CHECK(th::wallhug_trivial(*comb, path) == th::kWallhugFailed);

    // A*, the default planner, gets round the comb, ending at the goal.
    CHECK(th::astar_route(*comb, path) != th::kWallhugFailed && route_walkable(*comb, path));
    CHECK(path.count > 0 && path.route[path.count - 1].x == 80 && path.route[path.count - 1].y == 70);
    // ... and fails only where nothing gets through: a goal walled in.
    comb->map[size_t(80 + 70 * 128)].flags = 1; // an impassable footprint
    CHECK(th::astar_route(*comb, path) == th::kWallhugFailed && path.count == 0);
}

} // namespace

// The wall-hugging path finder round a wall, into a closed box, and past
// its corner limit.
void test_wallhug()
{
    auto w = std::make_unique<th::World>();
    // These check the game's own planner, not the default A*.
    w->path_finder = th::PathFinder::Original;
    th::WallhugPath path;
    path.start = {10, 10};
    path.end = {20, 15};
    CHECK(th::wallhug_tricky(*w, path) == 15 && path.count == 1 && route_walkable(*w, path));

    // A wall from (15, 5) to (15, 20) across the way.
    for (int y = 5; y <= 20; y++)
        w->map[size_t(15 + y * 128)].wall_w = 1;
    path.start = {10, 12};
    path.end = {20, 12};
    CHECK(th::wallhug_trivial(*w, path) != th::kWallhugFailed && route_walkable(*w, path));
    int trivial = path.count;
    CHECK(th::wallhug_tricky(*w, path) != th::kWallhugFailed && route_walkable(*w, path));
    CHECK(path.count <= trivial);

    th::OrthonavPath nav;
    th::orthonav_init(*w, nav, 10, 12, 20, 12);
    CHECK(nav.count == std::min<int>(path.count, 4) && nav.x0 == 10 && nav.y1 == 12 && nav.x1 == 20);
    for (int i = 0; i < nav.count; i++)
        CHECK(nav.steps[size_t(i)].x == path.route[size_t(i)].x && nav.steps[size_t(i)].y == path.route[size_t(i)].y);

    // A closed box round (40, 40): no route in or out. A failed plan keeps
    // its old destination.
    for (int i = 39; i <= 41; i++) {
        w->map[size_t(i + 39 * 128)].wall_n = 1;
        w->map[size_t(i + 42 * 128)].wall_n = 1;
        w->map[size_t(39 + i * 128)].wall_w = 1;
        w->map[size_t(42 + i * 128)].wall_w = 1;
    }
    path.start = {10, 40};
    path.end = {40, 40};
    CHECK(th::wallhug_tricky(*w, path) == th::kWallhugFailed);
    path.start = {40, 40};
    path.end = {10, 40};
    CHECK(th::wallhug_tricky(*w, path) == th::kWallhugFailed);
    th::orthonav_init(*w, nav, 40, 40, 10, 40);
    CHECK(nav.count == 0 && nav.x1 == 20);

    check_wallhug_comb(path);
}

// orthonav_whatnext steering a person round a wall one cell at a time, and
// its failures.
void test_orthonav_whatnext()
{
    auto w = std::make_unique<th::World>();
    for (int y = 5; y <= 20; y++)
        w->map[size_t(15 + y * 128)].wall_w = 1;
    th::Thing &p = w->things[1];
    p.type = th::kThingPatient;
    p.x.tile = 10;
    p.y.tile = 12;
    p.dest_x = 20;
    p.dest_y = 12;
    th::insert_node(*w, 1, p.x, p.y);
    int steps = 0;
    int dir = 0;
    while (steps < 200 && (dir = th::person_head_for(*w, 1)) >= 0 && dir < 4) {
        CHECK(!th::wall_in_way(*w, p.x.tile, p.y.tile, uint32_t(dir)));
        p.facing = uint8_t(dir);
        th::move_node(*w, 1, {0x80, uint8_t(p.x.tile + th::kAroundXy[size_t(dir)][0])},
                      {0x80, uint8_t(p.y.tile + th::kAroundXy[size_t(dir)][1])});
        steps++;
    }
    CHECK(dir == th::kNavArrived && p.x.tile == 20 && p.y.tile == 12);
    std::printf("orthonav: round the wall in %d steps\n", steps);

    // Walled in on its cell, or with the destination out of reach.
    w->map[size_t(30 + 30 * 128)].wall_n = 1;
    w->map[size_t(30 + 31 * 128)].wall_n = 1;
    w->map[size_t(30 + 30 * 128)].wall_w = 1;
    w->map[size_t(31 + 30 * 128)].wall_w = 1;
    th::move_node(*w, 1, {0x80, 30}, {0x80, 30});
    CHECK(th::person_head_for(*w, 1) == th::kNavWalledIn);
    th::move_node(*w, 1, {0x80, 50}, {0x80, 50});
    p.dest_x = 30;
    p.dest_y = 30;
    CHECK(th::person_head_for(*w, 1) == th::kNavNoPath);
}

namespace
{

// One run of test_walk_level1's walks, with A* or with the game's planner.
void walk_level1_run(th::World &w, const th::GameData &gd, const th::Level &level, bool astar)
{
    th::start_level(w, level, gd.anims());
    w.path_finder = astar ? th::PathFinder::AStar : th::PathFinder::Original;
    uint16_t id = 3; // after the two parts of the entrance door
    CHECK(w.things[id].type == th::kThingFree);
    th::Thing &p = w.things[id];
    p.type = th::kThingPatient;
    p.speed = 0x20;
    p.anim_speed = 1;
    int entrance = level.entrances[0][0];
    p.x = {0x80, uint8_t(entrance % th::kMapSize)};
    p.y = {0x80, uint8_t(entrance / th::kMapSize)};
    th::insert_node(w, id, p.x, p.y);
    th::play_frame(p, gd.anims(), th::kPatientAnims[0].walk);

    struct Leg {
        int x, y;
        th::WalkResult result;
        int turns;
    };
    const Leg legs[] = {
        {63, 78, th::WalkResult::Arrived, 305},
        {40, 60, th::WalkResult::NoPath, 1},
        {entrance % th::kMapSize, entrance / th::kMapSize, th::WalkResult::Arrived, 305},
    };
    uint32_t turn = 1;
    for (const Leg &leg : legs) {
        th::set_destination_coords(w, id, leg.x + leg.y * th::kMapSize);
        int turns = 0;
        bool consistent = true;
        th::WalkResult r;
        do {
            r = th::walk_turn(w, gd.anims(), id, ++turn);
            consistent &= cell_lists_consistent(w);
            turns++;
        } while (r == th::WalkResult::Walking && turns < 5000);
        CHECK(consistent);
        CHECK(r == leg.result && (astar ? turns <= leg.turns : turns == leg.turns));
        if (r == th::WalkResult::Arrived)
            CHECK(p.x.tile == leg.x && p.y.tile == leg.y && p.x.frac == 0x80 && p.y.frac == 0x80);
        std::printf("walk to %d,%d%s: result %d in %d turns\n", leg.x, leg.y, astar ? " (A*)" : "", int(r), turns);
    }
    if (!astar)
        CHECK(w.seed == 0x2b6c015b);
}

} // namespace

// With the game data: a patient walks on level 1 from where patients arrive
// (the level's entrance cell) in through the entrance door, and back out,
// in a fixed number of turns from the level's starting seed, keeping the
// cell lists consistent every turn; (40, 60), walled off from it, is out
// of reach.
void test_walk_level1(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto level = gd.load_level(1, &error);
    CHECK(level.has_value());
    if (!level)
        return;
    auto w = std::make_unique<th::World>();
    // Twice with the game's planner (exact turns and draws), then with A*,
    // whose walks must arrive and fail alike and take no longer.
    for (int run = 0; run < 3; run++)
        walk_level1_run(*w, gd, *level, run == 2);
}
