#include "hosp.h"

extern s16 opposite_angle[];

u16 find_free_object_anywhere(u16 id, u8 kind);
void staff_in_corridor(Thing *w);
RoomInfo *allocate_reception_roominfo(u8 owner);

// FUNCTION: 0x00097ce0
void receptionist_finds_reception(Thing *w)
{
    u16 pos;
    u16 id;
    Thing *desk;

    if (listhead[w->owner].rooms[4]) {
        id = find_free_object_anywhere(w - thing, 0xb);
        if (id) {
            desk = &thing[id];
            if (desk > thing) {
                pos = desk->room_prev;
                reset_icon_value(w, 0x19);
                w->state_b = 0x4a;
                w->unknown69 = id;
                set_complex(w, 0x50, pos);
                w->unknown94 = 0xc;
                fn_0003e724(w);
                w->unknown73 = 0;
            }
        } else {
            staff_in_corridor(w);
        }
    } else {
        staff_in_corridor(w);
    }
}

// FUNCTION: 0x00097de0
void receptionist_setup(Thing *w)
{
    Thing *o;
    Thing *desk;
    RoomInfo *room;

    room = 0;
    for (o = &thing[listhead[w->owner].objects[0xb]]; o > thing; o = &thing[o->unknown75w]) {
        if ((u16)(o - thing) != w->unknown69) {
            if (o->unknown98 != 0) {
                if ((o->x.tile - w->x.tile < 0 ? -(o->x.tile - w->x.tile) : (o->x.tile - w->x.tile)) <= 10) {
                    if ((o->y.tile - w->y.tile < 0 ? -(o->y.tile - w->y.tile) : (o->y.tile - w->y.tile)) <= 10) {
                        desk = &thing[o->unknown91];
                        if (desk > thing) {
                            room = desk->room;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (room)
        w->room = room;
    else
        w->room = allocate_reception_roominfo(w->owner);
    w->kind = 10;
}

// FUNCTION: 0x00097fac
void receptionist_works(Thing *w)
{
    Thing *desk;
    u16 action;
    u16 id;
    u16 capacity;
    u16 pos;
    CompFrame *fr;
    u16 kind;

    if (w->room > room_info) {
        if (actual_game_turn % w->anim_speed == 0) {
            w->sprite = frame[w->sprite].next;
            fr = &frame[w->sprite];
            if (fr->flags & 1) {
                id = w->room->queue_head;
                if (id) {
                    for (; id; id = thing[id].queue_next) {
                        if (thing[id].flags & 0x40)
                            break;
                    }
                    if (id == 0)
                        id = w->room->queue_head;
                    for (; id; id = thing[id].queue_next) {
                        kind = thing[id].unknown171;
                        capacity = all_queue_capacity(kind, 1, w->owner);
                        if (capacity > 0) {
                            desk = &thing[w->unknown69];
                            if (desk > thing) {
                                pos = desk->room_next;
                                if (call_person_to_room(w, id, 0x6e, pos, 0)) {
                                    set_icon_value(w, 0x19);
                                    w->state_b = 0x20;
                                    set_complex(w, 0x1e, 0);
                                    fn_0003e724(w);
                                    break;
                                }
                            }
                        }
                    }
                } else {
                    reset_icon_value(w, 0x19);
                    othervar.f0 = old_newrand() % 100;
                    if (othervar.f0 >= 0 && othervar.f0 < 30) {
                        w->state_b = 10;
                        action = recept_tasks[old_newrand() % recept_tasks[0] + 1];
                        set_complex(w, action, 0);
                        fn_0003e724(w);
                    } else {
                        u16 anim;

                        anim = check_frame(staff_anim_starts[w->subtype].anims[2], w) + w->unknown17;
                        w->sprite = start[anim].frame;
                        w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                        w->kind = 10;
                    }
                }
            }
        }
    }
}

// FUNCTION: 0x000982a8
void recept_waits_for_patient(Thing *w)
{
    u8 result;
    u16 pos;
    Thing *desk;
    Thing *p;

    p = &thing[w->unknown73];
    if (p > thing) {
        if (p->kind == 0x46) {
            if (allhosps[p->owner].rooms[p->unknown171].unknown1C == 1)
                set_complex(w, 0xe6, 0);
            else
                set_complex(w, 0xf0, 0);
            p->unknown17 = opposite_angle[w->unknown17];
            w->state_b = 0x22;
            fn_0003e724(w);
            othervar.f8ac_u16a[1] = 0;
        } else {
            w->timer--;
            if (w->timer <= 0) {
                desk = &thing[w->unknown69];
                if (desk > thing) {
                    pos = desk->room_next;
                    reset_icon_value(w, 0x19);
                    result = call_person_again(w, 0x6e, pos);
                    if (result == 0) {
                        w->kind = 10;
                        w->unknown73 = 0;
                    } else if (result == 1) {
                        w->kind = 10;
                        interrupt_person(p);
                        start_person_going(p);
                        w->unknown73 = 0;
                        othervar.f8ac_u16a[1] = 0;
                    }
                }
            } else if (fn_0003c1e8(w, 0x6e)) {
                w->kind = 10;
                w->unknown73 = 0;
            }
        }
    } else {
        reset_icon_value(w, 0x19);
        w->unknown73 = 0;
        w->kind = 10;
        othervar.f8ac_u16a[1] = 0;
    }
}

// FUNCTION: 0x000984a0
void recept_admits_patient(Thing *w)
{
    Thing *p;

    p = &thing[w->unknown73];
    if (p > thing) {
        p->unknown124 = 0;
        p->unknown126 = 0;
        if (shall_peep_pay(p))
            pay_for_treatment(p, 3);
        else
            p->unknown171 = 0;
        p->kind = 0x26;
        w->room->f3 = 0;
        w->room->count++;
        w->unknown73 = 0;
        reset_icon_value(w, 0x19);
        w->kind = 10;
    }
}

// FUNCTION: 0x00098554
void receptionist_reset(Thing *w)
{
    u8 found;
    Thing *desk;
    RoomInfo *room;
    Thing *o;

    room = 0;
    found = 0;
    if (w->room && w->room->type == 4) {
        for (o = &thing[listhead[w->owner].objects[0xb]]; o > thing; o = &thing[o->unknown75w]) {
            if ((u16)(o - thing) != w->unknown69) {
                if (o->unknown98 != 0) {
                    desk = &thing[o->unknown91];
                    if (desk > thing) {
                        if (w->room == desk->room) {
                            found = 1;
                            break;
                        }
                    }
                }
            }
        }
        if (found == 0) {
            room = w->room;
            restart_all_queue_peeps(room);
            memset(room, 0, 0x23);
        }
        w->room = 0;
    }
}
