#include "hosp.h"

extern "C" {


u8 fn_0002b400(Thing *w);
Thing *fn_00019aac(Map *t, u16 kind);
void fn_0003b234(Thing *w);

}

// FUNCTION: 0x0003a6e0
void doctor_in_office(Thing *doctor)
{
    if (fn_0002b400(doctor) ? 1 : (process_staff_complex(doctor), 0)) {
        if (find_object(((u16)doctor->y.tile << 7) + (u16)doctor->x.tile, 1, 1, 0x96, doctor->owner)) {
            reset_icon_value(doctor, 0x19);
            doctor->state_b = 8;
            doctor->state_a = 6;
            set_complex(doctor, 10, 0);
            fn_0003e724(doctor);
        } else {
            process_staff_complex(doctor);
        }
    }
}

// FUNCTION: 0x0003a7a8
void doctor_works_at_desk(Thing *doctor)
{
    u16 queue;
    CompFrame *f;

    if (actual_game_turn % doctor->anim_speed == 0) {
        doctor->sprite = frame[doctor->sprite].next;
        f = &frame[doctor->sprite];
        if (f->flags & 1) {
            doctor->f8e &= 0xaf;
            if (fn_0002b400(doctor) ? 1 : (process_staff_complex(doctor), 0)) {
                if (!shall_i_resign(doctor)) {
                    if (doctor->room->f22 || doctor->room->f3 == 4) {
                        process_staff_complex(doctor);
                    } else if ((doctor->f8e & 1) && take_a_break(doctor)) {
                    } else {
                        othervar.f0 = old_newrand() % 100;
                        if (othervar.f0 >= 0 && othervar.f0 < 25) {
                            queue = doctor->room->queue_head;
                            if (queue) {
                                if (call_person_to_room(doctor, queue, 0x28, doctor->room->unknown4, 1)) {
                                    set_icon_value(doctor, 0x19);
                                    if (find_object((u16)doctor->x.tile + ((u16)doctor->y.tile << 7), 2, 1, 0x96, doctor->owner)) {
                                        doctor->state_b = 0x14;
                                        set_complex(doctor, 0x32, 0);
                                        fn_0003e724(doctor);
                                    } else {
                                        doctor->kind = 0x14;
                                    }
                                }
                            }
                        } else {
                            othervar.f0 = old_newrand() % 100;
                            if (othervar.f0 >= 25 && othervar.f0 < 32) {
                                if (find_object(((u16)doctor->y.tile << 7) + (u16)doctor->x.tile, 2, 1, 0x96, doctor->owner)) {
                                    doctor->state_a = 6;
                                    set_complex(doctor, 0x14, 0);
                                    fn_0003e724(doctor);
                                }
                            } else if (doctor->room->unknown29) {
                                u8 paged;

                                paged = 0;
                                if (doctor->f78 > allhosps[doctor->owner].f21d && take_a_break(doctor)) {
                                    paged = 1;
                                } else if ((allhosps[doctor->owner].f21f && doctor->room->queue_head == 0)
                                           && (actual_game_turn % (doctor->f78 > allhosps[doctor->owner].f21d
                                                                      ? doctor->unknown141 * 2 + 1
                                                                      : doctor->unknown141 + 1) == 0
                                           && obey_pager(doctor))) {
                                    reset_icon_value(doctor, 0x19);
                                    paged = 1;
                                }
                                if (paged && doctor->room->queue_head) {
                                    page_member_of_staff(0x10, doctor->room - room_info, 0, 1, doctor->owner);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// FUNCTION: 0x0003aba4
void doctor_waits_for_patient_to_arrive(Thing *doctor)
{
    u8 result;
    Thing *patient;

    patient = &thing[doctor->unknown73];
    if (patient > thing) {
        if (patient->kind == 0x46) {
            patient->kind = 0x20;
            if ((doctor->subtype == 1 && doctor->unknown148[2]) && (old_newrand() & 3) == 0) {
                send_person_somewhere_else(patient);
                doctor->kind = get_state(doctor->room->type, doctor);
                doctor->unknown73 = 0;
                return;
            }
            patient->state_a = 0;
            switch (doctor->room->type) {
            case 7:
                if (find_object(((u16)patient->y.tile << 7) + (u16)patient->x.tile, 6, 1, 0x96, patient->owner)) {
                    patient->state_b = 0x20;
                    if (set_complex(patient, 0x82, 0)) {
                        process_person_complex(patient);
                    }
                    doctor->unknown103++;
                }
                doctor->state_b = 0x1c;
                doctor->unknown103 = 1;
                set_complex(doctor, 0xfa, 0);
                fn_0003e724(doctor);
                break;
            case 17:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 10;
                set_complex(doctor, 0x1e0, 0);
                fn_0003e724(doctor);
                break;
            case 12:
                patient->state_b = 0x20;
                set_complex(patient, 0xaa, 0);
                process_person_complex(patient);
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0x1e;
                set_complex(doctor, 0xb4, 0);
                fn_0003e724(doctor);
                break;
            case 13:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0x32;
                if (illness_abilities[patient->style][0]) {
                    set_complex(doctor, 0xd2, 0);
                } else {
                    set_complex(doctor, 0x500, 0);
                }
                fn_0003e724(doctor);
                break;
            case 8:
                if (find_object(((u16)doctor->y.tile << 7) + (u16)doctor->x.tile, 0x38, 1, 0x96, doctor->owner)) {
                    set_complex(doctor, 0x370, 0);
                }
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0x3c;
                set_complex(doctor, 0x37a, 0);
                fn_0003e724(doctor);
                patient->state_b = 0x20;
                set_complex(patient, 0x1e0, 0);
                process_person_complex(patient);
                break;
            case 10: {
                Thing *assistant;

                doctor->unknown148w = doctor->room->staff[1];
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0x46;
                set_complex(doctor, 0x316, 0);
                fn_0003e724(doctor);
                assistant = &thing[doctor->unknown148w];
                assistant->state_b = 0x18;
                set_complex(assistant, 0x32a, 0);
                fn_0003e724(assistant);
                break;
            }
            case 14:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0x50;
                set_complex(doctor, 0x492, 0);
                fn_0003e724(doctor);
                break;
            case 15:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0x5a;
                set_complex(doctor, 0x398, 0);
                fn_0003e724(doctor);
                break;
            case 16:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0x64;
                set_complex(doctor, 0x276, 0);
                fn_0003e724(doctor);
                break;
            case 27:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0x6e;
                set_complex(doctor, 0x442, 0);
                fn_0003e724(doctor);
                break;
            case 19:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0x78;
                set_complex(doctor, 0x2bc, 0);
                fn_0003e724(doctor);
                break;
            case 20:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0x82;
                set_complex(doctor, 0x1cc, 0);
                fn_0003e724(doctor);
                break;
            case 21:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0x8c;
                set_complex(doctor, 0x42e, 0);
                fn_0003e724(doctor);
                break;
            case 23:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0xa0;
                set_complex(doctor, 0x186, 0);
                fn_0003e724(doctor);
                break;
            case 24:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0xaa;
                set_complex(doctor, 0x19a, 0);
                fn_0003e724(doctor);
                break;
            case 11:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0xb4;
                set_complex(doctor, 0x244, 0);
                fn_0003e724(doctor);
                break;
            case 30:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0xd2;
                set_complex(doctor, 0x28a, 0);
                fn_0003e724(doctor);
                break;
            case 28:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0xdc;
                set_complex(doctor, 0x35c, 0);
                fn_0003e724(doctor);
                break;
            case 18:
                doctor->state_b = 0x1c;
                doctor->unknown103 = 0xf0;
                set_complex(doctor, 0x47e, 0);
                fn_0003e724(doctor);
                break;
            case 9:
            case 22:
            case 25:
            case 26:
            case 29:
            default:
                break;
            }
        } else {
            doctor->timer--;
            if (doctor->timer <= 0) {
                result = call_person_again(doctor, 0x28, doctor->room->unknown4);
                if (result == 0) {
                    fn_0003b234(doctor);
                } else if (result == 1) {
                    fn_0003b234(doctor);
                    interrupt_person(patient);
                    start_person_going(patient);
                }
            } else if (fn_0003c1e8(doctor, 0x28)) {
                fn_0003b234(doctor);
            }
        }
    } else {
        doctor->kind = get_state(doctor->room->type, doctor);
    }
}

// FUNCTION: 0x0003b234
void fn_0003b234(Thing *w)
{
    w->kind = get_state(w->room->type, w);
    w->unknown73 = 0;
    if (w->subtype == 4) {
        w = fn_0003c154(w->room, 0x18);
        if (w) {
            w->kind = get_state(w->room->type, w);
            w->unknown73 = 0;
        }
    }
}

// FUNCTION: 0x0003b2c4
void doctor_in_machine_room(Thing *doctor)
{
    u16 queue;
    Thing *e;
    CompFrame *f;

    if (actual_game_turn % doctor->anim_speed == 0) {
        doctor->sprite = frame[doctor->sprite].next;
        f = &frame[doctor->sprite];
        if (f->flags & 1) {
            if (doctor->room->type == 10 && doctor->subtype == 1) {
                doctor->kind = 0x42;
            } else {
                doctor->f8e &= 0xaf;
                if (fn_0002b400(doctor) ? 1 : (process_staff_complex(doctor), 0)) {
                    if (!shall_i_resign(doctor)) {
                        if (doctor->room->f22 || doctor->room->f3 == 4) {
                            process_staff_complex(doctor);
                        } else if ((doctor->f8e & 1) && take_a_break(doctor)) {
                        } else if ((doctor->subtype == 0 && allhosps[doctor->owner].unknown11f0 == 0) && could_nurse_vaccinate(doctor)) {
                            process_staff_complex(doctor);
                        } else {
                            if (doctor->room->f3 == 5) {
                                doctor->room->f3 = 6;
                            }
                            othervar.f0 = old_newrand() % 100;
                            if (othervar.f0 >= 0 && othervar.f0 < 25) {
                                if (doctor->room->f3 != 6 && (queue = doctor->room->queue_head)) {
                                    if (doctor->room->type == 10) {
                                        if (am_i_lead_staff(doctor)) {
                                            doctor->timer = 200;
                                            doctor->kind = 0x10;
                                        } else {
                                            e = fn_0003c154(doctor->room, 0x10);
                                            if (e > thing && call_person_to_room(e, queue, 0x28, doctor->room->unknown4, 1)) {
                                                u16 anim;

                                                doctor->kind = 0x18;
                                                set_icon_value(e, 0x19);
                                                anim = check_frame(staff_anim_starts[e->subtype].unknownA, e) + (u16)e->unknown17;
                                                e->sprite = start[anim].frame;
                                                e->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                                                e->kind = 0x14;
                                            }
                                        }
                                    } else if (call_person_to_room(doctor, queue, 0x28, doctor->room->unknown4, 1)) {
                                        u16 anim;

                                        set_icon_value(doctor, 0x19);
                                        anim = check_frame(staff_anim_starts[doctor->subtype].unknownA, doctor) + (u16)doctor->unknown17;
                                        doctor->sprite = start[anim].frame;
                                        doctor->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                                        doctor->kind = 0x14;
                                    }
                                }
                            } else {
                                othervar.f0 = old_newrand() % 100;
                                if ((othervar.f0 >= 25 && othervar.f0 < 27)
                                    || ((s16)doctor->y.tile << 7) + (s16)doctor->x.tile == doctor->room->unknown6) {
                                    u16 tile;
                                    RoomInfo *cur_room;
                                    u16 pos;

                                    tile = (u16)doctor->x.tile + ((u16)doctor->y.tile << 7);
                                    cur_room = which_room(tile);
                                    othervar.f0 = old_newrand() % 100;
                                    if (othervar.f0 >= 25 && othervar.f0 < 26) {
                                        pos = ((s16)(cur_room->f2 / 2) << 7) + (s16)(cur_room->f1 / 2) + cur_room->pos;
                                    } else {
                                        pos = ((s16)(old_newrand() % cur_room->f2) << 7) + (s16)(old_newrand() % cur_room->f1) + cur_room->pos;
                                    }
                                    if (tile != pos && can_i_get_to(doctor, pos)) {
                                        doctor->state_b = doctor->state_a = 0x24;
                                        set_complex(doctor, 0xbe, pos);
                                        fn_0003e724(doctor);
                                    }
                                } else if (doctor->room->unknown29) {
                                    u8 paged;

                                    paged = 0;
                                    if (doctor->f78 > allhosps[doctor->owner].f21d && take_a_break(doctor)) {
                                        paged = 1;
                                    } else if ((allhosps[doctor->owner].f21f && doctor->room->queue_head == 0)
                                               && (actual_game_turn % (doctor->f78 > allhosps[doctor->owner].f21d
                                                                          ? doctor->unknown141 * 2 + 1
                                                                          : doctor->unknown141 + 1) == 0
                                               && obey_pager(doctor))) {
                                        paged = 1;
                                    }
                                    if (paged && doctor->room->queue_head) {
                                        page_member_of_staff(rooms[doctor->room->type].unknown8, doctor->room - room_info, 0, 1, doctor->owner);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// FUNCTION: 0x0003b9a0
void doctor_researches(Thing *doctor)
{
    u16 i;
    u16 ok;
    u16 obj;
    u16 queue;
    CompFrame *f;
    Thing *user;
    Thing *occupant;

    if (actual_game_turn % doctor->anim_speed == 0) {
        doctor->sprite = frame[doctor->sprite].next;
        f = &frame[doctor->sprite];
        if (f->flags & 1) {
            if (!(doctor->unknown124 & 0x100)) {
                process_staff_complex(doctor);
            } else if (!shall_i_resign(doctor)) {
                doctor->f8e |= 0x10;
                if (doctor->room->f22 || doctor->room->f3 == 4) {
                    process_staff_complex(doctor);
                } else if (doctor->f8e & 8) {
                    doctor->f8e &= 0xf0;
                    process_staff_complex(doctor);
                } else if (((doctor->f8e & 1) || doctor->f78 > allhosps[doctor->owner].f21d) && take_a_break(doctor)) {
                } else {
                    othervar.f0 = old_newrand() % 100;
                    if (othervar.f0 >= 0 && othervar.f0 < 25) {
                        queue = doctor->room->queue_head;
                        if (queue) {
                            ok = 1;
                            for (occupant = &thing[doctor->room->first]; occupant > thing; occupant = &thing[occupant->room_next]) {
                                if (occupant->renderer == 4 && occupant->unknown73) {
                                    ok = 0;
                                } else if (occupant->renderer == 2) {
                                    ok = 0;
                                }
                            }
                            if (ok) {
                                if (call_person_to_room(doctor, queue, 0x28, doctor->room->unknown4, 1)) {
                                    set_icon_value(doctor, 0x19);
                                    doctor->kind = 0x14;
                                    return;
                                }
                            }
                        }
                    } else {
                        for (i = 0; i < 4; i++) {
                            obj = find_object((u16)doctor->x.tile + ((u16)doctor->y.tile << 7), research_tasks[i].object, 1, 0x96, doctor->owner);
                            if (obj) {
                                user = fn_00019aac(&map[thing[obj].room_prev], 4);
                                if (!(user == 0 || user == doctor)) {
                                    continue;
                                }
                                if (old_newrand() % research_tasks[i].chance == 0) {
                                    doctor->state_a = 0x3e;
                                    doctor->state_b = 0x3e;
                                    set_complex(doctor, research_tasks[i].action, 0);
                                    fn_0003e724(doctor);
                                    return;
                                }
                            }
                        }
                    }
                    overlapping_staff_check(doctor, 0x3e);
                }
            }
        }
    }
}

// FUNCTION: 0x0003bd34
void doctor_cracks_up(Thing *doctor)
{
    u32 mean;
    u32 spread;

    if (doctor->subtype == 1) {
        if (doctor->subtype == 1 && doctor->unknown148[2]) {
            return;
        }
        if (doctor->unknown30[4] == 1) {
            doctor->unknown30[4] = 3;
        } else if (doctor->unknown30[4] == 2) {
            doctor->unknown30[4] = 4;
        }
        doctor->unknown148[2] = 1;
        mean = gbv.f156;
        spread = gbv.f158;
        if (doctor->unknown124 & 0x80) {
            mean = gbv.f158;
            spread = 1000;
        } else if (doctor->unknown124 & 8) {
            mean = 30;
            spread = gbv.f156;
        }
        doctor->unknown126 = old_normal_rand(mean, spread);
        if (doctor->owner == player) {
            adcounts.unknown8e++;
        }
    }
}

// FUNCTION: 0x0003be38
u8 count_surgeons(RoomInfo *room)
{
    Thing *person;
    u8 count;

    count = 0;
    for (person = &thing[room->first]; person > thing; person = &thing[person->room_next]) {
        if (person->renderer == 4 && person->subtype == 4) {
            count++;
        }
    }
    return count;
}

// FUNCTION: 0x0003bed0
void doctor_in_theatre(Thing *doctor)
{
    u8 count;
    u16 obj;
    u16 pos;
    RoomInfo *room;

    room = doctor->room;
    count = count_surgeons(doctor->room);
    doctor->unknown69 = 0;
    if (!(doctor->unknown124 & 0x20)) {
        process_staff_complex(doctor);
        return;
    }
    switch (count) {
    case 0:
        obj = find_object(((u16)doctor->y.tile << 7) + (u16)doctor->x.tile, 0x23, 0, 0x96, doctor->owner);
        if (obj) {
            set_complex(doctor, 0x4d8, 0);
            doctor->state_a = 0x42;
            doctor->state_b = 0x24;
            fn_0003e724(doctor);
            return;
        }
        break;
    case 1:
        obj = find_object(((u16)doctor->y.tile << 7) + (u16)doctor->x.tile, 0x23, 0, 0x96, doctor->owner);
        if (obj) {
            set_complex(doctor, 0x4ec, 0);
            doctor->state_a = 0x42;
            doctor->state_b = 0x24;
            fn_0003e724(doctor);
            return;
        }
        break;
    default:
        process_staff_complex(doctor);
        return;
    }
    pos = (s16)(old_newrand() % room->f1) + ((s16)(old_newrand() % room->f2) << 7) + room->pos;
    if (can_i_get_to(doctor, pos)) {
        doctor->state_b = 0x42;
        set_complex(doctor, 0xbe, pos);
        fn_0003e724(doctor);
    }
}

// FUNCTION: 0x0003c0a8
void change_surgeon_to_doc(Thing *surgeon)
{
    u8 count;

    count = count_surgeons(surgeon->room);
    if (count == 1) {
        surgeon->state_a = 0x36;
        set_complex(surgeon, 0x3ca, 0);
    } else if (count == 2) {
        surgeon->state_a = 0x36;
        set_complex(surgeon, 0x3d4, 0);
    } else {
        surgeon->subtype = 1;
    }
}

// FUNCTION: 0x0003c11c
void surgeon_waits(Thing *surgeon)
{
    surgeon->timer--;
    if (surgeon->timer <= 0) {
        surgeon->kind = 0x24;
    }
}

// FUNCTION: 0x0003c154
Thing *fn_0003c154(RoomInfo *room, u8 kind)
{
    Thing *person;

    for (person = &thing[room->first]; person > thing; person = &thing[person->room_next]) {
        if (person->renderer == 4 && person->kind == kind) {
            return person;
        }
    }
    return 0;
}

// FUNCTION: 0x0003c1e8
u8 fn_0003c1e8(Thing *w, u16 action)
{
    Thing *e;
    Thing *slave;
    u16 room;
    u8 result;

    result = 0;
    slave = &thing[w->unknown73];
    if (slave > thing) {
        if (slave->unknown130 <= 0) {
            result = 1;
        } else if (slave->renderer != 2) {
            result = 1;
        } else if (slave->unknown140) {
            result = 1;
        } else if (slave->state_b != 0x46) {
            if (slave->state_b != 0x18) {
                result = 1;
            } else if (w->subtype == 3) {
                e = &thing[w->unknown69];
                if (e > thing) {
                    room = e->room_next;
                    call_person_to_room(w, w->unknown73, action, room, 0);
                }
            } else {
                room = w->room->unknown4;
                if (room) {
                    call_person_to_room(w, w->unknown73, action, room, 0);
                }
            }
        }
    } else {
        result = 1;
    }
    return result;
}
