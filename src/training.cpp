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


u8 fn_0002b400(Thing *w);

}

// FUNCTION: 0x00042a10
void doctor_is_trained(Thing *doctor)
{
    RoomInfo *room;
    u8 x;
    u8 y;

    doctor->f8e &= ~4;
    if (doctor->unknown124 & 0x80) {
        doctor->kind = 0x48;
        return;
    }
    if (shall_i_resign(doctor)) {
        return;
    }
    doctor->f8e |= 0x40;
    if (doctor->room->f22 || doctor->room->f3 == 4) {
        process_staff_complex(doctor);
        return;
    }
    if (check_training_command(doctor)) {
        return;
    }
    if (doctor->f8e & 8) {
        doctor->f8e &= 0xf0;
        process_staff_complex(doctor);
        return;
    }
    if (((doctor->f8e & 1) || doctor->f78 > allhosps[doctor->owner].f21d) && take_a_break(doctor)) {
        return;
    }
    if (find_object(((u16)doctor->y.tile << 7) + (u16)doctor->x.tile, 0x24, 1, 0x96, doctor->owner) && doctor->icon != 0x41a) {
        doctor->state_b = 0x46;
        doctor->state_a = 0x46;
        set_complex(doctor, 0x41a, 0);
        fn_0003e724(doctor);
        return;
    }
    if (doctor->icon == 0x41a) {
        return;
    }
    room = doctor->room;
    x = old_newrand() % room->f1 + room->pos % 128;
    y = old_newrand() % room->f2 + room->pos / 128;
    if (map[((s16)y << 7) + (s16)x].first == 0 && can_i_get_to(doctor, (s16)(x + (y << 7)))) {
        doctor->state_b = 0x46;
        doctor->state_a = 0x46;
        set_complex(doctor, 0xbe, (s16)(x + (y << 7)));
        fn_0003e724(doctor);
    }
}

// FUNCTION: 0x00042c90
void consultant_trains(Thing *consultant)
{
    Thing *student;

    consultant->f8e &= ~4;
    if (shall_i_resign(consultant)) {
        return;
    }
    if (consultant->room->f22 || consultant->room->f3 == 4) {
        process_staff_complex(consultant);
        return;
    }
    if (check_training_command(consultant)) {
        return;
    }
    if (consultant->f8e & 8) {
        consultant->f8e &= 0xf0;
        process_staff_complex(consultant);
        return;
    }
    if (!(fn_0002b400(consultant) ? 1 : (process_staff_complex(consultant), 0))) {
        return;
    }
    consultant->f8e |= 0x40;
    if (((consultant->f8e & 1) || consultant->f78 > allhosps[consultant->owner].f21d) && take_a_break(consultant)) {
        return;
    }
    student = &thing[find_object(consultant->x.tile + ((u16)consultant->y.tile << 7), 0x25, 0, 0x97, consultant->owner)];
    if (student <= thing) {
        return;
    }
    if (!student->label[0]) {
        student->label[0] = 1;
        set_complex(consultant, 0x3fc, 0);
        consultant->state_b = 0x48;
        fn_0003e724(consultant);
        return;
    }
    if (old_newrand() % 1000 < 100 && find_object(((u16)consultant->y.tile << 7) + (u16)consultant->x.tile, 0x38, 1, 0x96, consultant->owner)) {
        training_done(consultant, 2);
        consultant->state_b = 0x48;
        consultant->state_a = 0x48;
        set_complex(consultant, 0x370, 0);
        fn_0003e724(consultant);
        return;
    }
    if (old_newrand() % 1000 < 100 && find_object(((u16)consultant->y.tile << 7) + (u16)consultant->x.tile, 0x3c, 1, 0x96, consultant->owner)) {
        training_done(consultant, 1);
        consultant->state_b = 0x48;
        consultant->state_a = 0x48;
        set_complex(consultant, 0x410, 0);
        fn_0003e724(consultant);
        return;
    }
    if (consultant->icon != 0x3de && consultant->icon != 0x3e8) {
        consultant->state_b = 0x48;
        consultant->state_a = 0x36;
        set_complex(consultant, 0x3de, 0);
        fn_0003e724(consultant);
        return;
    }
    training_done(consultant, 0);
    consultant->unknown61 = 0;
    consultant->unknown63 = 0;
    consultant->unknown75w = 0;
    consultant->unknown65 = 0;
    consultant->state_b = 0x48;
    set_complex(consultant, 0x3e8, 0);
    fn_0003e724(consultant);
}

