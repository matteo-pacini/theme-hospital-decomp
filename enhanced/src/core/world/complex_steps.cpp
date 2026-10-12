// A patient's complex steps: running the actions of a complex, abandoning
// it and failing it (src/complex.cpp, src/patient.cpp).

#include "core/world/complex_internal.h"

#include "core/world/complex_tables.h"
#include "core/world/counts.h"
#include "core/world/nav.h"
#include "core/world/patient_tables.h"
#include "core/world/reception.h"
#include "core/world/rooms.h"
#include "core/world/trace.h"
#include "core/world/vaccine.h"

#include <cstdlib>

namespace th
{

namespace
{

// RoomInfo.state of a room being furnished or edited (RS_EDITED).
constexpr int8_t kRoomEdited = 4;

// The game's PLAY_FRAME after person_head_for: faces the step's direction,
// if any, and starts walking.
void head_off(const Run &r, uint16_t id)
{
    Thing &p = r.world.things[id];
    int dir = person_head_for(r.world, id);
    if (dir >= 0 && dir < 4)
        p.facing = uint8_t(dir);
    play_patient_anim(r.anims, p, kAnimWalk);
}

} // namespace

inline namespace complex_detail
{

// The game's PLAY_FRAME of patient_stand after a failed or abandoned
// complex.
void stand(const Run &r, Thing &p)
{
    play_patient_anim(r.anims, p, kAnimStand);
}
} // namespace complex_detail

inline namespace complex_detail
{

void fail(const Run &r, uint16_t id)
{
    Thing &p = r.world.things[id];
    trace_event("fail_complex %u", unsigned(id));
    if (p.resume_state) {
        p.state = p.resume_state;
        p.complex_pc = p.complex_queued = p.complex_queued2 = 0;
        p.complex_action = p.complex_resume = 0;
        stand(r, p);
    } else {
        p.state = kPsComplexRetry;
    }
}
} // namespace complex_detail

namespace
{

// The other half of double door `door` swings open as `user` goes through,
// at their pace.
void swing_partner(const Run &r, const Thing &door, const Thing &user)
{
    Thing &other = r.world.things[door.door_partner];
    other.state = kOsDoubleDoorSwings;
    size_t start = kAnimSwingOpen + user.facing;
    if (start < r.anims.starts.size()) {
        other.sprite = uint16_t(r.anims.starts[start].frame);
        other.flip = uint8_t(r.anims.starts[start].flip);
    }
    other.anim_speed = user.anim_speed;
}

// Action 22 of a patient's complex: off its door onto the cell beyond
// (exit_tile), in that cell's room or none; a single door free again, a
// double door swinging shut.
void leave_door(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    Thing &door = world.things[p.object];
    MapPos x{0x80, uint8_t(p.exit_tile % kMapSize)};
    MapPos y{0x80, uint8_t(p.exit_tile / kMapSize)};
    p.dest_x = x.tile;
    p.dest_y = y.tile;
    move_node(world, id, x, y);
    door.user = 0;
    p.object = 0;
    move_person_in_room_list(world, id, p.exit_tile);
    MapCell &cell = world.map[cell_of(door)];
    if (!door.door_cell_flag)
        cell.flags &= uint8_t(~kCellDoorInUse);
    if (door.subtype == kObjDoor) {
        door.in_use = 0;
        door.state = kOsStatic;
    } else {
        door.state = kOsDoubleDoorSwings;
        size_t start = kAnimSwingClose + p.facing;
        if (start < r.anims.starts.size()) {
            door.sprite = uint16_t(r.anims.starts[start].frame);
            door.flip = uint8_t(r.anims.starts[start].flip);
        }
    }
}

// What a step of a complex returns when it carried on; any other value is
// what process returns at once.
constexpr uint8_t kStepDone = 1;

// Action 6: onto the object, from its use cell use_pos[1].
uint8_t process_use_object(const Run &r, uint16_t id, uint16_t tile)
{
    World &world = r.world;
    Thing &p = world.things[id];
    Thing &obj = world.things[p.object];
    if (obj.in_use == 1 || obj.state == kOsRemoved || obj.type != kThingObject || obj.use_pos[1] != tile) {
        fail(r, id);
        return 3;
    }
    trace_event("use_object %u obj=%u kind=%u", unsigned(id), unsigned(p.object), unsigned(obj.subtype));
    obj.in_use = 1;
    obj.user = id;
    p.facing = obj.facing;
    if (obj.use_style == 1 || obj.use_style == 2 || obj.use_style == 3) {
        p.exit_tile = obj.use_pos[4];
        move_node(world, id, obj.use_x, obj.use_y);
        obj.state = kOsInUse;
        p.depth = obj.depth;
        if (kObjectUse[obj.subtype].overlay_anim) {
            obj.saved_depth = obj.depth;
            obj.depth = 8;
        }
        world.map[cell_of(p)].flags |= kCellBig;
    }
    for (size_t i = 10; i < 13; i++)
        p.layers[i] = obj.layers[i];
    for (size_t i = 0; i < 5; i++)
        obj.layers[i] = p.layers[i];
    return kStepDone;
}

// Action 8: off the object, back onto its use cell use_pos[1].
void process_leave_object(const Run &r, uint16_t id, uint16_t tile)
{
    World &world = r.world;
    Thing &p = world.things[id];
    Thing &obj = world.things[p.object];
    obj.user = 0;
    p.object = 0;
    obj.in_use = 0;
    if (obj.use_style == 1 || obj.use_style == 3) {
        adjust_big_cell_flag(world, tile);
        move_node(world, id, {0x80, uint8_t(obj.use_pos[1] % kMapSize)}, {0x80, uint8_t(obj.use_pos[1] / kMapSize)});
        p.depth = 0;
        if (kObjectUse[obj.subtype].overlay_anim) {
            obj.depth = obj.saved_depth;
            obj.saved_depth = 0;
        }
        obj.state = kOsStatic;
    }
    stand(r, p);
    p.layers[10] = p.layers[11] = p.layers[12] = 0;
}

// Action 18: faces the way its object's kind faces its user.
void process_face_object(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (!p.object)
        return;
    const Thing &obj = world.things[p.object];
    int face = kObjectUse[obj.subtype].face_dirn;
    // Kinds whose face_dirn is -2 or -99 make the game read before
    // face_dirns; here they leave the facing.
    if (face < 0 || size_t(face) >= kFaceDirns.size())
        return;
    int8_t d = kFaceDirns[size_t(face)][size_t(obj.facing) + 4];
    if (d != -1) {
        p.facing = uint8_t(d);
        stand(r, p);
    }
}

// Action 20: onto the door of the cell it faces, unless the room beyond is
// being edited; a double door's other half swings open.
uint8_t process_enter_door(const Run &r, uint16_t id, uint16_t tile)
{
    World &world = r.world;
    Thing &p = world.things[id];
    uint16_t door_id = find_door(world, id);
    if (!door_id) {
        fail(r, id);
        return 0;
    }
    Thing &door = world.things[door_id];
    bool double_door = door.subtype != kObjDoor;
    if (door.in_use == 1) {
        p.state = kPsComplexRetry;
        return 3;
    }
    const RoomInfo &room = world.rooms[door.room_id];
    if (p.room == 0 && (room.being_edited || room.state == kRoomEdited)) {
        fail(r, id);
        return 3;
    }
    door.in_use = 1;
    p.facing = p.room ? door.facing : kOppositeAngle[door.facing & 3];
    p.exit_tile = tile == door.use_pos[0] ? door.use_pos[3] : door.use_pos[0];
    play_patient_anim(r.anims, p, double_door ? kAnimDoubleDoor : kAnimSingleDoor);
    p.object = door_id;
    door.user = id;
    move_node(world, id, door.x, door.y);
    door.state = kOsInUse;
    MapCell &cell = world.map[cell_of(door)];
    door.door_cell_flag = cell.flags & kCellDoorInUse ? 1 : 0;
    cell.flags |= kCellDoorInUse;
    if (double_door)
        swing_partner(r, door, p);
    return kStepDone;
}

// Action 32, transform_peep_who (0x16b30).
void process_transform(const Run &r, Thing &p, const ComplexStep &entry)
{
    int8_t kind = kChangePersonWho[entry.anim][p.subtype];
    p.orig_subtype = p.subtype;
    if (kind != -1) {
        p.subtype = uint8_t(kind);
        play_patient_anim(r.anims, p, kAnimWalk);
    }
}

// Action 16: a random animation of the step's group.
void process_group_anim(const Run &r, Thing &p, const ComplexStep &entry)
{
    const GroupAnim &group = kGroupAnims[entry.anim];
    uint32_t choice = old_newrand(r.world.seed, "process_person_complex") % uint32_t(group.count);
    play_patient_anim(r.anims, p, group.anims[choice]);
}

// The step's actions that sequence the complex, animate or change the
// patient alone.
void process_sequence_action(const Run &r, uint16_t id, const ComplexStep &entry)
{
    World &world = r.world;
    Thing &p = world.things[id];
    switch (entry.action) {
    case 4:
        play_patient_anim(r.anims, p, entry.anim);
        break;
    case 16:
        process_group_anim(r, p, entry);
        break;
    case 14:
        if (uint16_t door = find_door(world, id))
            p.facing = kOppositeAngle[world.things[door].facing & 3];
        break;
    case 24:
        p.complex_pc++;
        p.complex_resume = uint16_t(r.complexes.at(p.complex_pc) * 10);
        break;
    case 32:
        process_transform(r, p, entry);
        break;
    case 34:
        p.subtype = p.orig_subtype;
        break;
    case 46:
        head_off(r, id);
        break;
    case 52:
        complex_success(world, id, entry.anim);
        break;
    default:
        // 0, 12 and 26 do nothing, nor does 50 here (see process).
        break;
    }
}

// Runs the action of step `entry`: kStepDone, or the value process returns.
uint8_t process_action(const Run &r, uint16_t id, const ComplexStep &entry, uint16_t obj_id, uint16_t tile)
{
    World &world = r.world;
    Thing &p = world.things[id];
    switch (entry.action) {
    case 2:
        p.object = obj_id;
        [[fallthrough]];
    case 44:
        set_destination_coords(world, id, world.things[p.object].use_pos[1]);
        head_off(r, id);
        p.countdown = 0;
        return kStepDone;
    case 6:
        return process_use_object(r, id, tile);
    case 8:
        process_leave_object(r, id, tile);
        return kStepDone;
    case 10:
        if (p.object)
            play_frame(world.things[p.object], r.anims, entry.anim);
        return kStepDone;
    case 18:
        process_face_object(r, id);
        return kStepDone;
    case 20:
        return process_enter_door(r, id, tile);
    case 22:
        leave_door(r, id);
        return kStepDone;
    default:
        process_sequence_action(r, id, entry);
        return kStepDone;
    }
}

// Moves the complex on past step `entry`: at once, or after it has repeated
// entry.repeat times (a random 1 to -entry.repeat when negative).
void process_advance(World &world, Thing &p, const ComplexStep &entry)
{
    if (!entry.repeat) {
        p.complex_pc++;
        return;
    }
    if (p.complex_repeat) {
        p.complex_repeat--;
        if (p.complex_repeat == 0)
            p.complex_pc++;
    } else if (entry.repeat < 0) {
        p.complex_repeat = uint8_t(old_newrand(world.seed, "process_person_complex") % uint32_t(-entry.repeat) + 1);
    } else {
        p.complex_repeat = uint8_t(entry.repeat);
    }
}

// At the end of the complex, starts the one queued behind it (running its
// first step) or moves on to next_state; otherwise runs a chained step.
void process_finish(const Run &r, uint16_t id, const ComplexStep &entry)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (r.complexes.at(p.complex_pc) != 0) {
        if (entry.chain)
            process(r, id);
        return;
    }
    p.complex_pc = 0;
    uint16_t action = p.complex_queued;
    if (action) {
        uint16_t arg = uint16_t(p.queued_arg_x + (p.queued_arg_y << 7));
        p.complex_queued = p.complex_queued2;
        p.complex_queued2 = 0;
        p.queued_arg_x = p.queued2_arg_x;
        p.queued2_arg_x = 0;
        p.queued_arg_y = p.queued2_arg_y;
        p.queued2_arg_y = 0;
        if (set_complex(world, id, action, arg))
            process(r, id);
    } else if (p.next_state) {
        p.state = p.next_state;
        p.next_state = 0;
    }
}

} // namespace

