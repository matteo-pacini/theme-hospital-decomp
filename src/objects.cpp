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

struct SlidingDoorControl {
    u16 open_anim;
    u16 close_anim;
    s16 offset[4];
};

struct ObjectWalls {
    u8 count;
    u16 pos[4];
    u8 side[4];
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
void place_existing_object(Thing *w, u16 tile, u8 dir)
{
    Map *cell;
    ObjectControl *obj;
    Shapes *shape;
    RoomInfo *room;
    NextPart *part;
    Thing *other;
    u16 frame;

    cell = &map[tile];
    obj = &objects[w->subtype];
    shape = &shapes[obj->shapes[dir]];
    w->kind = obj->unknown23;
    w->unknown17 = dir;
    w->sprite = start[obj->unknownE + w->unknown17].frame;
    w->palette = start[obj->unknownE + w->unknown17].flags;
    mark_object_shape(cell, shape, dir, obj, 1);
    w->use_pos[0] = tile + shape->unknown13[0];
    w->use_pos[1] = tile + shape->unknown13[1];
    w->use_pos[2] = tile + shape->unknown13[2];
    w->use_pos[3] = tile + shape->unknown19[0];
    w->use_pos[4] = tile + shape->unknown19[1];
    w->use_pos[5] = tile + shape->unknown19[2];
    w->depth = shape->unknown1f[2];
    w->x.tile = tile % 128;
    w->y.tile = tile / 128;
    w->text[1] = w->x.tile + shape->unknown1f[0];
    w->text[3] = w->y.tile + shape->unknown1f[1];
    w->x.frac = w->text[0] = 0x80;
    w->y.frac = w->text[2] = 0x80;
    insert_node(w, w->x, w->y);
    insert_object(tile, w, w->subtype);
    inc_hardware_stats(w->subtype, w->owner);
    if (obj->unknown14 == 6 && (room = which_room(tile)) != 0) {
        room->unknown26++;
    }
    if (obj->unknown14 == 7 && listhead[w->owner].rooms[4] == 0) {
        listhead[w->owner].rooms[4] = room - room_info;
    }
    if (obj->unknown17 != -1) {
        part = &next_part[obj->unknown17];
        other = &thing[fn_00037098((u16)(tile + part->dirs[dir].offset), part->kind, part->dirs[dir].dir, w->owner)];
        if (other > thing) {
            other->group = w - thing;
            w->group = other - thing;
        }
    }
    if (w->subtype == 0x3a || w->subtype == 0x3b) {
        if (w->unknown17 == 1) {
            map[(s16)w->x.tile + ((s16)w->y.tile << 7) + 0x7f].flags |= 0x20;
        }
    } else {
        adjust_big_cell_flag(tile);
    }
    switch (w->subtype) {
    case 0x2d:
        w->sprite = compute_plant_frame(w);
        if (w->owner == player) {
            NRAdcounts.unknown0++;
        }
        break;
    case 4:
        if (w->owner == player) {
            NRAdcounts.unknown4++;
        }
        break;
    case 0x11:
        frame = check_frame(w->label[0] + 0x9bf, w) + w->unknown17;
        w->sprite = start[frame].frame;
        w->palette = start[frame].flags > 0;
        w->owner = (map[tile].unknown7 & 2) ? 1 : (map[tile].unknown7 & 4) ? 2 : (map[tile].unknown7 & 8) ? 3 : 0;
        break;
    }
}

// FUNCTION: 0x00037098
u16 fn_00037098(u16 tile, u16 kind, u8 dir, u8 owner)
{
    BuildState *build;
    u8 i;
    Thing *w;
    ObjectControl *obj;
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
        w = &thing[build->objects[i]];
        if (w > thing) {
            if (w->subtype == kind && w->owner == owner) {
                place_existing_object(w, tile, dir);
                build->objects[i] = 0;
                return w - thing;
            }
        }
    }
    obj = &objects[kind];
    w = allocate_thing(0);
    if (w) {
        cost = allhosps[owner].object_stats[kind].cost;
        if (cost) {
            change_balance(3, -cost, 0, owner);
        }
        w->renderer = 6;
        w->subtype = kind;
        w->anim_speed = 2;
        w->unknown97 = obj->unknown15[0];
        w->owner = owner;
        w->value = allhosps[othervar.ff].object_stats[kind].unknown8;
        w->mode = 0;
        w->unknown39 = 0;
        w->unknown40 = old_newrand() & 3;
        w->unknown41 = old_newrand() & 3;
        if (kind == 0x2d) {
            w->unknown100 = 400;
            if (w->owner == player) {
                NRAdcounts.unknownb += 0x37;
                if (NRAdcounts.unknownb < 0) {
                    NRAdcounts.unknownb = 0;
                } else if (NRAdcounts.unknown0 * 0x37 < NRAdcounts.unknownb) {
                    NRAdcounts.unknownb = NRAdcounts.unknown0 * 0x37;
                }
            }
        }
        place_existing_object(w, tile, dir);
        return w - thing;
    }
    return 0;
}

