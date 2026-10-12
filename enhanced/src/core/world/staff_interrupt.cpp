// Interrupting staff, picking them up and sacking them (src/staff.cpp,
// src/complex.cpp, src/recept.cpp).

#include "core/world/staff.h"

#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/nav.h"
#include "core/world/object_tables.h"
#include "core/world/queue.h"
#include "core/world/reception.h"
#include "core/world/rooms.h"
#include "core/world/staff_internal.h"
#include "core/world/trace.h"

namespace th
{

namespace
{

// Getting off a door or an object (interrupt_complex).
constexpr uint16_t kComplexOffDoor = 0x4ce;
constexpr uint16_t kComplexOffObject = 0x1b8;

// Giving up the complex (abandon_complex), and a receptionist's at her desk.
constexpr uint16_t kComplexAbandon = 0xf0;
constexpr uint16_t kComplexAbandonDesk = 0x4ba;

// interrupt_complex (0x427d8) for a member of staff: their complex, the
// complexes queued and to resume and their next and resume states cleared,
// centred on their cell; one using a door or object gets off it.
void staff_interrupt_complex(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    s.complex_pc = s.complex_queued = s.complex_queued2 = s.complex_resume = 0;
    s.next_state = 0;
    s.resume_state = 0;
    s.x.frac = s.y.frac = 0x80;
    s.countdown = 0;
    if (!s.object)
        return;
    const Thing &obj = world.things[s.object];
    if ((obj.type != kThingObject && obj.type != kThingDoor) || obj.user != id)
        return;
    set_complex(world, id, obj.type == kThingDoor ? kComplexOffDoor : kComplexOffObject, 0);
    process(c, id);
}

// receptionist_reset (0x98554): the receptionist leaves her reception; if
// no other desk's receptionist works at it, its queue is sent on
// (restart_all_queue_peeps) and the reception's entry freed.
void receptionist_reset(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &r = world.things[id];
    if (!r.room || world.rooms[r.room].type != kRoomReception)
        return;
    bool shared = false;
    for (uint16_t o = r.owner == 0 ? world.first_of_kind[kObjReceptionDesk] : 0; o && !shared;
         o = world.things[o].type_obj_next) {
        const Thing &desk = world.things[o];
        shared = o != r.object && desk.in_use != 0 && desk.user && world.things[desk.user].room == r.room;
    }
    if (!shared) {
        restart_all_queue_peeps(world, c.anims, c.person, r.room, c.game_turn);
        world.rooms[r.room] = RoomInfo{};
    }
    r.room = 0;
}

// private_interrupt_staff (0x2c484): stops what the member of staff does,
// frees the patient they deal with in a room (interrupt_person) and puts
// them in the state for where they are, their pages dropped; a
// receptionist leaves her reception (receptionist_reset); a handyman drops
// his repair and plant pages, reopening the room of the machine he was to
// repair.
void private_interrupt_staff(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    uint16_t patient = s.slave;
    if (s.subtype == kStaffHandyman) {
        auto reopen = [](RoomInfo &room) {
            if (room.state == kRoomRepairDue || room.state == kRoomClosed)
                room.state = 0;
        };
        if (uint16_t target = thingpage_remind_me(world.repair_pager, id); target && world.things[target].room_id)
            reopen(world.rooms[world.things[target].room_id]);
        if (s.room && s.object && kObjectIsMachine[world.things[s.object].subtype % kObjectIsMachine.size()])
            reopen(world.rooms[s.room]);
        thingpage_cancel_my_response(world.repair_pager, id);
        thingpage_cancel_my_response(world.plant_pager, id);
    }
    staff_interrupt_complex(c, id);
    if (s.subtype == kStaffNurse && patient && s.room == 0) {
        world.things[patient].person_flags &= ~kPfVaccineNurseSent;
        s.slave = 0;
    } else if (patient && world.things[patient].type == kThingPatient && (s.room || s.subtype == kStaffReceptionist)) {
        s.slave = 0;
        interrupt_person(world, c.anims, c.person, patient, c.game_turn);
    }
    s.state = get_state(s.room ? world.rooms[s.room].type : 0, s);
    s.substate = 0;
    staff_abandon_pager_response(world, id);
    if (s.subtype == kStaffReceptionist)
        receptionist_reset(c, id);
}
} // namespace

inline namespace staff_detail
{

// interrupt_staff (0x2c3bc): private_interrupt_staff, and for a surgeon the
// other surgeon holding the room.
void interrupt_staff_in(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    private_interrupt_staff(c, id);
    const Thing &s = world.things[id];
    if (s.subtype != kStaffSurgeon || !s.room)
        return;
    for (uint16_t partner : world.rooms[s.room].staff) {
        if (partner && partner != id) {
            if (world.things[partner].owner != kNoOwner)
                private_interrupt_staff(c, partner);
            return;
        }
    }
}

} // namespace staff_detail

namespace
{

// abandon_complex (0x41f84) for a member of staff: runs kComplexAbandon
// (kComplexAbandonDesk for a receptionist with an object, who leaves her
// reception) for up to 12 steps, each from its destination cell, puts them
// back where they were unless they used their object or changed room,
// interrupts their slave (sent on unless in the same room) and stands them
// in resume_state.
void staff_abandon_complex(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    trace_event("abandon_complex %u", unsigned(id));
    MapPos old_x = s.x;
    MapPos old_y = s.y;
    bool user = s.object && world.things[s.object].user == id;
    if (s.object && s.subtype == kStaffReceptionist) {
        set_complex(world, id, kComplexAbandonDesk, 0);
        s.room = 0;
    } else {
        set_complex(world, id, kComplexAbandon, 0);
    }
    uint16_t room = s.room;
    for (int i = 0; i < 12; i++) {
        move_node(world, id, {0x80, s.dest_x}, {0x80, s.dest_y});
        if (process(c, id) != 1)
            break;
    }
    if (!user && room == s.room)
        move_node(world, id, old_x, old_y);
    if (uint16_t patient = s.slave) {
        interrupt_person(world, c.anims, c.person, patient, c.game_turn);
        if (s.room == 0 || s.room != world.things[patient].room)
            start_person_going(world, c.anims, c.person, patient, c.game_turn);
    }
    s.state = s.resume_state;
    play_staff_anim(c.anims, s, kAnimStand);
}

// clear_my_mutex (0x2bb58): the member of staff no longer holds their room.
void clear_my_mutex(World &world, uint16_t id)
{
    const Thing &s = world.things[id];
    if (!s.room)
        return;
    std::array<uint16_t, 2> &holders = world.rooms[s.room].staff;
    if (holders[0] == id)
        holders[0] = 0;
    else if (holders[1] == id)
        holders[1] = 0;
}
} // namespace

inline namespace staff_detail
{

// staff_picked_up (0x2becc): the member of staff leaves their room and what
// they were doing (abandon_complex) and is taken off the map (state
// kStaffHeld), World::held_staff_tile noting their cell. The interface's
// part (placing them) is hire_staff_after_turn's.
void staff_picked_up(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    trace_event("staff_picked_up %u", unsigned(id));
    staff_exit_room_check(world, s);
    staff_abandon_complex(c, id);
    world.held_staff_tile = cell_of(s);
    clear_my_mutex(world, id);
    // remove_staff_from_map.
    remove_person_from_room_list(world, id);
    remove_node(world, id);
    s.state = kStaffHeld;
}

// sack_staff (0x2a804): the member of staff stops holding their room and
// what they do (interrupt_staff) and, unless mode 2, walks out of their room
// and to their hospital's first entrance, to go back to the applicants
// (kStaffBackToApplicants); mode 0 pays them a month's wage, mode 1 (notice
// run out) clears it, mode 2 removes them at once. Their skills no longer
// count (remove_expertise_staff, and the counts by going to kNoOwner). Not for staff already no one's.
void sack(const Ctx &c, uint16_t id, uint8_t mode)
{
    World &world = c.world;
    Thing &s = world.things[id];
    trace_event("sack_staff %u mode=%u", unsigned(id), unsigned(mode));
    clear_my_mutex(world, id);
    set_icon_value(c.anims, s, kIconLeaving);
    if (s.owner == kNoOwner)
        return;
    interrupt_staff_in(c, id);
    if (mode == 0 && world.ledger && world.balance && world.date)
        change_balance(*world.ledger, *world.balance, world.date->day, world.date->month, {kTxSeverancePay, -s.salary});
    else if (mode == 1)
        s.orders &= uint8_t(~kOrderResign);
    if (mode != 2) {
        staff_exit_room_check(world, s);
        if (s.room)
            set_complex(world, id, kComplexLeaveRoom, world.rooms[s.room].door_inside_tile);
        set_complex(world, id, kComplexWalkTo,
                    world.rooms[get_room_info(world, kRoomEntrance, 0x65, s.owner)].door_tile);
        s.next_state = kStaffBackToApplicants;
        s.resume_state = 0;
        process(c, id);
    }
    remove_expertise_staff(world, id);
    s.work_delay = uint8_t(s.work_delay + s.work_delay / 4);
    s.happiness = 100;
    s.resign_count = 0;
    s.resign_time = 0;
    s.orders = 0;
    s.discontent = 0;
    clear_peep_icon(s);
    s.owner = kNoOwner;
    if (mode == 2) {
        remove_person_from_room_list(world, id);
        trace_event("remove_thing %u type=%u", unsigned(id), unsigned(s.type));
        remove_node(world, id);
        s = Thing{};
    }
}

} // namespace staff_detail

void interrupt_staff(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    interrupt_staff_in(ctx_of(world, anims, game_turn), id);
}

void staff_abandon_for_pick_up(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Ctx c = ctx_of(world, anims, game_turn);
    Thing &s = world.things[id];
    if (s.subtype == kStaffReceptionist) {
        set_complex(world, id, kComplexOffObject, 0);
        s.next_state = get_state(0, s);
        staff_abandon_complex(c, id);
        play_staff_anim(anims, s, kAnimWalk);
    } else {
        staff_abandon_complex(c, id);
        s.complex_action = 0;
    }
}

void sack_staff(World &world, const AnimData &anims, uint16_t id, uint8_t mode, uint32_t game_turn)
{
    sack(ctx_of(world, anims, game_turn), id, mode);
}

} // namespace th
