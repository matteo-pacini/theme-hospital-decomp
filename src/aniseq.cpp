#include "hosp.h"

extern s16 ani_seq[];

void setup_next_aniseq(Thing *w);

// Fills statvar.aniseq_start with the ani_seq index of each sequence's first
// entry. ani_seq holds sequences of start[] indexes, each ended by -1; -2 ends
// the table. Sequence numbers start at 1.
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

// Starts animation sequence seq (statvar.aniseq_start index) on w, showing its
// first frame. A staff member is put into SS_PLAY_ANISEQ; w enters next_state
// once the sequence ends (play_aniseq).
// FUNCTION: 0x0009b380
void setup_aniseq(Thing *w, u8 seq, u8 next_state)
{
    if (w->type == THING_STAFF)
        w->state = SS_PLAY_ANISEQ;
    w->next_state = next_state;
    w->seq_index = statvar.aniseq_start[seq];
    setup_next_aniseq(w);
}

// Shows the next animation of w's sequence, offset by its facing, and advances
// seq_index; at the sequence's end (-1) w enters next_state instead.
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

// Advances w's sequence by one frame every anim_speed turns, moving on to the
// sequence's next animation when the current one would wrap back to its
// first frame (frame flag 1).
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

// Starts a random sequence from ani_seq_choice[group] (a zero-ended list of
// sequence numbers) on w, as setup_aniseq does.
// FUNCTION: 0x0009b510
void choose_aniseq(Thing *w, u8 group, u8 next_state)
{
    u8 count;

    for (count = 0; ani_seq_choice[group][count]; count++)
        ;
    setup_aniseq(w, ani_seq_choice[group][old_newrand() % count], next_state);
}
