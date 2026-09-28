#include "hosp.h"

struct FillInWindow {
    u8 wall;
    u8 window;
};

extern FillInWindow fill_in_windows[];

struct RoomNavControl {
    u8 door_count;
    u8 door_objects[10];
    u8 count;
    u8 objects[10];
};

extern RoomNavControl room_nav[];

struct Bounds {
    s8 minX;
    s8 minY;
    s8 maxX;
    s8 maxY;
};

extern "C" {

extern Bounds D_00155f4f[][4];
extern u8 D_001553c7;
extern u8 D_001553c8;
extern u8 D_00155d7d;
extern u8 D_00155e03;
extern u8 D_001547e7[];

u8 fn_00029a54(RoomInfo *room);

}

void mark_object_shape(Map *cell, Shapes *shape, u8 dir, ObjectControl *obj, u8 mark);
void remove_object_shape(Map *cell, Shapes *shape, u8 dir, ObjectControl *obj);
void mark_blueprint_on_map(s16 x0, s16 y0, s16 x1, s16 y1, u8 mode);
void clear_pager_responses(u16 room);
void add_static_shadows(void);
u8 search_fill_in_window(u8 wall);
void check_inside_w_wall(u16 x, u16 y);
void check_inside_n_wall(u16 x, u16 y);
void check_outside_s_wall(u16 x, u16 y);
void check_outside_e_wall(u16 x, u16 y);

// FUNCTION: 0x00026488
u8 search_fill_in_window(u8 wall)
{
    u8 i;
    u8 window;

    i = 0;
    window = 0;
    while (fill_in_windows[i].wall != 0) {
        if (fill_in_windows[i].wall == wall) {
            window = fill_in_windows[i].window;
            break;
        }
        i++;
    }
    return window;
}

