#include "hosp.h"

extern "C" {
extern DoorAnim D_0012b598[];

void fn_000549bc(Thing *w);
}

// FUNCTION: 0x00054870
extern "C" void fn_00054870(Thing *w)
{
    Thing *o;
    CompFrame *fr;
    u16 f;

    if (w->f64 != 0) {
        if (actual_game_turn % w->anim_speed == 0) {
            f = frame[w->sprite].next;
            w->sprite = f;
            fr = &frame[f];
            if (fr->flags & 1)
                w->f64 = 2;
        }
    }
    o = &thing[w->unknown73];
    if (o > thing) {
        if (o->f64 != 0) {
            if (actual_game_turn % o->anim_speed == 0) {
                f = frame[o->sprite].next;
                o->sprite = f;
                fr = &frame[f];
                if (fr->flags & 1)
                    o->f64 = 2;
            }
        }
    }
    if (w->f64 != 1 && o->f64 != 1)
        fn_000549bc(w);
}

// FUNCTION: 0x000549bc
extern "C" void fn_000549bc(Thing *w)
{
    u8 idx;
    s16 anim;
    Thing *o;

    if (w->f65 > 0) {
        idx = w->unknown99;
        w->f65--;
    } else {
        w->unknown99++;
        idx = w->unknown99;
        w->f65 = D_0012b598[idx].count - 1;
    }
    anim = D_0012b598[idx].v;
    if (anim) {
        if (anim == -1) {
            w->unknown99 = 0;
            w->f64 = 0;
            o = &thing[w->unknown73];
            if (o > thing) {
                o->unknown99 = 0;
                o->f64 = 0;
            }
            switch (w->renderer) {
            case 2:
                process_person_complex(w);
                break;
            case 4:
                fn_0003e724(w);
                break;
            }
            return;
        }
        w->f64 = 1;
        if (w->renderer == 4) {
            u16 f;

            f = check_frame(staff_anim_starts[w->subtype].anims[anim], w) + w->unknown17;
            w->sprite = start[f].frame;
            w->palette = start[f].flags > 0;
        } else {
            u16 f;

            f = check_frame(anim_starts[w->subtype].anims[anim], w) + w->unknown17;
            w->sprite = start[f].frame;
            w->palette = start[f].flags > 0;
        }
    } else {
        w->f64 = 0;
    }
    o = &thing[w->unknown73];
    if (o > thing) {
        anim = D_0012b598[idx].other;
        if (anim) {
            o->f64 = 1;
            if (o->renderer == 2) {
                u16 f;

                f = check_frame(anim_starts[o->subtype].anims[anim], o) + o->unknown17;
                o->sprite = start[f].frame;
                o->palette = start[f].flags > 0;
            } else {
                u16 f;

                f = check_frame(staff_anim_starts[o->subtype].anims[anim], o) + o->unknown17;
                o->sprite = start[f].frame;
                o->palette = start[f].flags > 0;
            }
        } else {
            o->f64 = 0;
        }
    }
}

// FUNCTION: 0x00054d60
extern "C" void fn_00054d60(void)
{
    s16 n;
    s16 i;

    n = 0;
    i = 0;
    statvar.unknowne8[0] = 0;
    for (;;) {
        if (D_0012b598[i].v == -1) {
            statvar.unknowne8[++n] = i + 1;
        } else if (D_0012b598[i].v == -99) {
            break;
        }
        i++;
    }
}

// FUNCTION: 0x00054dd4
void start_new_interaction(Thing *w, u8 who)
{
    switch (w->renderer) {
    case 2:
        w->kind = 0x22;
        break;
    case 4:
        w->kind = 0x1a;
        break;
    default:
        return;
    }
    w->unknown99 = statvar.unknowne8[who] - 1;
    w->label[1] = 0;
    fn_000549bc(w);
}
