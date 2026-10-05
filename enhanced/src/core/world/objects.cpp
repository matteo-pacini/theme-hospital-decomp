#include "core/world/objects.h"

#include "core/world/complex.h"
#include "core/world/events.h"
#include "core/world/game.h"
#include "core/world/nav.h"
#include "core/world/world.h"

namespace th
{

namespace
{

constexpr uint8_t kObjReceptionDesk = 11;
constexpr uint8_t kObjLitterBomb = 17;
constexpr uint8_t kObjPlant = 45;
constexpr uint8_t kObjGatesOfHell = 48;

// PS_WAITS_FOR_DECISION (include/enums.h).
constexpr uint8_t kPsWaitsForDecision = 0x44;

// The local player; the engine has no other.
constexpr int kPlayer = 0;

// next_angle (src/tables.cpp).
constexpr std::array<uint8_t, 4> kNextAngle = {1, 2, 3, 0};

// around24 (src/tables.cpp): the 24 cells nearest a cell, as offsets.
constexpr std::array<int16_t, 24> kAround24 = {
    -129, -128, -127, -1,   1,   127, 128, 129, -256, 2,    256, -2,
    -130, -257, -255, -126, 130, 257, 255, 126, -258, -254, 258, 254,
};

const ShapeInfo &shape_of(int kind, int dir)
{
    return kShapes[kObjects[size_t(kind)].shapes[size_t(dir)]];
}

// check_points_connected (src/utils.cpp).
bool points_connected(const World &w, int from, int to)
{
    OrthonavPath path;
    orthonav_init(w, path, from % kMapSize, from / kMapSize, to % kMapSize, to / kMapSize);
    return path.count != 0;
}

// The cell where the player's patients come in: the door tile of its first
// entrance (room_info[myhosp->entrances[0]].door_tile).
int entrance_tile(const World &w)
{
    return w.entrances[kPlayer][0];
}

// check_room_access (src/blueprnt.cpp) with no rooms: both use cells of each
// of the player's reception desks reachable from the entrance.
bool check_room_access(const World &w)
{
    int pos = entrance_tile(w);
    for (size_t i = 1; i < w.things.size(); i++) {
        const Thing &t = w.things[i];
        if (t.type == kThingObject && t.subtype == kObjReceptionDesk && t.owner == kPlayer &&
            (!points_connected(w, pos, t.use_pos[0]) || !points_connected(w, pos, t.use_pos[1])))
            return false;
    }
    return true;
}

// check_no_peeps_on_cell (src/objects.cpp) outside rooms: false if someone
// stands on `tile` and, unless side is -1, faces `side`, except patients
// in PS_ENTERTAINS_THEMSELF on any complex but 0x46, or awaiting a decision.
// The mayor, the inspector and the reaper do not exist yet.
bool no_peeps_on_cell(const World &w, int tile, int side)
{
    if (tile < 0 || tile >= kMapCells)
        return true;
    for (int id = w.map[size_t(tile)].first; id > 0; id = w.things[size_t(id)].cell_next) {
        const Thing &t = w.things[size_t(id)];
        if (t.type != kThingPatient && t.type != kThingStaff)
            continue;
        if (side == -1)
            return false;
        if (t.facing == side) {
            if (t.type == kThingPatient && t.state == kPsEntertainsThemself && t.complex_action != 0x46)
                return true;
            if (t.type == kThingPatient && t.state == kPsWaitsForDecision)
                return true;
            return false;
        }
    }
    return true;
}

// check_no_peeps_in_way (src/objects.cpp).
bool no_peeps_in_way(const World &w, int tile, int kind, int dir)
{
    const ShapeInfo &shape = shape_of(kind, dir);
    for (int i = shape.count - 1; i >= 0; i--) {
        int p = tile + shape.points[size_t(i)];
        if (!(no_peeps_on_cell(w, p, -1) && no_peeps_on_cell(w, p - 0x80, 2) && no_peeps_on_cell(w, p + 1, 3) &&
              no_peeps_on_cell(w, p + 0x80, 0) && no_peeps_on_cell(w, p - 1, 1)))
            return false;
    }
    return true;
}

// place_object (src/objects.cpp) for the player in the corridor: which_room
// is always 0 and nothing has been picked up, so a new object is bought at
// object_stats' cost (the level's StartCost; research does not lower it yet)
// and the balance is charged (change_balance TX_BUY_OBJECT).
uint16_t player_place_object(Game &game, int tile, int kind, int dir)
{
    World &w = game.world;
    if (tile < 0 || tile >= kMapCells || kind <= 0 || kind >= kObjectKinds || dir < 0 || dir >= 4)
        return 0;
    if (!(kind == kObjGatesOfHell || can_place_object(w, tile, kind, dir, kPlayer)))
        return 0;
    mark_object_shape(w, tile, kind, dir);
    bool blocked = !check_room_access(w);
    remove_object_shape(w, tile, kind, dir);
    // The game also sets advisor.forced_advice_id 7 when blocked; there is no
    // advisor yet.
    if (blocked || !no_peeps_in_way(w, tile, kind, dir))
        return 0;
    uint16_t id = place_object(w, game.anims(), tile, kind, dir, kPlayer, true);
    if (id) {
        int cost = game.settings().objects[size_t(kind)].start_cost;
        if (cost)
            change_balance(game.finance, game.balance, game.date.day, game.date.month, kTxBuyObject, -cost);
    }
    return id;
}

// corridor_object_nav_check (src/build.cpp) with no rooms. Only the first
// thing on each nearby cell is looked at, as in the game.
bool corridor_object_nav_check(World &w, int pos, int kind, int dir)
{
    const ShapeInfo &shape = shape_of(kind, dir);
    int entrance = entrance_tile(w);
    // The game picks use cell 0 for a reception desk only for the object
    // placed: for the objects round it its side variable is always 1 by the
    // time it is used.
    int side = kind == kObjReceptionDesk ? 0 : 1;
    bool ok = true;
    mark_object_shape(w, pos, kind, dir);
    if (!(kind == kObjPlant || kind == kObjLitterBomb ||
          points_connected(w, entrance, pos + shape.use_offsets[size_t(side)]))) {
        ok = false;
    } else {
        for (int16_t d : kAround24) {
            int tile = pos + d;
            if (tile < 0 || tile >= kMapCells)
                continue;
            int id = w.map[size_t(tile)].first;
            if (id <= 0)
                continue;
            const Thing &obj = w.things[size_t(id)];
            if (obj.type != kThingObject || obj.subtype == kObjPlant || obj.subtype == kObjLitterBomb)
                continue;
            const ShapeInfo &other = shape_of(obj.subtype, obj.facing);
            if (!points_connected(w, entrance, tile + other.use_offsets[1])) {
                ok = false;
                break;
            }
        }
    }
    remove_object_shape(w, pos, kind, dir);
    return ok;
}

// set_edit_room_requester_mode (src/req14.cpp) for modes 6 and 0xa in the
// corridor, opening the edit room requester if no requester is up
// (open_edit_room_requester).
void set_edit_room_requester_mode(ObjectPlacing &p, int mode)
{
    p.requester_mode = uint8_t(mode);
    if (mode == 6) {
        // Picking up is refused in the corridor, or in a room with no objects.
        p.arg |= 1u << kEditRoomPickUp;
    } else if (mode == 0xa) {
        p.arg = 1u << kEditRoomConfirm | 1u << kEditRoomFurnish | 1u << kEditRoomPickUp;
        p.hint = 0x10;
        p.room_type = kRoomCorridor;
    }
    if (p.requester == ObjectRequester::None)
        p.requester = ObjectRequester::EditRoom;
}

// setup_place_object_requester (src/build.cpp).
void setup_place_object_requester(ObjectPlacing &p)
{
    p.buy_kind_count = 0;
    for (int i = 0; i < kObjectKinds - 1; i++)
        if (p.buy_count[size_t(i)])
            p.buy_kind_count++;
    set_edit_room_requester_mode(p, 6);
}

// object_placed (src/build.cpp).
void object_placed(ObjectPlacing &p)
{
    p.buy_count[p.kind]--;
    setup_place_object_requester(p);
    if (p.buy_count[p.kind] == 0) {
        if (p.buy_kind_count == 0) {
            p.placing = false;
            if (p.mode == 2)
                p.requester = ObjectRequester::None;
            return;
        }
        init_place_object(p, next_object_to_build(p), p.mode, p.room, 0);
    } else {
        init_place_object(p, p.kind, p.mode, p.room, 0);
    }
}

// set_buyable_items (src/req1.cpp) for the corridor: the kinds the player
// can buy, available from the start of the level (equipment[].available is
// object_stats' equip_available, the level's StartAvail, while research and
// WhenAvail dates are not ported) and in the level (AvailableForLevel).
void set_buyable_items(ObjectPlacing &p, const Balance &settings)
{
    p.requester_mode = 3;
    size_t count = 0;
    for (uint8_t kind : kCorridorObjects) {
        const Balance::Object &o = settings.objects[kind];
        if (count < p.offered.size() && o.available_for_level == 1 && o.start_avail == 1)
            p.offered[count++] = kind;
    }
    for (; count < p.offered.size(); count++)
        p.offered[count] = 0;
}

// clear_build_objects (src/build.cpp) for the player: nothing is picked up
// to refund, so it only empties the buy list.
void clear_build_objects(ObjectPlacing &p)
{
    p.buy_count = {};
}

} // namespace

int next_object_to_build(const ObjectPlacing &p)
{
    for (int i = 0; i < kObjectKinds - 1; i++)
        if (p.buy_count[size_t(i)])
            return i;
    return 0;
}

bool change_placing_angle(ObjectPlacing &p, int angle)
{
    const ObjectInfo &control = kObjects[p.kind];
    if (angle == -1) {
        angle = kNextAngle[p.dir];
        while (control.shapes[size_t(angle)] == 0)
            angle = kNextAngle[size_t(angle)];
    } else if (!control.shapes[size_t(angle)]) {
        angle = 0;
    }
    bool changed = p.dir != angle;
    p.dir = uint8_t(angle);
    p.redraw = true;
    return changed;
}

void init_place_object(ObjectPlacing &p, int kind, int mode, int room, int offset)
{
    if (mode == 1)
        mode = 0;
    // The game then makes mode 0 mode 1 for objects hugging walls
    // (wall_hug), which are placed in rooms; rooms do not exist yet.
    p.kind = uint8_t(kind);
    p.mode = uint8_t(mode);
    p.room = uint16_t(room);
    p.offset = int16_t(offset);
    change_placing_angle(p, p.dir);
    p.placing = true;
    if (mode == 2)
        p.room_type = kRoomCorridor;
    setup_place_object_requester(p);
    p.arg3 = uint16_t(kind);
    if (mode == 2)
        set_edit_room_requester_mode(p, 0xa);
}

void select_object_to_place(ObjectPlacing &p, int kind)
{
    p.arg3 = uint16_t(kind);
    init_place_object(p, kind, p.mode, p.room, 0);
    set_edit_room_requester_mode(p, 6);
}

void update_placement(World &world, ObjectPlacing &p, int mouse_cell)
{
    if (!p.placing || (mouse_cell == p.mouse_cell && !p.redraw))
        return;
    p.mouse_cell = mouse_cell;
    bool force = p.redraw;
    p.redraw = false;
    // Modes 0 and 1 keep the object inside room p.room
    // (place_object_restrict_to_room, place_object_wall_hug); there are no
    // rooms yet.
    if (p.mode != 2)
        return;
    int tile = mouse_cell + p.offset;
    if (tile == p.tile && !force)
        return;
    p.tile = uint16_t(tile);
    int kind = p.kind, dir = p.dir;
    p.ok = tile >= 0 && tile < kMapCells && can_place_object(world, tile, kind, dir, kPlayer);
    if (!p.ok)
        return;
    int8_t part = kObjects[size_t(kind)].next_part;
    if (part != -1) {
        const NextPartInfo &next = kNextParts[size_t(part)];
        int tile2 = tile + next.offset[size_t(dir)];
        p.ok = tile2 >= 0 && tile2 < kMapCells &&
               can_place_object(world, tile2, next.kind, next.dir[size_t(dir)], kPlayer);
        if (!p.ok)
            return;
    }
    // In a room (which_room) the game checks the room's objects instead
    // (corridor_object_room_nav_check); there are no rooms yet.
    p.ok = corridor_object_nav_check(world, tile, kind, dir);
}

void cancel_buy_objects(ObjectPlacing &p)
{
    if (p.requester == ObjectRequester::BuyObjects && !p.arg3) {
        clear_build_objects(p);
        p.requester = ObjectRequester::None;
    }
}

bool apply_object_event(Game &game, const Event &event)
{
    ObjectPlacing &p = game.world.placing;
    switch (event.type) {
    case EV_OPEN_FURNISH_CORRIDOR:
        // cancel_top_level_requesters(REQ_BUY_OBJECTS) closes the build room
        // and hire staff requesters: the interface does (see
        // core/ui/buy_objects.h).
        if (p.requester != ObjectRequester::None)
            return true;
        p.arg = 1;
        set_buyable_items(p, game.settings());
        p.requester = ObjectRequester::BuyObjects;
        p.room_type = kRoomCorridor;
        p.arg3 = 0;
        p.requester_mode = 3;
        p.item = p.offered[0];
        p.menu_disabled = true;
        return true;
    case EV_BUY_OBJECT:
        // req1_buy_object (src/req1.cpp).
        if (event.value < kObjectKinds) {
            uint8_t &count = p.buy_count[event.value];
            count = count + event.arg < 99 ? uint8_t(count + event.arg) : 99;
        }
        return true;
    case EV_SELL_OBJECT:
        // req11_sell_object (src/req11.cpp).
        if (event.value < kObjectKinds) {
            uint8_t &count = p.buy_count[event.value];
            count = count >= event.arg ? uint8_t(count - event.arg) : 0;
        }
        return true;
    case EV_PLACE_NEXT_OBJECT:
        p.requester = ObjectRequester::None;
        if (int kind = next_object_to_build(p))
            init_place_object(p, kind, 2, 0, 0);
        p.menu_disabled = false;
        return true;
    case EV_CANCEL_BUY_OBJECTS:
        p.requester = ObjectRequester::None;
        p.menu_disabled = false;
        return true;
    case EV_PLACE_OBJECT:
        if (player_place_object(game, event.value, event.arg & 0xff, event.arg >> 8))
            object_placed(p);
        return true;
    case EV_CANCEL_BUILD:
        clear_build_objects(p);
        p.requester = ObjectRequester::None;
        p.placing = false;
        p.kind = 0;
        p.menu_disabled = false;
        return false;
    default:
        return false;
    }
}

} // namespace th
