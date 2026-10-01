#include "hosp.h"

extern s16 dying_place[];
extern s16 reap_arrive[];
extern s16 gate_arrive[];

u8 can_place_object(u16 tile, u16 kind, u8 dir, u8 owner);
u8 corridor_object_nav_check(u16 pos, u8 kind, u8 dir);
void reaper_leaves_normally(Thing *w);

extern "C" {
s32 fn_00038d04(Thing *w, u8 owner);
#pragma aux fn_00038d04 "W?remove_object$n(pn$Thing$$)s"
void fn_000444e0(Thing *w);
}

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
    reaper = &thing[victim->unknown73];
    if (reaper > thing) {
        if (reaper->renderer == 10)
            return 0;
    }
    reaper = allocate_thing(0x32);
    if (reaper) {
        reaper->renderer = 10;
        victim_pos = (u16)victim->x.tile + ((u16)victim->y.tile << 7);
        for (i = 1; i <= gate_arrive[0]; i++) {
            gate_pos = victim_pos + gate_arrive[i];
            if (can_i_get_to(victim, gate_pos) > 0) {
                if (can_place_object(gate_pos, 0x30, 0, victim->owner)) {
                    if (corridor_object_nav_check(gate_pos, 0x30, 0)) {
                        gate_id = fn_00037098(gate_pos, 0x30, 0, victim->owner);
                        if (gate_id) {
                            gate = &thing[gate_id];
                            gate->unknown100__ = start[0x4fa].frame;
                            gate->unknown102 = start[0x4fa].flags;
                            if (can_i_get_to(victim, gate->room_next) == 0) {
                                fn_00038d04(gate, gate->owner);
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
                if (can_i_get_to(reaper, gate->room_prev) > 0) {
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
                reaper->unknown18 = 0x10;
                reaper->anim_speed = 2;
                reaper->unknown17 = 1;
                insert_node(reaper, reaper->x, reaper->y);
                reaper->unknown73 = victim - thing;
                victim->unknown73 = reaper - thing;
                reaper->sprite = start[check_frame(0x33d, reaper) + reaper->unknown17].frame;
                reaper->palette = start[check_frame(0x33d, reaper) + reaper->unknown17].flags > 0 ? (u8)1 : (u8)0;
                reaper->kind = 2;
                reaper->timer = 0x5dc;
                reaper->unknown19 = gate->room_prev % 128;
                reaper->unknown20 = gate->room_prev / 128;
                reaper->height = 0;
                reaper->next_kind = 8;
                reaper->next_sprite = start[check_frame(0x1f1, reaper) + reaper->unknown17].frame;
                reaper->next_palette = start[check_frame(0x1f1, reaper) + reaper->unknown17].flags > 0 ? (u8)1 : (u8)0;
                fn_000444e0(reaper);
            } else {
                fn_00038d04(gate, gate->owner);
                reaper->renderer = 0;
                return 0;
            }
        } else {
            reaper->renderer = 0;
            return 0;
        }
    }
    return 1;
}

// FUNCTION: 0x0005c6f8
void reaper_play_anim(Thing *reaper)
{
    CompFrame *cur_frame;

    if (actual_game_turn % reaper->anim_speed == 0) {
        reaper->sprite = frame[reaper->sprite].next;
        cur_frame = &frame[reaper->sprite];
        if (cur_frame->flags & 1) {
            if (reaper->next_kind) {
                reaper->kind = reaper->next_kind;
                if (reaper->next_sprite) {
                    reaper->sprite = reaper->next_sprite;
                    reaper->palette = reaper->next_palette;
                }
            } else {
                reaper_leaves_normally(reaper);
            }
        }
    }
}

// FUNCTION: 0x0005c7b4
void reaper_calls_victim(Thing *reaper)
{
    Thing *victim;
    Thing *gate;

    reaper->unknown17 = 0;
    reaper->sprite = start[check_frame(0x1f5, reaper) + reaper->unknown17].frame;
    reaper->palette = start[check_frame(0x1f5, reaper) + reaper->unknown17].flags > 0 ? (u8)1 : (u8)0;
    reaper->kind = 4;
    victim = &thing[reaper->unknown73];
    if (victim > thing) {
        gate = &thing[reaper->gate];
        if (gate > thing) {
            if (set_complex(victim, 0x136, gate->room_next)) {
                victim->depth = 0;
                victim->unknown69 = reaper->gate;
                victim->state_b = 0x42;
                process_person_complex(victim);
            }
        }
    }
}

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
            victim = &thing[reaper->unknown73];
            if (victim > thing) {
                if (victim->kind == 0x42) {
                    reaper_leaves_normally(reaper);
                    remove_person(victim);
                } else {
                    gate = &thing[reaper->gate];
                    if (gate > thing) {
                        if (((s16)victim->y.tile << 7) + (s16)victim->x.tile == gate->room_next) {
                            reaper->sprite = start[check_frame(0x343, reaper) + reaper->unknown17].frame;
                            reaper->palette = start[check_frame(0x343, reaper) + reaper->unknown17].flags > 0 ? (u8)1 : (u8)0;
                            reaper->kind = 2;
                            reaper->next_kind = 4;
                            reaper->next_sprite = start[check_frame(0x1f5, reaper) + reaper->unknown17].frame;
                            reaper->next_palette = start[check_frame(0x1f5, reaper) + reaper->unknown17].flags > 0 ? (u8)1 : (u8)0;
                        } else if (--reaper->timer <= 0 || victim->state_b != 0x42) {
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

// FUNCTION: 0x0005cb34
void reaper_leaves_normally(Thing *reaper)
{
    reaper->unknown17 = 0;
    reaper->sprite = start[check_frame(0x347, reaper) + reaper->unknown17].frame;
    reaper->palette = start[check_frame(0x347, reaper) + reaper->unknown17].flags > 0 ? (u8)1 : (u8)0;
    reaper->kind = 2;
    reaper->next_kind = 6;
    reaper->next_sprite = 0;
    reaper->next_palette = 0;
}

// FUNCTION: 0x0005cbe4
void remove_reaper(Thing *reaper)
{
    Thing *gate;

    gate = &thing[reaper->gate];
    if (gate > thing) {
        gate->kind = 6;
        gate->unknown100__ = start[0x4fc].frame;
        gate->unknown102 = start[0x4fc].flags;
        gate->unknown104w = 1;
    }
    remove_thing(reaper);
}
