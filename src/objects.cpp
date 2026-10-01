#include "hosp.h"

struct NextPartDir {
    u8 dir;
    u16 offset;
    u8 unknown3[3];
};

struct NextPart {
    u8 kind;
    NextPartDir dirs[4];
};

struct PagerSlot {
    u16 thing;
    u8 unknown2[4];
    u16 staff;
    u8 unknown8;
};

struct PagerSlots {
    u16 count;
    PagerSlot slot[1];
};

extern NextPart next_part[];
extern s16 door_to_room[];
extern s16 special_around[];
extern SlidingDoorControl sliding_doors[];
extern ObjectWalls object_walls[];

extern "C" {


}

void mark_object_shape(Map *cell, Shapes *shape, u8 dir, ObjectControl *obj, u8 mark);
void insert_object(u16 tile, Thing *w, u16 kind);
void delete_object(u16 tile, Thing *w);
void inc_hardware_stats(u8 kind, u8 owner);
u32 compute_plant_frame(Thing *w);
void adjust_big_cell_flag(const u16 tile);
void remove_object_shape(Map *cell, Shapes *shape, u8 dir, ObjectControl *obj);
u8 check_room_access(u8 owner);
u8 check_no_peeps_in_way(u16 tile, u8 kind, u8 dir);
void remove_object_from_map(Thing *w);
u8 modify_machine(Thing *w);
u8 should_sliding_door_open(Thing *w);
u8 check_no_peeps_on_cell(RoomInfo *room, u16 tile, s8 side);
void remove_blank_wall(Map *cell, u8 side);
void insert_blank_wall(Map *cell, u8 side);
void cleanse_area(u8 type, u8 x, u8 y, u8 w, u8 h);
void litter_explosion(u32 pos, u8 owner);
u8 try_litter_explosion(u32 x, u32 y, u32 *count, u8 owner);
u8 thingpage_cancel(ThingPager *p, u16 id);
void interrupt_peep(Thing *w);

extern "C" {
void fn_0003751c(Thing *w);
void fn_00015b68(Thing *w);
Thing *fn_00019aac(Map *t, u16 kind);
s32 fn_00038d04(Thing *w, u8 owner);
#pragma aux fn_00038d04 "W?remove_object$n(pn$Thing$$)s"
}

