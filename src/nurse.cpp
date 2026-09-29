#include "hosp.h"

extern "C" {


void fn_000168a0(Thing *w);
u8 fn_0002b400(Thing *w);

}

u16 find_unallocated_bed(RoomInfo *room);

// FUNCTION: 0x0006c640
void reinit_nurse(Thing *w)
{
    RoomInfo *room;
    u16 pos;

    fn_000168a0(w);
    pos = w->x.tile + (w->y.tile << 7);
    room = which_room(pos);
    if (room > 0) {
        if (room->type != 1)
            insert_person_in_room_list(room, w);
        w->kind = get_state((u16)room->type, w);
        if (room->type == 9)
            w->unknown103 = 0;
    } else {
        w->kind = get_state(0, w);
    }
    w->unknown103 = 0;
}

// FUNCTION: 0x0006c6f4
u8 nurse_check_for_work(Thing *w)
{
    RoomInfo *room;

    room = get_room_info_rand(9, 100, w->owner);
    if (room == 0)
        return 0;
    set_complex(w, 0x14a, room->unknown4);
    w->state_a = 0x36;
    w->unknown103 = 0;
    w->state_b = 0x32;
    w->state_a = 0x28;
    w->unknown89 = room - room_info;
    fn_0003e724(w);
    return 1;
}

// FUNCTION: 0x0006c79c
u16 find_unallocated_bed(RoomInfo *room)
{
    Thing *bed;
    Thing *p;

    for (bed = &thing[room->first_object]; bed > thing; bed = &thing[bed->unknown71]) {
        if (bed->subtype == 8 && bed->unknown91 == 0) {
            for (p = &thing[room->first]; p > thing; p = &thing[p->room_next]) {
                if (p->renderer == 2 && &thing[p->unknown69] == bed)
                    break;
            }
            if (p <= thing)
                return bed - thing;
        }
    }
    return 0;
}

// FUNCTION: 0x0006c8d8
void nurse_in_ward(Thing *w)
{
    Thing *patient;
    RoomInfo *room;
    int result;
    int queue;
    int cur;
    int target;
    int bed;
    u32 obj;

    obj = find_object(w->x.tile + (w->y.tile << 7), 1, 0, 0x97, w->owner);
    switch (w->unknown103) {
    case 0:
        w->room->f3 == 4 ? 0 : (w->room->f3 = 1);
        if (!(fn_0002b400(w) ? 1 : (process_staff_complex(w), 0)))
            return;
        if (shall_i_resign(w))
            return;
        if ((w->f8e & 1) && take_a_break(w))
            return;
        if (allhosps[w->owner].f21f
            && actual_game_turn % (w->f78 > allhosps[w->owner].f21d ? w->unknown141 * 2 + 1 : w->unknown141 + 1) == 0
            && obey_pager(w))
            return;
        if (w->f78 > allhosps[w->owner].f21d && take_a_break(w))
            return;
        if (w->room->f22 || w->room->f3 == 4) {
            process_staff_complex(w);
            return;
        }
        if (allhosps[w->owner].unknown11f0 == 0 && could_nurse_vaccinate(w)) {
            process_staff_complex(w);
            return;
        }
        queue = w->room->queue_head;
        if (queue) {
            bed = find_unallocated_bed(w->room);
            if (bed) {
                if (call_person_to_room(w, queue, 0x28, w->room->unknown4, 0)) {
                    w->unknown69 = bed;
                    w->state_b = 0x32;
                    w->unknown103 = 1;
                    set_complex(w, 0x15e, 0);
                    fn_0003e724(w);
                    return;
                }
            }
        }
        if (obj > 0 && w->unknown148w != obj && old_newrand() % 100 < 10) {
            w->unknown148w = obj;
            set_complex(w, 0x168, 0);
            w->state_b = 0x32;
            fn_0003e724(w);
            return;
        }
        patient = &thing[find_peep(w->room - room_info, 1, 2, 0x2c, w->owner)];
        if (patient > thing) {
            if ((u16)(patient - thing) != w->unknown148w) {
                w->unknown148w = patient - thing;
                w->unknown69 = patient->unknown69;
                w->state_b = 0x32;
                set_complex(w, 0x17c, 0);
                fn_0003e724(w);
                return;
            }
        }
        cur = (s16)w->x.tile + ((s16)w->y.tile << 7);
        room = which_room(cur);
        othervar.f0 = old_newrand() % 100;
        if (othervar.f0 >= 15 && othervar.f0 < 20)
            target = room->pos + (((s16)(room->f2 / 2) << 7) + (s16)(room->f1 / 2));
        else
            target = room->pos + (((s16)(old_newrand() % room->f2) << 7) + (s16)(old_newrand() % room->f1));
        if (cur != target && can_i_get_to(w, target)) {
            w->state_b = w->state_a = 0x32;
            set_complex(w, 0xbe, target);
            fn_0003e724(w);
            return;
        }
        othervar.f0 = old_newrand() % 100;
        if (othervar.f0 >= 0x46 && othervar.f0 < 0x50)
            w->unknown148w = 0;
        break;
    case 1:
        patient = &thing[w->unknown73];
        if (patient->kind == 0x46) {
            affect_diagnosis(w, patient);
            w->unknown103 = 0;
            w->unknown73 = 0;
            patient->unknown69 = w->unknown69;
            w->unknown69 = 0;
            set_complex(patient, 200, 0);
            patient->state_b = 0x2c;
            process_person_complex(patient);
        } else if (--w->timer <= 0) {
            result = call_person_again(w, 0x28, w->room->unknown4);
            if (result == 0) {
                w->unknown103 = 0;
                w->unknown73 = 0;
            } else if (result == 1) {
                w->unknown103 = 0;
                w->unknown73 = 0;
                interrupt_person(patient);
                start_person_going(patient);
            }
        } else if (fn_0003c1e8(w, 0x28)) {
            w->unknown103 = 0;
            w->unknown73 = 0;
        }
        break;
    }
}
