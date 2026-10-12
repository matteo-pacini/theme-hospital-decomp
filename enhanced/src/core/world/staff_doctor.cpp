// A doctor at work in a GP's office, a machine room and a research room
// (src/doctor.cpp).

#include "core/world/staff.h"

#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/reception.h"
#include "core/world/rooms.h"
#include "core/world/staff_internal.h"
#include "core/world/vaccine.h"

namespace th
{

namespace
{

// The game's research_tasks (src/tables.cpp): what a doctor in a research
// room may use, one time in `chance` when it is free, and the complex they
// run on it: the research computer, the atom analyser, a desk and a filing
// cabinet.
struct ResearchTask {
    uint16_t chance;
    uint8_t object;
    uint16_t complex;
};
constexpr std::array<ResearchTask, 4> kResearchTasks = {{{8, 40, 670}, {8, 41, 680}, {12, 1, 1190}, {12, 2, 1200}}};
} // namespace

inline namespace staff_detail
{

// The member of staff holds their room (check_room_mutex), or is sent out.
bool holds_room(const Ctx &c, uint16_t id)
{
    if (check_room_mutex(c.world, id))
        return true;
    evacuate_staff(c, id);
    return false;
}

// doctor_in_office (0x3a6e0): to the room's free desk to sit at it
// (SS_DOCTOR_WORKS_AT_DESK), or out without one.
void doctor_in_office(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (!holds_room(c, id))
        return;
    if (find_object(world, cell_of(d), kObjDesk, true, true, d.owner)) {
        reset_icon_value(d, kIconCalling);
        d.next_state = kStaffDoctorWorksAtDesk;
        d.resume_state = kStaffDoctorInOffice;
        set_complex(world, id, kComplexSitAtDesk, 0);
        process(c, id);
    } else {
        evacuate_staff(c, id);
    }
}

} // namespace staff_detail

namespace
{

// doctor_works_at_desk's call: the next patient of `room`'s queue called
// (call_person_to_room), the doctor to wait for them, visiting the filing
// cabinet on the way if there is one.
void doctor_works_at_desk_call_patient(const Ctx &c, uint16_t id, const RoomInfo &room)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t queue = room.queue_head;
    if (!queue || !call_person_to_room(c.person_run(), id, queue, kComplexCalledToRoom, room.door_tile, true))
        return;
    set_icon_value(c.anims, d, kIconCalling);
    if (find_object(world, cell_of(d), kObjCabinet, true, true, d.owner)) {
        d.next_state = kStaffDoctorWaitsForPatient;
        set_complex(world, id, kComplexCabinetThenWait, 0);
        process(c, id);
    } else {
        d.state = kStaffDoctorWaitsForPatient;
    }
}

// The end of doctor_works_at_desk and doctor_in_machine_room: free in their
// room (staff_free), a member of staff takes a break (take_a_break) when
// more tired than the policy's rest_threshold or, with the policy letting
// staff leave rooms and no one queueing, answers a page every
// work_delay + 1 turns (2 * work_delay + 1 tired); gone, a member of staff
// with `skills` is paged in their place if patients queue.
void free_staff_break_or_page(const Ctx &c, uint16_t id, uint16_t skills)
{
    World &world = c.world;
    const Thing &d = world.things[id];
    const RoomInfo &room = world.rooms[d.room];
    if (!room.staff_free)
        return;
    const HospitalSettings &policy = policy_of(world);
    bool tired = d.tiredness > policy.rest_threshold;
    bool paged = false;
    if (tired && take_a_break(c, id)) {
        paged = true;
    } else if (policy.staff_leave_rooms && room.queue_head == 0 &&
               c.game_turn % (tired ? d.work_delay * 2 + 1 : d.work_delay + 1) == 0 && obey_pager(c, id)) {
        paged = true;
    }
    if (paged && room.queue_head)
        page_member_of_staff(world, skills, d.room, false, 1, d.owner);
}
} // namespace

inline namespace staff_detail
{

// doctor_works_at_desk (0x3a7a8): at each end of the desk's animation,
// out if the room is being edited; else, a quarter of the time, the next
// patient of the room's queue called (call_person_to_room, the filing
// cabinet visited on the way if there is one); 7% of the rest a visit to
// the cabinet; else, free (staff_free), a break (take_a_break) when more
// tired than the policy's rest_threshold or, with the policy letting staff
// leave rooms and no one queueing, a page answered every work_delay + 1
// turns (2 * work_delay + 1 tired), a doctor paged in their place if
// patients queue; first, they may resign or leave (shall_i_resign), and
// take an ordered break (kOrderRest). The orders to go back to research or
// training are dropped.
void doctor_works_at_desk(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (c.game_turn % d.anim_speed != 0)
        return;
    d.sprite = c.anims.frames[d.sprite].next;
    if (!(c.anims.frames[d.sprite].flags & 1))
        return;
    d.orders &= uint8_t(~(kOrderReturnToResearch | kOrderReturnToTraining));
    if (!holds_room(c, id) || shall_i_resign(c, id))
        return;
    RoomInfo &room = world.rooms[d.room];
    if (room.being_edited || room.state == kRoomEdited) {
        evacuate_staff(c, id);
        return;
    }
    if ((d.orders & kOrderRest) && take_a_break(c, id))
        return;
    if (old_newrand(world.seed) % 100 < 25) {
        doctor_works_at_desk_call_patient(c, id, room);
        return;
    }
    uint32_t pct = old_newrand(world.seed) % 100;
    if (pct >= 25 && pct < 32) {
        if (find_object(world, cell_of(d), kObjCabinet, true, true, d.owner)) {
            d.resume_state = kStaffDoctorInOffice;
            set_complex(world, id, kComplexCabinet, 0);
            process(c, id);
        }
        return;
    }
    free_staff_break_or_page(c, id, kSkillDoctor);
}

} // namespace staff_detail

