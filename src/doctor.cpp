#include "hosp.h"

extern "C" {


u8 check_room_mutex(Thing *w);
Thing *search_mapwho(Map *t, u16 kind);
void fn_0003b234(Thing *w);

}

// FUNCTION: 0x0003a6e0
void doctor_in_office(Thing *doctor)
{
    if (check_room_mutex(doctor) ? 1 : (evacuate_staff(doctor), 0)) {
        if (find_object(((u16)doctor->y.tile << 7) + (u16)doctor->x.tile, OBJ_DESK, 1, 0x96, doctor->owner)) {
            reset_icon_value(doctor, ICON_CALLING);
            doctor->next_state = SS_DOCTOR_WORKS_AT_DESK;
            doctor->resume_state = SS_DOCTOR_IN_OFFICE;
            set_complex(doctor, 10, 0);
            process_staff_complex(doctor);
        } else {
            evacuate_staff(doctor);
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
            doctor->orders &= 0xaf;
            if (check_room_mutex(doctor) ? 1 : (evacuate_staff(doctor), 0)) {
                if (!shall_i_resign(doctor)) {
                    if (doctor->room->f22 || doctor->room->f3 == 4) {
                        evacuate_staff(doctor);
                    } else if ((doctor->orders & ORD_REST) && take_a_break(doctor)) {
                    } else {
                        othervar.f0 = old_newrand() % 100;
                        if (othervar.f0 >= 0 && othervar.f0 < 25) {
                            queue = doctor->room->queue_head;
                            if (queue) {
                                if (call_person_to_room(doctor, queue, 0x28, doctor->room->unknown4, 1)) {
                                    set_icon_value(doctor, ICON_CALLING);
                                    if (find_object((u16)doctor->x.tile + ((u16)doctor->y.tile << 7), OBJ_CABINET, 1, 0x96, doctor->owner)) {
                                        doctor->next_state = SS_DOCTOR_WAITS_FOR_PATIENT;
                                        set_complex(doctor, 0x32, 0);
                                        process_staff_complex(doctor);
                                    } else {
                                        doctor->state = SS_DOCTOR_WAITS_FOR_PATIENT;
                                    }
                                }
                            }
                        } else {
                            othervar.f0 = old_newrand() % 100;
                            if (othervar.f0 >= 25 && othervar.f0 < 32) {
                                if (find_object(((u16)doctor->y.tile << 7) + (u16)doctor->x.tile, OBJ_CABINET, 1, 0x96, doctor->owner)) {
                                    doctor->resume_state = SS_DOCTOR_IN_OFFICE;
                                    set_complex(doctor, 0x14, 0);
                                    process_staff_complex(doctor);
                                }
                            } else if (doctor->room->unknown29) {
                                u8 paged;

                                paged = 0;
                                if (doctor->tiredness > allhosps[doctor->owner].f21d && take_a_break(doctor)) {
                                    paged = 1;
                                } else if ((allhosps[doctor->owner].f21f && doctor->room->queue_head == 0)
                                           && (actual_game_turn % (doctor->tiredness > allhosps[doctor->owner].f21d
                                                                      ? doctor->work_delay * 2 + 1
                                                                      : doctor->work_delay + 1) == 0
                                           && obey_pager(doctor))) {
                                    reset_icon_value(doctor, ICON_CALLING);
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

    patient = &thing[doctor->slave];
    if (patient > thing) {
        if (patient->state == PS_ARRIVED_FROM_CALL) {
            patient->state = 0x20;
            if ((doctor->subtype == STAFF_DOCTOR && doctor->crazy) && (old_newrand() & 3) == 0) {
                send_person_somewhere_else(patient);
                doctor->state = get_state(doctor->room->type, doctor);
                doctor->slave = 0;
                return;
            }
            patient->resume_state = 0;
            switch (doctor->room->type) {
            case ROOM_GP:
                if (find_object(((u16)patient->y.tile << 7) + (u16)patient->x.tile, OBJ_CHAIR, 1, 0x96, patient->owner)) {
                    patient->next_state = 0x20;
                    if (set_complex(patient, 0x82, 0)) {
                        process_person_complex(patient);
                    }
                    doctor->substate++;
                }
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 1;
                set_complex(doctor, 0xfa, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_INFLATION:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 10;
                set_complex(doctor, 0x1e0, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_CARDIO:
                patient->next_state = 0x20;
                set_complex(patient, 0xaa, 0);
                process_person_complex(patient);
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0x1e;
                set_complex(doctor, 0xb4, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_SCANNER:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0x32;
                if (illness_abilities[patient->illness][0]) {
                    set_complex(doctor, 0xd2, 0);
                } else {
                    set_complex(doctor, 0x500, 0);
                }
                process_staff_complex(doctor);
                break;
            case ROOM_PSYCH:
                if (find_object(((u16)doctor->y.tile << 7) + (u16)doctor->x.tile, OBJ_BOOKCASE, 1, 0x96, doctor->owner)) {
                    set_complex(doctor, 0x370, 0);
                }
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0x3c;
                set_complex(doctor, 0x37a, 0);
                process_staff_complex(doctor);
                patient->next_state = 0x20;
                set_complex(patient, 0x1e0, 0);
                process_person_complex(patient);
                break;
            case ROOM_THEATRE: {
                Thing *assistant;

                doctor->work_target = doctor->room->staff[1];
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0x46;
                set_complex(doctor, 0x316, 0);
                process_staff_complex(doctor);
                assistant = &thing[doctor->work_target];
                assistant->next_state = 0x18;
                set_complex(assistant, 0x32a, 0);
                process_staff_complex(assistant);
                break;
            }
            case ROOM_ULTRASCAN:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0x50;
                set_complex(doctor, 0x492, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_BLOOD_MACHINE:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0x5a;
                set_complex(doctor, 0x398, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_XRAY:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0x64;
                set_complex(doctor, 0x276, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_GENERAL_DIAG:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0x6e;
                set_complex(doctor, 0x442, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_HAIR_RESTORER:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0x78;
                set_complex(doctor, 0x2bc, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_SLACK_TONGUE:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0x82;
                set_complex(doctor, 0x1cc, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_FRACTURE:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0x8c;
                set_complex(doctor, 0x42e, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_ELECTROLYSIS:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0xa0;
                set_complex(doctor, 0x186, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_JELLY_VAT:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0xaa;
                set_complex(doctor, 0x19a, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_PHARMACY:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0xb4;
                set_complex(doctor, 0x244, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_DECONTAMINATION:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0xd2;
                set_complex(doctor, 0x28a, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_RESEARCH:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0xdc;
                set_complex(doctor, 0x35c, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_DNA_FIXER:
                doctor->next_state = SS_DOCTOR_EXAMINES;
                doctor->substate = 0xf0;
                set_complex(doctor, 0x47e, 0);
                process_staff_complex(doctor);
                break;
            case ROOM_WARD:
            case ROOM_TRAINING:
            case ROOM_STAFF_ROOM:
            case 26:
            case ROOM_TOILETS:
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
            } else if (forget_about_my_slave(doctor, 0x28)) {
                fn_0003b234(doctor);
            }
        }
    } else {
        doctor->state = get_state(doctor->room->type, doctor);
    }
}

// FUNCTION: 0x0003b234
void fn_0003b234(Thing *w)
{
    w->state = get_state(w->room->type, w);
    w->slave = 0;
    if (w->subtype == STAFF_SURGEON) {
        w = find_waiting_surgeon(w->room, 0x18);
        if (w) {
            w->state = get_state(w->room->type, w);
            w->slave = 0;
        }
    }
}

// FUNCTION: 0x0003b2c4
void doctor_in_machine_room(Thing *doctor)
{
    u16 queue;
    Thing *lead;
    CompFrame *f;

    if (actual_game_turn % doctor->anim_speed == 0) {
        doctor->sprite = frame[doctor->sprite].next;
        f = &frame[doctor->sprite];
        if (f->flags & 1) {
            if (doctor->room->type == ROOM_THEATRE && doctor->subtype == STAFF_DOCTOR) {
                doctor->state = SS_DOCTOR_IN_THEATRE;
            } else {
                doctor->orders &= 0xaf;
                if (check_room_mutex(doctor) ? 1 : (evacuate_staff(doctor), 0)) {
                    if (!shall_i_resign(doctor)) {
                        if (doctor->room->f22 || doctor->room->f3 == 4) {
                            evacuate_staff(doctor);
                        } else if ((doctor->orders & ORD_REST) && take_a_break(doctor)) {
                        } else if ((doctor->subtype == STAFF_NURSE && allhosps[doctor->owner].unknown11f0 == 0) && could_nurse_vaccinate(doctor)) {
                            evacuate_staff(doctor);
                        } else {
                            if (doctor->room->f3 == 5) {
                                doctor->room->f3 = 6;
                            }
                            othervar.f0 = old_newrand() % 100;
                            if (othervar.f0 >= 0 && othervar.f0 < 25) {
                                if (doctor->room->f3 != 6 && (queue = doctor->room->queue_head)) {
                                    if (doctor->room->type == ROOM_THEATRE) {
                                        if (am_i_lead_staff(doctor)) {
                                            doctor->timer = 200;
                                            doctor->state = SS_SURGEON_WAITS;
                                        } else {
                                            lead = find_waiting_surgeon(doctor->room, 0x10);
                                            if (lead > thing && call_person_to_room(lead, queue, 0x28, doctor->room->unknown4, 1)) {
                                                u16 anim;

                                                doctor->state = 0x18;
                                                set_icon_value(lead, ICON_CALLING);
                                                anim = check_frame(staff_anim_starts[lead->subtype].unknownA, lead) + (u16)lead->facing;
                                                lead->sprite = start[anim].frame;
                                                lead->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
                                                lead->state = SS_DOCTOR_WAITS_FOR_PATIENT;
                                            }
                                        }
                                    } else if (call_person_to_room(doctor, queue, 0x28, doctor->room->unknown4, 1)) {
                                        u16 anim;

                                        set_icon_value(doctor, ICON_CALLING);
                                        anim = check_frame(staff_anim_starts[doctor->subtype].unknownA, doctor) + (u16)doctor->facing;
                                        doctor->sprite = start[anim].frame;
                                        doctor->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
                                        doctor->state = SS_DOCTOR_WAITS_FOR_PATIENT;
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
                                        doctor->next_state = doctor->resume_state = SS_DOCTOR_IN_MACHINE_ROOM;
                                        set_complex(doctor, 0xbe, pos);
                                        process_staff_complex(doctor);
                                    }
                                } else if (doctor->room->unknown29) {
                                    u8 paged;

                                    paged = 0;
                                    if (doctor->tiredness > allhosps[doctor->owner].f21d && take_a_break(doctor)) {
                                        paged = 1;
                                    } else if ((allhosps[doctor->owner].f21f && doctor->room->queue_head == 0)
                                               && (actual_game_turn % (doctor->tiredness > allhosps[doctor->owner].f21d
                                                                          ? doctor->work_delay * 2 + 1
                                                                          : doctor->work_delay + 1) == 0
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
            if (!(doctor->skills & SKILL_RESEARCHER)) {
                evacuate_staff(doctor);
            } else if (!shall_i_resign(doctor)) {
                doctor->orders |= ORD_RETURN_TO_RESEARCH;
                if (doctor->room->f22 || doctor->room->f3 == 4) {
                    evacuate_staff(doctor);
                } else if (doctor->orders & 8) {
                    doctor->orders &= 0xf0;
                    evacuate_staff(doctor);
                } else if (((doctor->orders & ORD_REST) || doctor->tiredness > allhosps[doctor->owner].f21d) && take_a_break(doctor)) {
                } else {
                    othervar.f0 = old_newrand() % 100;
                    if (othervar.f0 >= 0 && othervar.f0 < 25) {
                        queue = doctor->room->queue_head;
                        if (queue) {
                            ok = 1;
                            for (occupant = &thing[doctor->room->first]; occupant > thing; occupant = &thing[occupant->room_next]) {
                                if (occupant->type == THING_STAFF && occupant->slave) {
                                    ok = 0;
                                } else if (occupant->type == THING_PATIENT) {
                                    ok = 0;
                                }
                            }
                            if (ok) {
                                if (call_person_to_room(doctor, queue, 0x28, doctor->room->unknown4, 1)) {
                                    set_icon_value(doctor, ICON_CALLING);
                                    doctor->state = SS_DOCTOR_WAITS_FOR_PATIENT;
                                    return;
                                }
                            }
                        }
                    } else {
                        for (i = 0; i < 4; i++) {
                            obj = find_object((u16)doctor->x.tile + ((u16)doctor->y.tile << 7), research_tasks[i].object, 1, 0x96, doctor->owner);
                            if (obj) {
                                user = search_mapwho(&map[thing[obj].use_pos[0]], THING_STAFF);
                                if (!(user == 0 || user == doctor)) {
                                    continue;
                                }
                                if (old_newrand() % research_tasks[i].chance == 0) {
                                    doctor->resume_state = SS_DOCTOR_RESEARCHES;
                                    doctor->next_state = SS_DOCTOR_RESEARCHES;
                                    set_complex(doctor, research_tasks[i].action, 0);
                                    process_staff_complex(doctor);
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

    if (doctor->subtype == STAFF_DOCTOR) {
        if (doctor->subtype == STAFF_DOCTOR && doctor->crazy) {
            return;
        }
        if (doctor->layers[5] == 1) {
            doctor->layers[5] = 3;
        } else if (doctor->layers[5] == 2) {
            doctor->layers[5] = 4;
        }
        doctor->crazy = 1;
        mean = gbv.f156;
        spread = gbv.f158;
        if (doctor->skills & SKILL_CONSULTANT) {
            mean = gbv.f158;
            spread = 1000;
        } else if (doctor->skills & SKILL_JUNIOR) {
            mean = 30;
            spread = gbv.f156;
        }
        doctor->ability = old_normal_rand(mean, spread);
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
        if (person->type == THING_STAFF && person->subtype == STAFF_SURGEON) {
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
    doctor->object = 0;
    if (!(doctor->skills & SKILL_SURGEON)) {
        evacuate_staff(doctor);
        return;
    }
    switch (count) {
    case 0:
        obj = find_object(((u16)doctor->y.tile << 7) + (u16)doctor->x.tile, OBJ_SURGEON_SCREEN, 0, 0x96, doctor->owner);
        if (obj) {
            set_complex(doctor, 0x4d8, 0);
            doctor->resume_state = SS_DOCTOR_IN_THEATRE;
            doctor->next_state = SS_DOCTOR_IN_MACHINE_ROOM;
            process_staff_complex(doctor);
            return;
        }
        break;
    case 1:
        obj = find_object(((u16)doctor->y.tile << 7) + (u16)doctor->x.tile, OBJ_SURGEON_SCREEN, 0, 0x96, doctor->owner);
        if (obj) {
            set_complex(doctor, 0x4ec, 0);
            doctor->resume_state = SS_DOCTOR_IN_THEATRE;
            doctor->next_state = SS_DOCTOR_IN_MACHINE_ROOM;
            process_staff_complex(doctor);
            return;
        }
        break;
    default:
        evacuate_staff(doctor);
        return;
    }
    pos = (s16)(old_newrand() % room->f1) + ((s16)(old_newrand() % room->f2) << 7) + room->pos;
    if (can_i_get_to(doctor, pos)) {
        doctor->next_state = SS_DOCTOR_IN_THEATRE;
        set_complex(doctor, 0xbe, pos);
        process_staff_complex(doctor);
    }
}

// FUNCTION: 0x0003c0a8
void change_surgeon_to_doc(Thing *surgeon)
{
    u8 count;

    count = count_surgeons(surgeon->room);
    if (count == 1) {
        surgeon->resume_state = SS_PROCESS_COMPLEX;
        set_complex(surgeon, 0x3ca, 0);
    } else if (count == 2) {
        surgeon->resume_state = SS_PROCESS_COMPLEX;
        set_complex(surgeon, 0x3d4, 0);
    } else {
        surgeon->subtype = STAFF_DOCTOR;
    }
}

// FUNCTION: 0x0003c11c
void surgeon_waits(Thing *surgeon)
{
    surgeon->timer--;
    if (surgeon->timer <= 0) {
        surgeon->state = SS_DOCTOR_IN_MACHINE_ROOM;
    }
}

// FUNCTION: 0x0003c154
Thing *find_waiting_surgeon(RoomInfo *room, u8 state)
{
    Thing *person;

    for (person = &thing[room->first]; person > thing; person = &thing[person->room_next]) {
        if (person->type == THING_STAFF && person->state == state) {
            return person;
        }
    }
    return 0;
}

// FUNCTION: 0x0003c1e8
u8 forget_about_my_slave(Thing *w, u16 action)
{
    Thing *desk;
    Thing *slave;
    u16 tile;
    u8 result;

    result = 0;
    slave = &thing[w->slave];
    if (slave > thing) {
        if (slave->health <= 0) {
            result = 1;
        } else if (slave->type != THING_PATIENT) {
            result = 1;
        } else if (slave->queue_room) {
            result = 1;
        } else if (slave->next_state != PS_ARRIVED_FROM_CALL) {
            if (slave->next_state != PS_ENTERTAINS_THEMSELF) {
                result = 1;
            } else if (w->subtype == STAFF_RECEPTIONIST) {
                desk = &thing[w->object];
                if (desk > thing) {
                    tile = desk->use_pos[1];
                    call_person_to_room(w, w->slave, action, tile, 0);
                }
            } else {
                tile = w->room->unknown4;
                if (tile) {
                    call_person_to_room(w, w->slave, action, tile, 0);
                }
            }
        }
    } else {
        result = 1;
    }
    return result;
}
