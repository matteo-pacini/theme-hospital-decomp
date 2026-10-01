#include "hosp.h"

extern "C" {
void set_next_destination_coords(Thing *w, u16 arg);
void set_next2_destination_coords(Thing *w, u16 arg);
u8 process_complex_either(Thing *w);
void person_complex_retry(Thing *w);
void person_wait_for_complex_slave(Thing *w);
void nav_init(Thing *w);
}

extern "C" {
extern u8 *staff_complex_group;
extern ComplexAnims group_anim[];
void fail_staff_complex(Thing *w);
}

extern Complex person_complex[];
extern u8 *person_complex_group;
extern FaceDirns face_dirns[];
extern s16 opposite_angle[];

void staff_exit_room_check(Thing *w);
s16 person_head_for(Thing *w, u8 a);
Thing *find_door(Thing *w);
void adjust_big_cell_flag(const u16 tile);
void start_new_interaction(Thing *w, u8 who);
void pager_arrival(Thing *w);
void clear_my_mutex(Thing *w);
u8 count_surgeons(RoomInfo *room);
u8 check_room_mutex_on_entry(Thing *w);
void merge_slave(Thing *w);
void complex_success(Thing *w, u8 kind);
u8 staff_go_near_object(Thing *w);
void fail_person_complex(Thing *w);
void thingpage_cancel_my_response(ThingPager *p, Thing *w);

#define PLAY_FRAME(t, a) \
    { \
        u16 anim; \
 \
        anim = check_frame(a, t) + (u16)(t)->facing; \
        (t)->sprite = start[anim].frame; \
        (t)->flip = start[anim].flags > 0 ? (u8)1 : (u8)0; \
    }
void rest_staff(Thing *w, s16 amount);
void research_contribution(Thing *w, s16 amount);
void person_autopsied(Thing *w);

// FUNCTION: 0x0003e6c0
extern "C" u8 process_complex_either(Thing *w)
{
    switch (w->type) {
    case THING_STAFF:
        return process_staff_complex(w);
    case THING_PATIENT:
        return process_person_complex(w);
    }
    return 0;
}

