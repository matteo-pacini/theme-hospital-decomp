// A receptionist at the reception desk (src/recept.cpp).

#include "core/world/staff.h"

#include "core/world/complex_tables.h"
#include "core/world/icons.h"
#include "core/world/patient_tables.h"
#include "core/world/queue.h"
#include "core/world/reception.h"
#include "core/world/staff_internal.h"
#include "core/world/trace.h"

#include <cstdlib>

namespace th
{

namespace
{

// The complex the patient called to the desk runs (P_COMP.LEX).
constexpr uint16_t kComplexToDesk = 0x6e;

// The complexes of a receptionist's idle tasks at the desk, after their
// count (the game's recept_tasks, src/tables.cpp).
constexpr std::array<uint16_t, 6> kReceptTasks = {5, 90, 100, 110, 120, 130};

// allocate_reception_roominfo (0x486a8): takes a free entry of
// World::rooms as a reception of `owner`. Returns it, 0 if there is none.
uint16_t allocate_reception_roominfo(World &world, uint8_t owner)
{
    for (uint16_t i = 1; i < world.rooms.size(); i++) {
        RoomInfo &room = world.rooms[i];
        if (room.type == kRoomFree) {
            room.type = kRoomReception;
            room.pos = 0x3fff;
            room.width = room.height = 1;
            room.state = 0;
            room.owner = owner;
            room.queue_capacity = 0x14;
            trace_event("reception %u", unsigned(i));
            return i;
        }
    }
    return 0;
}
} // namespace

inline namespace staff_detail
{

// receptionist_finds_reception (0x97ce0).
void receptionist_finds_reception(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &r = world.things[id];
    uint16_t desk = has_reception(world, r.owner) ? find_free_object_anywhere(world, id, kObjReceptionDesk) : 0;
    if (!desk) {
        reset_icon_value(r, kIconCalling);
        staff_in_corridor(r, c.anims);
        return;
    }
    reset_icon_value(r, kIconCalling);
    r.next_state = kStaffReceptionistSetup;
    r.object = desk;
    set_complex(world, id, kComplexGoToDesk, world.things[desk].use_pos[0]);
    r.resume_state = kStaffReceptionistFindsReception;
    process(c, id);
    r.slave = 0;
}

// receptionist_setup (0x97de0).
void receptionist_setup(World &world, uint16_t id)
{
    Thing &r = world.things[id];
    uint16_t room = 0;
    for (uint16_t o = r.owner == 0 ? world.first_of_kind[kObjReceptionDesk] : 0; o; o = world.things[o].type_obj_next) {
        const Thing &desk = world.things[o];
        if (o == r.object || desk.in_use == 0 || std::abs(desk.x.tile - r.x.tile) > 10 ||
            std::abs(desk.y.tile - r.y.tile) > 10)
            continue;
        if (desk.user) {
            room = world.things[desk.user].room;
            break;
        }
    }
    r.room = room ? room : allocate_reception_roominfo(world, r.owner);
    r.state = kStaffReceptionistWorks;
}

// receptionist_works (0x97fac). othervar.rand_pct, which it overwrites, is
// not kept.
void receptionist_works(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &r = world.things[id];
    if (r.room == 0 || c.game_turn % r.anim_speed != 0)
        return;
    r.sprite = c.anims.frames[r.sprite].next;
    if (!(c.anims.frames[r.sprite].flags & 1))
        return;
    uint16_t head = reception_queue_head(world, r.room);
    if (head) {
        uint16_t p = head;
        while (p && !reception_emergency(world, p))
            p = reception_queue_next(world, p);
        if (!p)
            p = head;
        for (; p; p = reception_queue_next(world, p)) {
            // The game keeps the capacity in a u16.
            if (uint16_t(all_queue_capacity(world, world.things[p].treatment, true, r.owner)) == 0 || !r.object)
                continue;
            uint16_t pos = world.things[r.object].use_pos[1];
            if (call_person_to_room(c.person_run(), id, p, kComplexToDesk, pos, false)) {
                set_icon_value(c.anims, r, kIconCalling);
                r.next_state = kStaffReceptWaitsForPatient;
                set_complex(world, id, kComplexCallPatient, 0);
                process(c, id);
                break;
            }
        }
        return;
    }
    if (old_newrand(world.seed) % 100 < 30) {
        r.next_state = kStaffReceptionistWorks;
        uint16_t action = kReceptTasks[old_newrand(world.seed) % kReceptTasks[0] + 1];
        set_complex(world, id, action, 0);
        process(c, id);
    } else {
        play_staff_anim(c.anims, r, 2);
        r.state = kStaffReceptionistWorks;
    }
}

// recept_waits_for_patient (0x982a8). othervar.recept_call_timer, which it
// clears, is not kept.
void recept_waits_for_patient(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &r = world.things[id];
    if (!r.slave) {
        reset_icon_value(r, kIconCalling);
        r.state = kStaffReceptionistWorks;
        return;
    }
    Thing &patient = world.things[r.slave];
    if (patient.state == kPsArrivedFromCall) {
        set_complex(world, id,
                    reception_cure_available(world, r.slave) ? kComplexPatientCured : kComplexPatientNotCured, 0);
        patient.facing = kOppositeAngle[r.facing & 3];
        r.next_state = kStaffReceptAdmitsPatient;
        process(c, id);
    } else if (--r.timer <= 0) {
        if (!r.object)
            return;
        uint16_t pos = world.things[r.object].use_pos[1];
        uint8_t result = call_person_again(c.person_run(), id, kComplexToDesk, pos);
        if (result == 0) {
            reset_icon_value(r, kIconCalling);
            r.state = kStaffReceptionistWorks;
            r.slave = 0;
        } else if (result == 1) {
            reset_icon_value(r, kIconCalling);
            r.state = kStaffReceptionistWorks;
            interrupt_person(world, c.anims, c.person, r.slave, c.game_turn);
            start_person_going(world, c.anims, c.person, r.slave, c.game_turn);
            r.slave = 0;
        }
    } else if (forget_about_my_slave(world, c.anims, c.person, id, kComplexToDesk, c.game_turn)) {
        reset_icon_value(r, kIconCalling);
        r.state = kStaffReceptionistWorks;
        r.slave = 0;
    }
}

// recept_admits_patient (0x984a0).
void recept_admits_patient(World &world, const AnimData &anims, uint16_t id)
{
    Thing &r = world.things[id];
    if (!r.slave)
        return;
    reception_serves_patient(world, anims, id, r.slave);
    RoomInfo &room = world.rooms[r.room];
    room.state = 0;
    room.patients_seen++;
    reset_icon_value(r, kIconCalling);
    r.slave = 0;
    r.state = kStaffReceptionistWorks;
}

} // namespace staff_detail

} // namespace th
