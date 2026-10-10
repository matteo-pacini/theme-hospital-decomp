#include "core/world/build.h"

#include "core/data/blocks.h"
#include "core/render/shadows.h"
#include "core/world/complex.h"
#include "core/world/events.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/objects.h"
#include "core/world/queue.h"
#include "core/world/rats.h"
#include "core/world/reception.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"
#include "core/world/wrap32.h"

#include <cstdlib>
#include <optional>

namespace th
{

namespace
{

// The local player; the engine has no other.
constexpr int kPlayer = 0;

constexpr uint8_t kObjDoor = 3;
constexpr uint8_t kObjReceptionDesk = 11;
constexpr uint8_t kObjLitterBomb = 17;
constexpr uint8_t kObjPlant = 45;
constexpr uint8_t kObjAutoAutopsy = 55;
constexpr uint8_t kObjGatesOfHell = 48;
constexpr uint8_t kObjSwingDoor = 52;
constexpr uint8_t kObjSwingDoor2 = 53;

// RoomInfo.type of a room destroyed by a machine disaster (ROOM_DESTROYED).
constexpr uint8_t kRoomDestroyed = 6;

// OS_STATIC, OS_IN_USE and OS_REMOVED (include/enums.h).
constexpr uint8_t kObjectStatic = 2;
constexpr uint8_t kObjectInUse = 4;
constexpr uint8_t kObjectRemoved = 0x12;

// SFX_* (include/enums.h).
constexpr uint8_t kSfxWrong2 = 0x08;
constexpr uint8_t kSfxBuildClick = 0x1e;
constexpr uint8_t kSfxSwoosh = 0x22;
constexpr uint8_t kSfxYesX = 0x41;
constexpr uint8_t kSfxDeBuild = 0x97;
constexpr uint8_t kSfxPickUpSmall = 0xba;

// PTR_* (include/enums.h): the pointers modify_blueprint shows.
constexpr uint8_t kPtrResizeRoom = 3;
constexpr uint8_t kPtrResizeYEdge = 5;
constexpr uint8_t kPtrResizeXEdge = 6;
constexpr uint8_t kPtrResizeCorner = 7;
constexpr uint8_t kPtrMoveRoom = 8;

// The floor block of a blueprint's cells (mark_blueprint_on_map).
constexpr uint8_t kBlueprintFloor = 0x17;

// Blocks of the invisible walls insert_blank_wall puts on a cell's north
// edge, +1 for its west edge.
constexpr uint8_t kBlankWall = 0x8e;

// forced_advice_id messages (language section 0x14).
constexpr uint16_t kAdviceRoomInvalid = 4;
constexpr uint16_t kAdviceRoomBlocks = 5;
constexpr uint16_t kAdviceDoorBlocks = 6;

// door_to_room (src/tables.cpp): by facing, the offset from a door's cell
// to the cell of the room it belongs to.
constexpr std::array<int16_t, 4> kDoorToRoom = {0, -1, -128, 0};

// fill_in_windows (src/tables.cpp): the block filling in a window piece met
// at a room's corner.
struct FillInWindow {
    uint8_t wall;
    uint8_t window;
};
constexpr std::array<FillInWindow, 10> kFillInWindows = {{
    {114, 112},
    {115, 113},
    {116, 112},
    {117, 113},
    {118, 112},
    {119, 113},
    {122, 120},
    {123, 121},
    {124, 120},
    {125, 121},
}};

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

// The game indexes cells as column + (row << 7) and offsets without
// checking the result against the map's bounds; here cells off the map are
// skipped (writes) or read as empty.
bool in_map(int index)
{
    return index >= 0 && index < kMapCells;
}

int cell_index(int x, int y)
{
    return x + (y << 7);
}

uint8_t wall_at(const World &w, int index, int side)
{
    if (!in_map(index))
        return 0;
    const MapCell &c = w.map[size_t(index)];
    return side == 0 ? c.wall_n : c.wall_w;
}

void set_flags(World &w, int index, uint8_t bits)
{
    if (in_map(index))
        w.map[size_t(index)].flags |= bits;
}

const ShapeInfo &shape_of(int kind, int dir)
{
    return kShapes[kObjects[size_t(kind)].shapes[size_t(dir)]];
}

void play(World &w, uint8_t sfx)
{
    w.building.sounds.push_back(sfx);
}

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

namespace
{

// find_bluep_who (src/build.cpp): the negated marker at the end of `tile`'s
// list, 0 for none or for markers past 0x4b.
int find_bluep_who(const World &w, int tile)
{
    int id = w.map[size_t(tile)].first;
    if (id > 0) {
        int t = id;
        while (w.things[size_t(t)].cell_next > 0)
            t = w.things[size_t(t)].cell_next;
        id = w.things[size_t(t)].cell_next;
    }
    return id < -0x4b ? 0 : id;
}

// insert_blank_wall and remove_blank_wall (src/build.cpp).
void insert_blank_wall(World &w, int index, int side)
{
    if (!in_map(index))
        return;
    MapCell &c = w.map[size_t(index)];
    uint8_t &wall = side == 0 ? c.wall_n : c.wall_w;
    if (wall == 0)
        wall = uint8_t(kBlankWall + side);
    else if (wall == kBlankWall + side)
        wall = uint8_t(kBlankWall + 2 + side);
}

void remove_blank_wall(World &w, int index, int side)
{
    if (!in_map(index))
        return;
    MapCell &c = w.map[size_t(index)];
    uint8_t &wall = side == 0 ? c.wall_n : c.wall_w;
    if (wall == kBlankWall + side)
        wall = 0;
    else if (wall == kBlankWall + 2 + side)
        wall = uint8_t(kBlankWall + side);
}

// The thing ids of the people (patients and staff) in room `room`, the
// game's room->first_person list, which the engine does not keep: people
// carry their room in Thing.room.
template <typename Each> void for_each_person_in(const World &w, uint16_t room, Each &&each)
{
    for (size_t i = 1; i < w.things.size(); i++) {
        const Thing &t = w.things[i];
        if ((t.type == kThingPatient || t.type == kThingStaff) && t.room == room)
            each(uint16_t(i));
    }
}

// is_tile_valid (src/blueprnt.cpp): only people stand on `tile`, its object
// flags are clear and it is `owner`'s indoor land. With `flags`
// (advisor.flags), sets kAdvisorBadPlacement where the game does: for an
// object that is not a person, and for land that is not `owner`'s indoors.
bool is_tile_valid(const World &w, int tile, int owner, uint8_t *flags = nullptr)
{
    if (!in_map(tile))
        return false;
    const MapCell &c = w.map[size_t(tile)];
    for (int id = c.first; id > 0; id = w.things[size_t(id)].cell_next) {
        const Thing &t = w.things[size_t(id)];
        if (t.type != kThingPatient && t.type != kThingStaff) {
            if (flags)
                *flags |= kAdvisorBadPlacement;
            return false;
        }
    }
    if (c.flags & 7)
        return false;
    if (!(c.owner_flags & kCellOutdoor) && (c.owner_flags & (1 << owner)))
        return true;
    if (flags)
        *flags |= kAdvisorBadPlacement;
    return false;
}

// is_valid_no_room (src/blueprnt.cpp); with `flags`, sets
// kAdvisorBadPlacement for a tile in a room.
bool is_valid_no_room(const World &w, int tile, uint8_t *flags = nullptr)
{
    if (room_at(w, uint16_t(tile)) == 0)
        return true;
    if (flags)
        *flags |= kAdvisorBadPlacement;
    return false;
}

// check_near_half_wall and is_tile_near_half_wall (src/blueprnt.cpp): a
// half-height wall (block flag 4) on cell (x, y)'s edges inside the
// rectangle whose far corner is (x1, y1), or on its neighbour's across the
// far edges. The game passes the coordinates as bytes.
bool check_near_half_wall(const World &w, int x, int y, int x1, int y1)
{
    int i = cell_index(uint8_t(x), uint8_t(y));
    if (y != y1 + 1 && (block_flags(wall_at(w, i, 1)) & 4))
        return true;
    if (x != x1 + 1 && (block_flags(wall_at(w, i, 0)) & 4))
        return true;
    return false;
}

bool is_tile_near_half_wall(const World &w, int x, int y, int x1, int y1)
{
    if (check_near_half_wall(w, x, y, x1, y1))
        return true;
    if (x == x1 && check_near_half_wall(w, x + 1, y, x1, y1))
        return true;
    if (x == x1 + 1 && check_near_half_wall(w, x - 1, y, x1, y1))
        return true;
    if (y == y1 && check_near_half_wall(w, x, y + 1, x1, y1))
        return true;
    if (y == y1 + 1 && check_near_half_wall(w, x, y - 1, x1, y1))
        return true;
    return false;
}

// is_room_size_valid (src/blueprnt.cpp); with `flags`, sets
// kAdvisorBlueprintSmall for a blueprint too small and clears it otherwise.
bool is_room_size_valid(const World &w, int x0, int y0, int x1, int y1, uint8_t *flags = nullptr)
{
    const RoomTypeInfo &type = kRoomTypeInfo[w.building.build.room_type % kRoomTypes];
    uint16_t width = uint16_t(x1 - x0 + 1), height = uint16_t(y1 - y0 + 1);
    bool ok = width >= type.min_width && height >= type.min_height;
    if (flags)
        *flags = ok ? uint8_t(*flags & ~kAdvisorBlueprintSmall) : uint8_t(*flags | kAdvisorBlueprintSmall);
    return ok;
}

// is_valid_room (src/blueprnt.cpp): big enough for the room's type, on no
// room and on cells is_tile_valid accepts away from half-height walls.
bool is_valid_room(const World &w, int x0, int y0, int x1, int y1, int owner)
{
    uint16_t width = uint16_t(x1 - x0 + 1), height = uint16_t(y1 - y0 + 1);
    const RoomTypeInfo &type = kRoomTypeInfo[w.rooms[w.building.build.room].type % kRoomTypes];
    if (width < type.min_width || height < type.min_height)
        return false;
    if (room_at(w, uint16_t((y0 << 7) + x0)) || room_at(w, uint16_t((x1 - 1) + ((y1 - 1) << 7))))
        return false;
    for (uint16_t y = uint16_t(y0); y < y0 + height; y++) {
        for (uint16_t x = uint16_t(x0); x < x0 + width; x++) {
            int tile = (y << 7) + x;
            if (room_at(w, uint16_t(tile)) || !is_tile_valid(w, tile, owner))
                return false;
            if (is_tile_near_half_wall(w, x, y, x1, y1))
                return false;
        }
    }
    return true;
}

// is_valid_door (src/blueprnt.cpp): the cell outside a door of `type` at
// `tile` is indoors and in no room.
bool is_valid_door(const World &w, int tile, int type)
{
    int pos = tile + kDoorTypes[size_t(type % 4 + 4)].door_offset;
    if (!in_map(pos))
        return false;
    return !(w.map[size_t(pos)].owner_flags & kCellOutdoor) && room_at(w, uint16_t(pos)) == 0;
}

// blank_wall_around_blueprint (src/blueprnt.cpp): invisible walls round the
// blueprint for path checks, or takes them down.
void blank_wall_around_blueprint(World &w, bool blank)
{
    const BuildState &b = w.building.build;
    for (int x = b.x0; x <= b.x1; x++) {
        if (blank) {
            insert_blank_wall(w, cell_index(x, b.y0), 0);
            insert_blank_wall(w, cell_index(x, b.y1 + 1), 0);
        } else {
            remove_blank_wall(w, cell_index(x, b.y0), 0);
            remove_blank_wall(w, cell_index(x, b.y1 + 1), 0);
        }
    }
    for (int y = b.y0; y <= b.y1; y++) {
        if (blank) {
            insert_blank_wall(w, cell_index(b.x0, y), 1);
            insert_blank_wall(w, cell_index(b.x1 + 1, y), 1);
        } else {
            remove_blank_wall(w, cell_index(b.x0, y), 1);
            remove_blank_wall(w, cell_index(b.x1 + 1, y), 1);
        }
    }
}

// check_room_access_with_blueprint (src/blueprnt.cpp).
bool check_room_access_with_blueprint(World &w)
{
    blank_wall_around_blueprint(w, true);
    bool ok = check_room_access(w, kPlayer);
    blank_wall_around_blueprint(w, false);
    return ok;
}

// The outline piece mark_blueprint_on_map and draw_blueprint_area put on
// cell (x, y) of the blueprint (x0, y0)-(x1, y1): 3 the top-left corner, 1
// a north edge, 2 a west edge, 0 none.
int outline_piece(int x, int y, int x0, int y0, int x1, int y1)
{
    if (y == y0)
        return x == x0 ? 3 : x == x1 + 1 ? 2 : 1;
    if (y == y1 + 1)
        return x == x1 + 1 ? 0 : 1;
    return x == x0 || x == x1 + 1 ? 2 : 0;
}

// The blueprint floor block mark_blueprint_on_map puts on cell (x, y) of the
// blueprint (x0, y0)-(x1, y1): the centre four cells of an even-sized one
// carry a centre mark.
uint8_t mark_blueprint_on_map_floor(int x, int y, int x0, int y0, int x1, int y1)
{
    int width = x1 - x0 + 1, height = y1 - y0 + 1;
    if (height % 2 != 0 || width % 2 != 0)
        return kBlueprintFloor;
    if (y == (height - 1) / 2 + y0 && x == (width - 1) / 2 + x0)
        return 0x26;
    if (y == (height - 1) / 2 + y0 && x == (width + 1) / 2 + x0)
        return 0x25;
    if (y == (height + 1) / 2 + y0 && x == (width - 1) / 2 + x0)
        return 0x24;
    if (y == (height + 1) / 2 + y0 && x == (width + 1) / 2 + x0)
        return 0x27;
    return kBlueprintFloor;
}

// mark_blueprint_on_map (src/blueprnt.cpp): the outline's markers and, with
// `overlay`, the blueprint floor (with a centre mark for even sizes).
void mark_blueprint_on_map(World &w, int x0, int y0, int x1, int y1, bool overlay)
{
    for (int y = y0; y <= y1 + 1; y++) {
        for (int x = x0; x <= x1 + 1; x++) {
            int piece = outline_piece(x, y, x0, y0, x1, y1);
            int i = cell_index(x, y);
            if (!in_map(i))
                continue;
            MapCell &c = w.map[size_t(i)];
            if (overlay && y != y1 + 1 && x != x1 + 1)
                c.floor_block = mark_blueprint_on_map_floor(x, y, x0, y0, x1, y1);
            if (piece) {
                if (!((block_flags(c.wall_n) & 0x10) || (block_flags(c.wall_w) & 0x10)))
                    place_small_thing(w, i, 0, false);
                place_small_thing(w, i, piece, false);
            }
        }
    }
}

// cleanse_area (src/rats.cpp) is in rats.cpp: it removes the rat holes and
// the litter on the box of cells.

// Room under the build state.
RoomInfo &built_room(World &w)
{
    return w.rooms[w.building.build.room % kRoomCount];
}

// commit_blueprint (src/blueprnt.cpp): the blueprint becomes the room's
// floor plan and doors are placed next.
bool commit_blueprint(Game &game)
{
    trace_event("commit_blueprint");
    World &w = game.world;
    BuildState &b = w.building.build;
    if (!is_valid_room(w, b.x0, b.y0, b.x1, b.y1, kPlayer)) {
        trace_event("blueprint_invalid");
        game.advisor.forced_advice_id = kAdviceRoomInvalid;
        w.building.mouse = BuildMouse::Blueprint;
        return false;
    }
    if (!check_room_access_with_blueprint(w)) {
        trace_event("blueprint_blocked");
        game.advisor.forced_advice_id = kAdviceRoomBlocks;
        w.building.mouse = BuildMouse::Blueprint;
        return false;
    }
    RoomInfo &room = built_room(w);
    b.door_kind = kRoomTypeInfo[b.room_type % kRoomTypes].door_kind;
    b.draw_mode = 0;
    b.saved_floor_block = w.map[size_t(cell_index(b.x0, b.y0))].floor_block;
    mark_blueprint_on_map(w, b.x0, b.y0, b.x1, b.y1, true);
    room.created_turn = game.turn_count;
    room.pos = uint16_t(b.x0 + (b.y0 << 7));
    room.width = int8_t(b.x1 - b.x0 + 1);
    room.height = int8_t(b.y1 - b.y0 + 1);
    cleanse_area(w, room.owner, room.pos % 128, room.pos / 128, room.width, room.height);
    blank_wall_around_blueprint(w, true);
    b.state = 1;
    set_room_requester_mode(w, 2);
    w.building.mouse = BuildMouse::PlaceDoor;
    for (int y = b.y0; y <= b.y1; y++)
        for (int x = b.x0; x <= b.x1; x++)
            w.map[size_t(cell_index(x, y))].litter_shadow &= 0xaf;
    return true;
}

// uncommit_blueprint (src/blueprnt.cpp): the blueprint off the map and back
// to dragging.
void uncommit_blueprint(World &w)
{
    trace_event("uncommit_blueprint");
    BuildState &b = w.building.build;
    blank_wall_around_blueprint(w, false);
    for (int x = b.x0; x <= b.x1 + 1; x++)
        for (int y = b.y0; y <= b.y1 + 1; y++)
            place_small_thing(w, cell_index(x, y), 0, false);
    for (int x = b.x0; x <= b.x1; x++)
        for (int y = b.y0; y <= b.y1; y++)
            w.map[size_t(cell_index(x, y))].floor_block = b.saved_floor_block;
    b.draw_mode = 1;
    b.state = 0;
}

// Where `tile` lies on the blueprint, for modify_blueprint.
struct BlueprintSpot {
    uint8_t x, y;
    bool on_x0, on_y0, on_x1, on_y1, inside;
};

// modify_blueprint with no drag going: the pointer for `at`, or the end of
// a drag.
void modify_blueprint_pointer(World &w, const BlueprintSpot &at)
{
    BuildState &b = w.building.build;
    if (b.drag != kDragNone) {
        if (b.drag <= kDragMove)
            b.drag = kDragNone;
        return;
    }
    if (!at.inside)
        w.building.pointer = kPtrResizeRoom;
    else if ((at.on_x0 && at.on_y0) || (at.on_x0 && at.on_y1) || (at.on_x1 && at.on_y0) || (at.on_x1 && at.on_y1))
        w.building.pointer = kPtrResizeCorner;
    else if (at.on_x0 || at.on_x1)
        w.building.pointer = kPtrResizeXEdge;
    else if (at.on_y0 || at.on_y1)
        w.building.pointer = kPtrResizeYEdge;
    else
        w.building.pointer = kPtrMoveRoom;
}

// modify_blueprint's first dragging call: what the drag does, by where it
// starts.
void modify_blueprint_start_drag(BuildState &b, const BlueprintSpot &at)
{
    b.draw_mode = 1;
    if (!at.inside) {
        b.anchor_x = b.x0 = b.x1 = at.x;
        b.anchor_y = b.y0 = b.y1 = at.y;
        b.drag = kDragCorner;
        return;
    }
    bool on_x = at.on_x0 || at.on_x1, on_y = at.on_y0 || at.on_y1;
    if (!on_x && !on_y) {
        b.anchor_x = at.x;
        b.anchor_y = at.y;
        b.drag = kDragMove;
        return;
    }
    // The corner or edge across from the one picked stays put; an edge
    // drag also keeps the far end of the picked edge (the pivot).
    b.anchor_x = at.on_x0 ? b.x1 : b.x0;
    b.anchor_y = at.on_y0 ? b.y1 : b.y0;
    if (on_x && on_y) {
        b.drag = kDragCorner;
        return;
    }
    b.pivot_x = at.on_x1 ? b.x0 : b.x1;
    b.pivot_y = at.on_y1 ? b.y0 : b.y1;
    b.drag = kDragEdge;
}

// modify_blueprint dragging an edge to (x, y): the anchored edge stays and
// the picked one follows.
void modify_blueprint_drag_edge(BuildState &b, uint8_t x, uint8_t y)
{
    if (b.anchor_y == b.pivot_y) {
        b.x0 = b.anchor_x;
        b.y0 = y < b.anchor_y ? y : b.anchor_y;
        b.x1 = b.pivot_x;
        b.y1 = y > b.anchor_y ? y : b.anchor_y;
    } else {
        b.x0 = b.anchor_x < x ? b.anchor_x : x;
        b.y0 = b.anchor_y;
        b.x1 = b.anchor_x > x ? b.anchor_x : x;
        b.y1 = b.pivot_y;
    }
}

// modify_blueprint (src/blueprnt.cpp): while `dragging`, the first call
// picks what the drag does by where it starts (outside: a new rectangle; a
// corner or edge: resizing; inside: moving) and later ones do it; otherwise
// it ends a drag or picks the pointer for one from `tile`.
void modify_blueprint(World &w, bool dragging, uint16_t tile)
{
    BuildState &b = w.building.build;
    trace_event("blueprint x0=%u y0=%u x1=%u y1=%u drag=%u", unsigned(b.x0), unsigned(b.y0), unsigned(b.x1),
                unsigned(b.y1), unsigned(b.drag));
    uint8_t x = uint8_t(tile % 128), y = uint8_t(tile / 128);
    BlueprintSpot at{
        x, y, x == b.x0, y == b.y0, x == b.x1, y == b.y1, x >= b.x0 && x <= b.x1 && y >= b.y0 && y <= b.y1};
    if (!dragging) {
        modify_blueprint_pointer(w, at);
        return;
    }
    switch (b.drag) {
    case kDragNone:
        modify_blueprint_start_drag(b, at);
        break;
    case kDragCorner:
        b.x0 = b.anchor_x < x ? b.anchor_x : x;
        b.y0 = b.anchor_y < y ? b.anchor_y : y;
        b.x1 = b.anchor_x > x ? b.anchor_x : x;
        b.y1 = b.anchor_y > y ? b.anchor_y : y;
        break;
    case kDragEdge:
        modify_blueprint_drag_edge(b, x, y);
        break;
    case kDragMove: {
        // Byte arithmetic, as the game's: moving off the map's edge wraps.
        uint8_t dx = uint8_t(x - b.anchor_x), dy = uint8_t(y - b.anchor_y);
        b.x0 = uint8_t(b.x0 + dx);
        b.y0 = uint8_t(b.y0 + dy);
        b.x1 = uint8_t(b.x1 + dx);
        b.y1 = uint8_t(b.y1 + dy);
        b.anchor_x = x;
        b.anchor_y = y;
        break;
    }
    default:
        break;
    }
}

// can_place_door (src/objects.cpp): nothing stands on the cells of a door
// of `kind` facing `dir` at `tile`.
bool can_place_door(const World &w, int tile, int kind, int dir)
{
    const ShapeInfo &shape = shape_of(kind, dir);
    for (int i = 0; i < shape.count; i++) {
        int c = tile + shape.points[size_t(i)];
        if (in_map(c) && (w.map[size_t(c)].flags & 7))
            return false;
    }
    return true;
}

// temp_place_door's margins for a door of `door_kind`: the wall cells it
// may take, from `start` to the wall's length less `end`, and whether a
// piece is checked with is_valid_door.
struct DoorMargins {
    int start, end;
    bool check_valid;
};

DoorMargins temp_place_door_margins(int door_kind)
{
    switch (door_kind) {
    case 0:
        return {1, 3, true};
    case 4:
        return {2, 2, false};
    case 3:
        return {1, 1, false};
    default:
        return {1, 2, true};
    }
}

// The wall cell temp_place_door has found so far nearest the pointer, with
// its door type's direction (0-3).
struct DoorSpot {
    int best = 9999, tile = 0, type = 0;
};

// temp_place_door on the west and east walls.
void temp_place_door_scan_columns(const World &w, uint16_t pos, const DoorMargins &m, DoorSpot &spot)
{
    const BuildState &b = w.building.build;
    const RoomInfo &room = w.rooms[b.room % kRoomCount];
    for (int i = m.start; i <= room.height - m.end; i++) {
        for (int j = 0; j <= room.width; j += room.width) {
            int tile = room.pos + j + (i << 7);
            // Double doors need the cell below to be a west edge piece
            // too; the game reads its first thing as the marker.
            if (b.door_kind || (in_map(tile + 128) && (-w.map[size_t(tile + 128)].first & 3) == 2)) {
                int d = std::abs(int16_t(pos) % 128 - tile % 128) + std::abs(int16_t(pos) / 128 - tile / 128);
                if (d < spot.best)
                    spot = {d, tile, j == 0 ? 2 : 3};
            }
            if (room.width == 0)
                break;
        }
    }
}

// temp_place_door on the north and south walls.
void temp_place_door_scan_rows(const World &w, uint16_t pos, const DoorMargins &m, DoorSpot &spot)
{
    const BuildState &b = w.building.build;
    const RoomInfo &room = w.rooms[b.room % kRoomCount];
    for (int i = m.start; i <= room.width - m.end; i++) {
        for (int j = 0; j <= room.height; j += room.height) {
            int tile = room.pos + i + (j << 7);
            if (b.door_kind || (in_map(tile - 1) && (-w.map[size_t(tile - 1)].first & 3) == 1)) {
                int d = std::abs(int16_t(pos) % 128 - tile % 128) + std::abs(int16_t(pos) / 128 - tile / 128);
                if (d < spot.best)
                    spot = {d, tile, j == 0 ? 0 : 1};
            }
            if (room.height == 0)
                break;
        }
    }
}

// temp_place_door's check of the door of `type` at `best_tile`: marks the
// pieces that cannot go.
void temp_place_door_check_pieces(World &w, int best_tile, int type, bool check_valid)
{
    Building &bd = w.building;
    const BuildState &b = bd.build;
    const DoorType &door = kDoorTypes[size_t(type) % kDoorTypes.size()];
    bd.door_piece_invalid = {};
    for (int j = 0; j < door.count; j++) {
        int tile = best_tile + door.tiles[size_t(j)];
        bool blocked;
        if (b.door_kind != 2) {
            const DoorType &single = kDoorTypes[size_t(type % 4 + 4)];
            blocked = !can_place_door(w, tile, kObjDoor, kBlueprints[single.pieces[0]].build_type - 1);
        } else {
            int kind = in_map(tile) ? kBlueprints[find_small_thing(w, tile) % kBlueprints.size()].object_kind : 0;
            blocked = kind == kObjDoor || kind == kObjSwingDoor || kind == kObjSwingDoor2;
        }
        if ((check_valid || blocked) && (blocked || !is_valid_door(w, tile, type))) {
            bd.door_piece_invalid[size_t(j)] = true;
            trace_event("door_invalid");
            bd.door_invalid = true;
        }
    }
}

// temp_place_door (src/build.cpp): the wall cell of the blueprint nearest
// `pos` where the door being placed (a window once the door is in) can go,
// whether it may (Building::door_invalid) and the pieces to draw red.
void temp_place_door(World &w, uint16_t pos)
{
    Building &bd = w.building;
    BuildState &b = bd.build;
    const RoomInfo &room = built_room(w);
    bd.door_invalid = false;
    const int min_w = 3, min_h = 3;
    DoorMargins m = temp_place_door_margins(b.door_kind);
    // The game leaves `best` unset unless the room is taller than three;
    // every room type is at least four high.
    DoorSpot spot;
    if (room.height > min_h)
        temp_place_door_scan_columns(w, pos, m, spot);
    if (room.width > min_w)
        temp_place_door_scan_rows(w, pos, m, spot);
    int best_tile = spot.tile, type = spot.type;
    if (best_tile == room.pos && room.pos % 128 == pos % 128)
        type = 2;
    if (spot.best == 9999) {
        bd.door_shown = false;
        return;
    }
    type += b.door_kind << 2;
    temp_place_door_check_pieces(w, best_tile, type, m.check_valid);
    bd.door_shown = true;
    b.door_tile = uint16_t(best_tile);
    b.door_type = uint8_t(type);
}

// place_bp_door's markers of `door` at the build state's door tile, and the
// floor under them.
void place_bp_door_pieces(World &w, const DoorType &door)
{
    const BuildState &b = w.building.build;
    for (int i = 0; i < door.count; i++) {
        int tile = b.door_tile + door.tiles[size_t(i)];
        if (!in_map(tile))
            continue;
        int piece = door.pieces[size_t(i)];
        if ((kBlueprints[find_small_thing(w, tile) % kBlueprints.size()].flags & 3) == 3)
            piece += 6;
        place_small_thing(w, tile, piece, false);
        if (in_map(tile - door.offset))
            w.map[size_t(tile - door.offset)].floor_block = door.floor_blocks[size_t(i)];
    }
}

// place_bp_door (src/build.cpp): puts the door or window temp_place_door
// found on the blueprint, or with `swap` takes the window there out. Then
// windows are placed.
void place_bp_door(Game &game, bool swap)
{
    World &w = game.world;
    BuildState &b = w.building.build;
    RoomInfo &room = built_room(w);
    int delta = 1;
    if (w.building.door_invalid) {
        play(w, kSfxWrong2);
        return;
    }
    const DoorType *door = &kDoorTypes[b.door_type % kDoorTypes.size()];
    if (door->is_entrance) {
        room.door_tile = uint16_t(b.door_tile + door->door_offset);
        room.door_inside_tile = uint16_t(b.door_tile + door->door_inside_offset);
    }
    if (!check_room_access_with_blueprint(w)) {
        trace_event("door_blocked");
        game.advisor.forced_advice_id = kAdviceDoorBlocks;
        play(w, kSfxWrong2);
        return;
    }
    int type = b.door_type;
    bool on_window = kBlueprints[find_small_thing(w, b.door_tile + door->tiles[0]) % kBlueprints.size()].kind == 0x7e;
    if (swap) {
        if (!(on_window && b.door_kind == 2))
            return;
        type = type % 4 + 0x14;
        delta = -1;
        play(w, kSfxDeBuild);
    } else {
        play(w, b.door_kind == 2 && on_window ? kSfxWrong2 : kSfxBuildClick);
    }
    place_bp_door_pieces(w, kDoorTypes[size_t(type)]);
    if (b.door_kind == 2)
        room.windows = uint8_t(room.windows + delta);
    if (b.door_kind == 1 || b.door_kind == 0) {
        set_room_requester_mode(w, 4);
        b.door_kind = 2;
    }
}

// is_room_clear_yet (src/blueprnt.cpp): nothing stands on the room's floor.
bool is_room_clear_yet(const World &w, const RoomInfo &room)
{
    int x0 = room.pos % 128, y0 = room.pos / 128;
    for (int x = x0; x < x0 + room.width; x++)
        for (int y = y0; y < y0 + room.height; y++)
            if (in_map(cell_index(x, y)) && w.map[size_t(cell_index(x, y))].first > 0)
                return false;
    return true;
}

// place_door (src/objects.cpp) facing `dir` (gamevar.place_dir): a door
// thing of `kind`, owned by the room it opens into. Returns its id, 0 if
// something stands in its way or the things table is full.
uint16_t place_door(World &w, const AnimData &anims, int tile, int kind, int dir)
{
    if (!can_place_door(w, tile, kind, dir))
        return 0;
    uint16_t id = 0;
    for (size_t i = 1; i < w.things.size(); i++)
        if (w.things[i].type == kThingFree) {
            id = uint16_t(i);
            break;
        }
    if (!id)
        return 0;
    const ShapeInfo &shape = shape_of(kind, dir);
    for (int i = 0; i < shape.count; i++)
        set_flags(w, tile + shape.points[size_t(i)], 2);
    Thing &door = w.things[id];
    door.type = kThingDoor;
    door.facing = uint8_t(dir);
    door.subtype = uint8_t(kind);
    size_t start = size_t(kObjects[size_t(kind)].anim_start) + door.facing;
    if (start < anims.starts.size()) {
        door.sprite = uint16_t(anims.starts[start].frame);
        door.flip = uint8_t(anims.starts[start].flip);
    }
    door.anim_speed = 2;
    door.use_pos[0] = uint16_t(tile + shape.use_offsets[0]);
    door.use_pos[3] = uint16_t(tile + shape.use_offsets[3]);
    door.x.tile = uint8_t(tile % kMapSize);
    door.y.tile = uint8_t(tile / kMapSize);
    door.x.frac = door.y.frac = 0x80;
    door.depth = shape.depth;
    door.state = kObjectStatic;
    insert_node(w, id, door.x, door.y);
    if (uint16_t room = room_at(w, uint16_t(tile + kDoorToRoom[door.facing]))) {
        door.room_id = room;
        door.owner = w.rooms[room].owner;
    }
    return id;
}

// Window pieces build_all_rooms keeps when it puts a wall up.
bool is_window_n(uint8_t wall)
{
    return wall >= 0x7e && wall <= 0x8a && wall % 2 == 0;
}

bool is_window_w(uint8_t wall)
{
    return wall >= 0x7f && wall <= 0x8b && wall % 2 == 1;
}

// The game's test for a "type 3" thing first on a cell, which keeps walls
// and recolouring off it; no thing has that type.
bool first_is_type3(const World &w, int index)
{
    if (!in_map(index))
        return false;
    int id = w.map[size_t(index)].first;
    return id > 0 && w.things[size_t(id)].type == 3;
}

// A blueprint marker build_all_rooms is building on cell `i`, whose north
// and west walls were `fixed_n`/`fixed_w` (block flag 0x10) beforehand.
struct BuildCell {
    int i;
    bool fixed_n, fixed_w;
};

// build_all_rooms on a wall marker: the wall in colour 7, kept off windows
// and type 3 things.
void build_all_rooms_wall(World &w, const BuildCell &at, const BlueprintPiece &piece)
{
    // The game looks for the colour of a wall next to the blueprint and then
    // sets corner_wall_colour to 7 whatever it found, so new walls are always
    // of colour 7.
    const int col = 7 * 2;
    int i = at.i;
    MapCell &c = w.map[size_t(i)];
    if (piece.flags < 3)
        set_flags(w, i - 0x81, 0x10);
    if ((piece.flags & 1) && !at.fixed_n) {
        bool across = first_is_type3(w, i) && w.things[size_t(c.first)].facing % 2 == 0;
        if (!across && !is_window_n(c.wall_n))
            c.wall_n = uint8_t(piece.kind + col);
    }
    if ((piece.flags & 2) && !at.fixed_w) {
        bool across = first_is_type3(w, i) && w.things[size_t(c.first)].facing % 2 == 1;
        if (!across && !is_window_w(c.wall_w))
            c.wall_w = uint8_t(piece.kind + col + 1);
    }
    if (c.wall_n && wall_at(w, i - 0x7f, 1))
        set_flags(w, i - 0x80, 0x10);
}

// build_all_rooms on a door marker facing build_type - 1. Returns false when
// the door cannot go: the marker is then the plain wall it stands in, to be
// built again.
bool build_all_rooms_door(Game &game, int i, const BlueprintPiece &piece)
{
    World &w = game.world;
    int dir = piece.build_type - 1;
    if (!place_door(w, game.anims(), i, piece.kind, dir)) {
        place_small_thing(w, i, dir % 2 + 1, false);
        return false;
    }
    insert_blank_wall(w, i, dir % 2);
    set_flags(w, i - 0x81, 0x10);
    return true;
}

// build_all_rooms on build type 10 markers.
void build_all_rooms_type10(MapCell &c, const BlueprintPiece &piece)
{
    if (piece.flags >= 9)
        return;
    c.flags |= 1;
    if (piece.flags == 7) {
        c.wall_n = 0x97;
        c.wall_w = 0x98;
    } else {
        (piece.wall_layer ? c.wall_w : c.wall_n) = uint8_t(piece.kind);
    }
}

// build_all_rooms on level editor markers; no blueprint of a room has them.
// The game's place_object would also charge for the object.
void build_all_rooms_object(Game &game, int i, int bp, const BlueprintPiece &piece)
{
    World &w = game.world;
    uint8_t dir = uint8_t(bp - 0x20);
    if (dir >= 4)
        return;
    place_small_thing(w, i, 0, false);
    place_small_thing(w, i + (dir % 2 ? 0x80 : 1), 0, false);
    place_object(w, game.anims(), {i, piece.kind, dir}, kPlayer, false);
}

// build_all_rooms on marker `bp` at `at.i`. Returns true when `bp` is set to
// a marker to build next on the same cell; `at.i` goes back one cell when
// the cell is to be built again.
bool build_all_rooms_piece(Game &game, BuildCell &at, int &bp)
{
    World &w = game.world;
    int i = at.i;
    MapCell &c = w.map[size_t(i)];
    const BlueprintPiece &piece = kBlueprints[size_t(bp) % kBlueprints.size()];
    switch (piece.build_type) {
    case 0:
        build_all_rooms_wall(w, at, piece);
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        if (!build_all_rooms_door(game, i, piece))
            at.i--;
        break;
    case 5:
        if ((piece.flags & 1) && !at.fixed_n)
            c.wall_n = 0x7e;
        if ((piece.flags & 2) && !at.fixed_w)
            c.wall_w = 0x7f;
        set_flags(w, i - 0x81, 0x10);
        break;
    case 10:
        build_all_rooms_type10(c, piece);
        break;
    case 11:
    case 13:
        place_door(w, game.anims(), i, piece.kind, 0);
        break;
    case 12:
    case 14:
        place_door(w, game.anims(), i, piece.kind, 1);
        break;
    case 15:
    case 16:
    case 17:
    case 18:
        build_all_rooms_object(game, i, bp, piece);
        break;
    case 21:
        c.wall_n = uint8_t(piece.kind);
        break;
    case 22:
        c.wall_w = uint8_t(piece.kind);
        break;
    case 23:
    case 25:
        c.wall_n = 0x60;
        bp = piece.kind;
        return true;
    case 24:
    case 26:
        c.wall_w = 0xc4;
        bp = piece.kind;
        return true;
    default:
        break;
    }
    return false;
}

// build_all_rooms (src/build.cpp) for the local player: the blueprint
// markers on its land become walls, doors, windows and objects.
void build_all_rooms(Game &game)
{
    World &w = game.world;
    const uint8_t mask = 1 << kPlayer;
    for (int i = 0; i < kMapCells; i++) {
        MapCell &c = w.map[size_t(i)];
        if (!(c.owner_flags & mask)) {
            if (c.owner_flags & (mask | kCellOutdoor))
                place_small_thing(w, i, 0, false);
            continue;
        }
        int id = find_bluep_who(w, i);
        if (id >= 0)
            continue;
        place_small_thing(w, i, 0, false);
        BuildCell at{i, (block_flags(c.wall_n) & 0x10) != 0, (block_flags(c.wall_w) & 0x10) != 0};
        if (uint16_t r = room_at(w, uint16_t(i)))
            w.rooms[r].owner = kPlayer;
        int bp = -id;
        while (build_all_rooms_piece(game, at, bp)) {
        }
        i = at.i;
    }
    for (MapCell &c : w.map)
        if ((c.owner_flags & mask) && block_flags(c.floor_block) == 0x20)
            c.floor_block = 0x13;
    for (size_t r = 1; r < w.rooms.size(); r++)
        if (w.rooms[r].state < 0)
            w.rooms[r].state = 0;
}

// The recolour_inside_room and recolour_outside_room switches: a wall block
// of any colour in the colour of `base` (window pieces in that of `b`).
uint8_t recoloured_n(uint8_t wall, int base, int b)
{
    if (wall >= 0x7e && wall <= 0x8a && wall % 2 == 0)
        return uint8_t(b);
    if (wall >= 0x50 && wall <= 0x5e && wall % 2 == 0)
        return uint8_t(base);
    if (wall >= 0xc3 && wall <= 0xd1 && wall % 2 == 1)
        return uint8_t(base + 0x73);
    if (wall >= 0x60 && wall <= 0x6e && wall % 2 == 0)
        return uint8_t(base + 0x10);
    return wall;
}

uint8_t recoloured_w(uint8_t wall, int base, int b)
{
    if (wall >= 0x7f && wall <= 0x8b && wall % 2 == 1)
        return uint8_t(b + 1);
    if (wall >= 0x51 && wall <= 0x5f && wall % 2 == 1)
        return uint8_t(base + 1);
    if (wall >= 0xc4 && wall <= 0xd2 && wall % 2 == 0)
        return uint8_t(base + 0x74);
    if (wall >= 0x61 && wall <= 0x6f && wall % 2 == 1)
        return uint8_t(base + 0x11);
    return wall;
}

// recolour_inside_room (src/build.cpp): the room's north and west walls.
void recolour_inside_room(World &w, const RoomInfo &room, int base, int b)
{
    int x0 = room.pos % 128, y0 = room.pos / 128;
    for (int x = x0; x < x0 + room.width; x++) {
        int i = cell_index(x, y0);
        if (in_map(i) && !first_is_type3(w, i))
            w.map[size_t(i)].wall_n = recoloured_n(w.map[size_t(i)].wall_n, base, b);
    }
    for (int y = y0; y < y0 + room.height; y++) {
        int i = cell_index(x0, y);
        if (in_map(i) && !first_is_type3(w, i))
            w.map[size_t(i)].wall_w = recoloured_w(w.map[size_t(i)].wall_w, base, b);
    }
}

// recolour_outside_room (src/build.cpp): the room's south and east walls.
// Its skip test reads the room's top row, and for the east wall the column
// the first loop ended on, as the game does.
void recolour_outside_room(World &w, const RoomInfo &room, int base, int b)
{
    int x0 = room.pos % 128, y0 = room.pos / 128;
    int x = x0;
    for (; x < x0 + room.width; x++) {
        int i = cell_index(x, y0 + room.height);
        if (!in_map(i))
            continue;
        if (!(w.map[size_t(i)].first && first_is_type3(w, cell_index(x, y0))))
            w.map[size_t(i)].wall_n = recoloured_n(w.map[size_t(i)].wall_n, base, b);
    }
    for (int y = y0; y < y0 + room.height; y++) {
        int i = cell_index(x0 + room.width, y);
        if (!in_map(i))
            continue;
        if (!(w.map[size_t(i)].first && first_is_type3(w, cell_index(x, y0))))
            w.map[size_t(i)].wall_w = recoloured_w(w.map[size_t(i)].wall_w, base, b);
    }
}

// search_fill_in_window (src/build.cpp).
uint8_t fill_block_for(uint8_t wall)
{
    for (const FillInWindow &f : kFillInWindows)
        if (f.wall == wall)
            return f.window;
    return 0;
}

uint8_t &wall_ref(World &w, int x, int y, int side, uint8_t &none)
{
    int i = cell_index(x, y);
    if (!in_map(i)) {
        none = 0;
        return none;
    }
    return side == 0 ? w.map[size_t(i)].wall_n : w.map[size_t(i)].wall_w;
}

// check_inside_w_wall, check_inside_n_wall, check_outside_s_wall and
// check_outside_e_wall (src/build.cpp): window pieces meeting at a room's
// corner filled in.
void check_inside_w_wall(World &w, int x, int y)
{
    uint8_t none;
    uint8_t &here = wall_ref(w, x, y, 1, none);
    uint8_t fill = fill_block_for(here);
    if (fill && here == 0x7d) {
        here = fill;
        uint8_t &above = wall_ref(w, x, y - 1, 1, none);
        if (above == 0x7b)
            above = fill;
    }
}

void check_inside_n_wall(World &w, int x, int y)
{
    uint8_t none;
    uint8_t &here = wall_ref(w, x, y, 0, none);
    uint8_t fill = fill_block_for(here);
    if (fill && here == 0x7c) {
        here = fill;
        uint8_t &left = wall_ref(w, x - 1, y, 0, none);
        if (left == 0x7a)
            left = fill;
    }
}

void check_outside_s_wall(World &w, int x, int y)
{
    uint8_t none;
    uint8_t &left = wall_ref(w, x - 1, y, 0, none);
    uint8_t fill = fill_block_for(left);
    if (!fill)
        return;
    uint8_t &here = wall_ref(w, x, y, 0, none);
    if (left == 0x72) {
        left = fill;
        if (here == 0x74) {
            here = fill;
            uint8_t &right = wall_ref(w, x + 1, y, 0, none);
            if (right == 0x76)
                right = fill;
        } else if (here == 0x76) {
            here = fill;
        }
    } else if (left == 0x74) {
        left = fill;
        uint8_t &left2 = wall_ref(w, x - 2, y, 0, none);
        if (left2 == 0x72)
            left2 = fill;
        uint8_t &here2 = wall_ref(w, x, y, 0, none);
        if (here2 == 0x76)
            here2 = fill;
    }
}

// The game tests the north walls of the column for its second case.
void check_outside_e_wall(World &w, int x, int y)
{
    uint8_t none;
    uint8_t &above = wall_ref(w, x, y - 1, 1, none);
    uint8_t fill = fill_block_for(above);
    if (!fill)
        return;
    if (above == 0x73) {
        above = fill;
        uint8_t &here = wall_ref(w, x, y, 1, none);
        if (here == 0x75) {
            here = fill;
            uint8_t &below = wall_ref(w, x, y + 1, 1, none);
            if (below == 0x77)
                below = fill;
        } else if (here == 0x77) {
            here = fill;
        }
        return;
    }
    uint8_t &above_n = wall_ref(w, x, y - 1, 0, none);
    if (above_n == 0x75) {
        above_n = fill;
        uint8_t &above2 = wall_ref(w, x, y - 2, 0, none);
        if (above2 == 0x73)
            above2 = fill;
        uint8_t &here = wall_ref(w, x, y, 0, none);
        if (here == 0x77)
            here = fill;
    }
}

// fill_in_window (src/build.cpp): side 1 top right, 2 bottom left, 3
// bottom right, 4 top left.
void fill_in_window(World &w, int x, int y, int side)
{
    switch (side) {
    case 1:
        check_inside_n_wall(w, x, y);
        check_outside_e_wall(w, x, y);
        break;
    case 2:
        check_inside_w_wall(w, x, y);
        check_outside_s_wall(w, x, y);
        break;
    case 3:
        check_outside_s_wall(w, x, y);
        check_outside_e_wall(w, x, y);
        break;
    case 4:
        check_inside_w_wall(w, x, y);
        check_inside_n_wall(w, x, y);
        break;
    default:
        break;
    }
}

// pixel_buffer_map (src/build.cpp): map flag 0x10 on the cells behind
// visible walls, for cells 1 to 126 of rows 1 to 126. A cell's flag is
// cleared when it is reached, after the cells before it may have set it.
void pixel_buffer_map(World &w)
{
    int m = 129;
    for (int y = 125; y >= 0; y--) {
        for (int x = 125; x >= 0; x--, m++) {
            MapCell &c = w.map[size_t(m)];
            c.flags &= ~0x10;
            if ((c.wall_n && c.wall_n < 0x8e) || c.wall_n > 0x91) {
                set_flags(w, m - 129, 0x10);
                set_flags(w, m - 128, 0x10);
                set_flags(w, m - 127, 0x10);
            }
            if ((c.wall_w && c.wall_w < 0x8e) || c.wall_w > 0x91) {
                set_flags(w, m - 129, 0x10);
                set_flags(w, m - 1, 0x10);
                set_flags(w, m + 127, 0x10);
            }
        }
        m += 2;
    }
}

// recolour_all_rooms (src/build.cpp): every room's walls in its type's
// colours, the window corners filled in, then the wall flags.
void recolour_all_rooms(World &w)
{
    static constexpr std::array<int, 7> kOuter = {0, 0, 0, 0x58, 0x5a, 0x5c, 0x5e};
    static constexpr std::array<int, 7> kInner = {0, 0, 0, 0x50, 0x52, 0x54, 0x56};
    static constexpr std::array<int, 7> kWindow = {0, 0, 0, 0x7e, 0x80, 0x82, 0x84};
    for (size_t r = 1; r < w.rooms.size(); r++) {
        const RoomInfo &room = w.rooms[r];
        if (room.type == kRoomFree)
            continue;
        int style = kRoomTypeInfo[room.type % kRoomTypes].wall_style;
        if (style < 7 && kOuter[size_t(style)] > 0 && kWindow[size_t(style)] > 0)
            recolour_outside_room(w, room, kOuter[size_t(style)], kWindow[size_t(style)]);
    }
    for (size_t r = 1; r < w.rooms.size(); r++) {
        const RoomInfo &room = w.rooms[r];
        if (!room.pos || room.type == kRoomFree)
            continue;
        int style = kRoomTypeInfo[room.type % kRoomTypes].wall_style;
        if (style < 7 && kInner[size_t(style)] > 0 && kWindow[size_t(style)] > 0)
            recolour_inside_room(w, room, kInner[size_t(style)], kWindow[size_t(style)]);
        int x = room.pos % 128, y = room.pos / 128;
        fill_in_window(w, x, y, 4);
        fill_in_window(w, room.width + x, y, 1);
        fill_in_window(w, x, y + room.height, 2);
        fill_in_window(w, room.width + x, y + room.height, 3);
    }
    pixel_buffer_map(w);
}

// The object kinds a room type's furnishing offers, as set_buyable_items
// (src/req1.cpp) lists them: available to the player (equipment_available:
// from the start, by their month or researched) and in the level.
void set_buyable_items(World &w, int room_type)
{
    ObjectPlacing &p = w.placing;
    const RoomTypeInfo &type = kRoomTypeInfo[size_t(room_type) % kRoomTypes];
    p.requester_mode = 3;
    p.item = 0;
    size_t count = 0;
    for (int i = 0; i < type.buyable_count && count < p.offered.size(); i++) {
        uint8_t obj = type.buyable[size_t(i)];
        if (equipment_available(w, obj))
            p.offered[count++] = obj;
    }
    for (; count < p.offered.size(); count++)
        p.offered[count] = 0;
    // reset_dept_object.
    p.item = kRoomTypeInfo[p.room_type % kRoomTypes].buyable[0];
}

} // namespace

bool room_available(const World &world, int room_type)
{
    const RoomTypeInfo &type = kRoomTypeInfo[size_t(room_type) % kRoomTypes];
    for (int i = 0; i < type.compulsory_count; i++)
        if (!equipment_available(world, type.objects[size_t(i)]))
            return false;
    return true;
}

namespace
{

// delete_object (src/objects.cpp), as remove_object_from_map calls it: `obj`
// out of the object list of `room` (if any) and of its kind; the autopsy
// unavailable once the player has no autopsy machine left.
void remove_object_from_map_delete_object(World &w, Thing &obj, uint16_t room)
{
    if (room) {
        RoomInfo &r = w.rooms[room];
        if (r.first_object) {
            if (obj.room_obj_prev)
                w.things[obj.room_obj_prev].room_obj_next = obj.room_obj_next;
            else
                r.first_object = obj.room_obj_next;
            if (obj.room_obj_next)
                w.things[obj.room_obj_next].room_obj_prev = obj.room_obj_prev;
        }
        obj.room_obj_prev = obj.room_obj_next = 0;
    }
    uint16_t &first = w.first_of_kind[obj.subtype];
    if (first) {
        if (obj.type_obj_prev)
            w.things[obj.type_obj_prev].type_obj_next = obj.type_obj_next;
        else
            first = obj.type_obj_next;
        if (obj.type_obj_next)
            w.things[obj.type_obj_next].type_obj_prev = obj.type_obj_prev;
        obj.type_obj_prev = obj.type_obj_next = 0;
    }
    if (obj.owner == kPlayer && w.first_of_kind[kObjAutoAutopsy] == 0)
        w.cure_available[kIllAutopsy] = 0;
}

// adjust_big_cell_flag's test: thing `t` is a person using an object or an
// object of more than one cell.
bool remove_object_from_map_is_big(const World &w, int t)
{
    const Thing &o = w.things[size_t(t)];
    if (o.type == kThingPatient || o.type == kThingStaff)
        return o.object && o.object < w.things.size() && w.things[o.object].user == t;
    if (o.type == kThingObject)
        return shape_of(o.subtype, o.facing).count > 1;
    return false;
}

// remove_object_from_map (src/objects.cpp): its plant and repair pages cancelled, its plant figures kept
// (World::plants, plant_health_sum), its user interrupted, the object off the map and its lists, kept in the things
// table (state OS_REMOVED).
void remove_object_from_map(World &w, uint16_t id)
{
    Thing &obj = w.things[id];
    if (obj.subtype == kObjPlant) {
        thingpage_cancel(w.plant_pager, id);
        w.plant_health_sum -= (obj.plant_health * 2 + 0x50) / 16;
        if (obj.owner == kPlayer) {
            w.plants--;
            if (obj.plant_health < 0)
                w.thirsty_plants--;
        }
    }
    if (obj.owner == kPlayer)
        thingpage_cancel(w.repair_pager, id);
    int tile = obj.x.tile + (obj.y.tile << 7);
    remove_object_shape(w, tile, obj.subtype, obj.facing);
    add_object_value(w, obj.subtype, -1);
    uint16_t room = room_at(w, uint16_t(tile));
    if (kObjects[obj.subtype].category == 6 && room)
        w.rooms[room].radiators--;
    // remove_object_from_map interrupts the person using the object before it
    // goes (interrupt_peep).
    if (obj.state == kObjectInUse && obj.user)
        interrupt_peep(w, obj.user);
    obj.state = kObjectRemoved;
    remove_object_from_map_delete_object(w, obj, room);
    // adjust_big_cell_flag (src/engine.cpp), which still finds the object on
    // the cell.
    MapCell &c = w.map[size_t(tile)];
    c.flags &= ~0x20;
    for (int t = c.first; t > 0; t = w.things[size_t(t)].cell_next) {
        if (remove_object_from_map_is_big(w, t)) {
            c.flags |= 0x20;
            break;
        }
    }
    remove_node(w, id);
}

// The door facing the shadows see on a cell (search_mapwho for a door).
void add_shadows(World &w)
{
    add_static_shadows(w.map, [&w](int cell) {
        for (int id = w.map[size_t(cell)].first; id > 0; id = w.things[size_t(id)].cell_next)
            if (w.things[size_t(id)].type == kThingDoor)
                return int(w.things[size_t(id)].facing);
        return -1;
    });
}

// delete_wall (src/build.cpp): takes the north (side 0) or west wall of
// `tile` down unless it bounds another room than `room` or its block has
// flag 0x10.
void delete_wall(World &w, int tile, uint16_t room, int side)
{
    int x = tile % 128, y = tile / 128;
    uint16_t before = side == 0 ? room_at(w, uint16_t((y - 1) * 128 + x)) : room_at(w, uint16_t(x - 1 + y * 128));
    uint16_t after = side == 0 ? room_at(w, uint16_t((y + 1) * 128 + x)) : room_at(w, uint16_t(x + 1 + y * 128));
    bool clear = true;
    if (before > 0 && before != room)
        clear = false;
    if (after > 0 && after != room && room_at(w, uint16_t(y * 128 + x)) != 0)
        clear = false;
    MapCell &c = w.map[size_t(cell_index(x, y))];
    uint8_t &wall = side == 0 ? c.wall_n : c.wall_w;
    if (block_flags(wall) & 0x10)
        clear = false;
    if (clear)
        wall = 0;
}

// delete_room on cell (x, y) of the room's outline: the walls of room `id`
// and the doors in them.
void delete_room_walls_and_doors(World &w, const RoomInfo &room, uint16_t id, int x, int y)
{
    int x0 = room.pos % 128, y0 = room.pos / 128;
    int i = cell_index(x, y);
    if (!in_map(i))
        return;
    if (x < x0 + room.width && w.map[size_t(i)].wall_n)
        delete_wall(w, i, id, 0);
    if (y < y0 + room.height && w.map[size_t(i)].wall_w)
        delete_wall(w, i, id, 1);
    for (int t = w.map[size_t(i)].first; t > 0; t = w.things[size_t(t)].cell_next) {
        const Thing &obj = w.things[size_t(t)];
        if (!(obj.type == kThingDoor ||
              (obj.type == kThingObject && (obj.subtype == kObjSwingDoor || obj.subtype == kObjSwingDoor2))))
            continue;
        if (obj.facing % 2 == 0 ? x < x0 + room.width : y < y0 + room.height)
            remove_object(w, uint16_t(t), kPlayer);
    }
}

// delete_room (src/build.cpp): the room's objects, then cell by cell (the
// room and its walls) the walls it does not share, its doors and the rat
// holes on them, off the map. Only the first thing on each
// cell is looked at, as removing it ends the cell's list walk in the game.
void delete_room(World &w, uint16_t r)
{
    trace_event("delete_room %u", unsigned(r));
    const RoomInfo &room = w.rooms[r];
    int x0 = room.pos % 128, y0 = room.pos / 128;
    uint16_t id = room_at(w, uint16_t(((y0 + room.height / 2) << 7) + x0 + room.width / 2));
    for (int y = y0; y < y0 + room.height; y++) {
        for (int x = x0; x < x0 + room.width; x++) {
            for (int t = w.map[size_t(cell_index(x, y))].first; t > 0; t = w.things[size_t(t)].cell_next) {
                if (w.things[size_t(t)].type == kThingObject)
                    remove_object(w, uint16_t(t), kPlayer);
            }
        }
    }
    for (int y = y0; y < y0 + room.height + 1; y++)
        for (int x = x0; x < x0 + room.width + 1; x++) {
            delete_room_walls_and_doors(w, room, id, x, y);
            clear_cell_of_ratholes(w, room.owner, uint16_t((y << 7) + x));
        }
    add_shadows(w);
    w.counts.open_rooms--;
}

// check_invis_walls_around_object (src/build.cpp): the invisible walls of
// object `obj` put back where missing; other things are left alone.
void check_invis_walls_around_object(World &w, const Thing &obj)
{
    int8_t group = kObjects[obj.subtype].walls;
    if (obj.type != kThingObject || group == -1)
        return;
    const ObjectWallsInfo &walls = kObjectWalls[size_t(group * 4 + obj.facing)];
    for (int k = 0; k < walls.count; k++) {
        int c = obj.x.tile + (obj.y.tile << 7) + walls.pos[size_t(k)];
        if (wall_at(w, c, walls.side[size_t(k)]) == 0)
            insert_blank_wall(w, c, walls.side[size_t(k)]);
    }
}

// repair_invis_walls_around_room (src/build.cpp): the invisible walls of
// the objects on the ring of cells round the room put back where missing
// (check_invis_walls_around_object).
void repair_invis_walls_around_room(World &w, const RoomInfo &room)
{
    int x0 = room.pos % 128 - 1, y0 = room.pos / 128 - 1;
    int x1 = room.pos % 128 + room.width, y1 = room.pos / 128 + room.height;
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            if (!(x == x0 || x == x1 || y == y0 || y == y1) || !in_map(cell_index(x, y)))
                continue;
            for (int t = w.map[size_t(cell_index(x, y))].first; t > 0; t = w.things[size_t(t)].cell_next)
                check_invis_walls_around_object(w, w.things[size_t(t)]);
        }
    }
}

// clear_build_objects (src/build.cpp) for the player: the lifted objects
// refunded in full (depreciation 100%) and freed, and nothing left to
// place.
void clear_build_objects(Game &game)
{
    World &w = game.world;
    for (uint16_t &id : w.building.build.objects) {
        if (!id)
            continue;
        int cost = game.settings().objects[w.things[id].subtype].start_cost;
        change_balance(game.finance, game.balance, game.date.day, game.date.month, {kTxBuyObject, cost});
        w.things[id] = Thing{};
        id = 0;
    }
    w.placing.buy_count = {};
}

// delete_blueprint_room (src/blueprnt.cpp): the room abandoned, half its
// price back if it was paid for, the patients queueing for it sent on
// (restart_all_queue_peeps) and its treatments gone if it was the last of
// its type.
void delete_blueprint_room(Game &game)
{
    trace_event("delete_blueprint_room");
    World &w = game.world;
    BuildState &b = w.building.build;
    RoomInfo &room = built_room(w);
    if (room.paid_for) {
        int cost = game.settings().rooms[room.type % kRoomTypes].cost;
        change_balance(game.finance, game.balance, game.date.day, game.date.month, {kTxBuildRoom, cost * 50 / 100});
        room.paid_for = 0;
    }
    if (b.draw_mode == 0)
        uncommit_blueprint(w);
    b.draw_mode = 0;
    b.state = 0;
    if (w.person_complexes)
        restart_all_queue_peeps(w, game.anims(), *w.person_complexes, b.room, game.turn_count);
    remove_expertise_room(w, b.room);
    room = RoomInfo{};
    b = BuildState{};
}

// edit_blueprint (src/blueprnt.cpp): the room being built back to a
// blueprint, its objects lifted to be placed again.
void edit_blueprint(World &w)
{
    BuildState &b = w.building.build;
    RoomInfo &room = built_room(w);
    size_t slot = 0;
    while (uint16_t id = room.first_object) {
        Thing &obj = w.things[id];
        if (!obj.pair || kObjects[w.things[obj.pair].subtype].next_part == -1)
            w.placing.buy_count[obj.subtype]++;
        remove_object_from_map(w, id);
        for (; slot < b.objects.size(); slot++) {
            if (b.objects[slot] == 0) {
                b.objects[slot] = id;
                break;
            }
        }
    }
    if (room.door_tile && room.door_tile < 0x4000)
        b.saved_floor_block = w.map[room.door_tile].floor_block;
    room_deleted_update_listhead(w, b.room);
    delete_room(w, b.room);
    uncommit_blueprint(w);
    repair_invis_walls_around_room(w, room);
    room.pos = 0;
    room.door_tile = 0;
    room.door_inside_tile = 0;
    room.width = 0;
    room.height = 0;
    room.state = kRoomStateBlueprint;
    set_room_requester_mode(w, 0);
    w.placing.placing = false;
    w.building.mouse = BuildMouse::Blueprint;
}

// is_room_empty (src/build.cpp): nobody in the room and its door not in
// use.
bool is_room_empty(const World &w, uint16_t r)
{
    bool people = false;
    for_each_person_in(w, r, [&](uint16_t) { people = true; });
    if (people)
        return false;
    uint16_t door_tile = w.rooms[r].door_tile;
    if (!in_map(door_tile))
        return true;
    for (int id = w.map[door_tile].first; id > 0; id = w.things[size_t(id)].cell_next)
        if (w.things[size_t(id)].type == kThingDoor)
            return w.things[size_t(id)].user == 0;
    return true;
}

// setup_place_object_requester (src/build.cpp) for a room.
void setup_place_object_requester(World &w)
{
    ObjectPlacing &p = w.placing;
    p.buy_kind_count = 0;
    for (int i = 0; i < kObjectKinds - 1; i++)
        if (p.buy_count[size_t(i)])
            p.buy_kind_count++;
    set_room_requester_mode(w, 6);
}

// Starts placing the next object in the room being furnished
// (init_place_object with the room's requester mode).
void place_next_in_room(World &w)
{
    init_place_object(w.placing, next_object_to_build(w.placing), 0, w.building.build.room, 0);
    set_room_requester_mode(w, 6);
}

// init_edit_room (src/build.cpp) for a new room: its outline back on the
// map while it is furnished.
void init_edit_room(World &w, uint16_t r)
{
    RoomInfo &room = w.rooms[r];
    BuildState &b = w.building.build;
    int x = room.pos % 128, y = room.pos / 128;
    int x1 = x + room.width - 1, y1 = y + room.height - 1;
    mark_blueprint_on_map(w, x, y, x1, y1, false);
    b.room_type = room.type;
    b.room = r;
    b.x0 = uint8_t(x);
    b.y0 = uint8_t(y);
    b.x1 = uint8_t(x1);
    b.y1 = uint8_t(y1);
    w.placing.room_type = room.type;
    w.placing.mode = 0;
    if (is_room_empty(w, r)) {
        room.state = kRoomStateEdited;
        setup_place_object_requester(w);
    } else {
        // The game waits for the room to empty (MM_EDIT_ROOM_WAIT), which a
        // new room always is.
        room.being_edited = 1;
        w.building.mouse = BuildMouse::EditRoomWait;
    }
}

// price_all_rooms (src/build.cpp): despite its name, prices nothing: the
// player's blueprint room gets its type's floor and wall colours, and its
// objects are placed next.
void price_all_rooms(World &w, int type)
{
    for (size_t r = 1; r < w.rooms.size(); r++) {
        RoomInfo &room = w.rooms[r];
        if (room.state != kRoomStateBlueprint || room.owner != kPlayer)
            continue;
        int x0 = room.pos % 128, y0 = room.pos / 128;
        for (int y = y0; y < y0 + room.height; y++)
            for (int x = x0; x < x0 + room.width; x++)
                w.map[size_t(cell_index(x, y))].floor_block = kRoomTypeInfo[size_t(type) % kRoomTypes].floor_block;
        recolour_all_rooms(w);
        init_place_object(w.placing, next_object_to_build(w.placing), 0, uint16_t(r), 0);
        break;
    }
}

// search_mapwho_for_room_door (src/utils.cpp).
Thing *room_door_on(World &w, int tile, uint16_t room)
{
    if (!in_map(tile))
        return nullptr;
    for (int id = w.map[size_t(tile)].first; id > 0; id = w.things[size_t(id)].cell_next) {
        Thing &t = w.things[size_t(id)];
        if (t.type == kThingDoor && t.room_id == room)
            return &t;
    }
    return nullptr;
}

// init_double_doors (src/build.cpp): the two halves of the room's double
// door linked, the second turned into an object.
void init_double_doors(World &w, uint16_t r)
{
    const RoomInfo &room = w.rooms[r];
    Thing *door = room_door_on(w, room.door_tile, r), *other = nullptr;
    if (door) {
        other = room_door_on(w, room.door_tile - (door->facing == 1 ? 128 : 1), r);
    } else {
        door = room_door_on(w, room.door_inside_tile, r);
        if (door)
            other = room_door_on(w, room.door_inside_tile - (door->facing == 0 ? 1 : 128), r);
    }
    // The game goes on without both halves; a room it builds has them.
    if (!door || !other)
        return;
    other->type = kThingObject;
    door->door_partner = uint16_t(other - w.things.data());
    other->door_main = uint16_t(door - w.things.data());
    if (door->facing == 1 || door->facing == 3) {
        set_flags(w, (door->y.tile << 7) + door->x.tile + 127, 0x20);
        set_flags(w, (other->y.tile << 7) + other->x.tile + 127, 0x20);
    }
}

// commit_doors (src/blueprnt.cpp): once nothing stands on the room's
// floor, the room built and its objects placed next; until then it waits
// (state 2).
void commit_doors(Game &game)
{
    trace_event("commit_doors");
    World &w = game.world;
    BuildState &b = w.building.build;
    b.state = 2;
    if (!is_room_clear_yet(w, built_room(w)))
        return;
    b.state = 0;
    build_all_rooms(game);
    price_all_rooms(w, b.room_type);
    if (kRoomTypeInfo[b.room_type % kRoomTypes].door_kind == 0)
        init_double_doors(w, b.room);
    init_edit_room(w, b.room);
    w.room_of_type[b.room_type % kRoomTypes] = b.room;
    place_next_in_room(w);
    w.building.mouse = BuildMouse::None;
}

// open_room (src/build.cpp): the room finished: its pages unanswered
// again, its outline off the map, lifted objects refunded, the room paid
// for, shadows recomputed. The interface shows its help page.
void open_room(Game &game)
{
    trace_event("open_room");

    World &w = game.world;
    BuildState &b = w.building.build;
    if (!b.room)
        return;
    RoomInfo &room = built_room(w);
    room.state = 0;
    clear_pager_responses(w, b.room);
    for (int x = b.x0; x <= b.x1 + 1; x++)
        for (int y = b.y0; y <= b.y1 + 1; y++)
            place_small_thing(w, cell_index(x, y), 0, false);
    clear_build_objects(game);
    if (room.paid_for == 0) {
        int cost = game.settings().rooms[room.type % kRoomTypes].cost;
        change_balance(game.finance, game.balance, game.date.day, game.date.month, {kTxBuildRoom, -cost});
        room.paid_for = 1;
    }
    if (!w.editing_room)
        w.building.last_built_room = room.type;
    add_shadows(w);
    w.building.mouse = BuildMouse::None;
    w.placing.placing = false;
    if (w.placing.requester == ObjectRequester::EditRoom)
        w.placing.requester = ObjectRequester::None;
    b.room = 0;
    w.editing_room = 0;
    w.counts.open_rooms++;
}

// allocate_roominfo (src/blueprnt.cpp): a free room for the blueprint, whose
// treatments become available if the hospital has the staff
// (set_expertise_room).
void allocate_roominfo(Game &game, int type)
{
    World &w = game.world;
    for (size_t r = 1; r < w.rooms.size(); r++) {
        RoomInfo &room = w.rooms[r];
        if (room.type != kRoomFree)
            continue;
        w.building.build.room = uint16_t(r);
        room = RoomInfo{};
        room.owner = kPlayer;
        room.type = uint8_t(type);
        room.state = kRoomStateBlueprint;
        room.created_turn = game.turn_count;
        room.queue_capacity = 6;
        set_expertise_room(w, uint16_t(r));
        break;
    }
}

// The build part of EV_CANCEL_BUILD (process_turn): lifted objects refunded
// and the mouse back to normal.
void cancel_build(Game &game)
{
    clear_build_objects(game);
    game.world.building.mouse = BuildMouse::None;
}

// try_move_peep_cell (src/blueprnt.cpp).
bool try_move_peep_cell(World &w, Thing &peep, int x, int y)
{
    int pos = w.rooms[w.entrances[peep.owner % 4][0]].door_tile;
    int tile = int16_t(uint8_t(x) + (uint8_t(y) << 7));
    if (!in_map(tile) || (w.map[size_t(tile)].owner_flags & kCellOutdoor) || room_at(w, uint16_t(tile)) ||
        !points_connected(w, tile, pos))
        return false;
    peep.saved_state = peep.state;
    peep.saved_dest_x = uint8_t(x);
    peep.saved_dest_y = uint8_t(y);
    peep.state = peep.type == kThingPatient ? kPsBpGoto : kStaffBpGoto;
    peep.countdown = 0;
    return true;
}

// try_move_peep_to_wall (src/blueprnt.cpp): the cells along side `side`
// (0 north, 1 east, 2 south, 3 west) outside the blueprint, level with the
// person first, then at random.
bool try_move_peep_to_wall(World &w, Thing &peep, int side)
{
    const BuildState &b = w.building.build;
    uint8_t count, base, idx;
    if (side == 0 || side == 2) {
        count = uint8_t(b.x1 - b.x0 + 1);
        base = b.x0;
        idx = uint8_t(peep.x.tile - base);
    } else {
        count = uint8_t(b.y1 - b.y0 + 1);
        base = b.y0;
        idx = uint8_t(peep.y.tile - base);
    }
    std::array<uint8_t, 256> cells{};
    for (int i = 0; i < count; i++)
        cells[size_t(i)] = uint8_t(base + i);
    for (;;) {
        int c = cells[idx];
        bool moved = side == 0   ? try_move_peep_cell(w, peep, c, b.y0 - 1)
                     : side == 1 ? try_move_peep_cell(w, peep, b.x1 + 1, c)
                     : side == 2 ? try_move_peep_cell(w, peep, c, b.y1 + 1)
                                 : try_move_peep_cell(w, peep, b.x0 - 1, c);
        if (moved)
            return true;
        --count;
        cells[idx] = cells[count];
        if (count == 0)
            break;
        idx = uint8_t(old_newrand(w.seed) % count);
    }
    return false;
}

} // namespace

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

