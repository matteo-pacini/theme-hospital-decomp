#include "hosp.h"

u8 check_training_command(Thing *doctor);
u8 send_doctor_to_training(Thing *doctor);
u16 find_training_room_no_consultant(u8 owner, u16 exclude);
u16 find_emptiest_training_room(u8 owner, u16 exclude);
void training_done(const Thing *consultant, const u8 kind);
void staff_exit_room_check(Thing *staff);
void set_expertise_staff(Thing *staff);
void insert_skill(u16 mask, u8 owner);

#define TRAINING_OBJECT(t)                                                                                             \
    ((t)->object && (thing[(t)->object].type == THING_OBJECT || thing[(t)->object].type == THING_DOOR) &&              \
             (u16)((t) - thing) == thing[(t)->object].user                                                             \
         ? (u32)(t)->object                                                                                            \
         : 0)

extern "C" {

u8 check_room_mutex(Thing *staff);
}

// Staff state SS_DOCTOR_IS_TRAINED (a student in the training room): a
// consultant switches to teaching; otherwise leaves for a rest, new orders
// or an evacuation as needed, else sits on a free lecture chair or walks to
// a random spot. Marks them to return here after a rest.
// FUNCTION: 0x00042a10
void doctor_is_trained(Thing *doctor)
{
    RoomInfo *room;
    u8 x;
    u8 y;

    doctor->orders &= ~ORD_TRAIN;
    if (doctor->skills & SKILL_CONSULTANT) {
        doctor->state = SS_CONSULTANT_TRAINS;
        return;
    }
    if (shall_i_resign(doctor)) {
        return;
    }
    doctor->orders |= ORD_RETURN_TO_TRAINING;
    if (doctor->room->being_edited || doctor->room->state == RS_EDITED) {
        evacuate_staff(doctor);
        return;
    }
    if (check_training_command(doctor)) {
        return;
    }
    if (doctor->orders & ORD_LEAVE_ROOM) {
        doctor->orders &= 0xf0;
        evacuate_staff(doctor);
        return;
    }
    if (((doctor->orders & ORD_REST) || doctor->tiredness > allhosps[doctor->owner].rest_threshold) &&
        take_a_break(doctor)) {
        return;
    }
    if (find_object(((u16)doctor->y.tile << 7) + (u16)doctor->x.tile, OBJ_LECTURE_CHAIR, 1, 0x96, doctor->owner) &&
        doctor->complex_action != 0x41a) {
        doctor->next_state = SS_DOCTOR_IS_TRAINED;
        doctor->resume_state = SS_DOCTOR_IS_TRAINED;
        set_complex(doctor, 0x41a, 0);
        process_staff_complex(doctor);
        return;
    }
    if (doctor->complex_action == 0x41a) {
        return;
    }
    room = doctor->room;
    x = old_newrand() % room->width + room->pos % 128;
    y = old_newrand() % room->height + room->pos / 128;
    if (map[((s16)y << 7) + (s16)x].first == 0 && can_i_get_to(doctor, (s16)(x + (y << 7)))) {
        doctor->next_state = SS_DOCTOR_IS_TRAINED;
        doctor->resume_state = SS_DOCTOR_IS_TRAINED;
        set_complex(doctor, 0xbe, (s16)(x + (y << 7)));
        process_staff_complex(doctor);
    }
}

