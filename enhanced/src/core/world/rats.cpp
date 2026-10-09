#include "core/world/rats.h"

#include "core/data/level.h"
#include "core/world/build.h"
#include "core/world/disaster.h"
#include "core/world/game.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

#include <array>
#include <cstdint>

namespace th
{

namespace
{

// SFX_* (include/enums.h).
constexpr uint8_t kSfxShotgun = 0x3f;
constexpr uint8_t kSfxDeadrat2 = 0x40;
constexpr uint8_t kSfxUzibrst = 0xd3;
constexpr uint8_t kSfxRocket = 0xd4;

// An explosion's animation (the game's 0x902, blast_things' debris); 0x902
// is the first frame of the rat game's rocket explosion.
constexpr uint16_t kRatExplosionAnim = 0x902;

// The rat frames: start[] indices of a rat coming out of a hole (0x3c4
// north, 0x3c5 west), going back in (0x3c2 north, 0x3c3 west) and the eight
// directions it scurries in (0x3ba onwards, plus the angle).
constexpr uint16_t kRatArriveN = 0x3c4;
constexpr uint16_t kRatArriveW = 0x3c5;
constexpr uint16_t kRatLeaveN = 0x3c2;
constexpr uint16_t kRatLeaveW = 0x3c3;
constexpr uint16_t kRatScurry0 = 0x3ba;

// allocate_thing (0x1b42c) with `reserve` things kept back at the end.
uint16_t allocate_thing(const World &world, size_t reserve)
{
    for (size_t i = 1; i + reserve < world.things.size(); i++)
        if (world.things[i].type == kThingFree)
            return uint16_t(i);
    return 0;
}

// remove_thing (0x199e8), traced as the game's is.
void remove_thing(World &world, uint16_t id)
{
    trace_event("remove_thing %u type=%u", unsigned(id), unsigned(world.things[id].type));
    remove_node(world, id);
    world.things[id] = Thing{};
}

// which_room (0x2384c): the room whose floor holds `tile`, 0 for a corridor
// cell. (A private copy of the game's function, as litter.cpp keeps one.)
uint16_t which_room(const World &world, uint16_t tile)
{
    int x = tile % kMapSize;
    int y = tile / kMapSize;
    for (uint16_t r = 1; r < kRoomCount; r++) {
        const RoomInfo &room = world.rooms[r];
        if (room.type == kRoomFree)
            continue;
        int x0 = room.pos % kMapSize, y0 = room.pos / kMapSize;
        if (x0 > x || x0 + room.width - 1 < x || y0 > y || y0 + room.height - 1 < y)
            continue;
        return r;
    }
    return 0;
}

// The map position as its 16-bit raw (the game's MapPos.raw): the fraction
// in the low byte, the tile in the high byte.
uint16_t raw_of(MapPos p)
{
    return uint16_t(p.frac) | uint16_t(uint16_t(p.tile) << 8);
}

MapPos pos_of(uint16_t raw)
{
    return MapPos{uint8_t(raw & 0xff), uint8_t(raw >> 8)};
}

// start[`anim`]'s frame and mirroring into `rat`.
void rat_play_frame(Thing &rat, const AnimData &anims, uint16_t anim)
{
    if (anim < anims.starts.size()) {
        rat.sprite = uint16_t(anims.starts[anim].frame);
        rat.flip = uint8_t(anims.starts[anim].flip);
    }
}

// saferand's rotation (src/rand.cpp): the game's second generator, used to
// scatter a special level's contents.
uint16_t rat_saferand(uint32_t &safeseed)
{
    safeseed = safeseed * 0x24a1u + 0x24dfu;
    safeseed = (safeseed >> 13) | (safeseed << 19);
    return uint16_t(safeseed);
}

// The game's rat_fest_level table (src/tables.cpp).
struct RatFestLevel {
    uint16_t litter_max;
    uint16_t litter_chance;
    uint16_t holes_max;
    uint16_t holes_chance;
    uint32_t seed;
    uint16_t staff_max;
    uint16_t staff_chance;
};
constexpr std::array<RatFestLevel, 4> kRatFestLevel = {{
    {0, 0, 0, 0, 0, 0, 0},
    {100, 10, 32, 10, 5684990, 3, 50},
    {50, 15, 32, 10, 2467894, 5, 30},
    {0, 0, 0, 0, 4598213, 3, 30},
}};

} // namespace

uint32_t lb_sqr_l(uint32_t n)
{
    if (n == 0)
        return 0;
    static constexpr std::array<uint16_t, 32> kTable = {
        0x0001, 0x0002, 0x0002, 0x0004, 0x0005, 0x0008, 0x000b, 0x0010, 0x0016, 0x0020, 0x002d,
        0x0040, 0x005a, 0x0080, 0x00b5, 0x0100, 0x016a, 0x0200, 0x02d4, 0x0400, 0x05a8, 0x0800,
        0x0b50, 0x1000, 0x16a0, 0x2000, 0x2d41, 0x4000, 0x5a82, 0x8000, 0xb504, 0xffff,
    };
    uint32_t b = 31 - uint32_t(__builtin_clz(n)); // bit index of the highest set bit
    uint32_t guess = kTable[b];
    for (;;) {
        uint32_t q = n / guess;
        if (q >= guess)
            return guess;
        guess = (guess + q) >> 1;
    }
}

namespace
{

// The arctangent table (src/asm/arctan.s, .L000cd500): table[i] for i =
// (smaller << 8) / larger, the angle in a quarter turn. i is 256 when the
// two are equal, so the table has 257 entries.
constexpr std::array<uint16_t, 257> kArcTanTable = {
    0x000, 0x001, 0x002, 0x003, 0x005, 0x006, 0x007, 0x008, 0x00a, 0x00b, 0x00c, 0x00d, 0x00f, 0x010, 0x011, 0x013,
    0x014, 0x015, 0x016, 0x018, 0x019, 0x01a, 0x01b, 0x01d, 0x01e, 0x01f, 0x020, 0x022, 0x023, 0x024, 0x026, 0x027,
    0x028, 0x029, 0x02b, 0x02c, 0x02d, 0x02e, 0x030, 0x031, 0x032, 0x033, 0x035, 0x036, 0x037, 0x038, 0x039, 0x03b,
    0x03c, 0x03d, 0x03e, 0x040, 0x041, 0x042, 0x043, 0x044, 0x046, 0x047, 0x048, 0x049, 0x04b, 0x04c, 0x04d, 0x04e,
    0x04f, 0x051, 0x052, 0x053, 0x054, 0x055, 0x057, 0x058, 0x059, 0x05a, 0x05b, 0x05c, 0x05e, 0x05f, 0x060, 0x061,
    0x062, 0x063, 0x065, 0x066, 0x067, 0x068, 0x069, 0x06a, 0x06b, 0x06d, 0x06e, 0x06f, 0x070, 0x071, 0x072, 0x073,
    0x074, 0x076, 0x077, 0x078, 0x079, 0x07a, 0x07b, 0x07c, 0x07d, 0x07e, 0x07f, 0x081, 0x082, 0x083, 0x084, 0x085,
    0x086, 0x087, 0x088, 0x089, 0x08a, 0x08b, 0x08c, 0x08d, 0x08e, 0x08f, 0x090, 0x091, 0x093, 0x094, 0x095, 0x096,
    0x097, 0x098, 0x099, 0x09a, 0x09b, 0x09c, 0x09d, 0x09e, 0x09f, 0x0a0, 0x0a1, 0x0a2, 0x0a3, 0x0a4, 0x0a5, 0x0a6,
    0x0a7, 0x0a7, 0x0a8, 0x0a9, 0x0aa, 0x0ab, 0x0ac, 0x0ad, 0x0ae, 0x0af, 0x0b0, 0x0b1, 0x0b2, 0x0b3, 0x0b4, 0x0b5,
    0x0b6, 0x0b6, 0x0b7, 0x0b8, 0x0b9, 0x0ba, 0x0bb, 0x0bc, 0x0bd, 0x0be, 0x0bf, 0x0bf, 0x0c0, 0x0c1, 0x0c2, 0x0c3,
    0x0c4, 0x0c5, 0x0c6, 0x0c6, 0x0c7, 0x0c8, 0x0c9, 0x0ca, 0x0cb, 0x0cb, 0x0cc, 0x0cd, 0x0ce, 0x0cf, 0x0d0, 0x0d0,
    0x0d1, 0x0d2, 0x0d3, 0x0d4, 0x0d4, 0x0d5, 0x0d6, 0x0d7, 0x0d8, 0x0d8, 0x0d9, 0x0da, 0x0db, 0x0dc, 0x0dc, 0x0dd,
    0x0de, 0x0df, 0x0df, 0x0e0, 0x0e1, 0x0e2, 0x0e2, 0x0e3, 0x0e4, 0x0e5, 0x0e5, 0x0e6, 0x0e7, 0x0e8, 0x0e8, 0x0e9,
    0x0ea, 0x0eb, 0x0eb, 0x0ec, 0x0ed, 0x0ed, 0x0ee, 0x0ef, 0x0ef, 0x0f0, 0x0f1, 0x0f2, 0x0f2, 0x0f3, 0x0f4, 0x0f4,
    0x0f5, 0x0f6, 0x0f6, 0x0f7, 0x0f8, 0x0f8, 0x0f9, 0x0fa, 0x0fa, 0x0fb, 0x0fc, 0x0fc, 0x0fd, 0x0fe, 0x0fe, 0x0ff,
    0x100,
};

} // namespace

// LbArcTan (0xcd3f4, src/asm/arctan.s): the game's arctangent as 1/2048 of a
// turn, the table indexed by the smaller over the larger of dx and -dy.
uint16_t lb_arc_tan(int32_t dx, int32_t dy)
{
    if (dx == 0 && dy == 0)
        return 0;
    if (dx >= 0) {
        if (dy >= 0) {
            if (dx < dy)
                return uint16_t(0x400 - kArcTanTable[(uint32_t(dx) << 8) / uint32_t(dy)]);
            return uint16_t(0x200 + kArcTanTable[(uint32_t(dy) << 8) / uint32_t(dx)]);
        }
        uint32_t n = uint32_t(-dy);
        if (uint32_t(dx) < n)
            return kArcTanTable[(uint32_t(dx) << 8) / n];
        return uint16_t(0x200 - int32_t(kArcTanTable[(n << 8) / uint32_t(dx)]));
    }
    uint32_t n = uint32_t(-dx);
    if (dy >= 0) {
        if (n < uint32_t(dy))
            return uint16_t(0x400 + kArcTanTable[(n << 8) / uint32_t(dy)]);
        return uint16_t(0x600 - int32_t(kArcTanTable[(uint32_t(dy) << 8) / n]));
    }
    uint32_t m = uint32_t(-dy);
    if (n < m)
        return uint16_t(0x800 - int32_t(kArcTanTable[(n << 8) / m]));
    return uint16_t(0x600 + kArcTanTable[(m << 8) / n]);
}

namespace
{

// Whether a wall byte `wall` is one try_create_rathole accepts: a wall that
// is not one of the hole floor sprites' pieces (0x8e-0x91).
bool rathole_wall_ok(uint8_t wall)
{
    return wall != 0 && (wall < 0x8e || wall > 0x91);
}

// Whether, from cell `cell` walking in `side`'s direction over hospital
// `owner`'s land, there is an accepted wall by the cell reached; `cell` is
// then that cell.
bool rathole_walk(const World &world, int &cell, uint8_t side, uint8_t owner)
{
    int dir = side == 0 ? -kMapSize : side == 1 ? 1 : side == 2 ? kMapSize : -1;
    for (; cell >= 0 && cell < kMapCells && world.map[size_t(cell)].owner_flags == 1u << owner; cell += dir) {
        switch (side) {
        case 0:
            if (rathole_wall_ok(world.map[size_t(cell)].wall_n))
                return true;
            break;
        case 1:
            if (cell + 1 < kMapCells && rathole_wall_ok(world.map[size_t(cell + 1)].wall_w))
                return true;
            break;
        case 2:
            if (cell + kMapSize < kMapCells && rathole_wall_ok(world.map[size_t(cell + kMapSize)].wall_n))
                return true;
            break;
        default:
            if (rathole_wall_ok(world.map[size_t(cell)].wall_w))
                return true;
            break;
        }
    }
    return false;
}

// try_create_rathole (0x7cd80): tries to make a rat hole for hospital
// `owner`: walks from World::Rats::last_litter_tile in a random direction
// over the owner's land to the first wall, and adds a hole there (40 at
// most) unless it is in a room or already has one. Holes on sides 0 and 3
// also get a floor sprite.
void try_create_rathole(World &world, uint8_t owner)
{
    uint8_t side = uint8_t(old_newrand(world.seed) & 3);
    if (world.rats.hole_count == World::Rats::kMaxHoles)
        return;
    int cell = int(world.rats.last_litter_tile);
    if (!rathole_walk(world, cell, side, owner))
        return;
    auto tile = uint16_t(cell);
    if (which_room(world, tile) != 0)
        return;
    for (int i = world.rats.hole_count - 1; i >= 0; i--)
        if (world.rats.holes[size_t(i)].tile == tile)
            return;
    uint8_t slot = world.rats.hole_count++;
    world.rats.holes[slot].tile = tile;
    world.rats.holes[slot].side = side;
    trace_event("rathole %u owner=%u side=%u", unsigned(tile), unsigned(owner), unsigned(side));
    if (side == 0 || side == 3) {
        remove_litter(world, tile, owner);
        world.map[size_t(tile)].litter_shadow &= 0xf0;
        world.map[size_t(tile)].litter_shadow |= side == 0 ? 7 : 8;
    }
}

// remove_rand_rathole (0x7d114): removes one of hospital `owner`'s rat holes
// at random.
void remove_rand_rathole(World &world, uint8_t owner)
{
    if (world.rats.hole_count == 0)
        return;
    remove_rathole(world, owner, uint8_t(old_newrand(world.seed) % world.rats.hole_count));
}

// ratnav_init's arrival at (or short of) its destination: dives into the
// hole there (RAT_LEAVES) or vanishes.
void ratnav_finish(World &world, const AnimData &anims, uint16_t id)
{
    Thing &rat = world.things[id];
    for (int i = world.rats.hole_count - 1; i >= 0; i--) {
        if (world.rats.holes[size_t(i)].tile != uint16_t(int16_t(rat.y.tile) * kMapSize + int16_t(rat.x.tile)))
            continue;
        switch (world.rats.holes[size_t(i)].side) {
        case 0:
            rat_play_frame(rat, anims, kRatLeaveN);
            break;
        case 3:
            rat_play_frame(rat, anims, kRatLeaveW);
            break;
        default:
            remove_thing(world, id);
            return;
        }
        rat.state = kRatLeaves;
        rat.y.frac = 0x80;
        rat.x.frac = rat.y.frac;
        rat.depth = -9;
        return;
    }
    remove_thing(world, id);
}

// ratnav_init's step towards its next waypoint.
void ratnav_step(World &world, const AnimData &anims, uint16_t id)
{
    Thing &rat = world.things[id];
    int32_t dx = int32_t(rat.path.steps[0].x) - rat.x.tile;
    int32_t dy = int32_t(rat.path.steps[0].y) - rat.y.tile;
    uint32_t sx = uint32_t(dx << 8), sy = uint32_t(dy << 8);
    rat.rat_ticks = int32_t(lb_sqr_l(sx * sx + sy * sy) / rat.speed);
    if (rat.rat_ticks == 0) {
        remove_thing(world, id);
        return;
    }
    rat.rat_dx = (dx << 16) / rat.rat_ticks;
    rat.rat_dy = (dy << 16) / rat.rat_ticks;
    rat_play_frame(rat, anims, uint16_t((lb_arc_tan(dx, dy) + 0x80) % 0x800 * 8 / 0x800 + kRatScurry0));
}

// ratnav_init (0x7d760): plans a rat's next leg: at its destination it dives
// into the hole there (RAT_LEAVES) or vanishes; otherwise sets its step and
// sprite towards the next waypoint.
void ratnav_init(World &world, const AnimData &anims, uint16_t id)
{
    Thing &rat = world.things[id];
    nav_init(world, rat);
    if ((rat.x.tile == rat.dest_x && rat.y.tile == rat.dest_y) || rat.path.count == 0) {
        ratnav_finish(world, anims, id);
        return;
    }
    if (rat.x.tile == rat.path.steps[0].x && rat.y.tile == rat.path.steps[0].y) {
        if (rat.path.count > 1 && (rat.x.tile != rat.path.steps[1].x || rat.y.tile != rat.path.steps[1].y)) {
            rat.path.steps[0] = rat.path.steps[1];
            rat.path.count = 1;
        } else {
            remove_thing(world, id);
            return;
        }
    }
    ratnav_step(world, anims, id);
}

} // namespace

void remove_rathole(World &world, uint8_t owner, uint8_t idx)
{
    RatHole &hole = world.rats.holes[idx];
    RatHole &last = world.rats.holes[world.rats.hole_count - 1];
    trace_event("rathole_remove %u owner=%u", unsigned(hole.tile), unsigned(owner));
    if (hole.side == 0 || hole.side == 3) {
        world.map[hole.tile].litter_shadow &= 0xf0;
        world.map[hole.tile].litter_shadow |= 0;
    }
    hole = last;
    last = RatHole{};
    world.rats.hole_count--;
}

void clear_cell_of_ratholes(World &world, uint8_t owner, uint16_t tile)
{
    for (int i = 0; i + 1 < world.rats.hole_count; i++) {
        if (tile == world.rats.holes[size_t(i)].tile) {
            remove_rathole(world, owner, uint8_t(i));
            break;
        }
    }
}

void make_a_rat(World &world, const AnimData &anims, uint8_t owner)
{
    uint32_t count = world.rats.hole_count;
    if (count < 2)
        return;
    uint16_t id = allocate_thing(world, 0x32);
    if (id == 0)
        return;
    Thing &rat = world.things[id];
    rat.type = kThingRat;
    rat.owner = owner;
    if (world.rats.next_rat_layer) {
        rat.layers[0] = world.rats.next_rat_layer;
        world.rats.next_rat_layer = 0;
    }
    uint32_t from = old_newrand(world.seed) % count;
    rat.x.tile = uint8_t(world.rats.holes[from].tile % kMapSize);
    rat.y.tile = uint8_t(world.rats.holes[from].tile / kMapSize);
    rat.x.frac = 0x80;
    rat.y.frac = 0x80;
    rat.speed = 0x40;
    uint32_t to = old_newrand(world.seed) % count;
    if (to == from)
        to = (to + 1) % count;
    rat.dest_x = uint8_t(world.rats.holes[to].tile % kMapSize);
    rat.dest_y = uint8_t(world.rats.holes[to].tile / kMapSize);
    trace_event("rat %u owner=%u from=%u to=%u", unsigned(id), unsigned(owner), unsigned(world.rats.holes[from].tile),
                unsigned(world.rats.holes[to].tile));
    insert_node(world, id, rat.x, rat.y);
    switch (world.rats.holes[from].side) {
    case 0:
        rat.state = kRatArrives;
        rat_play_frame(rat, anims, kRatArriveN);
        rat.depth = -9;
        break;
    case 3:
        rat.state = kRatArrives;
        rat_play_frame(rat, anims, kRatArriveW);
        rat.depth = -9;
        break;
    default:
        rat.state = kRatScurries;
        ratnav_init(world, anims, id);
        break;
    }
}

void manage_rats(World &world, const AnimData &anims)
{
    if (old_newrand(world.seed) % 100 < world.litter_count)
        try_create_rathole(world, 0);
    if (old_newrand(world.seed) % 4000 < 10)
        remove_rand_rathole(world, 0);
    if (old_newrand(world.seed) % 100 < 10)
        make_a_rat(world, anims, 0);
}

// rat_scurries (0x7d6cc): rat state RAT_SCURRIES: moves the rat a step
// towards its next waypoint, planning the next leg when it gets there.
void rat_turn(World &world, const AnimData &anims, uint16_t id)
{
    Thing &rat = world.things[id];
    switch (rat.state) {
    case kRatScurries: {
        MapPos x = pos_of(uint16_t(int32_t(rat.rat_dx >> 8) + int32_t(int16_t(raw_of(rat.x)))));
        MapPos y = pos_of(uint16_t(int32_t(rat.rat_dy >> 8) + int32_t(int16_t(raw_of(rat.y)))));
        move_node(world, id, x, y);
        if (--rat.rat_ticks == 0) {
            ratnav_init(world, anims, id);
        } else if (rat.sprite < anims.frames.size()) {
            rat.sprite = anims.frames[rat.sprite].next;
        }
        break;
    }
    case kRatArrives:
        // rat_arrives (0x7dc44): plays the rat coming out of its hole, then
        // scurries.
        if (rat.sprite < anims.frames.size()) {
            rat.sprite = anims.frames[rat.sprite].next;
            if (anims.frames[rat.sprite].flags & 1) {
                rat.depth = 0;
                rat.state = kRatScurries;
                ratnav_init(world, anims, id);
            }
        }
        break;
    case kRatLeaves:
        // rat_leaves (0x7dcb4): plays the rat going into its hole, then
        // removes it.
        if (rat.sprite < anims.frames.size()) {
            rat.sprite = anims.frames[rat.sprite].next;
            if (anims.frames[rat.sprite].flags & 1)
                remove_thing(world, id);
        }
        break;
    default:
        break;
    }
}

void cleanse_area(World &world, uint8_t owner, int x, int y, int width, int height)
{
    for (int i = world.rats.hole_count - 1; i >= 0; i--) {
        int tx = world.rats.holes[size_t(i)].tile % kMapSize;
        int ty = world.rats.holes[size_t(i)].tile / kMapSize;
        if (((x <= tx && x + width > tx) && y <= ty) && y + height > ty)
            remove_rathole(world, owner, uint8_t(i));
    }
    for (int tx = x; x + width > tx; tx++) {
        for (int ty = y; y + height > ty; ty++) {
            int cell = tx + ty * kMapSize;
            if (cell < 0 || cell >= kMapCells)
                continue;
            if (world.map[size_t(cell)].litter_shadow & 0xf)
                remove_litter(world, uint16_t(cell), owner);
        }
    }
}

void shoot_rat(Game &game, uint16_t rat_id, uint8_t powerup, uint8_t hit)
{
    World &world = game.world;
    if (rat_id >= world.things.size())
        return;
    Thing &rat = world.things[rat_id];
    switch (powerup) {
    case 1:
        world.rat_sounds.push_back(kSfxUzibrst);
        break;
    case 2:
        world.rat_sounds.push_back(kSfxRocket);
        break;
    case 3:
        create_explosion(world, game.anims(), uint16_t(rat.x.tile + (rat.y.tile << 7)), kRatExplosionAnim);
        break;
    default:
        world.rat_sounds.push_back(kSfxShotgun);
        break;
    }
    game.progress.award_counts.rat_shots++;
    game.progress.rat_shots++;
    trace_event("shoot_rat %u killed=%u powerup=%u shots=%u kills=%u", unsigned(rat_id), unsigned(hit),
                unsigned(powerup), unsigned(game.progress.award_counts.rat_shots),
                unsigned(game.progress.award_counts.rats_killed));
    if (hit) {
        game.progress.award_counts.rats_killed++;
        world.rat_sounds.push_back(kSfxDeadrat2);
        game.progress.rats_killed++;
        create_numfx(world, kNumfxNumber, int16_t(game.progress.rats_killed), uint16_t(rat.x.tile + (rat.y.tile << 7)),
                     colour_dark_blue(world), 0x20);
        place_litter(world, uint16_t(rat.x.tile + (rat.y.tile << 7)), rat.owner, 4);
        remove_thing(world, rat_id);
    }
}

void rat_fest_control(Game &game)
{
    World &world = game.world;
    if (!world.rats.fest_level) {
        world.rats.near_margin = 0x10;
        world.rats.hit_margin = 4;
        return;
    }
    world.rats.near_margin = 0x10;
    if (game.progress.rats_killed - world.rats.fest_kills_mark > 10) {
        world.rats.fest_kills_mark = game.progress.rats_killed;
        world.rats.next_rat_layer = 1;
        world.rats.fest_powerup = uint8_t(old_newrand(world.seed) % 3 + 1);
        switch (world.rats.fest_powerup) {
        case 1:
            world.rats.hit_margin = 4;
            world.rats.powerup_shots = 10;
            break;
        case 2:
            world.rats.hit_margin = 8;
            world.rats.powerup_shots = 5;
            break;
        case 3:
            world.rats.hit_margin = 0x10;
            world.rats.powerup_shots = 3;
            break;
        default:
            world.rats.fest_powerup = 0;
            world.rats.hit_margin = 2;
            world.rats.powerup_shots = 0;
            break;
        }
    }
    uint8_t count = uint8_t(old_newrand(world.seed) % 5 + 1);
    for (uint8_t i = 0; i < count; i++) {
        make_a_rat(world, game.anims(), 0);
        world.rats.fest_rats += count;
    }
    // The end of the game and the extra handymen are the rat bonus level's
    // mode, which the port does not run: its level selection has no special
    // levels. World::Rats::fest_level is only set by rat_fest_builder, which
    // nothing calls.
}

void rat_fest_builder(Game &game, StaffMarket &market, uint8_t level)
{
    World &world = game.world;
    const RatFestLevel &lvl = kRatFestLevel[level % kRatFestLevel.size()];
    uint32_t safeseed = lvl.seed;
    world.seed = lvl.seed;
    uint32_t holes = 0, litter = 0, staff = 0;
    for (int cell = 0; cell < kMapCells; cell++) {
        if (world.map[size_t(cell)].owner_flags != 1u << 0)
            continue;
        if (lvl.litter_max > litter && rat_saferand(safeseed) % lvl.litter_chance == 0) {
            place_litter(world, uint16_t(cell), 0, uint8_t(rat_saferand(safeseed) % 3 + 1));
            litter++;
        }
        if ((lvl.holes_max > holes && rat_saferand(safeseed) % lvl.holes_chance == 0) && world.rats.last_litter_tile) {
            try_create_rathole(world, 0);
            holes++;
        }
        if (lvl.staff_max > staff && rat_saferand(safeseed) % lvl.staff_chance == 0) {
            uint16_t id = place_staff_in_struct(world, market, game.settings(), 1, 0, game.turn_count);
            if (id) {
                place_staff_on_map(world, {game.anims(), game.settings(), game.turn_count}, cell, id, false);
                staff++;
            }
        }
    }
    world.rats.fest_level = level;
    world.rats.fest_powerup = 0;
    world.rats.near_margin = 0x10;
    world.rats.hit_margin = 4;
}

} // namespace th
