// The operating theatre's surgeons: changing at the screen, leading and
// assisting, and the surgery itself (src/doctor.cpp, src/staff.cpp).

#include "core/world/staff.h"

#include "core/world/rooms.h"
#include "core/world/staff_internal.h"
#include "core/world/trace.h"

namespace th
{

namespace
{

// An operating theatre: changing at the surgeon screen as the first or
// second surgeon and back again (doctor_in_theatre, change_surgeon_to_doc);
// the lead surgeon's and the assistant's surgery complexes, the two the
// assistant runs before and after it, the lead's at the table, and the
// patient's surgery (P_COMP.LEX).
constexpr uint16_t kComplexChangeFirstSurgeon = 0x4d8;
constexpr uint16_t kComplexChangeSecondSurgeon = 0x4ec;
constexpr uint16_t kComplexChangeBackFirst = 0x3ca;
constexpr uint16_t kComplexChangeBackSecond = 0x3d4;
constexpr uint16_t kComplexSurgeryLead = 0x316;
constexpr uint16_t kComplexSurgeryAssistant = 0x32a;
constexpr uint16_t kComplexAssistantFirst = 0x33e;
constexpr uint16_t kComplexAssistantDone = 0x352;
constexpr uint16_t kComplexSurgeryHold = 0x334;
constexpr uint16_t kComplexSurgeryAfterAssistant = 0x348;
constexpr uint16_t kComplexPatientSurgery = 0x276;
} // namespace

inline namespace staff_detail
{

// surgeon_screen_free (0x2d11c): false for a surgeon whose room's surgeon
// screen is in use.
bool surgeon_screen_free(World &world, uint16_t id)
{
    const Thing &s = world.things[id];
    if (s.subtype != kStaffSurgeon)
        return true;
    uint16_t screen = find_object(world, cell_of(s), kObjSurgeonScreen, false, false, s.owner);
    return !(screen && world.things[screen].in_use != 0);
}

// change_lockholder (0x2c9cc): the member of staff holds `room` in place of
// its first holder, who is interrupted and sent out (but in an operating
// theatre with a free place). Nothing if no one holds it.
void change_lockholder(const Ctx &c, uint16_t id, uint16_t room_id)
{
    RoomInfo &room = c.world.rooms[room_id];
    uint16_t *slot = room.staff[0] ? &room.staff[0] : room.staff[1] ? &room.staff[1] : nullptr;
    if (!slot)
        return;
    if (room.type != kRoomTheatre || (room.staff[0] && room.staff[1])) {
        uint16_t old = *slot;
        interrupt_staff_in(c, old);
        evacuate_staff(c, old);
    }
    *slot = id;
}

// am_i_lead_staff (0x2b528): whether member of staff `id` holds their room's
// first lock (in an operating theatre, only once both surgeons are there).
bool am_i_lead_staff(const World &world, uint16_t id)
{
    const Thing &s = world.things[id];
    if (!s.room)
        return false;
    const RoomInfo &room = world.rooms[s.room];
    if (room.type == kRoomTheatre && (!room.staff[0] || !room.staff[1]))
        return false;
    return room.staff[0] == id;
}

// find_waiting_surgeon (0x3c154): the first member of staff in room
// `room_id` whose state is `state`, or 0. Despite its name, not limited to
// surgeons: SS_SURGEON_WAITS or SS_ASSISTANT_READY are passed.
uint16_t find_waiting_surgeon(const World &world, uint16_t room_id, uint8_t state)
{
    const RoomInfo &room = world.rooms[room_id];
    for (uint16_t t = room.first_person; t > 0; t = world.things[t].room_next)
        if (world.things[t].type == kThingStaff && world.things[t].state == state)
            return t;
    return 0;
}

// change_surgeon_to_doc (0x3c0a8): starts the surgeon changing back from
// surgical dress (the complex for the first or second surgeon), or turns
// them back into kStaffDoctor at once if the room counts neither one nor two
// surgeons.
void change_surgeon_to_doc(World &world, uint16_t id)
{
    Thing &s = world.things[id];
    uint8_t count = count_surgeons(world, s.room);
    if (count == 1) {
        s.resume_state = kStaffEvacuate;
        set_complex(world, id, kComplexChangeBackFirst, 0);
    } else if (count == 2) {
        s.resume_state = kStaffEvacuate;
        set_complex(world, id, kComplexChangeBackSecond, 0);
    } else {
        s.subtype = kStaffDoctor;
    }
}

// doctor_in_theatre (0x3bed0): sends a qualified surgeon to change at the
// surgeon screen (as the first or second surgeon), or to wander if there is
// no screen; others, or a third surgeon, leave.
void doctor_in_theatre(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    const RoomInfo &room = world.rooms[d.room];
    uint8_t count = count_surgeons(world, d.room);
    d.object = 0;
    if (!(d.skills & kSkillSurgeon)) {
        evacuate_staff(c, id);
        return;
    }
    if (count == 0 || count == 1) {
        if (find_object(world, cell_of(d), kObjSurgeonScreen, false, true, d.owner)) {
            set_complex(world, id, count == 0 ? kComplexChangeFirstSurgeon : kComplexChangeSecondSurgeon, 0);
            d.resume_state = kStaffDoctorInTheatre;
            d.next_state = kStaffDoctorInMachineRoom;
            process(c, id);
            return;
        }
    } else {
        evacuate_staff(c, id);
        return;
    }
    // The row is drawn first, as the game's compiled code does.
    auto row = int16_t(old_newrand(world.seed, "doctor_in_theatre") % uint32_t(room.height));
    auto column = int16_t(old_newrand(world.seed, "doctor_in_theatre") % uint32_t(room.width));
    uint16_t pos = uint16_t((row << 7) + column + room.pos);
    if (can_i_get_to(world, d, pos)) {
        d.next_state = kStaffDoctorInTheatre;
        set_complex(world, id, kComplexWalkTo, pos);
        process(c, id);
    }
}

// surgeon_waits (0x3c11c): the lead surgeon counts down, then returns to
// SS_DOCTOR_IN_MACHINE_ROOM.
void surgeon_waits(const Ctx &c, uint16_t id)
{
    Thing &s = c.world.things[id];
    s.timer--;
    if (s.timer <= 0)
        s.state = kStaffDoctorInMachineRoom;
}

// stop_waiting_for_patient (0x3b234): the call dropped, back to the room's
// working state, releasing a surgeon's assistant waiting with them.
void stop_waiting_for_patient(World &world, Thing &s)
{
    trace_event("stop_waiting");
    s.state = get_state(world.rooms[s.room].type, s);
    s.slave = 0;
    if (s.subtype == kStaffSurgeon) {
        uint16_t partner = find_waiting_surgeon(world, s.room, kStaffAssistantReady);
        if (partner) {
            Thing &p = world.things[partner];
            p.state = get_state(world.rooms[p.room].type, p);
            p.slave = 0;
        }
    }
}

// doctor_waits_for_patient's operating theatre: the lead surgeon (who called)
// starts the surgery in SS_DOCTOR_EXAMINES substate 70 and the assistant (the
// room's other holder) waits in SS_ASSISTANT_READY.
void doctor_waits_for_patient_theatre(const Ctx &c, uint16_t id, const RoomInfo &room)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t assistant_id = room.staff[1];
    d.work_target = assistant_id;
    d.next_state = kStaffDoctorExamines;
    d.substate = 0x46;
    set_complex(world, id, kComplexSurgeryLead, 0);
    process(c, id);
    world.things[assistant_id].next_state = kStaffAssistantReady;
    set_complex(world, assistant_id, kComplexSurgeryAssistant, 0);
    process(c, assistant_id);
}

// doctor_examines's theatre substates 70-72: the lead surgeon waits (70),
// brings the assistant in (71) and, the assistant ready, sends it off while
// the surgery finishes (72).
void doctor_examines_theatre(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (d.substate == 70) {
        d.next_state = kStaffDoctorExamines;
        set_complex(world, id, kComplexSurgeryHold, 0);
        process(c, id);
        d.substate = 71;
        return;
    }
    Thing &assistant = world.things[d.work_target];
    if (assistant.state != kStaffAssistantReady)
        return;
    if (d.substate == 71) {
        assistant.next_state = kStaffAssistantReady;
        set_complex(world, d.work_target, kComplexAssistantFirst, 0);
        process(c, d.work_target);
        d.substate = 72;
        return;
    }
    assistant.next_state = kStaffDoctorInMachineRoom;
    set_complex(world, d.work_target, kComplexAssistantDone, 0);
    process(c, d.work_target);
    d.next_state = kStaffDoctorExamines;
    set_complex(world, id, kComplexSurgeryAfterAssistant, 0);
    process(c, id);
    d.substate = 73;
    d.work_target = 0;
}

// doctor_examines's theatre substate 73: the surgery over, unless the table
// is still in use (treatment_machine_used) the patient's surgery runs,
// the treatment is done and the lead surgeon free.
void doctor_examines_theatre_done(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t pid = d.slave;
    const Thing &patient = world.things[pid];
    if (treatment_machine_used(world, c.anims, patient.treatment, cell_of(patient), c.game_turn))
        return;
    set_complex(world, pid, kComplexPatientSurgery, 0);
    process_person_complex(world, c.anims, c.person, pid, c.game_turn);
    done_treatment(world, c.anims, id, pid, c.game_turn);
    d.slave = 0;
    d.state = kStaffDoctorInMachineRoom;
    world.rooms[d.room].staff_free = 1;
}

} // namespace staff_detail

// count_surgeons (0x3be38): the members of staff in room `room_id` dressed
// as surgeons (kStaffSurgeon).
uint8_t count_surgeons(const World &world, uint16_t room_id)
{
    uint8_t count = 0;
    const RoomInfo &room = world.rooms[room_id];
    for (uint16_t t = room.first_person; t > 0; t = world.things[t].room_next)
        if (world.things[t].type == kThingStaff && world.things[t].subtype == kStaffSurgeon)
            count++;
    return count;
}

} // namespace th
