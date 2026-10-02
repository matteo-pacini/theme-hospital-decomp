#include "hosp.h"

u8 treatment_machine_used(const u16 illness, const u16 tile);

extern "C" {
extern s16 frame_bounds[8];
}

// SS_DOCTOR_EXAMINES: steps the examination or treatment of doctor's patient
// (slave) through the room-specific sequence chosen by doctor->substate (set
// by doctor_waits_for_patient_to_arrive), starting each stage's complexes for
// doctor, patient and any assistant; at the end applies the result
// (done_treatment, unless the treatment machine is still in use) and returns
// doctor to the room's working state.
// FUNCTION: 0x000530e0
void doctor_carries_out_examination(Thing *doctor)
{
    Thing *patient;
    Thing *obj;

    patient = &thing[doctor->slave];
    if (patient <= thing)
        return;
    switch (doctor->substate) {
    case 1:
        doctor->state = SS_DOCTOR_WORKS_AT_DESK;
        doctor->slave = 0;
        done_treatment(doctor, patient);
        doctor->room->staff_free = 1;
        break;
    case 30:
        if (patient->state != PS_SLAVE_READY) {
            othervar.rand_pct = old_newrand() % 100;
            if (othervar.rand_pct >= 30 && othervar.rand_pct < 32) {
                doctor->next_state = SS_DOCTOR_EXAMINES;
                set_complex(doctor, 0x10e, 0);
                process_staff_complex(doctor);
            }
            break;
        }
        doctor->substate++;
        patient->next_state = PS_SLAVE_READY;
        set_complex(patient, 0x78, 0);
        process_person_complex(patient);
        break;
    case 31:
        if (patient->state != PS_SLAVE_READY) {
            othervar.rand_pct = old_newrand() % 100;
            if (othervar.rand_pct >= 33 && othervar.rand_pct < 35) {
                doctor->next_state = SS_DOCTOR_EXAMINES;
                set_complex(doctor, 0x10e, 0);
                process_staff_complex(doctor);
            }
            break;
        }
        doctor->substate++;
        doctor->next_state = SS_DOCTOR_EXAMINES;
        set_complex(doctor, 0x118, 0);
        process_staff_complex(doctor);
        break;
    case 32:
        othervar.rand_pct = old_newrand() % 100;
        if (othervar.rand_pct >= 0 && othervar.rand_pct < 5) {
            patient->anim_speed = 2;
        } else {
            othervar.rand_pct = old_newrand() % 100;
            if (othervar.rand_pct >= 5 && othervar.rand_pct < 30) {
                patient->anim_speed = 1;
            } else {
                othervar.rand_pct = old_newrand() % 100;
                if (othervar.rand_pct >= 30 && othervar.rand_pct < 35) {
                    patient->anim_speed = 4;
                } else {
                    othervar.rand_pct = old_newrand() % 100;
                    if (othervar.rand_pct >= 35 && othervar.rand_pct < 50) {
                        patient->anim_speed = gbv.peep_anim_speed;
                        doctor->substate = 33;
                        patient->next_state = PS_SLAVE_READY;
                        set_complex(patient, 0xbe, 0);
                        process_person_complex(patient);
                        doctor->next_state = SS_DOCTOR_EXAMINES;
                        set_complex(doctor, 0x10e, 0);
                        process_staff_complex(doctor);
                        break;
                    } else {
                        doctor->next_state = SS_DOCTOR_EXAMINES;
                        set_complex(doctor, 0x122, 0);
                        process_staff_complex(doctor);
                        break;
                    }
                }
            }
        }
        doctor->next_state = SS_DOCTOR_EXAMINES;
        set_complex(doctor, 0x118, 0);
        process_staff_complex(doctor);
        break;
    case 33:
        if (patient->state != PS_SLAVE_READY)
            break;
        doctor->substate = 40;
        patient->next_state = PS_SLAVE_READY;
        set_complex(patient, 0xb4, 0);
        process_person_complex(patient);
        break;
    case 40:
        if (patient->state != PS_SLAVE_READY)
            break;
        doctor->state = SS_DOCTOR_IN_MACHINE_ROOM;
        doctor->slave = 0;
        if (treatment_machine_used((u16)patient->treatment, patient->x.tile + (patient->y.tile << 7)))
            break;
        done_treatment(doctor, patient);
        doctor->room->staff_free = 1;
        break;
    case 60:
        if (patient->subtype == 3) {
            if (set_complex(patient, 500, 0))
                process_person_complex(patient);
            patient->next_state = PS_SLAVE_READY;
            if (find_object(doctor->x.tile + (doctor->y.tile << 7), OBJ_BOOKCASE, 1, 0x96, doctor->owner)) {
                if (set_complex(doctor, 0x370, 0))
                    process_staff_complex(doctor);
            }
            if (set_complex(doctor, 0x2b2, 0))
                process_staff_complex(doctor);
            doctor->next_state = SS_DOCTOR_EXAMINES;
        }
        doctor->substate = 230;
        break;
    case 10:
    case 50:
    case 80:
    case 90:
    case 100:
    case 120:
    case 130:
    case 140:
    case 160:
    case 170:
    case 210:
    case 230:
        if (treatment_machine_used((u16)patient->treatment, patient->x.tile + (patient->y.tile << 7)))
            break;
        done_treatment(doctor, patient);
        doctor->next_state = SS_DOCTOR_IN_MACHINE_ROOM;
        set_complex(doctor, 0x154, 0);
        process_staff_complex(doctor);
        doctor->slave = 0;
        doctor->room->staff_free = 1;
        break;
    case 240:
        doctor->substate++;
        patient->state = PS_FILLS;
        obj = &thing[patient->object];
        {
            u16 entry;

            entry = check_frame(anim_starts[patient->subtype].anims[0x79], patient) + patient->facing;
            patient->sprite = start[entry].frame;
            patient->flip = start[entry].flags > 0;
        }
        {
            u16 entry;

            entry = check_frame(anim_starts[patient->orig_subtype].anims[0x79], patient) + patient->facing;
            obj->aux_sprite = start[entry].frame;
            obj->aux_flip = start[entry].flags > 0;
        }
        frame_bounds[4] = frame_bounds[5] = 999;
        full_draw_frame(patient, patient->sprite, 0, 0, 0, 1, patient->flip);
        obj->split_height =
            -((TbSprite *)(element[elelist[frame[patient->sprite].list]].sprite + (u32)sprite))->height -
            frame_bounds[5];
        break;
    case 241:
        if (patient->state != PS_SLAVE_READY)
            break;
        patient->state = PS_MERGED;
        effect_illness(patient, ILL_ALIEN_DNA, 1);
        set_complex(doctor, 0x488, 0);
        doctor->next_state = SS_DOCTOR_EXAMINES;
        process_staff_complex(doctor);
        doctor->substate = 230;
        break;
    case 110: {
        u32 complexes[] = {0x456, 0x460, 0x46a};

        doctor->substate = 111;
        doctor->next_state = SS_DOCTOR_EXAMINES;
        set_complex(doctor, complexes[old_newrand() % 3], 0);
        process_staff_complex(doctor);
    } break;
    case 111:
        doctor->substate = 230;
        doctor->next_state = SS_DOCTOR_EXAMINES;
        set_complex(doctor, 0x474, 0);
        process_staff_complex(doctor);
        break;
    case 70:
        doctor->next_state = SS_DOCTOR_EXAMINES;
        set_complex(doctor, 0x334, 0);
        process_staff_complex(doctor);
        doctor->substate = 71;
        break;
    case 71: {
        Thing *assistant;

        assistant = &thing[doctor->work_target];
        if (assistant->state != SS_ASSISTANT_READY)
            break;
        assistant->next_state = SS_ASSISTANT_READY;
        set_complex(assistant, 0x33e, 0);
        process_staff_complex(assistant);
        doctor->substate = 72;
    } break;
    case 72: {
        Thing *assistant;

        assistant = &thing[doctor->work_target];
        if (assistant->state != SS_ASSISTANT_READY)
            break;
        assistant->next_state = SS_DOCTOR_IN_MACHINE_ROOM;
        set_complex(assistant, 0x352, 0);
        process_staff_complex(assistant);
        doctor->next_state = SS_DOCTOR_EXAMINES;
        set_complex(doctor, 0x348, 0);
        process_staff_complex(doctor);
        doctor->substate = 73;
        doctor->work_target = 0;
    } break;
    case 73:
        if (treatment_machine_used((u16)patient->treatment, patient->x.tile + (patient->y.tile << 7)))
            break;
        set_complex(patient, 0x276, 0);
        process_person_complex(patient);
        done_treatment(doctor, patient);
        doctor->slave = 0;
        doctor->state = SS_DOCTOR_IN_MACHINE_ROOM;
        doctor->room->staff_free = 1;
        break;
    case 180:
        if (!pre_administer_drugs(patient)) {
            person_go_home(patient);
            doctor->slave = 0;
            doctor->state = SS_DOCTOR_IN_MACHINE_ROOM;
            doctor->room->staff_free = 1;
        } else if (patient->illness == ILL_TRANSPARENCY) {
            doctor->next_state = SS_DOCTOR_EXAMINES;
            doctor->substate = 190;
            set_complex(doctor, 0x21c, 0);
            process_staff_complex(doctor);
        } else if (patient->illness == ILL_INVISIBILITY) {
            doctor->next_state = SS_DOCTOR_EXAMINES;
            doctor->substate = 200;
            set_complex(doctor, 0x21c, 0);
            process_staff_complex(doctor);
        } else {
            doctor->next_state = SS_DOCTOR_EXAMINES;
            doctor->substate = 181;
            set_complex(doctor, 500, 0);
            process_staff_complex(doctor);
        }
        break;
    case 181:
        doctor->next_state = SS_DOCTOR_IN_MACHINE_ROOM;
        doctor->slave = 0;
        set_complex(doctor, 0x154, 0);
        process_staff_complex(doctor);
        administer_drugs(doctor, patient);
        doctor->room->staff_free = 1;
        break;
    case 190: {
        u16 entry;

        doctor->substate++;
        entry = check_frame(staff_anim_starts[doctor->subtype].anims[15], doctor) + doctor->facing;
        doctor->sprite = start[entry].frame;
        doctor->flip = start[entry].flags > 0;
    }
        patient->state = PS_FILLS;
        patient->object = doctor->object;
        obj = &thing[patient->object];
        {
            u16 entry;

            entry = check_frame(anim_starts[patient->subtype].anims[43], patient) + patient->facing;
            patient->sprite = start[entry].frame;
            patient->flip = start[entry].flags > 0;
        }
        if (patient->subtype == 6) {
            u16 entry;

            entry = check_frame(anim_starts[0].anims[43], patient) + patient->facing;
            obj->aux_sprite = start[entry].frame;
            obj->aux_flip = start[entry].flags > 0;
        } else {
            u16 entry;

            entry = check_frame(anim_starts[1].anims[43], patient) + patient->facing;
            obj->aux_sprite = start[entry].frame;
            obj->aux_flip = start[entry].flags > 0;
        }
        obj->split_height = 0;
        break;
    case 191:
        if (patient->state != PS_SLAVE_READY)
            break;
        transform_peep_who(patient, 4);
        doctor->next_state = SS_DOCTOR_EXAMINES;
        set_complex(doctor, 0x230, 0);
        process_staff_complex(doctor);
        doctor->substate = 181;
        patient->state = PS_MERGED;
        break;
    case 200: {
        u16 entry;

        doctor->substate++;
        entry = check_frame(staff_anim_starts[doctor->subtype].anims[15], doctor) + doctor->facing;
        doctor->sprite = start[entry].frame;
        doctor->flip = start[entry].flags > 0;
    }
        patient->state = PS_ENERGISES;
        patient->object = doctor->object;
        obj = &thing[patient->object];
        {
            u16 entry;

            entry = check_frame(anim_starts[patient->subtype].anims[43], patient) + patient->facing;
            patient->sprite = start[entry].frame;
            patient->flip = start[entry].flags > 0;
        }
        obj->aux_draw_flags = 4;
        {
            u16 entry;

            entry = check_frame(anim_starts[0].anims[43], patient) + patient->facing;
            obj->aux_sprite = start[entry].frame;
            obj->aux_flip = start[entry].flags > 0;
        }
        obj->aux_param = 0;
        obj->split_height = 0;
        break;
    case 201:
        obj = &thing[patient->object];
        if (obj->split_height == 200) {
            doctor->substate++;
            obj->aux_draw_flags = 8;
            obj->aux_param = 1;
            obj->split_height = 0;
        } else {
            obj->split_height += 10;
        }
        break;
    case 202:
        obj = &thing[patient->object];
        if (obj->split_height == 200) {
            doctor->substate++;
            obj->aux_draw_flags = 0xff;
            obj->aux_param = 1;
            obj->split_height = 0;
        } else {
            obj->split_height += 10;
        }
        break;
    case 203:
        obj = &thing[patient->object];
        if (obj->split_height == 200) {
            doctor->substate++;
            patient->sprite = obj->aux_sprite;
            patient->flip = obj->aux_flip;
            obj->aux_draw_flags = 0xff;
            obj->aux_param = 1;
            obj->split_height = 200;
        } else {
            obj->split_height += 10;
        }
        break;
    case 204:
        obj = &thing[patient->object];
        if (obj->split_height == 0) {
            doctor->substate++;
            obj->aux_draw_flags = 8;
            obj->aux_param = 5;
            obj->split_height = 200;
        } else {
            obj->split_height -= 10;
        }
        break;
    case 205:
        obj = &thing[patient->object];
        if (obj->split_height == 0) {
            doctor->substate++;
            obj->aux_draw_flags = 4;
            obj->aux_param = 0;
            obj->split_height = 200;
        } else {
            obj->split_height -= 10;
        }
        break;
    case 206:
        obj = &thing[patient->object];
        if (obj->split_height == 0) {
            transform_peep_who(patient, 0xa);
            doctor->next_state = SS_DOCTOR_EXAMINES;
            set_complex(doctor, 0x230, 0);
            process_staff_complex(doctor);
            doctor->substate = 181;
            patient->state = PS_MERGED;
        } else {
            obj->split_height -= 10;
        }
        break;
    case 220:
        if (treatment_machine_used(ILL_AUTOPSY, patient->x.tile + (patient->y.tile << 7)))
            break;
        doctor->next_state = SS_DOCTOR_RESEARCHES;
        set_complex(doctor, 0x154, 0);
        process_staff_complex(doctor);
        doctor->slave = 0;
        doctor->room->staff_free = 1;
        remove_person(patient);
        break;
    default:
        doctor->substate = 1;
        set_complex(doctor, 0xfa, 0);
        process_staff_complex(doctor);
        done_treatment(doctor, patient);
        break;
    }
}

