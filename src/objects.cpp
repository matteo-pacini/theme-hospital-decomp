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

void mark_object_shape(Map *base_cell, Shapes *shape, u8 dir, ObjectControl *control, u8 cleanse);
void insert_object(u16 tile, Thing *obj, u16 kind);
void delete_object(u16 tile, Thing *obj);
void inc_hardware_stats(u8 kind, u8 owner);
u32 compute_plant_frame(Thing *plant);
void adjust_big_cell_flag(const u16 cell);
void remove_object_shape(Map *cell, Shapes *shape, u8 dir, ObjectControl *control);
u8 check_room_access(u8 owner);
u8 check_no_peeps_in_way(u16 tile, u8 kind, u8 dir);
void remove_object_from_map(Thing *obj);
u8 modify_machine(Thing *machine);
u8 should_sliding_door_open(Thing *door);
u8 check_no_peeps_on_cell(RoomInfo *room, u16 tile, s8 side);
void insert_blank_wall(Map *cell, u8 side);
void remove_blank_wall(Map *cell, u8 side);
void cleanse_area(u8 owner, u8 x, u8 y, u8 w, u8 h);
void litter_explosion(u32 pos, u8 owner);
u8 try_litter_explosion(u32 x, u32 y, u32 *count, u8 owner);
u8 thingpage_cancel(ThingPager *p, u16 id);
void interrupt_peep(Thing *w);

extern "C" {
void object_hell(Thing *obj);
void remove_node(Thing *node);
Thing *search_mapwho(Map *cell, u16 kind);
s32 remove_object(Thing *obj, u8 owner);
#pragma aux remove_object "W?remove_object$n(pn$Thing$$)s"
}

// Returns 1 if an object of kind `kind` (objects[] index) facing dir fits
// with its corner at tile for player `owner`: no walls, objects or use tiles
// in its footprint, all of it on the owner's land (litter bombs excepted),
// and its use tiles free. Always 1 while othervar.force_place_objects is set.
// FUNCTION: 0x00036930
u8 can_place_object(u16 tile, u16 kind, u8 dir, u8 owner)
{
    Map *cell;
    ObjectControl *control;
    Shapes *shape;
    u8 i;

    if (othervar.force_place_objects) {
        return 1;
    }
    cell = &map[tile];
    control = &objects[kind];
    shape = &shapes[control->shapes[dir]];
    for (i = 0; i < shape->count; i++) {
        if ((cell[(s16)shape->points[i]].wall_n && (shape->point_wall_flags[i] & 1)) ||
            (cell[(s16)shape->points[i]].wall_w && (shape->point_wall_flags[i] & 2)) ||
            (cell[(s16)shape->points[i]].flags & 7)) {
            return 0;
        }
        if (cell[(s16)shape->points[i]].owner_flags != 1 << owner && kind != OBJ_LITTER_BOMB) {
            return 0;
        }
        if (cell->owner_flags & MAP_OUTDOOR) {
            return 0;
        }
    }
    for (i = 0; i < 3; i++) {
        if ((cell[shape->use_offsets[i]].wall_n && (shape->use_wall_flags & 1)) ||
            (cell[shape->use_offsets[i]].wall_w && (shape->use_wall_flags & 2)) ||
            (cell[shape->use_offsets[i]].flags & 7)) {
            return 0;
        }
    }
    return 1;
}

