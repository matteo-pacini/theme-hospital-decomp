#include "hosp.h"

extern "C" {


void remove_person_from_room_list(Thing *w);
u8 check_room_mutex(Thing *w);

}

u16 find_unallocated_bed(RoomInfo *room);

// Puts nurse back into the state for the room she stands in (or the corridor
// state), re-entering her in that room's list of people.
// FUNCTION: 0x0006c640
void reinit_nurse(Thing *nurse)
{
    RoomInfo *room;
    u16 pos;

    remove_person_from_room_list(nurse);
    pos = nurse->x.tile + (nurse->y.tile << 7);
    room = which_room(pos);
    if (room > 0) {
        if (room->type != 1)
            insert_person_in_room_list(room, nurse);
        nurse->state = get_state((u16)room->type, nurse);
        if (room->type == ROOM_WARD)
            nurse->substate = 0;
    } else {
        nurse->state = get_state(0, nurse);
    }
    nurse->substate = 0;
}

// Sends nurse to a random ward of her hospital. Returns 1 if there was one,
// 0 otherwise.
// FUNCTION: 0x0006c6f4
u8 nurse_check_for_work(Thing *nurse)
{
    RoomInfo *room;

    room = get_room_info_rand(ROOM_WARD, 100, nurse->owner);
    if (room == 0)
        return 0;
    set_complex(nurse, 0x14a, room->door_tile);
    nurse->resume_state = SS_PROCESS_COMPLEX;
    nurse->substate = 0;
    nurse->next_state = SS_NURSE_IN_WARD;
    nurse->resume_state = SS_IN_CORRIDOR;
    nurse->dest_room = room - room_info;
    process_staff_complex(nurse);
    return 1;
}

// Returns a bed in room (a ward) that no one uses or is assigned to, or 0.
// FUNCTION: 0x0006c79c
u16 find_unallocated_bed(RoomInfo *room)
{
    Thing *bed;
    Thing *occupant;

    for (bed = &thing[room->first_object]; bed > thing; bed = &thing[bed->room_obj_next]) {
        if (bed->subtype == OBJ_BED && bed->user == 0) {
            for (occupant = &thing[room->first]; occupant > thing; occupant = &thing[occupant->room_next]) {
                if (occupant->type == THING_PATIENT && &thing[occupant->object] == bed)
                    break;
            }
            if (occupant <= thing)
                return bed - thing;
        }
    }
    return 0;
}

// Nurse working in a ward. Substate 0: leaves for a break, the pager,
// vaccinations or an evacuation as needed; otherwise calls the next queued
// patient to a free bed, or visits the desk, a patient in bed, or a random
// spot. Substate 1: waits for the called patient and puts them to bed.
// FUNCTION: 0x0006c8d8
void nurse_in_ward(Thing *nurse)
{
    Thing *patient;
    RoomInfo *room;
    int result;
    int queue;
    int cur;
    int target;
    int bed;
    u32 obj;

    obj = find_object(nurse->x.tile + (nurse->y.tile << 7), OBJ_DESK, 0, 0x97, nurse->owner);
    switch (nurse->substate) {
    case 0:
        nurse->room->state == 4 ? 0 : (nurse->room->state = 1);
        if (!(check_room_mutex(nurse) ? 1 : (evacuate_staff(nurse), 0)))
            return;
        if (shall_i_resign(nurse))
            return;
        if ((nurse->orders & ORD_REST) && take_a_break(nurse))
            return;
        if (allhosps[nurse->owner].staff_leave_rooms
            && actual_game_turn % (nurse->tiredness > allhosps[nurse->owner].rest_threshold ? nurse->work_delay * 2 + 1 : nurse->work_delay + 1) == 0
            && obey_pager(nurse))
            return;
        if (nurse->tiredness > allhosps[nurse->owner].rest_threshold && take_a_break(nurse))
            return;
        if (nurse->room->being_edited || nurse->room->state == 4) {
            evacuate_staff(nurse);
            return;
        }
        if (allhosps[nurse->owner].idle_nurses == 0 && could_nurse_vaccinate(nurse)) {
            evacuate_staff(nurse);
            return;
        }
        queue = nurse->room->queue_head;
        if (queue) {
            bed = find_unallocated_bed(nurse->room);
            if (bed) {
                if (call_person_to_room(nurse, queue, 0x28, nurse->room->door_tile, 0)) {
                    nurse->object = bed;
                    nurse->next_state = SS_NURSE_IN_WARD;
                    nurse->substate = 1;
                    set_complex(nurse, 0x15e, 0);
                    process_staff_complex(nurse);
                    return;
                }
            }
        }
        if (obj > 0 && nurse->work_target != obj && old_newrand() % 100 < 10) {
            nurse->work_target = obj;
            set_complex(nurse, 0x168, 0);
            nurse->next_state = SS_NURSE_IN_WARD;
            process_staff_complex(nurse);
            return;
        }
        patient = &thing[find_peep(nurse->room - room_info, 1, THING_PATIENT, PS_IN_BED, nurse->owner)];
        if (patient > thing) {
            if ((u16)(patient - thing) != nurse->work_target) {
                nurse->work_target = patient - thing;
                nurse->object = patient->object;
                nurse->next_state = SS_NURSE_IN_WARD;
                set_complex(nurse, 0x17c, 0);
                process_staff_complex(nurse);
                return;
            }
        }
        cur = (s16)nurse->x.tile + ((s16)nurse->y.tile << 7);
        room = which_room(cur);
        othervar.rand_pct = old_newrand() % 100;
        if (othervar.rand_pct >= 15 && othervar.rand_pct < 20)
            target = room->pos + (((s16)(room->height / 2) << 7) + (s16)(room->width / 2));
        else
            target = room->pos + (((s16)(old_newrand() % room->height) << 7) + (s16)(old_newrand() % room->width));
        if (cur != target && can_i_get_to(nurse, target)) {
            nurse->next_state = nurse->resume_state = SS_NURSE_IN_WARD;
            set_complex(nurse, 0xbe, target);
            process_staff_complex(nurse);
            return;
        }
        othervar.rand_pct = old_newrand() % 100;
        if (othervar.rand_pct >= 0x46 && othervar.rand_pct < 0x50)
            nurse->work_target = 0;
        break;
    case 1:
        patient = &thing[nurse->slave];
        if (patient->state == PS_ARRIVED_FROM_CALL) {
            affect_diagnosis(nurse, patient);
            nurse->substate = 0;
            nurse->slave = 0;
            patient->object = nurse->object;
            nurse->object = 0;
            set_complex(patient, 200, 0);
            patient->next_state = PS_IN_BED;
            process_person_complex(patient);
        } else if (--nurse->timer <= 0) {
            result = call_person_again(nurse, 0x28, nurse->room->door_tile);
            if (result == 0) {
                nurse->substate = 0;
                nurse->slave = 0;
            } else if (result == 1) {
                nurse->substate = 0;
                nurse->slave = 0;
                interrupt_person(patient);
                start_person_going(patient);
            }
        } else if (forget_about_my_slave(nurse, 0x28)) {
            nurse->substate = 0;
            nurse->slave = 0;
        }
        break;
    }
}