namespace
{

// doctor_in_machine_room's stroll to a random cell of the room they stand
// in (1% of the time its middle) they can get to.
void doctor_in_machine_room_stroll(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t tile = cell_of(d);
    const RoomInfo &here = world.rooms[room_at(world, tile)];
    uint16_t pos;
    if (old_newrand(world.seed, "doctor_in_machine_room") % 100 == 25) {
        pos = uint16_t(((here.height / 2) << 7) + here.width / 2 + here.pos);
    } else {
        // The row is drawn first, as the game's code does.
        auto row = int16_t(old_newrand(world.seed, "doctor_in_machine_room") % uint32_t(here.height));
        auto column = int16_t(old_newrand(world.seed, "doctor_in_machine_room") % uint32_t(here.width));
        pos = uint16_t((row << 7) + column + here.pos);
    }
    if (tile != pos && can_i_get_to(world, d, pos)) {
        d.next_state = d.resume_state = kStaffDoctorInMachineRoom;
        set_complex(world, id, kComplexWalkTo, pos);
        process(c, id);
    }
}

// doctor_in_machine_room's call, when the room is open and a patient queues:
// in rooms other than an operating theatre the member of staff calls them
// itself and waits; in a theatre the lead surgeon (waiting in
// SS_SURGEON_WAITS for the other to arrive) is found and calls, this
// surgeon waiting as its assistant, or, being the lead itself, waits.
void doctor_in_machine_room_call(const Ctx &c, uint16_t id, const RoomInfo &room)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t queue = room.queue_head;
    if (room.state == kRoomClosed || !queue)
        return;
    if (room.type != kRoomTheatre) {
        if (call_person_to_room(c.person_run(), id, queue, kComplexCalledToRoom, room.door_tile, true)) {
            set_icon_value(c.anims, d, kIconCalling);
            play_staff_anim(c.anims, d, kAnimStand);
            d.state = kStaffDoctorWaitsForPatient;
        }
        return;
    }
    if (am_i_lead_staff(world, id)) {
        d.timer = 200;
        d.state = kStaffSurgeonWaits;
        return;
    }
    uint16_t lead = find_waiting_surgeon(world, d.room, kStaffSurgeonWaits);
    if (lead && call_person_to_room(c.person_run(), lead, queue, kComplexCalledToRoom, room.door_tile, true)) {
        d.state = kStaffAssistantReady;
        set_icon_value(c.anims, world.things[lead], kIconCalling);
        play_staff_anim(c.anims, world.things[lead], kAnimStand);
        world.things[lead].state = kStaffDoctorWaitsForPatient;
    }
}
} // namespace

inline namespace staff_detail
{

// doctor_in_machine_room (0x3b2c4): at each end of their animation, a
// member of staff in a room with machines (the diagnosis rooms) goes out if
// the room is being edited; else, a quarter of the time, the next patient
// of the room's queue is called (call_person_to_room) unless the room is
// closed; 2% of the rest, or always on the cell inside the door, they walk
// to a random cell of the room (1% of those its middle) they can get to;
// else, free (staff_free), a break (take_a_break) when more tired than the
// policy's rest_threshold or, with the policy letting staff leave rooms and
// no one queueing, a page answered every work_delay + 1 turns
// (2 * work_delay + 1 tired), another doctor paged if a queue is left.
// First, they may resign or leave (shall_i_resign), take an ordered break
// (kOrderRest), and a room due a repair is closed. A doctor in an
// operating theatre goes to SS_DOCTOR_IN_THEATRE; a nurse who could vaccinate leaves (could_nurse_vaccinate).
void doctor_in_machine_room(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (c.game_turn % d.anim_speed != 0)
        return;
    d.sprite = c.anims.frames[d.sprite].next;
    if (!(c.anims.frames[d.sprite].flags & 1))
        return;
    RoomInfo &room = world.rooms[d.room];
    if (room.type == kRoomTheatre && d.subtype == kStaffDoctor) {
        d.state = kStaffDoctorInTheatre;
        return;
    }
    d.orders &= uint8_t(~(kOrderReturnToResearch | kOrderReturnToTraining));
    if (!holds_room(c, id) || shall_i_resign(c, id))
        return;
    if (room.being_edited || room.state == kRoomEdited) {
        evacuate_staff(c, id);
        return;
    }
    if ((d.orders & kOrderRest) && take_a_break(c, id))
        return;
    if (d.subtype == kStaffNurse && world.idle_nurses == 0 && could_nurse_vaccinate(world, id)) {
        evacuate_staff(c, id);
        return;
    }
    if (room.state == kRoomRepairDue)
        room.state = kRoomClosed;
    if (old_newrand(world.seed) % 100 < 25) {
        doctor_in_machine_room_call(c, id, room);
        return;
    }
    uint32_t pct = old_newrand(world.seed) % 100;
    if ((pct >= 25 && pct < 27) || cell_of(d) == room.door_inside_tile) {
        doctor_in_machine_room_stroll(c, id);
        return;
    }
    free_staff_break_or_page(c, id, kRoomDoctorSkills[room.type % kRoomTypes]);
}

} // namespace staff_detail

