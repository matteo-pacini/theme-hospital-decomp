#include "hosp.h"

extern "C" {
extern DoorAnim interaction[];

void setup_next_interaction(Thing *w);
}

// Plays one turn of w's interaction with its partner (slave): advances both
// animations, and once neither is still playing (interact_anim 1) moves on to
// the next step.
// FUNCTION: 0x00054870
extern "C" void two_peeps_interact(Thing *w)
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
        setup_next_interaction(w);
}

// Starts the next step of w's interaction sequence (repeating the current step
// interact_repeat more times first): plays the step's animations on w and its
// partner (0 for none). At the sequence's end (-1) clears both and continues
// w's complex.
// FUNCTION: 0x000549bc
extern "C" void setup_next_interaction(Thing *w)
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
        w->interact_repeat = interaction[step].count - 1;
    }
    anim = interaction[step].v;
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
            case THING_PATIENT:
                process_person_complex(w);
                break;
            case THING_STAFF:
                process_staff_complex(w);
                break;
            }
            return;
        }
        w->interact_anim = 1;
        if (w->type == THING_STAFF) {
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
        anim = interaction[step].other;
        if (anim) {
            partner->interact_anim = 1;
            if (partner->type == THING_PATIENT) {
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

// Records in statvar.unknowne8 where each interaction sequence starts in
// interaction[] (sequences end with -1, the table with -99).
// FUNCTION: 0x00054d60
extern "C" void setup_interactions(void)
{
    s16 seq;
    s16 i;

    seq = 0;
    i = 0;
    statvar.unknowne8[0] = 0;
    for (;;) {
        if (interaction[i].v == -1) {
            statvar.unknowne8[++seq] = i + 1;
        } else if (interaction[i].v == -99) {
            break;
        }
        i++;
    }
}

// Starts interaction sequence seq (statvar.unknowne8 index) for w (state 0x22
// for a patient, 0x1a for staff) with its partner w->slave.
// FUNCTION: 0x00054dd4
void start_new_interaction(Thing *w, u8 seq)
{
    switch (w->type) {
    case THING_PATIENT:
        w->state = 0x22;
        break;
    case THING_STAFF:
        w->state = 0x1a;
        break;
    default:
        return;
    }
    w->seq_index = statvar.unknowne8[seq] - 1;
    w->interact_repeat = 0;
    setup_next_interaction(w);
}
