#include "hosp.h"

u8 treatment_machine_used(const u16 illness, const u16 tile);

extern "C" {
extern s16 D_001567b0[8];
}

// FUNCTION: 0x000530e0
void doctor_carries_out_examination(Thing *w)
{
    Thing *o;
    Thing *t;

    o = &thing[w->unknown73];
    if (o <= thing)
        return;
    switch (w->unknown103) {
    case 1:
        w->kind = 8;
        w->unknown73 = 0;
        done_treatment(w, o);
        w->room->unknown29 = 1;
        break;
    case 30:
        if (o->kind != 0x20) {
            othervar.f0 = old_newrand() % 100;
            if (othervar.f0 >= 30 && othervar.f0 < 32) {
                w->state_b = 0x1c;
                set_complex(w, 0x10e, 0);
                fn_0003e724(w);
            }
            break;
        }
        w->unknown103++;
        o->state_b = 0x20;
        set_complex(o, 0x78, 0);
        process_person_complex(o);
        break;
    case 31:
        if (o->kind != 0x20) {
            othervar.f0 = old_newrand() % 100;
            if (othervar.f0 >= 33 && othervar.f0 < 35) {
                w->state_b = 0x1c;
                set_complex(w, 0x10e, 0);
                fn_0003e724(w);
            }
            break;
        }
        w->unknown103++;
        w->state_b = 0x1c;
        set_complex(w, 0x118, 0);
        fn_0003e724(w);
        break;
    case 32:
        othervar.f0 = old_newrand() % 100;
        if (othervar.f0 >= 0 && othervar.f0 < 5) {
            o->anim_speed = 2;
        } else {
            othervar.f0 = old_newrand() % 100;
            if (othervar.f0 >= 5 && othervar.f0 < 30) {
                o->anim_speed = 1;
            } else {
                othervar.f0 = old_newrand() % 100;
                if (othervar.f0 >= 30 && othervar.f0 < 35) {
                    o->anim_speed = 4;
                } else {
                    othervar.f0 = old_newrand() % 100;
                    if (othervar.f0 >= 35 && othervar.f0 < 50) {
                        o->anim_speed = gbv.f14e;
                        w->unknown103 = 33;
                        o->state_b = 0x20;
                        set_complex(o, 0xbe, 0);
                        process_person_complex(o);
                        w->state_b = 0x1c;
                        set_complex(w, 0x10e, 0);
                        fn_0003e724(w);
                        break;
                    } else {
                        w->state_b = 0x1c;
                        set_complex(w, 0x122, 0);
                        fn_0003e724(w);
                        break;
                    }
                }
            }
        }
        w->state_b = 0x1c;
        set_complex(w, 0x118, 0);
        fn_0003e724(w);
        break;
    case 33:
        if (o->kind != 0x20)
            break;
        w->unknown103 = 40;
        o->state_b = 0x20;
        set_complex(o, 0xb4, 0);
        process_person_complex(o);
        break;
    case 40:
        if (o->kind != 0x20)
            break;
        w->kind = 0x24;
        w->unknown73 = 0;
        if (treatment_machine_used((u16)o->unknown171, o->x.tile + (o->y.tile << 7)))
            break;
        done_treatment(w, o);
        w->room->unknown29 = 1;
        break;
    case 60:
        if (o->subtype == 3) {
            if (set_complex(o, 500, 0))
                process_person_complex(o);
            o->state_b = 0x20;
            if (find_object(w->x.tile + (w->y.tile << 7), 0x38, 1, 0x96, w->owner)) {
                if (set_complex(w, 0x370, 0))
                    fn_0003e724(w);
            }
            if (set_complex(w, 0x2b2, 0))
                fn_0003e724(w);
            w->state_b = 0x1c;
        }
        w->unknown103 = 230;
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
        if (treatment_machine_used((u16)o->unknown171, o->x.tile + (o->y.tile << 7)))
            break;
        done_treatment(w, o);
        w->state_b = 0x24;
        set_complex(w, 0x154, 0);
        fn_0003e724(w);
        w->unknown73 = 0;
        w->room->unknown29 = 1;
        break;
    case 240:
        w->unknown103++;
        o->kind = 0x2e;
        t = &thing[o->unknown69];
        {
            u16 f;

            f = check_frame(anim_starts[o->subtype].anims[0x79], o) + o->unknown17;
            o->sprite = start[f].frame;
            o->palette = start[f].flags > 0;
        }
        {
            u16 f;

            f = check_frame(anim_starts[o->unknown170].anims[0x79], o) + o->unknown17;
            t->unknown100 = start[f].frame;
            t->state_b = start[f].flags > 0;
        }
        D_001567b0[4] = D_001567b0[5] = 999;
        full_draw_frame(o, o->sprite, 0, 0, 0, 1, o->palette);
        t->unknown103 = -((TbSprite *)(element[elelist[frame[o->sprite].list]].sprite + (u32)sprite))->height
                      - D_001567b0[5];
        break;
    case 241:
        if (o->kind != 0x20)
            break;
        o->kind = 0x28;
        effect_illness(o, 8, 1);
        set_complex(w, 0x488, 0);
        w->state_b = 0x1c;
        fn_0003e724(w);
        w->unknown103 = 230;
        break;
    case 110:
        {
            u32 complexes[] = { 0x456, 0x460, 0x46a };

            w->unknown103 = 111;
            w->state_b = 0x1c;
            set_complex(w, complexes[old_newrand() % 3], 0);
            fn_0003e724(w);
        }
        break;
    case 111:
        w->unknown103 = 230;
        w->state_b = 0x1c;
        set_complex(w, 0x474, 0);
        fn_0003e724(w);
        break;
    case 70:
        w->state_b = 0x1c;
        set_complex(w, 0x334, 0);
        fn_0003e724(w);
        w->unknown103 = 71;
        break;
    case 71:
        {
            Thing *h;

            h = &thing[w->unknown148w];
            if (h->kind != 0x18)
                break;
            h->state_b = 0x18;
            set_complex(h, 0x33e, 0);
            fn_0003e724(h);
            w->unknown103 = 72;
        }
        break;
    case 72:
        {
            Thing *h;

            h = &thing[w->unknown148w];
            if (h->kind != 0x18)
                break;
            h->state_b = 0x24;
            set_complex(h, 0x352, 0);
            fn_0003e724(h);
            w->state_b = 0x1c;
            set_complex(w, 0x348, 0);
            fn_0003e724(w);
            w->unknown103 = 73;
            w->unknown148w = 0;
        }
        break;
    case 73:
        if (treatment_machine_used((u16)o->unknown171, o->x.tile + (o->y.tile << 7)))
            break;
        set_complex(o, 0x276, 0);
        process_person_complex(o);
        done_treatment(w, o);
        w->unknown73 = 0;
        w->kind = 0x24;
        w->room->unknown29 = 1;
        break;
    case 180:
        if (!pre_administer_drugs(o)) {
            person_go_home(o);
            w->unknown73 = 0;
            w->kind = 0x24;
            w->room->unknown29 = 1;
        } else if (o->style == 15) {
            w->state_b = 0x1c;
            w->unknown103 = 190;
            set_complex(w, 0x21c, 0);
            fn_0003e724(w);
        } else if (o->style == 5) {
            w->state_b = 0x1c;
            w->unknown103 = 200;
            set_complex(w, 0x21c, 0);
            fn_0003e724(w);
        } else {
            w->state_b = 0x1c;
            w->unknown103 = 181;
            set_complex(w, 500, 0);
            fn_0003e724(w);
        }
        break;
    case 181:
        w->state_b = 0x24;
        w->unknown73 = 0;
        set_complex(w, 0x154, 0);
        fn_0003e724(w);
        administer_drugs(w, o);
        w->room->unknown29 = 1;
        break;
    case 190:
        {
            u16 f;

            w->unknown103++;
            f = check_frame(staff_anim_starts[w->subtype].anims[15], w) + w->unknown17;
            w->sprite = start[f].frame;
            w->palette = start[f].flags > 0;
        }
        o->kind = 0x2e;
        o->unknown69 = w->unknown69;
        t = &thing[o->unknown69];
        {
            u16 f;

            f = check_frame(anim_starts[o->subtype].anims[43], o) + o->unknown17;
            o->sprite = start[f].frame;
            o->palette = start[f].flags > 0;
        }
        if (o->subtype == 6) {
            u16 f;

            f = check_frame(anim_starts[0].anims[43], o) + o->unknown17;
            t->unknown100 = start[f].frame;
            t->state_b = start[f].flags > 0;
        } else {
            u16 f;

            f = check_frame(anim_starts[1].anims[43], o) + o->unknown17;
            t->unknown100 = start[f].frame;
            t->state_b = start[f].flags > 0;
        }
        t->unknown103 = 0;
        break;
    case 191:
        if (o->kind != 0x20)
            break;
        transform_peep_who(o, 4);
        w->state_b = 0x1c;
        set_complex(w, 0x230, 0);
        fn_0003e724(w);
        w->unknown103 = 181;
        o->kind = 0x28;
        break;
    case 200:
        {
            u16 f;

            w->unknown103++;
            f = check_frame(staff_anim_starts[w->subtype].anims[15], w) + w->unknown17;
            w->sprite = start[f].frame;
            w->palette = start[f].flags > 0;
        }
        o->kind = 0x30;
        o->unknown69 = w->unknown69;
        t = &thing[o->unknown69];
        {
            u16 f;

            f = check_frame(anim_starts[o->subtype].anims[43], o) + o->unknown17;
            o->sprite = start[f].frame;
            o->palette = start[f].flags > 0;
        }
        t->unknown105 = 4;
        {
            u16 f;

            f = check_frame(anim_starts[0].anims[43], o) + o->unknown17;
            t->unknown100 = start[f].frame;
            t->state_b = start[f].flags > 0;
        }
        t->unknown104 = 0;
        t->unknown103 = 0;
        break;
    case 201:
        t = &thing[o->unknown69];
        if (t->unknown103 == 200) {
            w->unknown103++;
            t->unknown105 = 8;
            t->unknown104 = 1;
            t->unknown103 = 0;
        } else {
            t->unknown103 += 10;
        }
        break;
    case 202:
        t = &thing[o->unknown69];
        if (t->unknown103 == 200) {
            w->unknown103++;
            t->unknown105 = 0xff;
            t->unknown104 = 1;
            t->unknown103 = 0;
        } else {
            t->unknown103 += 10;
        }
        break;
    case 203:
        t = &thing[o->unknown69];
        if (t->unknown103 == 200) {
            w->unknown103++;
            o->sprite = t->unknown100;
            o->palette = t->state_b;
            t->unknown105 = 0xff;
            t->unknown104 = 1;
            t->unknown103 = 200;
        } else {
            t->unknown103 += 10;
        }
        break;
    case 204:
        t = &thing[o->unknown69];
        if (t->unknown103 == 0) {
            w->unknown103++;
            t->unknown105 = 8;
            t->unknown104 = 5;
            t->unknown103 = 200;
        } else {
            t->unknown103 -= 10;
        }
        break;
    case 205:
        t = &thing[o->unknown69];
        if (t->unknown103 == 0) {
            w->unknown103++;
            t->unknown105 = 4;
            t->unknown104 = 0;
            t->unknown103 = 200;
        } else {
            t->unknown103 -= 10;
        }
        break;
    case 206:
        t = &thing[o->unknown69];
        if (t->unknown103 == 0) {
            transform_peep_who(o, 0xa);
            w->state_b = 0x1c;
            set_complex(w, 0x230, 0);
            fn_0003e724(w);
            w->unknown103 = 181;
            o->kind = 0x28;
        } else {
            t->unknown103 -= 10;
        }
        break;
    case 220:
        if (treatment_machine_used(0x2c, o->x.tile + (o->y.tile << 7)))
            break;
        w->state_b = 0x3e;
        set_complex(w, 0x154, 0);
        fn_0003e724(w);
        w->unknown73 = 0;
        w->room->unknown29 = 1;
        remove_person(o);
        break;
    default:
        w->unknown103 = 1;
        set_complex(w, 0xfa, 0);
        fn_0003e724(w);
        done_treatment(w, o);
        break;
    }
}

