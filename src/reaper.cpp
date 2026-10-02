#include "hosp.h"

extern s16 dying_place[];
extern s16 reap_arrive[];
extern s16 gate_arrive[];

u8 can_place_object(u16 tile, u16 kind, u8 dir, u8 owner);
u8 corridor_object_nav_check(u16 pos, u8 kind, u8 dir);
void reaper_leaves_normally(Thing *reaper);

extern "C" {
s32 remove_object(Thing *obj, u8 owner);
#pragma aux remove_object "W?remove_object$n(pn$Thing$$)s"
void nav_init(Thing *person);
}

// Summons the grim reaper for a dead patient: raises the gates of hell on a
// reachable tile near them and places the reaper near the gates, linked to
// the victim through slave. Returns 0 if a reaper is already coming or no
// place was found; 1 otherwise, including when no thing was free.
// FUNCTION: 0x0005c230
u8 summon_the_reaper(Thing *victim)
{
    Thing *reaper;
    Thing *gate;
    u16 victim_pos;
    u16 reaper_pos;
    u16 gate_pos;
    u16 i;
    u16 gate_id;
    u16 found;

    gate_id = 0;
    found = 0;
    reaper = &thing[victim->slave];
    if (reaper > thing) {
        if (reaper->type == THING_REAPER)
            return 0;
    }
    reaper = allocate_thing(0x32);
    if (reaper) {
        reaper->type = THING_REAPER;
        victim_pos = (u16)victim->x.tile + ((u16)victim->y.tile << 7);
        for (i = 1; i <= gate_arrive[0]; i++) {
            gate_pos = victim_pos + gate_arrive[i];
            if (can_i_get_to(victim, gate_pos) > 0) {
                if (can_place_object(gate_pos, OBJ_GATES_OF_HELL, 0, victim->owner)) {
                    if (corridor_object_nav_check(gate_pos, OBJ_GATES_OF_HELL, 0)) {
                        gate_id = place_object(gate_pos, OBJ_GATES_OF_HELL, 0, victim->owner);
                        if (gate_id) {
                            gate = &thing[gate_id];
                            gate->aux_sprite = start[0x4fa].frame;
                            gate->aux_flip_w = start[0x4fa].flags;
                            if (can_i_get_to(victim, gate->use_pos[1]) == 0) {
                                remove_object(gate, gate->owner);
                                gate_id = 0;
                            } else {
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (gate_id) {
            for (i = 1; i <= reap_arrive[0]; i++) {
                reaper_pos = gate_pos + reap_arrive[i];
                reaper->x.tile = reaper_pos % 128;
                reaper->y.tile = reaper_pos / 128;
                if (can_i_get_to(reaper, gate->use_pos[0]) > 0) {
                    found = 1;
                    break;
                }
            }
            if (found) {
                reaper->gate = gate_id;
                reaper->x.tile = reaper_pos % 128;
                reaper->y.tile = reaper_pos / 128;
                reaper->x.frac = 0x80;
                reaper->y.frac = 0x80;
                reaper->speed = 0x10;
                reaper->anim_speed = 2;
                reaper->facing = 1;
                insert_node(reaper, reaper->x, reaper->y);
                reaper->slave = victim - thing;
                victim->slave = reaper - thing;
                reaper->sprite = start[check_frame(0x33d, reaper) + reaper->facing].frame;
                reaper->flip = start[check_frame(0x33d, reaper) + reaper->facing].flags > 0 ? (u8)1 : (u8)0;
                reaper->state = REAPER_PLAY_ANIM;
                reaper->timer = 0x5dc;
                reaper->dest_x = gate->use_pos[0] % 128;
                reaper->dest_y = gate->use_pos[0] / 128;
                reaper->countdown = 0;
                reaper->reaper_next_state = REAPER_GOTO_HELL;
                reaper->next_sprite = start[check_frame(0x1f1, reaper) + reaper->facing].frame;
                reaper->next_flip = start[check_frame(0x1f1, reaper) + reaper->facing].flags > 0 ? (u8)1 : (u8)0;
                nav_init(reaper);
            } else {
                remove_object(gate, gate->owner);
                reaper->type = THING_FREE;
                return 0;
            }
        } else {
            reaper->type = THING_FREE;
            return 0;
        }
    }
    return 1;
}

// Reaper state REAPER_PLAY_ANIM: plays the current animation, then enters
// reaper_next_state with next_sprite, or leaves.
// FUNCTION: 0x0005c6f8
void reaper_play_anim(Thing *reaper)
{
    CompFrame *cur_frame;

    if (actual_game_turn % reaper->anim_speed == 0) {
        reaper->sprite = frame[reaper->sprite].next;
        cur_frame = &frame[reaper->sprite];
        if (cur_frame->flags & 1) {
            if (reaper->reaper_next_state) {
                reaper->state = reaper->reaper_next_state;
                if (reaper->next_sprite) {
                    reaper->sprite = reaper->next_sprite;
                    reaper->flip = reaper->next_flip;
                }
            } else {
                reaper_leaves_normally(reaper);
            }
        }
    }
}

// The reaper has reached the gates (from REAPER_GOTO_HELL): turns to call the
// victim, who walks to the gates (PS_AT_HELL_GATE on arrival), and waits
// (REAPER_WAITS).
// FUNCTION: 0x0005c7b4
void reaper_calls_victim(Thing *reaper)
{
    Thing *victim;
    Thing *gate;

    reaper->facing = 0;
    reaper->sprite = start[check_frame(0x1f5, reaper) + reaper->facing].frame;
    reaper->flip = start[check_frame(0x1f5, reaper) + reaper->facing].flags > 0 ? (u8)1 : (u8)0;
    reaper->state = REAPER_WAITS;
    victim = &thing[reaper->slave];
    if (victim > thing) {
        gate = &thing[reaper->gate];
        if (gate > thing) {
            if (set_complex(victim, 0x136, gate->use_pos[1])) {
                victim->depth = 0;
                victim->object = reaper->gate;
                victim->next_state = PS_AT_HELL_GATE;
                process_person_complex(victim);
            }
        }
    }
}

// Reaper state REAPER_WAITS: swings at the victim when they reach the gates,
// removes them once they have gone in (PS_AT_HELL_GATE), and leaves if they do
// not come before the timer runs out.
// FUNCTION: 0x0005c8e0
void reaper_waits(Thing *reaper)
{
    CompFrame *cur_frame;
    Thing *victim;
    Thing *gate;

    if (actual_game_turn % reaper->anim_speed == 0) {
        reaper->sprite = frame[reaper->sprite].next;
        cur_frame = &frame[reaper->sprite];
        if (cur_frame->flags & 1) {
            victim = &thing[reaper->slave];
            if (victim > thing) {
                if (victim->state == PS_AT_HELL_GATE) {
                    reaper_leaves_normally(reaper);
                    remove_person(victim);
                } else {
                    gate = &thing[reaper->gate];
                    if (gate > thing) {
                        if (((s16)victim->y.tile << 7) + (s16)victim->x.tile == gate->use_pos[1]) {
                            reaper->sprite = start[check_frame(0x343, reaper) + reaper->facing].frame;
                            reaper->flip = start[check_frame(0x343, reaper) + reaper->facing].flags > 0 ? (u8)1 : (u8)0;
                            reaper->state = REAPER_PLAY_ANIM;
                            reaper->reaper_next_state = REAPER_WAITS;
                            reaper->next_sprite = start[check_frame(0x1f5, reaper) + reaper->facing].frame;
                            reaper->next_flip =
                                start[check_frame(0x1f5, reaper) + reaper->facing].flags > 0 ? (u8)1 : (u8)0;
                        } else if (--reaper->timer <= 0 || victim->next_state != PS_AT_HELL_GATE) {
                            reaper_leaves_normally(reaper);
                        }
                    }
                }
            } else {
                reaper_leaves_normally(reaper);
            }
        }
    }
}

// Starts the reaper's leaving animation, after which it is removed
// (REAPER_REMOVE).
// FUNCTION: 0x0005cb34
void reaper_leaves_normally(Thing *reaper)
{
    reaper->facing = 0;
    reaper->sprite = start[check_frame(0x347, reaper) + reaper->facing].frame;
    reaper->flip = start[check_frame(0x347, reaper) + reaper->facing].flags > 0 ? (u8)1 : (u8)0;
    reaper->state = REAPER_PLAY_ANIM;
    reaper->reaper_next_state = REAPER_REMOVE;
    reaper->next_sprite = 0;
    reaper->next_flip = 0;
}

// Removes the reaper and makes its gates of hell sink away (OS_HELL, then
// removed when the animation ends).
// FUNCTION: 0x0005cbe4
void remove_reaper(Thing *reaper)
{
    Thing *gate;

    gate = &thing[reaper->gate];
    if (gate > thing) {
        gate->state = OS_HELL;
        gate->aux_sprite = start[0x4fc].frame;
        gate->aux_flip_w = start[0x4fc].flags;
        gate->remove_after_anim = 1;
    }
    remove_thing(reaper);
}
