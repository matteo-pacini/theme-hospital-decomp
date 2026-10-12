// A patient's turn: the states that fill and energise at a machine, bed and
// call, and the dispatch to the state's function (src/patient.cpp).

#include "core/world/complex_internal.h"

#include "core/world/counts.h"
#include "core/world/icons.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/reaper.h"
#include "core/world/reception.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

#include <cstdlib>

namespace th
{

namespace
{

// The patient kind of an alien (PK_ALIEN).
constexpr uint8_t kPatientAlien = 0xe;

// The complex that ends what a patient does where it stands (getting out
// of bed).
constexpr uint16_t kComplexStandStill = 240;

// peep_stuck (0x788d8) for a patient: stands for 100 turns, then, until a
// path to its saved destination opens (never once fed up), grows 15 more
// impatient each try, going home fed up past gbv.leave_max, and tries a
// random direction.
void stuck(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (r.game_turn % p.anim_speed == 0)
        p.sprite = r.anims.frames[p.sprite].next;
    if (--p.countdown <= 0) {
        if (p.facing != 4) {
            p.countdown = 100;
            stand(r, p);
            p.facing = 4;
            return;
        }
        uint16_t dest = uint16_t(p.saved_dest_x + (p.saved_dest_y << 7));
        if (!(p.person_flags & kPfFedUp) && path_exists(world, p.x.tile, p.y.tile, dest)) {
            p.state = p.saved_state;
            p.dest_x = p.saved_dest_x;
            p.dest_y = p.saved_dest_y;
            return;
        }
        p.impatience = uint16_t(p.impatience + 0xf);
        if (p.impatience > settings_of(world).gbv.leave_max) {
            p.impatience = 0;
            if (!(p.person_flags & kPfFedUp) && p.owner == 0)
                world.patients_left_count++;
            set_icon_value(r.anims, p, kIconFedUp);
            go_home(r, id);
            p.person_flags |= kPfFedUp;
            return;
        }
        auto dir = uint8_t(old_newrand(world.seed, "peep_stuck") & 3);
        if (!wall_in_way(world, p.x.tile, p.y.tile, dir)) {
            p.facing = dir;
            p.countdown = int16_t(0x100 / p.speed);
            play_patient_anim(r.anims, p, kAnimWalk);
        }
    }
    step_on(world, id);
}

// person_fills (0x17620), PS_FILLS: the patient fills its machine (the
// machine's aux_sprite drawn split at split_height, which grows a pixel a
// turn, two for an alien), both animating every anim_speed turns; once past
// the frame's height it is ready (PS_SLAVE_READY) in the machine's frame,
// and an alien's doctor in the room takes his turn at once.
void person_fills(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (!p.object)
        return;
    Thing &machine = world.things[p.object];
    if (r.game_turn % p.anim_speed == 0) {
        p.sprite = r.anims.frames[p.sprite].next;
        machine.aux_sprite = r.anims.frames[machine.aux_sprite].next;
    }
    const AnimFrame *cur;
    if (p.subtype == kPatientAlien) {
        machine.split_height = uint8_t(machine.split_height + 2);
        cur = &r.anims.frames[machine.aux_sprite];
    } else {
        machine.split_height++;
        cur = &r.anims.frames[p.sprite];
    }
    if (machine.split_height <= cur->height)
        return;
    p.state = kPsSlaveReady;
    p.sprite = machine.aux_sprite;
    p.flip = machine.aux_flip;
    if (p.subtype != kPatientAlien || !p.room || !world.staff_complexes)
        return;
    // find_person_in_room (0x1b150).
    for (uint16_t o = world.rooms[p.room].first_person; o; o = world.things[o].room_next) {
        if (world.things[o].type == kThingStaff && world.things[o].subtype == kStaffDoctor) {
            process_things_alone_begin(world, o, r.game_turn);
            staff_turn(world, r.anims, r.complexes, *world.staff_complexes, o, r.game_turn);
            final_count(world, r.game_turn);
            break;
        }
    }
}

// person_energises (0x17780), PS_ENERGISES: the patient's and its machine's
// animations play to their last frame and hold there.
void person_energises(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (!p.object || r.game_turn % p.anim_speed != 0)
        return;
    uint16_t next = r.anims.frames[p.sprite].next;
    if (r.anims.frames[next].flags & 1)
        return;
    p.sprite = next;
    Thing &machine = world.things[p.object];
    machine.aux_sprite = r.anims.frames[machine.aux_sprite].next;
}

// person_waits_for_busy (0x15c50).
void waits_for_busy(const Run &r, uint16_t id)
{
    Thing &p = r.world.things[id];
    trace_event("waits_for_busy %u", unsigned(id));
    if (r.game_turn % p.anim_speed == 0)
        p.sprite = r.anims.frames[p.sprite].next;
    if (p.timer-- <= 0)
        go_home(r, id);
}

// play_complex_person_anim (0x418dc).
void play_complex_anim(const Run &r, uint16_t id)
{
    Thing &p = r.world.things[id];
    if (r.game_turn % p.anim_speed != 0)
        return;
    uint16_t next = r.anims.frames[p.sprite].next;
    if (r.anims.frames[next].flags & 1)
        process(r, id);
    else
        p.sprite = next;
}

// person_arrives_at_door (0x15cc0): at the door of a room with a single
// door it turns away from it to queue (complex 50), else it queues at once.
void arrives_at_door(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    trace_event("at_door %u", unsigned(id));
    uint16_t room = is_this_a_door(world, cell_of(p));

    if (room && kRoomTypeInfo[world.rooms[room].type % kRoomTypes].door_kind != 0) {
        set_complex(world, id, kComplexAtDoor, 0);
        process(r, id);
    } else {
        p.state = kPsInQueue;
    }
}

// person_complex_retry (0x41a54).
void complex_retry(const Run &r, uint16_t id)
{
    Thing &p = r.world.things[id];
    if (r.game_turn % p.anim_speed == 0) {
        p.sprite = r.anims.frames[p.sprite].next;
        if (r.anims.frames[p.sprite].flags & 1)
            process(r, id);
    }
}

// person_in_bed (0x17194): in bed until the timer runs out (its needs
// looked at every 30 turns), then up and on to its next treatment.
void in_bed(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (p.timer == 0)
        return;
    if (r.game_turn % 30 == 0)
        modify_person(world, r.anims, settings_of(world), heating_level_of(world), id, r.game_turn);
    if (--p.timer == 0) {
        set_complex(world, id, kComplexStandStill, 0);
        process(r, id);
        setup_next_treatment(world, r.anims, 0, id, r.game_turn);
    }
}

// person_arrived_from_call (0x18f88): in a room, it waits for the member of
// staff who called it, and with none there leaves for its treatment; at a
// reception desk the receptionist takes it from here. Returns whether it
// waits.
bool arrived_from_call(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (!p.room)
        return false;
    for (const Thing &t : world.things)
        if (t.type == kThingStaff && t.room == p.room && t.slave == id)
            return true;
    set_complex(world, id, kComplexLeaveRoom, world.rooms[p.room].door_inside_tile);
    p.next_state = kPsGoForTreatment;
    process(r, id);
    return false;
}

// The turn of a patient in state `state`, other than PS_ARRIVED_FROM_CALL.
void turn_state(const Run &r, uint16_t id, uint8_t state)
{
    World &world = r.world;
    switch (state) {
    case kPsWaitsForBusy:
        return waits_for_busy(r, id);
    case kPsGotoDestination:
        return goto_destination(r, id);
    case kPsReturnToQueueArea:
        return return_to_queue_area(r, id);
    case kPsGoHome:
        return go_home(r, id);
    case kPsGone:
        // person_gone (0x1673c).
        return remove(r, id);
    case kPsPlayComplexAnim:
        return play_complex_anim(r, id);
    case kPsArrivesAtDoor:
        return arrives_at_door(r, id);
    case kPsInQueue:
        return wait_in_queue(r, id, false);
    case kPsGotoObject:
        return goto_object(r, id);
    case kPsEntertainsThemself:
        return wait_in_queue(r, id, true);
    case kPsGotoQueue:
        return goto_queue(r, id);
    case kPsGoForTreatment:
        return go_for_treatment(r, id);
    case kPsComplexRetry:
        return complex_retry(r, id);
    case kPsShowanim:
        return person_showanim_turn(world, r.anims, id, r.game_turn);
    case kPsStuck:
        return stuck(r, id);
    case kPsInBed:
        return in_bed(r, id);
    case kPsWaitsForDecision:
        return waits_for_decision(r, id);
    case kPsBpGoto:
        return peep_bp_goto_turn(world, r.anims, id, r.game_turn);
    case kPsInBogs:
        return person_in_bogs(r, id);
    case kPsLeaveToilets:
        return leave_toilets(r, id);
    case kPsFills:
        return person_fills(r, id);
    case kPsEnergises:
        return person_energises(r, id);
    case kPsDead:
        return person_dead(world, r.anims, id, r.game_turn);
    case kPsFlys:
        return person_flys(world, r.anims, id, r.game_turn);
    default:
        return;
    }
}

} // namespace

inline namespace complex_detail
{

bool turn(const Run &r, uint16_t id)
{
    World &world = r.world;
    uint8_t state = world.things[id].state;
    if (state != kPsArrivedFromCall)
        turn_state(r, id, state);
    else if (arrived_from_call(r, id))
        return true;
    return world.things[id].type == kThingPatient;
}
} // namespace complex_detail

} // namespace th