// FUNCTION: 0x0003e724
u8 process_staff_complex(Thing *employee)
{
    u8 run_things;
    s8 choice;
    MapPos x;
    s8 repeat;
    u8 op;
    s8 dir;
    u16 action;
    u16 arg;
    u16 tile;
    u16 obj_id;
    MapPos y;
    Thing *other;
    Thing *patient;
    Complex *entry;
    Map *cell;
    Thing *obj;
    Thing *door;

    run_things = 0;
    if (employee->complex_pc == 0) {
        return 2;
    }
    op = staff_complex_group[employee->complex_pc];
    if (op == 0) {
        employee->complex_pc = 0;
        return 2;
    }
    tile = employee->x.tile + (employee->y.tile << 7);
    entry = &staff_complex[op];
    if (entry->object) {
        obj_id = find_nearest_free_object(employee - thing, entry->object);
        if (obj_id) {
            obj = &thing[obj_id];
        } else {
            fail_staff_complex(employee);
            return 0;
        }
    } else {
        obj = 0;
    }
    if (entry->kind) {
        employee->state = entry->kind;
    }
    switch (entry->action) {
    case 0:
        break;
    case 56:
        if (staff_go_near_object(employee)) {
            dir = person_head_for(employee, 1);
            if (dir >= 0 && dir < 4) {
                employee->facing = dir;
            }
            PLAY_FRAME(employee, staff_anim_starts[employee->subtype].idle);
            employee->countdown = 0;
        } else {
            fail_staff_complex(employee);
            return 0;
        }
        break;
    case 2:
        employee->object = obj_id;
    case 44:
        obj = &thing[employee->object];
        set_destination_coords(employee, obj->use_pos[employee->subtype == STAFF_HANDYMAN ? 2 : 0]);
        dir = person_head_for(employee, 1);
        if (dir >= 0 && dir < 4) {
            employee->facing = dir;
        }
        PLAY_FRAME(employee, staff_anim_starts[employee->subtype].idle);
        employee->countdown = 0;
        break;
    case 4:
        PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[entry->anim]);
        break;
    case 54:
        patient = &thing[employee->slave];
        if (patient > thing) {
            u16 anim;

            anim = check_frame(anim_starts[patient->subtype].anims[entry->anim], thing) + (u16)employee->facing;
            employee->sprite = start[anim].frame;
            employee->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
        }
        break;
    case 6:
        obj = &thing[employee->object];
        if ((obj->in_use == 1 || obj->state == 0x12) || obj->type != THING_OBJECT
            || obj->use_pos[employee->subtype == STAFF_HANDYMAN ? 2 : 0] != ((s16)employee->y.tile << 7) + (s16)employee->x.tile) {
            fail_staff_complex(employee);
            return 3;
        }
        obj->in_use = 1;
        obj->user = employee - thing;
        employee->facing = obj->facing;
        if (obj->use_style == 1 || obj->use_style == 2) {
            employee->exit_tile = obj->use_pos[(employee->subtype == STAFF_HANDYMAN ? 2 : 0) + 3];
            move_node(employee, *(MapPos *)&obj->use_x, *(MapPos *)&obj->use_y);
            obj->state = 4;
            employee->depth = obj->depth;
            if (objects[obj->subtype].unknown1e) {
                obj->saved_depth = obj->depth;
                obj->depth = 8;
            }
            map[(s16)employee->x.tile + ((s16)employee->y.tile << 7)].flags |= 0x20;
        }
        employee->layers[10] = obj->layers[10];
        employee->layers[11] = obj->layers[11];
        employee->layers[12] = obj->layers[12];
        break;
    case 8:
        obj = &thing[employee->object];
        obj->user = 0;
        employee->object = 0;
        obj->in_use = 0;
        if (obj->use_style == 2 || obj->use_style == 1) {
            adjust_big_cell_flag(tile);
            x.tile = obj->use_pos[(employee->subtype == STAFF_HANDYMAN ? 2 : 0) + 3] % 128;
            y.tile = obj->use_pos[(employee->subtype == STAFF_HANDYMAN ? 2 : 0) + 3] / 128;
            x.frac = y.frac = 0x80;
            move_node(employee, x, y);
            employee->depth = 0;
            if (objects[obj->subtype].unknown1e) {
                obj->depth = obj->saved_depth;
                obj->saved_depth = 0;
            }
            obj->state = OS_STATIC;
            if (obj->use_style == 2) {
                patient = &thing[employee->slave];
                if (patient > thing) {
                    x.tile = obj->use_pos[4] % 128;
                    y.tile = obj->use_pos[4] / 128;
                    move_node(patient, x, y);
                    patient->object = 0;
                }
            }
        }
        PLAY_FRAME(employee, staff_anim_starts[employee->subtype].unknownA);
        employee->layers[10] = employee->layers[11] = employee->layers[12] = 0;
        break;
    case 10:
        obj = &thing[employee->object];
        if (obj > thing) {
            PLAY_FRAME(obj, entry->anim);
        }
        break;
    case 16:
        choice = old_newrand() % group_anim[entry->anim].count;
        PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[group_anim[entry->anim].anims[choice]]);
        break;
    case 14:
        obj = find_door(employee);
        if (obj > thing) {
            employee->facing = opposite_angle[obj->facing];
        }
        break;
    case 20:
        door = find_door(employee);
        if (door > thing) {
            u8 double_door;

            double_door = door->subtype != OBJ_DOOR ? (u8)1 : (u8)0;
            if (door->in_use == 1) {
                employee->state = SS_COMPLEX_RETRY;
                return 3;
            }
            if (employee->room) {
                employee->facing = door->facing;
            } else {
                employee->facing = opposite_angle[door->facing];
            }
            if (tile == door->use_pos[0]) {
                employee->exit_tile = door->use_pos[3];
            } else {
                employee->exit_tile = door->use_pos[0];
            }
            if (entry->anim == 1 && !check_room_mutex_on_entry(employee)) {
                fail_staff_complex(employee);
                return 4;
            }
            if (employee->room == 0
                && ((room_info[door->room_id].f22 || room_info[door->room_id].f3 == 4)
                    || door->room_id == allhosps[door->owner].unknown11b7)) {
                fail_staff_complex(employee);
                return 4;
            }
            if (double_door) {
                PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[8]);
            } else {
                PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[6]);
            }
            door->in_use = 1;
            employee->object = door - thing;
            door->user = employee - thing;
            move_node(employee, door->x, door->y);
            door->state = 4;
            cell = &map[(s16)door->x.tile + ((s16)door->y.tile << 7)];
            door->aux_param = cell->flags & 0x10 ? (u8)1 : (u8)0;
            cell->flags |= 0x10;
            if (double_door) {
                other = &thing[door->door_partner];
                other->state = OS_DOUBLE_DOOR_SWINGS;
                other->sprite = start[employee->facing + 0x3f8].frame;
                other->flip = start[employee->facing + 0x3f8].flags;
                other->anim_speed = employee->anim_speed;
            }
        } else {
            fail_staff_complex(employee);
            return 4;
        }
        break;
    case 22:
        clear_my_mutex(employee);
        door = &thing[employee->object];
        door->user = 0;
        employee->object = 0;
        x.tile = employee->exit_tile % 128;
        y.tile = employee->exit_tile / 128;
        x.frac = y.frac = 0x80;
        employee->dest_x = x.tile;
        employee->dest_y = y.tile;
        move_node(employee, x, y);
        move_person_in_room_list(employee, employee->exit_tile);
        if (employee->room) {
            employee->room->unknown29 = 0;
            pager_arrival(employee);
        }
        cell = &map[(s16)door->x.tile + ((s16)door->y.tile << 7)];
        if (!door->aux_param) {
            cell->flags &= ~0x10;
        }
        if (door->subtype == OBJ_DOOR) {
            door->in_use = 0;
            door->state = OS_STATIC;
        } else {
            door->state = OS_DOUBLE_DOOR_SWINGS;
            door->sprite = start[employee->facing + 0x400].frame;
            door->flip = start[employee->facing + 0x400].flags;
        }
        break;
    case 24:
        employee->complex_pc++;
        employee->complex_resume = (u16)staff_complex_group[employee->complex_pc] * 10;
        break;
    case 26:
        employee->complex_pc = employee->complex_queued - 1;
        employee->complex_queued = 0;
        break;
    case 28:
        employee->complex_pc++;
        start_new_interaction(employee, staff_complex_group[employee->complex_pc] + 1);
        break;
    case 30:
        merge_slave(employee);
        break;
    case 36: {
        s8 *list;
        u8 n;

        list = group_ia[entry->anim];
        n = 0;
        while (list[n] != -1 && n < 5) {
            n++;
        }
        start_new_interaction(employee, list[old_newrand() % n]);
        break;
    }
    case 38:
        patient = &thing[employee->slave];
        if (patient > thing) {
            employee->complex_pc++;
            patient->next_state = 0x20;
            set_complex(patient, (u16)staff_complex_group[employee->complex_pc] * 10, 0);
            process_complex_either(patient);
        } else {
            fail_staff_complex(employee);
            return 0;
        }
        break;
    case 40:
        patient = &thing[employee->slave];
        if (patient > thing) {
            employee->complex_pc++;
            patient->next_state = 0x20;
            set_complex(patient, (u16)staff_complex_group[employee->complex_pc] * 10, 0);
        } else {
            fail_staff_complex(employee);
            return 0;
        }
        break;
    case 46:
        dir = person_head_for(employee, 1);
        if (dir >= 0 && dir < 4) {
            employee->facing = dir;
        }
        PLAY_FRAME(employee, staff_anim_starts[employee->subtype].idle);
        break;
    case 48:
        patient = &thing[employee->slave];
        if (patient > thing && (obj = &thing[employee->object]) > thing) {
            x.tile = obj->use_pos[4] % 128;
            y.tile = obj->use_pos[4] / 128;
            x.frac = y.frac = 0x80;
            move_node(patient, x, y);
            patient->object = 0;
        }
        break;
    case 18:
        obj_id = employee->object;
        if (obj_id) {
            s8 kind;
            s8 face;
            s8 d;

            kind = thing[obj_id].subtype;
            face = objects[kind].unknown6[0];
            if (face != -1) {
                d = face_dirns[face].dirs[thing[obj_id].facing];
                if (d != -1) {
                    employee->facing = d;
                    PLAY_FRAME(employee, staff_anim_starts[employee->subtype].unknownA);
                }
            }
        }
        break;
    case 50:
        run_things = 1;
        break;
    case 52:
        complex_success(employee, entry->anim);
        break;
    case 58:
        employee->subtype = entry->anim;
        break;
    case 60:
        obj = &thing[employee->object];
        if (count_surgeons(employee->room) == 1) {
            PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[19]);
            PLAY_FRAME(obj, 0x56c);
        } else {
            PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[20]);
            PLAY_FRAME(obj, 0x56b);
        }
        break;
    case 62:
        obj = &thing[employee->object];
        if (count_surgeons(employee->room) == 2) {
            PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[22]);
            PLAY_FRAME(obj, 0x56c);
        } else {
            PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[21]);
            PLAY_FRAME(obj, 0x56a);
        }
        break;
    default:
        break;
    }
    repeat = entry->repeat;
    if (repeat) {
        if (employee->complex_repeat) {
            employee->complex_repeat--;
            if ((s8)employee->complex_repeat == 0) {
                employee->complex_pc++;
            }
        } else if (repeat < 0) {
            employee->complex_repeat = old_newrand() % -repeat + 1;
        } else {
            employee->complex_repeat = repeat;
        }
    } else {
        employee->complex_pc++;
    }
    if (staff_complex_group[employee->complex_pc] == 0) {
        employee->complex_pc = 0;
        action = employee->complex_queued;
        if (action) {
            arg = employee->queued_arg_x + (employee->queued_arg_y << 7);
            employee->complex_queued = employee->complex_queued2;
            employee->complex_queued2 = 0;
            employee->queued_arg_x = employee->queued2_arg_x;
            employee->queued2_arg_x = 0;
            employee->queued_arg_y = employee->queued2_arg_y;
            employee->queued2_arg_y = 0;
            if (set_complex(employee, action, arg)) {
                process_staff_complex(employee);
            }
        } else if (employee->next_state) {
            employee->state = employee->next_state;
            employee->next_state = 0;
        }
    } else if (entry->chain) {
        process_staff_complex(employee);
    }
    if (run_things) {
        process_things(employee, employee + 1);
    }
    return 1;
}

