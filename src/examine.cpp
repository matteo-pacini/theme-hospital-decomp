#include "hosp.h"

u8 treatment_machine_used(const u16 illness, const u16 tile);

extern "C" {
extern s16 D_001567b0[8];
}

// FUNCTION: 0x000530e0
void doctor_carries_out_examination(Thing *doctor)
{
    Thing *patient;
    Thing *t;

    patient = &thing[doctor->unknown73];
    if (patient <= thing)
        return;
    switch (doctor->unknown103) {
    case 1:
        doctor->kind = 8;
        doctor->unknown73 = 0;
        done_treatment(doctor, patient);
        doctor->room->unknown29 = 1;
        break;
    case 30:
        if (patient->kind != 0x20) {
            othervar.f0 = old_newrand() % 100;
            if (othervar.f0 >= 30 && othervar.f0 < 32) {
                doctor->state_b = 0x1c;
                set_complex(doctor, 0x10e, 0);
                fn_0003e724(doctor);
            }
            break;
        }
        doctor->unknown103++;
        patient->state_b = 0x20;
        set_complex(patient, 0x78, 0);
        process_person_complex(patient);
        break;
    case 31:
        if (patient->kind != 0x20) {
            othervar.f0 = old_newrand() % 100;
            if (othervar.f0 >= 33 && othervar.f0 < 35) {
                doctor->state_b = 0x1c;
                set_complex(doctor, 0x10e, 0);
                fn_0003e724(doctor);
            }
            break;
        }
        doctor->unknown103++;
        doctor->state_b = 0x1c;
        set_complex(doctor, 0x118, 0);
        fn_0003e724(doctor);
        break;
    case 32:
        othervar.f0 = old_newrand() % 100;
        if (othervar.f0 >= 0 && othervar.f0 < 5) {
            patient->anim_speed = 2;
        } else {
            othervar.f0 = old_newrand() % 100;
            if (othervar.f0 >= 5 && othervar.f0 < 30) {
                patient->anim_speed = 1;
            } else {
                othervar.f0 = old_newrand() % 100;
                if (othervar.f0 >= 30 && othervar.f0 < 35) {
                    patient->anim_speed = 4;
                } else {
                    othervar.f0 = old_newrand() % 100;
                    if (othervar.f0 >= 35 && othervar.f0 < 50) {
                        patient->anim_speed = gbv.f14e;
                        doctor->unknown103 = 33;
                        patient->state_b = 0x20;
                        set_complex(patient, 0xbe, 0);
                        process_person_complex(patient);
                        doctor->state_b = 0x1c;
                        set_complex(doctor, 0x10e, 0);
                        fn_0003e724(doctor);
                        break;
                    } else {
                        doctor->state_b = 0x1c;
                        set_complex(doctor, 0x122, 0);
                        fn_0003e724(doctor);
                        break;
                    }
                }
            }
        }
        doctor->state_b = 0x1c;
        set_complex(doctor, 0x118, 0);
        fn_0003e724(doctor);
        break;
    case 33:
        if (patient->kind != 0x20)
            break;
        doctor->unknown103 = 40;
        patient->state_b = 0x20;
        set_complex(patient, 0xb4, 0);
        process_person_complex(patient);
        break;
    case 40:
        if (patient->kind != 0x20)
            break;
        doctor->kind = 0x24;
        doctor->unknown73 = 0;
        if (treatment_machine_used((u16)patient->unknown171, patient->x.tile + (patient->y.tile << 7)))
            break;
        done_treatment(doctor, patient);
        doctor->room->unknown29 = 1;
        break;
    case 60:
        if (patient->subtype == 3) {
            if (set_complex(patient, 500, 0))
                process_person_complex(patient);
            patient->state_b = 0x20;
            if (find_object(doctor->x.tile + (doctor->y.tile << 7), 0x38, 1, 0x96, doctor->owner)) {
                if (set_complex(doctor, 0x370, 0))
                    fn_0003e724(doctor);
            }
            if (set_complex(doctor, 0x2b2, 0))
                fn_0003e724(doctor);
            doctor->state_b = 0x1c;
        }
        doctor->unknown103 = 230;
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
        if (treatment_machine_used((u16)patient->unknown171, patient->x.tile + (patient->y.tile << 7)))
            break;
        done_treatment(doctor, patient);
        doctor->state_b = 0x24;
        set_complex(doctor, 0x154, 0);
        fn_0003e724(doctor);
        doctor->unknown73 = 0;
        doctor->room->unknown29 = 1;
        break;
    case 240:
        doctor->unknown103++;
        patient->kind = 0x2e;
        t = &thing[patient->unknown69];
        {
            u16 f;

            f = check_frame(anim_starts[patient->subtype].anims[0x79], patient) + patient->unknown17;
            patient->sprite = start[f].frame;
            patient->palette = start[f].flags > 0;
        }
        {
            u16 f;

            f = check_frame(anim_starts[patient->unknown170].anims[0x79], patient) + patient->unknown17;
            t->unknown100 = start[f].frame;
            t->state_b = start[f].flags > 0;
        }
        D_001567b0[4] = D_001567b0[5] = 999;
        full_draw_frame(patient, patient->sprite, 0, 0, 0, 1, patient->palette);
        t->unknown103 = -((TbSprite *)(element[elelist[frame[patient->sprite].list]].sprite + (u32)sprite))->height
                      - D_001567b0[5];
        break;
    case 241:
        if (patient->kind != 0x20)
            break;
        patient->kind = 0x28;
        effect_illness(patient, 8, 1);
        set_complex(doctor, 0x488, 0);
        doctor->state_b = 0x1c;
        fn_0003e724(doctor);
        doctor->unknown103 = 230;
        break;
    case 110:
        {
            u32 complexes[] = { 0x456, 0x460, 0x46a };

            doctor->unknown103 = 111;
            doctor->state_b = 0x1c;
            set_complex(doctor, complexes[old_newrand() % 3], 0);
            fn_0003e724(doctor);
        }
        break;
    case 111:
        doctor->unknown103 = 230;
        doctor->state_b = 0x1c;
        set_complex(doctor, 0x474, 0);
        fn_0003e724(doctor);
        break;
    case 70:
        doctor->state_b = 0x1c;
        set_complex(doctor, 0x334, 0);
        fn_0003e724(doctor);
        doctor->unknown103 = 71;
        break;
    case 71:
        {
            Thing *h;

            h = &thing[doctor->unknown148w];
            if (h->kind != 0x18)
                break;
            h->state_b = 0x18;
            set_complex(h, 0x33e, 0);
            fn_0003e724(h);
            doctor->unknown103 = 72;
        }
        break;
    case 72:
        {
            Thing *h;

            h = &thing[doctor->unknown148w];
            if (h->kind != 0x18)
                break;
            h->state_b = 0x24;
            set_complex(h, 0x352, 0);
            fn_0003e724(h);
            doctor->state_b = 0x1c;
            set_complex(doctor, 0x348, 0);
            fn_0003e724(doctor);
            doctor->unknown103 = 73;
            doctor->unknown148w = 0;
        }
        break;
    case 73:
        if (treatment_machine_used((u16)patient->unknown171, patient->x.tile + (patient->y.tile << 7)))
            break;
        set_complex(patient, 0x276, 0);
        process_person_complex(patient);
        done_treatment(doctor, patient);
        doctor->unknown73 = 0;
        doctor->kind = 0x24;
        doctor->room->unknown29 = 1;
        break;
    case 180:
        if (!pre_administer_drugs(patient)) {
            person_go_home(patient);
            doctor->unknown73 = 0;
            doctor->kind = 0x24;
            doctor->room->unknown29 = 1;
        } else if (patient->style == 15) {
            doctor->state_b = 0x1c;
            doctor->unknown103 = 190;
            set_complex(doctor, 0x21c, 0);
            fn_0003e724(doctor);
        } else if (patient->style == 5) {
            doctor->state_b = 0x1c;
            doctor->unknown103 = 200;
            set_complex(doctor, 0x21c, 0);
            fn_0003e724(doctor);
        } else {
            doctor->state_b = 0x1c;
            doctor->unknown103 = 181;
            set_complex(doctor, 500, 0);
            fn_0003e724(doctor);
        }
        break;
    case 181:
        doctor->state_b = 0x24;
        doctor->unknown73 = 0;
        set_complex(doctor, 0x154, 0);
        fn_0003e724(doctor);
        administer_drugs(doctor, patient);
        doctor->room->unknown29 = 1;
        break;
    case 190:
        {
            u16 f;

            doctor->unknown103++;
            f = check_frame(staff_anim_starts[doctor->subtype].anims[15], doctor) + doctor->unknown17;
            doctor->sprite = start[f].frame;
            doctor->palette = start[f].flags > 0;
        }
        patient->kind = 0x2e;
        patient->unknown69 = doctor->unknown69;
        t = &thing[patient->unknown69];
        {
            u16 f;

            f = check_frame(anim_starts[patient->subtype].anims[43], patient) + patient->unknown17;
            patient->sprite = start[f].frame;
            patient->palette = start[f].flags > 0;
        }
        if (patient->subtype == 6) {
            u16 f;

            f = check_frame(anim_starts[0].anims[43], patient) + patient->unknown17;
            t->unknown100 = start[f].frame;
            t->state_b = start[f].flags > 0;
        } else {
            u16 f;

            f = check_frame(anim_starts[1].anims[43], patient) + patient->unknown17;
            t->unknown100 = start[f].frame;
            t->state_b = start[f].flags > 0;
        }
        t->unknown103 = 0;
        break;
    case 191:
        if (patient->kind != 0x20)
            break;
        transform_peep_who(patient, 4);
        doctor->state_b = 0x1c;
        set_complex(doctor, 0x230, 0);
        fn_0003e724(doctor);
        doctor->unknown103 = 181;
        patient->kind = 0x28;
        break;
    case 200:
        {
            u16 f;

            doctor->unknown103++;
            f = check_frame(staff_anim_starts[doctor->subtype].anims[15], doctor) + doctor->unknown17;
            doctor->sprite = start[f].frame;
            doctor->palette = start[f].flags > 0;
        }
        patient->kind = 0x30;
        patient->unknown69 = doctor->unknown69;
        t = &thing[patient->unknown69];
        {
            u16 f;

            f = check_frame(anim_starts[patient->subtype].anims[43], patient) + patient->unknown17;
            patient->sprite = start[f].frame;
            patient->palette = start[f].flags > 0;
        }
        t->unknown105 = 4;
        {
            u16 f;

            f = check_frame(anim_starts[0].anims[43], patient) + patient->unknown17;
            t->unknown100 = start[f].frame;
            t->state_b = start[f].flags > 0;
        }
        t->unknown104 = 0;
        t->unknown103 = 0;
        break;
    case 201:
        t = &thing[patient->unknown69];
        if (t->unknown103 == 200) {
            doctor->unknown103++;
            t->unknown105 = 8;
            t->unknown104 = 1;
            t->unknown103 = 0;
        } else {
            t->unknown103 += 10;
        }
        break;
    case 202:
        t = &thing[patient->unknown69];
        if (t->unknown103 == 200) {
            doctor->unknown103++;
            t->unknown105 = 0xff;
            t->unknown104 = 1;
            t->unknown103 = 0;
        } else {
            t->unknown103 += 10;
        }
        break;
    case 203:
        t = &thing[patient->unknown69];
        if (t->unknown103 == 200) {
            doctor->unknown103++;
            patient->sprite = t->unknown100;
            patient->palette = t->state_b;
            t->unknown105 = 0xff;
            t->unknown104 = 1;
            t->unknown103 = 200;
        } else {
            t->unknown103 += 10;
        }
        break;
    case 204:
        t = &thing[patient->unknown69];
        if (t->unknown103 == 0) {
            doctor->unknown103++;
            t->unknown105 = 8;
            t->unknown104 = 5;
            t->unknown103 = 200;
        } else {
            t->unknown103 -= 10;
        }
        break;
    case 205:
        t = &thing[patient->unknown69];
        if (t->unknown103 == 0) {
            doctor->unknown103++;
            t->unknown105 = 4;
            t->unknown104 = 0;
            t->unknown103 = 200;
        } else {
            t->unknown103 -= 10;
        }
        break;
    case 206:
        t = &thing[patient->unknown69];
        if (t->unknown103 == 0) {
            transform_peep_who(patient, 0xa);
            doctor->state_b = 0x1c;
            set_complex(doctor, 0x230, 0);
            fn_0003e724(doctor);
            doctor->unknown103 = 181;
            patient->kind = 0x28;
        } else {
            t->unknown103 -= 10;
        }
        break;
    case 220:
        if (treatment_machine_used(0x2c, patient->x.tile + (patient->y.tile << 7)))
            break;
        doctor->state_b = 0x3e;
        set_complex(doctor, 0x154, 0);
        fn_0003e724(doctor);
        doctor->unknown73 = 0;
        doctor->room->unknown29 = 1;
        remove_person(patient);
        break;
    default:
        doctor->unknown103 = 1;
        set_complex(doctor, 0xfa, 0);
        fn_0003e724(doctor);
        done_treatment(doctor, patient);
        break;
    }
}

