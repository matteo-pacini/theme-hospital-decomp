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

    if (w->interact_anim != 0) {
        if (actual_game_turn % w->anim_speed == 0) {
            next_frame = frame[w->sprite].next;
            w->sprite = next_frame;
            fr = &frame[next_frame];
            if (fr->flags & 1)
                w->interact_anim = 2;
        }
    }
    partner = &thing[w->slave];
    if (partner > thing) {
        if (partner->interact_anim != 0) {
            if (actual_game_turn % partner->anim_speed == 0) {
                next_frame = frame[partner->sprite].next;
                partner->sprite = next_frame;
                fr = &frame[next_frame];
                if (fr->flags & 1)
                    partner->interact_anim = 2;
            }
        }
    }
    if (w->interact_anim != 1 && partner->interact_anim != 1)
        fn_000549bc(w);
}

// FUNCTION: 0x000549bc
extern "C" void fn_000549bc(Thing *w)
{
    u8 step;
    s16 anim;
    Thing *partner;

    if (w->interact_repeat > 0) {
        step = w->seq_index;
        w->interact_repeat--;
    } else {
        w->seq_index++;
        step = w->seq_index;
        w->interact_repeat = D_0012b598[step].count - 1;
    }
    anim = D_0012b598[step].v;
    if (anim) {
        if (anim == -1) {
            w->seq_index = 0;
            w->interact_anim = 0;
            partner = &thing[w->slave];
            if (partner > thing) {
                partner->seq_index = 0;
                partner->interact_anim = 0;
            }
            switch (w->type) {
            case 2:
                process_person_complex(w);
                break;
            case 4:
                fn_0003e724(w);
                break;
            }
            return;
        }
        w->interact_anim = 1;
        if (w->type == 4) {
            u16 entry;

            entry = check_frame(staff_anim_starts[w->subtype].anims[anim], w) + w->facing;
            w->sprite = start[entry].frame;
            w->flip = start[entry].flags > 0;
        } else {
            u16 entry;

            entry = check_frame(anim_starts[w->subtype].anims[anim], w) + w->facing;
            w->sprite = start[entry].frame;
            w->flip = start[entry].flags > 0;
        }
    } else {
        w->interact_anim = 0;
    }
    partner = &thing[w->slave];
    if (partner > thing) {
        anim = D_0012b598[step].other;
        if (anim) {
            partner->interact_anim = 1;
            if (partner->type == 2) {
                u16 entry;

                entry = check_frame(anim_starts[partner->subtype].anims[anim], partner) + partner->facing;
                partner->sprite = start[entry].frame;
                partner->flip = start[entry].flags > 0;
            } else {
                u16 entry;

                entry = check_frame(staff_anim_starts[partner->subtype].anims[anim], partner) + partner->facing;
                partner->sprite = start[entry].frame;
                partner->flip = start[entry].flags > 0;
            }
        } else {
            partner->interact_anim = 0;
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
    switch (w->type) {
    case 2:
        w->state = 0x22;
        break;
    case 4:
        w->state = 0x1a;
        break;
    default:
        return;
    }
    w->seq_index = statvar.unknowne8[interaction] - 1;
    w->interact_repeat = 0;
    fn_000549bc(w);
}