// FUNCTION: 0x0004029c
void play_complex_staff_anim(Thing *employee)
{
    CompFrame *f;
    u16 next;

    if (actual_game_turn % employee->anim_speed == 0) {
        next = frame[employee->sprite].next;
        f = &frame[next];
        if (f->flags & 1) {
            process_staff_complex(employee);
        } else {
            employee->sprite = next;
        }
    }
}

// FUNCTION: 0x0004031c
void staff_complex_retry(Thing *employee)
{
    CompFrame *f;

    if (actual_game_turn % employee->anim_speed == 0) {
        employee->sprite = frame[employee->sprite].next;
        f = &frame[employee->sprite];
        if (f->flags & 1) {
            process_staff_complex(employee);
            sprintf((char *)&debug.unknownc[0x64], "COMPLEX RETRY OCCURRING PEEP %d", (u16)(employee - thing), 0);
        }
    }
}

// FUNCTION: 0x000403d0
void staff_wait_for_complex_slave(Thing *employee)
{
    Thing *slave;

    slave = &thing[employee->slave];
    if (slave > thing) {
        if (slave->state == 0x20) {
            process_staff_complex(employee);
        }
    }
}

// FUNCTION: 0x0004042c
void merge_slave(Thing *employee)
{
    Thing *slave;
    Thing *obj;

    slave = &thing[employee->slave];
    if (slave > thing) {
        if (slave->state == 0x20) {
            slave->state = 0x28;
            obj = &thing[employee->object];
            if (obj > thing) {
                employee->layers[10] = obj->layers[10];
                employee->layers[11] = obj->layers[11];
                employee->layers[12] = obj->layers[12];
            }
            employee->layers[0] = slave->layers[0];
            employee->layers[1] = slave->layers[1];
            employee->layers[2] = slave->layers[2];
            employee->layers[3] = slave->layers[3];
            employee->layers[4] = slave->layers[4];
            slave->object = employee->object;
        }
    }
}


