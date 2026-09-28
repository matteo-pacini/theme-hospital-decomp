#include "hosp.h"

struct Bounds {
    s8 minX;
    s8 minY;
    s8 maxX;
    s8 maxY;
};

struct WallHug {
    u8 kind;
    u8 angle[4];
};

struct NextPartEntry {
    u8 angle;
    u32 pos;
    u8 unknown5;
};

struct NextPart {
    u8 kind;
    NextPartEntry parts[4];
};

extern WallHug wall_hug[];
extern NextPart next_part[];

void remove_object_from_map(Thing *w);
s32 depreciation(void);
s16 get_state(const u16 a, const Thing *w);
void mark_object_shape(Map *cell, Shapes *shape, u8 dir, ObjectControl *obj, u8 mark);
void remove_object_shape(Map *cell, Shapes *shape, u8 dir, ObjectControl *obj);

u8 place_object_nav_check(RoomInfo *room, u16 pos, u8 kind, u8 dir, u16 a, u8 b, u8 c);

void check_invis_walls_around_object(Thing *w);
void remove_rathole(u8 owner, u8 index);

extern s16 around24[];

extern "C" {

extern u16 D_001553f9;
extern u8 D_00155d7e, D_00155d7f, D_00155d80, D_00155d81;

Thing *fn_00019aac(Map *t, u16 kind);

extern u8 D_001553c7, D_001553c8;
extern u16 D_001553e0, D_001553fb;
extern Bounds D_00155f4f[][4];
extern Event D_001549d0[];

}

// FUNCTION: 0x00027d48
u16 compute_object_pointed(RoomInfo *room)
{
    Thing *w;

    w = &thing[D_001553f9];
    if (w->owner == player) {
        if (room) {
            if (!(!(w->renderer == 6 && (u16)(room - room_info) == w->unknown89) || w->subtype == 0x34 || w->subtype == 0x35)) {
                return D_001553f9;
            } else {
                return 0;
            }
        } else {
            return D_001553f9;
        }
    } else {
        return 0;
    }
}

// FUNCTION: 0x00027e1c
void pick_up_object(u16 id, u8 both)
{
    Thing *w;
    u16 room;
    BuildState *build;
    u8 i;
    Thing *other;
    Thing *staff;
    u16 anim;

    w = &thing[id];
    room = 0;
    build = &allhosps[w->owner].build;
    for (i = 0; i < 20; i++) {
        if (build->objects[i] == 0) {
            build->objects[i] = id;
            break;
        }
    }
    if (both) {
        remove_object_from_map(w);
        other = &thing[w->group];
        if (other > thing) {
            for (; i < 20; i++) {
                if (build->objects[i] == 0) {
                    build->objects[i] = other - thing;
                    break;
                }
            }
            remove_object_from_map(other);
            if (objects[other->subtype].unknown17 != -1) {
                w = other;
            }
        }
        room = build->room;
    } else {
        if (w->unknown98 == 1 && w->unknown91) {
            staff = &thing[w->unknown91];
            if (staff->renderer == 4 && staff->subtype == 3) {
                set_complex(staff, 0x1b8, 0);
                staff->state_b = get_state(0, staff);
                abandon_complex(staff);
                anim = check_frame(staff_anim_starts[staff->subtype].idle, staff) + (u16)staff->unknown17;
                staff->sprite = start[anim].frame;
                staff->palette = start[anim].flags > 0;
            } else {
                abandon_complex(staff);
                staff->icon = 0;
            }
        }
        remove_object_from_map(w);
        if (D_001553c7 == player) {
            gamevar.unknown26 = 2;
            crequester.unknown8 = 3;
        }
    }
    if (D_001553c7 == player) {
        gamevar.unknown33.unknown0[w->subtype]++;
        gamevar.f28 = w->unknown17;
        init_place_object(w->subtype, gamevar.unknown26, room, w->x.tile + (w->y.tile << 7) - D_001553e0);
        D_001553fb = 0;
    }
}

