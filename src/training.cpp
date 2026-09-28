#include "hosp.h"

u8 check_training_command(Thing *w);
u8 send_doctor_to_training(Thing *w);
u16 find_training_room_no_consultant(u8 owner, u16 exclude);
u16 find_emptiest_training_room(u8 owner, u16 exclude);
void training_done(const Thing *w, const u8 kind);
void staff_exit_room_check(Thing *w);
void set_expertise_staff(Thing *w);
void insert_skill(u16 skill, u8 owner);

#define TRAINING_OBJECT(t) \
    ((t)->unknown69 && (thing[(t)->unknown69].renderer == 6 || thing[(t)->unknown69].renderer == 8) \
        && (u16)((t) - thing) == thing[(t)->unknown69].unknown91 ? (u32)(t)->unknown69 : 0)

extern "C" {

extern u16 D_0012dd76[];
extern u16 D_0012ddc1;
extern u16 D_0012ddc6;
extern u16 D_0012ddc8;
extern u16 D_0012ddca;

u8 fn_0002b400(Thing *w);

}

// FUNCTION: 0x00042a10
void doctor_is_trained(Thing *w)
{
    RoomInfo *room;
    u8 x;
    u8 y;

    w->f8e &= ~4;
    if (w->unknown124 & 0x80) {
        w->kind = 0x48;
        return;
    }
    if (shall_i_resign(w)) {
        return;
    }
    w->f8e |= 0x40;
    if (w->room->f22 || w->room->f3 == 4) {
        process_staff_complex(w);
        return;
    }
    if (check_training_command(w)) {
        return;
    }
    if (w->f8e & 8) {
        w->f8e &= 0xf0;
        process_staff_complex(w);
        return;
    }
    if (((w->f8e & 1) || w->f78 > allhosps[w->owner].f21d) && take_a_break(w)) {
        return;
    }
    if (find_object(((u16)w->y.tile << 7) + (u16)w->x.tile, 0x24, 1, 0x96, w->owner) && w->icon != 0x41a) {
        w->state_b = 0x46;
        w->state_a = 0x46;
        set_complex(w, 0x41a, 0);
        fn_0003e724(w);
        return;
    }
    if (w->icon == 0x41a) {
        return;
    }
    room = w->room;
    x = old_newrand() % room->f1 + room->pos % 128;
    y = old_newrand() % room->f2 + room->pos / 128;
    if (map[((s16)y << 7) + (s16)x].first == 0 && can_i_get_to(w, (s16)(x + (y << 7)))) {
        w->state_b = 0x46;
        w->state_a = 0x46;
        set_complex(w, 0xbe, (s16)(x + (y << 7)));
        fn_0003e724(w);
    }
}

// FUNCTION: 0x00042c90
void consultant_trains(Thing *w)
{
    Thing *student;

    w->f8e &= ~4;
    if (shall_i_resign(w)) {
        return;
    }
    if (w->room->f22 || w->room->f3 == 4) {
        process_staff_complex(w);
        return;
    }
    if (check_training_command(w)) {
        return;
    }
    if (w->f8e & 8) {
        w->f8e &= 0xf0;
        process_staff_complex(w);
        return;
    }
    if (!(fn_0002b400(w) ? 1 : (process_staff_complex(w), 0))) {
        return;
    }
    w->f8e |= 0x40;
    if (((w->f8e & 1) || w->f78 > allhosps[w->owner].f21d) && take_a_break(w)) {
        return;
    }
    student = &thing[find_object(w->x.tile + ((u16)w->y.tile << 7), 0x25, 0, 0x97, w->owner)];
    if (student <= thing) {
        return;
    }
    if (!student->label[0]) {
        student->label[0] = 1;
        set_complex(w, 0x3fc, 0);
        w->state_b = 0x48;
        fn_0003e724(w);
        return;
    }
    if (old_newrand() % 1000 < 100 && find_object(((u16)w->y.tile << 7) + (u16)w->x.tile, 0x38, 1, 0x96, w->owner)) {
        training_done(w, 2);
        w->state_b = 0x48;
        w->state_a = 0x48;
        set_complex(w, 0x370, 0);
        fn_0003e724(w);
        return;
    }
    if (old_newrand() % 1000 < 100 && find_object(((u16)w->y.tile << 7) + (u16)w->x.tile, 0x3c, 1, 0x96, w->owner)) {
        training_done(w, 1);
        w->state_b = 0x48;
        w->state_a = 0x48;
        set_complex(w, 0x410, 0);
        fn_0003e724(w);
        return;
    }
    if (w->icon != 0x3de && w->icon != 0x3e8) {
        w->state_b = 0x48;
        w->state_a = 0x36;
        set_complex(w, 0x3de, 0);
        fn_0003e724(w);
        return;
    }
    training_done(w, 0);
    w->unknown61 = 0;
    w->unknown63 = 0;
    w->unknown75w = 0;
    w->unknown65 = 0;
    w->state_b = 0x48;
    set_complex(w, 0x3e8, 0);
    fn_0003e724(w);
}

// FUNCTION: 0x00042ff0
u8 check_training_command(Thing *w)
{
    if ((w->f8e & 4) && send_doctor_to_training(w)) {
        w->f8e &= 0xf0;
        return 1;
    }
    return 0;
}