// FUNCTION: 0x00054444
void effect_illness(Thing *patient, u8 illness, u8 cured)
{
    u8 r;

    switch (illness) {
    case 2:
        if (cured == 1) {
            patient->layers[0] -= 5;
        } else {
            patient->layers[0] += 5;
            patient->layers[2] = 0;
        }
        break;
    case 3:
        if (cured == 1) {
            transform_peep_who(patient, 2);
        } else {
            patient->layers[0] = 1;
            transform_peep_who(patient, 1);
            patient->layers[2] = 0;
        }
        break;
    case 4:
        if (cured == 1) {
            transform_peep_who(patient, 6);
        } else {
            patient->layers[0] = 1;
            transform_peep_who(patient, 5);
            patient->layers[2] = 0;
        }
        break;
    case 15:
        if (cured == 0)
            transform_peep_who(patient, 3);
        break;
    case 7:
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
    case 5:
        if (cured == 1) {
            transform_peep_who(patient, 0xa);
        } else {
            patient->layers[0] = 1;
            transform_peep_who(patient, 9);
        }
        break;
    case 10:
        if (cured == 1) {
            patient->layers[0] = 7;
        } else {
            transform_peep_who(patient, 7);
            patient->layers[0] = 6;
        }
        break;
    case 9:
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
    case 8:
        if (cured == 1) {
            patient->subtype = patient->unknown170;
        } else {
            patient->layers[0] = 1;
            patient->unknown170 = patient->subtype;
            patient->subtype = 0xe;
        }
        break;
    case 6:
    case 11:
    case 12:
    case 13:
    case 14:
    default:
        break;
    }
    if (cured == 0) {
        patient->style = illness;
    } else {
        allhosps[patient->owner].f1e9++;
        if (patient->owner == player)
            advisor.f128 |= 0x40;
        allhosps[patient->owner].rooms[patient->style].unknown1E++;
        reputation_affected(patient->style, 0, 0, patient->owner, 0);
        patient->unknown169 = patient->style = 0;
        if (patient->unknown174)
            clean_contagious_peep(patient);
        if (patient->owner == player)
            mycontrol->fc++;
        if (patient->flags & 0x40) {
            allhosps[patient->owner].f1e5++;
            patient->flags &= ~0x40;
            reset_icon_value(patient, 6);
        }
        record_person_as_patient(patient);
    }
}
