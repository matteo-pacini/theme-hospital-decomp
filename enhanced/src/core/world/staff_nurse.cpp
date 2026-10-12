// A nurse in a ward (src/nurse.cpp).

#include "core/world/staff.h"

#include "core/world/game.h"
#include "core/world/reception.h"
#include "core/world/rooms.h"
#include "core/world/staff_internal.h"
#include "core/world/vaccine.h"

namespace th
{

namespace
{

// A nurse's in a ward: to the bed for the patient called, to the desk, to a
// patient in bed; and the patient's into bed (P_COMP.LEX).
constexpr uint16_t kComplexNurseToBed = 0x15e;
constexpr uint16_t kComplexNurseToDesk = 0x168;
constexpr uint16_t kComplexNurseVisit = 0x17c;
constexpr uint16_t kComplexIntoBed = 200;

// find_peep (0x1720c): a person of hospital `owner` in room `room` of thing
// type `type` in state `state` (0 for any): the first in its list of
// people, or with `random` one drawn from world.seed; 0 if none.
uint16_t find_peep(World &world, uint16_t room, bool random, uint8_t type, uint8_t state, uint8_t owner)
{
    auto matches = [&](const Thing &t) {
        return t.owner == owner && (!type || t.type == type) && (!state || t.state == state);
    };
    uint16_t count = 1;
    if (random) {
        count = 0;
        for (uint16_t p = world.rooms[room].first_person; p; p = world.things[p].room_next)
            count = uint16_t(count + matches(world.things[p]));
        if (count == 0)
            return 0;
        count = uint16_t(old_newrand(world.seed, "find_peep") % count + 1);
    }
    for (uint16_t p = world.rooms[room].first_person; p; p = world.things[p].room_next)
        if (matches(world.things[p]) && --count == 0)
            return p;
    return 0;
}

// find_unallocated_bed (0x6c79c): a bed in ward `room` no one uses nor is
// given (a patient in the room whose object it is); 0 if none.
uint16_t find_unallocated_bed(const World &world, uint16_t room)
{
    for (uint16_t bed = world.rooms[room].first_object; bed; bed = world.things[bed].room_obj_next) {
        if (world.things[bed].subtype != kObjBed || world.things[bed].user != 0)
            continue;
        uint16_t p = world.rooms[room].first_person;
        while (p && !(world.things[p].type == kThingPatient && world.things[p].object == bed))
            p = world.things[p].room_next;
        if (!p)
            return bed;
    }
    return 0;
}
} // namespace

inline namespace staff_detail
{

// reinit_nurse (0x6c640): the nurse in the list of the room she stands in
// (not an entrance) and in its state, or in the corridor's.
void reinit_nurse(World &world, uint16_t id)
{
    Thing &nurse = world.things[id];
    remove_person_from_room_list(world, id);
    if (uint16_t room = room_at(world, cell_of(nurse))) {
        if (world.rooms[room].type != kRoomEntrance)
            insert_person_in_room_list(world, room, id);
        nurse.state = get_state(world.rooms[room].type, nurse);
    } else {
        nurse.state = get_state(0, nurse);
    }
    nurse.substate = 0;
}

} // namespace staff_detail

namespace
{

// nurse_in_ward's call: the next patient of ward `room`'s queue called to
// a free bed (find_unallocated_bed), the nurse to it (substate 1). Returns
// whether she called one.
bool nurse_in_ward_call_patient(const Ctx &c, uint16_t id, const RoomInfo &room)
{
    World &world = c.world;
    Thing &nurse = world.things[id];
    uint16_t queue = room.queue_head;
    if (!queue)
        return false;
    uint16_t bed = find_unallocated_bed(world, nurse.room);
    if (!bed || !call_person_to_room(c.person_run(), id, queue, kComplexCalledToRoom, room.door_tile, false))
        return false;
    nurse.object = bed;
    nurse.next_state = kStaffNurseInWard;
    nurse.substate = 1;
    set_complex(world, id, kComplexNurseToBed, 0);
    process(c, id);
    return true;
}

// nurse_in_ward's stroll to a random cell of the room she stands in (5% its
// middle) she can get to; staying put, now and then she forgets the
// patient she visited last.
void nurse_in_ward_stroll(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &nurse = world.things[id];
    uint16_t cur = cell_of(nurse);
    const RoomInfo &here = world.rooms[room_at(world, cur)];
    uint32_t pct = old_newrand(world.seed, "nurse_in_ward") % 100;
    uint16_t target;
    if (pct >= 15 && pct < 20) {
        target = uint16_t(here.pos + (((here.height / 2) << 7) + here.width / 2));
    } else {
        // The row is drawn first, as the game's code does.
        auto row = int16_t(old_newrand(world.seed, "nurse_in_ward") % uint32_t(here.height));
        auto column = int16_t(old_newrand(world.seed, "nurse_in_ward") % uint32_t(here.width));
        target = uint16_t(here.pos + ((row << 7) + column));
    }
    if (cur != target && can_i_get_to(world, nurse, target)) {
        nurse.next_state = nurse.resume_state = kStaffNurseInWard;
        set_complex(world, id, kComplexWalkTo, target);
        process(c, id);
        return;
    }
    pct = old_newrand(world.seed, "nurse_in_ward") % 100;
    if (pct >= 0x46 && pct < 0x50)
        nurse.work_target = 0;
}

// nurse_in_ward's substate 0 in ward `room` with desk `desk` (0 if none):
// the decision.
void nurse_in_ward_decide(const Ctx &c, uint16_t id, RoomInfo &room, uint16_t desk)
{
    World &world = c.world;
    Thing &nurse = world.things[id];
    if (room.state != kRoomEdited)
        room.state = 1;
    if (!holds_room(c, id) || shall_i_resign(c, id))
        return;
    if ((nurse.orders & kOrderRest) && take_a_break(c, id))
        return;
    const HospitalSettings &policy = policy_of(world);
    bool tired = nurse.tiredness > policy.rest_threshold;
    if (policy.staff_leave_rooms && c.game_turn % (tired ? nurse.work_delay * 2 + 1 : nurse.work_delay + 1) == 0 &&
        obey_pager(c, id))
        return;
    if (tired && take_a_break(c, id))
        return;
    if (room.being_edited || room.state == kRoomEdited) {
        evacuate_staff(c, id);
        return;
    }
    if (world.idle_nurses == 0 && could_nurse_vaccinate(world, id)) {
        evacuate_staff(c, id);
        return;
    }
    if (nurse_in_ward_call_patient(c, id, room))
        return;
    if (desk && nurse.work_target != desk && old_newrand(world.seed, "nurse_in_ward") % 100 < 10) {
        nurse.work_target = desk;
        set_complex(world, id, kComplexNurseToDesk, 0);
        nurse.next_state = kStaffNurseInWard;
        process(c, id);
        return;
    }
    uint16_t patient = find_peep(world, nurse.room, true, kThingPatient, kPsInBed, nurse.owner);
    if (patient && patient != nurse.work_target) {
        nurse.work_target = patient;
        nurse.object = world.things[patient].object;
        nurse.next_state = kStaffNurseInWard;
        set_complex(world, id, kComplexNurseVisit, 0);
        process(c, id);
        return;
    }
    nurse_in_ward_stroll(c, id);
}
} // namespace

inline namespace staff_detail
{

// nurse_in_ward (0x6c8d8). Substate 0, at each turn: the ward in use (unless
// edited); out if she cannot hold it; she may resign, take an ordered
// break (kOrderRest, take_a_break), answer a page every work_delay + 1
// turns (2 * work_delay + 1 tired) as the policy lets staff leave rooms, or
// take a break when more tired than the policy's rest_threshold; out if the
// ward is being edited; else the queue's next patient called
// to a free bed (find_unallocated_bed); else, 10% of the time, to the
// desk; else to a patient in bed she did not visit last (find_peep, drawn
// from world.seed); else to a random cell of the ward (5% its middle).
// Substate 1: the patient called arrived, she examines them
// (affect_diagnosis) and they go to bed (PS_IN_BED); until then the call is
// repeated every 200 turns (call_person_again) and given up on a patient
// who no longer comes.
void nurse_in_ward(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &nurse = world.things[id];
    uint16_t desk = find_object(world, cell_of(nurse), kObjDesk, false, false, nurse.owner);
    RoomInfo &room = world.rooms[nurse.room];
    if (nurse.substate == 0) {
        nurse_in_ward_decide(c, id, room, desk);
        return;
    }
    if (nurse.substate != 1)
        return;
    uint16_t pid = nurse.slave;
    Thing &patient = world.things[pid];
    if (patient.state == kPsArrivedFromCall) {
        affect_diagnosis(world, c.anims, id, pid, c.game_turn);
        nurse.substate = 0;
        nurse.slave = 0;
        patient.object = nurse.object;
        nurse.object = 0;
        set_complex(world, pid, kComplexIntoBed, 0);
        patient.next_state = kPsInBed;
        process_person_complex(world, c.anims, c.person, pid, c.game_turn);
    } else if (--nurse.timer <= 0) {
        uint8_t result = call_person_again(c.person_run(), id, kComplexCalledToRoom, room.door_tile);
        if (result == 0) {
            nurse.substate = 0;
            nurse.slave = 0;
        } else if (result == 1) {
            nurse.substate = 0;
            nurse.slave = 0;
            interrupt_person(world, c.anims, c.person, pid, c.game_turn);
            start_person_going(world, c.anims, c.person, pid, c.game_turn);
        }
    } else if (forget_about_my_slave(world, c.anims, c.person, id, kComplexCalledToRoom, c.game_turn)) {
        nurse.substate = 0;
        nurse.slave = 0;
    }
}

} // namespace staff_detail

} // namespace th
