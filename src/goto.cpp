#include "hosp.h"

extern "C" {
extern s8 around_xy[][2];
#pragma aux around_xy "W?around_vxy$n[]$VXY$$"
}

u8 do_reaction(Thing *peep);
s16 person_head_for(Thing *person, u8 a);
void join_queue(Thing *person, RoomInfo *room);
void reaper_calls_victim(Thing *reaper);
void handyman_check_for_litter(Thing *handyman);
void staff_nav_failure(Thing *staff);
extern "C" void fail_staff_complex(Thing *w);
extern "C" void nav_init(Thing *w);
u16 find_free_object_anywhere(u16 id, u8 kind);
u8 should_nurse_vaccinate(Thing *nurse);
u8 try_staff_wander(Thing *employee, s8 dx, s8 dy, u8 substate);

#define PLAY_FRAME(t, a) \
    { \
        u16 anim; \
\
        anim = check_frame(a, t) + (u16)(t)->facing; \
        (t)->sprite = start[anim].frame; \
        (t)->flip = start[anim].flags > 0 ? (u8)1 : (u8)0; \
    }

// PS_GOTO_DESTINATION: walks person a step towards dest_x,dest_y, animating
// them; at each new tile also leaves a blueprint, updates their needs and lets
// them react. On arrival runs their complex (or waits in PS_WAITS_FOR_BUSY if
// none); a navigation failure goes to person_nav_failure. Always returns 0.
// FUNCTION: 0x000763d0
s16 person_goto_destination(Thing *person)
{
    MapPos x;
    MapPos y;
    s16 dir;

    if (actual_game_turn % person->anim_speed == 0)
        person->sprite = frame[person->sprite].next;
    person->countdown--;
    if (person->countdown <= 0) {
        if (blocks[map[(s16)person->x.tile + ((s16)person->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(person))
            return 0;
        modify_person(person);
        if (do_reaction(person))
            return 0;
        person->countdown = person->speed == 0 ? div_err() : 0x100 / person->speed;
        dir = person_head_for(person, 3);
        switch (dir) {
        case 4:
            if (process_person_complex(person) == 2) {
                person->state = PS_WAITS_FOR_BUSY;
                person->timer = 200;
            }
            person->countdown = 0;
            return 0;
        case -1:
            PLAY_FRAME(person, anim_starts[person->subtype].patient_stand);
            person->countdown = 0;
            return 0;
        case -2:
        case -3:
            person_nav_failure(person);
            return 0;
        default:
            if (person->facing != dir || start[anim_starts[person->subtype].patient_stand + person->facing].frame == person->sprite) {
                person->facing = dir;
                PLAY_FRAME(person, anim_starts[person->subtype].walk);
            }
            break;
        }
    }
    x.raw = person->x.raw + person->speed * around_xy[person->facing][0];
    y.raw = person->y.raw + around_xy[person->facing][1] * person->speed;
    move_node(person, x, y);
    return 0;
}

// PS_GOTO_OBJECT: walks person towards their object as person_goto_destination
// does; close to it, or on arrival, runs the complex to use it, waiting if it
// is busy. If the object's tile cannot be reached the complex still runs, with
// the object treated as busy. Always returns 0.
// FUNCTION: 0x00076768
s16 person_goto_object(Thing *person)
{
    u8 saved_in_use;
    MapPos x;
    MapPos y;
    s16 dir;
    Thing *obj;

    if (actual_game_turn % person->anim_speed == 0)
        person->sprite = frame[person->sprite].next;
    person->countdown--;
    if (person->countdown <= 0) {
        if (blocks[map[(s16)person->x.tile + ((s16)person->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(person))
            return 0;
        modify_person(person);
        if (person->health <= 0)
            return 0;
        if (do_reaction(person))
            return 0;
        person->countdown = person->speed == 0 ? div_err() : 0x100 / person->speed;
        dir = person_head_for(person, 1);
        if (dir >= 0 && dir <= 3) {
            if (fn_000cb2db(person->x.tile - person->dest_x) < othervar.object_near_dist && fn_000cb2db(person->y.tile - person->dest_y) < othervar.object_near_dist) {
                obj = &thing[person->object];
                if (obj->in_use == 1 || obj->state == OS_REMOVED || (obj->type != THING_DOOR && obj->type != THING_OBJECT))
                    dir = 4;
            }
        }
        switch (dir) {
        case 4:
            process_person_complex(person);
            person->countdown = 0;
            return 0;
        case -1:
            PLAY_FRAME(person, anim_starts[person->subtype].patient_stand);
            person->countdown = 0;
            return 0;
        case -2:
            person_nav_failure(person);
            return 0;
        case -3:
            obj = &thing[person->object];
            if (obj > thing) {
                saved_in_use = obj->in_use;
                obj->in_use = 1;
                process_person_complex(person);
                obj->in_use = saved_in_use;
            } else {
                person_nav_failure(person);
            }
            return 0;
        case 0:
        case 1:
        case 2:
        case 3:
        default:
            if (person->facing != dir || start[anim_starts[person->subtype].patient_stand + person->facing].frame == person->sprite) {
                person->facing = dir;
                PLAY_FRAME(person, anim_starts[person->subtype].walk);
            }
            break;
        }
    }
    x.raw = person->x.raw + person->speed * around_xy[person->facing][0];
    y.raw = person->y.raw + around_xy[person->facing][1] * person->speed;
    move_node(person, x, y);
    return 0;
}

// PS_GOTO_QUEUE: walks person towards the room they are heading for
// (dest_room); near its door joins its queue (paging the staff it needs,
// PS_ARRIVES_AT_DOOR) or the reception queue (PS_IN_QUEUE). If the room has
// changed kind or lost its door, starts them going again.
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
    person->countdown--;
    if (person->countdown <= 0) {
        if (blocks[map[(s16)person->x.tile + ((s16)person->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(person))
            return;
        modify_person(person);
        if (person->health <= 0)
            return;
        if (do_reaction(person))
            return;
        person->countdown = person->speed == 0 ? div_err() : 0x100 / person->speed;
        room = &room_info[person->dest_room];
        dir = person_head_for(person, 3);
        if (dir >= 0 && dir <= 3) {
            if (fn_000cb2db(person->x.tile - person->dest_x) <= othervar.queue_near_dist && fn_000cb2db(person->y.tile - person->dest_y) <= othervar.queue_near_dist) {
                if (room->type == ROOM_RECEPTION || queue_size(room) > 0)
                    dir = 4;
            }
        }
        switch (dir) {
        case 4:
            if (room->type != ROOM_RECEPTION) {
                if (room->type != expertise[person->treatment].category || room->door_tile == 0) {
                    if (person->dest_room && room->door_tile != 0)
                        room->queue_incoming--;
                    person->dest_room = 0;
                    person->complex_pc = 0;
                    person->complex_queued = 0;
                    person->complex_queued2 = 0;
                    start_person_going(person);
                    return;
                }
                if (fn_000cb2db(person->x.tile - room->door_tile % 128) > othervar.queue_near_dist
                    || fn_000cb2db(person->y.tile - room->door_tile / 128) > othervar.queue_near_dist) {
                    set_destination_coords(person, room->door_tile);
                    person->countdown = 0;
                    return;
                }
            }
            person->dest_room = 0;
            person->complex_pc = 0;
            person->complex_queued = 0;
            person->complex_queued2 = 0;
            join_queue(person, room);
            if (room->type != ROOM_RECEPTION) {
                page_staff_i_need(person, room);
                person->state = PS_ARRIVES_AT_DOOR;
            } else {
                person->state = PS_IN_QUEUE;
                PLAY_FRAME(person, anim_starts[person->subtype].patient_stand);
            }
            person->countdown = 0;
            return;
        case -1:
            PLAY_FRAME(person, anim_starts[person->subtype].patient_stand);
            person->countdown = 0;
            return;
        case -2:
        case -3:
            person_nav_failure(person);
            return;
        default:
            if (person->facing != dir || start[anim_starts[person->subtype].patient_stand + person->facing].frame == person->sprite) {
                person->facing = dir;
                PLAY_FRAME(person, anim_starts[person->subtype].walk);
            }
            break;
        }
    }
    x.raw = person->x.raw + person->speed * around_xy[person->facing][0];
    y.raw = person->y.raw + around_xy[person->facing][1] * person->speed;
    move_node(person, x, y);
}

// REAPER_GOTO_HELL: walks the grim reaper towards its destination; on arrival
// it calls its victim. Always returns 0.
// FUNCTION: 0x0007724c
s16 reaper_goto_hell(Thing *reaper)
{
    MapPos x;
    MapPos y;
    s16 dir;

    if (actual_game_turn % reaper->anim_speed == 0)
        reaper->sprite = frame[reaper->sprite].next;
    reaper->countdown--;
    if (reaper->countdown <= 0) {
        reaper->countdown = reaper->speed == 0 ? div_err() : 0x100 / reaper->speed;
        switch (dir = person_head_for(reaper, 3)) {
        case 4:
            reaper_calls_victim(reaper);
            reaper->countdown = 0;
            return 0;
        case -1:
            reaper->sprite = start[check_frame(0x1f5, reaper) + reaper->facing].frame;
            reaper->flip = start[check_frame(0x1f5, reaper) + reaper->facing].flags > 0 ? (u8)1 : (u8)0;
            reaper->countdown = 0;
            return 0;
        case -2:
        case -3:
            person_nav_failure(reaper);
            return 0;
        default:
            if (reaper->facing != dir) {
                reaper->facing = dir;
                reaper->sprite = start[check_frame(0x1f1, reaper) + reaper->facing].frame;
                reaper->flip = start[check_frame(0x1f1, reaper) + reaper->facing].flags > 0 ? (u8)1 : (u8)0;
            }
            break;
        }
    }
    x.raw = reaper->x.raw + reaper->speed * around_xy[reaper->facing][0];
    y.raw = reaper->y.raw + around_xy[reaper->facing][1] * reaper->speed;
    move_node(reaper, x, y);
    return 0;
}

// SS_GOTO_OBJECT: walks employee towards their destination (a handyman also
// looks for litter on the way); on arrival runs their complex; a navigation
// failure ends the complex or goes to staff_nav_failure. Always returns 0.
// FUNCTION: 0x00077508
s16 staff_goto_object(Thing *employee)
{
    MapPos x;
    MapPos y;
    s16 dir;

    if (employee->subtype == STAFF_HANDYMAN)
        handyman_check_for_litter(employee);
    if (actual_game_turn % employee->anim_speed == 0)
        employee->sprite = frame[employee->sprite].next;
    employee->countdown--;
    if (employee->countdown <= 0) {
        if (blocks[map[(s16)employee->x.tile + ((s16)employee->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(employee))
            return 0;
        employee->countdown = employee->speed == 0 ? div_err() : 0x100 / employee->speed;
        dir = person_head_for(employee, 1);
        switch (dir) {
        case 4:
            employee->dest_room = 0;
            process_staff_complex(employee);
            return 0;
        case -1:
            PLAY_FRAME(employee, staff_anim_starts[employee->subtype].staff_stand);
            employee->countdown = 0;
            return 0;
        case -2:
            staff_nav_failure(employee);
            return 0;
        case -3:
            if (employee->resume_state)
                fail_staff_complex(employee);
            else
                staff_nav_failure(employee);
            return 0;
        case 0:
        case 1:
        case 2:
        case 3:
        default:
            if (employee->facing != dir || start[staff_anim_starts[employee->subtype].staff_stand + employee->facing].frame == employee->sprite) {
                employee->facing = dir;
                PLAY_FRAME(employee, staff_anim_starts[employee->subtype].walk);
            }
            break;
        }
    }
    x.raw = employee->x.raw + employee->speed * around_xy[employee->facing][0];
    y.raw = employee->y.raw + around_xy[employee->facing][1] * employee->speed;
    move_node(employee, x, y);
    return 0;
}

// Sends wandering employee dx,dy tiles on from their destination and sets
// substate (the direction wandered) if the tile is free corridor on their own
// land; otherwise sets substate 6 (decide again). Returns 1 if they set off.
// FUNCTION: 0x000778b4
u8 try_staff_wander(Thing *employee, s8 dx, s8 dy, u8 substate)
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
        || m->owner_flags != 1 << employee->owner || which_room(tile)) {
        employee->substate = 6;
        return 0;
    }
    if (!(employee->dest_x + dx >= 0 && employee->dest_x + dx < 0x80 && employee->dest_y + dy >= 0 && employee->dest_y + dy < 0x80)) {
        employee->substate = 6;
        return 0;
    }
    employee->dest_x += dx;
    employee->dest_y += dy;
    PLAY_FRAME(employee, staff_anim_starts[employee->subtype].walk);
    nav_init(employee);
    employee->countdown = 1;
    employee->substate = substate;
    return 1;
}

// SS_WANDERS: an idle member of staff strolls the corridors, usually pacing 3
// tiles back and forth, sometimes pausing or picking a new direction; between
// strolls they check whether to vaccinate, rest, answer a page or take a break,
// and a receptionist goes to a free desk. Handymen go straight to work.
// FUNCTION: 0x00077ab8
void staff_wanders(Thing *employee)
{
    u8 roll;
    MapPos y;
    s16 dir;
    MapPos x;

    if (employee->subtype == STAFF_HANDYMAN) {
        employee->state = SS_HANDYMAN_WORKS;
        employee->substate = 0;
        return;
    }
    if (employee->subtype == STAFF_NURSE)
        allhosps[employee->owner].idle_nurses++;
    if (shall_i_resign(employee))
        return;
    if (employee->substate == 0) {
        if (blocks[map[(s16)employee->x.tile + ((s16)employee->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(employee))
            return;
        employee->countdown--;
        if (employee->countdown <= 0)
            goto decide;
    } else {
        if (employee->substate == 6)
            goto decide;
        if (actual_game_turn % employee->anim_speed == 0)
            employee->sprite = frame[employee->sprite].next;
        employee->countdown--;
        if (employee->countdown <= 0) {
            if (employee->subtype == STAFF_RECEPTIONIST && listhead[employee->owner].rooms[ROOM_RECEPTION]) {
                if (find_free_object_anywhere(employee - thing, OBJ_RECEPTION_DESK)) {
                    employee->state = SS_RECEPTIONIST_FINDS_RECEPTION;
                    return;
                }
            }
            if (blocks[map[(s16)employee->x.tile + ((s16)employee->y.tile << 7)].overlay].width == 0x20 && move_peep_from_blueprint(employee))
                return;
            if (shall_i_resign(employee))
                return;
            employee->countdown = employee->speed == 0 ? div_err() : 0x100 / employee->speed;
            dir = person_head_for(employee, 3);
            switch (dir) {
            case 4:
            case -3:
                employee->countdown = 1;
                goto decide;
            case -1:
                PLAY_FRAME(employee, staff_anim_starts[employee->subtype].staff_stand);
                employee->countdown = 0;
                return;
            case -2:
                staff_nav_failure(employee);
                return;
            case 0:
            case 1:
            case 2:
            case 3:
            default:
                if (employee->facing != dir) {
                    employee->facing = dir;
                    PLAY_FRAME(employee, staff_anim_starts[employee->subtype].walk);
                }
                break;
            }
        }
        x.raw = employee->x.raw + employee->speed * around_xy[employee->facing][0];
        y.raw = employee->y.raw + around_xy[employee->facing][1] * employee->speed;
        move_node(employee, x, y);
    }
    return;
decide:
    employee->orders &= ~(ORD_RETURN_TO_RESEARCH | ORD_RETURN_TO_TRAINING);
    if (employee->subtype == STAFF_NURSE && should_nurse_vaccinate(employee))
        return;
    if ((employee->orders & ORD_REST) && take_a_break(employee))
        return;
    if (obey_pager(employee))
        return;
    if (employee->tiredness > allhosps[employee->owner].rest_threshold) {
        if (take_a_break(employee))
            return;
    }
    if (old_newrand() % 100 < 0x55) {
        switch (employee->substate) {
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
    employee->substate = 6;
    PLAY_FRAME(employee, staff_anim_starts[employee->subtype].staff_stand);
    roll = old_newrand() % 100;
    if (roll < 0xf) {
        employee->substate = 0;
        employee->countdown = old_newrand() % 30 + 20;
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

// PS_BP_GOTO / SS_BP_GOTO: walks peep off a blueprint towards saved_dest_x,y
// along the grid, and returns them to saved_state once off it or there.
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
    peep->countdown--;
    if (peep->countdown <= 0) {
        if (blocks[map[(s16)peep->x.tile + ((s16)peep->y.tile << 7)].overlay].width != 0x20
            || (peep->x.tile == peep->saved_dest_x && peep->y.tile == peep->saved_dest_y)) {
            peep->state = peep->saved_state;
            return;
        }
        if (peep->x.tile == peep->saved_dest_x) {
            vertical = 1;
        } else if (peep->y.tile == peep->saved_dest_y) {
            vertical = 0;
        } else {
            vertical = old_newrand() % 1000 < 500 ? (u8)1 : (u8)0;
            retry = 1;
        }
        for (;;) {
            if (vertical) {
                if (peep->y.tile < peep->saved_dest_y)
                    dir = 2;
                else
                    dir = 0;
            } else {
                if (peep->x.tile < peep->saved_dest_x)
                    dir = 1;
                else
                    dir = 3;
            }
            if (!retry || blocks[map[(s16)((u16)peep->x.tile + ((u16)peep->y.tile << 7)) + around[dir]].overlay].width == 0x20)
                break;
            vertical = !vertical;
            retry = 0;
        }
        peep->countdown = peep->speed == 0 ? div_err() : 0x100 / peep->speed;
        if (peep->type == THING_STAFF) {
            if (peep->facing != dir || start[staff_anim_starts[peep->subtype].staff_stand + peep->facing].frame == peep->sprite) {
                peep->facing = dir;
                PLAY_FRAME(peep, staff_anim_starts[peep->subtype].walk);
            }
        } else {
            if (peep->facing != dir || start[anim_starts[peep->subtype].patient_stand + peep->facing].frame == peep->sprite) {
                peep->facing = dir;
                PLAY_FRAME(peep, anim_starts[peep->subtype].walk);
            }
        }
    }
    x.raw = peep->x.raw + peep->speed * around_xy[peep->facing][0];
    y.raw = peep->y.raw + around_xy[peep->facing][1] * peep->speed;
    move_node(peep, x, y);
}

void mayor_inspects(Thing *mayor);

// MAYOR_GOTO: walks the mayor or inspector towards their destination; on
// arrival they inspect and enter goto_state; a navigation failure makes them
// leave (MAYOR_GONE). Always returns 0.
// FUNCTION: 0x00078640
s16 mayor_goto(Thing *mayor)
{
    MapPos x;
    MapPos y;
    s16 dir;

    if (actual_game_turn % mayor->anim_speed == 0)
        mayor->sprite = frame[mayor->sprite].next;
    mayor->countdown--;
    if (mayor->countdown <= 0) {
        mayor->countdown = mayor->speed == 0 ? div_err() : 0x100 / mayor->speed;
        switch (dir = person_head_for(mayor, 3)) {
        case 4:
            mayor_inspects(mayor);
            mayor->state = mayor->goto_state;
            mayor->countdown = 0;
            return 0;
        case -1:
            PLAY_FRAME(mayor, 0x89);
            mayor->countdown = 0;
            return 0;
        case -2:
        case -3:
            mayor->state = MAYOR_GONE;
            return 0;
        default:
            if (mayor->facing != dir) {
                mayor->facing = dir;
                PLAY_FRAME(mayor, 0x85);
            }
            break;
        }
    }
    x.raw = mayor->x.raw + mayor->speed * around_xy[mayor->facing][0];
    y.raw = mayor->y.raw + around_xy[mayor->facing][1] * mayor->speed;
    move_node(mayor, x, y);
    return 0;
}

s32 wall_in_way(u32 x, u32 y, u32 dir);

extern "C" {
}

// PS_STUCK / SS_STUCK: peep stands still, then tries random directions until
// a path to their saved destination opens, then resumes saved_state. A
// patient grows impatient meanwhile and eventually goes home fed up.
// FUNCTION: 0x000788d8
void peep_stuck(Thing *peep)
{
    u8 dir;
    MapPos y;
    MapPos x;
    s32 dest;

    if (actual_game_turn % peep->anim_speed == 0)
        peep->sprite = frame[peep->sprite].next;
    peep->countdown--;
    if (peep->countdown <= 0) {
        if (peep->facing != 4) {
            peep->countdown = 100;
            if (peep->type == THING_PATIENT)
                PLAY_FRAME(peep, anim_starts[peep->subtype].patient_stand)
            else
                PLAY_FRAME(peep, staff_anim_starts[peep->subtype].staff_stand)
            peep->facing = 4;
            return;
        }
        dest = (s16)peep->saved_dest_x + ((s16)peep->saved_dest_y << 7);
        if ((peep->type != THING_PATIENT || !(peep->flags & PF_FED_UP))
            && check_points_connected((u16)peep->x.tile + ((u16)peep->y.tile << 7), dest)) {
            peep->state = peep->saved_state;
            peep->dest_x = peep->saved_dest_x;
            peep->dest_y = peep->saved_dest_y;
            return;
        }
        if (peep->type == THING_PATIENT) {
            peep->impatience += 0xf;
            if (peep->impatience > gbv.leave_max) {
                peep->impatience = 0;
                if (!(peep->flags & PF_FED_UP)) {
                    if (peep->owner == player)
                        othervar.patients_left_count++;
                }
                set_icon_value(peep, ICON_FED_UP);
                person_go_home(peep);
                peep->flags |= PF_FED_UP;
                return;
            }
        }
        dir = old_newrand() & 3;
        if (!wall_in_way(peep->x.tile, peep->y.tile, dir)) {
            peep->facing = dir;
            peep->countdown = peep->speed == 0 ? div_err() : 0x100 / peep->speed;
            if (peep->type == THING_PATIENT)
                PLAY_FRAME(peep, anim_starts[peep->subtype].walk)
            else
                PLAY_FRAME(peep, staff_anim_starts[peep->subtype].walk)
        }
    }
    if (peep->facing != 4) {
        x.raw = peep->x.raw + peep->speed * around_xy[peep->facing][0];
        y.raw = peep->y.raw + around_xy[peep->facing][1] * peep->speed;
        move_node(peep, x, y);
    }
}