// Puts the existing object obj on the map at tile facing dir: marks its
// footprint, sets its use tiles, links it into the map, room and owner
// lists, counts it in the statistics, and places its second part if it has
// one. Does not check that it fits.
// FUNCTION: 0x00036b28
void place_existing_object(Thing *obj, u16 tile, u8 dir)
{
    Map *cell;
    ObjectControl *control;
    Shapes *shape;
    RoomInfo *room;
    NextPart *part;
    Thing *other;
    u16 anim;

    cell = &map[tile];
    control = &objects[obj->subtype];
    shape = &shapes[control->shapes[dir]];
    obj->state = control->initial_state;
    obj->facing = dir;
    obj->sprite = start[control->anim_start + obj->facing].frame;
    obj->flip = start[control->anim_start + obj->facing].flags;
    mark_object_shape(cell, shape, dir, control, 1);
    obj->use_pos[0] = tile + shape->use_offsets[0];
    obj->use_pos[1] = tile + shape->use_offsets[1];
    obj->use_pos[2] = tile + shape->use_offsets[2];
    obj->use_pos[3] = tile + shape->extra_use_offsets[0];
    obj->use_pos[4] = tile + shape->extra_use_offsets[1];
    obj->use_pos[5] = tile + shape->extra_use_offsets[2];
    obj->depth = shape->use_xy_depth[2];
    obj->x.tile = tile % 128;
    obj->y.tile = tile / 128;
    obj->use_x.tile = obj->x.tile + shape->use_xy_depth[0];
    obj->use_y.tile = obj->y.tile + shape->use_xy_depth[1];
    obj->x.frac = obj->use_x.frac = 0x80;
    obj->y.frac = obj->use_y.frac = 0x80;
    insert_node(obj, obj->x, obj->y);
    insert_object(tile, obj, obj->subtype);
    inc_hardware_stats(obj->subtype, obj->owner);
    if (control->category == 6 && (room = which_room(tile)) != 0) {
        room->radiators++;
    }
    if (control->category == 7 && listhead[obj->owner].rooms[ROOM_RECEPTION] == 0) {
        listhead[obj->owner].rooms[ROOM_RECEPTION] = room - room_info;
    }
    if (control->next_part_index != -1) {
        part = &next_part[control->next_part_index];
        other = &thing[place_object((u16)(tile + part->dirs[dir].offset), part->kind, part->dirs[dir].dir, obj->owner)];
        if (other > thing) {
            other->pair = obj - thing;
            obj->pair = other - thing;
        }
    }
    if (obj->subtype == OBJ_ENTRANCE_LEFT_DOOR || obj->subtype == OBJ_ENTRANCE_RIGHT_DOOR) {
        if (obj->facing == 1) {
            map[(s16)obj->x.tile + ((s16)obj->y.tile << 7) + 0x7f].flags |= 0x20;
        }
    } else {
        adjust_big_cell_flag(tile);
    }
    switch (obj->subtype) {
    case OBJ_PLANT:
        obj->sprite = compute_plant_frame(obj);
        if (obj->owner == player) {
            NRAdcounts.plants++;
        }
        break;
    case OBJ_BENCH:
        if (obj->owner == player) {
            NRAdcounts.benches++;
        }
        break;
    case OBJ_LITTER_BOMB:
        anim = check_frame(obj->fuse + 0x9bf, obj) + obj->facing;
        obj->sprite = start[anim].frame;
        obj->flip = start[anim].flags > 0;
        obj->owner = (map[tile].owner_flags & 2)   ? 1
                     : (map[tile].owner_flags & 4) ? 2
                     : (map[tile].owner_flags & 8) ? 3
                                                   : 0;
        break;
    }
}

// Places an object of kind `kind` at tile facing dir for player `owner`,
// reusing one the player picked up (allhosps.build.objects) or buying a new
// one. Refuses (returning 0) if it does not fit, would block access to a
// room, or someone is in the way; otherwise returns the object's thing index.
// FUNCTION: 0x00037098
u16 place_object(u16 tile, u16 kind, u8 dir, u8 owner)
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
    othervar.force_place_objects = 0;
    if (!which_room(tile)) {
        u8 blocked;

        mark_object_shape(&map[tile], &shapes[objects[kind].shapes[dir]], dir, &objects[kind], 0);
        blocked = !check_room_access(owner);
        remove_object_shape(&map[tile], &shapes[objects[kind].shapes[dir]], dir, &objects[kind]);
        if (blocked) {
            if (othervar.cur_player == player) {
                advisor.forced_advice_id = 7;
            }
            return 0;
        }
    }
    if (!check_no_peeps_in_way(tile, kind, dir)) {
        return 0;
    }
    if (kind == 0x37) {
        allhosps[owner].rooms[ILL_AUTOPSY].cure_available = 1;
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
            change_balance(TX_BUY_OBJECT, -cost, 0, owner);
        }
        obj->type = THING_OBJECT;
        obj->subtype = kind;
        obj->anim_speed = 2;
        obj->use_style = control->use_style[0];
        obj->owner = owner;
        obj->strength = allhosps[othervar.cur_player].object_stats[kind].strength;
        obj->times_used = 0;
        obj->layers[10] = 0;
        obj->layers[11] = old_newrand() & 3;
        obj->layers[12] = old_newrand() & 3;
        if (kind == 0x2d) {
            obj->plant_health = 400;
            if (obj->owner == player) {
                NRAdcounts.plant_health_sum += 0x37;
                if (NRAdcounts.plant_health_sum < 0) {
                    NRAdcounts.plant_health_sum = 0;
                } else if (NRAdcounts.plants * 0x37 < NRAdcounts.plant_health_sum) {
                    NRAdcounts.plant_health_sum = NRAdcounts.plants * 0x37;
                }
            }
        }
        place_existing_object(obj, tile, dir);
        return obj - thing;
    }
    return 0;
}

// Object state OS_STATIC: advances its animation and overlay.
// FUNCTION: 0x0003749c
void object_static(Thing *obj)
{
    if (actual_game_turn % obj->anim_speed == 0) {
        obj->sprite = frame[obj->sprite].next;
        if (obj->overlay_frame) {
            obj->overlay_frame = frame[obj->overlay_frame].next;
        }
    }
}

// Animates an object playing a one-off animation: at its end the object
// switches to aux_sprite, or is removed if remove_after_anim is set.
// FUNCTION: 0x0003751c
void object_hell(Thing *obj)
{
    CompFrame *cur_frame;

    if (actual_game_turn % obj->anim_speed == 0) {
        obj->sprite = frame[obj->sprite].next;
        cur_frame = &frame[obj->sprite];
        if (cur_frame->flags & 1) {
            if (obj->aux_sprite) {
                obj->sprite = obj->aux_sprite;
                obj->flip = obj->aux_flip_w;
                obj->aux_sprite = 0;
                obj->aux_flip_w = 0;
            } else if (obj->remove_after_anim) {
                remove_object(obj, obj->owner);
            }
        }
    }
}

