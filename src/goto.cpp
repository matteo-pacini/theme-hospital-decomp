#include "hosp.h"

extern "C" {
extern s8 around_xy[][2];
#pragma aux around_xy "W?around_vxy$n[]$VXY$$"
}

u8 do_reaction(Thing *w);
s16 person_head_for(Thing *w, u8 a);
void join_queue(Thing *w, RoomInfo *room);
void reaper_calls_victim(Thing *w);
void handyman_check_for_litter(Thing *w);
void staff_nav_failure(Thing *w);
extern "C" void fn_00041c0c(Thing *w);
extern "C" void fn_000444e0(Thing *w);
u16 find_free_object_anywhere(u16 id, u8 kind);
u8 should_nurse_vaccinate(Thing *w);
u8 try_staff_wander(Thing *w, s8 dx, s8 dy, u8 state);

#define PLAY_FRAME(t, a) \
    { \
        u16 anim; \
\
        anim = check_frame(a, t) + (u16)(t)->unknown17; \
        (t)->sprite = start[anim].frame; \
        (t)->palette = start[anim].flags > 0 ? (u8)1 : (u8)0; \
    }

// FUNCTION: 0x000763d0
s16 person_goto_destination(Thing *w)
{
    MapPos x;
    MapPos y;
    s16 dir;

    if (actual_game_turn % w->anim_speed == 0)
        w->sprite = frame[w->sprite].next;
    w->height--;
    if (w->height <= 0) {
        if (blocks[map[(s16)w->x.tile + ((s16)w->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(w))
            return 0;
        modify_person(w);
        if (do_reaction(w))
            return 0;
        w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
        dir = person_head_for(w, 3);
        switch (dir) {
        case 4:
            if (process_person_complex(w) == 2) {
                w->kind = 4;
                w->timer = 200;
            }
            w->height = 0;
            return 0;
        case -1:
            PLAY_FRAME(w, anim_starts[w->subtype].unknown12);
            w->height = 0;
            return 0;
        case -2:
        case -3:
            person_nav_failure(w);
            return 0;
        default:
            if (w->unknown17 != dir || start[anim_starts[w->subtype].unknown12 + w->unknown17].frame == w->sprite) {
                w->unknown17 = dir;
                PLAY_FRAME(w, anim_starts[w->subtype].idle);
            }
            break;
        }
    }
    x.raw = w->x.raw + w->unknown18 * around_xy[w->unknown17][0];
    y.raw = w->y.raw + around_xy[w->unknown17][1] * w->unknown18;
    move_node(w, x, y);
    return 0;
}

// FUNCTION: 0x00076768
s16 person_goto_object(Thing *w)
{
    u8 save;
    MapPos x;
    MapPos y;
    s16 dir;
    Thing *obj;

    if (actual_game_turn % w->anim_speed == 0)
        w->sprite = frame[w->sprite].next;
    w->height--;
    if (w->height <= 0) {
        if (blocks[map[(s16)w->x.tile + ((s16)w->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(w))
            return 0;
        modify_person(w);
        if (w->unknown130 <= 0)
            return 0;
        if (do_reaction(w))
            return 0;
        w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
        dir = person_head_for(w, 1);
        if (dir >= 0 && dir <= 3) {
            if (fn_000cb2db(w->x.tile - w->unknown19) < othervar.f2f && fn_000cb2db(w->y.tile - w->unknown20) < othervar.f2f) {
                obj = &thing[w->unknown69];
                if (obj->unknown98 == 1 || obj->kind == 0x12 || (obj->renderer != 8 && obj->renderer != 6))
                    dir = 4;
            }
        }
        switch (dir) {
        case 4:
            process_person_complex(w);
            w->height = 0;
            return 0;
        case -1:
            PLAY_FRAME(w, anim_starts[w->subtype].unknown12);
            w->height = 0;
            return 0;
        case -2:
            person_nav_failure(w);
            return 0;
        case -3:
            obj = &thing[w->unknown69];
            if (obj > thing) {
                save = obj->unknown98;
                obj->unknown98 = 1;
                process_person_complex(w);
                obj->unknown98 = save;
            } else {
                person_nav_failure(w);
            }
            return 0;
        case 0:
        case 1:
        case 2:
        case 3:
        default:
            if (w->unknown17 != dir || start[anim_starts[w->subtype].unknown12 + w->unknown17].frame == w->sprite) {
                w->unknown17 = dir;
                PLAY_FRAME(w, anim_starts[w->subtype].idle);
            }
            break;
        }
    }
    x.raw = w->x.raw + w->unknown18 * around_xy[w->unknown17][0];
    y.raw = w->y.raw + around_xy[w->unknown17][1] * w->unknown18;
    move_node(w, x, y);
    return 0;
}

// FUNCTION: 0x00076c44
void person_goto_queue(Thing *w)
{
    u16 unused;
    MapPos x;
    MapPos y;
    s16 dir;
    RoomInfo *room;

    unused = 0;
    if (actual_game_turn % w->anim_speed == 0)
        w->sprite = frame[w->sprite].next;
    w->height--;
    if (w->height <= 0) {
        if (blocks[map[(s16)w->x.tile + ((s16)w->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(w))
            return;
        modify_person(w);
        if (w->unknown130 <= 0)
            return;
        if (do_reaction(w))
            return;
        w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
        room = &room_info[w->unknown89];
        dir = person_head_for(w, 3);
        if (dir >= 0 && dir <= 3) {
            if (fn_000cb2db(w->x.tile - w->unknown19) <= othervar.fe && fn_000cb2db(w->y.tile - w->unknown20) <= othervar.fe) {
                if (room->type == 4 || queue_size(room) > 0)
                    dir = 4;
            }
        }
        switch (dir) {
        case 4:
            if (room->type != 4) {
                if (room->type != expertise[w->unknown171].category || room->unknown4 == 0) {
                    if (w->unknown89 && room->unknown4 != 0)
                        room->unknown32--;
                    w->unknown89 = 0;
                    w->unknown61 = 0;
                    w->unknown63 = 0;
                    w->unknown75w = 0;
                    start_person_going(w);
                    return;
                }
                if (fn_000cb2db(w->x.tile - room->unknown4 % 128) > othervar.fe
                    || fn_000cb2db(w->y.tile - room->unknown4 / 128) > othervar.fe) {
                    fn_0004195c(w, room->unknown4);
                    w->height = 0;
                    return;
                }
            }
            w->unknown89 = 0;
            w->unknown61 = 0;
            w->unknown63 = 0;
            w->unknown75w = 0;
            join_queue(w, room);
            if (room->type != 4) {
                page_staff_i_need(w, room);
                w->kind = 0x12;
            } else {
                w->kind = 0x14;
                PLAY_FRAME(w, anim_starts[w->subtype].unknown12);
            }
            w->height = 0;
            return;
        case -1:
            PLAY_FRAME(w, anim_starts[w->subtype].unknown12);
            w->height = 0;
            return;
        case -2:
        case -3:
            person_nav_failure(w);
            return;
        default:
            if (w->unknown17 != dir || start[anim_starts[w->subtype].unknown12 + w->unknown17].frame == w->sprite) {
                w->unknown17 = dir;
                PLAY_FRAME(w, anim_starts[w->subtype].idle);
            }
            break;
        }
    }
    x.raw = w->x.raw + w->unknown18 * around_xy[w->unknown17][0];
    y.raw = w->y.raw + around_xy[w->unknown17][1] * w->unknown18;
    move_node(w, x, y);
}

// FUNCTION: 0x0007724c
s16 reaper_goto_hell(Thing *w)
{
    MapPos x;
    MapPos y;
    s16 dir;

    if (actual_game_turn % w->anim_speed == 0)
        w->sprite = frame[w->sprite].next;
    w->height--;
    if (w->height <= 0) {
        w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
        switch (dir = person_head_for(w, 3)) {
        case 4:
            reaper_calls_victim(w);
            w->height = 0;
            return 0;
        case -1:
            w->sprite = start[check_frame(0x1f5, w) + w->unknown17].frame;
            w->palette = start[check_frame(0x1f5, w) + w->unknown17].flags > 0 ? (u8)1 : (u8)0;
            w->height = 0;
            return 0;
        case -2:
        case -3:
            person_nav_failure(w);
            return 0;
        default:
            if (w->unknown17 != dir) {
                w->unknown17 = dir;
                w->sprite = start[check_frame(0x1f1, w) + w->unknown17].frame;
                w->palette = start[check_frame(0x1f1, w) + w->unknown17].flags > 0 ? (u8)1 : (u8)0;
            }
            break;
        }
    }
    x.raw = w->x.raw + w->unknown18 * around_xy[w->unknown17][0];
    y.raw = w->y.raw + around_xy[w->unknown17][1] * w->unknown18;
    move_node(w, x, y);
    return 0;
}

// FUNCTION: 0x00077508
s16 staff_goto_object(Thing *w)
{
    MapPos x;
    MapPos y;
    s16 dir;

    if (w->subtype == 2)
        handyman_check_for_litter(w);
    if (actual_game_turn % w->anim_speed == 0)
        w->sprite = frame[w->sprite].next;
    w->height--;
    if (w->height <= 0) {
        if (blocks[map[(s16)w->x.tile + ((s16)w->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(w))
            return 0;
        w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
        dir = person_head_for(w, 1);
        switch (dir) {
        case 4:
            w->unknown89 = 0;
            fn_0003e724(w);
            return 0;
        case -1:
            PLAY_FRAME(w, staff_anim_starts[w->subtype].unknownA);
            w->height = 0;
            return 0;
        case -2:
            staff_nav_failure(w);
            return 0;
        case -3:
            if (w->unknown94)
                fn_00041c0c(w);
            else
                staff_nav_failure(w);
            return 0;
        case 0:
        case 1:
        case 2:
        case 3:
        default:
            if (w->unknown17 != dir || start[staff_anim_starts[w->subtype].unknownA + w->unknown17].frame == w->sprite) {
                w->unknown17 = dir;
                PLAY_FRAME(w, staff_anim_starts[w->subtype].idle);
            }
            break;
        }
    }
    x.raw = w->x.raw + w->unknown18 * around_xy[w->unknown17][0];
    y.raw = w->y.raw + around_xy[w->unknown17][1] * w->unknown18;
    move_node(w, x, y);
    return 0;
}

// FUNCTION: 0x000778b4
u8 try_staff_wander(Thing *w, s8 dx, s8 dy, u8 state)
{
    s16 x;
    s16 y;
    u16 tile;
    Map *m;

    x = w->x.tile + dx;
    y = w->y.tile + dy;
    tile = x + (y << 7);
    m = &map[tile];
    if (!(m >= map && m < end_map && !(m->flags & 3) && (x >= 0 && x < 0x80 && y >= 0 && y < 0x80))
        || m->unknown7 != 1 << w->owner || which_room(tile)) {
        w->unknown103 = 6;
        return 0;
    }
    if (!(w->unknown19 + dx >= 0 && w->unknown19 + dx < 0x80 && w->unknown20 + dy >= 0 && w->unknown20 + dy < 0x80)) {
        w->unknown103 = 6;
        return 0;
    }
    w->unknown19 += dx;
    w->unknown20 += dy;
    PLAY_FRAME(w, staff_anim_starts[w->subtype].idle);
    fn_000444e0(w);
    w->height = 1;
    w->unknown103 = state;
    return 1;
}

// FUNCTION: 0x00077ab8
void staff_wanders(Thing *w)
{
    u8 r;
    MapPos y;
    s16 dir;
    MapPos x;

    if (w->subtype == 2) {
        w->kind = 0x3c;
        w->unknown103 = 0;
        return;
    }
    if (w->subtype == 0)
        allhosps[w->owner].unknown11f0++;
    if (shall_i_resign(w))
        return;
    if (w->unknown103 == 0) {
        if (blocks[map[(s16)w->x.tile + ((s16)w->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(w))
            return;
        w->height--;
        if (w->height <= 0)
            goto decide;
    } else {
        if (w->unknown103 == 6)
            goto decide;
        if (actual_game_turn % w->anim_speed == 0)
            w->sprite = frame[w->sprite].next;
        w->height--;
        if (w->height <= 0) {
            if (w->subtype == 3 && listhead[w->owner].rooms[4]) {
                if (find_free_object_anywhere(w - thing, 0xb)) {
                    w->kind = 0xc;
                    return;
                }
            }
            if (blocks[map[(s16)w->x.tile + ((s16)w->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(w))
                return;
            if (shall_i_resign(w))
                return;
            w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
            dir = person_head_for(w, 3);
            switch (dir) {
            case 4:
            case -3:
                w->height = 1;
                goto decide;
            case -1:
                PLAY_FRAME(w, staff_anim_starts[w->subtype].unknownA);
                w->height = 0;
                return;
            case -2:
                staff_nav_failure(w);
                return;
            case 0:
            case 1:
            case 2:
            case 3:
            default:
                if (w->unknown17 != dir) {
                    w->unknown17 = dir;
                    PLAY_FRAME(w, staff_anim_starts[w->subtype].idle);
                }
                break;
            }
        }
        x.raw = w->x.raw + w->unknown18 * around_xy[w->unknown17][0];
        y.raw = w->y.raw + around_xy[w->unknown17][1] * w->unknown18;
        move_node(w, x, y);
    }
    return;
decide:
    w->f8e &= 0xaf;
    if (w->subtype == 0 && should_nurse_vaccinate(w))
        return;
    if ((w->f8e & 1) && take_a_break(w))
        return;
    if (obey_pager(w))
        return;
    if (w->f78 > allhosps[w->owner].f21d) {
        if (take_a_break(w))
            return;
    }
    if (old_newrand() % 100 < 0x55) {
        switch (w->unknown103) {
        case 1:
            try_staff_wander(w, 0, 3, 3);
            return;
        case 3:
            try_staff_wander(w, 0, -3, 1);
            return;
        case 2:
            try_staff_wander(w, 3, 0, 4);
            return;
        case 4:
            try_staff_wander(w, -3, 0, 2);
            return;
        }
    }
    w->unknown103 = 6;
    PLAY_FRAME(w, staff_anim_starts[w->subtype].unknownA);
    r = old_newrand() % 100;
    if (r < 0xf) {
        w->unknown103 = 0;
        w->height = old_newrand() % 30 + 20;
    } else if (r < 0x17) {
        try_staff_wander(w, 0, -3, 1);
    } else if (r < 0x1f) {
        try_staff_wander(w, -3, 0, 2);
    } else if (r < 0x27) {
        try_staff_wander(w, 0, 3, 3);
    } else if (r < 0x2f) {
        try_staff_wander(w, 3, 0, 4);
    } else {
        s8 dx;
        s8 dy;

        dx = old_newrand() % 7 - 3;
        dy = old_newrand() % 7 - 3;
        try_staff_wander(w, dx, dy, 5);
    }
}

// FUNCTION: 0x000781f8
void peep_bp_goto(Thing *w)
{
    u8 vertical;
    u8 retry;
    u8 dir;
    MapPos y;
    MapPos x;

    vertical = 0;
    retry = 0;
    if (actual_game_turn % w->anim_speed == 0)
        w->sprite = frame[w->sprite].next;
    w->height--;
    if (w->height <= 0) {
        if (blocks[map[(s16)w->x.tile + ((s16)w->y.tile << 7)].overlay].width != 0x20
            || (w->x.tile == w->unknown106 && w->y.tile == w->unknown107)) {
            w->kind = w->unknown105;
            return;
        }
        if (w->x.tile == w->unknown106) {
            vertical = 1;
        } else if (w->y.tile == w->unknown107) {
            vertical = 0;
        } else {
            vertical = old_newrand() % 1000 < 500 ? (u8)1 : (u8)0;
            retry = 1;
        }
        for (;;) {
            if (vertical) {
                if (w->y.tile < w->unknown107)
                    dir = 2;
                else
                    dir = 0;
            } else {
                if (w->x.tile < w->unknown106)
                    dir = 1;
                else
                    dir = 3;
            }
            if (!retry || blocks[map[(s16)((u16)w->x.tile + ((u16)w->y.tile << 7)) + around[dir]].overlay].width == 0x20)
                break;
            vertical = !vertical;
            retry = 0;
        }
        w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
        if (w->renderer == 4) {
            if (w->unknown17 != dir || start[staff_anim_starts[w->subtype].unknownA + w->unknown17].frame == w->sprite) {
                w->unknown17 = dir;
                PLAY_FRAME(w, staff_anim_starts[w->subtype].idle);
            }
        } else {
            if (w->unknown17 != dir || start[anim_starts[w->subtype].unknown12 + w->unknown17].frame == w->sprite) {
                w->unknown17 = dir;
                PLAY_FRAME(w, anim_starts[w->subtype].idle);
            }
        }
    }
    x.raw = w->x.raw + w->unknown18 * around_xy[w->unknown17][0];
    y.raw = w->y.raw + around_xy[w->unknown17][1] * w->unknown18;
    move_node(w, x, y);
}

void mayor_inspects(Thing *w);

// FUNCTION: 0x00078640
s16 mayor_goto(Thing *w)
{
    MapPos x;
    MapPos y;
    s16 dir;

    if (actual_game_turn % w->anim_speed == 0)
        w->sprite = frame[w->sprite].next;
    w->height--;
    if (w->height <= 0) {
        w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
        switch (dir = person_head_for(w, 3)) {
        case 4:
            mayor_inspects(w);
            w->kind = w->f6e;
            w->height = 0;
            return 0;
        case -1:
            PLAY_FRAME(w, 0x89);
            w->height = 0;
            return 0;
        case -2:
        case -3:
            w->kind = 6;
            return 0;
        default:
            if (w->unknown17 != dir) {
                w->unknown17 = dir;
                PLAY_FRAME(w, 0x85);
            }
            break;
        }
    }
    x.raw = w->x.raw + w->unknown18 * around_xy[w->unknown17][0];
    y.raw = w->y.raw + around_xy[w->unknown17][1] * w->unknown18;
    move_node(w, x, y);
    return 0;
}

s32 wall_in_way(u32 x, u32 y, u32 dir);

extern "C" {
}

// FUNCTION: 0x000788d8
void peep_stuck(Thing *w)
{
    u8 dir;
    MapPos y;
    MapPos x;
    s32 dest;

    if (actual_game_turn % w->anim_speed == 0)
        w->sprite = frame[w->sprite].next;
    w->height--;
    if (w->height <= 0) {
        if (w->unknown17 != 4) {
            w->height = 100;
            if (w->renderer == 2)
                PLAY_FRAME(w, anim_starts[w->subtype].unknown12)
            else
                PLAY_FRAME(w, staff_anim_starts[w->subtype].unknownA)
            w->unknown17 = 4;
            return;
        }
        dest = (s16)w->unknown106 + ((s16)w->unknown107 << 7);
        if ((w->renderer != 2 || !(w->flags & 0x4000))
            && check_points_connected((u16)w->x.tile + ((u16)w->y.tile << 7), dest)) {
            w->kind = w->unknown105;
            w->unknown19 = w->unknown106;
            w->unknown20 = w->unknown107;
            return;
        }
        if (w->renderer == 2) {
            w->unknown124 += 0xf;
            if (w->unknown124 > gbv.f108) {
                w->unknown124 = 0;
                if (!(w->flags & 0x4000)) {
                    if (w->owner == player)
                        othervar.f89++;
                }
                set_icon_value(w, 7);
                person_go_home(w);
                w->flags |= 0x4000;
                return;
            }
        }
        dir = old_newrand() & 3;
        if (!wall_in_way(w->x.tile, w->y.tile, dir)) {
            w->unknown17 = dir;
            w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
            if (w->renderer == 2)
                PLAY_FRAME(w, anim_starts[w->subtype].idle)
            else
                PLAY_FRAME(w, staff_anim_starts[w->subtype].idle)
        }
    }
    if (w->unknown17 != 4) {
        x.raw = w->x.raw + w->unknown18 * around_xy[w->unknown17][0];
        y.raw = w->y.raw + around_xy[w->unknown17][1] * w->unknown18;
        move_node(w, x, y);
    }
}

struct GraphLabel {
    u8 kind;
    u16 value;
    s32 data;
};

struct GraphRect {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

extern "C" {
extern GraphLabel D_0015eba0[];
extern s16 D_0015eb9c;
void fn_00078d70(s16 kind, s16 value, s32 data);
void fn_00078e38(u16 gap);
}

// FUNCTION: 0x00078d70
void fn_00078d70(s16 kind, s16 value, s32 data)
{
    u8 i;

    i = 0;
    while (D_0015eba0[i].value <= value && i < D_0015eb9c)
        i++;
    memmove(&D_0015eba0[i + 1], &D_0015eba0[i], (D_0015eb9c - i) * 7);
    D_0015eba0[i].kind = kind;
    D_0015eba0[i].value = value;
    D_0015eba0[i].data = data;
    D_0015eb9c++;
}

// FUNCTION: 0x00078e38
void fn_00078e38(u16 gap)
{
    u8 moved;
    s16 i;
    u8 below;
    u8 above;

    moved = 1;
    while (moved) {
        moved = 0;
        for (i = 0; i < D_0015eb9c; i++) {
            below = i > 0 && D_0015eba0[i].value - D_0015eba0[i - 1].value < gap ? (u8)1 : (u8)0;
            above = i < D_0015eb9c - 1 && D_0015eba0[i + 1].value - D_0015eba0[i].value < gap ? (u8)1 : (u8)0;
            if (below && !above) {
                if (D_0015eba0[i].value < resvars.f3c_GraphRectp[46].y)
                    D_0015eba0[i].value++;
                moved = 1;
            }
            if (!below && above) {
                if (D_0015eba0[i].value > resvars.f3c_GraphRectp[46].y - resvars.f3c_GraphRectp[46].h - gap)
                    D_0015eba0[i].value--;
                moved = 1;
            }
        }
    }
}
