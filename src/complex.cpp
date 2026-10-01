#include "hosp.h"

extern "C" {
void fn_000419bc(Thing *w, u16 arg);
void fn_00041a08(Thing *w, u16 arg);
u8 fn_0003e6c0(Thing *w);
void fn_00041a54(Thing *w);
void fn_00041b08(Thing *w);
void fn_000444e0(Thing *w);
}

extern "C" {
extern u8 *D_00153f9c;
extern ComplexAnims D_001310ac[];
void fn_00041c0c(Thing *w);
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
void remove_person_from_room_list(Thing *w);
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
        anim = check_frame(a, t) + (u16)(t)->unknown17; \
        (t)->sprite = start[anim].frame; \
        (t)->palette = start[anim].flags > 0 ? (u8)1 : (u8)0; \
    }
void rest_staff(Thing *w, s16 amount);
void research_contribution(Thing *w, s16 amount);
void person_autopsied(Thing *w);

// FUNCTION: 0x0003e6c0
extern "C" u8 fn_0003e6c0(Thing *w)
{
    switch (w->renderer) {
    case 4:
        return fn_0003e724(w);
    case 2:
        return process_person_complex(w);
    }
    return 0;
}

// FUNCTION: 0x0003e724
u8 fn_0003e724(Thing *employee)
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
    if (employee->unknown61 == 0) {
        return 2;
    }
    op = D_00153f9c[employee->unknown61];
    if (op == 0) {
        employee->unknown61 = 0;
        return 2;
    }
    tile = employee->x.tile + (employee->y.tile << 7);
    entry = &staff_complex[op];
    if (entry->object) {
        obj_id = find_nearest_free_object(employee - thing, entry->object);
        if (obj_id) {
            obj = &thing[obj_id];
        } else {
            fn_00041c0c(employee);
            return 0;
        }
    } else {
        obj = 0;
    }
    if (entry->kind) {
        employee->kind = entry->kind;
    }
    switch (entry->action) {
    case 0:
        break;
    case 56:
        if (staff_go_near_object(employee)) {
            dir = person_head_for(employee, 1);
            if (dir >= 0 && dir < 4) {
                employee->unknown17 = dir;
            }
            PLAY_FRAME(employee, staff_anim_starts[employee->subtype].idle);
            employee->height = 0;
        } else {
            fn_00041c0c(employee);
            return 0;
        }
        break;
    case 2:
        employee->unknown69 = obj_id;
    case 44:
        obj = &thing[employee->unknown69];
        fn_0004195c(employee, obj->use_pos[employee->subtype == 2 ? 2 : 0]);
        dir = person_head_for(employee, 1);
        if (dir >= 0 && dir < 4) {
            employee->unknown17 = dir;
        }
        PLAY_FRAME(employee, staff_anim_starts[employee->subtype].idle);
        employee->height = 0;
        break;
    case 4:
        PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[entry->anim]);
        break;
    case 54:
        patient = &thing[employee->unknown73];
        if (patient > thing) {
            u16 anim;

            anim = check_frame(anim_starts[patient->subtype].anims[entry->anim], thing) + (u16)employee->unknown17;
            employee->sprite = start[anim].frame;
            employee->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
        }
        break;
    case 6:
        obj = &thing[employee->unknown69];
        if ((obj->unknown98 == 1 || obj->kind == 0x12) || obj->renderer != 6
            || obj->use_pos[employee->subtype == 2 ? 2 : 0] != ((s16)employee->y.tile << 7) + (s16)employee->x.tile) {
            fn_00041c0c(employee);
            return 3;
        }
        obj->unknown98 = 1;
        obj->unknown91 = employee - thing;
        employee->unknown17 = obj->unknown17;
        if (obj->unknown97 == 1 || obj->unknown97 == 2) {
            employee->unknown71 = obj->use_pos[(employee->subtype == 2 ? 2 : 0) + 3];
            move_node(employee, *(MapPos *)&obj->unknown63, *(MapPos *)&obj->unknown65);
            obj->kind = 4;
            employee->depth = obj->depth;
            if (objects[obj->subtype].unknown1e) {
                obj->unknown112[1] = obj->depth;
                obj->depth = 8;
            }
            map[(s16)employee->x.tile + ((s16)employee->y.tile << 7)].flags |= 0x20;
        }
        employee->unknown39 = obj->unknown39;
        employee->unknown40 = obj->unknown40;
        employee->unknown41 = obj->unknown41;
        break;
    case 8:
        obj = &thing[employee->unknown69];
        obj->unknown91 = 0;
        employee->unknown69 = 0;
        obj->unknown98 = 0;
        if (obj->unknown97 == 2 || obj->unknown97 == 1) {
            adjust_big_cell_flag(tile);
            x.tile = obj->use_pos[(employee->subtype == 2 ? 2 : 0) + 3] % 128;
            y.tile = obj->use_pos[(employee->subtype == 2 ? 2 : 0) + 3] / 128;
            x.frac = y.frac = 0x80;
            move_node(employee, x, y);
            employee->depth = 0;
            if (objects[obj->subtype].unknown1e) {
                obj->depth = obj->unknown112[1];
                obj->unknown112[1] = 0;
            }
            obj->kind = 2;
            if (obj->unknown97 == 2) {
                patient = &thing[employee->unknown73];
                if (patient > thing) {
                    x.tile = obj->unknown85 % 128;
                    y.tile = obj->unknown85 / 128;
                    move_node(patient, x, y);
                    patient->unknown69 = 0;
                }
            }
        }
        PLAY_FRAME(employee, staff_anim_starts[employee->subtype].unknownA);
        employee->unknown39 = employee->unknown40 = employee->unknown41 = 0;
        break;
    case 10:
        obj = &thing[employee->unknown69];
        if (obj > thing) {
            PLAY_FRAME(obj, entry->anim);
        }
        break;
    case 16:
        choice = old_newrand() % D_001310ac[entry->anim].count;
        PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[D_001310ac[entry->anim].anims[choice]]);
        break;
    case 14:
        obj = find_door(employee);
        if (obj > thing) {
            employee->unknown17 = opposite_angle[obj->unknown17];
        }
        break;
    case 20:
        door = find_door(employee);
        if (door > thing) {
            u8 flag;

            flag = door->subtype != 3 ? (u8)1 : (u8)0;
            if (door->unknown98 == 1) {
                employee->kind = 0x2c;
                return 3;
            }
            if (employee->room) {
                employee->unknown17 = door->unknown17;
            } else {
                employee->unknown17 = opposite_angle[door->unknown17];
            }
            if (tile == door->use_pos[0]) {
                employee->unknown71 = door->use_pos[3];
            } else {
                employee->unknown71 = door->use_pos[0];
            }
            if (entry->anim == 1 && !check_room_mutex_on_entry(employee)) {
                fn_00041c0c(employee);
                return 4;
            }
            if (employee->room == 0
                && ((room_info[door->unknown89].f22 || room_info[door->unknown89].f3 == 4)
                    || door->unknown89 == allhosps[door->owner].unknown11b7)) {
                fn_00041c0c(employee);
                return 4;
            }
            if (flag) {
                PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[8]);
            } else {
                PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[6]);
            }
            door->unknown98 = 1;
            employee->unknown69 = door - thing;
            door->unknown91 = employee - thing;
            move_node(employee, door->x, door->y);
            door->kind = 4;
            cell = &map[(s16)door->x.tile + ((s16)door->y.tile << 7)];
            door->unknown104 = cell->flags & 0x10 ? (u8)1 : (u8)0;
            cell->flags |= 0x10;
            if (flag) {
                other = &thing[door->unknown100];
                other->kind = 0x10;
                other->sprite = start[employee->unknown17 + 0x3f8].frame;
                other->palette = start[employee->unknown17 + 0x3f8].flags;
                other->anim_speed = employee->anim_speed;
            }
        } else {
            fn_00041c0c(employee);
            return 4;
        }
        break;
    case 22:
        remove_person_from_room_list(employee);
        door = &thing[employee->unknown69];
        door->unknown91 = 0;
        employee->unknown69 = 0;
        x.tile = employee->unknown71 % 128;
        y.tile = employee->unknown71 / 128;
        x.frac = y.frac = 0x80;
        employee->unknown19 = x.tile;
        employee->unknown20 = y.tile;
        move_node(employee, x, y);
        move_person_in_room_list(employee, employee->unknown71);
        if (employee->room) {
            employee->room->unknown29 = 0;
            pager_arrival(employee);
        }
        cell = &map[(s16)door->x.tile + ((s16)door->y.tile << 7)];
        if (!door->unknown104) {
            cell->flags &= ~0x10;
        }
        if (door->subtype == 3) {
            door->unknown98 = 0;
            door->kind = 2;
        } else {
            door->kind = 0x10;
            door->sprite = start[employee->unknown17 + 0x400].frame;
            door->palette = start[employee->unknown17 + 0x400].flags;
        }
        break;
    case 24:
        employee->unknown61++;
        employee->unknown65 = (u16)D_00153f9c[employee->unknown61] * 10;
        break;
    case 26:
        employee->unknown61 = employee->unknown63 - 1;
        employee->unknown63 = 0;
        break;
    case 28:
        employee->unknown61++;
        start_new_interaction(employee, D_00153f9c[employee->unknown61] + 1);
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
        patient = &thing[employee->unknown73];
        if (patient > thing) {
            employee->unknown61++;
            patient->state_b = 0x20;
            set_complex(patient, (u16)D_00153f9c[employee->unknown61] * 10, 0);
            fn_0003e6c0(patient);
        } else {
            fn_00041c0c(employee);
            return 0;
        }
        break;
    case 40:
        patient = &thing[employee->unknown73];
        if (patient > thing) {
            employee->unknown61++;
            patient->state_b = 0x20;
            set_complex(patient, (u16)D_00153f9c[employee->unknown61] * 10, 0);
        } else {
            fn_00041c0c(employee);
            return 0;
        }
        break;
    case 46:
        dir = person_head_for(employee, 1);
        if (dir >= 0 && dir < 4) {
            employee->unknown17 = dir;
        }
        PLAY_FRAME(employee, staff_anim_starts[employee->subtype].idle);
        break;
    case 48:
        patient = &thing[employee->unknown73];
        if (patient > thing && (obj = &thing[employee->unknown69]) > thing) {
            x.tile = obj->unknown85 % 128;
            y.tile = obj->unknown85 / 128;
            x.frac = y.frac = 0x80;
            move_node(patient, x, y);
            patient->unknown69 = 0;
        }
        break;
    case 18:
        obj_id = employee->unknown69;
        if (obj_id) {
            s8 kind;
            s8 face;
            s8 d;

            kind = thing[obj_id].subtype;
            face = objects[kind].unknown6[0];
            if (face != -1) {
                d = face_dirns[face].dirs[thing[obj_id].unknown17];
                if (d != -1) {
                    employee->unknown17 = d;
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
        obj = &thing[employee->unknown69];
        if (count_surgeons(employee->room) == 1) {
            PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[19]);
            PLAY_FRAME(obj, 0x56c);
        } else {
            PLAY_FRAME(employee, staff_anim_starts[employee->subtype].anims[20]);
            PLAY_FRAME(obj, 0x56b);
        }
        break;
    case 62:
        obj = &thing[employee->unknown69];
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
        if (employee->unknown93) {
            employee->unknown93--;
            if ((s8)employee->unknown93 == 0) {
                employee->unknown61++;
            }
        } else if (repeat < 0) {
            employee->unknown93 = old_newrand() % -repeat + 1;
        } else {
            employee->unknown93 = repeat;
        }
    } else {
        employee->unknown61++;
    }
    if (D_00153f9c[employee->unknown61] == 0) {
        employee->unknown61 = 0;
        action = employee->unknown63;
        if (action) {
            arg = employee->unknown95 + (employee->unknown96 << 7);
            employee->unknown63 = employee->unknown75w;
            employee->unknown75w = 0;
            employee->unknown95 = employee->unknown97;
            employee->unknown97 = 0;
            employee->unknown96 = employee->unknown98;
            employee->unknown98 = 0;
            if (set_complex(employee, action, arg)) {
                fn_0003e724(employee);
            }
        } else if (employee->state_b) {
            employee->kind = employee->state_b;
            employee->state_b = 0;
        }
    } else if (entry->chain) {
        fn_0003e724(employee);
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
            fn_0003e724(employee);
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
            fn_0003e724(employee);
            sprintf((char *)&debug.unknownc[0x64], "COMPLEX RETRY OCCURRING PEEP %d", (u16)(employee - thing), 0);
        }
    }
}