// Staff state SS_CONSULTANT_TRAINS: the consultant holds the room, starts the
// projector, and gives lectures (training_done kind 0), or now and then
// teaches from the bookcase (2) or skeleton (1); leaves for a rest, new
// orders or an evacuation as needed.
// FUNCTION: 0x00042c90
void consultant_trains(Thing *consultant)
{
    Thing *projector;

    consultant->orders &= ~ORD_TRAIN;
    if (shall_i_resign(consultant)) {
        return;
    }
    if (consultant->room->being_edited || consultant->room->state == RS_EDITED) {
        evacuate_staff(consultant);
        return;
    }
    if (check_training_command(consultant)) {
        return;
    }
    if (consultant->orders & ORD_LEAVE_ROOM) {
        consultant->orders &= 0xf0;
        evacuate_staff(consultant);
        return;
    }
    if (!(check_room_mutex(consultant) ? 1 : (evacuate_staff(consultant), 0))) {
        return;
    }
    consultant->orders |= ORD_RETURN_TO_TRAINING;
    if (((consultant->orders & ORD_REST) || consultant->tiredness > allhosps[consultant->owner].rest_threshold) &&
        take_a_break(consultant)) {
        return;
    }
    projector = &thing[find_object(consultant->x.tile + ((u16)consultant->y.tile << 7), OBJ_PROJECTOR, 0, 0x97,
                                   consultant->owner)];
    if (projector <= thing) {
        return;
    }
    if (!projector->lecture_started) {
        projector->lecture_started = 1;
        set_complex(consultant, 0x3fc, 0);
        consultant->next_state = SS_CONSULTANT_TRAINS;
        process_staff_complex(consultant);
        return;
    }
    if (old_newrand() % 1000 < 100 && find_object(((u16)consultant->y.tile << 7) + (u16)consultant->x.tile,
                                                  OBJ_BOOKCASE, 1, 0x96, consultant->owner)) {
        training_done(consultant, 2);
        consultant->next_state = SS_CONSULTANT_TRAINS;
        consultant->resume_state = SS_CONSULTANT_TRAINS;
        set_complex(consultant, 0x370, 0);
        process_staff_complex(consultant);
        return;
    }
    if (old_newrand() % 1000 < 100 && find_object(((u16)consultant->y.tile << 7) + (u16)consultant->x.tile,
                                                  OBJ_SKELETON, 1, 0x96, consultant->owner)) {
        training_done(consultant, 1);
        consultant->next_state = SS_CONSULTANT_TRAINS;
        consultant->resume_state = SS_CONSULTANT_TRAINS;
        set_complex(consultant, 0x410, 0);
        process_staff_complex(consultant);
        return;
    }
    if (consultant->complex_action != 0x3de && consultant->complex_action != 0x3e8) {
        consultant->next_state = SS_CONSULTANT_TRAINS;
        consultant->resume_state = SS_EVACUATE;
        set_complex(consultant, 0x3de, 0);
        process_staff_complex(consultant);
        return;
    }
    training_done(consultant, 0);
    consultant->complex_pc = 0;
    consultant->complex_queued = 0;
    consultant->complex_queued2 = 0;
    consultant->complex_resume = 0;
    consultant->next_state = SS_CONSULTANT_TRAINS;
    set_complex(consultant, 0x3e8, 0);
    process_staff_complex(consultant);
}

// If doctor has been ordered to train (ORD_TRAIN) and a training room is
// available, sends them there and clears the order. Returns 1 if sent.
// FUNCTION: 0x00042ff0
u8 check_training_command(Thing *doctor)
{
    if ((doctor->orders & ORD_TRAIN) && send_doctor_to_training(doctor)) {
        doctor->orders &= 0xf0;
        return 1;
    }
    return 0;
}

// Sends doctor to a training room other than their current one: a
// consultant to one without a consultant (to teach), others to the one
// with the most free lecture chairs. Returns 1 if sent, 0 if there is none.
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
    if (doctor->skills & SKILL_CONSULTANT) {
        room = find_training_room_no_consultant(doctor->owner, current_room);
        state = SS_CONSULTANT_TRAINS;
    } else {
        room = find_emptiest_training_room(doctor->owner, current_room);
        state = SS_DOCTOR_IS_TRAINED;
    }
    if (state && room) {
        staff_exit_room_check(doctor);
        if (doctor->room) {
            set_complex(doctor, 0x136, doctor->room->door_inside_tile);
        }
        doctor->next_state = state;
        doctor->resume_state = SS_EVACUATE;
        set_complex(doctor, 0x96, room_info[room].door_tile);
        doctor->dest_room = room;
        process_staff_complex(doctor);
        return 1;
    }
    return 0;
}

