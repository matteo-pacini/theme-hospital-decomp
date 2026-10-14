// Applying the build events, build_turn and the pointer queries of room editing
// (src/build.cpp).

#include "core/world/build.h"
#include "core/world/build_internal.h"

#include "core/world/complex.h"
#include "core/world/events.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/objects.h"
#include "core/world/rats.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <optional>

namespace th
{

namespace
{

// RoomInfo.type of a room destroyed by a machine disaster (ROOM_DESTROYED).
constexpr uint8_t kRoomDestroyed = 6;

// SFX_* (include/enums.h).
constexpr uint8_t kSfxYesX = 0x41;

constexpr uint8_t kSfxPickUpSmall = 0xba;

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

} // namespace

inline namespace build_detail
{

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
} // namespace build_detail

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

// apply_build_event on EV_CLOSE_YES_NO: set_mouse_mode(arg) for the no event
// of an edited room's question, which names MM_BLUEPRINT.
void apply_build_event_close_yes_no(World &w, const Event &event)
{
    if (event.arg == uint16_t(BuildMouse::Blueprint))
        w.building.mouse = BuildMouse::Blueprint;
}

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
    case EV_CLOSE_YES_NO:
        apply_build_event_close_yes_no(w, event);
        return false;
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
    case EV_CLOSE_QUEUE_WINDOW: // set_mouse_mode(MM_NORMAL)
        bd.mouse = BuildMouse::None;
        return false;
    case EV_RETURN_FROM_SCREEN:
    case EV_RETURN_TO_THING_WINDOW:
        // return_from_screen's set_mouse_mode(MM_NORMAL); the blueprint, door
        // and pick-up modes come back from the edit-room requester's mode, so
        // they stay.
        if (bd.mouse == BuildMouse::EditRoom || bd.mouse == BuildMouse::MoveQueuePerson)
            bd.mouse = BuildMouse::None;
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
