// Placing objects in a room: wall hugging, the use-cell reachability checks and
// the requester modes (src/build.cpp).

#include "core/world/build.h"
#include "core/world/build_internal.h"

#include "core/world/finance.h"
#include "core/world/nav.h"
#include "core/world/objects.h"
#include "core/world/trace.h"
#include "core/world/world.h"

namespace th
{

namespace
{

// SFX_* (include/enums.h).
constexpr uint8_t kSfxSwoosh = 0x22;

// room_nav (src/tables.cpp), by room type: the objects whose use cells must
// stay reachable from the room's door: door_objects through use cells 1 and
// 4, objects through 0 and 3, and 2 and 5.
struct RoomNav {
    uint8_t door_count;
    std::array<uint8_t, 10> door_objects;
    uint8_t count;
    std::array<uint8_t, 10> objects;
};

constexpr std::array<RoomNav, kRoomTypes> kRoomNav = {{
    {0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},        // 0
    {0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},        // 1
    {0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},        // 2
    {0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},        // 3
    {0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},        // 4
    {0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},        // 5
    {0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},        // 6
    {1, {6, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 2, {1, 2, 0, 0, 0, 0, 0, 0, 0, 0}},        // 7
    {2, {16, 18, 0, 0, 0, 0, 0, 0, 0, 0}, 2, {61, 56, 0, 0, 0, 0, 0, 0, 0, 0}},    // 8
    {1, {8, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 2, {8, 1, 0, 0, 0, 0, 0, 0, 0, 0}},        // 9
    {1, {30, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 5, {30, 31, 35, 33, 34, 29, 0, 0, 0, 0}}, // 10
    {1, {39, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1, {39, 0, 0, 0, 0, 0, 0, 0, 0, 0}},      // 11
    {2, {13, 16, 0, 0, 0, 0, 0, 0, 0, 0}, 2, {13, 0, 0, 0, 0, 0, 0, 0, 0, 0}},     // 12
    {2, {16, 14, 0, 0, 0, 0, 0, 0, 0, 0}, 1, {15, 0, 0, 0, 0, 0, 0, 0, 0, 0}},     // 13
    {1, {22, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1, {22, 0, 0, 0, 0, 0, 0, 0, 0, 0}},      // 14
    {1, {42, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1, {42, 0, 0, 0, 0, 0, 0, 0, 0, 0}},      // 15
    {1, {27, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1, {15, 0, 0, 0, 0, 0, 0, 0, 0, 0}},      // 16
    {1, {9, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1, {9, 0, 0, 0, 0, 0, 0, 0, 0, 0}},        // 17
    {1, {23, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 2, {23, 15, 0, 0, 0, 0, 0, 0, 0, 0}},     // 18
    {1, {25, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1, {15, 0, 0, 0, 0, 0, 0, 0, 0, 0}},      // 19
    {1, {26, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1, {26, 0, 0, 0, 0, 0, 0, 0, 0, 0}},      // 20
    {1, {24, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1, {24, 0, 0, 0, 0, 0, 0, 0, 0, 0}},      // 21
    {0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {37, 60, 56, 36, 0, 0, 0, 0, 0, 0}},    // 22
    {1, {46, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1, {15, 0, 0, 0, 0, 0, 0, 0, 0, 0}},      // 23
    {1, {47, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1, {47, 0, 0, 0, 0, 0, 0, 0, 0, 0}},      // 24
    {0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 3, {19, 10, 57, 0, 0, 0, 0, 0, 0, 0}},     // 25
    {0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},        // 26
    {2, {16, 20, 0, 0, 0, 0, 0, 0, 0, 0}, 1, {20, 0, 0, 0, 0, 0, 0, 0, 0, 0}},     // 27
    {1, {55, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 5, {40, 1, 2, 41, 55, 0, 0, 0, 0, 0}},    // 28
    {2, {51, 32, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},      // 29
    {1, {54, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1, {15, 0, 0, 0, 0, 0, 0, 0, 0, 0}},      // 30
}};

// convert_MW_to_xy (src/utils.cpp): a cell offset as a column and row
// offset, the column within -64..64.
void offset_to_xy(int offset, int8_t &x, int8_t &y)
{
    uint16_t pos = uint16_t(offset);
    int8_t lx = int8_t(pos % 128), ly = int8_t(pos / 128);
    if (lx > 0x40) {
        lx = int8_t(lx + 0x80);
        ly++;
    }
    x = lx;
    y = ly;
}

// statvar.object_bounds (measure_objects, src/utils.cpp): the extent of a
// kind's footprint facing `dir`, in columns and rows from its cell.
struct Bounds {
    int8_t min_x, min_y, max_x, max_y;
};

Bounds object_bounds(int kind, int dir)
{
    const ShapeInfo &shape = shape_of(kind, dir);
    Bounds b{100, 100, -100, -100};
    for (int k = 0; k < shape.count; k++) {
        int8_t x, y;
        offset_to_xy(shape.points[size_t(k)], x, y);
        b.min_x = std::min(b.min_x, x);
        b.min_y = std::min(b.min_y, y);
        b.max_x = std::max(b.max_x, x);
        b.max_y = std::max(b.max_y, y);
    }
    return b;
}

// The cells of a room for place_object_wall_hug: (x0, y0) its first, (x1,
// y1) one past its last.
struct HugRect {
    uint8_t x0, y0, x1, y1;
};

// place_object_wall_hug's choice of wall for the pointer at (x, y): the
// side (0 north, 1 east, 2 south, 3 west; only north and west unless
// `corner`) by which triangle of the room it is in, and the cell on it.
struct HugSide {
    int side;
    uint8_t x, y;
};

HugSide place_object_wall_hug_side(const RoomInfo &room, const HugRect &r, uint8_t x, uint8_t y, bool corner)
{
    int8_t dx = int8_t(x - r.x0), dy = int8_t(y - r.y0);
    bool below = room.width && dy < room.height * dx / room.width;
    bool below2 = room.width && dy < room.height - dx * room.height / room.width;
    if ((below && below2) || (!corner && below))
        return {0, x, r.y0};
    if ((below2 && !below) || (!corner && !below))
        return {3, r.x0, y};
    if (below && !below2)
        return {1, r.x1, y};
    return {2, x, r.y1};
}

// place_object_wall_hug's clamp of (sx, sy) so a footprint of bounds `b`,
// moved by (ox, oy), stays inside `r`.
void place_object_wall_hug_clamp(const Bounds &b, int8_t ox, int8_t oy, const HugRect &r, uint8_t &sx, uint8_t &sy)
{
    if (sx + b.min_x + ox < r.x0)
        sx = uint8_t(r.x0 - b.min_x - ox);
    if (sy + b.min_y + oy < r.y0)
        sy = uint8_t(r.y0 - b.min_y - oy);
    if (sx + b.max_x + ox >= r.x1)
        sx = uint8_t(r.x1 - b.max_x - 1 - ox);
    if (sy + b.max_y + oy >= r.y1)
        sy = uint8_t(r.y1 - b.max_y - 1 - oy);
}

} // namespace

int place_object_restrict_to_room(const World &world, const ObjectPlacing &p, int pos, int offset, int kind, int dir)
{
    const RoomInfo &room = world.rooms[p.room % kRoomCount];
    Bounds b = object_bounds(kind, dir);
    uint16_t at = uint16_t(pos + offset);
    int8_t x = int8_t(at % 128), y = int8_t(at / 128);
    int8_t room_x = int8_t(room.pos % 128), room_y = int8_t(room.pos / 128);
    int8_t new_x = int8_t(uint16_t(pos) % 128), new_y = int8_t(uint16_t(pos) / 128);
    if (x + b.max_x >= room_x + room.width)
        new_x = int8_t(new_x - (x + b.max_x - room_x - room.width + 1));
    if (x + b.min_x < room_x)
        new_x = int8_t(new_x + room_x - x - b.min_x);
    if (y + b.max_y >= room_y + room.height)
        new_y = int8_t(new_y - (y + b.max_y - room_y - room.height + 1));
    if (y + b.min_y < room_y)
        new_y = int8_t(new_y + room_y - y - b.min_y);
    return uint16_t(new_x + (new_y << 7));
}

int place_object_wall_hug(World &world, ObjectPlacing &p, int pos, int kind)
{
    const RoomInfo &room = world.rooms[p.room % kRoomCount];
    const WallHugInfo &hug = kWallHugs[size_t(kObjectWallHug[size_t(kind)]) % kWallHugs.size()];
    uint8_t x = uint8_t(uint16_t(pos) % 128), y = uint8_t(uint16_t(pos) / 128);
    uint8_t rx = uint8_t(room.pos % 128), ry = uint8_t(room.pos / 128);
    HugRect r{rx, ry, uint8_t(rx + room.width), uint8_t(ry + room.height)};
    HugSide at = place_object_wall_hug_side(room, r, x, y, hug.kind == 4);
    uint8_t sx = at.x, sy = at.y;
    uint8_t angle = hug.angle[size_t(at.side)];
    if (angle != p.dir)
        play(world, kSfxSwoosh);
    p.dir = angle;
    place_object_wall_hug_clamp(object_bounds(kind, angle % 4), 0, 0, r, sx, sy);
    int8_t part_index = kObjects[size_t(kind)].next_part;
    if (part_index != -1) {
        const NextPartInfo &part = kNextParts[size_t(part_index)];
        int8_t ox, oy;
        offset_to_xy(part.offset[angle % 4], ox, oy);
        place_object_wall_hug_clamp(object_bounds(part.kind, part.dir[angle % 4]), ox, oy, r, sx, sy);
    }
    return int16_t(sx + (sy << 7));
}

namespace
{

// check_object_accessible (src/build.cpp).
bool object_accessible(const World &w, int pos, int other, int target)
{
    if (pos == other)
        return points_connected(w, pos, target);
    return points_connected(w, pos, target) && points_connected(w, other, target);
}

// check_object_nav (src/build.cpp): an object of `kind` at `pos` that room
// type `type` needs stays usable from the room's door.
bool check_object_nav(const World &w, int kind, const ShapeInfo &shape, const RoomInfo &room, int pos, int type)
{
    const RoomNav &nav = kRoomNav[size_t(type) % kRoomTypes];
    const std::array<int16_t, 6> &u = shape.use_offsets;
    for (int i = 0; i < nav.door_count; i++) {
        if (kind == nav.door_objects[size_t(i)]) {
            if (!object_accessible(w, pos + u[1], pos + u[4], room.door_inside_tile))
                return false;
            break;
        }
    }
    for (int i = 0; i < nav.count; i++) {
        if (kind == nav.objects[size_t(i)]) {
            if (!object_accessible(w, pos + u[0], pos + u[3], room.door_inside_tile))
                return false;
            if (!object_accessible(w, pos + u[2], pos + u[5], room.door_inside_tile))
                return false;
            break;
        }
    }
    return true;
}

} // namespace

bool place_object_nav_check(World &world, uint16_t room_id, ObjectAt object, ObjectAt part2)
{
    const RoomInfo &room = world.rooms[room_id % kRoomCount];
    mark_object_shape(world, object.pos, object.kind, object.dir);
    if (part2.pos)
        mark_object_shape(world, part2.pos, part2.kind, part2.dir);
    bool ok = check_object_nav(world, object.kind, shape_of(object.kind, object.dir), room, object.pos, room.type) &&
              (!part2.pos ||
               check_object_nav(world, part2.kind, shape_of(part2.kind, part2.dir), room, part2.pos, room.type));
    for (uint16_t id = room.first_object; ok && id; id = world.things[id].room_obj_next) {
        const Thing &obj = world.things[id];
        if (obj.state != kObjectRemoved && !check_object_nav(world, obj.subtype, shape_of(obj.subtype, obj.facing),
                                                             room, (obj.y.tile << 7) + obj.x.tile, room.type))
            ok = false;
    }
    remove_object_shape(world, object.pos, object.kind, object.dir);
    if (part2.pos)
        remove_object_shape(world, part2.pos, part2.kind, part2.dir);
    return ok;
}

bool corridor_object_room_nav_check(World &world, int pos, int kind, int dir, uint16_t room_id)
{
    if (!place_object_nav_check(world, room_id, {pos, kind, dir}, {}))
        return false;
    const RoomInfo &room = world.rooms[room_id % kRoomCount];
    mark_object_shape(world, pos, kind, dir);
    bool ok = true;
    for_each_person_in(world, room_id, [&](uint16_t id) {
        const Thing &person = world.things[id];
        // LINKED_OBJECT: the object or door they use.
        uint16_t linked = 0;
        if (person.object && person.object < world.things.size()) {
            const Thing &o = world.things[person.object];
            if ((o.type == kThingObject || o.type == kThingDoor) && o.user == id)
                linked = person.object;
        }
        int from = linked ? world.things[linked].use_pos[person.type == kThingStaff ? 3 : 4]
                          : person.x.tile + (person.y.tile << 7);
        if (ok && !points_connected(world, from, room.door_inside_tile))
            ok = false;
    });
    remove_object_shape(world, pos, kind, dir);
    return ok;
}

bool room_has_compulsory_objects(const World &world, uint16_t room_id)
{
    const RoomInfo &room = world.rooms[room_id % kRoomCount];
    const RoomTypeInfo &type = kRoomTypeInfo[room.type % kRoomTypes];
    for (int i = type.compulsory_count - 1; i >= 0; i--) {
        bool found = false;
        for (uint16_t id = room.first_object; id && !found; id = world.things[id].room_obj_next)
            found = world.things[id].subtype == type.objects[size_t(i)];
        if (!found)
            return false;
    }
    return true;
}

void set_room_requester_mode(World &world, int mode)
{
    ObjectPlacing &p = world.placing;
    p.requester_mode = uint8_t(mode);
    switch (mode) {
    case 0:
        p.arg = 1u << kEditRoomFurnish | 1u << kEditRoomPickUp;
        p.hint = 0xa;
        break;
    case 2:
        p.arg = 1u << kEditRoomFurnish | 1u << kEditRoomPickUp | 1u << kEditRoomConfirm;
        p.hint = 0xb;
        break;
    case 4:
        p.arg = 1u << kEditRoomFurnish | 1u << kEditRoomPickUp;
        p.hint = 0xc;
        break;
    case 6:
    case 8:
        set_room_requester_mode_furnishing(world);
        break;
    case 0xa:
        p.arg = 1u << kEditRoomConfirm | 1u << kEditRoomFurnish | 1u << kEditRoomPickUp;
        p.hint = 0x10;
        p.room_type = kRoomCorridor;
        break;
    default:
        break;
    }
    if (mode == 8)
        p.hint = 0xf;
    if (p.requester == ObjectRequester::None)
        p.requester = ObjectRequester::EditRoom;
}

bool move_peep_from_blueprint(World &world, uint16_t id)
{
    Thing &peep = world.things[id];
    const BuildState &b = world.building.build;
    uint8_t x = peep.x.tile, y = peep.y.tile;
    if (!(x >= b.x0 && x <= b.x1 && y >= b.y0 && y <= b.y1))
        return false;
    std::array<uint8_t, 4> dist = {uint8_t(y - b.y0), uint8_t(b.x1 - x), uint8_t(b.y1 - y), uint8_t(x - b.x0)};
    for (;;) {
        uint8_t best = 0xff;
        int side = 0;
        for (int i = 3; i >= 0; i--) {
            if (dist[size_t(i)] < best) {
                best = dist[size_t(i)];
                side = i;
            }
        }
        if (best == 0xff)
            return false;
        dist[size_t(side)] = 0xff;
        if (try_move_peep_to_wall(world, peep, side)) {
            trace_event("bp_goto %u", unsigned(id));
            return true;
        }
    }
}

} // namespace th