// Returns a training room of player `owner` other than exclude with no
// consultant in it, or 0.
// FUNCTION: 0x00043168
u16 find_training_room_no_consultant(u8 owner, u16 exclude)
{
    RoomInfo *room;

    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->type == ROOM_TRAINING && room->owner == owner && (u16)(room - room_info) != exclude) {
            u8 free;
            Thing *occupant;

            free = 1;
            for (occupant = &thing[room->first]; occupant > thing; occupant = &thing[occupant->room_next]) {
                if (occupant->type == THING_STAFF && occupant->subtype == STAFF_DOCTOR &&
                    (occupant->skills & SKILL_CONSULTANT)) {
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

// Returns player `owner`'s training room other than exclude with the most
// lecture chairs left free by students, or 0 if none.
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
        if (room->type == ROOM_TRAINING && room->owner == owner && (u16)(room - room_info) != exclude) {
            space = 0;
            for (t = &thing[room->first_object]; t > thing; t = &thing[t->room_obj_next]) {
                if (t->subtype == OBJ_LECTURE_CHAIR) {
                    space++;
                }
            }
            for (t = &thing[room->first]; t > thing; t = &thing[t->room_next]) {
                if (t->type == THING_STAFF && t->subtype == STAFF_DOCTOR && !(t->skills & SKILL_CONSULTANT)) {
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

// A lesson of kind `kind` (0 lecture, 1 skeleton, 2 bookcase) by consultant:
// each seated student whose ability is below a random part of the
// consultant's raises it, and gains training points towards the
// consultant's surgeon, psychiatrist and researcher skills they lack,
// getting each skill (announced for this player) once past its threshold.
// More students share the gain.
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

    base = gbv.training_value[kind];
    count = 1;
    students = 0;
    points = 0;
    limit = old_newrand() % consultant->ability;
    skills = (u16)(consultant->skills & (SKILL_SURGEON | SKILL_PSYCHIATRIST | SKILL_RESEARCHER));
    for (student = &thing[consultant->room->first]; student > thing; student = &thing[student->room_next]) {
        if (student->type == THING_STAFF && student->subtype == STAFF_DOCTOR && !(student->skills & SKILL_CONSULTANT) &&
            (TRAINING_OBJECT(student) && thing[TRAINING_OBJECT(student)].subtype == OBJ_LECTURE_CHAIR)) {
            students++;
        }
    }
    students = (students >> 2) + 1;
    for (student = &thing[consultant->room->first]; student > thing; student = &thing[student->room_next]) {
        if (student->type == THING_STAFF && student->subtype == STAFF_DOCTOR && !(student->skills & SKILL_CONSULTANT) &&
            (TRAINING_OBJECT(student) && thing[TRAINING_OBJECT(student)].subtype == OBJ_LECTURE_CHAIR)) {
            learn = skills & ~(u16)(student->skills & (SKILL_SURGEON | SKILL_PSYCHIATRIST | SKILL_RESEARCHER));
            if (learn & (SKILL_SURGEON | SKILL_PSYCHIATRIST | SKILL_RESEARCHER)) {
                if (learn & 0x20) {
                    count++;
                } else if (learn & 0x40) {
                    count++;
                } else if (learn & 0x100) {
                    count++;
                }
            } else if (learn == 0) {
            }
            if (student->ability < limit) {
                if (gbv.training_rate / (students * count) > 0) {
                    student->ability += gbv.training_rate / (count * students);
                    if (student->ability < 30) {
                        student->ability = 30;
                    } else if (student->ability > 1000) {
                        student->ability = 1000;
                    }
                } else {
                    student->ability++;
                    if (student->ability < 30) {
                        student->ability = 30;
                    } else if (student->ability > 1000) {
                        student->ability = 1000;
                    }
                }
                points = base / (students * count);
                if (points == 0) {
                    points = 1;
                }
                if (learn & 0x20) {
                    student->surgeon_training += points;
                    if (student->surgeon_training > gbv.ability_threshold_surgeon) {
                        student->skills |= SKILL_SURGEON;
                        allhosps[student->owner].surgeon_count++;
                        if (student->owner == player) {
                            announce_this(ANN_PROMOTED, 5);
                        }
                        set_expertise_staff(student);
                        insert_skill(0x20, student->owner);
                    }
                }
                if (learn & 0x40) {
                    student->psych_training += points;
                    if (student->psych_training > gbv.ability_threshold_psych) {
                        student->skills |= SKILL_PSYCHIATRIST;
                        if (student->owner == player) {
                            announce_this(ANN_PROMOTED, 6);
                        }
                        set_expertise_staff(student);
                        insert_skill(0x40, student->owner);
                    }
                }
                if (learn & 0x100) {
                    student->research_training += points;
                    if (student->research_training > gbv.ability_threshold_research) {
                        student->skills |= SKILL_RESEARCHER;
                        if (student->owner == player) {
                            announce_this(ANN_PROMOTED, 8);
                        }
                        set_expertise_staff(student);
                        insert_skill(0x100, student->owner);
                    }
                }
            }
        }
    }
}
