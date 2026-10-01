#include "hosp.h"

extern s16 opposite_angle[];

u16 find_free_object_anywhere(u16 id, u8 kind);
void staff_in_corridor(Thing *w);
RoomInfo *allocate_reception_roominfo(u8 owner);

// FUNCTION: 0x00097ce0
void receptionist_finds_reception(Thing *receptionist)
{
    u16 pos;
    u16 id;
    Thing *desk;

    if (listhead[receptionist->owner].rooms[4]) {
        id = find_free_object_anywhere(receptionist - thing, 0xb);
        if (id) {
            desk = &thing[id];
            if (desk > thing) {
                pos = desk->room_prev;
                reset_icon_value(receptionist, 0x19);
                receptionist->state_b = 0x4a;
                receptionist->unknown69 = id;
                set_complex(receptionist, 0x50, pos);
                receptionist->unknown94 = 0xc;
                fn_0003e724(receptionist);
                receptionist->unknown73 = 0;
            }
        } else {
            staff_in_corridor(receptionist);
        }
    } else {
        staff_in_corridor(receptionist);
    }
}

// FUNCTION: 0x00097de0
void receptionist_setup(Thing *receptionist)
{
    Thing *other_desk;
    Thing *desk_user;
    RoomInfo *room;

    room = 0;
    for (other_desk = &thing[listhead[receptionist->owner].objects[0xb]]; other_desk > thing; other_desk = &thing[other_desk->unknown75w]) {
        if ((u16)(other_desk - thing) != receptionist->unknown69) {
            if (other_desk->unknown98 != 0) {
                if ((other_desk->x.tile - receptionist->x.tile < 0 ? -(other_desk->x.tile - receptionist->x.tile) : (other_desk->x.tile - receptionist->x.tile)) <= 10) {
                    if ((other_desk->y.tile - receptionist->y.tile < 0 ? -(other_desk->y.tile - receptionist->y.tile) : (other_desk->y.tile - receptionist->y.tile)) <= 10) {
                        desk_user = &thing[other_desk->unknown91];
                        if (desk_user > thing) {
                            room = desk_user->room;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (room)
        receptionist->room = room;
    else
        receptionist->room = allocate_reception_roominfo(receptionist->owner);
    receptionist->kind = 10;
}

// FUNCTION: 0x00097fac
void receptionist_works(Thing *receptionist)
{
    Thing *desk;
    u16 action;
    u16 id;
    u16 capacity;
    u16 pos;
    CompFrame *cur_frame;
    u16 kind;

    if (receptionist->room > room_info) {
        if (actual_game_turn % receptionist->anim_speed == 0) {
            receptionist->sprite = frame[receptionist->sprite].next;
            cur_frame = &frame[receptionist->sprite];
            if (cur_frame->flags & 1) {
                id = receptionist->room->queue_head;
                if (id) {
                    for (; id; id = thing[id].queue_next) {
                        if (thing[id].flags & 0x40)
                            break;
                    }
                    if (id == 0)
                        id = receptionist->room->queue_head;
                    for (; id; id = thing[id].queue_next) {
                        kind = thing[id].unknown171;
                        capacity = all_queue_capacity(kind, 1, receptionist->owner);
                        if (capacity > 0) {
                            desk = &thing[receptionist->unknown69];
                            if (desk > thing) {
                                pos = desk->room_next;
                                if (call_person_to_room(receptionist, id, 0x6e, pos, 0)) {
                                    set_icon_value(receptionist, 0x19);
                                    receptionist->state_b = 0x20;
                                    set_complex(receptionist, 0x1e, 0);
                                    fn_0003e724(receptionist);
                                    break;
                                }
                            }
                        }
                    }
                } else {
                    reset_icon_value(receptionist, 0x19);
                    othervar.f0 = old_newrand() % 100;
                    if (othervar.f0 >= 0 && othervar.f0 < 30) {
                        receptionist->state_b = 10;
                        action = recept_tasks[old_newrand() % recept_tasks[0] + 1];
                        set_complex(receptionist, action, 0);
                        fn_0003e724(receptionist);
                    } else {
                        u16 anim;

                        anim = check_frame(staff_anim_starts[receptionist->subtype].anims[2], receptionist) + receptionist->unknown17;
                        receptionist->sprite = start[anim].frame;
                        receptionist->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                        receptionist->kind = 10;
                    }
                }
            }
        }
    }
}

// FUNCTION: 0x000982a8
void recept_waits_for_patient(Thing *receptionist)
{
    u8 result;
    u16 pos;
    Thing *desk;
    Thing *patient;

    patient = &thing[receptionist->unknown73];
    if (patient > thing) {
        if (patient->kind == 0x46) {
            if (allhosps[patient->owner].rooms[patient->unknown171].unknown1C == 1)
                set_complex(receptionist, 0xe6, 0);
            else
                set_complex(receptionist, 0xf0, 0);
            patient->unknown17 = opposite_angle[receptionist->unknown17];
            receptionist->state_b = 0x22;
            fn_0003e724(receptionist);
            othervar.f8ac_u16a[1] = 0;
        } else {
            receptionist->timer--;
            if (receptionist->timer <= 0) {
                desk = &thing[receptionist->unknown69];
                if (desk > thing) {
                    pos = desk->room_next;
                    reset_icon_value(receptionist, 0x19);
                    result = call_person_again(receptionist, 0x6e, pos);
                    if (result == 0) {
                        receptionist->kind = 10;
                        receptionist->unknown73 = 0;
                    } else if (result == 1) {
                        receptionist->kind = 10;
                        interrupt_person(patient);
                        start_person_going(patient);
                        receptionist->unknown73 = 0;
                        othervar.f8ac_u16a[1] = 0;
                    }
                }
            } else if (fn_0003c1e8(receptionist, 0x6e)) {
                receptionist->kind = 10;
                receptionist->unknown73 = 0;
            }
        }
    } else {
        reset_icon_value(receptionist, 0x19);
        receptionist->unknown73 = 0;
        receptionist->kind = 10;
        othervar.f8ac_u16a[1] = 0;
    }
}

// FUNCTION: 0x000984a0
void recept_admits_patient(Thing *receptionist)
{
    Thing *patient;

    patient = &thing[receptionist->unknown73];
    if (patient > thing) {
        patient->unknown124 = 0;
        patient->unknown126 = 0;
        if (shall_peep_pay(patient))
            pay_for_treatment(patient, 3);
        else
            patient->unknown171 = 0;
        patient->kind = 0x26;
        receptionist->room->f3 = 0;
        receptionist->room->count++;
        receptionist->unknown73 = 0;
        reset_icon_value(receptionist, 0x19);
        receptionist->kind = 10;
    }
}

// FUNCTION: 0x00098554
void receptionist_reset(Thing *receptionist)
{
    u8 found;
    Thing *desk_user;
    RoomInfo *room;
    Thing *other_desk;

    room = 0;
    found = 0;
    if (receptionist->room && receptionist->room->type == 4) {
        for (other_desk = &thing[listhead[receptionist->owner].objects[0xb]]; other_desk > thing; other_desk = &thing[other_desk->unknown75w]) {
            if ((u16)(other_desk - thing) != receptionist->unknown69) {
                if (other_desk->unknown98 != 0) {
                    desk_user = &thing[other_desk->unknown91];
                    if (desk_user > thing) {
                        if (receptionist->room == desk_user->room) {
                            found = 1;
                            break;
                        }
                    }
                }
            }
        }
        if (found == 0) {
            room = receptionist->room;
            restart_all_queue_peeps(room);
            memset(room, 0, 0x23);
        }
        receptionist->room = 0;
    }
}
