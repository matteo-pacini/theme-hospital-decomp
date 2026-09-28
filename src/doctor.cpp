#include "hosp.h"

extern "C" {

extern s16 D_001553b8;

u8 fn_0002b400(Thing *w);
Thing *fn_00019aac(Map *t, u16 kind);
void fn_0003b234(Thing *w);

}

// FUNCTION: 0x0003a6e0
void doctor_in_office(Thing *w)
{
    if (fn_0002b400(w) ? 1 : (process_staff_complex(w), 0)) {
        if (find_object(((u16)w->y.tile << 7) + (u16)w->x.tile, 1, 1, 0x96, w->owner)) {
            reset_icon_value(w, 0x19);
            w->state_b = 8;
            w->state_a = 6;
            set_complex(w, 10, 0);
            fn_0003e724(w);
        } else {
            process_staff_complex(w);
        }
    }
}

// FUNCTION: 0x0003a7a8
void doctor_works_at_desk(Thing *w)
{
    u16 queue;
    CompFrame *f;

    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
        f = &frame[w->sprite];
        if (f->flags & 1) {
            w->f8e &= 0xaf;
            if (fn_0002b400(w) ? 1 : (process_staff_complex(w), 0)) {
                if (!shall_i_resign(w)) {
                    if (w->room->f22 || w->room->f3 == 4) {
                        process_staff_complex(w);
                    } else if ((w->f8e & 1) && take_a_break(w)) {
                    } else {
                        D_001553b8 = old_newrand() % 100;
                        if (D_001553b8 >= 0 && D_001553b8 < 25) {
                            queue = w->room->queue_head;
                            if (queue) {
                                if (call_person_to_room(w, queue, 0x28, w->room->unknown4, 1)) {
                                    set_icon_value(w, 0x19);
                                    if (find_object((u16)w->x.tile + ((u16)w->y.tile << 7), 2, 1, 0x96, w->owner)) {
                                        w->state_b = 0x14;
                                        set_complex(w, 0x32, 0);
                                        fn_0003e724(w);
                                    } else {
                                        w->kind = 0x14;
                                    }
                                }
                            }
                        } else {
                            D_001553b8 = old_newrand() % 100;
                            if (D_001553b8 >= 25 && D_001553b8 < 32) {
                                if (find_object(((u16)w->y.tile << 7) + (u16)w->x.tile, 2, 1, 0x96, w->owner)) {
                                    w->state_a = 6;
                                    set_complex(w, 0x14, 0);
                                    fn_0003e724(w);
                                }
                            } else if (w->room->unknown29) {
                                u8 paged;

                                paged = 0;
                                if (w->f78 > allhosps[w->owner].f21d && take_a_break(w)) {
                                    paged = 1;
                                } else if ((allhosps[w->owner].f21f && w->room->queue_head == 0)
                                           && (actual_game_turn % (w->f78 > allhosps[w->owner].f21d
                                                                      ? w->unknown141 * 2 + 1
                                                                      : w->unknown141 + 1) == 0
                                           && obey_pager(w))) {
                                    reset_icon_value(w, 0x19);
                                    paged = 1;
                                }
                                if (paged && w->room->queue_head) {
                                    page_member_of_staff(0x10, w->room - room_info, 0, 1, w->owner);
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
void doctor_waits_for_patient_to_arrive(Thing *w)
{
    u8 result;
    Thing *patient;

    patient = &thing[w->unknown73];
    if (patient > thing) {
        if (patient->kind == 0x46) {
            patient->kind = 0x20;
            if ((w->subtype == 1 && w->unknown148[2]) && (old_newrand() & 3) == 0) {
                send_person_somewhere_else(patient);
                w->kind = get_state(w->room->type, w);
                w->unknown73 = 0;
                return;
            }
            patient->state_a = 0;
            switch (w->room->type) {
            case 7:
                if (find_object(((u16)patient->y.tile << 7) + (u16)patient->x.tile, 6, 1, 0x96, patient->owner)) {
                    patient->state_b = 0x20;
                    if (set_complex(patient, 0x82, 0)) {
                        process_person_complex(patient);
                    }
                    w->unknown103++;
                }
                w->state_b = 0x1c;
                w->unknown103 = 1;
                set_complex(w, 0xfa, 0);
                fn_0003e724(w);
                break;
            case 17:
                w->state_b = 0x1c;
                w->unknown103 = 10;
                set_complex(w, 0x1e0, 0);
                fn_0003e724(w);
                break;
            case 12:
                patient->state_b = 0x20;
                set_complex(patient, 0xaa, 0);
                process_person_complex(patient);
                w->state_b = 0x1c;
                w->unknown103 = 0x1e;
                set_complex(w, 0xb4, 0);
                fn_0003e724(w);
                break;
            case 13:
                w->state_b = 0x1c;
                w->unknown103 = 0x32;
                if (illness_abilities[patient->style][0]) {
                    set_complex(w, 0xd2, 0);
                } else {
                    set_complex(w, 0x500, 0);
                }
                fn_0003e724(w);
                break;
            case 8:
                if (find_object(((u16)w->y.tile << 7) + (u16)w->x.tile, 0x38, 1, 0x96, w->owner)) {
                    set_complex(w, 0x370, 0);
                }
                w->state_b = 0x1c;
                w->unknown103 = 0x3c;
                set_complex(w, 0x37a, 0);
                fn_0003e724(w);
                patient->state_b = 0x20;
                set_complex(patient, 0x1e0, 0);
                process_person_complex(patient);
                break;
            case 10: {
                Thing *e;

                w->unknown148w = w->room->staff[1];
                w->state_b = 0x1c;
                w->unknown103 = 0x46;
                set_complex(w, 0x316, 0);
                fn_0003e724(w);
                e = &thing[w->unknown148w];
                e->state_b = 0x18;
                set_complex(e, 0x32a, 0);
                fn_0003e724(e);
                break;
            }
            case 14:
                w->state_b = 0x1c;
                w->unknown103 = 0x50;
                set_complex(w, 0x492, 0);
                fn_0003e724(w);
                break;
            case 15:
                w->state_b = 0x1c;
                w->unknown103 = 0x5a;
                set_complex(w, 0x398, 0);
                fn_0003e724(w);
                break;
            case 16:
                w->state_b = 0x1c;
                w->unknown103 = 0x64;
                set_complex(w, 0x276, 0);
                fn_0003e724(w);
                break;
            case 27:
                w->state_b = 0x1c;
                w->unknown103 = 0x6e;
                set_complex(w, 0x442, 0);
                fn_0003e724(w);
                break;
            case 19:
                w->state_b = 0x1c;
                w->unknown103 = 0x78;
                set_complex(w, 0x2bc, 0);
                fn_0003e724(w);
                break;
            case 20:
                w->state_b = 0x1c;
                w->unknown103 = 0x82;
                set_complex(w, 0x1cc, 0);
                fn_0003e724(w);
                break;
            case 21:
                w->state_b = 0x1c;
                w->unknown103 = 0x8c;
                set_complex(w, 0x42e, 0);
                fn_0003e724(w);
                break;
            case 23:
                w->state_b = 0x1c;
                w->unknown103 = 0xa0;
                set_complex(w, 0x186, 0);
                fn_0003e724(w);
                break;
            case 24:
                w->state_b = 0x1c;
                w->unknown103 = 0xaa;
                set_complex(w, 0x19a, 0);
                fn_0003e724(w);
                break;
            case 11:
                w->state_b = 0x1c;
                w->unknown103 = 0xb4;
                set_complex(w, 0x244, 0);
                fn_0003e724(w);
                break;
            case 30:
                w->state_b = 0x1c;
                w->unknown103 = 0xd2;
                set_complex(w, 0x28a, 0);
                fn_0003e724(w);
                break;
            case 28:
                w->state_b = 0x1c;
                w->unknown103 = 0xdc;
                set_complex(w, 0x35c, 0);
                fn_0003e724(w);
                break;
            case 18:
                w->state_b = 0x1c;
                w->unknown103 = 0xf0;
                set_complex(w, 0x47e, 0);
                fn_0003e724(w);
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
            w->timer--;
            if (w->timer <= 0) {
                result = call_person_again(w, 0x28, w->room->unknown4);
                if (result == 0) {
                    fn_0003b234(w);
                } else if (result == 1) {
                    fn_0003b234(w);
                    interrupt_person(patient);
                    start_person_going(patient);
                }
            } else if (fn_0003c1e8(w, 0x28)) {
                fn_0003b234(w);
            }
        }
    } else {
        w->kind = get_state(w->room->type, w);
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
void doctor_in_machine_room(Thing *w)
{
    u16 queue;
    Thing *e;
    CompFrame *f;

    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
        f = &frame[w->sprite];
        if (f->flags & 1) {
            if (w->room->type == 10 && w->subtype == 1) {
                w->kind = 0x42;
            } else {
                w->f8e &= 0xaf;
                if (fn_0002b400(w) ? 1 : (process_staff_complex(w), 0)) {
                    if (!shall_i_resign(w)) {
                        if (w->room->f22 || w->room->f3 == 4) {
                            process_staff_complex(w);
                        } else if ((w->f8e & 1) && take_a_break(w)) {
                        } else if ((w->subtype == 0 && allhosps[w->owner].unknown11f0 == 0) && could_nurse_vaccinate(w)) {
                            process_staff_complex(w);
                        } else {
                            if (w->room->f3 == 5) {
                                w->room->f3 = 6;
                            }
                            D_001553b8 = old_newrand() % 100;
                            if (D_001553b8 >= 0 && D_001553b8 < 25) {
                                if (w->room->f3 != 6 && (queue = w->room->queue_head)) {
                                    if (w->room->type == 10) {
                                        if (am_i_lead_staff(w)) {
                                            w->timer = 200;
                                            w->kind = 0x10;
                                        } else {
                                            e = fn_0003c154(w->room, 0x10);
                                            if (e > thing && call_person_to_room(e, queue, 0x28, w->room->unknown4, 1)) {
                                                u16 anim;

                                                w->kind = 0x18;
                                                set_icon_value(e, 0x19);
                                                anim = check_frame(staff_anim_starts[e->subtype].unknownA, e) + (u16)e->unknown17;
                                                e->sprite = start[anim].frame;
                                                e->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                                                e->kind = 0x14;
                                            }
                                        }
                                    } else if (call_person_to_room(w, queue, 0x28, w->room->unknown4, 1)) {
                                        u16 anim;

                                        set_icon_value(w, 0x19);
                                        anim = check_frame(staff_anim_starts[w->subtype].unknownA, w) + (u16)w->unknown17;
                                        w->sprite = start[anim].frame;
                                        w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                                        w->kind = 0x14;
                                    }
                                }
                            } else {
                                D_001553b8 = old_newrand() % 100;
                                if ((D_001553b8 >= 25 && D_001553b8 < 27)
                                    || ((s16)w->y.tile << 7) + (s16)w->x.tile == w->room->unknown6) {
                                    u16 tile;
                                    RoomInfo *r;
                                    u16 pos;

                                    tile = (u16)w->x.tile + ((u16)w->y.tile << 7);
                                    r = which_room(tile);
                                    D_001553b8 = old_newrand() % 100;
                                    if (D_001553b8 >= 25 && D_001553b8 < 26) {
                                        pos = ((s16)(r->f2 / 2) << 7) + (s16)(r->f1 / 2) + r->pos;
                                    } else {
                                        pos = ((s16)(old_newrand() % r->f2) << 7) + (s16)(old_newrand() % r->f1) + r->pos;
                                    }
                                    if (tile != pos && can_i_get_to(w, pos)) {
                                        w->state_b = w->state_a = 0x24;
                                        set_complex(w, 0xbe, pos);
                                        fn_0003e724(w);
                                    }
                                } else if (w->room->unknown29) {
                                    u8 paged;

                                    paged = 0;
                                    if (w->f78 > allhosps[w->owner].f21d && take_a_break(w)) {
                                        paged = 1;
                                    } else if ((allhosps[w->owner].f21f && w->room->queue_head == 0)
                                               && (actual_game_turn % (w->f78 > allhosps[w->owner].f21d
                                                                          ? w->unknown141 * 2 + 1
                                                                          : w->unknown141 + 1) == 0
                                               && obey_pager(w))) {
                                        paged = 1;
                                    }
                                    if (paged && w->room->queue_head) {
                                        page_member_of_staff(rooms[w->room->type].unknown8, w->room - room_info, 0, 1, w->owner);
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
void doctor_researches(Thing *w)
{
    u16 i;
    u16 ok;
    u16 obj;
    u16 queue;
    CompFrame *f;
    Thing *o;
    Thing *e;

    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
        f = &frame[w->sprite];
        if (f->flags & 1) {
            if (!(w->unknown124 & 0x100)) {
                process_staff_complex(w);
            } else if (!shall_i_resign(w)) {
                w->f8e |= 0x10;
                if (w->room->f22 || w->room->f3 == 4) {
                    process_staff_complex(w);
                } else if (w->f8e & 8) {
                    w->f8e &= 0xf0;
                    process_staff_complex(w);
                } else if (((w->f8e & 1) || w->f78 > allhosps[w->owner].f21d) && take_a_break(w)) {
                } else {
                    D_001553b8 = old_newrand() % 100;
                    if (D_001553b8 >= 0 && D_001553b8 < 25) {
                        queue = w->room->queue_head;
                        if (queue) {
                            ok = 1;
                            for (e = &thing[w->room->first]; e > thing; e = &thing[e->room_next]) {
                                if (e->renderer == 4 && e->unknown73) {
                                    ok = 0;
                                } else if (e->renderer == 2) {
                                    ok = 0;
                                }
                            }
                            if (ok) {
                                if (call_person_to_room(w, queue, 0x28, w->room->unknown4, 1)) {
                                    set_icon_value(w, 0x19);
                                    w->kind = 0x14;
                                    return;
                                }
                            }
                        }
                    } else {
                        for (i = 0; i < 4; i++) {
                            obj = find_object((u16)w->x.tile + ((u16)w->y.tile << 7), research_tasks[i].object, 1, 0x96, w->owner);
                            if (obj) {
                                o = fn_00019aac(&map[thing[obj].room_prev], 4);
                                if (!(o == 0 || o == w)) {
                                    continue;
                                }
                                if (old_newrand() % research_tasks[i].chance == 0) {
                                    w->state_a = 0x3e;
                                    w->state_b = 0x3e;
                                    set_complex(w, research_tasks[i].action, 0);
                                    fn_0003e724(w);
                                    return;
                                }
                            }
                        }
                    }
                    overlapping_staff_check(w, 0x3e);
                }
            }
        }
    }
}

// FUNCTION: 0x0003bd34
void doctor_cracks_up(Thing *w)
{
    u32 mean;
    u32 spread;

    if (w->subtype == 1) {
        if (w->subtype == 1 && w->unknown148[2]) {
            return;
        }
        if (w->unknown30[4] == 1) {
            w->unknown30[4] = 3;
        } else if (w->unknown30[4] == 2) {
            w->unknown30[4] = 4;
        }
        w->unknown148[2] = 1;
        mean = gbv.f156;
        spread = gbv.f158;
        if (w->unknown124 & 0x80) {
            mean = gbv.f158;
            spread = 1000;
        } else if (w->unknown124 & 8) {
            mean = 30;
            spread = gbv.f156;
        }
        w->unknown126 = old_normal_rand(mean, spread);
        if (w->owner == player) {
            adcounts.unknown8e++;
        }
    }
}

// FUNCTION: 0x0003be38
u8 count_surgeons(RoomInfo *room)
{
    Thing *e;
    u8 count;

    count = 0;
    for (e = &thing[room->first]; e > thing; e = &thing[e->room_next]) {
        if (e->renderer == 4 && e->subtype == 4) {
            count++;
        }
    }
    return count;
}

// FUNCTION: 0x0003bed0
void doctor_in_theatre(Thing *w)
{
    u8 count;
    u16 obj;
    u16 pos;
    RoomInfo *room;

    room = w->room;
    count = count_surgeons(w->room);
    w->unknown69 = 0;
    if (!(w->unknown124 & 0x20)) {
        process_staff_complex(w);
        return;
    }
    switch (count) {
    case 0:
        obj = find_object(((u16)w->y.tile << 7) + (u16)w->x.tile, 0x23, 0, 0x96, w->owner);
        if (obj) {
            set_complex(w, 0x4d8, 0);
            w->state_a = 0x42;
            w->state_b = 0x24;
            fn_0003e724(w);
            return;
        }
        break;
    case 1:
        obj = find_object(((u16)w->y.tile << 7) + (u16)w->x.tile, 0x23, 0, 0x96, w->owner);
        if (obj) {
            set_complex(w, 0x4ec, 0);
            w->state_a = 0x42;
            w->state_b = 0x24;
            fn_0003e724(w);
            return;
        }
        break;
    default:
        process_staff_complex(w);
        return;
    }
    pos = (s16)(old_newrand() % room->f1) + ((s16)(old_newrand() % room->f2) << 7) + room->pos;
    if (can_i_get_to(w, pos)) {
        w->state_b = 0x42;
        set_complex(w, 0xbe, pos);
        fn_0003e724(w);
    }
}

// FUNCTION: 0x0003c0a8
void change_surgeon_to_doc(Thing *w)
{
    u8 count;

    count = count_surgeons(w->room);
    if (count == 1) {
        w->state_a = 0x36;
        set_complex(w, 0x3ca, 0);
    } else if (count == 2) {
        w->state_a = 0x36;
        set_complex(w, 0x3d4, 0);
    } else {
        w->subtype = 1;
    }
}

// FUNCTION: 0x0003c11c
void surgeon_waits(Thing *w)
{
    w->timer--;
    if (w->timer <= 0) {
        w->kind = 0x24;
    }
}

// FUNCTION: 0x0003c154
Thing *fn_0003c154(RoomInfo *room, u8 kind)
{
    Thing *e;

    for (e = &thing[room->first]; e > thing; e = &thing[e->room_next]) {
        if (e->renderer == 4 && e->kind == kind) {
            return e;
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
