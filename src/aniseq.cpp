#include "hosp.h"

extern s16 ani_seq[];

void setup_next_aniseq(Thing *w);

// FUNCTION: 0x0009b300
void setup_aniseq_indices(void)
{
    u32 seq;
    u32 i;

    seq = 0;
    i = 0;
    while (ani_seq[i] != -2) {
        ++i;
        ++seq;
        statvar.aniseq_start[seq] = i;
        while (ani_seq[i] != -1 && ani_seq[i] != -2)
            i++;
    }
}

// FUNCTION: 0x0009b380
void setup_aniseq(Thing *w, u8 seq, u8 mode)
{
    if (w->type == 4)
        w->state = 0x50;
    w->next_state = mode;
    w->seq_index = statvar.aniseq_start[seq];
    setup_next_aniseq(w);
}

// FUNCTION: 0x0009b3d8
void setup_next_aniseq(Thing *w)
{
    if (ani_seq[w->seq_index] == -1) {
        w->state = w->next_state;
    } else {
        u16 entry;

        entry = check_frame(ani_seq[w->seq_index], w) + w->facing;
        w->sprite = start[entry].frame;
        w->flip = start[entry].flags > 0 ? (u8)1 : (u8)0;
        w->seq_index++;
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
    u8 count;

    for (count = 0; ani_seq_choice[group][count]; count++)
        ;
    setup_aniseq(w, ani_seq_choice[group][old_newrand() % count], mode);
}