// With cured 0, gives patient illness and changes their appearance to show it.
// With cured 1, restores their appearance and records the cure of their
// current illness: cure counts, reputation, contagion cleared, emergency
// progress, and the patient record. illness picks the appearance change.
// FUNCTION: 0x00054444
void effect_illness(Thing *patient, u8 illness, u8 cured)
{
    u8 r;

    switch (illness) {
    case ILL_BLOATY_HEAD:
        if (cured == 1) {
            patient->layers[0] -= 5;
        } else {
            patient->layers[0] += 5;
            patient->layers[2] = 0;
        }
        break;
    case ILL_HAIRYITIS:
        if (cured == 1) {
            transform_peep_who(patient, 2);
        } else {
            patient->layers[0] = 1;
            transform_peep_who(patient, 1);
            patient->layers[2] = 0;
        }
        break;
    case ILL_KING_COMPLEX:
        if (cured == 1) {
            transform_peep_who(patient, 6);
        } else {
            patient->layers[0] = 1;
            transform_peep_who(patient, 5);
            patient->layers[2] = 0;
        }
        break;
    case ILL_TRANSPARENCY:
        if (cured == 0)
            transform_peep_who(patient, 3);
        break;
    case ILL_SLACK_TONGUE:
        if (cured == 1) {
            if (patient->subtype == 1)
                patient->layers[0] -= 4;
            else
                transform_peep_who(patient, 8);
        } else {
            if (patient->subtype == 1)
                patient->layers[0] += 4;
            else
                transform_peep_who(patient, 7);
        }
        break;
    case ILL_INVISIBILITY:
        if (cured == 1) {
            transform_peep_who(patient, 0xa);
        } else {
            patient->layers[0] = 1;
            transform_peep_who(patient, 9);
        }
        break;
    case ILL_BALDNESS:
        if (cured == 1) {
            patient->layers[0] = 7;
        } else {
            transform_peep_who(patient, 7);
            patient->layers[0] = 6;
        }
        break;
    case ILL_FRACTURED_BONES:
        if (patient->subtype == 1) {
            if (cured == 1) {
                patient->layers[2] = 0;
                patient->layers[3] = 0;
            } else {
                r = old_newrand() % 3 + 1;
                patient->layers[2] = r & 1;
                patient->layers[3] = (r >> 1) & 1;
            }
        } else {
            if (cured == 1) {
                transform_peep_who(patient, 0xe);
            } else {
                transform_peep_who(patient, 0xd);
                patient->layers[2] = old_newrand() & 3;
                patient->layers[3] = old_newrand() % 6;
                patient->layers[4] = old_newrand() % 6;
            }
        }
        break;
    case ILL_ALIEN_DNA:
        if (cured == 1) {
            patient->subtype = patient->orig_subtype;
        } else {
            patient->layers[0] = 1;
            patient->orig_subtype = patient->subtype;
            patient->subtype = PK_ALIEN;
        }
        break;
    case ILL_SERIOUS_RADIATION:
    case ILL_DISCRETE_ITCHING:
    case ILL_JELLYITIS:
    case ILL_SLEEPING_ILLNESS:
    case ILL_PREGNANT:
    default:
        break;
    }
    if (cured == 0) {
        patient->illness = illness;
    } else {
        allhosps[patient->owner].cures++;
        if (patient->owner == player)
            advisor.event_flags |= 0x40;
        allhosps[patient->owner].rooms[patient->illness].cured++;
        reputation_affected(patient->illness, REP_CURED, 0, patient->owner, 0);
        patient->known_illness = patient->illness = 0;
        if (patient->contagion)
            clean_contagious_peep(patient);
        if (patient->owner == player)
            mycontrol->cures_this_month++;
        if (patient->flags & PF_EMERGENCY) {
            allhosps[patient->owner].emergency_cured++;
            patient->flags &= ~PF_EMERGENCY;
            reset_icon_value(patient, ICON_EMERGENCY);
        }
        record_person_as_patient(patient);
    }
}
