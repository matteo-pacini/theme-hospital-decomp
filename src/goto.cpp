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
s16 person_goto_destination(Thing *person)
{
    MapPos x;
    MapPos y;
    s16 dir;

    if (actual_game_turn % person->anim_speed == 0)
        person->sprite = frame[person->sprite].next;
    person->height--;
    if (person->height <= 0) {
        if (blocks[map[(s16)person->x.tile + ((s16)person->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(person))
            return 0;
        modify_person(person);
        if (do_reaction(person))
            return 0;
        person->height = person->unknown18 == 0 ? div_err() : 0x100 / person->unknown18;
        dir = person_head_for(person, 3);
        switch (dir) {
        case 4:
            if (process_person_complex(person) == 2) {
                person->kind = 4;
                person->timer = 200;
            }
            person->height = 0;
            return 0;
        case -1:
            PLAY_FRAME(person, anim_starts[person->subtype].unknown12);
            person->height = 0;
            return 0;
        case -2:
        case -3:
            person_nav_failure(person);
            return 0;
        default:
            if (person->unknown17 != dir || start[anim_starts[person->subtype].unknown12 + person->unknown17].frame == person->sprite) {
                person->unknown17 = dir;
                PLAY_FRAME(person, anim_starts[person->subtype].idle);
            }
            break;
        }
    }
    x.raw = person->x.raw + person->unknown18 * around_xy[person->unknown17][0];
    y.raw = person->y.raw + around_xy[person->unknown17][1] * person->unknown18;
    move_node(person, x, y);
    return 0;
}

// FUNCTION: 0x00076768
s16 person_goto_object(Thing *person)
{
    u8 save;
    MapPos x;
    MapPos y;
    s16 dir;
    Thing *obj;

    if (actual_game_turn % person->anim_speed == 0)
        person->sprite = frame[person->sprite].next;
    person->height--;
    if (person->height <= 0) {
        if (blocks[map[(s16)person->x.tile + ((s16)person->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(person))
            return 0;
        modify_person(person);
        if (person->unknown130 <= 0)
            return 0;
        if (do_reaction(person))
            return 0;
        person->height = person->unknown18 == 0 ? div_err() : 0x100 / person->unknown18;
        dir = person_head_for(person, 1);
        if (dir >= 0 && dir <= 3) {
            if (fn_000cb2db(person->x.tile - person->unknown19) < othervar.f2f && fn_000cb2db(person->y.tile - person->unknown20) < othervar.f2f) {
                obj = &thing[person->unknown69];
                if (obj->unknown98 == 1 || obj->kind == 0x12 || (obj->renderer != 8 && obj->renderer != 6))
                    dir = 4;
            }
        }
        switch (dir) {
        case 4:
            process_person_complex(person);
            person->height = 0;
            return 0;
        case -1:
            PLAY_FRAME(person, anim_starts[person->subtype].unknown12);
            person->height = 0;
            return 0;
        case -2:
            person_nav_failure(person);
            return 0;
        case -3:
            obj = &thing[person->unknown69];
            if (obj > thing) {
                save = obj->unknown98;
                obj->unknown98 = 1;
                process_person_complex(person);
                obj->unknown98 = save;
            } else {
                person_nav_failure(person);
            }
            return 0;
        case 0:
        case 1:
        case 2:
        case 3:
        default:
            if (person->unknown17 != dir || start[anim_starts[person->subtype].unknown12 + person->unknown17].frame == person->sprite) {
                person->unknown17 = dir;
                PLAY_FRAME(person, anim_starts[person->subtype].idle);
            }
            break;
        }
    }
    x.raw = person->x.raw + person->unknown18 * around_xy[person->unknown17][0];
    y.raw = person->y.raw + around_xy[person->unknown17][1] * person->unknown18;
    move_node(person, x, y);
    return 0;
}

// FUNCTION: 0x00076c44
void person_goto_queue(Thing *person)
{
    u16 unused;
    MapPos x;
    MapPos y;
    s16 dir;
    RoomInfo *room;

    unused = 0;
    if (actual_game_turn % person->anim_speed == 0)
        person->sprite = frame[person->sprite].next;
    person->height--;
    if (person->height <= 0) {
        if (blocks[map[(s16)person->x.tile + ((s16)person->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(person))
            return;
        modify_person(person);
        if (person->unknown130 <= 0)
            return;
        if (do_reaction(person))
            return;
        person->height = person->unknown18 == 0 ? div_err() : 0x100 / person->unknown18;
        room = &room_info[person->unknown89];
        dir = person_head_for(person, 3);
        if (dir >= 0 && dir <= 3) {
            if (fn_000cb2db(person->x.tile - person->unknown19) <= othervar.fe && fn_000cb2db(person->y.tile - person->unknown20) <= othervar.fe) {
                if (room->type == 4 || queue_size(room) > 0)
                    dir = 4;
            }
        }
        switch (dir) {
        case 4:
            if (room->type != 4) {
                if (room->type != expertise[person->unknown171].category || room->unknown4 == 0) {
                    if (person->unknown89 && room->unknown4 != 0)
                        room->unknown32--;
                    person->unknown89 = 0;
                    person->unknown61 = 0;
                    person->unknown63 = 0;
                    person->unknown75w = 0;
                    start_person_going(person);
                    return;
                }
                if (fn_000cb2db(person->x.tile - room->unknown4 % 128) > othervar.fe
                    || fn_000cb2db(person->y.tile - room->unknown4 / 128) > othervar.fe) {
                    fn_0004195c(person, room->unknown4);
                    person->height = 0;
                    return;
                }
            }
            person->unknown89 = 0;
            person->unknown61 = 0;
            person->unknown63 = 0;
            person->unknown75w = 0;
            join_queue(person, room);
            if (room->type != 4) {
                page_staff_i_need(person, room);
                person->kind = 0x12;
            } else {
                person->kind = 0x14;
                PLAY_FRAME(person, anim_starts[person->subtype].unknown12);
            }
            person->height = 0;
            return;
        case -1:
            PLAY_FRAME(person, anim_starts[person->subtype].unknown12);
            person->height = 0;
            return;
        case -2:
        case -3:
            person_nav_failure(person);
            return;
        default:
            if (person->unknown17 != dir || start[anim_starts[person->subtype].unknown12 + person->unknown17].frame == person->sprite) {
                person->unknown17 = dir;
                PLAY_FRAME(person, anim_starts[person->subtype].idle);
            }
            break;
        }
    }
    x.raw = person->x.raw + person->unknown18 * around_xy[person->unknown17][0];
    y.raw = person->y.raw + around_xy[person->unknown17][1] * person->unknown18;
    move_node(person, x, y);
}

// FUNCTION: 0x0007724c
s16 reaper_goto_hell(Thing *reaper)
{
    MapPos x;
    MapPos y;
    s16 dir;

    if (actual_game_turn % reaper->anim_speed == 0)
        reaper->sprite = frame[reaper->sprite].next;
    reaper->height--;
    if (reaper->height <= 0) {
        reaper->height = reaper->unknown18 == 0 ? div_err() : 0x100 / reaper->unknown18;
        switch (dir = person_head_for(reaper, 3)) {
        case 4:
            reaper_calls_victim(reaper);
            reaper->height = 0;
            return 0;
        case -1:
            reaper->sprite = start[check_frame(0x1f5, reaper) + reaper->unknown17].frame;
            reaper->palette = start[check_frame(0x1f5, reaper) + reaper->unknown17].flags > 0 ? (u8)1 : (u8)0;
            reaper->height = 0;
            return 0;
        case -2:
        case -3:
            person_nav_failure(reaper);
            return 0;
        default:
            if (reaper->unknown17 != dir) {
                reaper->unknown17 = dir;
                reaper->sprite = start[check_frame(0x1f1, reaper) + reaper->unknown17].frame;
                reaper->palette = start[check_frame(0x1f1, reaper) + reaper->unknown17].flags > 0 ? (u8)1 : (u8)0;
            }
            break;
        }
    }
    x.raw = reaper->x.raw + reaper->unknown18 * around_xy[reaper->unknown17][0];
    y.raw = reaper->y.raw + around_xy[reaper->unknown17][1] * reaper->unknown18;
    move_node(reaper, x, y);
    return 0;
}

// FUNCTION: 0x00077508
s16 staff_goto_object(Thing *employee)
{
    MapPos x;
    MapPos y;
    s16 dir;

    if (employee->subtype == 2)
        handyman_check_for_litter(employee);
    if (actual_game_turn % employee->anim_speed == 0)
        employee->sprite = frame[employee->sprite].next;
    employee->height--;
    if (employee->height <= 0) {
        if (blocks[map[(s16)employee->x.tile + ((s16)employee->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(employee))
            return 0;
        employee->height = employee->unknown18 == 0 ? div_err() : 0x100 / employee->unknown18;
        dir = person_head_for(employee, 1);
        switch (dir) {
        case 4:
            employee->unknown89 = 0;
            fn_0003e724(employee);
            return 0;
        case -1:
            PLAY_FRAME(employee, staff_anim_starts[employee->subtype].unknownA);
            employee->height = 0;
            return 0;
        case -2:
            staff_nav_failure(employee);
            return 0;
        case -3:
            if (employee->unknown94)
                fn_00041c0c(employee);
            else
                staff_nav_failure(employee);
            return 0;
        case 0:
        case 1:
        case 2:
        case 3:
        default:
            if (employee->unknown17 != dir || start[staff_anim_starts[employee->subtype].unknownA + employee->unknown17].frame == employee->sprite) {
                employee->unknown17 = dir;
                PLAY_FRAME(employee, staff_anim_starts[employee->subtype].idle);
            }
            break;
        }
    }
    x.raw = employee->x.raw + employee->unknown18 * around_xy[employee->unknown17][0];
    y.raw = employee->y.raw + around_xy[employee->unknown17][1] * employee->unknown18;
    move_node(employee, x, y);
    return 0;
}

// FUNCTION: 0x000778b4
u8 try_staff_wander(Thing *employee, s8 dx, s8 dy, u8 state)
{
    s16 x;
    s16 y;
    u16 tile;
    Map *m;

    x = employee->x.tile + dx;
    y = employee->y.tile + dy;
    tile = x + (y << 7);
    m = &map[tile];
    if (!(m >= map && m < end_map && !(m->flags & 3) && (x >= 0 && x < 0x80 && y >= 0 && y < 0x80))
        || m->unknown7 != 1 << employee->owner || which_room(tile)) {
        employee->unknown103 = 6;
        return 0;
    }
    if (!(employee->unknown19 + dx >= 0 && employee->unknown19 + dx < 0x80 && employee->unknown20 + dy >= 0 && employee->unknown20 + dy < 0x80)) {
        employee->unknown103 = 6;
        return 0;
    }
    employee->unknown19 += dx;
    employee->unknown20 += dy;
    PLAY_FRAME(employee, staff_anim_starts[employee->subtype].idle);
    fn_000444e0(employee);
    employee->height = 1;
    employee->unknown103 = state;
    return 1;
}

// FUNCTION: 0x00077ab8
void staff_wanders(Thing *employee)
{
    u8 roll;
    MapPos y;
    s16 dir;
    MapPos x;

    if (employee->subtype == 2) {
        employee->kind = 0x3c;
        employee->unknown103 = 0;
        return;
    }
    if (employee->subtype == 0)
        allhosps[employee->owner].unknown11f0++;
    if (shall_i_resign(employee))
        return;
    if (employee->unknown103 == 0) {
        if (blocks[map[(s16)employee->x.tile + ((s16)employee->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(employee))
            return;
        employee->height--;
        if (employee->height <= 0)
            goto decide;
    } else {
        if (employee->unknown103 == 6)
            goto decide;
        if (actual_game_turn % employee->anim_speed == 0)
            employee->sprite = frame[employee->sprite].next;
        employee->height--;
        if (employee->height <= 0) {
            if (employee->subtype == 3 && listhead[employee->owner].rooms[4]) {
                if (find_free_object_anywhere(employee - thing, 0xb)) {
                    employee->kind = 0xc;
                    return;
                }
            }
            if (blocks[map[(s16)employee->x.tile + ((s16)employee->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(employee))
                return;
            if (shall_i_resign(employee))
                return;
            employee->height = employee->unknown18 == 0 ? div_err() : 0x100 / employee->unknown18;
            dir = person_head_for(employee, 3);
            switch (dir) {
            case 4:
            case -3:
                employee->height = 1;
                goto decide;
            case -1:
                PLAY_FRAME(employee, staff_anim_starts[employee->subtype].unknownA);
                employee->height = 0;
                return;
            case -2:
                staff_nav_failure(employee);
                return;
            case 0:
            case 1:
            case 2:
            case 3:
            default:
                if (employee->unknown17 != dir) {
                    employee->unknown17 = dir;
                    PLAY_FRAME(employee, staff_anim_starts[employee->subtype].idle);
                }
                break;
            }
        }
        x.raw = employee->x.raw + employee->unknown18 * around_xy[employee->unknown17][0];
        y.raw = employee->y.raw + around_xy[employee->unknown17][1] * employee->unknown18;
        move_node(employee, x, y);
    }
    return;
decide:
    employee->f8e &= 0xaf;
    if (employee->subtype == 0 && should_nurse_vaccinate(employee))
        return;
    if ((employee->f8e & 1) && take_a_break(employee))
        return;
    if (obey_pager(employee))
        return;
    if (employee->f78 > allhosps[employee->owner].f21d) {
        if (take_a_break(employee))
            return;
    }
    if (old_newrand() % 100 < 0x55) {
        switch (employee->unknown103) {
        case 1:
            try_staff_wander(employee, 0, 3, 3);
            return;
        case 3:
            try_staff_wander(employee, 0, -3, 1);
            return;
        case 2:
            try_staff_wander(employee, 3, 0, 4);
            return;
        case 4:
            try_staff_wander(employee, -3, 0, 2);
            return;
        }
    }
    employee->unknown103 = 6;
    PLAY_FRAME(employee, staff_anim_starts[employee->subtype].unknownA);
    roll = old_newrand() % 100;
    if (roll < 0xf) {
        employee->unknown103 = 0;
        employee->height = old_newrand() % 30 + 20;
    } else if (roll < 0x17) {
        try_staff_wander(employee, 0, -3, 1);
    } else if (roll < 0x1f) {
        try_staff_wander(employee, -3, 0, 2);
    } else if (roll < 0x27) {
        try_staff_wander(employee, 0, 3, 3);
    } else if (roll < 0x2f) {
        try_staff_wander(employee, 3, 0, 4);
    } else {
        s8 dx;
        s8 dy;

        dx = old_newrand() % 7 - 3;
        dy = old_newrand() % 7 - 3;
        try_staff_wander(employee, dx, dy, 5);
    }
}

// FUNCTION: 0x000781f8
void peep_bp_goto(Thing *peep)
{
    u8 vertical;
    u8 retry;
    u8 dir;
    MapPos y;
    MapPos x;

    vertical = 0;
    retry = 0;
    if (actual_game_turn % peep->anim_speed == 0)
        peep->sprite = frame[peep->sprite].next;
    peep->height--;
    if (peep->height <= 0) {
        if (blocks[map[(s16)peep->x.tile + ((s16)peep->y.tile << 7)].overlay].width != 0x20
            || (peep->x.tile == peep->unknown106 && peep->y.tile == peep->unknown107)) {
            peep->kind = peep->unknown105;
            return;
        }
        if (peep->x.tile == peep->unknown106) {
            vertical = 1;
        } else if (peep->y.tile == peep->unknown107) {
            vertical = 0;
        } else {
            vertical = old_newrand() % 1000 < 500 ? (u8)1 : (u8)0;
            retry = 1;
        }
        for (;;) {
            if (vertical) {
                if (peep->y.tile < peep->unknown107)
                    dir = 2;
                else
                    dir = 0;
            } else {
                if (peep->x.tile < peep->unknown106)
                    dir = 1;
                else
                    dir = 3;
            }
            if (!retry || blocks[map[(s16)((u16)peep->x.tile + ((u16)peep->y.tile << 7)) + around[dir]].overlay].width == 0x20)
                break;
            vertical = !vertical;
            retry = 0;
        }
        peep->height = peep->unknown18 == 0 ? div_err() : 0x100 / peep->unknown18;
        if (peep->renderer == 4) {
            if (peep->unknown17 != dir || start[staff_anim_starts[peep->subtype].unknownA + peep->unknown17].frame == peep->sprite) {
                peep->unknown17 = dir;
                PLAY_FRAME(peep, staff_anim_starts[peep->subtype].idle);
            }
        } else {
            if (peep->unknown17 != dir || start[anim_starts[peep->subtype].unknown12 + peep->unknown17].frame == peep->sprite) {
                peep->unknown17 = dir;
                PLAY_FRAME(peep, anim_starts[peep->subtype].idle);
            }
        }
    }
    x.raw = peep->x.raw + peep->unknown18 * around_xy[peep->unknown17][0];
    y.raw = peep->y.raw + around_xy[peep->unknown17][1] * peep->unknown18;
    move_node(peep, x, y);
}

void mayor_inspects(Thing *w);

// FUNCTION: 0x00078640
s16 mayor_goto(Thing *mayor)
{
    MapPos x;
    MapPos y;
    s16 dir;

    if (actual_game_turn % mayor->anim_speed == 0)
        mayor->sprite = frame[mayor->sprite].next;
    mayor->height--;
    if (mayor->height <= 0) {
        mayor->height = mayor->unknown18 == 0 ? div_err() : 0x100 / mayor->unknown18;
        switch (dir = person_head_for(mayor, 3)) {
        case 4:
            mayor_inspects(mayor);
            mayor->kind = mayor->f6e;
            mayor->height = 0;
            return 0;
        case -1:
            PLAY_FRAME(mayor, 0x89);
            mayor->height = 0;
            return 0;
        case -2:
        case -3:
            mayor->kind = 6;
            return 0;
        default:
            if (mayor->unknown17 != dir) {
                mayor->unknown17 = dir;
                PLAY_FRAME(mayor, 0x85);
            }
            break;
        }
    }
    x.raw = mayor->x.raw + mayor->unknown18 * around_xy[mayor->unknown17][0];
    y.raw = mayor->y.raw + around_xy[mayor->unknown17][1] * mayor->unknown18;
    move_node(mayor, x, y);
    return 0;
}

s32 wall_in_way(u32 x, u32 y, u32 dir);

extern "C" {
}

// FUNCTION: 0x000788d8
void peep_stuck(Thing *peep)
{
    u8 dir;
    MapPos y;
    MapPos x;
    s32 dest;

    if (actual_game_turn % peep->anim_speed == 0)
        peep->sprite = frame[peep->sprite].next;
    peep->height--;
    if (peep->height <= 0) {
        if (peep->unknown17 != 4) {
            peep->height = 100;
            if (peep->renderer == 2)
                PLAY_FRAME(peep, anim_starts[peep->subtype].unknown12)
            else
                PLAY_FRAME(peep, staff_anim_starts[peep->subtype].unknownA)
            peep->unknown17 = 4;
            return;
        }
        dest = (s16)peep->unknown106 + ((s16)peep->unknown107 << 7);
        if ((peep->renderer != 2 || !(peep->flags & 0x4000))
            && check_points_connected((u16)peep->x.tile + ((u16)peep->y.tile << 7), dest)) {
            peep->kind = peep->unknown105;
            peep->unknown19 = peep->unknown106;
            peep->unknown20 = peep->unknown107;
            return;
        }
        if (peep->renderer == 2) {
            peep->unknown124 += 0xf;
            if (peep->unknown124 > gbv.f108) {
                peep->unknown124 = 0;
                if (!(peep->flags & 0x4000)) {
                    if (peep->owner == player)
                        othervar.f89++;
                }
                set_icon_value(peep, 7);
                person_go_home(peep);
                peep->flags |= 0x4000;
                return;
            }
        }
        dir = old_newrand() & 3;
        if (!wall_in_way(peep->x.tile, peep->y.tile, dir)) {
            peep->unknown17 = dir;
            peep->height = peep->unknown18 == 0 ? div_err() : 0x100 / peep->unknown18;
            if (peep->renderer == 2)
                PLAY_FRAME(peep, anim_starts[peep->subtype].idle)
            else
                PLAY_FRAME(peep, staff_anim_starts[peep->subtype].idle)
        }
    }
    if (peep->unknown17 != 4) {
        x.raw = peep->x.raw + peep->unknown18 * around_xy[peep->unknown17][0];
        y.raw = peep->y.raw + around_xy[peep->unknown17][1] * peep->unknown18;
        move_node(peep, x, y);
    }
}