// NONMATCHING: register choice
// FUNCTION: 0x00040524
u8 process_person_complex(Thing *person)
{
    s8 dir;
    u8 run_things;
    u8 op;
    s8 repeat;
    s8 choice;
    u16 action;
    u16 arg;
    u16 tile;
    u16 obj_id;
    MapPos x;
    MapPos y;
    Thing *other;
    Complex *entry;
    Map *cell;
    Thing *obj;
    Thing *door;

    run_things = 0;
    if (person->complex_pc == 0) {
        return 2;
    }
    op = person_complex_group[person->complex_pc];
    if (op == 0) {
        person->complex_pc = 0;
        return 2;
    }
    tile = person->x.tile + (person->y.tile << 7);
    entry = &person_complex[op];
    if (entry->object) {
        obj_id = find_nearest_free_object(person - thing, entry->object);
        if (obj_id) {
            obj = &thing[obj_id];
        } else {
            fail_person_complex(person);
            return 0;
        }
    } else {
        obj = 0;
    }
    if (entry->kind) {
        person->state = entry->kind;
    }
    switch (entry->action) {
    case 0:
        break;
    case 2:
        person->object = obj_id;
    case 44:
        obj = &thing[person->object];
        set_destination_coords(person, obj->use_pos[1]);
        dir = person_head_for(person, 1);
        if (dir >= 0 && dir < 4) {
            person->facing = dir;
        }
        PLAY_FRAME(person, anim_starts[person->subtype].idle);
        person->countdown = 0;
        break;
    case 4:
        PLAY_FRAME(person, anim_starts[person->subtype].anims[entry->anim]);
        break;
    case 6:
        obj = &thing[person->object];
        if ((obj->in_use == 1 || obj->state == 0x12) || obj->type != THING_OBJECT
            || obj->use_pos[1] != ((s16)person->y.tile << 7) + (s16)person->x.tile) {
            fail_person_complex(person);
            return 3;
        }
        obj->in_use = 1;
        obj->user = person - thing;
        person->facing = obj->facing;
        if ((obj->use_style == 1 || obj->use_style == 2) || obj->use_style == 3) {
            person->exit_tile = obj->use_pos[4];
            move_node(person, *(MapPos *)&obj->use_x, *(MapPos *)&obj->use_y);
            obj->state = 4;
            person->depth = obj->depth;
            if (objects[obj->subtype].unknown1e) {
                obj->saved_depth = obj->depth;
                obj->depth = 8;
            }
            map[(s16)person->x.tile + ((s16)person->y.tile << 7)].flags |= 0x20;
        }
        person->layers[10] = obj->layers[10];
        person->layers[11] = obj->layers[11];
        person->layers[12] = obj->layers[12];
        obj->layers[0] = person->layers[0];
        obj->layers[1] = person->layers[1];
        obj->layers[2] = person->layers[2];
        obj->layers[3] = person->layers[3];
        obj->layers[4] = person->layers[4];
        break;
    case 8:
        obj = &thing[person->object];
        obj->user = 0;
        person->object = 0;
        obj->in_use = 0;
        if (obj->use_style == 1 || obj->use_style == 3) {
            adjust_big_cell_flag(tile);
            x.tile = obj->use_pos[1] % 128;
            y.tile = obj->use_pos[1] / 128;
            x.frac = y.frac = 0x80;
            move_node(person, x, y);
            person->depth = 0;
            if (objects[obj->subtype].unknown1e) {
                obj->depth = obj->saved_depth;
                obj->saved_depth = 0;
            }
            obj->state = OS_STATIC;
        }
        PLAY_FRAME(person, anim_starts[person->subtype].unknown12);
        person->layers[10] = person->layers[11] = person->layers[12] = 0;
        break;
    case 10:
        obj = &thing[person->object];
        if (obj > thing) {
            PLAY_FRAME(obj, entry->anim);
        }
        break;
    case 12:
        break;
    case 16:
        choice = old_newrand() % group_anim[entry->anim].count;
        PLAY_FRAME(person, anim_starts[person->subtype].anims[group_anim[entry->anim].anims[choice]]);
        break;
    case 14:
        obj = find_door(person);
        if (obj > thing) {
            person->facing = opposite_angle[obj->facing];
        }
        break;
    case 20:
        door = find_door(person);
        if (door > thing) {
            u8 double_door;

            double_door = door->subtype != OBJ_DOOR ? (u8)1 : (u8)0;
            if (door->in_use == 1) {
                person->state = 0x2a;
                return 3;
            }
            if (person->room == 0 && (room_info[door->room_id].f22 || room_info[door->room_id].f3 == 4)) {
                fail_person_complex(person);
                return 3;
            }
            door->in_use = 1;
            if (person->room) {
                person->facing = door->facing;
            } else {
                person->facing = opposite_angle[door->facing];
            }
            if (tile == door->use_pos[0]) {
                person->exit_tile = door->use_pos[3];
            } else {
                person->exit_tile = door->use_pos[0];
            }
            if (double_door) {
                PLAY_FRAME(person, anim_starts[person->subtype].anims[67]);
            } else {
                PLAY_FRAME(person, anim_starts[person->subtype].anims[7]);
            }
            person->object = door - thing;
            door->user = person - thing;
            move_node(person, door->x, door->y);
            door->state = 4;
            cell = &map[(s16)door->x.tile + ((s16)door->y.tile << 7)];
            door->aux_param = cell->flags & 0x10 ? (u8)1 : (u8)0;
            cell->flags |= 0x10;
            if (double_door) {
                other = &thing[door->door_partner];
                other->state = OS_DOUBLE_DOOR_SWINGS;
                other->sprite = start[person->facing + 0x3f8].frame;
                other->flip = start[person->facing + 0x3f8].flags;
                other->anim_speed = person->anim_speed;
            }
        } else {
            fail_person_complex(person);
            return 0;
        }
        break;
    case 22:
        door = &thing[person->object];
        x.tile = person->exit_tile % 128;
        y.tile = person->exit_tile / 128;
        x.frac = y.frac = 0x80;
        person->dest_x = x.tile;
        person->dest_y = y.tile;
        move_node(person, x, y);
        door->user = 0;
        person->object = 0;
        move_person_in_room_list(person, person->exit_tile);
        cell = &map[(s16)door->x.tile + ((s16)door->y.tile << 7)];
        if (!door->aux_param) {
            cell->flags &= ~0x10;
        }
        if (door->subtype == OBJ_DOOR) {
            door->in_use = 0;
            door->state = OS_STATIC;
        } else {
            door->state = OS_DOUBLE_DOOR_SWINGS;
            door->sprite = start[person->facing + 0x400].frame;
            door->flip = start[person->facing + 0x400].flags;
        }
        break;
    case 24:
        person->complex_pc++;
        person->complex_resume = (u16)person_complex_group[person->complex_pc] * 10;
        break;
    case 26:
        break;
    case 32:
        if (transform_peep_who(person, entry->anim)) {
            PLAY_FRAME(person, anim_starts[person->subtype].idle);
        }
        break;
    case 34:
        person->subtype = person->orig_subtype;
        break;
    case 46:
        dir = person_head_for(person, 1);
        if (dir >= 0 && dir < 4) {
            person->facing = dir;
        }
        PLAY_FRAME(person, anim_starts[person->subtype].idle);
        break;
    case 18:
        obj_id = person->object;
        if (obj_id) {
            s8 kind;
            s8 face;
            s8 d;

            kind = thing[obj_id].subtype;
            face = objects[kind].unknown6[0];
            if (face != -1) {
                d = face_dirns[face].dirs[thing[obj_id].facing + 4];
                if (d != -1) {
                    person->facing = d;
                    PLAY_FRAME(person, anim_starts[person->subtype].unknown12);
                }
            }
        }
        break;
    case 52:
        complex_success(person, entry->anim);
        break;
    case 50:
        run_things = 1;
        break;
    default:
        break;
    }
    repeat = entry->repeat;
    if (repeat) {
        if (person->complex_repeat) {
            person->complex_repeat--;
            if ((s8)person->complex_repeat == 0) {
                person->complex_pc++;
            }
        } else if (repeat < 0) {
            person->complex_repeat = old_newrand() % -repeat + 1;
        } else {
            person->complex_repeat = repeat;
        }
    } else {
        person->complex_pc++;
    }
    if (person_complex_group[person->complex_pc] == 0) {
        person->complex_pc = 0;
        action = person->complex_queued;
        if (action) {
            arg = person->queued_arg_x + (person->queued_arg_y << 7);
            person->complex_queued = person->complex_queued2;
            person->complex_queued2 = 0;
            person->queued_arg_x = person->queued2_arg_x;
            person->queued2_arg_x = 0;
            person->queued_arg_y = person->queued2_arg_y;
            person->queued2_arg_y = 0;
            if (set_complex(person, action, arg)) {
                process_person_complex(person);
            }
        } else if (person->next_state) {
            person->state = person->next_state;
            person->next_state = 0;
        }
    } else if (entry->chain) {
        process_person_complex(person);
    }
    if (run_things) {
        process_things(person, person + 1);
    }
    return 1;
}