// FUNCTION: 0x0002812c
void clear_build_objects(u8 owner)
{
    Thing *w;
    BuildState *build;
    u8 i;
    u8 has;

    build = &allhosps[owner].build;
    for (i = 0; i < 20; i++) {
        has = build->objects[i];
        if (has) {
            w = &thing[build->objects[i]];
            if (statvar.unknownfd > 1 && w->subtype == 0x11) {
                continue;
            }
            change_balance(3, objects[w->subtype].cost * depreciation() / 100, 0, owner);
            memset(w, 0, sizeof(Thing));
            build->objects[i] = 0;
        }
    }
    if (owner == player) {
        for (i = 0; i < 0x3e; i++) {
            if (statvar.unknownfd > 1 && i == 0x11) {
                continue;
            }
            gamevar.unknown33.unknown0[i] = 0;
        }
    }
}

// NONMATCHING: frame layout (local slot order)
// FUNCTION: 0x00028288
u16 place_object_wall_hug(u16 pos, u8 kind)
{
    RoomInfo *room;
    u8 x;
    u8 y;
    u8 rx;
    u8 ry;
    u8 corner;
    u8 rx2;
    u8 ry2;
    s8 dx;
    s8 dy;
    u8 below;
    u8 below2;
    u8 side;
    u8 sx;
    u8 sy;
    u8 angle;
    Bounds *b;
    u16 part_kind;
    u8 part_angle;
    u32 part_pos;
    s8 ox;
    s8 oy;

    room = &room_info[gamevar.unknown2f];
    x = pos % 128;
    y = pos / 128;
    rx = room->pos % 128;
    ry = room->pos / 128;
    corner = wall_hug[gamevar.f27].kind == 4;
    rx2 = rx + room->f1;
    ry2 = ry + room->f2;
    dx = x - rx;
    dy = y - ry;
    below = dy < room->f2 * dx / room->f1;
    below2 = dy < room->f2 - dx * room->f2 / room->f1;
    if ((below && below2) || (!corner && below)) {
        side = 0;
        sx = x;
        sy = ry;
    } else if ((below2 && !below) || (!corner && !below)) {
        side = 3;
        sx = rx;
        sy = y;
    } else if (below && !below2) {
        side = 1;
        sx = rx2;
        sy = y;
    } else {
        side = 2;
        sx = x;
        sy = ry2;
    }
    angle = wall_hug[gamevar.f27].angle[side];
    if (angle != gamevar.f28)
        play_sound(10, 0x22, -1, -1);
    gamevar.f28 = angle;
    b = &D_00155f4f[kind][angle];
    if (sx + b->minX < rx) {
        sx = rx - b->minX;
    }
    if (sy + b->minY < ry) {
        sy = ry - b->minY;
    }
    if (sx + b->maxX >= rx2) {
        sx = rx2 - b->maxX - 1;
    }
    if (sy + b->maxY >= ry2) {
        sy = ry2 - b->maxY - 1;
    }
    if (objects[kind].unknown17 != -1) {
        part_kind = next_part[objects[kind].unknown17].kind;
        part_angle = next_part[objects[kind].unknown17].parts[angle].angle;
        part_pos = next_part[objects[kind].unknown17].parts[angle].pos;
        b = &D_00155f4f[part_kind][part_angle];
        convert_MW_to_xy(next_part[objects[kind].unknown17].parts[angle].pos, &ox, &oy);
        if (sx + b->minX + ox < rx) {
            sx = rx - b->minX - ox;
        }
        if (sy + b->minY + oy < ry) {
            sy = ry - b->minY - oy;
        }
        if (sx + b->maxX + ox >= rx2) {
            sx = rx2 - b->maxX - 1 - ox;
        }
        if (sy + b->maxY + oy >= ry2) {
            sy = ry2 - b->maxY - 1 - oy;
        }
    }
    return (s16)(sx + (sy << 7));
}

// FUNCTION: 0x00028710
s32 depreciation(void)
{
    return 100;
}

