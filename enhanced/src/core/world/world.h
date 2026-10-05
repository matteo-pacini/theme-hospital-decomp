#pragma once

#include "core/data/anims.h"
#include "core/data/level.h"
#include "core/render/iso.h"
#include "core/world/object_tables.h"

#include <array>
#include <cstdint>
#include <vector>

namespace th
{

// A coordinate on the map in 1/256 cell, as the game's MapPos: `tile` is the
// cell's column (for x) or row (for y), `frac` the position across it, 0x80
// its middle.
struct MapPos {
    uint8_t frac = 0;
    uint8_t tile = 0;
};

// Thing.type values.
constexpr uint8_t kThingFree = 0;
constexpr uint8_t kThingObject = 6;

// Entries of the things table, including the unused entry 0 (the game's
// thing[] holds 1500).
constexpr int kThingCount = 1500;

// The random seed setup_new_level (src/start.cpp) sets before loading a level.
constexpr uint32_t kLevelStartSeed = 0x546b2;

// One entry of the things table, with the fields of the game's Thing
// (include/hosp.h) that placing and drawing objects use.
struct Thing {
    // Links in the list of things on the cell at (x.tile, y.tile)
    // (MapCell.first); 0 ends the list.
    int16_t cell_prev = 0;
    int16_t cell_next = 0;
    MapPos x;
    MapPos y;
    // Current animation frame (AnimData::frames index).
    uint16_t sprite = 0;
    // kThingFree or kThingObject.
    uint8_t type = kThingFree;
    // Even state number; objects start in kObjects[subtype].initial_state.
    uint8_t state = 0;
    // Object kind (kObjects index).
    uint8_t subtype = 0;
    // The animation advances every anim_speed turns.
    uint8_t anim_speed = 0;
    // Nonzero: the frame is drawn mirrored.
    uint8_t flip = 0;
    // Direction the object faces, 0-3.
    uint8_t facing = 0;
    // Added to the draw-order bucket within the cell (see thing_bucket).
    int8_t depth = 0;
    // Player index.
    uint8_t owner = 0;
    // The element id drawn for each animation layer; objects set 10-12.
    std::array<uint8_t, 13> layers{};
    // Where a person stands while using the object.
    MapPos use_x;
    MapPos use_y;
    // Links in the list of objects of the same room (0: none, the level's
    // objects are in no room) and of the same kind and owner
    // (World::first_of_kind).
    uint16_t room_obj_prev = 0;
    uint16_t room_obj_next = 0;
    uint16_t type_obj_prev = 0;
    uint16_t type_obj_next = 0;
    // Cells a person uses the object from (0-2) and leaves to afterwards (3-5).
    std::array<uint16_t, 6> use_pos{};
    // The room the object is in, 0 for none.
    uint16_t room_id = 0;
    // The other half of a two-part object, 0 for none.
    uint16_t pair = 0;
    // How people use the object (kObjects[subtype].use_style).
    uint8_t use_style = 0;
    // Plants: 400 healthy, wilting below.
    int16_t plant_health = 0;
};

// The map and the things on it.
struct World {
    std::array<MapCell, kMapCells> map{};
    std::vector<Thing> things = std::vector<Thing>(kThingCount);
    // Per object kind, the first object of player 0's list of that kind
    // (Thing.type_obj_next), as the game's listhead[0].objects.
    std::array<uint16_t, kObjectKinds> first_of_kind{};
    // The simulation's random generator state (the game's seed).
    uint32_t seed = kLevelStartSeed;
};

// Advances `seed` as the game's old_newrand (src/rand.cpp) and returns its
// new value.
uint32_t old_newrand(uint32_t &seed);

// Resets `world` and builds a level's starting map with its objects, as the
// game's recreate_map (src/town.cpp) when a level starts: build_start_map,
// with the object a cell holds (all of player 0) placed as soon as the cell
// is copied (adjust_mapwho, place_object).
// So a cell's copy overwrites what objects on earlier cells marked on it
// (flags, invisible walls, a second part on its list), as in the game.
// Shadows are not computed. The random generator starts from `seed`.
void start_level(World &world, const Level &level, const AnimData &anims, uint32_t seed = kLevelStartSeed);

// Map pixel position (as cell_to_world) of the point a thing's frame is
// drawn from, as the game's do_mapwho at 64 x 32 cells: the cell's top corner
// moved by its position within the cell, so the middle of a cell (fractions
// 0x80) is the middle of its diamond.
inline Point thing_to_world(const Thing &t)
{
    Point p = cell_to_world(t.x.tile, t.y.tile);
    int dx = int(t.x.frac) - int(t.y.frac);
    int sum = int(t.x.frac) + int(t.y.frac);
    return {p.x + (dx >> 3), p.y + (sum >> 4)};
}

// The game's draw-order bucket of a thing within its cell (do_mapwho): the
// cell's things are drawn by ascending bucket and, within a bucket, from the
// end of the cell's list to its head.
inline int thing_bucket(const Thing &t)
{
    return ((int(t.x.frac) + int(t.y.frac)) >> 4) + t.depth;
}

} // namespace th
