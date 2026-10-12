// Placing a room's door: the door's blueprint pieces, the position that suits the
// pointer and place_door (src/blueprnt.cpp, src/build.cpp).

#include "core/world/build.h"
#include "core/world/build_internal.h"

#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/nav.h"
#include "core/world/objects.h"
#include "core/world/rats.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <cstdlib>

namespace th
{

namespace
{

constexpr uint8_t kObjDoor = 3;

// OS_STATIC, OS_IN_USE and OS_REMOVED (include/enums.h).
constexpr uint8_t kObjectStatic = 2;

// SFX_* (include/enums.h).
constexpr uint8_t kSfxBuildClick = 0x1e;

constexpr uint8_t kSfxDeBuild = 0x97;

// forced_advice_id messages (language section 0x14).
constexpr uint16_t kAdviceDoorBlocks = 6;

// door_to_room (src/tables.cpp): by facing, the offset from a door's cell
// to the cell of the room it belongs to.
constexpr std::array<int16_t, 4> kDoorToRoom = {0, -1, -128, 0};

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

} // namespace

inline namespace build_detail
{

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
} // namespace build_detail

namespace
{

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

} // namespace

inline namespace build_detail
{

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
} // namespace build_detail

} // namespace th