// FUNCTION: 0x00028734
s32 room_depreciation(void)
{
    return 50;
}

// FUNCTION: 0x00028758
void clicked_for_edit(void)
{
    u8 edit;
    u16 room;

    room = allhosps[player].build.edit_room;
    if (D_001553fb) {
        edit = crequester.unknown2 == 5 || (crequester.unknown2 == 10 && crequester.unknown4 != 1);
        if (edit) {
            if (thing[D_001553fb].unknown89 != room)
                return;
        }
        D_001549d0[player].arg = edit;
        if (thing[D_001553fb].renderer == 6) {
            D_001549d0[player].value = D_001553fb;
        } else {
            D_001549d0[player].value = thing[D_001553fb].unknown69;
        }
        D_001549d0[player].type = 0x44;
    } else if (room) {
        D_001549d0[player].type = 0x46;
        D_001549d0[player].value = room;
        set_mouse_mode(0);
    }
}

// FUNCTION: 0x000288ac
void check_object_right_click_in_room_edit(void)
{
    Thing *w;

    w = &thing[D_001553f9];
    if (w > thing && w->renderer == 6 && w->unknown89 == allhosps[player].unknown11b7) {
        D_001549d0[player].type = 0x44;
        D_001549d0[player].value = D_001553f9;
        D_001549d0[player].arg = 1;
    }
}

// FUNCTION: 0x00028950
void init_object_drag_shape(void)
{
    D_00155d7e = 0xff;
    D_00155d7f = 0xff;
    D_00155d80 = 0;
    D_00155d81 = 0;
}

#define LINKED_OBJECT(w) ((w)->unknown69 && (thing[(w)->unknown69].renderer == 6 || thing[(w)->unknown69].renderer == 8) && (u16)((w) - thing) == thing[(w)->unknown69].unknown91 ? (w)->unknown69 : 0)

#define DRAG_MIN(a, b) ((a) > (b) ? ((a) = (b)) : 0)
#define DRAG_MAX(a, b) ((a) < (b) ? ((a) = (b)) : 0)

// FUNCTION: 0x00028984
void compute_object_drag_shape(u16 pos, u8 kind, u8 angle)
{
    Shapes *shape;
    Map *cell;
    int i;
    u8 x;
    u8 y;

    shape = &shapes[objects[kind].shapes[angle]];
    cell = &map[pos];
    for (i = shape->count - 1; i >= 0; i--) {
        x = (pos + (s16)shape->points[i]) % 128;
        y = (pos + (s16)shape->points[i]) / 128;
        DRAG_MIN(D_00155d7e, x);
        DRAG_MIN(D_00155d7f, y);
        DRAG_MAX(D_00155d80, x);
        DRAG_MAX(D_00155d81, y);
        if (map[pos + (s16)shape->points[i]].flags & 7) {
            map[pos + (s16)shape->points[i]].flags |= 0x80;
        } else {
            map[pos + (s16)shape->points[i]].flags |= 0x40;
        }
    }
    for (i = 2; i >= 0; i--) {
        x = (pos + shape->unknown13[i]) % 128;
        y = (pos + shape->unknown13[i]) / 128;
        DRAG_MIN(D_00155d7e, x);
        DRAG_MIN(D_00155d7f, y);
        DRAG_MAX(D_00155d80, x);
        DRAG_MAX(D_00155d81, y);
        if (map[pos + shape->unknown13[i]].flags & 7) {
            map[pos + shape->unknown13[i]].flags |= 0x80;
        } else {
            map[pos + shape->unknown13[i]].flags |= 0x40;
        }
        x = (pos + shape->unknown19[i]) % 128;
        y = (pos + shape->unknown19[i]) / 128;
        DRAG_MIN(D_00155d7e, x);
        DRAG_MIN(D_00155d7f, y);
        DRAG_MAX(D_00155d80, x);
        DRAG_MAX(D_00155d81, y);
        if (map[pos + shape->unknown19[i]].flags & 7) {
            map[pos + shape->unknown19[i]].flags |= 0x80;
        } else {
            map[pos + shape->unknown19[i]].flags |= 0x40;
        }
    }
}

