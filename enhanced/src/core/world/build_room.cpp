// Removing and editing rooms: remove_object_from_map, delete_room, the room edit
// mode, commit_doors, open_room and cancel_build (src/build.cpp, src/objects.cpp).

#include "core/world/build.h"
#include "core/world/build_internal.h"

#include "core/data/blocks.h"
#include "core/render/shadows.h"
#include "core/world/complex.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/nav.h"
#include "core/world/objects.h"
#include "core/world/queue.h"
#include "core/world/rats.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

namespace th
{

namespace
{

constexpr uint8_t kObjPlant = 45;

constexpr uint8_t kObjAutoAutopsy = 55;

// OS_STATIC, OS_IN_USE and OS_REMOVED (include/enums.h).
constexpr uint8_t kObjectInUse = 4;

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

} // namespace

inline namespace build_detail
{

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
} // namespace build_detail

namespace
{

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

} // namespace

inline namespace build_detail
{

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
} // namespace build_detail

namespace
{

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

} // namespace

inline namespace build_detail
{

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
} // namespace build_detail

namespace
{

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

} // namespace

inline namespace build_detail
{

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
} // namespace build_detail

} // namespace th