// FUNCTION: 0x000417ac
u8 set_complex(Thing *w, u16 action, u16 arg)
{
    if (w->complex_pc == 0) {
        if (w->complex_resume) {
            w->complex_pc = w->complex_resume;
            w->complex_action = action;
            if (w->complex_queued) {
                w->complex_queued2 = action;
                set_next2_destination_coords(w, arg);
            } else {
                w->complex_queued = action;
                set_next_destination_coords(w, arg);
            }
            w->complex_resume = 0;
        } else {
            w->complex_pc = w->complex_action = action;
            if (arg) {
                set_destination_coords(w, arg);
            }
        }
        return 1;
    }
    if (w->complex_queued) {
        w->complex_queued2 = action;
        if (arg) {
            set_next2_destination_coords(w, arg);
        }
        return 0;
    }
    w->complex_queued = w->complex_action = action;
    if (arg) {
        set_next_destination_coords(w, arg);
    }
    return 0;
}

// FUNCTION: 0x000418dc
void play_complex_person_anim(Thing *person)
{
    CompFrame *f;
    u16 next;

    if (actual_game_turn % person->anim_speed == 0) {
        next = frame[person->sprite].next;
        f = &frame[next];
        if (f->flags & 1) {
            process_person_complex(person);
        } else {
            person->sprite = next;
        }
    }
}

