// Rats: manage_rats' hole, the rat it sends between two and the player's
// shot at it (src/rats.cpp, io.cpp's EV_SHOOT_RAT).

#include "common.h"

#include "core/data/anims.h"
#include "core/world/game.h"
#include "core/world/rats.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
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

namespace
{

// Starts level `number` with the game's own balance settings.
std::unique_ptr<th::Game> start_special_level_game(const th::GameData &gd, const char *dir, int number)
{
    std::string error;
    auto level = gd.load_level(number, &error);
    th::Balance settings;
    auto game = std::make_unique<th::Game>();
    if (!level || !th::load_balance(dir, number, th::kDefaultBalanceId, settings, &error))
        return nullptr;
    game->staff_market.ref_counts = th::reference_counts(gd.text());
    game->start(*level, number, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
    return game;
}

// How many of hospital 0's staff are of `kind`.
int count_placed_staff(const th::World &world, uint8_t kind)
{
    int n = 0;
    for (const th::Thing &t : world.things)
        n += t.type == th::kThingStaff && t.subtype == kind && t.owner == 0;
    return n;
}

// The training level: three consultant doctors (a psychiatrist, a surgeon
// and a researcher) on the first tile of the player's land passing the
// level's roll, the generators seeded from rat_fest_level[3].
void test_training_level(const th::GameData &gd, const char *dir)
{
    auto game = start_special_level_game(gd, dir, 5);
    CHECK(game);
    if (!game)
        return;
    const th::World &w = game->world;
    CHECK(w.rats.fest_level == 0 && game->safeseed != th::kLevelStartSeed);
    int doctors = 0;
    uint16_t skills = 0, first_tile = 0;
    for (const th::Thing &t : w.things) {
        if (t.type != th::kThingStaff || t.subtype != th::kStaffDoctor || t.owner != 0)
            continue;
        uint16_t tile = uint16_t(t.x.tile + (t.y.tile << 7));
        if (doctors++ == 0)
            first_tile = tile;
        CHECK(tile == first_tile && (t.skills & th::kSkillConsultant));
        skills |= t.skills;
    }
    CHECK(doctors == 3);
    CHECK((skills & th::kSkillPsychiatrist) && (skills & th::kSkillSurgeon) && (skills & th::kSkillResearcher));
}

// The rat bonus levels: litter, rat holes and handymen from
// rat_fest_level[1] and [2], the hospital open at once and no calls; the
// game ends when the litter falls to 4.
void test_rat_level(const th::GameData &gd, const char *dir, int number, uint8_t fest, int handymen)
{
    auto game = start_special_level_game(gd, dir, number);
    CHECK(game);
    if (!game)
        return;
    th::World &w = game->world;
    CHECK(w.rats.fest_level == fest && game->open && game->opening_countdown == 0);
    CHECK(w.litter_count > 4 && w.rats.hole_count > 0);
    CHECK(count_placed_staff(w, th::kStaffHandyman) == handymen);
    CHECK(th::make_a_playercall(w, 1, 0, 3, 0) == 0);
    th::rat_fest_control(*game);
    CHECK(!game->progress.rat_game_over);
    w.litter_count = 4;
    th::rat_fest_control(*game);
    CHECK(game->progress.rat_game_over);
}

// The weapon in hand goes with a shot and its shots run down.
void test_arm_rat_shot()
{
    auto world = std::make_unique<th::World>();
    world->rats.fest_powerup = 2;
    world->rats.powerup_shots = 2;
    world->rats.hit_margin = 8;
    th::Event shot{th::EV_SHOOT_RAT, 7, 0, 0x0100};
    th::arm_rat_shot(*world, shot);
    CHECK(shot.arg == 0x0102 && world->rats.powerup_shots == 1 && world->rats.fest_powerup == 2);
    th::arm_rat_shot(*world, shot);
    CHECK(shot.arg == 0x0102 && world->rats.powerup_shots == 0 && world->rats.fest_powerup == 0 &&
          world->rats.hit_margin == 2);
    th::Event other{th::EV_OPEN_BUILD_ROOM, 0, 0, 0x0100};
    world->rats.fest_powerup = 3;
    th::arm_rat_shot(*world, other);
    CHECK(other.arg == 0x0100);
    th::Event miss{th::EV_SHOOT_RAT, 0, 0, 0};
    th::arm_rat_shot(*world, miss);
    CHECK(miss.arg == 3 && miss.value == 0);
}

} // namespace

void test_special_levels(const char *dir)
{
    th::GameData gd;
    std::string error;
    if (!gd.load(dir, &error)) {
        CHECK(false);
        return;
    }
    test_arm_rat_shot();
    test_training_level(gd, dir);
    test_rat_level(gd, dir, 13, 1, 3);
    test_rat_level(gd, dir, 14, 2, 5);
}