// Links obj at the head of the object list of the room at tile (setting
// room_id, 0 outside rooms) and of its owner's list of kind `kind`.
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
            obj->room_obj_next = first;
            thing[first].room_obj_prev = id;
        }
        room->first_object = id;
        obj->room_id = room - room_info;
    } else {
        obj->room_id = 0;
    }
    first = listhead[obj->owner].objects[kind];
    if (first) {
        obj->type_obj_next = first;
        thing[first].type_obj_prev = id;
    }
    listhead[obj->owner].objects[kind] = id;
}

// Unlinks obj from its room's and owner's object lists (see insert_object),
// clearing the auto-autopsy flag when the owner has none left.
// FUNCTION: 0x00037718
void delete_object(u16 tile, Thing *obj)
{
    RoomInfo *room;
    Thing *prev;
    Thing *next;

    room = which_room(tile);
    if (room) {
        if (room->first_object) {
            prev = &thing[obj->room_obj_prev];
            if (prev > thing) {
                prev->room_obj_next = obj->room_obj_next;
            } else {
                room->first_object = obj->room_obj_next;
            }
            next = &thing[obj->room_obj_next];
            if (next > thing) {
                next->room_obj_prev = obj->room_obj_prev;
            }
        }
        obj->room_obj_prev = obj->room_obj_next = 0;
    }
    if (listhead[obj->owner].objects[obj->subtype]) {
        prev = &thing[obj->type_obj_prev];
        if (prev > thing) {
            prev->type_obj_next = obj->type_obj_next;
        } else {
            listhead[obj->owner].objects[obj->subtype] = obj->type_obj_next;
        }
        next = &thing[obj->type_obj_next];
        if (next > thing) {
            next->type_obj_prev = obj->type_obj_prev;
        }
        obj->type_obj_prev = obj->type_obj_next = 0;
    }
    if (listhead[obj->owner].objects[OBJ_AUTO_AUTOPSY] == 0) {
        allhosps[obj->owner].rooms[ILL_AUTOPSY].cure_available = 0;
    }
}

// Finds an object of kind `kind` for tile: in the room at tile, or outside
// rooms within 7 tiles of it and reachable. flag 0x96 skips objects in use;
// random picks among up to 30 matches. Returns the thing index, or 0.
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
                    if (flag != 0x96 || obj->in_use == 0) {
                        if (random && count < 30) {
                            found[count++] = obj - thing;
                        } else {
                            return obj - thing;
                        }
                    }
                }
            } while ((obj = &thing[obj->room_obj_next]) > thing);
        }
    } else {
        obj = &thing[listhead[owner].objects[kind]];
        if (obj > thing) {
            x = tile % 128;
            y = tile / 128;
            do {
                if (flag == 0x96 && obj->in_use != 0) {
                    continue;
                }
                if (abs(x - obj->x.tile) <= 7) {
                    if (abs(y - obj->y.tile) <= 7) {
                        if (can_i_get_to(obj, tile) != 0) {
                            if (!obj->room_id) {
                                if (random && count < 30) {
                                    found[count++] = obj - thing;
                                } else {
                                    return obj - thing;
                                }
                            }
                        }
                    }
                }
            } while ((obj = &thing[obj->type_obj_next]) > thing);
        }
    }
    if (count) {
        return found[old_newrand() % count];
    }
    return 0;
}

// Returns player `owner`'s object of kind `kind`: the first, a random one if
// random is set, or the one after thing `after` in the owner's list (0 if
// none).
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
        } while ((obj = &thing[obj->type_obj_next]) > thing);
    }
    if (count) {
        return found[old_newrand() % count];
    }
    return 0;
}

