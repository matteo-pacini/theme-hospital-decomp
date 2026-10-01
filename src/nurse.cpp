#include "hosp.h"

extern "C" {


void fn_000168a0(Thing *w);
u8 fn_0002b400(Thing *w);

}

u16 find_unallocated_bed(RoomInfo *room);

// FUNCTION: 0x0006c640
void reinit_nurse(Thing *nurse)
{
    RoomInfo *room;
    u16 pos;

    fn_000168a0(nurse);
    pos = nurse->x.tile + (nurse->y.tile << 7);
    room = which_room(pos);
    if (room > 0) {
        if (room->type != 1)
            insert_person_in_room_list(room, nurse);
        nurse->state = get_state((u16)room->type, nurse);
        if (room->type == 9)
            nurse->substate = 0;
    } else {
        nurse->state = get_state(0, nurse);
    }
    nurse->substate = 0;
}

// FUNCTION: 0x0006c6f4
u8 nurse_check_for_work(Thing *nurse)
{
    RoomInfo *room;

    room = get_room_info_rand(9, 100, nurse->owner);
    if (room == 0)
        return 0;
    set_complex(nurse, 0x14a, room->unknown4);
    nurse->resume_state = 0x36;
    nurse->substate = 0;
    nurse->next_state = 0x32;
    nurse->resume_state = 0x28;
    nurse->dest_room = room - room_info;
    fn_0003e724(nurse);
    return 1;
}

// FUNCTION: 0x0006c79c
u16 find_unallocated_bed(RoomInfo *room)
{
    Thing *bed;
    Thing *occupant;

    for (bed = &thing[room->first_object]; bed > thing; bed = &thing[bed->room_obj_next]) {
        if (bed->subtype == 8 && bed->user == 0) {
            for (occupant = &thing[room->first]; occupant > thing; occupant = &thing[occupant->room_next]) {
                if (occupant->type == 2 && &thing[occupant->object] == bed)
                    break;
            }
            if (occupant <= thing)
                return bed - thing;
        }
    }
    return 0;
}

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

    obj = find_object(nurse->x.tile + (nurse->y.tile << 7), 1, 0, 0x97, nurse->owner);
    switch (nurse->substate) {
    case 0:
        nurse->room->f3 == 4 ? 0 : (nurse->room->f3 = 1);
        if (!(fn_0002b400(nurse) ? 1 : (process_staff_complex(nurse), 0)))
            return;
        if (shall_i_resign(nurse))
            return;
        if ((nurse->orders & 1) && take_a_break(nurse))
            return;
        if (allhosps[nurse->owner].f21f
            && actual_game_turn % (nurse->tiredness > allhosps[nurse->owner].f21d ? nurse->work_delay * 2 + 1 : nurse->work_delay + 1) == 0
            && obey_pager(nurse))
            return;
        if (nurse->tiredness > allhosps[nurse->owner].f21d && take_a_break(nurse))
            return;
        if (nurse->room->f22 || nurse->room->f3 == 4) {
            process_staff_complex(nurse);
            return;
        }
        if (allhosps[nurse->owner].unknown11f0 == 0 && could_nurse_vaccinate(nurse)) {
            process_staff_complex(nurse);
            return;
        }
        queue = nurse->room->queue_head;
        if (queue) {
            bed = find_unallocated_bed(nurse->room);
            if (bed) {
                if (call_person_to_room(nurse, queue, 0x28, nurse->room->unknown4, 0)) {
                    nurse->object = bed;
                    nurse->next_state = 0x32;
                    nurse->substate = 1;
                    set_complex(nurse, 0x15e, 0);
                    fn_0003e724(nurse);
                    return;
                }
            }
        }
        if (obj > 0 && nurse->work_target != obj && old_newrand() % 100 < 10) {
            nurse->work_target = obj;
            set_complex(nurse, 0x168, 0);
            nurse->next_state = 0x32;
            fn_0003e724(nurse);
            return;
        }
        patient = &thing[find_peep(nurse->room - room_info, 1, 2, 0x2c, nurse->owner)];
        if (patient > thing) {
            if ((u16)(patient - thing) != nurse->work_target) {
                nurse->work_target = patient - thing;
                nurse->object = patient->object;
                nurse->next_state = 0x32;
                set_complex(nurse, 0x17c, 0);
                fn_0003e724(nurse);
                return;
            }
        }
        cur = (s16)nurse->x.tile + ((s16)nurse->y.tile << 7);
        room = which_room(cur);
        othervar.f0 = old_newrand() % 100;
        if (othervar.f0 >= 15 && othervar.f0 < 20)
            target = room->pos + (((s16)(room->f2 / 2) << 7) + (s16)(room->f1 / 2));
        else
            target = room->pos + (((s16)(old_newrand() % room->f2) << 7) + (s16)(old_newrand() % room->f1));
        if (cur != target && can_i_get_to(nurse, target)) {
            nurse->next_state = nurse->resume_state = 0x32;
            set_complex(nurse, 0xbe, target);
            fn_0003e724(nurse);
            return;
        }
        othervar.f0 = old_newrand() % 100;
        if (othervar.f0 >= 0x46 && othervar.f0 < 0x50)
            nurse->work_target = 0;
        break;
    case 1:
        patient = &thing[nurse->slave];
        if (patient->state == 0x46) {
            affect_diagnosis(nurse, patient);
            nurse->substate = 0;
            nurse->slave = 0;
            patient->object = nurse->object;
            nurse->object = 0;
            set_complex(patient, 200, 0);
            patient->next_state = 0x2c;
            process_person_complex(patient);
        } else if (--nurse->timer <= 0) {
            result = call_person_again(nurse, 0x28, nurse->room->unknown4);
            if (result == 0) {
                nurse->substate = 0;
                nurse->slave = 0;
            } else if (result == 1) {
                nurse->substate = 0;
                nurse->slave = 0;
                interrupt_person(patient);
                start_person_going(patient);
            }
        } else if (fn_0003c1e8(nurse, 0x28)) {
            nurse->substate = 0;
            nurse->slave = 0;
        }
        break;
    }
}