// FUNCTION: 0x0003749c
void object_static(Thing *w)
{
    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
        if (w->icon) {
            w->icon = frame[w->icon].next;
        }
    }
}

// FUNCTION: 0x0003751c
void fn_0003751c(Thing *w)
{
    CompFrame *f;

    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
        f = &frame[w->sprite];
        if (f->flags & 1) {
            if (w->unknown100) {
                w->sprite = w->unknown100;
                w->palette = w->unknown102;
                w->unknown100 = 0;
                w->unknown102 = 0;
            } else if (w->unknown104w) {
                fn_00038d04(w, w->owner);
            }
        }
    }
}

// FUNCTION: 0x000375ec
void insert_object(u16 tile, Thing *w, u16 kind)
{
    u16 id;
    RoomInfo *room;
    u16 first;

    id = w - thing;
    room = which_room(tile);
    if (room) {
        first = room->first_object;
        if (first) {
            w->unknown71 = first;
            thing[first].unknown69 = id;
        }
        room->first_object = id;
        w->unknown89 = room - room_info;
    } else {
        w->unknown89 = 0;
    }
    first = listhead[w->owner].objects[kind];
    if (first) {
        w->unknown75w = first;
        thing[first].unknown73 = id;
    }
    listhead[w->owner].objects[kind] = id;
}

// FUNCTION: 0x00037718
void delete_object(u16 tile, Thing *w)
{
    RoomInfo *room;
    Thing *prev;
    Thing *next;

    room = which_room(tile);
    if (room) {
        if (room->first_object) {
            prev = &thing[w->unknown69];
            if (prev > thing) {
                prev->unknown71 = w->unknown71;
            } else {
                room->first_object = w->unknown71;
            }
            next = &thing[w->unknown71];
            if (next > thing) {
                next->unknown69 = w->unknown69;
            }
        }
        w->unknown69 = w->unknown71 = 0;
    }
    if (listhead[w->owner].objects[w->subtype]) {
        prev = &thing[w->unknown73];
        if (prev > thing) {
            prev->unknown75w = w->unknown75w;
        } else {
            listhead[w->owner].objects[w->subtype] = w->unknown75w;
        }
        next = &thing[w->unknown75w];
        if (next > thing) {
            next->unknown73 = w->unknown73;
        }
        w->unknown73 = w->unknown75w = 0;
    }
    if (listhead[w->owner].objects[0x37] == 0) {
        allhosps[w->owner].rooms[0x2c].unknown1C = 0;
    }
}