namespace
{

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

// apply_build_event on EV_OPEN_BUILD_ROOM:
// cancel_top_level_requesters(REQ_BUILD_ROOM): the buy objects requester
// closes unless placing; the hire staff one is the interface's.
void apply_build_event_open_build_room(ObjectPlacing &p)
{
    cancel_buy_objects(p);
    if (p.requester != ObjectRequester::None)
        return;
    p.requester = ObjectRequester::BuildRoom;
    p.requester_mode = 1;
    p.room_type = 0;
    p.arg3 = 0;
    p.offered = {};
    p.menu_disabled = true;
}

// apply_build_event on EV_DELETE_ROOM: the room being built abandoned; it
// goes on as EV_CANCEL_BUILD with value 0.
void apply_build_event_delete_room(Game &game, const Event &event)
{
    clear_build_objects(game);
    delete_blueprint_room(game);
    Event cancel{EV_CANCEL_BUILD, 0, 0, event.arg};
    apply_staff_event(game, cancel);
    cancel_build(game);
    apply_object_event(game, cancel);
}

// apply_build_event on EV_CANCEL_EDIT_ROOM: cancel_room_edit then
// EV_CANCEL_BUILD value 0 and the mouse mode as arg (src/io.cpp).
void apply_build_event_cancel_edit_room(Game &game, const Event &event)
{
    cancel_room_edit(game.world);
    Event cancel{EV_CANCEL_BUILD, 0, 0, event.arg};
    apply_staff_event(game, cancel);
    cancel_build(game);
    apply_object_event(game, cancel);
}

// apply_build_event on EV_PICK_UP_OBJECT: pick_up_object (src/build.cpp):
// lifts object `value` into the build list, with `arg` also its other half
// (editing a room), takes it off the map and starts placing it again from
// where it was.
// The first free slot of the lifted-objects list, `id` in it; -1 if full.
int lift_object_slot(World &w, uint16_t id)
{
    for (int i = 0; i < int(w.building.build.objects.size()); i++)
        if (w.building.build.objects[size_t(i)] == 0) {
            w.building.build.objects[size_t(i)] = id;
            return i;
        }
    return -1;
}

// pick_up_object's `both`: lift the object's other half too, and go on with
// whichever part has a next part.
void pick_up_object_pair(World &w, uint16_t id, Thing *&obj)
{
    remove_object_from_map(w, id);
    Thing *other = obj->pair < w.things.size() ? &w.things[obj->pair] : nullptr;
    if (!other || !obj->pair)
        return;
    lift_object_slot(w, obj->pair);
    remove_object_from_map(w, obj->pair);
    if (kObjects[other->subtype % kObjectKinds].next_part != -1)
        obj = other;
}

// pick_up_object's else: stop whoever uses `obj` (a member of staff is
// abandoned, a patient taken off its complex).
void pick_up_object_stop_user(Game &game, const Thing &obj)
{
    World &w = game.world;
    if (obj.in_use != 1 || !obj.user || obj.user >= w.things.size())
        return;
    const Thing &user = w.things[obj.user];
    if (user.type == kThingStaff)
        staff_abandon_for_pick_up(w, game.anims(), obj.user, game.turn_count);
    else if (user.type == kThingPatient)
        patient_abandon_complex(w, game.anims(), game.person_complexes(), obj.user, game.turn_count);
}

// apply_build_event on EV_PICK_UP_OBJECT: pick_up_object (src/build.cpp).
void apply_build_event_pick_up_object(Game &game, const Event &event)
{
    World &w = game.world;
    ObjectPlacing &p = w.placing;
    if (event.value >= w.things.size())
        return;
    Thing *obj = &w.things[event.value];
    uint16_t room = 0;
    lift_object_slot(w, event.value);
    if (event.arg) {
        pick_up_object_pair(w, event.value, obj);
        room = w.building.build.room;
    } else {
        pick_up_object_stop_user(game, *obj);
        remove_object_from_map(w, event.value);
        p.mode = 2;
        p.room_type = kRoomCorridor;
    }
    p.buy_count[obj->subtype % kObjectKinds]++;
    p.dir = obj->facing;
    int offset = obj->x.tile + (obj->y.tile << 7) - p.mouse_cell;
    init_place_object(p, obj->subtype, p.mode, room, offset);
}

// apply_build_event on EV_INIT_EDIT_ROOM: init_edit_room (src/build.cpp) for
// an existing room; clicked_for_edit left the mouse mode normal first, and
// an occupied room's init_edit_room sets it back to waiting.
void apply_build_event_init_edit_room(Game &game, uint16_t room)
{
    game.world.editing_room = 1;
    game.world.building.mouse = BuildMouse::None;
    init_edit_room(game.world, room);
}

// apply_build_event on EV_SETUP_PLACE_OBJECT_REQUESTER: the room being edited
// finished emptying, so it goes on to placing its objects (src/io.cpp).
void apply_build_event_setup_place_object(Game &game)
{
    World &w = game.world;
    RoomInfo &room = w.rooms[w.building.build.room % kRoomCount];
    room.state = kRoomStateEdited;
    room.being_edited = 0;
    setup_place_object_requester(w);
    w.building.mouse = BuildMouse::None;
}

// apply_build_event on EV_REQ1_RESET_ROOMS: req1_reset_rooms (src/req1.cpp).
void apply_build_event_reset_rooms(Game &game, uint16_t dept_id)
{
    ObjectPlacing &p = game.world.placing;
    p.arg = dept_id;
    p.requester_mode = 4;
    const DepartmentInfo &dept = kDepartments[dept_id % kDepartments.size()];
    size_t count = 0;
    for (int i = 0; i < dept.count; i++)
        if (room_available(game.world, dept.rooms[size_t(i)]))
            p.offered[count++] = dept.rooms[size_t(i)];
    for (; count < p.offered.size(); count++)
        p.offered[count] = 0;
}

// apply_build_event on EV_INIT_BLUEPRINT: a new blueprint of room `type`.
void apply_build_event_init_blueprint(Game &game, int type)
{
    World &w = game.world;
    ObjectPlacing &p = w.placing;
    BuildState &b = w.building.build;
    // init_blueprint.
    b = BuildState{};
    b.room_type = uint8_t(type);
    p.room_type = uint16_t(type);
    set_buyable_items(w, type);
    // buy_compulsory_items (src/req1.cpp).
    p.buy_count = {};
    const RoomTypeInfo &info = kRoomTypeInfo[size_t(type)];
    for (int i = 0; i < info.compulsory_count; i++)
        p.buy_count[info.objects[size_t(i)]]++;
    p.requester = ObjectRequester::None;
    p.placing = false;
    w.building.mouse = BuildMouse::Blueprint;
    p.requester = ObjectRequester::EditRoom;
    set_room_requester_mode(w, 0);
    p.menu_disabled = false;
    allocate_roominfo(game, type);
}

// apply_build_event on EV_FURNISH_EDITED_ROOM: the buy objects requester
// for the room being edited.
void apply_build_event_furnish_edited_room(Game &game)
{
    World &w = game.world;
    ObjectPlacing &p = w.placing;
    Building &bd = w.building;
    const RoomInfo &room = built_room(w);
    p.requester = ObjectRequester::None;
    bd.saved_buy_count = p.buy_count;
    // adjust_build_palette_for_shop (src/req1.cpp): the compulsory
    // objects the room lacks off the list while buying more.
    std::array<uint8_t, kObjectKinds> counts{};
    for (uint16_t id = room.first_object; id; id = w.things[id].room_obj_next)
        counts[w.things[id].subtype]++;
    bd.shop_adjust = {};
    const RoomTypeInfo &info = kRoomTypeInfo[room.type % kRoomTypes];
    for (int i = info.compulsory_count - 1; i >= 0; i--) {
        uint8_t obj = info.objects[size_t(i)];
        if (counts[obj] == 0) {
            p.buy_count[obj]--;
            bd.shop_adjust[obj]++;
        } else {
            counts[obj]--;
        }
    }
    set_buyable_items(w, room.type);
    p.requester = ObjectRequester::BuyObjects;
    p.room_type = bd.build.room_type;
    p.arg3 = room.pos & 0xff;
    p.requester_mode = 3;
    p.item = p.offered[0];
    p.menu_disabled = false;
}

// apply_build_event on EV_BACK_TO_PLACE_OBJECT:
// go_back_to_place_object_requester (src/req14.cpp); with `restore` the
// counts from before the shop.
void apply_build_event_back_to_place_object(World &w, bool restore)
{
    ObjectPlacing &p = w.placing;
    Building &bd = w.building;
    p.requester = ObjectRequester::EditRoom;
    if (restore) {
        p.buy_count = bd.saved_buy_count;
    } else {
        // post_shop_readjust_build_palette.
        for (int i = kObjectKinds - 2; i >= 0; i--)
            p.buy_count[size_t(i)] = uint8_t(p.buy_count[size_t(i)] + bd.shop_adjust[size_t(i)]);
    }
    if (next_object_to_build(p)) {
        place_next_in_room(w);
    } else {
        p.placing = false;
        setup_place_object_requester(w);
    }
}

// set_room_requester_mode in modes 6 and 8: what the room still needs.
void set_room_requester_mode_furnishing(World &world)
{
    ObjectPlacing &p = world.placing;
    uint16_t room = world.building.build.room;
    if (p.room_type == kRoomCorridor || world.rooms[room % kRoomCount].first_object == 0)
        p.arg |= 1u << kEditRoomPickUp;
    if (p.room_type == kRoomCorridor)
        return;
    p.furnished = room_has_compulsory_objects(world, room);
    if (p.furnished) {
        p.arg = 0;
        p.hint = 0xe;
    } else {
        p.arg = 1u << kEditRoomConfirm;
        p.hint = 0xd;
    }
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

void build_turn(Game &game)
{
    if (game.world.building.build.state == 2)
        commit_doors(game);
    // game.cpp's end-of-turn edit-room check: once the room being edited is
    // empty, go on to placing its objects, or (no longer waiting) cancel the
    // edit.
    World &w = game.world;
    uint16_t r = w.building.build.room;
    if (!r)
        return;
    RoomInfo &room = w.rooms[r % kRoomCount];
    if (!room.being_edited || !is_room_empty(w, r))
        return;
    if (w.building.mouse == BuildMouse::EditRoomWait) {
        if (room.first_person == 0)
            apply_build_event(game, Event{EV_SETUP_PLACE_OBJECT_REQUESTER, r, 0, 0});
        return;
    }
    apply_build_event(game, Event{EV_CANCEL_EDIT_ROOM, 0, 0, uint16_t(w.building.mouse)});
}

namespace
{

// apply_build_event on the requester and object placing events: what
// apply_build_event returns for them, or nothing for other events.
std::optional<bool> apply_build_event_requesters(Game &game, const Event &event)
{
    World &w = game.world;
    ObjectPlacing &p = w.placing;
    switch (event.type) {
    case EV_OPEN_HIRE_STAFF:
    case EV_OPEN_FURNISH_CORRIDOR:
        // cancel_top_level_requesters closes the build room requester; the
        // rest of the command is the hire staff and objects parts'.
        if (p.requester == ObjectRequester::BuildRoom)
            p.requester = ObjectRequester::None;
        return false;
    case EV_OPEN_BUILD_ROOM:
        apply_build_event_open_build_room(p);
        return true;
    case EV_REQ1_RESET_ROOMS:
        apply_build_event_reset_rooms(game, event.value);
        return true;
    case EV_CLOSE_REQUESTERS:
        p.requester = ObjectRequester::None;
        p.menu_disabled = false;
        return true;
    case EV_INIT_BLUEPRINT:
        apply_build_event_init_blueprint(game, event.value % kRoomTypes);
        return true;
    case EV_FURNISH_EDITED_ROOM:
        apply_build_event_furnish_edited_room(game);
        return true;
    case EV_BACK_TO_PLACE_OBJECT:
        apply_build_event_back_to_place_object(w, event.value != 0);
        return true;
    case EV_PLACE_OBJECT:
        if (!w.building.build.room || p.mode == 2)
            return false;
        apply_object_event(game, event);
        if (p.requester == ObjectRequester::EditRoom && p.requester_mode >= 6)
            set_room_requester_mode(w, p.requester_mode);
        return true;
    default:
        return std::nullopt;
    }
}

} // namespace

bool apply_build_event(Game &game, const Event &event)
{
    if (std::optional<bool> handled = apply_build_event_requesters(game, event))
        return *handled;
    World &w = game.world;
    Building &bd = w.building;
    switch (event.type) {
    case EV_MODIFY_BLUEPRINT:
        modify_blueprint(w, true, event.value);
        return true;
    case EV_HOVER_BLUEPRINT:
        modify_blueprint(w, false, event.value);
        return true;
    case EV_COMMIT_BLUEPRINT:
        play(w, commit_blueprint(game) ? kSfxYesX : kSfxWrong2);
        return true;
    case EV_UNCOMMIT_BLUEPRINT:
        uncommit_blueprint(w);
        bd.mouse = BuildMouse::Blueprint;
        return true;
    case EV_TEMP_PLACE_DOOR:
        temp_place_door(w, event.value);
        return true;
    case EV_PLACE_BP_DOOR:
        place_bp_door(game, event.value != 0);
        return true;
    case EV_COMMIT_DOORS:
        bd.mouse = BuildMouse::WaitClear;
        commit_doors(game);
        return true;
    case EV_OPEN_ROOM:
        open_room(game);
        return true;
    case EV_EDIT_BLUEPRINT:
        edit_blueprint(w);
        return true;
    case EV_INIT_EDIT_ROOM:
        apply_build_event_init_edit_room(game, event.value);
        return true;
    case EV_PICK_UP_OBJECT:
        apply_build_event_pick_up_object(game, event);
        play(w, kSfxPickUpSmall);
        return true;
    case EV_SETUP_PLACE_OBJECT_REQUESTER:
        apply_build_event_setup_place_object(game);
        return true;
    case EV_CANCEL_EDIT_ROOM:
        apply_build_event_cancel_edit_room(game, event);
        return true;
    case EV_DELETE_ROOM:
        apply_build_event_delete_room(game, event);
        return true;
    case EV_CANCEL_BUILD:
        cancel_build(game);
        return false;
    default:
        return false;
    }
}

uint16_t compute_object_pointed(const World &world, uint16_t pointed, uint16_t room)
{
    const Thing *obj = pointed < world.things.size() ? &world.things[pointed] : nullptr;
    if (!obj || obj->owner != kPlayer)
        return 0;
    if (!room)
        return pointed;
    if (obj->type == kThingObject && obj->room_id == room && obj->subtype != kObjSwingDoor &&
        obj->subtype != kObjSwingDoor2)
        return pointed;
    return 0;
}

void compute_room_pointed(World &world, int tile, uint16_t pointed, uint16_t &pickup_thing)
{
    uint16_t r = room_at(world, uint16_t(tile));
    BuildState &b = world.building.build;
    if (r && world.rooms[r % kRoomCount].type != kRoomDestroyed) {
        const RoomInfo &room = world.rooms[r % kRoomCount];
        b.edit_room = r;
        b.draw_mode = 2;
        b.x0 = uint8_t(room.pos % 128);
        b.y0 = uint8_t(room.pos / 128);
        b.x1 = uint8_t(room.pos % 128 + room.width - 1);
        b.y1 = uint8_t(room.pos / 128 + room.height - 1);
        return;
    }
    b.edit_room = 0;
    b.draw_mode = 0;
    const Thing *p = pointed < world.things.size() ? &world.things[pointed] : nullptr;
    if (!p)
        return;
    if (p->type == kThingObject) {
        if (p->room_id == 0 && !(kObjectFlags[p->subtype % kObjectKinds] & 1) && p->owner == kPlayer)
            pickup_thing = pointed;
    } else if (p->type == kThingPatient || p->type == kThingStaff) {
        const Thing *used = p->object < world.things.size() ? &world.things[p->object] : nullptr;
        if (used && (used->type == kThingObject || used->type == kThingDoor) && used->user == pointed &&
            used->room_id == 0 && p->owner == kPlayer)
            pickup_thing = pointed;
    }
}

Event clicked_for_edit(World &world, uint16_t pickup_thing)
{
    Event e;
    // The edit or buy-objects requester open means the object is lifted with
    // its other half; click_to_edit's `edit`.
    const ObjectRequester requester = world.placing.requester;
    bool edit =
        requester == ObjectRequester::EditRoom || (requester == ObjectRequester::BuyObjects && world.placing.arg != 1);
    if (pickup_thing && pickup_thing < world.things.size()) {
        const Thing &p = world.things[pickup_thing];
        if (edit && p.room_id != world.building.build.edit_room)
            return e;
        e.type = EV_PICK_UP_OBJECT;
        e.value = p.type == kThingObject ? pickup_thing : p.object;
        e.arg = edit ? 1 : 0;
    } else if (world.building.build.edit_room) {
        e.type = EV_INIT_EDIT_ROOM;
        e.value = world.building.build.edit_room;
    }
    return e;
}

Event pick_up_object_right_click(World &world, uint16_t pointed)
{
    const Thing *clicked = pointed < world.things.size() ? &world.things[pointed] : nullptr;
    if (!clicked)
        return {};
    // LINKED_OBJECT: a patient using an object.
    if (clicked->type == kThingPatient) {
        const Thing *used = clicked->object < world.things.size() ? &world.things[clicked->object] : nullptr;
        if (used && (used->type == kThingObject || used->type == kThingDoor) && used->user == pointed)
            clicked = used;
    }
    if (clicked->type == kThingObject && clicked->owner == kPlayer &&
        (!clicked->room_id || (kObjectFlags[clicked->subtype % kObjectKinds] & 8)) &&
        !(kObjectFlags[clicked->subtype % kObjectKinds] & 1))
        return clicked_for_edit(world, pointed);
    return {};
}

Event check_object_right_click_in_room_edit(const World &world, uint16_t pointed)
{
    const Thing *obj = pointed < world.things.size() ? &world.things[pointed] : nullptr;
    if (obj && obj->type == kThingObject && obj->room_id == world.building.build.room)
        return {EV_PICK_UP_OBJECT, pointed, 0, 1};
    return {};
}

void cancel_room_edit(World &world)
{
    BuildState &b = world.building.build;
    if (!b.room)
        return;
    RoomInfo &room = world.rooms[b.room % kRoomCount];
    room.being_edited = 0;
    for (int x = b.x0; x <= b.x1 + 1; x++)
        for (int y = b.y0; y <= b.y1 + 1; y++)
            place_small_thing(world, cell_index(x, y), 0, false);
    b.room = 0;
}

} // namespace th