// FUNCTION: 0x00042ff0
u8 check_training_command(Thing *doctor)
{
    if ((doctor->f8e & 4) && send_doctor_to_training(doctor)) {
        doctor->f8e &= 0xf0;
        return 1;
    }
    return 0;
}

// FUNCTION: 0x0004303c
u8 send_doctor_to_training(Thing *doctor)
{
    u8 state;
    u16 room;
    u16 current_room;

    state = 0;
    room = 0;
    current_room = 0;
    if (doctor->room) {
        current_room = doctor->room - room_info;
    }
    if (doctor->unknown124 & 0x80) {
        room = find_training_room_no_consultant(doctor->owner, current_room);
        state = 0x48;
    } else {
        room = find_emptiest_training_room(doctor->owner, current_room);
        state = 0x46;
    }
    if (state && room) {
    staff_exit_room_check(doctor);
    if (doctor->room) {
        set_complex(doctor, 0x136, doctor->room->unknown6);
    }
    doctor->state_b = state;
    doctor->state_a = 0x36;
    set_complex(doctor, 0x96, room_info[room].unknown4);
    doctor->unknown89 = room;
    fn_0003e724(doctor);
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
            Thing *occupant;

            free = 1;
            for (occupant = &thing[room->first]; occupant > thing; occupant = &thing[occupant->room_next]) {
                if (occupant->renderer == 4 && occupant->subtype == 1 && (occupant->unknown124 & 0x80)) {
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
void training_done(const Thing *consultant, const u8 kind)
{
    u32 base;
    u8 count;
    u32 students;
    u32 points;
    u32 limit;
    u32 skills;
    Thing *student;
    u32 learn;

    base = gbv.ff6[kind];
    count = 1;
    students = 0;
    points = 0;
    limit = old_newrand() % consultant->unknown126;
    skills = (u16)(consultant->unknown124 & 0x160);
    for (student = &thing[consultant->room->first]; student > thing; student = &thing[student->room_next]) {
        if (student->renderer == 4 && student->subtype == 1 && !(student->unknown124 & 0x80)
            && (TRAINING_OBJECT(student) && thing[TRAINING_OBJECT(student)].subtype == 0x24)) {
            students++;
        }
    }
    students = (students >> 2) + 1;
    for (student = &thing[consultant->room->first]; student > thing; student = &thing[student->room_next]) {
        if (student->renderer == 4 && student->subtype == 1 && !(student->unknown124 & 0x80)
            && (TRAINING_OBJECT(student) && thing[TRAINING_OBJECT(student)].subtype == 0x24)) {
            learn = skills & ~(u16)(student->unknown124 & 0x160);
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
            if (student->unknown126 < limit) {
                if (gbv.f141 / (students * count) > 0) {
                    student->unknown126 += gbv.f141 / (count * students);
                    if (student->unknown126 < 30) {
                        student->unknown126 = 30;
                    } else if (student->unknown126 > 1000) {
                        student->unknown126 = 1000;
                    }
                } else {
                    student->unknown126++;
                    if (student->unknown126 < 30) {
                        student->unknown126 = 30;
                    } else if (student->unknown126 > 1000) {
                        student->unknown126 = 1000;
                    }
                }
                points = base / (students * count);
                if (points == 0) {
                    points = 1;
                }
                if (learn & 0x20) {
                    student->unknown148[3] += points;
                    if (student->unknown148[3] > gbv.f146) {
                        student->unknown124 |= 0x20;
                        allhosps[student->owner].fca6++;
                        if (student->owner == player) {
                            announce_this(4, 5);
                        }
                        set_expertise_staff(student);
                        insert_skill(0x20, student->owner);
                    }
                }
                if (learn & 0x40) {
                    student->unknown148[4] += points;
                    if (student->unknown148[4] > gbv.f148) {
                        student->unknown124 |= 0x40;
                        if (student->owner == player) {
                            announce_this(4, 6);
                        }
                        set_expertise_staff(student);
                        insert_skill(0x40, student->owner);
                    }
                }
                if (learn & 0x100) {
                    student->unknown148[5] += points;
                    if (student->unknown148[5] > gbv.f14a) {
                        student->unknown124 |= 0x100;
                        if (student->owner == player) {
                            announce_this(4, 8);
                        }
                        set_expertise_staff(student);
                        insert_skill(0x100, student->owner);
                    }
                }
            }
        }
    }
}