// FUNCTION: 0x000403d0
void staff_wait_for_complex_slave(Thing *employee)
{
    Thing *slave;

    slave = &thing[employee->unknown73];
    if (slave > thing) {
        if (slave->kind == 0x20) {
            fn_0003e724(employee);
        }
    }
}

// FUNCTION: 0x0004042c
void merge_slave(Thing *employee)
{
    Thing *slave;
    Thing *obj;

    slave = &thing[employee->unknown73];
    if (slave > thing) {
        if (slave->kind == 0x20) {
            slave->kind = 0x28;
            obj = &thing[employee->unknown69];
            if (obj > thing) {
                employee->unknown39 = obj->unknown39;
                employee->unknown40 = obj->unknown40;
                employee->unknown41 = obj->unknown41;
            }
            employee->unknown29 = slave->unknown29;
            employee->unknown30[0] = slave->unknown30[0];
            employee->unknown30[1] = slave->unknown30[1];
            employee->unknown30[2] = slave->unknown30[2];
            employee->unknown30[3] = slave->unknown30[3];
            slave->unknown69 = employee->unknown69;
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
    if (person->unknown61 == 0) {
        return 2;
    }
    op = person_complex_group[person->unknown61];
    if (op == 0) {
        person->unknown61 = 0;
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
        person->kind = entry->kind;
    }
    switch (entry->action) {
    case 0:
        break;
    case 2:
        person->unknown69 = obj_id;
    case 44:
        obj = &thing[person->unknown69];
        fn_0004195c(person, obj->use_pos[1]);
        dir = person_head_for(person, 1);
        if (dir >= 0 && dir < 4) {
            person->unknown17 = dir;
        }
        PLAY_FRAME(person, anim_starts[person->subtype].idle);
        person->height = 0;
        break;
    case 4:
        PLAY_FRAME(person, anim_starts[person->subtype].anims[entry->anim]);
        break;
    case 6:
        obj = &thing[person->unknown69];
        if ((obj->unknown98 == 1 || obj->kind == 0x12) || obj->renderer != 6
            || obj->use_pos[1] != ((s16)person->y.tile << 7) + (s16)person->x.tile) {
            fail_person_complex(person);
            return 3;
        }
        obj->unknown98 = 1;
        obj->unknown91 = person - thing;
        person->unknown17 = obj->unknown17;
        if ((obj->unknown97 == 1 || obj->unknown97 == 2) || obj->unknown97 == 3) {
            person->unknown71 = obj->unknown85;
            move_node(person, *(MapPos *)&obj->unknown63, *(MapPos *)&obj->unknown65);
            obj->kind = 4;
            person->depth = obj->depth;
            if (objects[obj->subtype].unknown1e) {
                obj->unknown112[1] = obj->depth;
                obj->depth = 8;
            }
            map[(s16)person->x.tile + ((s16)person->y.tile << 7)].flags |= 0x20;
        }
        person->unknown39 = obj->unknown39;
        person->unknown40 = obj->unknown40;
        person->unknown41 = obj->unknown41;
        obj->unknown29 = person->unknown29;
        obj->unknown30[0] = person->unknown30[0];
        obj->unknown30[1] = person->unknown30[1];
        obj->unknown30[2] = person->unknown30[2];
        obj->unknown30[3] = person->unknown30[3];
        break;
    case 8:
        obj = &thing[person->unknown69];
        obj->unknown91 = 0;
        person->unknown69 = 0;
        obj->unknown98 = 0;
        if (obj->unknown97 == 1 || obj->unknown97 == 3) {
            adjust_big_cell_flag(tile);
            x.tile = obj->use_pos[1] % 128;
            y.tile = obj->use_pos[1] / 128;
            x.frac = y.frac = 0x80;
            move_node(person, x, y);
            person->depth = 0;
            if (objects[obj->subtype].unknown1e) {
                obj->depth = obj->unknown112[1];
                obj->unknown112[1] = 0;
            }
            obj->kind = 2;
        }
        PLAY_FRAME(person, anim_starts[person->subtype].unknown12);
        person->unknown39 = person->unknown40 = person->unknown41 = 0;
        break;
    case 10:
        obj = &thing[person->unknown69];
        if (obj > thing) {
            PLAY_FRAME(obj, entry->anim);
        }
        break;
    case 12:
        break;
    case 16:
        choice = old_newrand() % D_001310ac[entry->anim].count;
        PLAY_FRAME(person, anim_starts[person->subtype].anims[D_001310ac[entry->anim].anims[choice]]);
        break;
    case 14:
        obj = find_door(person);
        if (obj > thing) {
            person->unknown17 = opposite_angle[obj->unknown17];
        }
        break;
    case 20:
        door = find_door(person);
        if (door > thing) {
            u8 flag;

            flag = door->subtype != 3 ? (u8)1 : (u8)0;
            if (door->unknown98 == 1) {
                person->kind = 0x2a;
                return 3;
            }
            if (person->room == 0 && (room_info[door->unknown89].f22 || room_info[door->unknown89].f3 == 4)) {
                fail_person_complex(person);
                return 3;
            }
            door->unknown98 = 1;
            if (person->room) {
                person->unknown17 = door->unknown17;
            } else {
                person->unknown17 = opposite_angle[door->unknown17];
            }
            if (tile == door->use_pos[0]) {
                person->unknown71 = door->use_pos[3];
            } else {
                person->unknown71 = door->use_pos[0];
            }
            if (flag) {
                PLAY_FRAME(person, anim_starts[person->subtype].anims[67]);
            } else {
                PLAY_FRAME(person, anim_starts[person->subtype].anims[7]);
            }
            person->unknown69 = door - thing;
            door->unknown91 = person - thing;
            move_node(person, door->x, door->y);
            door->kind = 4;
            cell = &map[(s16)door->x.tile + ((s16)door->y.tile << 7)];
            door->unknown104 = cell->flags & 0x10 ? (u8)1 : (u8)0;
            cell->flags |= 0x10;
            if (flag) {
                other = &thing[door->unknown100];
                other->kind = 0x10;
                other->sprite = start[person->unknown17 + 0x3f8].frame;
                other->palette = start[person->unknown17 + 0x3f8].flags;
                other->anim_speed = person->anim_speed;
            }
        } else {
            fail_person_complex(person);
            return 0;
        }
        break;
    case 22:
        door = &thing[person->unknown69];
        x.tile = person->unknown71 % 128;
        y.tile = person->unknown71 / 128;
        x.frac = y.frac = 0x80;
        person->unknown19 = x.tile;
        person->unknown20 = y.tile;
        move_node(person, x, y);
        door->unknown91 = 0;
        person->unknown69 = 0;
        move_person_in_room_list(person, person->unknown71);
        cell = &map[(s16)door->x.tile + ((s16)door->y.tile << 7)];
        if (!door->unknown104) {
            cell->flags &= ~0x10;
        }
        if (door->subtype == 3) {
            door->unknown98 = 0;
            door->kind = 2;
        } else {
            door->kind = 0x10;
            door->sprite = start[person->unknown17 + 0x400].frame;
            door->palette = start[person->unknown17 + 0x400].flags;
        }
        break;
    case 24:
        person->unknown61++;
        person->unknown65 = (u16)person_complex_group[person->unknown61] * 10;
        break;
    case 26:
        break;
    case 32:
        if (transform_peep_who(person, entry->anim)) {
            PLAY_FRAME(person, anim_starts[person->subtype].idle);
        }
        break;
    case 34:
        person->subtype = person->unknown170;
        break;
    case 46:
        dir = person_head_for(person, 1);
        if (dir >= 0 && dir < 4) {
            person->unknown17 = dir;
        }
        PLAY_FRAME(person, anim_starts[person->subtype].idle);
        break;
    case 18:
        obj_id = person->unknown69;
        if (obj_id) {
            s8 kind;
            s8 face;
            s8 d;

            kind = thing[obj_id].subtype;
            face = objects[kind].unknown6[0];
            if (face != -1) {
                d = face_dirns[face].dirs[thing[obj_id].unknown17 + 4];
                if (d != -1) {
                    person->unknown17 = d;
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
        if (person->unknown93) {
            person->unknown93--;
            if ((s8)person->unknown93 == 0) {
                person->unknown61++;
            }
        } else if (repeat < 0) {
            person->unknown93 = old_newrand() % -repeat + 1;
        } else {
            person->unknown93 = repeat;
        }
    } else {
        person->unknown61++;
    }
    if (person_complex_group[person->unknown61] == 0) {
        person->unknown61 = 0;
        action = person->unknown63;
        if (action) {
            arg = person->unknown95 + (person->unknown96 << 7);
            person->unknown63 = person->unknown75w;
            person->unknown75w = 0;
            person->unknown95 = person->unknown97;
            person->unknown97 = 0;
            person->unknown96 = person->unknown98;
            person->unknown98 = 0;
            if (set_complex(person, action, arg)) {
                process_person_complex(person);
            }
        } else if (person->state_b) {
            person->kind = person->state_b;
            person->state_b = 0;
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
    if (w->unknown61 == 0) {
        if (w->unknown65) {
            w->unknown61 = w->unknown65;
            w->icon = action;
            if (w->unknown63) {
                w->unknown75w = action;
                fn_00041a08(w, arg);
            } else {
                w->unknown63 = action;
                fn_000419bc(w, arg);
            }
            w->unknown65 = 0;
        } else {
            w->unknown61 = w->icon = action;
            if (arg) {
                fn_0004195c(w, arg);
            }
        }
        return 1;
    }
    if (w->unknown63) {
        w->unknown75w = action;
        if (arg) {
            fn_00041a08(w, arg);
        }
        return 0;
    }
    w->unknown63 = w->icon = action;
    if (arg) {
        fn_000419bc(w, arg);
    }
    return 0;
}

// FUNCTION: 0x000418dc
void person_complex_retry(Thing *person)
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
void fn_0004195c(Thing *w, u16 pos)
{
    w->unknown19 = pos % 128;
    w->unknown20 = pos / 128;
    w->height = 0;
    fn_000444e0(w);
}

// FUNCTION: 0x000419bc
void fn_000419bc(Thing *w, u16 pos)
{
    w->unknown95 = pos % 128;
    w->unknown96 = pos / 128;
}

// FUNCTION: 0x00041a08
void fn_00041a08(Thing *w, u16 pos)
{
    w->unknown97 = pos % 128;
    w->unknown98 = pos / 128;
}

// FUNCTION: 0x00041a54
void fn_00041a54(Thing *person)
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
void fn_00041b08(Thing *person)
{
    Thing *other;
    Thing *src;

    other = &thing[person->unknown73];
    if (other > thing) {
        if (other->kind == 0x18) {
        person->kind = person->state_b;
        person->state_b = 0;
        process_person_complex(person);
        other->kind = 0x2a;
        src = &thing[person->unknown69];
        if (src > thing) {
            person->unknown30[4] = other->unknown30[4];
            person->unknown30[5] = other->unknown30[5];
            person->unknown30[6] = other->unknown30[6];
            person->unknown30[7] = other->unknown30[7];
            person->unknown30[8] = other->unknown30[8];
        }
        person->unknown30[9] = src->unknown30[9];
        person->unknown30[0xa] = src->unknown30[0xa];
        person->unknown30[0xb] = src->unknown30[0xb];
        }
    }
}

// FUNCTION: 0x00041c0c
void fn_00041c0c(Thing *employee)
{
    if (employee->state_a) {
        u16 anim;

        employee->kind = employee->state_a;
        employee->unknown61 = employee->unknown63 = employee->unknown75w = 0;
        employee->icon = employee->unknown65 = 0;
        anim = check_frame(staff_anim_starts[employee->subtype].unknownA, employee) + (u16)employee->unknown17;
        employee->sprite = start[anim].frame;
        employee->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
        if (employee->subtype == 2) {
            thingpage_cancel_my_response(&allhosps[employee->owner].fd2b, employee);
            thingpage_cancel_my_response(&allhosps[employee->owner].ff6d, employee);
        }
    } else {
        employee->kind = 0x2c;
    }
}

// FUNCTION: 0x00041d54
void fail_person_complex(Thing *person)
{
    if (person->state_a) {
        u16 anim;

        person->kind = person->state_a;
        person->unknown61 = person->unknown63 = person->unknown75w = 0;
        person->icon = person->unknown65 = 0;
        anim = check_frame(anim_starts[person->subtype].unknown12, person) + (u16)person->unknown17;
        person->sprite = start[anim].frame;
        person->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
    } else {
        person->kind = 0x2a;
    }
}

// FUNCTION: 0x00041e44
u8 check_room_mutex_on_entry(Thing *w)
{
    RoomInfo *room;
    u16 *staff;

    room = which_room(w->unknown71);
    if (room <= room_info) {
        return 1;
    }
    staff = room->staff;
    if ((u16)(w - thing) == staff[0] || (u16)(w - thing) == staff[1]) {
        return 1;
    }
    if (room->type == 10) {
        if (staff[0] == 0) {
            staff[0] = w - thing;
        } else if (staff[1] == 0) {
            staff[1] = w - thing;
        } else {
            return 0;
        }
        return 1;
    }
    if (staff[0] == 0) {
        staff[0] = w - thing;
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
    x.tile = w->unknown19;
    y.tile = w->unknown20;
    old_x = w->x;
    old_y = w->y;
    obj = &thing[w->unknown69];
    if (obj > thing) {
        if ((u16)(w - thing) == obj->unknown91) {
            if (w->renderer == 4 && w->subtype == 3) {
                set_complex(w, 0x4ba, 0);
                w->room = 0;
            } else {
                set_complex(w, 0xf0, 0);
            }
            for (i = 0; i < 12; i++) {
                x.tile = w->unknown19;
                y.tile = w->unknown20;
                move_node(w, x, y);
                if (fn_0003e6c0(w) != 1) {
                    break;
                }
            }
        } else {
            if (w->renderer == 4 && w->subtype == 3) {
                set_complex(w, 0x4ba, 0);
                w->room = 0;
            } else {
                set_complex(w, 0xf0, 0);
            }
            room = w->room;
            for (i = 0; i < 12; i++) {
                x.tile = w->unknown19;
                y.tile = w->unknown20;
                move_node(w, x, y);
                if (fn_0003e6c0(w) != 1) {
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
            x.tile = w->unknown19;
            y.tile = w->unknown20;
            move_node(w, x, y);
            if (fn_0003e6c0(w) != 1) {
                break;
            }
        }
        if (room == w->room) {
            move_node(w, old_x, old_y);
        }
    }
    if (w->renderer == 4) {
        patient = &thing[w->unknown73];
        if (patient > thing) {
            interrupt_person(patient);
            if (w->room == 0 || w->room != patient->room) {
                start_person_going(patient);
            }
        }
    }
    w->kind = w->state_a;
    switch (w->renderer) {
    case 2: {
        u16 anim;

        anim = check_frame(anim_starts[w->subtype].unknown12, w) + (u16)w->unknown17;
        w->sprite = start[anim].frame;
        w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
        break;
    }
    case 4: {
        u16 anim;

        anim = check_frame(staff_anim_starts[w->subtype].unknownA, w) + (u16)w->unknown17;
        w->sprite = start[anim].frame;
        w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
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
        reset_icon_value(w, 12);
        w->unknown134 = 100;
        w->unknown81 += gbv.f112;
        w->unknown128 += 10;
        w->unknown136 += 10;
        if ((s16)w->unknown83w > 60) {
            w->unknown83w = 60;
        }
        w->f70 -= 20;
        create_numfx(3, 20, (u16)w->x.tile + ((u16)w->y.tile << 7), color.unknownD, 0x40);
        allhosps[w->owner].fd5 += 20;
        break;
    case 4:
        w->unknown81++;
        w->f70 -= 10;
        create_numfx(3, 10, (u16)w->x.tile + ((u16)w->y.tile << 7), color.unknownD, 0x40);
        allhosps[w->owner].fd9 += 10;
        break;
    case 6:
        reset_icon_value(w, 11);
        w->unknown81 += gbv.f114;
        if (w->style == 0x1d) {
            w->unknown136 = gbv.f116 / 2;
        } else {
            w->unknown136 = 0;
        }
        w->unknown124 = 0;
        w->unknown126 = 0;
        break;
    case 8:
        research_contribution(w, 200);
        break;
    case 10:
        research_contribution(w, 400);
        break;
    case 12:
        person_autopsied(&thing[w->unknown73]);
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
        thing[w->unknown69].mode++;
        break;
    default:
        break;
    }
    if ((s16)w->unknown81 < 0) {
        w->unknown81 = 0;
    } else if ((s16)w->unknown81 > 100) {
        w->unknown81 = 100;
    }
}

// FUNCTION: 0x00042748
u8 staff_go_near_object(Thing *w)
{
    u16 pos;

    pos = can_i_go_near_point(w, (s16)((thing[w->unknown69].y.tile << 7) + (u16)thing[w->unknown69].x.tile));
    if (pos) {
        fn_0004195c(w, pos);
        return 1;
    }
    return 0;
}

// FUNCTION: 0x000427d8
void interrupt_complex(Thing *w)
{
    Thing *obj;

    w->unknown61 = 0;
    w->unknown63 = 0;
    w->unknown75w = 0;
    w->unknown65 = 0;
    w->state_b = 0;
    w->state_a = 0;
    w->x.frac = w->y.frac = 0x80;
    w->height = 0;
    obj = &thing[w->unknown69 && (thing[w->unknown69].renderer == 6 || thing[w->unknown69].renderer == 8)
                 && (u16)(w - thing) == thing[w->unknown69].unknown91 ? w->unknown69 : 0];
    if (obj > thing) {
        if (obj->renderer == 8) {
            if (w->renderer == 4) {
                set_complex(w, 0x4ce, 0);
                fn_0003e724(w);
            } else {
                set_complex(w, 0x26c, 0);
                process_person_complex(w);
            }
        } else if (w->renderer == 4) {
            set_complex(w, 0x1b8, 0);
            fn_0003e724(w);
        } else {
            set_complex(w, 0xa0, 0);
            process_person_complex(w);
            w->unknown18 = gbv.f14c;
            w->anim_speed = gbv.f14e;
        }
    }
}

// FUNCTION: 0x000429b4
void interrupt_peep(Thing *w)
{
    switch (w->renderer) {
    case 4:
        interrupt_staff(w);
        break;
    case 2:
        interrupt_person(w);
        break;
    default:
        break;
    }
}