// FUNCTION: 0x0004195c
void set_destination_coords(Thing *w, u16 pos)
{
    w->dest_x = pos % 128;
    w->dest_y = pos / 128;
    w->countdown = 0;
    nav_init(w);
}

// FUNCTION: 0x000419bc
void set_next_destination_coords(Thing *w, u16 pos)
{
    w->queued_arg_x = pos % 128;
    w->queued_arg_y = pos / 128;
}

// FUNCTION: 0x00041a08
void set_next2_destination_coords(Thing *w, u16 pos)
{
    w->queued2_arg_x = pos % 128;
    w->queued2_arg_y = pos / 128;
}

// FUNCTION: 0x00041a54
void person_complex_retry(Thing *person)
{
    CompFrame *f;

    if (actual_game_turn % person->anim_speed == 0) {
        person->sprite = frame[person->sprite].next;
        f = &frame[person->sprite];
        if (f->flags & 1) {
            process_person_complex(person);
            sprintf((char *)&debug.unknownc[0x64], "COMPLEX RETRY OCCURRING PEEP %d", (u16)(person - thing), 0);
        }
    }
}

// FUNCTION: 0x00041b08
void person_wait_for_complex_slave(Thing *person)
{
    Thing *slave;
    Thing *obj;

    slave = &thing[person->slave];
    if (slave > thing) {
        if (slave->state == PS_ENTERTAINS_THEMSELF) {
        person->state = person->next_state;
        person->next_state = 0;
        process_person_complex(person);
        slave->state = 0x2a;
        obj = &thing[person->object];
        if (obj > thing) {
            person->layers[5] = slave->layers[5];
            person->layers[6] = slave->layers[6];
            person->layers[7] = slave->layers[7];
            person->layers[8] = slave->layers[8];
            person->layers[9] = slave->layers[9];
        }
        person->layers[10] = obj->layers[10];
        person->layers[11] = obj->layers[11];
        person->layers[12] = obj->layers[12];
        }
    }
}

