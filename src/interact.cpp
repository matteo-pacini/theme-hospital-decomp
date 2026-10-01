#include "hosp.h"

extern "C" {
extern DoorAnim D_0012b598[];

void fn_000549bc(Thing *w);
}

// FUNCTION: 0x00054870
extern "C" void fn_00054870(Thing *w)
{
    Thing *partner;
    CompFrame *fr;
    u16 next_frame;

    if (w->f64 != 0) {
        if (actual_game_turn % w->anim_speed == 0) {
            next_frame = frame[w->sprite].next;
            w->sprite = next_frame;
            fr = &frame[next_frame];
            if (fr->flags & 1)
                w->f64 = 2;
        }
    }
    partner = &thing[w->unknown73];
    if (partner > thing) {
        if (partner->f64 != 0) {
            if (actual_game_turn % partner->anim_speed == 0) {
                next_frame = frame[partner->sprite].next;
                partner->sprite = next_frame;
                fr = &frame[next_frame];
                if (fr->flags & 1)
                    partner->f64 = 2;
            }
        }
    }
    if (w->f64 != 1 && partner->f64 != 1)
        fn_000549bc(w);
}

// FUNCTION: 0x000549bc
extern "C" void fn_000549bc(Thing *w)
{
    u8 idx;
    s16 anim;
    Thing *partner;

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
            partner = &thing[w->unknown73];
            if (partner > thing) {
                partner->unknown99 = 0;
                partner->f64 = 0;
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
    partner = &thing[w->unknown73];
    if (partner > thing) {
        anim = D_0012b598[idx].other;
        if (anim) {
            partner->f64 = 1;
            if (partner->renderer == 2) {
                u16 f;

                f = check_frame(anim_starts[partner->subtype].anims[anim], partner) + partner->unknown17;
                partner->sprite = start[f].frame;
                partner->palette = start[f].flags > 0;
            } else {
                u16 f;

                f = check_frame(staff_anim_starts[partner->subtype].anims[anim], partner) + partner->unknown17;
                partner->sprite = start[f].frame;
                partner->palette = start[f].flags > 0;
            }
        } else {
            partner->f64 = 0;
        }
    }
}

// FUNCTION: 0x00054d60
extern "C" void fn_00054d60(void)
{
    s16 seq;
    s16 i;

    seq = 0;
    i = 0;
    statvar.unknowne8[0] = 0;
    for (;;) {
        if (D_0012b598[i].v == -1) {
            statvar.unknowne8[++seq] = i + 1;
        } else if (D_0012b598[i].v == -99) {
            break;
        }
        i++;
    }
}

// FUNCTION: 0x00054dd4
void start_new_interaction(Thing *w, u8 interaction)
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
    w->unknown99 = statvar.unknowne8[interaction] - 1;
    w->label[1] = 0;
    fn_000549bc(w);
}
