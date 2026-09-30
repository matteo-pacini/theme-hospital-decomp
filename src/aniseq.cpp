#include "hosp.h"

extern s16 ani_seq[];

void setup_next_aniseq(Thing *w);

// FUNCTION: 0x0009b300
void setup_aniseq_indices(void)
{
    u32 n;
    u32 i;

    n = 0;
    i = 0;
    while (ani_seq[i] != -2) {
        ++i;
        ++n;
        statvar.aniseq_start[n] = i;
        while (ani_seq[i] != -1 && ani_seq[i] != -2)
            i++;
    }
}

// FUNCTION: 0x0009b380
void setup_aniseq(Thing *w, u8 seq, u8 mode)
{
    if (w->renderer == 4)
        w->kind = 0x50;
    w->state_b = mode;
    w->unknown99 = statvar.aniseq_start[seq];
    setup_next_aniseq(w);
}

// FUNCTION: 0x0009b3d8
void setup_next_aniseq(Thing *w)
{
    if (ani_seq[w->unknown99] == -1) {
        w->kind = w->state_b;
    } else {
        u16 f;

        f = check_frame(ani_seq[w->unknown99], w) + w->unknown17;
        w->sprite = start[f].frame;
        w->palette = start[f].flags > 0 ? (u8)1 : (u8)0;
        w->unknown99++;
    }
}

// FUNCTION: 0x0009b498
void play_aniseq(Thing *w)
{
    u16 next;

    if (actual_game_turn % w->anim_speed == 0) {
        next = frame[w->sprite].next;
        if (frame[next].flags & 1)
            setup_next_aniseq(w);
        else
            w->sprite = next;
    }
}

// FUNCTION: 0x0009b510
void choose_aniseq(Thing *w, u8 group, u8 mode)
{
    u8 n;

    for (n = 0; ani_seq_choice[group][n]; n++)
        ;
    setup_aniseq(w, ani_seq_choice[group][old_newrand() % n], mode);
}