// FUNCTION: 0x00054444
void effect_illness(Thing *w, u8 illness, u8 cured)
{
    u8 r;

    switch (illness) {
    case 2:
        if (cured == 1) {
            w->layers[0] -= 5;
        } else {
            w->layers[0] += 5;
            w->layers[2] = 0;
        }
        break;
    case 3:
        if (cured == 1) {
            transform_peep_who(w, 2);
        } else {
            w->layers[0] = 1;
            transform_peep_who(w, 1);
            w->layers[2] = 0;
        }
        break;
    case 4:
        if (cured == 1) {
            transform_peep_who(w, 6);
        } else {
            w->layers[0] = 1;
            transform_peep_who(w, 5);
            w->layers[2] = 0;
        }
        break;
    case 15:
        if (cured == 0)
            transform_peep_who(w, 3);
        break;
    case 7:
        if (cured == 1) {
            if (w->subtype == 1)
                w->layers[0] -= 4;
            else
                transform_peep_who(w, 8);
        } else {
            if (w->subtype == 1)
                w->layers[0] += 4;
            else
                transform_peep_who(w, 7);
        }
        break;
    case 5:
        if (cured == 1) {
            transform_peep_who(w, 0xa);
        } else {
            w->layers[0] = 1;
            transform_peep_who(w, 9);
        }
        break;
    case 10:
        if (cured == 1) {
            w->layers[0] = 7;
        } else {
            transform_peep_who(w, 7);
            w->layers[0] = 6;
        }
        break;
    case 9:
        if (w->subtype == 1) {
            if (cured == 1) {
                w->layers[2] = 0;
                w->layers[3] = 0;
            } else {
                r = old_newrand() % 3 + 1;
                w->layers[2] = r & 1;
                w->layers[3] = (r >> 1) & 1;
            }
        } else {
            if (cured == 1) {
                transform_peep_who(w, 0xe);
            } else {
                transform_peep_who(w, 0xd);
                w->layers[2] = old_newrand() & 3;
                w->layers[3] = old_newrand() % 6;
                w->layers[4] = old_newrand() % 6;
            }
        }
        break;
    case 8:
        if (cured == 1) {
            w->subtype = w->unknown170;
        } else {
            w->layers[0] = 1;
            w->unknown170 = w->subtype;
            w->subtype = 0xe;
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
        w->style = illness;
    } else {
        allhosps[w->owner].f1e9++;
        if (w->owner == player)
            advisor.f128 |= 0x40;
        allhosps[w->owner].rooms[w->style].unknown1E++;
        reputation_affected(w->style, 0, 0, w->owner, 0);
        w->unknown169 = w->style = 0;
        if (w->unknown174)
            clean_contagious_peep(w);
        if (w->owner == player)
            mycontrol->fc++;
        if (w->flags & 0x40) {
            allhosps[w->owner].f1e5++;
            w->flags &= ~0x40;
            reset_icon_value(w, 6);
        }
        record_person_as_patient(w);
    }
}
