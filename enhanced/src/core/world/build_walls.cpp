// Building a committed blueprint into the map: its walls, doors and objects, the
// recolouring of walls and the window fill-in (src/build.cpp).

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

} // namespace

inline namespace build_detail
{

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
} // namespace build_detail

namespace
{

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

} // namespace

inline namespace build_detail
{

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
} // namespace build_detail

} // namespace th