// FUNCTION: 0x000264f8
void fill_in_window(u16 x, u16 y, u8 side)
{
    switch (side) {
    case 1:
        check_inside_n_wall(x, y);
        check_outside_e_wall(x, y);
        break;
    case 2:
        check_inside_w_wall(x, y);
        check_outside_s_wall(x, y);
        break;
    case 3:
        check_outside_s_wall(x, y);
        check_outside_e_wall(x, y);
        break;
    case 4:
        check_inside_w_wall(x, y);
        check_inside_n_wall(x, y);
        break;
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x000265e4
void check_inside_w_wall(u16 x, u16 y)
{
    u8 window;

    window = search_fill_in_window(map[((s16)y << 7) + (s16)x].unknown4);
    if (window) {
        if (map[((s16)y << 7) + (s16)x].unknown4 == 0x7d) {
            map[((s16)y << 7) + (s16)x].unknown4 = window;
            if (map[((s16)(y - 1) << 7) + (s16)x].unknown4 == 0x7b) {
                map[((s16)(y - 1) << 7) + (s16)x].unknown4 = window;
            }
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x000266bc
void check_inside_n_wall(u16 x, u16 y)
{
    u8 window;

    window = search_fill_in_window(map[((s16)y << 7) + (s16)x].unknown3);
    if (window) {
        if (map[((s16)y << 7) + (s16)x].unknown3 == 0x7c) {
            map[((s16)y << 7) + (s16)x].unknown3 = window;
            if (map[(s16)(x - 1) + ((s16)y << 7)].unknown3 == 0x7a) {
                map[(s16)(x - 1) + ((s16)y << 7)].unknown3 = window;
            }
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x00026798
void check_outside_s_wall(u16 x, u16 y)
{
    u8 window;

    window = search_fill_in_window(map[(s16)(x - 1) + ((s16)y << 7)].unknown3);
    if (window) {
        if (map[(s16)(x - 1) + ((s16)y << 7)].unknown3 == 0x72) {
            map[(s16)(x - 1) + ((s16)y << 7)].unknown3 = window;
            if (map[((s16)y << 7) + (s16)x].unknown3 == 0x74) {
                map[((s16)y << 7) + (s16)x].unknown3 = window;
                if (map[(s16)(x + 1) + ((s16)y << 7)].unknown3 == 0x76) {
                    map[(s16)(x + 1) + ((s16)y << 7)].unknown3 = window;
                }
            } else if (map[((s16)y << 7) + (s16)x].unknown3 == 0x76) {
                map[((s16)y << 7) + (s16)x].unknown3 = window;
            }
        } else if (map[(s16)(x - 1) + ((s16)y << 7)].unknown3 == 0x74) {
            map[(s16)(x - 1) + ((s16)y << 7)].unknown3 = window;
            if (map[(s16)(x - 2) + ((s16)y << 7)].unknown3 == 0x72) {
                map[(s16)(x - 2) + ((s16)y << 7)].unknown3 = window;
            }
            if (map[((s16)y << 7) + (s16)x].unknown3 == 0x76) {
                map[((s16)y << 7) + (s16)x].unknown3 = window;
            }
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x000269e8
void check_outside_e_wall(u16 x, u16 y)
{
    u8 window;

    window = search_fill_in_window(map[((s16)(y - 1) << 7) + (s16)x].unknown4);
    if (window) {
        if (map[((s16)(y - 1) << 7) + (s16)x].unknown4 == 0x73) {
            map[((s16)(y - 1) << 7) + (s16)x].unknown4 = window;
            if (map[((s16)y << 7) + (s16)x].unknown4 == 0x75) {
                map[((s16)y << 7) + (s16)x].unknown4 = window;
                if (map[((s16)(y + 1) << 7) + (s16)x].unknown4 == 0x77) {
                    map[((s16)(y + 1) << 7) + (s16)x].unknown4 = window;
                }
            } else if (map[((s16)y << 7) + (s16)x].unknown4 == 0x77) {
                map[((s16)y << 7) + (s16)x].unknown4 = window;
            }
        } else if (map[((s16)(y - 1) << 7) + (s16)x].unknown3 == 0x75) {
            map[((s16)(y - 1) << 7) + (s16)x].unknown3 = window;
            if (map[((s16)(y - 2) << 7) + (s16)x].unknown3 == 0x73) {
                map[((s16)(y - 2) << 7) + (s16)x].unknown3 = window;
            }
            if (map[((s16)y << 7) + (s16)x].unknown3 == 0x77) {
                map[((s16)y << 7) + (s16)x].unknown3 = window;
            }
        }
    }
}

// FUNCTION: 0x00026c28
void remove_blank_wall(Map *cell, u8 side)
{
    if ((&cell->unknown3)[side] == 0) {
        (&cell->unknown3)[side] = side + 0x8e;
    } else if ((&cell->unknown3)[side] == side + 0x8e) {
        (&cell->unknown3)[side] = side + 0x90;
    }
}

// FUNCTION: 0x00026c98
void insert_blank_wall(Map *cell, u8 side)
{
    if ((&cell->unknown3)[side] == side + 0x8e) {
        (&cell->unknown3)[side] = 0;
    } else if ((&cell->unknown3)[side] == side + 0x90) {
        (&cell->unknown3)[side] = side + 0x8e;
    }
}

// FUNCTION: 0x00026d08
u8 check_object_accessible(u16 pos, u16 other, u16 target)
{
    if (pos == other) {
        return check_points_connected(pos, target);
    }
    return check_points_connected(pos, target) && check_points_connected(other, target);
}

// FUNCTION: 0x00026d84
u8 check_object_nav(u16 object, Shapes *shape, RoomInfo *room, u16 pos, u16 type)
{
    RoomNavControl *nav;
    u8 i;

    nav = &room_nav[type];
    for (i = 0; i < nav->door_count; i++) {
        if (object == nav->door_objects[i]) {
            if (!check_object_accessible(pos + shape->points[10], pos + shape->points[13], room->unknown6)) {
                return 0;
            }
            break;
        }
    }
    for (i = 0; i < nav->count; i++) {
        if (object == nav->objects[i]) {
            if (!check_object_accessible(pos + shape->points[9], pos + shape->points[12], room->unknown6)) {
                return 0;
            }
            if (!check_object_accessible(pos + shape->points[11], pos + shape->points[14], room->unknown6)) {
                return 0;
            }
            break;
        }
    }
    return 1;
}

// FUNCTION: 0x00026ef0
u8 place_object_nav_check(RoomInfo *room, u16 pos, u8 kind, u8 dir, u16 pos2, u8 kind2, u8 dir2)
{
    u8 x;
    u8 y;
    Thing *w;
    Shapes *shape2;
    Shapes *shape;
    Map *cell;

    cell = &map[pos];
    shape = &shapes[objects[kind].shapes[dir]];
    shape2 = &shapes[objects[kind2].shapes[dir2]];
    mark_object_shape(cell, shape, dir, &objects[kind], 0);
    if (pos2) {
        mark_object_shape(&map[pos2], shape2, dir2, &objects[kind2], 0);
    }
    x = room->pos % 128;
    y = room->pos / 128;
    if (!check_object_nav((u16)kind, shape, room, pos, (u16)room->type)) {
        goto fail;
    }
    if (pos2 && !check_object_nav((u16)kind2, shape2, room, pos2, (u16)room->type)) {
        goto fail;
    }
    for (w = &thing[room->first_object]; w > thing; w = &thing[w->unknown71]) {
        if (w->kind != 0x12 && !check_object_nav((u16)w->subtype, &shapes[objects[w->subtype].shapes[w->unknown17]], room, ((u16)w->y.tile << 7) + (u16)w->x.tile, (u16)room->type)) {
            goto fail;
        }
    }
    remove_object_shape(cell, shape, dir, &objects[kind]);
    if (pos2) {
        remove_object_shape(&map[pos2], shape2, dir2, &objects[kind2]);
    }
    return 1;
fail:
    remove_object_shape(cell, shape, dir, &objects[kind]);
    if (pos2) {
        remove_object_shape(&map[pos2], shape2, dir2, &objects[kind2]);
    }
    return 0;
}

// FUNCTION: 0x000271fc
u16 place_object_restrict_to_room(u16 pos, u16 offset, u8 kind, u8 dir)
{
    s8 newY;
    s8 newX;
    s8 roomY;
    s8 roomX;
    s8 y;
    s8 x;
    Bounds *bounds;
    RoomInfo *room;

    room = &room_info[gamevar.unknown2f];
    bounds = &D_00155f4f[kind][dir];
    x = (pos + offset) % 128;
    y = (pos + offset) / 128;
    roomX = room->pos % 128;
    roomY = room->pos / 128;
    newX = pos % 128;
    newY = pos / 128;
    if (x + bounds->maxX >= roomX + room->f1) {
        newX -= x + bounds->maxX - roomX - room->f1 + 1;
    }
    if (x + bounds->minX < roomX) {
        newX += roomX - x - bounds->minX;
    }
    if (y + bounds->maxY >= roomY + room->f2) {
        newY -= y + bounds->maxY - roomY - room->f2 + 1;
    }
    if (y + bounds->minY < roomY) {
        newY += roomY - y - bounds->minY;
    }
    return newX + (newY << 7);
}

// NONMATCHING: register choice, u8 local slot order
// FUNCTION: 0x000273bc
u8 place_object_change_angle(s8 angle)
{
    s8 i;
    u8 changed;
    Shapes *shape;

    if (angle == -1) {
        angle = next_angle[gamevar.f28];
        while (objects[gamevar.unknown25].shapes[angle] == 0) {
            angle = next_angle[angle];
        }
    } else if (!objects[gamevar.unknown25].shapes[angle]) {
        angle = 0;
    }
    changed = gamevar.f28 != angle;
    gamevar.f28 = angle;
    gamevar.f29 = 1;
    shape = &shapes[objects[gamevar.unknown25].shapes[angle]];
    gamevar.f2b = gamevar.f2c = 0;
    gamevar.f2d = gamevar.f2e = 0;
    for (i = shape->count; i >= 0; i--) {
        s8 x;
        s8 y;

        x = (s16)shape->points[i] % 128;
        y = (s16)shape->points[i] / 128;
        if (x < -0x40) {
            x += 0x80;
            y += -1;
        }
        if (x > 0x40) {
            x -= 0x80;
            y += 1;
        }
        gamevar.f2b = gamevar.f2b > x ? gamevar.f2b : x;
        gamevar.f2d = gamevar.f2d < x ? gamevar.f2d : x;
        gamevar.f2c = gamevar.f2c > y ? gamevar.f2c : y;
        gamevar.f2e = gamevar.f2e < y ? gamevar.f2e : y;
    }
    return changed;
}

// FUNCTION: 0x000275c8
void init_place_object(u8 kind, u8 mode, u16 room, s16 offset)
{
    if (mode == 1) {
        mode = 0;
    }
    gamevar.f27 = objects[kind].unknown18[0];
    if (mode == 0 && gamevar.f27 != -1) {
        mode = 1;
    }
    gamevar.unknown25 = kind;
    gamevar.unknown26 = mode;
    gamevar.unknown2f = room;
    gamevar.f31 = offset;
    place_object_change_angle((s8)gamevar.f28);
    set_mouse_mode(5);
    if (mode == 2) {
        crequester.unknown8 = 3;
    }
    setup_place_object_requester();
    crequester.unknown9 = kind;
    if (mode == 2) {
        crequester.unknown8 = 3;
        set_edit_room_requester_mode(10);
    }
}

// FUNCTION: 0x00027694
void init_double_doors(RoomInfo *room)
{
    Thing *door;
    Thing *other;

    door = 0;
    other = 0;
    door = search_mapwho_for_room_door(&map[room->unknown4], room - room_info);
    if (door > thing) {
        if (door->unknown17 == 1) {
            other = search_mapwho_for_room_door(&map[room->unknown4 - 128], room - room_info);
        } else {
            other = search_mapwho_for_room_door(&map[room->unknown4 - 1], room - room_info);
        }
    } else {
        door = search_mapwho_for_room_door(&map[room->unknown6], room - room_info);
        if (door > thing) {
            if (door->unknown17 == 0) {
                other = search_mapwho_for_room_door(&map[room->unknown6 - 1], room - room_info);
            } else {
                other = search_mapwho_for_room_door(&map[room->unknown6 - 128], room - room_info);
            }
        }
    }
    other->renderer = 6;
    door->unknown100 = other - thing;
    other->unknown102 = door - thing;
    if (door->unknown17 == 1 || door->unknown17 == 3) {
        map[((s16)door->y.tile << 7) + (s16)door->x.tile + 127].flags |= 0x20;
        map[((s16)other->y.tile << 7) + (s16)other->x.tile + 127].flags |= 0x20;
    }
}

// FUNCTION: 0x00027920
void init_edit_room(RoomInfo *room)
{
    u8 x1;
    u8 x;
    u8 y1;
    u8 y;
    BuildState *build;

    build = &allhosps[room->owner].build;
    x = room->pos % 128;
    y = room->pos / 128;
    x1 = x + room->f1 - 1;
    y1 = y + room->f2 - 1;
    mark_blueprint_on_map(x, y, x1, y1, 0);
    build->unknownD = room->type;
    build->room = room - room_info;
    build->x0 = x;
    build->y0 = y;
    build->x1 = x1;
    build->y1 = y1;
    if (D_001553c7 == player) {
        crequester.unknown8 = room->type;
        gamevar.unknown26 = 0;
    }
    if (fn_00029a54(room)) {
        room->f3 = 4;
        if (D_001553c7 == player) {
            setup_place_object_requester();
        }
    } else {
        room->f22 = 1;
        if (D_001553c7 == player) {
            set_mouse_mode(14);
        }
    }
}

// FUNCTION: 0x00027a90
void setup_place_object_requester(void)
{
    u8 count;
    u8 i;

    gamevar.unknown71 = 0;
    for (i = 0; i < 0x3e; i++) {
        count = gamevar.unknown33.unknown0[i];
        if (count != 0) {
            gamevar.unknown71++;
        }
    }
    set_edit_room_requester_mode(6);
}

// FUNCTION: 0x00027af0
u8 next_object_to_build(void)
{
    u8 i;

    for (i = 0; i < 0x3e; i++) {
        if (gamevar.unknown33.unknown0[i] != 0) {
            return i;
        }
    }
    return 0;
}

// FUNCTION: 0x00027b44
void open_room(u8 p)
{
    RoomInfo *room;
    BuildState *build;
    u8 x;
    u8 y;

    room = &room_info[allhosps[p].unknown11b7];
    build = &allhosps[p].build;
    if (room > room_info) {
        room->f3 = 0;
        clear_pager_responses(room - room_info);
        for (x = build->x0; x <= build->x1 + 1; x++) {
            for (y = build->y0; y <= build->y1 + 1; y++) {
                place_small_thing((s16)(x + (y << 7)), 0, 0);
            }
        }
        clear_build_objects(p);
        if (room->unknown33 == 0) {
            change_balance(4, -rooms[room->type].cost, 0, (u16)p);
            room->unknown33 = 1;
        }
        if (p == player && D_00155d7d == 0) {
            D_00155e03 = room->type;
        }
        add_static_shadows();
        D_001553c8 = 1;
        D_00155d7d = 0;
        if (D_001553c7 == player) {
            set_mouse_mode(0);
            close_crequester(5);
            if (D_001547e7[room->type] == 0) {
                show_help_page(0, room->type);
                D_001547e7[room->type] = 1;
            }
        }
        allhosps[p].unknown11b7 = 0;
        NRAdcounts.unknown10++;
    }
}