inline namespace complex_detail
{

uint8_t process(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (p.complex_pc == 0)
        return 2;
    uint8_t op = r.complexes.at(p.complex_pc);
    if (op == 0) {
        p.complex_pc = 0;
        return 2;
    }
    uint16_t tile = cell_of(p);
    const ComplexStep &entry = kPersonComplex[op];
    // The game leaves this unset for a step of action 2 without an object
    // kind (person_complex[108], which no complex of P_COMP.LEX uses).
    uint16_t obj_id = 0;
    if (entry.object) {
        obj_id = find_nearest_free_object(world, id, entry.object);
        if (obj_id == 0) {
            fail(r, id);
            return 0;
        }
    }
    if (entry.state)
        p.state = entry.state;
    if (uint8_t result = process_action(r, id, entry, obj_id, tile); result != kStepDone)
        return result;
    process_advance(world, p, entry);
    process_finish(r, id, entry);
    // Action 50: the game runs process_things for the person alone.
    if (entry.action == 50) {
        process_things_alone_begin(world, id, r.game_turn);
        turn(r, id);
        final_count(world, r.game_turn);
    }
    return 1;
}
} // namespace complex_detail

inline namespace complex_detail
{

// abandon_complex (0x41f84), for a patient: runs kComplexAbandon (queued
// behind a running complex) for up to 12 steps, each from its destination
// cell, puts it back where it was unless it uses its object or changed
// room, and stands it in resume_state.
void abandon_complex(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    trace_event("abandon_complex %u", unsigned(id));
    MapPos old_x = p.x;
    MapPos old_y = p.y;
    bool user = p.object && world.things[p.object].user == id;
    set_complex(world, id, kComplexAbandon, 0);
    uint16_t room = p.room;
    for (int i = 0; i < 12; i++) {
        move_node(world, id, {0x80, p.dest_x}, {0x80, p.dest_y});
        if (process(r, id) != 1)
            break;
    }
    if (!user && room == p.room)
        move_node(world, id, old_x, old_y);
    p.state = p.resume_state;
    stand(r, p);
}
} // namespace complex_detail

inline namespace complex_detail
{

void remove(const Run &r, uint16_t id)
{
    leave_queue(r.world, id);
    abandon_complex(r, id);
    remove_person_from_room_list(r.world, id);
    cancel_vaccination_request(r.world, id);
    trace_event("remove_thing %u type=%u", unsigned(id), unsigned(r.world.things[id].type));
    remove_node(r.world, id);
    r.world.things[id] = Thing{};
}
} // namespace complex_detail

inline namespace complex_detail
{

void go_home(const Run &r, uint16_t id)
{
    person_go_home(r.world, r.anims, r.complexes, id, r.game_turn);
}
} // namespace complex_detail

inline namespace complex_detail
{

void start_going(const Run &r, uint16_t id)
{
    start_person_going(r.world, r.anims, r.complexes, id, r.game_turn);
}
} // namespace complex_detail

} // namespace th