// FUNCTION: 0x0003791c
u16 find_object(u16 tile, u16 kind, u8 random, u8 flag, u8 owner)
{
    u8 count;
    RoomInfo *room;
    Thing *w;
    u16 found[30];
    u16 x;
    u16 y;

    count = 0;
    room = which_room(tile);
    if (room) {
        w = &thing[room->first_object];
        if (w > thing) {
            do {
                if (w->subtype == kind) {
                    if (flag != 0x96 || w->unknown98 == 0) {
                        if (random && count < 30) {
                            found[count++] = w - thing;
                        } else {
                            return w - thing;
                        }
                    }
                }
            } while ((w = &thing[w->unknown71]) > thing);
        }
    } else {
        w = &thing[listhead[owner].objects[kind]];
        if (w > thing) {
            x = tile % 128;
            y = tile / 128;
            do {
                if (flag == 0x96 && w->unknown98 != 0) {
                    continue;
                }
                if (fn_000cb2db(x - w->x.tile) <= 7) {
                    if (fn_000cb2db(y - w->y.tile) <= 7) {
                        if (can_i_get_to(w, tile) != 0) {
                            if (!w->unknown89) {
                                if (random && count < 30) {
                                    found[count++] = w - thing;
                                } else {
                                    return w - thing;
                                }
                            }
                        }
                    }
                }
            } while ((w = &thing[w->unknown75w]) > thing);
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
    Thing *w;
    u16 id;
    u16 found[30];

    count = 0;
    seen = 0;
    w = &thing[listhead[owner].objects[kind]];
    if (w > thing) {
        do {
            id = w - thing;
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
        } while ((w = &thing[w->unknown75w]) > thing);
    }
    if (count) {
        return found[old_newrand() % count];
    }
    return 0;
}

// FUNCTION: 0x00037d28
u16 find_nearest_free_object(u16 id, u8 kind)
{
    Thing *e;
    Thing *best;
    u32 best_dist;
    u8 count;
    u16 pos;
    RoomInfo *room;
    Thing *w;
    u16 found[30];
    s32 side;
    u32 dx;
    u32 dy;

    e = &thing[id];
    best = thing;
    best_dist = 9999;
    count = 0;
    pos = (u16)e->x.tile + ((u16)e->y.tile << 7);
    room = which_room(pos);
    if (room) {
        for (w = &thing[room->first_object]; w > thing; w = &thing[w->unknown71]) {
            if (w->subtype == kind && w->unknown98 == 0) {
                if (count < 30) {
                    found[count++] = w - thing;
                } else {
                    return w - thing;
                }
            }
        }
        if (count) {
            return found[old_newrand() % count];
        }
        return 0;
    }
    if (e->renderer == 4) {
        side = e->subtype == 2 ? 2 : 0;
    } else {
        side = 1;
    }
    for (w = &thing[listhead[e->owner].objects[kind]]; w > thing; w = &thing[w->unknown75w]) {
        if (w->unknown98 == 0) {
            if ((dx = fn_000cb2db(e->x.tile - w->x.tile)) <= 7) {
                if ((dy = fn_000cb2db(e->y.tile - w->y.tile)) <= 7) {
                    pos = w->use_pos[side];
                    if (can_i_get_to(w, pos) != 0) {
                        if (!w->unknown89) {
                            if (dx + dy < best_dist) {
                                best_dist = dx + dy;
                                best = w;
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
    Thing *e;
    Thing *best;
    u32 best_dist;
    u8 count;
    RoomInfo *room;
    Thing *w;
    u16 found[30];
    s32 side;
    u16 pos;
    u32 dist;

    e = &thing[id];
    best = thing;
    best_dist = 9999;
    count = 0;
    room = which_room((s16)((e->y.tile << 7) + (u16)e->x.tile));
    if (room) {
        w = &thing[room->first_object];
        if (w > thing) {
            for (;;) {
                if (w->subtype == kind) {
                    if (w->unknown98 != 0) {
                        continue;
                    }
                    if (count < 30) {
                        found[count++] = w - thing;
                    } else {
                        return w - thing;
                    }
                }
                w = &thing[w->unknown71];
                if (w <= thing) {
                    break;
                }
            }
        }
        if (count) {
            return found[old_newrand() % count];
        }
        return 0;
    }
    if (e->renderer == 4) {
        side = e->subtype == 2 ? 2 : 0;
    } else {
        side = 1;
    }
    for (w = &thing[listhead[e->owner].objects[kind]]; w > thing; w = &thing[w->unknown75w]) {
        if (w->unknown98 == 0) {
            pos = w->use_pos[side];
            if (can_i_get_to(e, pos) != 0) {
                if (!w->unknown89) {
                    dist = fn_000cb2db(e->x.tile - w->x.tile) + fn_000cb2db(e->y.tile - w->y.tile);
                    if (dist < best_dist) {
                        best_dist = dist;
                        best = w;
                    }
                }
            }
        }
    }
    return best - thing;
}

// FUNCTION: 0x000382d8
u16 find_nearest_manned_reception(Thing *w, const u8 room, const u8 flag)
{
    Thing *best;
    s32 best_score;
    Thing *desk;
    Thing *staff;
    s32 free;
    s32 score;

    best = thing;
    best_score = 9999;
    for (desk = &thing[listhead[w->owner].objects[11]]; desk > thing; desk = &thing[desk->unknown75w]) {
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
            score += (fn_000cb2db(desk->x.tile - w->x.tile) + fn_000cb2db(desk->y.tile - w->y.tile)) * gbv.f102;
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
    ObjectControl *obj;
    Shapes *shape;
    Thing *w;
    u8 i;

    cell = &map[tile];
    obj = &objects[kind];
    shape = &shapes[obj->shapes[gamevar.f28]];
    if (!can_place_door(tile, kind, gamevar.f28)) {
        return 0;
    }
    w = allocate_thing(0);
    if (w) {
        for (i = 0; i < shape->count; i++) {
            cell[(s16)shape->points[i]].flags |= 2;
        }
        w->renderer = 8;
        w->unknown17 = gamevar.f28;
        w->subtype = kind;
        w->sprite = start[obj->unknownE + w->unknown17].frame;
        w->palette = start[obj->unknownE + w->unknown17].flags;
        w->anim_speed = 2;
        w->use_pos[0] = tile + shape->unknown13[0];
        w->use_pos[3] = tile + shape->unknown19[0];
        w->x.tile = tile % 128;
        w->y.tile = tile / 128;
        w->x.frac = w->y.frac = 0x80;
        w->depth = shape->unknown1f[2];
        w->kind = 2;
        insert_node(w, w->x, w->y);
        room = which_room(tile + door_to_room[w->unknown17]);
        if (room) {
            w->unknown89 = room - room_info;
            w->owner = room->owner;
        }
        return 1;
    }
    return 0;
}

// FUNCTION: 0x00038788
Thing *find_door(Thing *w)
{
    u8 i;
    Thing *door;
    Map *cell;

    cell = &map[((s16)w->y.tile << 7) + (s16)w->x.tile];
    for (i = 0; i < 5; i++) {
        door = fn_00019aac(&cell[special_around[i]], 8);
        if (door > thing) {
            if (w->room || (s16)w->x.tile + ((s16)w->y.tile << 7) == room_info[door->unknown89].unknown4) {
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
    Thing *t;
    s16 *link;

    cell = &map[tile];
    if (cell->first > 0) {
        t = &thing[cell->first];
        if (t > thing) {
            for (; t->next > 0; t = &thing[t->next]) {
            }
            link = &t->next;
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
void remove_object_from_map(Thing *w)
{
    Map *cell;
    ObjectControl *obj;
    Shapes *shape;
    Thing *t;
    RoomInfo *room;
    AllHosps *h;
    u16 tile;

    cell = &map[(s16)w->x.tile + ((s16)w->y.tile << 7)];
    h = &allhosps[w->owner];
    tile = (u16)w->x.tile + ((u16)w->y.tile << 7);
    switch (w->subtype) {
    case 0x2d:
        thingpage_cancel(&allhosps[w->owner].fd2b, w - thing);
        NRAdcounts.unknownb -= (w->unknown100s * 2 + 0x50) / 16;
        if (w->owner == player) {
            NRAdcounts.unknown0--;
            if (w->unknown100s < 0) {
                NRAdcounts.unknown2--;
            }
        }
        break;
    case 4:
        if (w->owner == player) {
            NRAdcounts.unknown4--;
        }
        break;
    default:
        break;
    }
    thingpage_cancel(&allhosps[w->owner].ff6d, w - thing);
    obj = &objects[w->subtype];
    shape = &shapes[obj->shapes[w->unknown17]];
    remove_object_shape(cell, shape, w->unknown17, obj);
    switch (obj->unknown14) {
    case 1:
    case 2:
        h->fc97--;
        h->f1bf -= h->object_stats[w->subtype].cost;
        break;
    case 3:
        h->fca8--;
        h->f1bf -= h->object_stats[w->subtype].cost;
        break;
    case 4:
        h->fc9a--;
        break;
    case 5:
        h->fc90--;
        break;
    case 6:
        h->fc92--;
        room = which_room(tile);
        if (room) {
            room->unknown26--;
        }
        break;
    default:
        break;
    }
    h->fc96--;
    h->object_stats[w->subtype].fA++;
    if (w->kind == 4) {
        t = &thing[w->unknown91];
        if (t > thing) {
            interrupt_peep(t);
        }
    }
    w->kind = 0x12;
    delete_object(tile, w);
    adjust_big_cell_flag(tile);
    fn_00015b68(w);
}

// FUNCTION: 0x00038d04
s32 fn_00038d04(Thing *w, u8 owner)
{
    Map *cell;

    cell = &map[(s16)w->x.tile + ((s16)w->y.tile << 7)];
    if (!(w->subtype == 0x30 || w->subtype == 0x11 || cell->unknown7 == 1 << owner)) {
        return 0;
    }
    remove_object_from_map(w);
    memset(w, 0, sizeof(Thing));
    return 1;
}

// FUNCTION: 0x00038db0
u8 treatment_machine_used(const u16 illness, const u16 tile)
{
    ObjectControl *obj;
    u16 id;
    u16 kind;
    Thing *t;
    s32 percent;

    kind = expertise[illness].kind;
    obj = &objects[kind];
    if (obj->unknown1e) {
        id = find_object(tile, kind, 0, 0x97, 0);
        t = &thing[id];
        if (t > thing) {
            if (t->value * 2 > t->mode) {
                t->mode++;
            }
            if (t->mode > t->value / 3) {
                percent = t->mode * 100 / (t->value + 1);
                thingpage_request(&allhosps[t->owner].ff6d, (u16)(t - thing), percent < 100 ? percent : 100);
            }
            if (t->value * 75 / 100 <= t->mode) {
                return modify_machine(t);
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00038f14
u8 modify_machine(Thing *w)
{
    ObjectControl *obj;
    s32 j;
    s32 i;
    s32 count;

    count = 0;
    obj = &objects[w->subtype];
    if (w->value * 75 / 100 <= w->mode) {
        if (w->owner == player) {
            if (w->subtype >= 0 && w->subtype < 0x3e) {
                play_ambient(w->subtype + 0x4c);
            }
        }
        for (i = 0; i < 3; i++) {
            if (w->smoke[i] == 1) {
                count++;
            }
        }
        if (w->value * 85 / 100 <= w->mode && count != 0) {
            play_sound(10, 0xd0, -1, -1);
            if (w->value * 95 / 100 <= w->mode) {
                w->smoke[0] = 1;
                w->smoke[1] = 1;
                w->smoke[2] = 1;
                w->icon = start[obj->unknown1e].frame;
            } else if (count == 1) {
                for (j = 0; j < 3; j++) {
                    if (w->smoke[j] == 1) {
                        break;
                    }
                }
                j = (j + 1 + (old_newrand() & 1)) % 3;
                w->smoke[j] = 1;
                w->icon = start[obj->unknown1e].frame;
            }
        } else {
            j = old_newrand() % 3;
            w->smoke[j] = 1;
            w->icon = start[obj->unknown1e].frame;
        }
    }
    if (w->mode >= w->value) {
        i = w->value * 3 / 2 - w->mode + 1;
        if (i <= 1 || old_newrand() % i == 0) {
            machine_disaster(w);
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0003917c
void mark_object_shape(Map *m, Shapes *shape, u8 dir, ObjectControl *obj, u8 cleanse)
{
    u8 i;
    Map *cell;
    ObjectWalls *walls;

    for (i = 0; i < shape->count; i++) {
        if (shape->points[9] == shape->points[i] || shape->points[10] == shape->points[i]
            || shape->points[11] == shape->points[i] || shape->points[12] == shape->points[i]
            || shape->points[13] == shape->points[i] || shape->points[14] == shape->points[i]) {
            m[(s16)shape->points[i]].flags |= 2;
        } else {
            m[(s16)shape->points[i]].flags |= 1;
        }
        if (cleanse) {
            cell = &m[(s16)shape->points[i]];
            cleanse_area((cell->unknown7 & 2) ? 1 : (cell->unknown7 & 4) ? 2 : (cell->unknown7 & 8) ? 3 : 0,
                         (u16)(cell - map) % 128, (u16)(cell - map) / 128, 1, 1);
        }
    }
    m[shape->unknown13[0]].flags |= 4;
    m[shape->unknown13[1]].flags |= 4;
    m[shape->unknown13[2]].flags |= 4;
    m[shape->unknown19[0]].flags |= 4;
    m[shape->unknown19[1]].flags |= 4;
    m[shape->unknown19[2]].flags |= 4;
    if (obj->walls != -1) {
        walls = &object_walls[obj->walls * 4 + dir];
        for (i = 0; i < walls->count; i++) {
            remove_blank_wall(&m[walls->pos[i]], walls->side[i]);
        }
    }
}

// FUNCTION: 0x00039470
void remove_object_shape(Map *m, Shapes *shape, u8 dir, ObjectControl *obj)
{
    u8 i;
    ObjectWalls *walls;

    for (i = 0; i < shape->count; i++) {
        m[(s16)shape->points[i]].flags &= ~7;
    }
    m[shape->unknown13[0]].flags &= ~4;
    m[shape->unknown13[1]].flags &= ~4;
    m[shape->unknown13[2]].flags &= ~4;
    m[shape->unknown19[0]].flags &= ~4;
    m[shape->unknown19[1]].flags &= ~4;
    m[shape->unknown19[2]].flags &= ~4;
    if (obj->walls != -1) {
        walls = &object_walls[obj->walls * 4 + dir];
        for (i = 0; i < walls->count; i++) {
            insert_blank_wall(&m[walls->pos[i]], walls->side[i]);
        }
    }
}

// FUNCTION: 0x000395b0
void double_door_swings(Thing *w)
{
    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
        if (frame[w->sprite].flags & 1) {
            w->unknown98 = 0;
            w->kind = 2;
            if (w->subtype == 0x35) {
                u16 f = check_frame(0x3eb, w) + w->unknown17;
                w->sprite = start[f].frame;
                w->palette = start[f].flags > 0;
            } else {
                u16 f2 = check_frame(0x3e6, w) + w->unknown17;
                w->sprite = start[f2].frame;
                w->palette = start[f2].flags > 0;
            }
        }
    }
}

// FUNCTION: 0x00039704
void sliding_door_closed(Thing *w)
{
    if (should_sliding_door_open(w)) {
        w->kind = 0x16;
    }
}

// FUNCTION: 0x00039734
void sliding_door_opens(Thing *w)
{
    u16 next;

    if (actual_game_turn % w->anim_speed == 0) {
        next = frame[w->sprite].next;
        if (frame[next].flags & 1) {
            w->height = 0x32;
            w->kind = 0x18;
        } else {
            w->sprite = next;
        }
    }
}

// FUNCTION: 0x000397b4
void sliding_door_open(Thing *w)
{
    if (should_sliding_door_open(w)) {
        w->height = 0x32;
    } else {
        w->height--;
        if (w->height == 0) {
            u16 f = check_frame(sliding_doors[w->subtype - 0x3a].close_anim, w) + w->unknown17;
            w->sprite = start[f].frame;
            w->palette = start[f].flags > 0;
            w->kind = 0x1a;
        }
    }
}

// FUNCTION: 0x00039880
void sliding_door_closes(Thing *w)
{
    u8 n;

    if (should_sliding_door_open(w)) {
        n = 0;
        while (!(frame[w->sprite].flags & 1)) {
            w->sprite = frame[w->sprite].next;
            n++;
        }
        u16 f = check_frame(sliding_doors[w->subtype - 0x3a].open_anim, w) + w->unknown17;
        w->sprite = start[f].frame;
        w->palette = start[f].flags > 0;
        for (n--; n != 0; n--) {
            w->sprite = frame[w->sprite].next;
        }
        w->kind = 0x16;
    } else if (actual_game_turn % w->anim_speed == 0) {
        u16 next = frame[w->sprite].next;
        if (frame[next].flags & 1) {
            u16 f2 = check_frame(sliding_doors[w->subtype - 0x3a].open_anim, w) + w->unknown17;
            w->sprite = start[f2].frame;
            w->palette = start[f2].flags > 0;
            w->kind = 0x14;
        } else {
            w->sprite = next;
        }
    }
}

// FUNCTION: 0x00039a90
u8 should_sliding_door_open(Thing *w)
{
    s16 offset;
    Thing *t;
    Map *cell;

    offset = sliding_doors[w->subtype - 0x3a].offset[w->unknown17];
    cell = &map[((s16)w->y.tile << 7) + (s16)w->x.tile + shapes[objects[w->subtype].shapes[w->unknown17]].unknown13[1] + offset];
    for (t = &thing[cell->first]; t > thing; t = &thing[t->next]) {
        if (t->renderer == 2 || t->renderer == 4 || t->renderer == 0x14 || t->renderer == 0x16) {
            return 1;
        }
    }
    cell = &map[shapes[objects[w->subtype].shapes[w->unknown17]].unknown19[1] + (((s16)w->y.tile << 7) + (s16)w->x.tile) + offset];
    for (t = &thing[cell->first]; t > thing; t = &thing[t->next]) {
        if (t->renderer == 2 || t->renderer == 4 || t->renderer == 0x14 || t->renderer == 0x16) {
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
    Thing *t;

    if (which_room(tile) != room) {
        return 1;
    }
    for (t = &thing[map[tile].first]; t > thing; t = &thing[t->next]) {
        if (t->renderer == 2 || t->renderer == 4 || t->renderer == 0x14 || t->renderer == 0x16
            || t->renderer == 0xa) {
            if (side == -1) {
                return 0;
            }
            if (t->unknown17 == side) {
                if (t->renderer == 2 && t->kind == 0x18 && t->icon != 0x46) {
                    return 1;
                }
                if (t->renderer == 2 && t->kind == 0x44) {
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
    AllHosps *h;

    h = &allhosps[owner];
    switch (objects[kind].unknown14) {
    case 1:
    case 2:
        h->fc97++;
        h->f1bf += h->object_stats[kind].cost;
        break;
    case 3:
        h->fca8++;
        h->f1bf += h->object_stats[kind].cost;
        break;
    case 4:
        h->fc9a++;
        break;
    case 5:
        h->fc90++;
        break;
    case 6:
        h->fc92++;
        break;
    default:
        break;
    }
    h->fc96++;
    h->object_stats[kind].fA++;
}

// FUNCTION: 0x0003a098
void bomb_ticks(Thing *w)
{
    if (othervar.f32) {
        w->fuse_tick++;
        if (w->fuse_tick >= gbv.f152) {
            w->fuse_tick = 0;
            w->fuse++;
            if (w->fuse >= 8) {
                create_explosion(((u16)w->y.tile << 7) + (u16)w->x.tile, 0x97);
                litter_explosion(((s16)w->y.tile << 7) + (s16)w->x.tile, w->owner);
                fn_00038d04(w, w->owner);
                return;
            }
            u16 f = check_frame(w->fuse + 0x9bf, w) + w->unknown17;
            w->sprite = start[f].frame;
            w->palette = start[f].flags > 0;
        }
    }
    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
    }
}

// FUNCTION: 0x0003a200
void litter_explosion(u32 pos, u8 owner)
{
    u32 count;
    u32 r;
    u32 x0;
    u32 x1;
    u32 y0;
    u32 y1;
    u32 x;
    u32 y;

    count = gbv.f153;
    for (r = 0; r <= gbv.f151; r++) {
        x0 = (pos & 0x7f) - r;
        x1 = x0 + r * 2;
        y0 = (pos >> 7) - r;
        y1 = y0 + r * 2;
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
void replace_machine(Thing *e)
{
    ThingPager *pager;
    s32 cost;
    s32 i;

    cost = allhosps[othervar.ff].object_stats[e->subtype].cost;
    if (allhosps[player].f35 >= cost && allhosps[player].f35 > 0) {
        change_balance(0x24, -cost, 0, e->owner);
        e->value = allhosps[othervar.ff].object_stats[e->subtype].unknown8;
        e->mode = 0;
        e->icon = 0;
        e->smoke[0] = 0;
        e->smoke[1] = 0;
        e->smoke[2] = 0;
        pager = &allhosps[e->owner].ff6d;
        for (i = ((PagerSlots *)pager)->count - 1; i >= 0; i--) {
            if (&thing[((PagerSlots *)pager)->slot[i].thing] == e && ((PagerSlots *)pager)->slot[i].staff) {
                interrupt_staff(&thing[((PagerSlots *)pager)->slot[i].staff]);
                break;
            }
        }
        thingpage_cancel(pager, e - thing);
        reset_icon_value(e, 0x1c);
        play_sound(10, 0x43, -1, -1);
    } else {
        play_sound(10, 0x42, -1, -1);
    }
}

// FUNCTION: 0x0003a5e0
void check_invis_walls_around_object(Thing *w)
{
    ObjectControl *obj;
    ObjectWalls *walls;
    Map *cell;
    u32 side;
    s32 i;

    obj = &objects[w->subtype];
    if (obj->walls != -1) {
        walls = &object_walls[obj->walls * 4 + w->unknown17];
        for (i = 0; i < walls->count; i++) {
            cell = &map[((s16)w->y.tile << 7) + (s16)w->x.tile + walls->pos[i]];
            side = walls->side[i];
            if ((&cell->unknown3)[side] == 0) {
                remove_blank_wall(cell, side);
            }
        }
    }
}
