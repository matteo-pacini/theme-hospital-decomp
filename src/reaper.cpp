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

// NONMATCHING: register choice; gate_arrive[0]/reap_arrive[0] relocs named as the preceding table
// FUNCTION: 0x0005c230
u8 summon_the_reaper(Thing *w)
{
    Thing *r;
    Thing *obj;
    u16 base;
    u16 found;
    u16 pos;
    u16 pos2;
    u16 i;
    u16 obj_id;

    obj_id = 0;
    found = 0;
    r = &thing[w->unknown73];
    if (r > thing && r->renderer == 10)
        return 0;
    r = allocate_thing(0x32);
    if (r) {
        r->renderer = 10;
        base = ((u16)w->y.tile << 7) + (u16)w->x.tile;
        for (i = 1; i <= gate_arrive[0]; i++) {
            pos = base + gate_arrive[i];
            if (can_i_get_to(w, pos) > 0) {
                if (can_place_object(pos, 0x30, 0, w->owner)) {
                    if (corridor_object_nav_check(pos, 0x30, 0)) {
                        obj_id = fn_00037098(pos, 0x30, 0, w->owner);
                        if (obj_id) {
                            obj = &thing[obj_id];
                            obj->unknown100__ = start[0x4fa].frame;
                            obj->unknown102 = start[0x4fa].flags;
                            if (can_i_get_to(w, obj->room_next) == 0) {
                                fn_00038d04(obj, obj->owner);
                                obj_id = 0;
                            } else {
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (obj_id) {
            for (i = 1; i <= reap_arrive[0]; i++) {
                pos2 = pos + reap_arrive[i];
                r->x.tile = pos2 % 128;
                r->y.tile = pos2 / 128;
                if (can_i_get_to(r, obj->room_prev) > 0) {
                    found = 1;
                    break;
                }
            }
            if (found) {
                r->gate = obj_id;
                r->x.tile = pos2 % 128;
                r->y.tile = pos2 / 128;
                r->x.frac = 0x80;
                r->y.frac = 0x80;
                r->unknown18 = 0x10;
                r->anim_speed = 2;
                r->unknown17 = 1;
                insert_node(r, r->x, r->y);
                r->unknown73 = w - thing;
                w->unknown73 = r - thing;
                r->sprite = start[check_frame(0x33d, r) + r->unknown17].frame;
                r->palette = start[check_frame(0x33d, r) + r->unknown17].flags > 0 ? (u8)1 : (u8)0;
                r->kind = 2;
                r->timer = 0x5dc;
                r->unknown19 = obj->room_prev % 128;
                r->unknown20 = obj->room_prev / 128;
                r->height = 0;
                r->next_kind = 8;
                r->next_sprite = start[check_frame(0x1f1, r) + r->unknown17].frame;
                r->next_palette = start[check_frame(0x1f1, r) + r->unknown17].flags > 0 ? (u8)1 : (u8)0;
                fn_000444e0(r);
            } else {
                fn_00038d04(obj, obj->owner);
                r->renderer = 0;
                return 0;
            }
        } else {
            r->renderer = 0;
            return 0;
        }
    }
    return 1;
}

// FUNCTION: 0x0005c6f8
void reaper_play_anim(Thing *w)
{
    CompFrame *f;

    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
        f = &frame[w->sprite];
        if (f->flags & 1) {
            if (w->next_kind) {
                w->kind = w->next_kind;
                if (w->next_sprite) {
                    w->sprite = w->next_sprite;
                    w->palette = w->next_palette;
                }
            } else {
                reaper_leaves_normally(w);
            }
        }
    }
}

// FUNCTION: 0x0005c7b4
void reaper_calls_victim(Thing *w)
{
    Thing *victim;
    Thing *gate;

    w->unknown17 = 0;
    w->sprite = start[check_frame(0x1f5, w) + w->unknown17].frame;
    w->palette = start[check_frame(0x1f5, w) + w->unknown17].flags > 0 ? (u8)1 : (u8)0;
    w->kind = 4;
    victim = &thing[w->unknown73];
    if (victim > thing) {
        gate = &thing[w->gate];
        if (gate > thing) {
            if (set_complex(victim, 0x136, gate->room_next)) {
                victim->depth = 0;
                victim->unknown69 = w->gate;
                victim->state_b = 0x42;
                process_person_complex(victim);
            }
        }
    }
}

// FUNCTION: 0x0005c8e0
void reaper_waits(Thing *w)
{
    CompFrame *f;
    Thing *victim;
    Thing *gate;

    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
        f = &frame[w->sprite];
        if (f->flags & 1) {
            victim = &thing[w->unknown73];
            if (victim > thing) {
                if (victim->kind == 0x42) {
                    reaper_leaves_normally(w);
                    remove_person(victim);
                } else {
                    gate = &thing[w->gate];
                    if (gate > thing) {
                        if (((s16)victim->y.tile << 7) + (s16)victim->x.tile == gate->room_next) {
                            w->sprite = start[check_frame(0x343, w) + w->unknown17].frame;
                            w->palette = start[check_frame(0x343, w) + w->unknown17].flags > 0 ? (u8)1 : (u8)0;
                            w->kind = 2;
                            w->next_kind = 4;
                            w->next_sprite = start[check_frame(0x1f5, w) + w->unknown17].frame;
                            w->next_palette = start[check_frame(0x1f5, w) + w->unknown17].flags > 0 ? (u8)1 : (u8)0;
                        } else if (--w->timer <= 0 || victim->state_b != 0x42) {
                            reaper_leaves_normally(w);
                        }
                    }
                }
            } else {
                reaper_leaves_normally(w);
            }
        }
    }
}

// FUNCTION: 0x0005cb34
void reaper_leaves_normally(Thing *w)
{
    w->unknown17 = 0;
    w->sprite = start[check_frame(0x347, w) + w->unknown17].frame;
    w->palette = start[check_frame(0x347, w) + w->unknown17].flags > 0 ? (u8)1 : (u8)0;
    w->kind = 2;
    w->next_kind = 6;
    w->next_sprite = 0;
    w->next_palette = 0;
}

// FUNCTION: 0x0005cbe4
void remove_reaper(Thing *w)
{
    Thing *gate;

    gate = &thing[w->gate];
    if (gate > thing) {
        gate->kind = 6;
        gate->unknown100__ = start[0x4fc].frame;
        gate->unknown102 = start[0x4fc].flags;
        gate->unknown104w = 1;
    }
    remove_thing(w);
}