namespace
{

// doctor_researches's call: with no patient in the room and no member of
// staff dealing with one, the next of its queue (patients sent for an
// autopsy) called (call_person_to_room), the doctor to wait for them.
// Returns whether they were called.
bool doctor_researches_call(const Ctx &c, uint16_t id, const RoomInfo &room)
{
    World &world = c.world;
    uint16_t queue = room.queue_head;
    if (!queue)
        return false;
    for (uint16_t t = room.first_person; t > 0; t = world.things[t].room_next) {
        const Thing &occupant = world.things[t];
        if ((occupant.type == kThingStaff && occupant.slave) || occupant.type == kThingPatient)
            return false;
    }
    if (!call_person_to_room(c.person_run(), id, queue, kComplexCalledToRoom, room.door_tile, true))
        return false;
    set_icon_value(c.anims, world.things[id], kIconCalling);
    world.things[id].state = kStaffDoctorWaitsForPatient;
    return true;
}

// doctor_researches's work: of the research tasks in order, the first
// whose object the room has free (find_object, drawing one of them) with
// no other member of staff on its use cell is taken up one time in its
// chance (a draw each). Returns whether one was.
bool doctor_researches_task(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    for (const ResearchTask &task : kResearchTasks) {
        uint16_t obj = find_object(world, cell_of(d), task.object, true, true, d.owner);
        if (!obj)
            continue;
        // search_mapwho: the first member of staff on the object's use cell.
        uint16_t user = 0;
        for (int16_t t = world.map[world.things[obj].use_pos[0]].first; t > 0 && !user;
             t = world.things[size_t(t)].cell_next)
            if (world.things[size_t(t)].type == kThingStaff)
                user = uint16_t(t);
        if (user != 0 && user != id)
            continue;
        if (old_newrand(world.seed, "doctor_researches") % task.chance == 0) {
            d.resume_state = kStaffDoctorResearches;
            d.next_state = kStaffDoctorResearches;
            set_complex(world, id, task.complex, 0);
            process(c, id);
            return true;
        }
    }
    return false;
}
} // namespace

inline namespace staff_detail
{

// doctor_researches (0x3b9a0): at each end of their animation, a doctor
// with no research skill leaves (evacuate_staff); one who may resign or
// leave (shall_i_resign) does; else they are to come back to research after
// a break (kOrderReturnToResearch), and leave if the room is being edited
// or they are ordered out (kOrderLeaveRoom, the low orders cleared), take a
// break when ordered or more tired than the policy's rest_threshold
// (take_a_break), and otherwise, a quarter of the time, call a patient for
// an autopsy (doctor_researches_call), else take up a research task
// (doctor_researches_task); failing those, they move off a crowded cell
// (overlapping_staff_check).
void doctor_researches(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (c.game_turn % d.anim_speed != 0)
        return;
    d.sprite = c.anims.frames[d.sprite].next;
    if (!(c.anims.frames[d.sprite].flags & 1))
        return;
    if (!(d.skills & kSkillResearcher)) {
        evacuate_staff(c, id);
        return;
    }
    if (shall_i_resign(c, id))
        return;
    d.orders |= kOrderReturnToResearch;
    const RoomInfo &room = world.rooms[d.room];
    if (room.being_edited || room.state == kRoomEdited) {
        evacuate_staff(c, id);
        return;
    }
    if (d.orders & kOrderLeaveRoom) {
        d.orders &= 0xf0;
        evacuate_staff(c, id);
        return;
    }
    if (((d.orders & kOrderRest) || d.tiredness > policy_of(world).rest_threshold) && take_a_break(c, id))
        return;
    if (old_newrand(world.seed) % 100 < 25) {
        if (doctor_researches_call(c, id, room))
            return;
    } else if (doctor_researches_task(c, id)) {
        return;
    }
    overlapping_staff_check(c, id, kStaffDoctorResearches);
}

} // namespace staff_detail

} // namespace th
