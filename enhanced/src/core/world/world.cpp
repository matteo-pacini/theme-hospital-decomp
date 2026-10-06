#include "core/world/world.h"

#include "core/data/balance.h"
#include "core/world/arrivals.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/trace.h"

#include <bit>

namespace th
{

namespace
{

constexpr uint8_t kObjLitterBomb = 17;
constexpr uint8_t kObjPlant = 45;
constexpr uint8_t kObjGatesOfHell = 48;
constexpr uint8_t kObjEntranceLeftDoor = 58;
constexpr uint8_t kObjEntranceRightDoor = 59;

// Anim start of a healthy plant; each wilting stage is one frame on.
constexpr uint16_t kPlantAnim = 0x3cf;
constexpr int16_t kPlantHealthy = 400;

// Map.flags bit for a cell holding something larger than one cell.
constexpr uint8_t kCellBig = 0x20;

// Block id of the invisible wall on a cell's north edge; +1 for the west
// edge, +2 for the second of two nested ones.
constexpr uint8_t kBlankWall = 0x8e;

// The game indexes cells as the object's cell plus an offset and does not
// check the result against the map's bounds; here cells off the map are
// skipped (marks) or refused (fit tests).
MapCell *cell_at(World &w, int index)
{
    return index >= 0 && index < kMapCells ? &w.map[size_t(index)] : nullptr;
}

uint8_t &wall(MapCell &c, int side)
{
    return side == 0 ? c.wall_n : c.wall_w;
}

// insert_blank_wall (src/build.cpp).
void insert_blank_wall(MapCell &c, int side)
{
    uint8_t &w = wall(c, side);
    if (w == 0)
        w = uint8_t(kBlankWall + side);
    else if (w == kBlankWall + side)
        w = uint8_t(kBlankWall + 2 + side);
}

// remove_blank_wall (src/build.cpp).
void remove_blank_wall(MapCell &c, int side)
{
    uint8_t &w = wall(c, side);
    if (w == kBlankWall + side)
        w = 0;
    else if (w == kBlankWall + 2 + side)
        w = uint8_t(kBlankWall + side);
}

const ShapeInfo &shape_of(int kind, int dir)
{
    return kShapes[kObjects[size_t(kind)].shapes[size_t(dir)]];
}

} // namespace

// can_place_object (src/objects.cpp) without force_place_objects.
bool can_place_object(World &w, int tile, int kind, int dir, int owner)
{
    const ShapeInfo &shape = shape_of(kind, dir);
    for (int i = 0; i < shape.count; i++) {
        const MapCell *c = cell_at(w, tile + shape.points[size_t(i)]);
        if (!c)
            return false;
        uint8_t walls = shape.point_wall_flags[size_t(i)];
        if ((c->wall_n && (walls & 1)) || (c->wall_w && (walls & 2)) || (c->flags & 7))
            return false;
        if (c->owner_flags != 1 << owner && kind != kObjLitterBomb)
            return false;
        if (w.map[size_t(tile)].owner_flags & kCellOutdoor)
            return false;
    }
    for (int i = 0; i < 3; i++) {
        const MapCell *c = cell_at(w, tile + shape.use_offsets[size_t(i)]);
        if (!c)
            return false;
        if ((c->wall_n && (shape.use_wall_flags & 1)) || (c->wall_w && (shape.use_wall_flags & 2)) || (c->flags & 7))
            return false;
    }
    return true;
}

namespace
{

// mark_object_shape: the hospital whose litter lies on `c`.
uint8_t mark_object_shape_litter_owner(const MapCell &c)
{
    if (c.owner_flags & 2)
        return 1;
    if (c.owner_flags & 4)
        return 2;
    return c.owner_flags & 8 ? 3 : 0;
}

} // namespace

void mark_object_shape(World &w, int tile, int kind, int dir, bool cleanse)
{
    const ShapeInfo &shape = shape_of(kind, dir);
    for (int i = 0; i < shape.count; i++) {
        int16_t p = shape.points[size_t(i)];
        bool use = false;
        for (int16_t u : shape.use_offsets)
            use |= u == p;
        MapCell *c = cell_at(w, tile + p);
        if (!c)
            continue;
        c->flags |= use ? 2 : 1;
        if (cleanse && (c->litter_shadow & 0xf))
            remove_litter(w, uint16_t(tile + p), mark_object_shape_litter_owner(*c));
    }
    for (int16_t u : shape.use_offsets)
        if (MapCell *c = cell_at(w, tile + u))
            c->flags |= 4;
    int8_t group = kObjects[size_t(kind)].walls;
    if (group != -1) {
        const ObjectWallsInfo &walls = kObjectWalls[size_t(group * 4 + dir)];
        for (int i = 0; i < walls.count; i++)
            if (MapCell *c = cell_at(w, tile + walls.pos[size_t(i)]))
                insert_blank_wall(*c, walls.side[size_t(i)]);
    }
}

// remove_object_shape (src/objects.cpp).
void remove_object_shape(World &w, int tile, int kind, int dir)
{
    const ShapeInfo &shape = shape_of(kind, dir);
    for (int i = 0; i < shape.count; i++)
        if (MapCell *c = cell_at(w, tile + shape.points[size_t(i)]))
            c->flags &= ~7;
    for (int16_t u : shape.use_offsets)
        if (MapCell *c = cell_at(w, tile + u))
            c->flags &= ~4;
    int8_t group = kObjects[size_t(kind)].walls;
    if (group != -1) {
        const ObjectWallsInfo &walls = kObjectWalls[size_t(group * 4 + dir)];
        for (int i = 0; i < walls.count; i++)
            if (MapCell *c = cell_at(w, tile + walls.pos[size_t(i)]))
                remove_blank_wall(*c, walls.side[size_t(i)]);
    }
}

namespace
{

// adjust_big_cell_flag (src/engine.cpp), for a cell holding only objects.
void adjust_big_cell_flag(World &w, int tile)
{
    MapCell &c = w.map[size_t(tile)];
    for (int id = c.first; id > 0; id = w.things[size_t(id)].cell_next) {
        const Thing &t = w.things[size_t(id)];
        if (t.type == kThingObject && shape_of(t.subtype, t.facing).count > 1) {
            c.flags |= kCellBig;
            return;
        }
    }
    c.flags &= ~kCellBig;
}

// compute_plant_frame (src/handyman.cpp).
uint16_t compute_plant_frame(const Thing &plant, const AnimData &anims)
{
    uint32_t f = anims.starts[kPlantAnim].frame;
    int stage = (kPlantHealthy - plant.plant_health) / 80;
    for (stage = stage > 4 ? 4 : stage; stage > 0; stage--)
        f = anims.frames[f].next;
    return uint16_t(f);
}

// place_existing_object's insert_node: at the head of the cell's list.
void place_existing_object_insert_node(World &w, uint16_t id, int tile)
{
    Thing &obj = w.things[id];
    MapCell &cell = w.map[size_t(tile)];
    if (cell.first) {
        obj.cell_next = cell.first;
        if (obj.cell_next > 0)
            w.things[size_t(cell.first)].cell_prev = int16_t(id);
    }
    cell.first = int16_t(id);
}

// place_existing_object's insert_object: at the head of the list of the room
// holding the cell, and of the owner's list of the kind.
void place_existing_object_insert_object(World &w, uint16_t id, int tile)
{
    Thing &obj = w.things[id];
    obj.room_id = room_at(w, uint16_t(tile));
    if (obj.room_id) {
        RoomInfo &room = w.rooms[obj.room_id];
        if (room.first_object) {
            obj.room_obj_next = room.first_object;
            w.things[room.first_object].room_obj_prev = id;
        }
        room.first_object = id;
    }
    uint16_t &first = w.first_of_kind[obj.subtype];
    if (first) {
        obj.type_obj_next = first;
        w.things[first].type_obj_prev = id;
    }
    first = id;
}

} // namespace

// place_existing_object (src/objects.cpp) without the hardware statistics,
// advert counts and the reception's room list, which are not kept.
void place_existing_object(World &w, const AnimData &anims, uint16_t id, int tile, int dir)
{
    Thing &obj = w.things[id];
    const ObjectInfo &control = kObjects[obj.subtype];
    const ShapeInfo &shape = shape_of(obj.subtype, dir);
    obj.state = control.initial_state;
    obj.facing = uint8_t(dir);
    size_t start = size_t(control.anim_start) + obj.facing;
    if (start < anims.starts.size()) {
        obj.sprite = uint16_t(anims.starts[start].frame);
        obj.flip = uint8_t(anims.starts[start].flip);
    }
    mark_object_shape(w, tile, obj.subtype, dir, true);
    for (size_t i = 0; i < obj.use_pos.size(); i++)
        obj.use_pos[i] = uint16_t(tile + shape.use_offsets[i]);
    obj.depth = shape.depth;
    obj.x.tile = uint8_t(tile % kMapSize);
    obj.y.tile = uint8_t(tile / kMapSize);
    obj.use_x.tile = uint8_t(obj.x.tile + shape.use_dx);
    obj.use_y.tile = uint8_t(obj.y.tile + shape.use_dy);
    obj.x.frac = obj.use_x.frac = 0x80;
    obj.y.frac = obj.use_y.frac = 0x80;

    place_existing_object_insert_node(w, id, tile);
    place_existing_object_insert_object(w, id, tile);
    if (control.category == 6 && obj.room_id)
        w.rooms[obj.room_id].radiators++;

    if (control.next_part != -1) {
        const NextPartInfo &part = kNextParts[size_t(control.next_part)];
        uint16_t other = place_object(
            w, anims, {uint16_t(tile + part.offset[size_t(dir)]), part.kind, part.dir[size_t(dir)]}, obj.owner, false);
        if (other) {
            w.things[other].pair = id;
            obj.pair = other;
        }
    }
    if (obj.subtype == kObjEntranceLeftDoor || obj.subtype == kObjEntranceRightDoor) {
        if (obj.facing == 1)
            if (MapCell *c = cell_at(w, obj.x.tile + (obj.y.tile << 7) + 0x7f))
                c->flags |= kCellBig;
    } else {
        adjust_big_cell_flag(w, tile);
    }
    if (obj.subtype == kObjPlant)
        obj.sprite = compute_plant_frame(obj, anims);
}

// place_object (src/objects.cpp), `force` standing for
// othervar.force_place_objects, as at level start: no people or picked-up
// objects exist, so check_no_peeps_in_way always passes and a new thing is
// always taken; check_room_access is taken to pass. Prices and strengths
// come from allhosps' object_stats, still zero then (init_player fills them
// after the map is built), so nothing is charged; later the strength is the
// level's StartStrength (World::settings; research does not raise it yet).
// The player's placing does those checks and the charge first
// (core/world/objects.cpp).
uint16_t place_object(World &w, const AnimData &anims, ObjectAt object, int owner, bool force)
{
    const int tile = object.pos, kind = object.kind, dir = object.dir;
    if (tile < 0 || tile >= kMapCells)
        return 0;
    if (!(kind == kObjGatesOfHell || force || can_place_object(w, tile, kind, dir, owner)))
        return 0;
    mark_object_shape(w, tile, kind, dir);
    remove_object_shape(w, tile, kind, dir);

    // allocate_thing(0).
    uint16_t id = 0;
    for (size_t i = 1; i < w.things.size(); i++)
        if (w.things[i].type == kThingFree) {
            id = uint16_t(i);
            break;
        }
    if (!id)
        return 0;
    Thing &obj = w.things[id];
    obj.type = kThingObject;
    obj.subtype = uint8_t(kind);
    obj.anim_speed = 2;
    obj.use_style = kObjects[size_t(kind)].use_style;
    obj.owner = uint8_t(owner);
    obj.strength = w.settings ? w.settings->objects[size_t(kind)].start_strength : 0;
    obj.times_used = 0;
    obj.layers[10] = 0;
    obj.layers[11] = uint8_t(old_newrand(w.seed) & 3);
    obj.layers[12] = uint8_t(old_newrand(w.seed) & 3);
    if (kind == kObjPlant)
        obj.plant_health = kPlantHealthy;
    trace_event("place_object %u kind=%d tile=%d dir=%d owner=%d", unsigned(id), kind, tile, dir, owner);
    place_existing_object(w, anims, id, tile, dir);
    return id;
}

void create_numfx(World &world, uint8_t type, uint16_t tile, uint8_t colour, uint8_t height)
{
    // allocate_thing(0x32).
    uint16_t id = 0;
    for (size_t i = 1; i < world.things.size() - 0x32; i++)
        if (world.things[i].type == kThingFree) {
            id = uint16_t(i);
            break;
        }
    if (!id)
        return;
    Thing &fx = world.things[id];
    fx.type = kThingNumfx;
    fx.numfx_type = type;
    fx.x = {0x80, uint8_t(tile % kMapSize)};
    fx.y = {0x80, uint8_t(tile / kMapSize)};
    fx.depth = 8;
    insert_node(world, id, fx.x, fx.y);
    fx.speed = uint8_t(old_normal_rand(world.seed, 0x16, 0x1e));
    fx.anim_speed = uint8_t(height + (old_newrand(world.seed) & 0xf));
    fx.subtype = colour;
}

void numfx_ages(World &world, uint16_t id)
{
    Thing &fx = world.things[id];
    if (fx.numfx_type == kNumfxMoney)
        fx.speed--;
    fx.speed--;
    if (int8_t(fx.speed) <= 0) {
        trace_event("remove_thing %u type=%u", unsigned(id), unsigned(fx.type));
        remove_node(world, id);
        fx = Thing{};
    }
}

namespace
{

// adjust_mapwho (src/town.cpp): places the object a level cell starts with
// (kind in the high byte of first, facing in the low) for player 0.
void adjust_mapwho(World &w, const AnimData &anims, int tile)
{
    MapCell &c = w.map[size_t(tile)];
    if (!c.first)
        return;
    int kind = uint16_t(c.first) >> 8;
    int dir = c.first & 0xff;
    c.first = 0;
    // Kinds and facings past the game's tables are not in its levels; the
    // game would read beyond them.
    if (kind >= kObjectKinds || dir >= 4)
        return;
    if (uint16_t id = place_object(w, anims, {tile, kind, dir}, 0, true))
        w.things[id].owner = 0;
}

} // namespace

uint32_t old_newrand(uint32_t &seed, std::source_location caller)
{
    return old_newrand(seed, caller.function_name());
}

uint32_t old_newrand(uint32_t &seed, const char *caller)
{
    seed = std::rotr(seed * 0x24a1u + 0x24dfu, 13);
    if (tracing_rand())
        trace_rand(caller, seed);
    return seed;
}

void start_level(World &world, const Level &level, const AnimData &anims, uint32_t seed)
{
    world.map = {};
    world.things.assign(kThingCount, Thing{});
    world.first_of_kind = {};
    world.placing = {};
    world.building = {};
    world.held_staff = 0;
    world.held_staff_tile = 0;
    world.seed = seed;
    build_start_map(world.map, level, [&](int cell) {
        world.map[size_t(cell)].first = level.tiles[size_t(cell)].first;
        adjust_mapwho(world, anims, cell);
    });
}

void start_entrance_rooms(World &world, const Level &level)
{
    world.rooms = {};
    std::array<uint16_t, 4> entrances{};
    uint16_t r = 1;
    for (size_t j = 0; j < entrances.size(); j++) {
        for (; r < kRoomCount; r++) {
            RoomInfo &room = world.rooms[r];
            if (room.type != kRoomFree)
                continue;
            room.type = kRoomEntrance;
            room.pos = room.door_tile = room.door_inside_tile = level.entrances[0][j];
            room.owner = 0;
            entrances[j] = r;
            break;
        }
    }
    world.entrances.fill(entrances);
}

uint16_t get_room_info(const World &world, uint8_t kind, uint8_t mode, uint8_t owner)
{
    uint16_t found = 0;
    for (uint16_t r = 1; r < kRoomCount; r++) {
        const RoomInfo &room = world.rooms[r];
        if (room.state < 0 || room.owner != owner || room.type != kind)
            continue;
        if (room.state == 0 || mode == 0x65)
            return r;
        if (mode == 0x64)
            found = r;
    }
    return found;
}

uint16_t get_room_info_rand(World &world, uint8_t kind, uint8_t mode, uint8_t owner)
{
    std::array<uint16_t, 30> list{};
    uint8_t count = 0;
    for (uint16_t r = 1; r < kRoomCount; r++) {
        const RoomInfo &room = world.rooms[r];
        if (room.state < 0 || room.owner != owner || room.type != kind || !(mode == 0x65 || room.state == 0))
            continue;
        if (count < list.size())
            list[count++] = r;
    }
    if (count == 0)
        return 0;
    return list[old_newrand(world.seed) % count];
}

uint16_t get_nearest_room_info(const World &world, const Thing &person, uint8_t kind)
{
    auto pos = uint16_t(person.x.tile + (person.y.tile << 7));
    uint16_t best = 0;
    uint16_t best_dist = 9999;
    for (uint16_t r = 1; r < kRoomCount; r++) {
        const RoomInfo &room = world.rooms[r];
        if (!(room.state == 0 || room.state == 1) || room.owner != person.owner || room.type != kind)
            continue;
        int dx = pos % 128 - room.door_tile % 128;
        int dy = pos / 128 - room.door_tile / 128;
        auto dist = uint16_t(dx * dx + dy * dy);
        if (dist < best_dist) {
            best = r;
            best_dist = dist;
        }
    }
    return best;
}

} // namespace th
