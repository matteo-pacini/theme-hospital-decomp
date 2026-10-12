// The room blueprint: which cells are valid, drawing it on the map, and
// committing, resizing and moving it (src/blueprnt.cpp, src/build.cpp).

#include "core/world/build.h"
#include "core/world/build_internal.h"

#include "core/data/blocks.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/objects.h"
#include "core/world/rats.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

namespace th
{

namespace
{

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

} // namespace

inline namespace build_detail
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
} // namespace build_detail

namespace
{

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

} // namespace

inline namespace build_detail
{

// is_tile_valid (src/blueprnt.cpp): only people stand on `tile`, its object
// flags are clear and it is `owner`'s indoor land. With `flags`
// (advisor.flags), sets kAdvisorBadPlacement where the game does: for an
// object that is not a person, and for land that is not `owner`'s indoors.
bool is_tile_valid(const World &w, int tile, int owner, uint8_t *flags)
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
bool is_valid_no_room(const World &w, int tile, uint8_t *flags)
{
    if (room_at(w, uint16_t(tile)) == 0)
        return true;
    if (flags)
        *flags |= kAdvisorBadPlacement;
    return false;
}
} // namespace build_detail

namespace
{

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

} // namespace

inline namespace build_detail
{

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
bool is_room_size_valid(const World &w, int x0, int y0, int x1, int y1, uint8_t *flags)
{
    const RoomTypeInfo &type = kRoomTypeInfo[w.building.build.room_type % kRoomTypes];
    uint16_t width = uint16_t(x1 - x0 + 1), height = uint16_t(y1 - y0 + 1);
    bool ok = width >= type.min_width && height >= type.min_height;
    if (flags)
        *flags = ok ? uint8_t(*flags & ~kAdvisorBlueprintSmall) : uint8_t(*flags | kAdvisorBlueprintSmall);
    return ok;
}
} // namespace build_detail

namespace
{

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

} // namespace

inline namespace build_detail
{

// is_valid_door (src/blueprnt.cpp): the cell outside a door of `type` at
// `tile` is indoors and in no room.
bool is_valid_door(const World &w, int tile, int type)
{
    int pos = tile + kDoorTypes[size_t(type % 4 + 4)].door_offset;
    if (!in_map(pos))
        return false;
    return !(w.map[size_t(pos)].owner_flags & kCellOutdoor) && room_at(w, uint16_t(pos)) == 0;
}
} // namespace build_detail

namespace
{

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

} // namespace

inline namespace build_detail
{

// check_room_access_with_blueprint (src/blueprnt.cpp).
bool check_room_access_with_blueprint(World &w)
{
    blank_wall_around_blueprint(w, true);
    bool ok = check_room_access(w, kPlayer);
    blank_wall_around_blueprint(w, false);
    return ok;
}
} // namespace build_detail

namespace
{

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

} // namespace

inline namespace build_detail
{

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
} // namespace build_detail

// cleanse_area (src/rats.cpp) is in rats.cpp: it removes the rat holes and
// the litter on the box of cells.

inline namespace build_detail
{

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
} // namespace build_detail

namespace
{

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

} // namespace

inline namespace build_detail
{

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
} // namespace build_detail

} // namespace th