// Returns a free object of kind `kind` for person id: a random one in their
// room, or the nearest reachable one outside rooms within 7 tiles. 0 if
// none.
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
        for (obj = &thing[room->first_object]; obj > thing; obj = &thing[obj->room_obj_next]) {
            if (obj->subtype == kind && obj->in_use == 0) {
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
    if (seeker->type == THING_STAFF) {
        side = seeker->subtype == STAFF_HANDYMAN ? 2 : 0;
    } else {
        side = 1;
    }
    for (obj = &thing[listhead[seeker->owner].objects[kind]]; obj > thing; obj = &thing[obj->type_obj_next]) {
        if (obj->in_use == 0) {
            if ((dx = abs(seeker->x.tile - obj->x.tile)) <= 7) {
                if ((dy = abs(seeker->y.tile - obj->y.tile)) <= 7) {
                    pos = obj->use_pos[side];
                    if (can_i_get_to(obj, pos) != 0) {
                        if (!obj->room_id) {
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

// As find_nearest_free_object, without the 7-tile limit outside rooms. In a
// room, an object of the kind that is in use makes it loop forever (the loop
// never advances past it).
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
                    if (obj->in_use != 0) {
                        continue;
                    }
                    if (count < 30) {
                        found[count++] = obj - thing;
                    } else {
                        return obj - thing;
                    }
                }
                obj = &thing[obj->room_obj_next];
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
    if (seeker->type == THING_STAFF) {
        side = seeker->subtype == STAFF_HANDYMAN ? 2 : 0;
    } else {
        side = 1;
    }
    for (obj = &thing[listhead[seeker->owner].objects[kind]]; obj > thing; obj = &thing[obj->type_obj_next]) {
        if (obj->in_use == 0) {
            pos = obj->use_pos[side];
            if (can_i_get_to(seeker, pos) != 0) {
                if (!obj->room_id) {
                    dist = abs(seeker->x.tile - obj->x.tile) + abs(seeker->y.tile - obj->y.tile);
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

// Returns the reception desk with a receptionist that suits person: in room
// `room` (0 any), with space in its queue if need_space, scoring queue length
// and distance (gbv.q_points, gbv.dist_points). 0 if none.
// FUNCTION: 0x000382d8
u16 find_nearest_manned_reception(Thing *person, const u8 room, const u8 need_space)
{
    Thing *best;
    s32 best_score;
    Thing *desk;
    Thing *staff;
    s32 free;
    s32 score;

    best = thing;
    best_score = 9999;
    for (desk = &thing[listhead[person->owner].objects[OBJ_RECEPTION_DESK]]; desk > thing;
         desk = &thing[desk->type_obj_next]) {
        staff = &thing[desk->user];
        if (staff > thing) {
            if (!staff->room || !(room == 0 || (u16)(staff->room - room_info) == room)) {
                continue;
            }
            free = staff->room->queue_capacity - staff->room->queue_length;
            if (free <= 0 && need_space) {
                continue;
            }
            score = staff->room->queue_length * gbv.q_points;
            score += (abs(desk->x.tile - person->x.tile) + abs(desk->y.tile - person->y.tile)) * gbv.dist_points;
            score -= free;
            if (score < best_score) {
                best_score = score;
                best = desk;
            }
        }
    }
    return best - thing;
}

// Returns 1 if no object stands in the footprint of a door of kind `kind`
// facing dir at tile.
// FUNCTION: 0x000384a4
u8 can_place_door(u16 tile, u16 kind, u8 dir)
{
    Map *cell;
    ObjectControl *control;
    Shapes *shape;
    u8 i;

    cell = &map[tile];
    control = &objects[kind];
    shape = &shapes[control->shapes[dir]];
    for (i = 0; i < shape->count; i++) {
        if (cell[(s16)shape->points[i]].flags & 7) {
            return 0;
        }
    }
    return 1;
}

// Creates a door of kind `kind` at tile facing gamevar.place_dir, belonging to the
// room it opens into. Returns 1 if it was placed, 0 if blocked or no thing
// was free.
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
    shape = &shapes[control->shapes[gamevar.place_dir]];
    if (!can_place_door(tile, kind, gamevar.place_dir)) {
        return 0;
    }
    door = allocate_thing(0);
    if (door) {
        for (i = 0; i < shape->count; i++) {
            cell[(s16)shape->points[i]].flags |= 2;
        }
        door->type = THING_DOOR;
        door->facing = gamevar.place_dir;
        door->subtype = kind;
        door->sprite = start[control->anim_start + door->facing].frame;
        door->flip = start[control->anim_start + door->facing].flags;
        door->anim_speed = 2;
        door->use_pos[0] = tile + shape->use_offsets[0];
        door->use_pos[3] = tile + shape->extra_use_offsets[0];
        door->x.tile = tile % 128;
        door->y.tile = tile / 128;
        door->x.frac = door->y.frac = 0x80;
        door->depth = shape->use_xy_depth[2];
        door->state = OS_STATIC;
        insert_node(door, door->x, door->y);
        room = which_room(tile + door_to_room[door->facing]);
        if (room) {
            door->room_id = room - room_info;
            door->owner = room->owner;
        }
        return 1;
    }
    return 0;
}

// Returns a door on or next to person's tile (special_around): the first found
// while they are in a room, else only one whose room's door_tile they stand
// on; 0 if none.
// FUNCTION: 0x00038788
Thing *find_door(Thing *person)
{
    u8 i;
    Thing *door;
    Map *cell;

    cell = &map[((s16)person->y.tile << 7) + (s16)person->x.tile];
    for (i = 0; i < 5; i++) {
        door = search_mapwho(&cell[special_around[i]], THING_DOOR);
        if (door > thing) {
            if (person->room ||
                (s16)person->x.tile + ((s16)person->y.tile << 7) == room_info[door->room_id].door_tile) {
                return door;
            }
        }
    }
    return 0;
}

// Puts small thing (blueprint marker) id on tile, stored negated at the end
// of the cell's thing list. With keep_existing, an existing one stays. Ids
// from 0x4b also set the marker's cell flags. Returns 0.
// FUNCTION: 0x00038870
s16 place_small_thing(u16 tile, u16 id, u8 keep_existing)
{
    Map *cell;
    Thing *last;
    s16 *link;

    cell = &map[tile];
    if (cell->first > 0) {
        last = &thing[cell->first];
        if (last > thing) {
            for (; last->cell_next > 0; last = &thing[last->cell_next]) {
            }
            link = &last->cell_next;
        }
    } else {
        link = &cell->first;
    }
    if (keep_existing == 0 || *link == 0) {
        *link = id;
        *link = -*link;
        if (id >= 0x4b) {
            cell->flags |= blueprint[id].flags;
        }
    }
    return 0;
}

// Returns the small thing (blueprint marker) id on tile, 0 if none.
// FUNCTION: 0x00038950
s16 find_small_thing(u16 tile)
{
    Map *cell;
    s16 id;

    cell = &map[tile];
    for (id = cell->first; id > 0; id = thing[id].cell_next) {
    }
    return -id;
}

// Takes obj off the map: cancels its pages, unmarks its footprint, updates
// the statistics, interrupts a person using it, and unlinks it from the map
// and object lists. The thing stays allocated.
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
    case OBJ_PLANT:
        thingpage_cancel(&allhosps[obj->owner].plant_pager, obj - thing);
        NRAdcounts.plant_health_sum -= (obj->plant_health * 2 + 0x50) / 16;
        if (obj->owner == player) {
            NRAdcounts.plants--;
            if (obj->plant_health < 0) {
                NRAdcounts.thirsty_plants--;
            }
        }
        break;
    case OBJ_BENCH:
        if (obj->owner == player) {
            NRAdcounts.benches--;
        }
        break;
    default:
        break;
    }
    thingpage_cancel(&allhosps[obj->owner].repair_pager, obj - thing);
    control = &objects[obj->subtype];
    shape = &shapes[control->shapes[obj->facing]];
    remove_object_shape(cell, shape, obj->facing, control);
    switch (control->category) {
    case 1:
    case 2:
        hosp->machine_count--;
        hosp->object_value -= hosp->object_stats[obj->subtype].cost;
        break;
    case 3:
        hosp->leisure_count--;
        hosp->object_value -= hosp->object_stats[obj->subtype].cost;
        break;
    case 4:
        hosp->amenity_count--;
        break;
    case 5:
        hosp->extinguisher_count--;
        break;
    case 6:
        hosp->radiator_count--;
        room = which_room(tile);
        if (room) {
            room->radiators--;
        }
        break;
    default:
        break;
    }
    hosp->object_count--;
    hosp->object_stats[obj->subtype].count++;
    if (obj->state == OS_IN_USE) {
        user = &thing[obj->user];
        if (user > thing) {
            interrupt_peep(user);
        }
    }
    obj->state = OS_REMOVED;
    delete_object(tile, obj);
    adjust_big_cell_flag(tile);
    remove_node(obj);
}

// Removes obj and frees its thing, if it stands on player `owner`'s land (or
// is the gates of hell or a litter bomb). Returns 1 if removed.
// FUNCTION: 0x00038d04
s32 remove_object(Thing *obj, u8 owner)
{
    Map *cell;

    cell = &map[(s16)obj->x.tile + ((s16)obj->y.tile << 7)];
    if (!(obj->subtype == OBJ_GATES_OF_HELL || obj->subtype == OBJ_LITTER_BOMB || cell->owner_flags == 1 << owner)) {
        return 0;
    }
    remove_object_from_map(obj);
    memset(obj, 0, sizeof(Thing));
    return 1;
}

// Counts a use of the treatment machine for illness in the room at tile:
// pages a handyman once it is worn past a third and calls modify_machine
// from 75%. Returns 1 if the machine broke down (machine_disaster).
// FUNCTION: 0x00038db0
u8 treatment_machine_used(const u16 illness, const u16 tile)
{
    ObjectControl *control;
    u16 id;
    u16 kind;
    Thing *machine;
    s32 percent;

    kind = expertise[illness].kind;
    control = &objects[kind];
    if (control->overlay_anim) {
        id = find_object(tile, kind, 0, 0x97, 0);
        machine = &thing[id];
        if (machine > thing) {
            if (machine->strength * 2 > machine->times_used) {
                machine->times_used++;
            }
            if (machine->times_used > machine->strength / 3) {
                percent = machine->times_used * 100 / (machine->strength + 1);
                thingpage_request(&allhosps[machine->owner].repair_pager, (u16)(machine - thing),
                                  percent < 100 ? percent : 100);
            }
            if (machine->strength * 75 / 100 <= machine->times_used) {
                return modify_machine(machine);
            }
        }
    }
    return 0;
}

// Updates a worn machine: smoke and warnings from 75% of its strength, and
// from 100% a random breakdown (machine_disaster), certain by 150%. Returns
// 1 if it broke down.
// FUNCTION: 0x00038f14
u8 modify_machine(Thing *machine)
{
    ObjectControl *control;
    s32 j;
    s32 i;
    s32 count;

    count = 0;
    control = &objects[machine->subtype];
    if (machine->strength * 75 / 100 <= machine->times_used) {
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
        if (machine->strength * 85 / 100 <= machine->times_used && count != 0) {
            play_sound(SNDSRC_GENERAL, SFX_MACHWARN, -1, -1);
            if (machine->strength * 95 / 100 <= machine->times_used) {
                machine->smoke[0] = 1;
                machine->smoke[1] = 1;
                machine->smoke[2] = 1;
                machine->overlay_frame = start[control->overlay_anim].frame;
            } else if (count == 1) {
                for (j = 0; j < 3; j++) {
                    if (machine->smoke[j] == 1) {
                        break;
                    }
                }
                j = (j + 1 + (old_newrand() & 1)) % 3;
                machine->smoke[j] = 1;
                machine->overlay_frame = start[control->overlay_anim].frame;
            }
        } else {
            j = old_newrand() % 3;
            machine->smoke[j] = 1;
            machine->overlay_frame = start[control->overlay_anim].frame;
        }
    }
    if (machine->times_used >= machine->strength) {
        i = machine->strength * 3 / 2 - machine->times_used + 1;
        if (i <= 1 || old_newrand() % i == 0) {
            machine_disaster(machine);
            return 1;
        }
    }
    return 0;
}

// Marks an object's footprint on the map from base_cell (flags bit 0, or bit 1
// on the tiles that are also shape->points[9] to [14]), its use tiles (bit 2)
// and its invisible walls. With
// cleanse, also cleans the litter under it.
// FUNCTION: 0x0003917c
void mark_object_shape(Map *base_cell, Shapes *shape, u8 dir, ObjectControl *control, u8 cleanse)
{
    u8 i;
    Map *cell;
    ObjectWalls *walls;

    for (i = 0; i < shape->count; i++) {
        if (shape->points[9] == shape->points[i] || shape->points[10] == shape->points[i] ||
            shape->points[11] == shape->points[i] || shape->points[12] == shape->points[i] ||
            shape->points[13] == shape->points[i] || shape->points[14] == shape->points[i]) {
            base_cell[(s16)shape->points[i]].flags |= 2;
        } else {
            base_cell[(s16)shape->points[i]].flags |= 1;
        }
        if (cleanse) {
            cell = &base_cell[(s16)shape->points[i]];
            cleanse_area((cell->owner_flags & 2)   ? 1
                         : (cell->owner_flags & 4) ? 2
                         : (cell->owner_flags & 8) ? 3
                                                   : 0,
                         (u16)(cell - map) % 128, (u16)(cell - map) / 128, 1, 1);
        }
    }
    base_cell[shape->use_offsets[0]].flags |= 4;
    base_cell[shape->use_offsets[1]].flags |= 4;
    base_cell[shape->use_offsets[2]].flags |= 4;
    base_cell[shape->extra_use_offsets[0]].flags |= 4;
    base_cell[shape->extra_use_offsets[1]].flags |= 4;
    base_cell[shape->extra_use_offsets[2]].flags |= 4;
    if (control->walls != -1) {
        walls = &object_walls[control->walls * 4 + dir];
        for (i = 0; i < walls->count; i++) {
            insert_blank_wall(&base_cell[walls->pos[i]], walls->side[i]);
        }
    }
}

// Clears what mark_object_shape marked.
// FUNCTION: 0x00039470
void remove_object_shape(Map *cell, Shapes *shape, u8 dir, ObjectControl *control)
{
    u8 i;
    ObjectWalls *walls;

    for (i = 0; i < shape->count; i++) {
        cell[(s16)shape->points[i]].flags &= ~7;
    }
    cell[shape->use_offsets[0]].flags &= ~4;
    cell[shape->use_offsets[1]].flags &= ~4;
    cell[shape->use_offsets[2]].flags &= ~4;
    cell[shape->extra_use_offsets[0]].flags &= ~4;
    cell[shape->extra_use_offsets[1]].flags &= ~4;
    cell[shape->extra_use_offsets[2]].flags &= ~4;
    if (control->walls != -1) {
        walls = &object_walls[control->walls * 4 + dir];
        for (i = 0; i < walls->count; i++) {
            remove_blank_wall(&cell[walls->pos[i]], walls->side[i]);
        }
    }
}

// Door state OS_DOUBLE_DOOR_SWINGS: plays the swing and returns to the
// closed frame and OS_STATIC.
// FUNCTION: 0x000395b0
void double_door_swings(Thing *door)
{
    if (actual_game_turn % door->anim_speed == 0) {
        door->sprite = frame[door->sprite].next;
        if (frame[door->sprite].flags & 1) {
            door->in_use = 0;
            door->state = OS_STATIC;
            if (door->subtype == OBJ_SWING_DOOR2) {
                u16 anim = check_frame(0x3eb, door) + door->facing;
                door->sprite = start[anim].frame;
                door->flip = start[anim].flags > 0;
            } else {
                u16 anim2 = check_frame(0x3e6, door) + door->facing;
                door->sprite = start[anim2].frame;
                door->flip = start[anim2].flags > 0;
            }
        }
    }
}

// Door state OS_SLIDING_DOOR_CLOSED: opens when someone is at the door.
// FUNCTION: 0x00039704
void sliding_door_closed(Thing *door)
{
    if (should_sliding_door_open(door)) {
        door->state = OS_SLIDING_DOOR_OPENS;
    }
}

// Door state OS_SLIDING_DOOR_OPENS: plays the opening animation, then stays
// open for 50 turns.
// FUNCTION: 0x00039734
void sliding_door_opens(Thing *door)
{
    u16 next;

    if (actual_game_turn % door->anim_speed == 0) {
        next = frame[door->sprite].next;
        if (frame[next].flags & 1) {
            door->countdown = 0x32;
            door->state = OS_SLIDING_DOOR_OPEN;
        } else {
            door->sprite = next;
        }
    }
}

// Door state OS_SLIDING_DOOR_OPEN: closes 50 turns after the last person
// passes.
// FUNCTION: 0x000397b4
void sliding_door_open(Thing *door)
{
    if (should_sliding_door_open(door)) {
        door->countdown = 0x32;
    } else {
        door->countdown--;
        if (door->countdown == 0) {
            u16 anim = check_frame(sliding_doors[door->subtype - 0x3a].close_anim, door) + door->facing;
            door->sprite = start[anim].frame;
            door->flip = start[anim].flags > 0;
            door->state = OS_SLIDING_DOOR_CLOSES;
        }
    }
}

// Door state OS_SLIDING_DOOR_CLOSES: plays the closing animation, reopening
// from the same position if someone arrives.
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
        u16 anim = check_frame(sliding_doors[door->subtype - 0x3a].open_anim, door) + door->facing;
        door->sprite = start[anim].frame;
        door->flip = start[anim].flags > 0;
        for (steps--; steps != 0; steps--) {
            door->sprite = frame[door->sprite].next;
        }
        door->state = OS_SLIDING_DOOR_OPENS;
    } else if (actual_game_turn % door->anim_speed == 0) {
        u16 next = frame[door->sprite].next;
        if (frame[next].flags & 1) {
            u16 anim2 = check_frame(sliding_doors[door->subtype - 0x3a].open_anim, door) + door->facing;
            door->sprite = start[anim2].frame;
            door->flip = start[anim2].flags > 0;
            door->state = OS_SLIDING_DOOR_CLOSED;
        } else {
            door->sprite = next;
        }
    }
}

// Returns 1 if a person, the mayor or the inspector stands on a sliding
// door's tile on either side.
// FUNCTION: 0x00039a90
u8 should_sliding_door_open(Thing *door)
{
    s16 offset;
    Thing *occupant;
    Map *cell;

    offset = sliding_doors[door->subtype - 0x3a].offset[door->facing];
    cell = &map[((s16)door->y.tile << 7) + (s16)door->x.tile +
                shapes[objects[door->subtype].shapes[door->facing]].use_offsets[1] + offset];
    for (occupant = &thing[cell->first]; occupant > thing; occupant = &thing[occupant->cell_next]) {
        if (occupant->type == THING_PATIENT || occupant->type == THING_STAFF || occupant->type == THING_MAYOR ||
            occupant->type == THING_INSPECTOR) {
            return 1;
        }
    }
    cell = &map[shapes[objects[door->subtype].shapes[door->facing]].extra_use_offsets[1] +
                (((s16)door->y.tile << 7) + (s16)door->x.tile) + offset];
    for (occupant = &thing[cell->first]; occupant > thing; occupant = &thing[occupant->cell_next]) {
        if (occupant->type == THING_PATIENT || occupant->type == THING_STAFF || occupant->type == THING_MAYOR ||
            occupant->type == THING_INSPECTOR) {
            return 1;
        }
    }
    return 0;
}

// Returns 1 if no one stands in the footprint of an object of kind `kind`
// facing dir at tile, or faces into it from a neighbouring tile of the same
// room (see check_no_peeps_on_cell).
// FUNCTION: 0x00039cb4
u8 check_no_peeps_in_way(u16 tile, u8 kind, u8 dir)
{
    s8 i;
    RoomInfo *room;
    Shapes *shape;

    room = which_room(tile);
    shape = &shapes[objects[kind].shapes[dir]];
    for (i = shape->count - 1; i >= 0; i--) {
        if (!(check_no_peeps_on_cell(room, tile + shape->points[i], -1) &&
              check_no_peeps_on_cell(room, tile + shape->points[i] - 0x80, 2) &&
              check_no_peeps_on_cell(room, tile + shape->points[i] + 1, 3) &&
              check_no_peeps_on_cell(room, tile + shape->points[i] + 0x80, 0) &&
              check_no_peeps_on_cell(room, tile + shape->points[i] - 1, 1))) {
            return 0;
        }
    }
    return 1;
}

// Returns 0 if a person (or the mayor, inspector or reaper) stands on tile in
// room `room` and, unless side is -1, faces direction side (patients idling or
// awaiting a decision excepted); 1 otherwise, or if tile is not in room.
// FUNCTION: 0x00039e10
u8 check_no_peeps_on_cell(RoomInfo *room, u16 tile, s8 side)
{
    Thing *occupant;

    if (which_room(tile) != room) {
        return 1;
    }
    for (occupant = &thing[map[tile].first]; occupant > thing; occupant = &thing[occupant->cell_next]) {
        if (occupant->type == THING_PATIENT || occupant->type == THING_STAFF || occupant->type == THING_MAYOR ||
            occupant->type == THING_INSPECTOR || occupant->type == THING_REAPER) {
            if (side == -1) {
                return 0;
            }
            if (occupant->facing == side) {
                if (occupant->type == THING_PATIENT && occupant->state == PS_ENTERTAINS_THEMSELF &&
                    occupant->complex_action != 0x46) {
                    return 1;
                }
                if (occupant->type == THING_PATIENT && occupant->state == PS_WAITS_FOR_DECISION) {
                    return 1;
                }
                return 0;
            }
        }
    }
    return 1;
}

// Counts a new object of kind `kind` in player `owner`'s hardware statistics
// and hospital value.
// FUNCTION: 0x00039f8c
void inc_hardware_stats(u8 kind, u8 owner)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    switch (objects[kind].category) {
    case 1:
    case 2:
        hosp->machine_count++;
        hosp->object_value += hosp->object_stats[kind].cost;
        break;
    case 3:
        hosp->leisure_count++;
        hosp->object_value += hosp->object_stats[kind].cost;
        break;
    case 4:
        hosp->amenity_count++;
        break;
    case 5:
        hosp->extinguisher_count++;
        break;
    case 6:
        hosp->radiator_count++;
        break;
    default:
        break;
    }
    hosp->object_count++;
    hosp->object_stats[kind].count++;
}

// Object state OS_BOMB_TICKS (litter bomb): burns the fuse one step every
// gbv.bomb_fuse_ticks turns while othervar.every_16_turns is set, and at the end explodes,
// scattering litter.
// FUNCTION: 0x0003a098
void bomb_ticks(Thing *bomb)
{
    if (othervar.every_16_turns) {
        bomb->fuse_tick++;
        if (bomb->fuse_tick >= gbv.bomb_fuse_ticks) {
            bomb->fuse_tick = 0;
            bomb->fuse++;
            if (bomb->fuse >= 8) {
                create_explosion(((u16)bomb->y.tile << 7) + (u16)bomb->x.tile, 0x97);
                litter_explosion(((s16)bomb->y.tile << 7) + (s16)bomb->x.tile, bomb->owner);
                remove_object(bomb, bomb->owner);
                return;
            }
            u16 anim = check_frame(bomb->fuse + 0x9bf, bomb) + bomb->facing;
            bomb->sprite = start[anim].frame;
            bomb->flip = start[anim].flags > 0;
        }
    }
    if (actual_game_turn % bomb->anim_speed == 0) {
        bomb->sprite = frame[bomb->sprite].next;
    }
}

// Scatters up to gbv.bomb_litter_count pieces of litter in rings out to gbv.bomb_litter_radius tiles
// around tile pos.
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

    count = gbv.bomb_litter_count;
    for (radius = 0; radius <= gbv.bomb_litter_radius; radius++) {
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

// With a gbv.bomb_litter_chance percent chance, places a random piece of litter on tile
// (x, y) if it is outside rooms and free. Decrements *count for each piece
// and returns 1 when it reaches 0.
// FUNCTION: 0x0003a310
u8 try_litter_explosion(u32 x, u32 y, u32 *count, u8 owner)
{
    s32 pos;
    Map *cell;

    pos = (s16)x + ((s16)y << 7);
    cell = &map[pos];
    if (old_newrand() % 100 > gbv.bomb_litter_chance) {
        return 0;
    }
    if (which_room(pos) == 0 && !(cell->flags & 7) && !(cell->floor & 0xf)) {
        place_litter(pos, (s8)owner, old_newrand() % 3 + 1);
        (*count)--;
        return *count == 0;
    }
    return 0;
}

// Replaces a worn machine with a new one for its cost, if the local player can
// afford it (the check uses the local player's balance, the charge goes to the
// owner), cancelling its repair pages.
// FUNCTION: 0x0003a3e0
void replace_machine(Thing *machine)
{
    ThingPager *pager;
    s32 cost;
    s32 i;

    cost = allhosps[othervar.cur_player].object_stats[machine->subtype].cost;
    if (allhosps[player].balance >= cost && allhosps[player].balance > 0) {
        change_balance(TX_REPLACEMENT_MACHINE, -cost, 0, machine->owner);
        machine->strength = allhosps[othervar.cur_player].object_stats[machine->subtype].strength;
        machine->times_used = 0;
        machine->overlay_frame = 0;
        machine->smoke[0] = 0;
        machine->smoke[1] = 0;
        machine->smoke[2] = 0;
        pager = &allhosps[machine->owner].repair_pager;
        for (i = ((PagerSlots *)pager)->count - 1; i >= 0; i--) {
            if (&thing[((PagerSlots *)pager)->slot[i].thing] == machine && ((PagerSlots *)pager)->slot[i].staff) {
                interrupt_staff(&thing[((PagerSlots *)pager)->slot[i].staff]);
                break;
            }
        }
        thingpage_cancel(pager, machine - thing);
        reset_icon_value(machine, ICON_NEEDS_REPAIR);
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
    } else {
        play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
    }
}

// Restores any of an object's invisible walls that are missing.
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
        walls = &object_walls[control->walls * 4 + obj->facing];
        for (i = 0; i < walls->count; i++) {
            cell = &map[((s16)obj->y.tile << 7) + (s16)obj->x.tile + walls->pos[i]];
            side = walls->side[i];
            if ((&cell->wall_n)[side] == 0) {
                insert_blank_wall(cell, side);
            }
        }
    }
}