// FUNCTION: 0x00028e08
void clear_object_drag_shape(void)
{
    int x;
    int y;

    for (x = D_00155d7e; x <= D_00155d80; x++) {
        for (y = D_00155d7f; y <= D_00155d81; y++) {
            map[(s16)x + ((s16)y << 7)].flags &= 0x3f;
        }
    }
    init_object_drag_shape();
}

// FUNCTION: 0x00028e80
void pixel_buffer_map(void)
{
    Map *m;
    int x;
    int y;

    m = map + 129;
    for (y = 125; y >= 0; y--) {
        for (x = 125; x >= 0; x--, m++) {
            m->flags &= ~0x10;
            if ((m->unknown3 && m->unknown3 < 0x8e) || m->unknown3 > 0x91) {
                m[-129].flags |= 0x10;
                m[-128].flags |= 0x10;
                m[-127].flags |= 0x10;
            }
            if ((m->unknown4 && m->unknown4 < 0x8e) || m->unknown4 > 0x91) {
                m[-129].flags |= 0x10;
                m[-1].flags |= 0x10;
                m[127].flags |= 0x10;
            }
        }
        m += 2;
    }
}

// FUNCTION: 0x00028f88
u8 corridor_object_nav_check(u16 pos, u8 kind, u8 dir)
{
    u8 ok;
    u8 i;
    u8 side;
    u8 count;
    Map *cell;
    Shapes *shape;
    u16 entrance;
    u16 tile;
    Map *other;
    s16 id;
    Thing *w;
    Shapes *other_shape;

    cell = &map[pos];
    shape = &shapes[objects[kind].shapes[dir]];
    entrance = room_info[myhosp->unknown1cf[0]].unknown4;
    ok = 1;
    side = 1;
    count = 0x18;
    if (kind == 0xb) {
        side = 0;
        count = 0x18;
    }
    mark_object_shape(cell, shape, dir, &objects[kind], 0);
    if (!(kind == 0x2d || kind == 0x11 || check_points_connected(entrance, pos + shape->unknown13[side]))) {
        ok = 0;
    } else {
        for (i = 0; i < count; i++) {
            tile = pos + around24[i];
            if (which_room(tile) == 0) {
                other = &map[tile];
                id = other->first;
                if (id > 0) {
                    w = &thing[id];
                    if (w->renderer == 6) {
                        if (w->subtype == 0xb) {
                            side = 0;
                        }
                        if (!(w->subtype == 0x2d || w->subtype == 0x11)) {
                            side = 1;
                            other_shape = &shapes[objects[w->subtype].shapes[w->unknown17]];
                            if (check_points_connected(entrance, tile + other_shape->unknown13[side]) == 0) {
                                ok = 0;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    remove_object_shape(cell, shape, dir, &objects[kind]);
    return ok;
}

// FUNCTION: 0x000291e8
u8 does_room_contain_compulsory_objects(RoomInfo *room)
{
    Thing *w;
    int i;
    int found;
    int kind;

    for (i = rooms[room->type].fc - 1; i >= 0; i--) {
        found = 0;
        kind = rooms[room->type].objects[i];
        for (w = &thing[room->first_object]; w > thing; w = &thing[w->unknown71]) {
            if (w->subtype == kind) {
                found = 1;
                break;
            }
        }
        if (found == 0) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: 0x000292d0
u8 corridor_object_room_nav_check(u16 pos, u8 kind, u8 dir, RoomInfo *room)
{
    u16 pos2;
    Map *cell;
    Shapes *shape;
    Thing *w;
    Thing *other;
    u8 ok;

    cell = &map[pos];
    shape = &shapes[objects[kind].shapes[dir]];
    ok = 1;
    if (!place_object_nav_check(room, pos, kind, dir, 0, 0, 0)) {
        return 0;
    }
    mark_object_shape(cell, shape, dir, &objects[kind], 0);
    for (w = &thing[room->first]; w > thing; w = &thing[w->room_next]) {
        other = &thing[LINKED_OBJECT(w)];
        if (other > thing) {
            if (w->renderer == 4) {
                pos2 = other->use_pos[3];
            } else {
                pos2 = other->use_pos[4];
            }
        } else {
            pos2 = (u16)w->x.tile + ((u16)w->y.tile << 7);
        }
        if (!check_points_connected(pos2, room->unknown6)) {
            ok = 0;
            break;
        }
    }
    remove_object_shape(cell, shape, dir, &objects[kind]);
    return ok;
}

// FUNCTION: 0x00029520
void pick_up_object_right_click(void)
{
    Thing *w;

    w = &thing[D_001553f9];
    if (w->renderer == 2 && LINKED_OBJECT(w)) {
        w = &thing[LINKED_OBJECT(w)];
    }
    if (w->renderer == 6 && w->owner == player && (!w->unknown89 || (objects[w->subtype].unknown20[2] & 8)) && !(objects[w->subtype].unknown20[2] & 1)) {
        D_001553fb = D_001553f9;
        clicked_for_edit();
    }
}

// FUNCTION: 0x00029748
extern "C" u8 fn_00029748(void)
{
    Map *m;
    int i;
    int r;

    if (allhosps[player].build.unknownE[0] == 1) {
        return 1;
    }
    for (m = map, i = 0; m < end_map; m++, i++) {
        r = find_small_thing(i);
        if (r != 0) {
            if (r >= 1 && r <= 3) {
                return 1;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x000297e4
void repair_invis_walls_around_room(RoomInfo *room)
{
    Thing *w;
    int x;
    int y;
    int x0;
    int y0;
    int x1;
    int y1;

    x0 = room->pos % 128 - 1;
    y0 = room->pos / 128 - 1;
    x1 = room->pos % 128 + room->f1;
    y1 = room->pos / 128 + room->f2;
    for (y = y0; y <= y1; ++y) {
        for (x = x0; x <= x1; ++x) {
            if (x == x0 || x == x1 || y == y0 || y == y1) {
                for (w = &thing[map[(s16)x + ((s16)y << 7)].first]; w > thing; w = &thing[w->next]) {
                    if (w->renderer == 6) {
                        check_invis_walls_around_object(w);
                    }
                }
            }
        }
    }
}

// FUNCTION: 0x0002994c
extern "C" void fn_0002994c(u8 owner)
{
    RoomInfo *room;
    BuildState *build;
    u8 x;
    u8 y;

    room = &room_info[allhosps[owner].unknown11b7];
    build = &allhosps[owner].build;
    if (room > room_info) {
        room->f22 = 0;
        for (x = build->unknown0[0]; x <= build->unknown0[2] + 1; x++) {
            for (y = build->unknown0[1]; y <= build->unknown0[3] + 1; y++) {
                place_small_thing((s16)(x + (y << 7)), 0, 0);
            }
        }
        D_001553c8 = 1;
        allhosps[owner].unknown11b7 = 0;
    }
}

// FUNCTION: 0x00029a54
extern "C" u8 fn_00029a54(RoomInfo *room)
{
    Thing *w;

    if (room->first) {
        return 0;
    }
    w = fn_00019aac(&map[room->unknown4], 8);
    if (w > thing) {
        if (w->unknown91) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: 0x0007d2a8
extern "C" void fn_0007d2a8(u8 owner, u16 tile)
{
    RatHole *first;
    RatHole *last;
    RatHole *hole;

    first = allhosps[owner].fcaf;
    last = allhosps[owner].fcaf + (allhosps[owner].fcae - 1);
    for (hole = first; hole < last; hole++) {
        if (tile == hole->tile) {
            remove_rathole(owner, hole - first);
            break;
        }
    }
}