// FUNCTION: 0x00041c0c
void fail_staff_complex(Thing *employee)
{
    if (employee->resume_state) {
        u16 anim;

        employee->state = employee->resume_state;
        employee->complex_pc = employee->complex_queued = employee->complex_queued2 = 0;
        employee->complex_action = employee->complex_resume = 0;
        anim = check_frame(staff_anim_starts[employee->subtype].unknownA, employee) + (u16)employee->facing;
        employee->sprite = start[anim].frame;
        employee->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
        if (employee->subtype == STAFF_HANDYMAN) {
            thingpage_cancel_my_response(&allhosps[employee->owner].fd2b, employee);
            thingpage_cancel_my_response(&allhosps[employee->owner].ff6d, employee);
        }
    } else {
        employee->state = SS_COMPLEX_RETRY;
    }
}

// FUNCTION: 0x00041d54
void fail_person_complex(Thing *person)
{
    if (person->resume_state) {
        u16 anim;

        person->state = person->resume_state;
        person->complex_pc = person->complex_queued = person->complex_queued2 = 0;
        person->complex_action = person->complex_resume = 0;
        anim = check_frame(anim_starts[person->subtype].unknown12, person) + (u16)person->facing;
        person->sprite = start[anim].frame;
        person->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
    } else {
        person->state = 0x2a;
    }
}

// FUNCTION: 0x00041e44
u8 check_room_mutex_on_entry(Thing *w)
{
    RoomInfo *room;
    u16 *room_staff;

    room = which_room(w->exit_tile);
    if (room <= room_info) {
        return 1;
    }
    room_staff = room->staff;
    if ((u16)(w - thing) == room_staff[0] || (u16)(w - thing) == room_staff[1]) {
        return 1;
    }
    if (room->type == ROOM_THEATRE) {
        if (room_staff[0] == 0) {
            room_staff[0] = w - thing;
        } else if (room_staff[1] == 0) {
            room_staff[1] = w - thing;
        } else {
            return 0;
        }
        return 1;
    }
    if (room_staff[0] == 0) {
        room_staff[0] = w - thing;
        return 1;
    }
    return 0;
}

// FUNCTION: 0x00041f84
void abandon_complex(Thing *w)
{
    u16 i;
    MapPos x;
    MapPos old_y;
    MapPos old_x;
    MapPos y;
    Thing *patient;
    RoomInfo *room;
    Thing *obj;

    x.frac = y.frac = 0x80;
    x.tile = w->dest_x;
    y.tile = w->dest_y;
    old_x = w->x;
    old_y = w->y;
    obj = &thing[w->object];
    if (obj > thing) {
        if ((u16)(w - thing) == obj->user) {
            if (w->type == THING_STAFF && w->subtype == STAFF_RECEPTIONIST) {
                set_complex(w, 0x4ba, 0);
                w->room = 0;
            } else {
                set_complex(w, 0xf0, 0);
            }
            for (i = 0; i < 12; i++) {
                x.tile = w->dest_x;
                y.tile = w->dest_y;
                move_node(w, x, y);
                if (process_complex_either(w) != 1) {
                    break;
                }
            }
        } else {
            if (w->type == THING_STAFF && w->subtype == STAFF_RECEPTIONIST) {
                set_complex(w, 0x4ba, 0);
                w->room = 0;
            } else {
                set_complex(w, 0xf0, 0);
            }
            room = w->room;
            for (i = 0; i < 12; i++) {
                x.tile = w->dest_x;
                y.tile = w->dest_y;
                move_node(w, x, y);
                if (process_complex_either(w) != 1) {
                    break;
                }
            }
            if (room == w->room) {
                move_node(w, old_x, old_y);
            }
        }
    } else {
        set_complex(w, 0xf0, 0);
        room = w->room;
        for (i = 0; i < 12; i++) {
            x.tile = w->dest_x;
            y.tile = w->dest_y;
            move_node(w, x, y);
            if (process_complex_either(w) != 1) {
                break;
            }
        }
        if (room == w->room) {
            move_node(w, old_x, old_y);
        }
    }
    if (w->type == THING_STAFF) {
        patient = &thing[w->slave];
        if (patient > thing) {
            interrupt_person(patient);
            if (w->room == 0 || w->room != patient->room) {
                start_person_going(patient);
            }
        }
    }
    w->state = w->resume_state;
    switch (w->type) {
    case THING_PATIENT: {
        u16 anim;

        anim = check_frame(anim_starts[w->subtype].unknown12, w) + (u16)w->facing;
        w->sprite = start[anim].frame;
        w->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
        break;
    }
    case THING_STAFF: {
        u16 anim;

        anim = check_frame(staff_anim_starts[w->subtype].unknownA, w) + (u16)w->facing;
        w->sprite = start[anim].frame;
        w->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
        break;
    }
    default: {
        char buf[100];

        sprintf(buf, "Invalid model for abandon_complex thing number = %d", (u16)(w - thing));
        break;
    }
    }
}

