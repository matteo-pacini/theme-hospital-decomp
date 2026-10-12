// The staff's interactions and the actions of their complexes' steps
// (src/interact.cpp, and the actions of process_staff_complex in
// src/complex.cpp).

#include "core/world/nav.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/staff_complex_internal.h"
#include "core/world/staff_complex_tables.h"
#include "core/world/trace.h"

namespace th
{

inline namespace staff_complex_detail
{

// setup_next_interaction (0x549bc): starts the next step of `id`'s
// interaction (or the current one again while it repeats), playing its
// animations on `id` and its slave; at the sequence's end clears both and
// goes on with `id`'s complex.
void setup_next_interaction(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &w = world.things[id];
    uint8_t step;
    if (w.interact_repeat > 0) {
        step = w.seq_index;
        w.interact_repeat--;
    } else {
        step = ++w.seq_index;
        w.interact_repeat = int8_t(kInteractions[step].count - 1);
    }
    int16_t anim = kInteractions[step].anim;
    if (anim == -1) {
        w.seq_index = 0;
        w.interact_anim = 0;
        if (w.slave) {
            Thing &partner = world.things[w.slave];
            partner.seq_index = 0;
            partner.interact_anim = 0;
        }
        process_either(r, id);
        return;
    }
    if (anim) {
        w.interact_anim = 1;
        if (w.type == kThingStaff)
            play_frame(w, r.anims, kStaffAnimStarts[w.subtype][size_t(anim)]);
        else
            play_frame(w, r.anims, kPatientAnimStarts[w.subtype][size_t(anim)]);
    } else {
        w.interact_anim = 0;
    }
    if (w.slave) {
        Thing &partner = world.things[w.slave];
        anim = kInteractions[step].partner_anim;
        if (anim) {
            partner.interact_anim = 1;
            if (partner.type == kThingPatient)
                play_frame(partner, r.anims, kPatientAnimStarts[partner.subtype][size_t(anim)]);
            else
                play_frame(partner, r.anims, kStaffAnimStarts[partner.subtype][size_t(anim)]);
        } else {
            partner.interact_anim = 0;
        }
    }
}

void start_interaction(const Run &r, uint16_t id, uint8_t seq)
{
    Thing &w = r.world.things[id];
    switch (w.type) {
    case kThingPatient:
        w.state = kPsTwoPeepsInteract;
        break;
    case kThingStaff:
        w.state = kStaffTwoPeepsInteract;
        break;
    default:
        return;
    }
    w.seq_index = uint8_t(kInteractionStarts[seq] - 1);
    w.interact_repeat = 0;
    setup_next_interaction(r, id);
}

// Advances an interacting thing's animation, noting in interact_anim when
// it has played through.
void interaction_frame(const Run &r, Thing &t)
{
    if (t.interact_anim != 0 && r.game_turn % t.anim_speed == 0) {
        t.sprite = r.anims.frames[t.sprite].next;
        if (r.anims.frames[t.sprite].flags & 1)
            t.interact_anim = 2;
    }
}

namespace
{

// merge_slave (0x4042c).
void merge_slave(World &world, uint16_t id)
{
    Thing &e = world.things[id];
    if (!e.slave)
        return;
    Thing &slave = world.things[e.slave];
    if (slave.state != kPsSlaveReady)
        return;
    slave.state = kPsMerged;
    if (e.object) {
        const Thing &obj = world.things[e.object];
        for (size_t i = 10; i < 13; i++)
            e.layers[i] = obj.layers[i];
    }
    for (size_t i = 0; i < 5; i++)
        e.layers[i] = slave.layers[i];
    slave.object = e.object;
}

// Action 56, staff_go_near_object (0x42748).
uint8_t process_go_near_object(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &e = world.things[id];
    const Thing &obj = world.things[e.object];
    uint16_t pos = can_i_go_near_point(world, e, cell_of(obj));
    if (!pos) {
        fail(r, id);
        return 0;
    }
    set_destination_coords(world, id, pos);
    head_off(r, id);
    e.countdown = 0;
    return kStepDone;
}

// Action 6: onto the object, from its use cell on the member of staff's side.
uint8_t process_use_object(const Run &r, uint16_t id, uint16_t tile, size_t side)
{
    World &world = r.world;
    Thing &e = world.things[id];
    Thing &obj = world.things[e.object];
    if (obj.in_use == 1 || obj.state == kOsRemoved || obj.type != kThingObject || obj.use_pos[side] != tile) {
        fail(r, id);
        return 3;
    }
    trace_event("use_object %u obj=%u kind=%u", unsigned(id), unsigned(e.object), unsigned(obj.subtype));
    obj.in_use = 1;
    obj.user = id;
    e.facing = obj.facing;
    if (obj.use_style == 1 || obj.use_style == 2) {
        e.exit_tile = obj.use_pos[side + 3];
        move_node(world, id, obj.use_x, obj.use_y);
        obj.state = kOsInUse;
        e.depth = obj.depth;
        if (kObjectUse[obj.subtype].overlay_anim) {
            obj.saved_depth = obj.depth;
            obj.depth = 8;
        }
        world.map[cell_of(e)].flags |= kCellBig;
    }
    for (size_t i = 10; i < 13; i++)
        e.layers[i] = obj.layers[i];
    return kStepDone;
}

// Action 8: off the object, onto its leave cell on the member of staff's
// side (a slave of a use style 2 object onto use_pos[4]).
void process_leave_object(const Run &r, uint16_t id, uint16_t tile, size_t side)
{
    World &world = r.world;
    Thing &e = world.things[id];
    Thing &obj = world.things[e.object];
    obj.user = 0;
    e.object = 0;
    obj.in_use = 0;
    if (obj.use_style == 2 || obj.use_style == 1) {
        adjust_big_cell_flag(world, tile);
        uint16_t leave = obj.use_pos[side + 3];
        move_node(world, id, {0x80, uint8_t(leave % kMapSize)}, {0x80, uint8_t(leave / kMapSize)});
        e.depth = 0;
        if (kObjectUse[obj.subtype].overlay_anim) {
            obj.depth = obj.saved_depth;
            obj.saved_depth = 0;
        }
        obj.state = kOsStatic;
        if (obj.use_style == 2 && e.slave) {
            move_node(world, e.slave, {0x80, uint8_t(obj.use_pos[4] % kMapSize)},
                      {0x80, uint8_t(obj.use_pos[4] / kMapSize)});
            world.things[e.slave].object = 0;
        }
    }
    play_staff_anim(r.anims, e, kAnimStand);
    e.layers[10] = e.layers[11] = e.layers[12] = 0;
}

// Action 18: faces the way its object's kind faces its user.
void process_face_object(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &e = world.things[id];
    if (!e.object)
        return;
    const Thing &obj = world.things[e.object];
    int face = kObjectUse[obj.subtype].face_dirn;
    // Kinds whose face_dirn is -2 or -99 make the game read before
    // face_dirns; here they leave the facing.
    if (face < 0 || size_t(face) >= kFaceDirns.size())
        return;
    int8_t d = kFaceDirns[size_t(face)][obj.facing];
    if (d != -1) {
        e.facing = uint8_t(d);
        play_staff_anim(r.anims, e, kAnimStand);
    }
}

// Action 20: onto the door of the cell it faces, claiming the room it
// enters; a double door's other half swings open.
uint8_t process_enter_door(const Run &r, uint16_t id, const ComplexStep &entry, uint16_t tile)
{
    World &world = r.world;
    Thing &e = world.things[id];
    uint16_t door_id = find_door(world, id);
    if (!door_id) {
        fail(r, id);
        return 4;
    }
    Thing &door = world.things[door_id];
    bool double_door = door.subtype != kObjDoor;
    if (door.in_use == 1) {
        e.state = kStaffComplexRetry;
        return 3;
    }
    e.facing = e.room ? door.facing : kOppositeAngle[door.facing & 3];
    e.exit_tile = tile == door.use_pos[0] ? door.use_pos[3] : door.use_pos[0];
    if (entry.anim == 1 && !check_room_mutex_on_entry(world, id)) {
        fail(r, id);
        return 4;
    }
    const RoomInfo &room = world.rooms[door.room_id];
    if (e.room == 0 && (room.being_edited || room.state == kRoomEdited ||
                        (door.owner == 0 && door.room_id == world.building.build.room))) {
        fail(r, id);
        return 4;
    }
    play_staff_anim(r.anims, e, double_door ? kAnimDoubleDoor : kAnimSingleDoor);
    door.in_use = 1;
    e.object = door_id;
    door.user = id;
    move_node(world, id, door.x, door.y);
    door.state = kOsInUse;
    MapCell &cell = world.map[cell_of(door)];
    door.door_cell_flag = cell.flags & kCellDoorInUse ? 1 : 0;
    cell.flags |= kCellDoorInUse;
    if (double_door) {
        Thing &other = world.things[door.door_partner];
        other.state = kOsDoubleDoorSwings;
        set_start(other, r.anims, kAnimSwingOpen + e.facing);
        other.anim_speed = e.anim_speed;
    }
    return kStepDone;
}

// Action 22: off its door onto exit_tile, in that cell's room or none
// (answering its pager there); a single door free again, a double door
// swinging shut.
void process_leave_door(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &e = world.things[id];
    clear_my_mutex(world, id);
    Thing &door = world.things[e.object];
    door.user = 0;
    e.object = 0;
    MapPos x{0x80, uint8_t(e.exit_tile % kMapSize)};
    MapPos y{0x80, uint8_t(e.exit_tile / kMapSize)};
    e.dest_x = x.tile;
    e.dest_y = y.tile;
    move_node(world, id, x, y);
    move_person_in_room_list(world, id, e.exit_tile);
    if (e.room) {
        world.rooms[e.room].staff_free = 0;
        pager_arrival(world, id);
    }
    MapCell &cell = world.map[cell_of(door)];
    if (!door.door_cell_flag)
        cell.flags &= uint8_t(~kCellDoorInUse);
    if (door.subtype == kObjDoor) {
        door.in_use = 0;
        door.state = kOsStatic;
    } else {
        door.state = kOsDoubleDoorSwings;
        set_start(door, r.anims, kAnimSwingClose + e.facing);
    }
}

// Action 48: the slave off its object, onto the object's use_pos[4].
void process_move_slave_off(World &world, const Thing &e)
{
    if (e.slave && e.object) {
        uint16_t leave = world.things[e.object].use_pos[4];
        move_node(world, e.slave, {0x80, uint8_t(leave % kMapSize)}, {0x80, uint8_t(leave / kMapSize)});
        world.things[e.slave].object = 0;
    }
}

// Action 16: a random animation of the step's group.
void process_group_anim(const Run &r, Thing &e, const ComplexStep &entry)
{
    const GroupAnim &group = kGroupAnims[entry.anim];
    auto choice = int8_t(old_newrand(r.world.seed, "process_staff_complex") % uint32_t(group.count));
    play_staff_anim(r.anims, e, size_t(group.anims[size_t(choice)]));
}

// Action 36: a random interaction sequence of the step's list. The game
// counts the list from its first entry, dividing by zero for an empty one;
// none is (see kGroupInteractions).
void process_group_interaction(const Run &r, uint16_t id, const ComplexStep &entry)
{
    const std::array<int8_t, 5> &list = kGroupInteractions[entry.anim];
    uint8_t n = 1;
    while (n < 5 && list[n] != -1)
        n++;
    start_interaction(r, id, uint8_t(list[old_newrand(r.world.seed, "process_staff_complex") % n]));
}

// Actions 38 and 40: the slave starts the complex the next byte names (as
// a step number), then PS_SLAVE_READY; with 38 it runs its first step now.
uint8_t process_slave_complex(const Run &r, uint16_t id, const ComplexStep &entry)
{
    World &world = r.world;
    Thing &e = world.things[id];
    if (!e.slave) {
        fail(r, id);
        return 0;
    }
    e.complex_pc++;
    Thing &patient = world.things[e.slave];
    patient.next_state = kPsSlaveReady;
    set_complex(world, e.slave, uint16_t(r.staff.at(e.complex_pc) * 10), 0);
    if (entry.action == 38)
        process_either(r, e.slave);
    return kStepDone;
}

// Actions 60 and 62: a surgeon changing at the surgeon screen. The animation
// (staff and screen) follows the number of surgeons the room counts: the
// first and second each have one, a third none.
void screen_change_anim(const Run &r, uint16_t id, uint8_t action)
{
    Thing &e = r.world.things[id];
    uint8_t count = count_surgeons(r.world, e.room);
    bool matched = action == 60 ? count == 1 : count == 2;
    uint8_t staff_column = action == 60 ? (matched ? 19 : 20) : (matched ? 22 : 21);
    uint16_t screen_anim = matched ? 0x56c : (action == 60 ? 0x56b : 0x56a);
    play_staff_anim(r.anims, e, staff_column);
    play_frame(r.world.things[e.object], r.anims, screen_anim);
}

// The step's actions that sequence the complex, animate or change the
// member of staff alone.
uint8_t process_sequence_action(const Run &r, uint16_t id, const ComplexStep &entry)
{
    World &world = r.world;
    Thing &e = world.things[id];
    switch (entry.action) {
    case 4:
        play_staff_anim(r.anims, e, entry.anim);
        break;
    case 16:
        process_group_anim(r, e, entry);
        break;
    case 14:
        if (uint16_t door = find_door(world, id))
            e.facing = kOppositeAngle[world.things[door].facing & 3];
        break;
    case 24:
        e.complex_pc++;
        e.complex_resume = uint16_t(r.staff.at(e.complex_pc) * 10);
        break;
    case 26:
        e.complex_pc = uint16_t(e.complex_queued - 1);
        e.complex_queued = 0;
        break;
    case 28:
        e.complex_pc++;
        start_interaction(r, id, uint8_t(r.staff.at(e.complex_pc) + 1));
        break;
    case 30:
        merge_slave(world, id);
        break;
    case 36:
        process_group_interaction(r, id, entry);
        break;
    case 38:
    case 40:
        return process_slave_complex(r, id, entry);
    case 46:
        head_off(r, id);
        break;
    case 52:
        staff_success(world, e, entry.anim);
        break;
    case 58:
        e.subtype = uint8_t(entry.anim);
        break;
    case 60:
    case 62:
        screen_change_anim(r, id, entry.action);
        break;
    default:
        // 0 and 12 do nothing, nor does 50 here (see process).
        break;
    }
    return kStepDone;
}

} // namespace

// Runs the action of step `entry`: kStepDone, or the value process returns.
uint8_t process_action(const Run &r, uint16_t id, const ComplexStep &entry, uint16_t obj_id, uint16_t tile)
{
    World &world = r.world;
    Thing &e = world.things[id];
    size_t side = use_side(e);
    switch (entry.action) {
    case 56:
        return process_go_near_object(r, id);
    case 2:
        e.object = obj_id;
        [[fallthrough]];
    case 44:
        set_destination_coords(world, id, world.things[e.object].use_pos[side]);
        head_off(r, id);
        e.countdown = 0;
        return kStepDone;
    case 54:
        if (e.slave)
            play_frame(e, r.anims, kPatientAnimStarts[world.things[e.slave].subtype][entry.anim]);
        return kStepDone;
    case 6:
        return process_use_object(r, id, tile, side);
    case 8:
        process_leave_object(r, id, tile, side);
        return kStepDone;
    case 10:
        if (e.object)
            play_frame(world.things[e.object], r.anims, entry.anim);
        return kStepDone;
    case 18:
        process_face_object(r, id);
        return kStepDone;
    case 20:
        return process_enter_door(r, id, entry, tile);
    case 22:
        process_leave_door(r, id);
        return kStepDone;
    case 48:
        process_move_slave_off(world, e);
        return kStepDone;
    default:
        return process_sequence_action(r, id, entry);
    }
}

} // namespace staff_complex_detail

} // namespace th