// FUNCTION: 0x00036930
u8 can_place_object(u16 tile, u16 kind, u8 dir, u8 owner)
{
    Map *cell;
    ObjectControl *obj;
    Shapes *shape;
    u8 i;

    if (othervar.f9bb) {
        return 1;
    }
    cell = &map[tile];
    obj = &objects[kind];
    shape = &shapes[obj->shapes[dir]];
    for (i = 0; i < shape->count; i++) {
        if ((cell[(s16)shape->points[i]].unknown3 && (shape->unknown22[i] & 1))
            || (cell[(s16)shape->points[i]].unknown4 && (shape->unknown22[i] & 2))
            || (cell[(s16)shape->points[i]].flags & 7)) {
            return 0;
        }
        if (cell[(s16)shape->points[i]].unknown7 != 1 << owner && kind != 0x11) {
            return 0;
        }
        if (cell->unknown7 & 0x10) {
            return 0;
        }
    }
    for (i = 0; i < 3; i++) {
        if ((cell[shape->unknown13[i]].unknown3 && (shape->unknown2b & 1))
            || (cell[shape->unknown13[i]].unknown4 && (shape->unknown2b & 2))
            || (cell[shape->unknown13[i]].flags & 7)) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: 0x00036b28
void place_existing_object(Thing *obj, u16 tile, u8 dir)
{
    Map *cell;
    ObjectControl *control;
    Shapes *shape;
    RoomInfo *room;
    NextPart *part;
    Thing *other;
    u16 frame;

    cell = &map[tile];
    control = &objects[obj->subtype];
    shape = &shapes[control->shapes[dir]];
    obj->kind = control->unknown23;
    obj->unknown17 = dir;
    obj->sprite = start[control->unknownE + obj->unknown17].frame;
    obj->palette = start[control->unknownE + obj->unknown17].flags;
    mark_object_shape(cell, shape, dir, control, 1);
    obj->use_pos[0] = tile + shape->unknown13[0];
    obj->use_pos[1] = tile + shape->unknown13[1];
    obj->use_pos[2] = tile + shape->unknown13[2];
    obj->use_pos[3] = tile + shape->unknown19[0];
    obj->use_pos[4] = tile + shape->unknown19[1];
    obj->use_pos[5] = tile + shape->unknown19[2];
    obj->depth = shape->unknown1f[2];
    obj->x.tile = tile % 128;
    obj->y.tile = tile / 128;
    obj->text[1] = obj->x.tile + shape->unknown1f[0];
    obj->text[3] = obj->y.tile + shape->unknown1f[1];
    obj->x.frac = obj->text[0] = 0x80;
    obj->y.frac = obj->text[2] = 0x80;
    insert_node(obj, obj->x, obj->y);
    insert_object(tile, obj, obj->subtype);
    inc_hardware_stats(obj->subtype, obj->owner);
    if (control->unknown14 == 6 && (room = which_room(tile)) != 0) {
        room->unknown26++;
    }
    if (control->unknown14 == 7 && listhead[obj->owner].rooms[4] == 0) {
        listhead[obj->owner].rooms[4] = room - room_info;
    }
    if (control->unknown17 != -1) {
        part = &next_part[control->unknown17];
        other = &thing[fn_00037098((u16)(tile + part->dirs[dir].offset), part->kind, part->dirs[dir].dir, obj->owner)];
        if (other > thing) {
            other->group = obj - thing;
            obj->group = other - thing;
        }
    }
    if (obj->subtype == 0x3a || obj->subtype == 0x3b) {
        if (obj->unknown17 == 1) {
            map[(s16)obj->x.tile + ((s16)obj->y.tile << 7) + 0x7f].flags |= 0x20;
        }
    } else {
        adjust_big_cell_flag(tile);
    }
    switch (obj->subtype) {
    case 0x2d:
        obj->sprite = compute_plant_frame(obj);
        if (obj->owner == player) {
            NRAdcounts.unknown0++;
        }
        break;
    case 4:
        if (obj->owner == player) {
            NRAdcounts.unknown4++;
        }
        break;
    case 0x11:
        frame = check_frame(obj->label[0] + 0x9bf, obj) + obj->unknown17;
        obj->sprite = start[frame].frame;
        obj->palette = start[frame].flags > 0;
        obj->owner = (map[tile].unknown7 & 2) ? 1 : (map[tile].unknown7 & 4) ? 2 : (map[tile].unknown7 & 8) ? 3 : 0;
        break;
    }
}

// FUNCTION: 0x00037098
u16 fn_00037098(u16 tile, u16 kind, u8 dir, u8 owner)
{
    BuildState *build;
    u8 i;
    Thing *obj;
    ObjectControl *control;
    s32 cost;

    build = &allhosps[owner].build;
    if (!(kind == 0x30 || can_place_object(tile, kind, dir, owner))) {
        return 0;
    }
    othervar.f9bb = 0;
    if (!which_room(tile)) {
        u8 blocked;

        mark_object_shape(&map[tile], &shapes[objects[kind].shapes[dir]], dir, &objects[kind], 0);
        blocked = !check_room_access(owner);
        remove_object_shape(&map[tile], &shapes[objects[kind].shapes[dir]], dir, &objects[kind]);
        if (blocked) {
            if (othervar.ff == player) {
                advisor.f2 = 7;
            }
            return 0;
        }
    }
    if (!check_no_peeps_in_way(tile, kind, dir)) {
        return 0;
    }
    if (kind == 0x37) {
        allhosps[owner].rooms[0x2c].unknown1C = 1;
    }
    for (i = 0; i < 20; i++) {
        obj = &thing[build->objects[i]];
        if (obj > thing) {
            if (obj->subtype == kind && obj->owner == owner) {
                place_existing_object(obj, tile, dir);
                build->objects[i] = 0;
                return obj - thing;
            }
        }
    }
    control = &objects[kind];
    obj = allocate_thing(0);
    if (obj) {
        cost = allhosps[owner].object_stats[kind].cost;
        if (cost) {
            change_balance(3, -cost, 0, owner);
        }
        obj->renderer = 6;
        obj->subtype = kind;
        obj->anim_speed = 2;
        obj->unknown97 = control->unknown15[0];
        obj->owner = owner;
        obj->value = allhosps[othervar.ff].object_stats[kind].unknown8;
        obj->mode = 0;
        obj->unknown39 = 0;
        obj->unknown40 = old_newrand() & 3;
        obj->unknown41 = old_newrand() & 3;
        if (kind == 0x2d) {
            obj->unknown100 = 400;
            if (obj->owner == player) {
                NRAdcounts.unknownb += 0x37;
                if (NRAdcounts.unknownb < 0) {
                    NRAdcounts.unknownb = 0;
                } else if (NRAdcounts.unknown0 * 0x37 < NRAdcounts.unknownb) {
                    NRAdcounts.unknownb = NRAdcounts.unknown0 * 0x37;
                }
            }
        }
        place_existing_object(obj, tile, dir);
        return obj - thing;
    }
    return 0;
}

// FUNCTION: 0x0003749c
void object_static(Thing *obj)
{
    if (actual_game_turn % obj->anim_speed == 0) {
        obj->sprite = frame[obj->sprite].next;
        if (obj->icon) {
            obj->icon = frame[obj->icon].next;
        }
    }
}

// FUNCTION: 0x0003751c
void fn_0003751c(Thing *obj)
{
    CompFrame *cur_frame;

    if (actual_game_turn % obj->anim_speed == 0) {
        obj->sprite = frame[obj->sprite].next;
        cur_frame = &frame[obj->sprite];
        if (cur_frame->flags & 1) {
            if (obj->unknown100) {
                obj->sprite = obj->unknown100;
                obj->palette = obj->unknown102;
                obj->unknown100 = 0;
                obj->unknown102 = 0;
            } else if (obj->unknown104w) {
                fn_00038d04(obj, obj->owner);
            }
        }
    }
}

// FUNCTION: 0x000375ec
void insert_object(u16 tile, Thing *obj, u16 kind)
{
    u16 id;
    RoomInfo *room;
    u16 first;

    id = obj - thing;
    room = which_room(tile);
    if (room) {
        first = room->first_object;
        if (first) {
            obj->unknown71 = first;
            thing[first].unknown69 = id;
        }
        room->first_object = id;
        obj->unknown89 = room - room_info;
    } else {
        obj->unknown89 = 0;
    }
    first = listhead[obj->owner].objects[kind];
    if (first) {
        obj->unknown75w = first;
        thing[first].unknown73 = id;
    }
    listhead[obj->owner].objects[kind] = id;
}

// FUNCTION: 0x00037718
void delete_object(u16 tile, Thing *obj)
{
    RoomInfo *room;
    Thing *prev;
    Thing *next;

    room = which_room(tile);
    if (room) {
        if (room->first_object) {
            prev = &thing[obj->unknown69];
            if (prev > thing) {
                prev->unknown71 = obj->unknown71;
            } else {
                room->first_object = obj->unknown71;
            }
            next = &thing[obj->unknown71];
            if (next > thing) {
                next->unknown69 = obj->unknown69;
            }
        }
        obj->unknown69 = obj->unknown71 = 0;
    }
    if (listhead[obj->owner].objects[obj->subtype]) {
        prev = &thing[obj->unknown73];
        if (prev > thing) {
            prev->unknown75w = obj->unknown75w;
        } else {
            listhead[obj->owner].objects[obj->subtype] = obj->unknown75w;
        }
        next = &thing[obj->unknown75w];
        if (next > thing) {
            next->unknown73 = obj->unknown73;
        }
        obj->unknown73 = obj->unknown75w = 0;
    }
    if (listhead[obj->owner].objects[0x37] == 0) {
        allhosps[obj->owner].rooms[0x2c].unknown1C = 0;
    }
}

// FUNCTION: 0x0003791c
u16 find_object(u16 tile, u16 kind, u8 random, u8 flag, u8 owner)
{
    u8 count;
    RoomInfo *room;
    Thing *obj;
    u16 found[30];
    u16 x;
    u16 y;

    count = 0;
    room = which_room(tile);
    if (room) {
        obj = &thing[room->first_object];
        if (obj > thing) {
            do {
                if (obj->subtype == kind) {
                    if (flag != 0x96 || obj->unknown98 == 0) {
                        if (random && count < 30) {
                            found[count++] = obj - thing;
                        } else {
                            return obj - thing;
                        }
                    }
                }
            } while ((obj = &thing[obj->unknown71]) > thing);
        }
    } else {
        obj = &thing[listhead[owner].objects[kind]];
        if (obj > thing) {
            x = tile % 128;
            y = tile / 128;
            do {
                if (flag == 0x96 && obj->unknown98 != 0) {
                    continue;
                }
                if (fn_000cb2db(x - obj->x.tile) <= 7) {
                    if (fn_000cb2db(y - obj->y.tile) <= 7) {
                        if (can_i_get_to(obj, tile) != 0) {
                            if (!obj->unknown89) {
                                if (random && count < 30) {
                                    found[count++] = obj - thing;
                                } else {
                                    return obj - thing;
                                }
                            }
                        }
                    }
                }
            } while ((obj = &thing[obj->unknown75w]) > thing);
        }
    }
    if (count) {
        return found[old_newrand() % count];
    }
    return 0;
}

// FUNCTION: 0x00037bec
u16 find_object_anywhere(u16 kind, u8 random, u16 after, u8 owner)
{
    u8 count;
    u8 seen;
    Thing *obj;
    u16 id;
    u16 found[30];

    count = 0;
    seen = 0;
    obj = &thing[listhead[owner].objects[kind]];
    if (obj > thing) {
        do {
            id = obj - thing;
            if (random && count < 30) {
                found[count++] = id;
            } else if (after) {
                if (after == id) {
                    seen = 1;
                } else if (seen) {
                    return id;
                }
            } else {
                return id;
            }
        } while ((obj = &thing[obj->unknown75w]) > thing);
    }
    if (count) {
        return found[old_newrand() % count];
    }
    return 0;
}

// FUNCTION: 0x00037d28
u16 find_nearest_free_object(u16 id, u8 kind)
{
    Thing *seeker;
    Thing *best;
    u32 best_dist;
    u8 count;
    u16 pos;
    RoomInfo *room;
    Thing *obj;
    u16 found[30];
    s32 side;
    u32 dx;
    u32 dy;

    seeker = &thing[id];
    best = thing;
    best_dist = 9999;
    count = 0;
    pos = (u16)seeker->x.tile + ((u16)seeker->y.tile << 7);
    room = which_room(pos);
    if (room) {
        for (obj = &thing[room->first_object]; obj > thing; obj = &thing[obj->unknown71]) {
            if (obj->subtype == kind && obj->unknown98 == 0) {
                if (count < 30) {
                    found[count++] = obj - thing;
                } else {
                    return obj - thing;
                }
            }
        }
        if (count) {
            return found[old_newrand() % count];
        }
        return 0;
    }
    if (seeker->renderer == 4) {
        side = seeker->subtype == 2 ? 2 : 0;
    } else {
        side = 1;
    }
    for (obj = &thing[listhead[seeker->owner].objects[kind]]; obj > thing; obj = &thing[obj->unknown75w]) {
        if (obj->unknown98 == 0) {
            if ((dx = fn_000cb2db(seeker->x.tile - obj->x.tile)) <= 7) {
                if ((dy = fn_000cb2db(seeker->y.tile - obj->y.tile)) <= 7) {
                    pos = obj->use_pos[side];
                    if (can_i_get_to(obj, pos) != 0) {
                        if (!obj->unknown89) {
                            if (dx + dy < best_dist) {
                                best_dist = dx + dy;
                                best = obj;
                            }
                        }
                    }
                }
            }
        }
    }
    return best - thing;
}

// FUNCTION: 0x0003800c
u16 find_free_object_anywhere(u16 id, u8 kind)
{
    Thing *seeker;
    Thing *best;
    u32 best_dist;
    u8 count;
    RoomInfo *room;
    Thing *obj;
    u16 found[30];
    s32 side;
    u16 pos;
    u32 dist;

    seeker = &thing[id];
    best = thing;
    best_dist = 9999;
    count = 0;
    room = which_room((s16)((seeker->y.tile << 7) + (u16)seeker->x.tile));
    if (room) {
        obj = &thing[room->first_object];
        if (obj > thing) {
            for (;;) {
                if (obj->subtype == kind) {
                    if (obj->unknown98 != 0) {
                        continue;
                    }
                    if (count < 30) {
                        found[count++] = obj - thing;
                    } else {
                        return obj - thing;
                    }
                }
                obj = &thing[obj->unknown71];
                if (obj <= thing) {
                    break;
                }
            }
        }
        if (count) {
            return found[old_newrand() % count];
        }
        return 0;
    }
    if (seeker->renderer == 4) {
        side = seeker->subtype == 2 ? 2 : 0;
    } else {
        side = 1;
    }
    for (obj = &thing[listhead[seeker->owner].objects[kind]]; obj > thing; obj = &thing[obj->unknown75w]) {
        if (obj->unknown98 == 0) {
            pos = obj->use_pos[side];
            if (can_i_get_to(seeker, pos) != 0) {
                if (!obj->unknown89) {
                    dist = fn_000cb2db(seeker->x.tile - obj->x.tile) + fn_000cb2db(seeker->y.tile - obj->y.tile);
                    if (dist < best_dist) {
                        best_dist = dist;
                        best = obj;
                    }
                }
            }
        }
    }
    return best - thing;
}

// FUNCTION: 0x000382d8
u16 find_nearest_manned_reception(Thing *person, const u8 room, const u8 flag)
{
    Thing *best;
    s32 best_score;
    Thing *desk;
    Thing *staff;
    s32 free;
    s32 score;

    best = thing;
    best_score = 9999;
    for (desk = &thing[listhead[person->owner].objects[11]]; desk > thing; desk = &thing[desk->unknown75w]) {
        staff = &thing[desk->unknown91];
        if (staff > thing) {
            if (!staff->room || !(room == 0 || (u16)(staff->room - room_info) == room)) {
                continue;
            }
            free = staff->room->f1e - staff->room->unknown31;
            if (free <= 0 && flag) {
                continue;
            }
            score = staff->room->unknown31 * gbv.f100;
            score += (fn_000cb2db(desk->x.tile - person->x.tile) + fn_000cb2db(desk->y.tile - person->y.tile)) * gbv.f102;
            score -= free;
            if (score < best_score) {
                best_score = score;
                best = desk;
            }
        }
    }
    return best - thing;
}

// FUNCTION: 0x000384a4
u8 can_place_door(u16 tile, u16 kind, u8 dir)
{
    Map *cell;
    ObjectControl *obj;
    Shapes *shape;
    u8 i;

    cell = &map[tile];
    obj = &objects[kind];
    shape = &shapes[obj->shapes[dir]];
    for (i = 0; i < shape->count; i++) {
        if (cell[(s16)shape->points[i]].flags & 7) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: 0x00038548
s16 place_door(u16 tile, u16 kind)
{
    RoomInfo *room;
    Map *cell;
    ObjectControl *control;
    Shapes *shape;
    Thing *door;
    u8 i;

    cell = &map[tile];
    control = &objects[kind];
    shape = &shapes[control->shapes[gamevar.f28]];
    if (!can_place_door(tile, kind, gamevar.f28)) {
        return 0;
    }
    door = allocate_thing(0);
    if (door) {
        for (i = 0; i < shape->count; i++) {
            cell[(s16)shape->points[i]].flags |= 2;
        }
        door->renderer = 8;
        door->unknown17 = gamevar.f28;
        door->subtype = kind;
        door->sprite = start[control->unknownE + door->unknown17].frame;
        door->palette = start[control->unknownE + door->unknown17].flags;
        door->anim_speed = 2;
        door->use_pos[0] = tile + shape->unknown13[0];
        door->use_pos[3] = tile + shape->unknown19[0];
        door->x.tile = tile % 128;
        door->y.tile = tile / 128;
        door->x.frac = door->y.frac = 0x80;
        door->depth = shape->unknown1f[2];
        door->kind = 2;
        insert_node(door, door->x, door->y);
        room = which_room(tile + door_to_room[door->unknown17]);
        if (room) {
            door->unknown89 = room - room_info;
            door->owner = room->owner;
        }
        return 1;
    }
    return 0;
}

// FUNCTION: 0x00038788
Thing *find_door(Thing *person)
{
    u8 i;
    Thing *door;
    Map *cell;

    cell = &map[((s16)person->y.tile << 7) + (s16)person->x.tile];
    for (i = 0; i < 5; i++) {
        door = fn_00019aac(&cell[special_around[i]], 8);
        if (door > thing) {
            if (person->room || (s16)person->x.tile + ((s16)person->y.tile << 7) == room_info[door->unknown89].unknown4) {
                return door;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00038870
s16 place_small_thing(u16 tile, u16 id, u8 flag)
{
    Map *cell;
    Thing *last;
    s16 *link;

    cell = &map[tile];
    if (cell->first > 0) {
        last = &thing[cell->first];
        if (last > thing) {
            for (; last->next > 0; last = &thing[last->next]) {
            }
            link = &last->next;
        }
    } else {
        link = &cell->first;
    }
    if (flag == 0 || *link == 0) {
        *link = id;
        *link = -*link;
        if (id >= 0x4b) {
            cell->flags |= blueprint[id].unknown2[3];
        }
    }
    return 0;
}

// FUNCTION: 0x00038950
s16 find_small_thing(u16 tile)
{
    Map *cell;
    s16 id;

    cell = &map[tile];
    for (id = cell->first; id > 0; id = thing[id].next) {
    }
    return -id;
}

// FUNCTION: 0x000389b8
void remove_object_from_map(Thing *obj)
{
    Map *cell;
    ObjectControl *control;
    Shapes *shape;
    Thing *user;
    RoomInfo *room;
    AllHosps *hosp;
    u16 tile;

    cell = &map[(s16)obj->x.tile + ((s16)obj->y.tile << 7)];
    hosp = &allhosps[obj->owner];
    tile = (u16)obj->x.tile + ((u16)obj->y.tile << 7);
    switch (obj->subtype) {
    case 0x2d:
        thingpage_cancel(&allhosps[obj->owner].fd2b, obj - thing);
        NRAdcounts.unknownb -= (obj->unknown100s * 2 + 0x50) / 16;
        if (obj->owner == player) {
            NRAdcounts.unknown0--;
            if (obj->unknown100s < 0) {
                NRAdcounts.unknown2--;
            }
        }
        break;
    case 4:
        if (obj->owner == player) {
            NRAdcounts.unknown4--;
        }
        break;
    default:
        break;
    }
    thingpage_cancel(&allhosps[obj->owner].ff6d, obj - thing);
    control = &objects[obj->subtype];
    shape = &shapes[control->shapes[obj->unknown17]];
    remove_object_shape(cell, shape, obj->unknown17, control);
    switch (control->unknown14) {
    case 1:
    case 2:
        hosp->fc97--;
        hosp->f1bf -= hosp->object_stats[obj->subtype].cost;
        break;
    case 3:
        hosp->fca8--;
        hosp->f1bf -= hosp->object_stats[obj->subtype].cost;
        break;
    case 4:
        hosp->fc9a--;
        break;
    case 5:
        hosp->fc90--;
        break;
    case 6:
        hosp->fc92--;
        room = which_room(tile);
        if (room) {
            room->unknown26--;
        }
        break;
    default:
        break;
    }
    hosp->fc96--;
    hosp->object_stats[obj->subtype].fA++;
    if (obj->kind == 4) {
        user = &thing[obj->unknown91];
        if (user > thing) {
            interrupt_peep(user);
        }
    }
    obj->kind = 0x12;
    delete_object(tile, obj);
    adjust_big_cell_flag(tile);
    fn_00015b68(obj);
}

// FUNCTION: 0x00038d04
s32 fn_00038d04(Thing *obj, u8 owner)
{
    Map *cell;

    cell = &map[(s16)obj->x.tile + ((s16)obj->y.tile << 7)];
    if (!(obj->subtype == 0x30 || obj->subtype == 0x11 || cell->unknown7 == 1 << owner)) {
        return 0;
    }
    remove_object_from_map(obj);
    memset(obj, 0, sizeof(Thing));
    return 1;
}

// FUNCTION: 0x00038db0
u8 treatment_machine_used(const u16 illness, const u16 tile)
{
    ObjectControl *obj;
    u16 id;
    u16 kind;
    Thing *machine;
    s32 percent;

    kind = expertise[illness].kind;
    obj = &objects[kind];
    if (obj->unknown1e) {
        id = find_object(tile, kind, 0, 0x97, 0);
        machine = &thing[id];
        if (machine > thing) {
            if (machine->value * 2 > machine->mode) {
                machine->mode++;
            }
            if (machine->mode > machine->value / 3) {
                percent = machine->mode * 100 / (machine->value + 1);
                thingpage_request(&allhosps[machine->owner].ff6d, (u16)(machine - thing), percent < 100 ? percent : 100);
            }
            if (machine->value * 75 / 100 <= machine->mode) {
                return modify_machine(machine);
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00038f14
u8 modify_machine(Thing *machine)
{
    ObjectControl *obj;
    s32 j;
    s32 i;
    s32 count;

    count = 0;
    obj = &objects[machine->subtype];
    if (machine->value * 75 / 100 <= machine->mode) {
        if (machine->owner == player) {
            if (machine->subtype >= 0 && machine->subtype < 0x3e) {
                play_ambient(machine->subtype + 0x4c);
            }
        }
        for (i = 0; i < 3; i++) {
            if (machine->smoke[i] == 1) {
                count++;
            }
        }
        if (machine->value * 85 / 100 <= machine->mode && count != 0) {
            play_sound(10, 0xd0, -1, -1);
            if (machine->value * 95 / 100 <= machine->mode) {
                machine->smoke[0] = 1;
                machine->smoke[1] = 1;
                machine->smoke[2] = 1;
                machine->icon = start[obj->unknown1e].frame;
            } else if (count == 1) {
                for (j = 0; j < 3; j++) {
                    if (machine->smoke[j] == 1) {
                        break;
                    }
                }
                j = (j + 1 + (old_newrand() & 1)) % 3;
                machine->smoke[j] = 1;
                machine->icon = start[obj->unknown1e].frame;
            }
        } else {
            j = old_newrand() % 3;
            machine->smoke[j] = 1;
            machine->icon = start[obj->unknown1e].frame;
        }
    }
    if (machine->mode >= machine->value) {
        i = machine->value * 3 / 2 - machine->mode + 1;
        if (i <= 1 || old_newrand() % i == 0) {
            machine_disaster(machine);
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0003917c
void mark_object_shape(Map *base_cell, Shapes *shape, u8 dir, ObjectControl *obj, u8 cleanse)
{
    u8 i;
    Map *cell;
    ObjectWalls *walls;

    for (i = 0; i < shape->count; i++) {
        if (shape->points[9] == shape->points[i] || shape->points[10] == shape->points[i]
            || shape->points[11] == shape->points[i] || shape->points[12] == shape->points[i]
            || shape->points[13] == shape->points[i] || shape->points[14] == shape->points[i]) {
            base_cell[(s16)shape->points[i]].flags |= 2;
        } else {
            base_cell[(s16)shape->points[i]].flags |= 1;
        }
        if (cleanse) {
            cell = &base_cell[(s16)shape->points[i]];
            cleanse_area((cell->unknown7 & 2) ? 1 : (cell->unknown7 & 4) ? 2 : (cell->unknown7 & 8) ? 3 : 0,
                         (u16)(cell - map) % 128, (u16)(cell - map) / 128, 1, 1);
        }
    }
    base_cell[shape->unknown13[0]].flags |= 4;
    base_cell[shape->unknown13[1]].flags |= 4;
    base_cell[shape->unknown13[2]].flags |= 4;
    base_cell[shape->unknown19[0]].flags |= 4;
    base_cell[shape->unknown19[1]].flags |= 4;
    base_cell[shape->unknown19[2]].flags |= 4;
    if (obj->walls != -1) {
        walls = &object_walls[obj->walls * 4 + dir];
        for (i = 0; i < walls->count; i++) {
            remove_blank_wall(&base_cell[walls->pos[i]], walls->side[i]);
        }
    }
}

// FUNCTION: 0x00039470
void remove_object_shape(Map *cell, Shapes *shape, u8 dir, ObjectControl *obj)
{
    u8 i;
    ObjectWalls *walls;

    for (i = 0; i < shape->count; i++) {
        cell[(s16)shape->points[i]].flags &= ~7;
    }
    cell[shape->unknown13[0]].flags &= ~4;
    cell[shape->unknown13[1]].flags &= ~4;
    cell[shape->unknown13[2]].flags &= ~4;
    cell[shape->unknown19[0]].flags &= ~4;
    cell[shape->unknown19[1]].flags &= ~4;
    cell[shape->unknown19[2]].flags &= ~4;
    if (obj->walls != -1) {
        walls = &object_walls[obj->walls * 4 + dir];
        for (i = 0; i < walls->count; i++) {
            insert_blank_wall(&cell[walls->pos[i]], walls->side[i]);
        }
    }
}

// FUNCTION: 0x000395b0
void double_door_swings(Thing *door)
{
    if (actual_game_turn % door->anim_speed == 0) {
        door->sprite = frame[door->sprite].next;
        if (frame[door->sprite].flags & 1) {
            door->unknown98 = 0;
            door->kind = 2;
            if (door->subtype == 0x35) {
                u16 anim = check_frame(0x3eb, door) + door->unknown17;
                door->sprite = start[anim].frame;
                door->palette = start[anim].flags > 0;
            } else {
                u16 anim2 = check_frame(0x3e6, door) + door->unknown17;
                door->sprite = start[anim2].frame;
                door->palette = start[anim2].flags > 0;
            }
        }
    }
}

// FUNCTION: 0x00039704
void sliding_door_closed(Thing *door)
{
    if (should_sliding_door_open(door)) {
        door->kind = 0x16;
    }
}

// FUNCTION: 0x00039734
void sliding_door_opens(Thing *door)
{
    u16 next;

    if (actual_game_turn % door->anim_speed == 0) {
        next = frame[door->sprite].next;
        if (frame[next].flags & 1) {
            door->height = 0x32;
            door->kind = 0x18;
        } else {
            door->sprite = next;
        }
    }
}

// FUNCTION: 0x000397b4
void sliding_door_open(Thing *door)
{
    if (should_sliding_door_open(door)) {
        door->height = 0x32;
    } else {
        door->height--;
        if (door->height == 0) {
            u16 anim = check_frame(sliding_doors[door->subtype - 0x3a].close_anim, door) + door->unknown17;
            door->sprite = start[anim].frame;
            door->palette = start[anim].flags > 0;
            door->kind = 0x1a;
        }
    }
}

// FUNCTION: 0x00039880
void sliding_door_closes(Thing *door)
{
    u8 steps;

    if (should_sliding_door_open(door)) {
        steps = 0;
        while (!(frame[door->sprite].flags & 1)) {
            door->sprite = frame[door->sprite].next;
            steps++;
        }
        u16 anim = check_frame(sliding_doors[door->subtype - 0x3a].open_anim, door) + door->unknown17;
        door->sprite = start[anim].frame;
        door->palette = start[anim].flags > 0;
        for (steps--; steps != 0; steps--) {
            door->sprite = frame[door->sprite].next;
        }
        door->kind = 0x16;
    } else if (actual_game_turn % door->anim_speed == 0) {
        u16 next = frame[door->sprite].next;
        if (frame[next].flags & 1) {
            u16 anim2 = check_frame(sliding_doors[door->subtype - 0x3a].open_anim, door) + door->unknown17;
            door->sprite = start[anim2].frame;
            door->palette = start[anim2].flags > 0;
            door->kind = 0x14;
        } else {
            door->sprite = next;
        }
    }
}

// FUNCTION: 0x00039a90
u8 should_sliding_door_open(Thing *door)
{
    s16 offset;
    Thing *occupant;
    Map *cell;

    offset = sliding_doors[door->subtype - 0x3a].offset[door->unknown17];
    cell = &map[((s16)door->y.tile << 7) + (s16)door->x.tile + shapes[objects[door->subtype].shapes[door->unknown17]].unknown13[1] + offset];
    for (occupant = &thing[cell->first]; occupant > thing; occupant = &thing[occupant->next]) {
        if (occupant->renderer == 2 || occupant->renderer == 4 || occupant->renderer == 0x14 || occupant->renderer == 0x16) {
            return 1;
        }
    }
    cell = &map[shapes[objects[door->subtype].shapes[door->unknown17]].unknown19[1] + (((s16)door->y.tile << 7) + (s16)door->x.tile) + offset];
    for (occupant = &thing[cell->first]; occupant > thing; occupant = &thing[occupant->next]) {
        if (occupant->renderer == 2 || occupant->renderer == 4 || occupant->renderer == 0x14 || occupant->renderer == 0x16) {
            return 1;
        }
    }
    return 0;
}
// FUNCTION: 0x00039cb4
u8 check_no_peeps_in_way(u16 tile, u8 kind, u8 dir)
{
    s8 i;
    RoomInfo *room;
    Shapes *shape;

    room = which_room(tile);
    shape = &shapes[objects[kind].shapes[dir]];
    for (i = shape->count - 1; i >= 0; i--) {
        if (!(check_no_peeps_on_cell(room, tile + shape->points[i], -1)
              && check_no_peeps_on_cell(room, tile + shape->points[i] - 0x80, 2)
              && check_no_peeps_on_cell(room, tile + shape->points[i] + 1, 3)
              && check_no_peeps_on_cell(room, tile + shape->points[i] + 0x80, 0)
              && check_no_peeps_on_cell(room, tile + shape->points[i] - 1, 1))) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: 0x00039e10
u8 check_no_peeps_on_cell(RoomInfo *room, u16 tile, s8 side)
{
    Thing *occupant;

    if (which_room(tile) != room) {
        return 1;
    }
    for (occupant = &thing[map[tile].first]; occupant > thing; occupant = &thing[occupant->next]) {
        if (occupant->renderer == 2 || occupant->renderer == 4 || occupant->renderer == 0x14 || occupant->renderer == 0x16
            || occupant->renderer == 0xa) {
            if (side == -1) {
                return 0;
            }
            if (occupant->unknown17 == side) {
                if (occupant->renderer == 2 && occupant->kind == 0x18 && occupant->icon != 0x46) {
                    return 1;
                }
                if (occupant->renderer == 2 && occupant->kind == 0x44) {
                    return 1;
                }
                return 0;
            }
        }
    }
    return 1;
}

// FUNCTION: 0x00039f8c
void inc_hardware_stats(u8 kind, u8 owner)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    switch (objects[kind].unknown14) {
    case 1:
    case 2:
        hosp->fc97++;
        hosp->f1bf += hosp->object_stats[kind].cost;
        break;
    case 3:
        hosp->fca8++;
        hosp->f1bf += hosp->object_stats[kind].cost;
        break;
    case 4:
        hosp->fc9a++;
        break;
    case 5:
        hosp->fc90++;
        break;
    case 6:
        hosp->fc92++;
        break;
    default:
        break;
    }
    hosp->fc96++;
    hosp->object_stats[kind].fA++;
}

// FUNCTION: 0x0003a098
void bomb_ticks(Thing *bomb)
{
    if (othervar.f32) {
        bomb->fuse_tick++;
        if (bomb->fuse_tick >= gbv.f152) {
            bomb->fuse_tick = 0;
            bomb->fuse++;
            if (bomb->fuse >= 8) {
                create_explosion(((u16)bomb->y.tile << 7) + (u16)bomb->x.tile, 0x97);
                litter_explosion(((s16)bomb->y.tile << 7) + (s16)bomb->x.tile, bomb->owner);
                fn_00038d04(bomb, bomb->owner);
                return;
            }
            u16 anim = check_frame(bomb->fuse + 0x9bf, bomb) + bomb->unknown17;
            bomb->sprite = start[anim].frame;
            bomb->palette = start[anim].flags > 0;
        }
    }
    if (actual_game_turn % bomb->anim_speed == 0) {
        bomb->sprite = frame[bomb->sprite].next;
    }
}

// FUNCTION: 0x0003a200
void litter_explosion(u32 pos, u8 owner)
{
    u32 count;
    u32 radius;
    u32 x0;
    u32 x1;
    u32 y0;
    u32 y1;
    u32 x;
    u32 y;

    count = gbv.f153;
    for (radius = 0; radius <= gbv.f151; radius++) {
        x0 = (pos & 0x7f) - radius;
        x1 = x0 + radius * 2;
        y0 = (pos >> 7) - radius;
        y1 = y0 + radius * 2;
        for (x = x0; x <= x1; x++) {
            if (try_litter_explosion(x, y0, &count, owner)) {
                return;
            }
            if (try_litter_explosion(x, y1, &count, owner)) {
                return;
            }
        }
        for (y = y0 + 1; y <= y1 - 1; y++) {
            if (try_litter_explosion(x0, y, &count, owner)) {
                return;
            }
            if (try_litter_explosion(x1, y, &count, owner)) {
                return;
            }
        }
    }
}

// FUNCTION: 0x0003a310
u8 try_litter_explosion(u32 x, u32 y, u32 *count, u8 owner)
{
    s32 pos;
    Map *cell;

    pos = (s16)x + ((s16)y << 7);
    cell = &map[pos];
    if (old_newrand() % 100 > gbv.f150) {
        return 0;
    }
    if (which_room(pos) == 0 && !(cell->flags & 7) && !(cell->floor & 0xf)) {
        place_litter(pos, (s8)owner, old_newrand() % 3 + 1);
        (*count)--;
        return *count == 0;
    }
    return 0;
}

// FUNCTION: 0x0003a3e0
void replace_machine(Thing *machine)
{
    ThingPager *pager;
    s32 cost;
    s32 i;

    cost = allhosps[othervar.ff].object_stats[machine->subtype].cost;
    if (allhosps[player].f35 >= cost && allhosps[player].f35 > 0) {
        change_balance(0x24, -cost, 0, machine->owner);
        machine->value = allhosps[othervar.ff].object_stats[machine->subtype].unknown8;
        machine->mode = 0;
        machine->icon = 0;
        machine->smoke[0] = 0;
        machine->smoke[1] = 0;
        machine->smoke[2] = 0;
        pager = &allhosps[machine->owner].ff6d;
        for (i = ((PagerSlots *)pager)->count - 1; i >= 0; i--) {
            if (&thing[((PagerSlots *)pager)->slot[i].thing] == machine && ((PagerSlots *)pager)->slot[i].staff) {
                interrupt_staff(&thing[((PagerSlots *)pager)->slot[i].staff]);
                break;
            }
        }
        thingpage_cancel(pager, machine - thing);
        reset_icon_value(machine, 0x1c);
        play_sound(10, 0x43, -1, -1);
    } else {
        play_sound(10, 0x42, -1, -1);
    }
}

// FUNCTION: 0x0003a5e0
void check_invis_walls_around_object(Thing *obj)
{
    ObjectControl *control;
    ObjectWalls *walls;
    Map *cell;
    u32 side;
    s32 i;

    control = &objects[obj->subtype];
    if (control->walls != -1) {
        walls = &object_walls[control->walls * 4 + obj->unknown17];
        for (i = 0; i < walls->count; i++) {
            cell = &map[((s16)obj->y.tile << 7) + (s16)obj->x.tile + walls->pos[i]];
            side = walls->side[i];
            if ((&cell->unknown3)[side] == 0) {
                remove_blank_wall(cell, side);
            }
        }
    }
}