// FUNCTION: 0x000423c8
void complex_success(Thing *w, u8 kind)
{
    switch (kind) {
    case 2:
        award_counts.f138++;
        reset_icon_value(w, ICON_THIRSTY);
        w->refreshment = 100;
        w->happiness += gbv.f112;
        w->litter_urge += 10;
        w->bladder += 10;
        if ((s16)w->warmth > 60) {
            w->warmth = 60;
        }
        w->cash -= 20;
        create_numfx(NUMFX_MONEY, 20, (u16)w->x.tile + ((u16)w->y.tile << 7), color.unknownD, 0x40);
        allhosps[w->owner].fd5 += 20;
        break;
    case 4:
        w->happiness++;
        w->cash -= 10;
        create_numfx(NUMFX_MONEY, 10, (u16)w->x.tile + ((u16)w->y.tile << 7), color.unknownD, 0x40);
        allhosps[w->owner].fd9 += 10;
        break;
    case 6:
        reset_icon_value(w, ICON_TOILET);
        w->happiness += gbv.f114;
        if (w->illness == ILL_THE_SQUITS) {
            w->bladder = gbv.f116 / 2;
        } else {
            w->bladder = 0;
        }
        w->impatience = 0;
        w->waiting_time = 0;
        break;
    case 8:
        research_contribution(w, 200);
        break;
    case 10:
        research_contribution(w, 400);
        break;
    case 12:
        person_autopsied(&thing[w->slave]);
        break;
    case 14:
        research_contribution(w, 50);
        break;
    case 16:
        research_contribution(w, 100);
        break;
    case 18:
        rest_staff(w, gbv.fdc);
        break;
    case 20:
        rest_staff(w, gbv.fe0);
        break;
    case 22:
        rest_staff(w, gbv.fde);
        break;
    case 24:
        thing[w->object].times_used++;
        break;
    default:
        break;
    }
    if ((s16)w->happiness < 0) {
        w->happiness = 0;
    } else if ((s16)w->happiness > 100) {
        w->happiness = 100;
    }
}

// FUNCTION: 0x00042748
u8 staff_go_near_object(Thing *w)
{
    u16 pos;

    pos = can_i_go_near_point(w, (s16)((thing[w->object].y.tile << 7) + (u16)thing[w->object].x.tile));
    if (pos) {
        set_destination_coords(w, pos);
        return 1;
    }
    return 0;
}

// FUNCTION: 0x000427d8
void interrupt_complex(Thing *w)
{
    Thing *obj;

    w->complex_pc = 0;
    w->complex_queued = 0;
    w->complex_queued2 = 0;
    w->complex_resume = 0;
    w->next_state = 0;
    w->resume_state = 0;
    w->x.frac = w->y.frac = 0x80;
    w->countdown = 0;
    obj = &thing[w->object && (thing[w->object].type == THING_OBJECT || thing[w->object].type == THING_DOOR)
                 && (u16)(w - thing) == thing[w->object].user ? w->object : 0];
    if (obj > thing) {
        if (obj->type == THING_DOOR) {
            if (w->type == THING_STAFF) {
                set_complex(w, 0x4ce, 0);
                process_staff_complex(w);
            } else {
                set_complex(w, 0x26c, 0);
                process_person_complex(w);
            }
        } else if (w->type == THING_STAFF) {
            set_complex(w, 0x1b8, 0);
            process_staff_complex(w);
        } else {
            set_complex(w, 0xa0, 0);
            process_person_complex(w);
            w->speed = gbv.f14c;
            w->anim_speed = gbv.f14e;
        }
    }
}

// FUNCTION: 0x000429b4
void interrupt_peep(Thing *w)
{
    switch (w->type) {
    case THING_STAFF:
        interrupt_staff(w);
        break;
    case THING_PATIENT:
        interrupt_person(w);
        break;
    default:
        break;
    }
}