// FUNCTION: 0x0004303c
u8 send_doctor_to_training(Thing *w)
{
    u8 state;
    u16 room;
    u16 current;

    state = 0;
    room = 0;
    current = 0;
    if (w->room) {
        current = w->room - room_info;
    }
    if (w->unknown124 & 0x80) {
        room = find_training_room_no_consultant(w->owner, current);
        state = 0x48;
    } else {
        room = find_emptiest_training_room(w->owner, current);
        state = 0x46;
    }
    if (state && room) {
    staff_exit_room_check(w);
    if (w->room) {
        set_complex(w, 0x136, w->room->unknown6);
    }
    w->state_b = state;
    w->state_a = 0x36;
    set_complex(w, 0x96, room_info[room].unknown4);
    w->unknown89 = room;
    fn_0003e724(w);
    return 1;
    }
    return 0;
}

// FUNCTION: 0x00043168
u16 find_training_room_no_consultant(u8 owner, u16 exclude)
{
    RoomInfo *room;

    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->type == 0x16 && room->owner == owner && (u16)(room - room_info) != exclude) {
            u8 free;
            Thing *t;

            free = 1;
            for (t = &thing[room->first]; t > thing; t = &thing[t->room_next]) {
                if (t->renderer == 4 && t->subtype == 1 && (t->unknown124 & 0x80)) {
                    free = 0;
                    break;
                }
            }
            if (free) {
                return room - room_info;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00043294
u16 find_emptiest_training_room(u8 owner, u16 exclude)
{
    s32 best;
    u16 found;
    RoomInfo *room;
    s32 space;
    Thing *t;

    best = -10000;
    found = 0;
    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->type == 0x16 && room->owner == owner && (u16)(room - room_info) != exclude) {
            space = 0;
            for (t = &thing[room->first_object]; t > thing; t = &thing[t->unknown71]) {
                if (t->subtype == 0x24) {
                    space++;
                }
            }
            for (t = &thing[room->first]; t > thing; t = &thing[t->room_next]) {
                if (t->renderer == 4 && t->subtype == 1 && !(t->unknown124 & 0x80)) {
                    space--;
                }
            }
            if (space > best) {
                best = space;
                found = room - room_info;
            }
        }
    }
    return found;
}

// FUNCTION: 0x00043434
void training_done(const Thing *w, const u8 kind)
{
    u32 base;
    u8 count;
    u32 students;
    u32 points;
    u32 limit;
    u32 skills;
    Thing *t;
    u32 learn;

    base = D_0012dd76[kind];
    count = 1;
    students = 0;
    points = 0;
    limit = old_newrand() % w->unknown126;
    skills = (u16)(w->unknown124 & 0x160);
    for (t = &thing[w->room->first]; t > thing; t = &thing[t->room_next]) {
        if (t->renderer == 4 && t->subtype == 1 && !(t->unknown124 & 0x80)
            && (TRAINING_OBJECT(t) && thing[TRAINING_OBJECT(t)].subtype == 0x24)) {
            students++;
        }
    }
    students = (students >> 2) + 1;
    for (t = &thing[w->room->first]; t > thing; t = &thing[t->room_next]) {
        if (t->renderer == 4 && t->subtype == 1 && !(t->unknown124 & 0x80)
            && (TRAINING_OBJECT(t) && thing[TRAINING_OBJECT(t)].subtype == 0x24)) {
            learn = skills & ~(u16)(t->unknown124 & 0x160);
            if (learn & 0x160) {
                if (learn & 0x20) {
                    count++;
                } else if (learn & 0x40) {
                    count++;
                } else if (learn & 0x100) {
                    count++;
                }
            } else if (learn == 0) {
            }
            if (t->unknown126 < limit) {
                if (D_0012ddc1 / (students * count) > 0) {
                    t->unknown126 += D_0012ddc1 / (count * students);
                    if (t->unknown126 < 30) {
                        t->unknown126 = 30;
                    } else if (t->unknown126 > 1000) {
                        t->unknown126 = 1000;
                    }
                } else {
                    t->unknown126++;
                    if (t->unknown126 < 30) {
                        t->unknown126 = 30;
                    } else if (t->unknown126 > 1000) {
                        t->unknown126 = 1000;
                    }
                }
                points = base / (students * count);
                if (points == 0) {
                    points = 1;
                }
                if (learn & 0x20) {
                    t->unknown148[3] += points;
                    if (t->unknown148[3] > D_0012ddc6) {
                        t->unknown124 |= 0x20;
                        allhosps[t->owner].fca6++;
                        if (t->owner == player) {
                            announce_this(4, 5);
                        }
                        set_expertise_staff(t);
                        insert_skill(0x20, t->owner);
                    }
                }
                if (learn & 0x40) {
                    t->unknown148[4] += points;
                    if (t->unknown148[4] > D_0012ddc8) {
                        t->unknown124 |= 0x40;
                        if (t->owner == player) {
                            announce_this(4, 6);
                        }
                        set_expertise_staff(t);
                        insert_skill(0x40, t->owner);
                    }
                }
                if (learn & 0x100) {
                    t->unknown148[5] += points;
                    if (t->unknown148[5] > D_0012ddca) {
                        t->unknown124 |= 0x100;
                        if (t->owner == player) {
                            announce_this(4, 8);
                        }
                        set_expertise_staff(t);
                        insert_skill(0x100, t->owner);
                    }
                }
            }
        }
    }
}
