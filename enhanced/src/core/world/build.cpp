// Building rooms: the map queries other parts use and room_available (src/build.cpp,
// src/blueprnt.cpp). The rest is in build_*.cpp, which share build_internal.h.

#include "core/world/build.h"
#include "core/world/build_internal.h"

#include "core/data/blocks.h"
#include "core/world/finance.h"
#include "core/world/nav.h"
#include "core/world/objects.h"
#include "core/world/trace.h"
#include "core/world/world.h"
#include "core/world/wrap32.h"

namespace th
{

namespace
{

constexpr uint8_t kObjReceptionDesk = 11;

constexpr uint8_t kObjLitterBomb = 17;

constexpr uint8_t kObjGatesOfHell = 48;

} // namespace

void place_small_thing(World &w, int tile, int id, bool keep)
{
    if (!in_map(tile))
        return;
    MapCell &c = w.map[size_t(tile)];
    int16_t *link = &c.first;
    if (c.first > 0) {
        int t = c.first;
        while (w.things[size_t(t)].cell_next > 0)
            t = w.things[size_t(t)].cell_next;
        link = &w.things[size_t(t)].cell_next;
    }
    if (!keep || *link == 0) {
        *link = int16_t(-id);
        if (id >= 0x4b)
            c.flags |= kBlueprints[size_t(id)].flags;
    }
}

bool room_available(const World &world, int room_type)
{
    const RoomTypeInfo &type = kRoomTypeInfo[size_t(room_type) % kRoomTypes];
    for (int i = 0; i < type.compulsory_count; i++)
        if (!equipment_available(world, type.objects[size_t(i)]))
            return false;
    return true;
}

// remove_object (src/objects.cpp): takes the object off the map and frees
// it if it stands on `owner`'s land (or is the gates of hell or a litter
// bomb).
bool remove_object(World &w, uint16_t id, int owner)
{
    trace_event("remove_object %u owner=%d", unsigned(id), owner);
    const Thing &obj = w.things[id];
    const MapCell &c = w.map[size_t(obj.x.tile + (obj.y.tile << 7))];
    if (!(obj.subtype == kObjGatesOfHell || obj.subtype == kObjLitterBomb || c.owner_flags == shl32(1, owner)))
        return false;
    remove_object_from_map(w, id);
    w.things[id] = Thing{};
    return true;
}

uint16_t room_at(const World &world, uint16_t tile)
{
    int x = tile % 128, y = tile / 128;
    for (uint16_t r = 1; r < kRoomCount; r++) {
        const RoomInfo &room = world.rooms[r];
        if (room.type == kRoomFree)
            continue;
        int x0 = room.pos % 128, y0 = room.pos / 128;
        if (x0 > x || x0 + room.width - 1 < x || y0 > y || y0 + room.height - 1 < y)
            continue;
        return r;
    }
    return 0;
}

bool check_room_access(const World &world, int owner)
{
    int pos = world.rooms[world.entrances[size_t(owner) % 4][0]].door_tile;
    for (size_t r = 1; r < world.rooms.size(); r++) {
        const RoomInfo &room = world.rooms[r];
        if (room.type == kRoomFree || room.state == kRoomStateBlueprint || room.owner != owner ||
            room.type == kRoomEntrance || room.type == kRoomReception)
            continue;
        if (!points_connected(world, pos, room.door_tile))
            return false;
        pos = room.door_tile;
    }
    for (size_t i = 1; i < world.things.size(); i++) {
        const Thing &t = world.things[i];
        if (t.type == kThingObject && t.subtype == kObjReceptionDesk && t.owner == owner &&
            (!points_connected(world, pos, t.use_pos[0]) || !points_connected(world, pos, t.use_pos[1])))
            return false;
    }
    return true;
}

uint16_t find_small_thing(const World &world, int tile)
{
    int id = world.map[size_t(tile)].first;
    while (id > 0)
        id = world.things[size_t(id)].cell_next;
    return uint16_t(-id);
}

bool is_player_building(const World &world)
{
    if (world.building.build.draw_mode == 1)
        return true;
    for (int tile = 0; tile < kMapCells; tile++) {
        uint16_t small = find_small_thing(world, tile);
        if (small >= 1 && small <= 3)
            return true;
    }
    return false;
}

bool blueprint_floor_invalid(const World &world, int x, int y, uint8_t *flags)
{
    const BuildState &b = world.building.build;
    int tile = cell_index(x, y);
    return !(is_room_size_valid(world, b.x0, b.y0, b.x1, b.y1, flags) && is_valid_no_room(world, tile, flags) &&
             is_tile_valid(world, tile, kPlayer, flags)) ||
           is_tile_near_half_wall(world, x, y, b.x1, b.y1);
}

bool blueprint_outline_invalid(const World &world, int x, int y, uint8_t *flags)
{
    const BuildState &b = world.building.build;
    int tx = x, ty = y;
    int i = cell_index(x, y);
    bool fixed = (block_flags(wall_at(world, i, 0)) & 0x10) || (block_flags(wall_at(world, i, 1)) & 0x10);
    if (y == b.y1 + 1 || x == b.x1 + 1) {
        if (x == b.x1 + 1 && y != b.y1 + 1) {
            tx--;
        } else if (x != b.x1 + 1 && y == b.y1 + 1) {
            ty--;
        } else {
            if (!fixed)
                tx--;
            ty--;
        }
    }
    int tile = cell_index(tx, ty);
    return !(is_valid_no_room(world, tile, flags) && is_tile_valid(world, tile, kPlayer, flags) &&
             is_room_size_valid(world, b.x0, b.y0, b.x1, b.y1, flags)) ||
           is_tile_near_half_wall(world, x, y, b.x1, b.y1);
}

uint8_t blueprint_draw_flags(const World &world, uint8_t flags)
{
    const BuildState &b = world.building.build;
    if (b.draw_mode != 1)
        return flags;
    for (int y = b.y0; y <= b.y1; y++)
        for (int x = b.x0; x <= b.x1; x++)
            blueprint_floor_invalid(world, x, y, &flags);
    for (int y = b.y0; y <= b.y1 + 1; y++) {
        for (int x = b.x0; x <= b.x1 + 1; x++) {
            bool piece = y == b.y0 || (y == b.y1 + 1 ? x != b.x1 + 1 : x == b.x0 || x == b.x1 + 1);
            if (piece)
                blueprint_outline_invalid(world, x, y, &flags);
        }
    }
    return flags;
}

} // namespace th
