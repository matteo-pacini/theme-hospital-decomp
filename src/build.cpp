#include "hosp.h"

extern FillInWindow fill_in_windows[];

extern RoomNavControl room_nav[];

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

extern s16 around24[];

extern "C" {

extern Event D_001549d0[];

s32 fn_00038d04(Thing *w, u8 owner);
#pragma aux fn_00038d04 "W?remove_object$n(pn$Thing$$)s"
void fn_00024a34(u16 tile, u16 room, u8 side);
#pragma aux fn_00024a34 "W?clear_cell_of_ratholes$n(ucus)v"
void fn_0007d2a8(u8 owner, u16 tile);
#pragma aux fn_0007d2a8 "W?delete_wall$n(ususuc)v"
u8 fn_00029a54(RoomInfo *room);
Thing *fn_00019aac(Map *t, u16 kind);

}

void give_screen_xy_of_cell_at_column_row(const u8 view, const s32 column, const s32 row);
u8 can_place_door(u16 tile, u16 a, u8 b);
u8 is_valid_door(u16 tile, u8 owner, u8 type);
s16 place_door(u16 tile, u16 kind);
s8 search_for_outer_color(Map *cell);
void remove_blank_wall(Map *cell, u8 side);
s16 find_bluep_who(Map *cell);
s8 get_wall_color(Map *cell);
void recolour_all_rooms(void);
u16 which_room_number(u16 tile);
void add_static_shadows(void);
void recolour_inside_room(RoomInfo *room, u16 wall_base, u16 b);
void recolour_outside_room(RoomInfo *room, u16 wall_base, u16 b);
void fill_in_window(u16 x, u16 y, u8 side);
void pixel_buffer_map(void);
void mark_object_shape(Map *cell, Shapes *shape, u8 dir, ObjectControl *obj, u8 mark);
void remove_object_shape(Map *cell, Shapes *shape, u8 dir, ObjectControl *obj);
void mark_blueprint_on_map(s16 x0, s16 y0, s16 x1, s16 y1, u8 mode);
void clear_pager_responses(u16 room);
u8 search_fill_in_window(u8 wall);
void check_inside_w_wall(u16 x, u16 y);
void check_inside_n_wall(u16 x, u16 y);
void check_outside_s_wall(u16 x, u16 y);
void check_outside_e_wall(u16 x, u16 y);
void remove_object_from_map(Thing *w);
s32 depreciation(void);
s16 get_state(const u16 a, const Thing *w);
u8 place_object_nav_check(RoomInfo *room, u16 pos, u8 kind, u8 dir, u16 a, u8 b, u8 c);
void check_invis_walls_around_object(Thing *w);

// FUNCTION: 0x00022b90
void temp_place_door(u16 pos, u8 owner)
{
    u8 blocked;
    u8 end_x;
    u8 end_y;
    u8 start_x;
    u8 type;
    u8 check_valid;
    u8 start_y;
    u8 min_w;
    u8 min_h;
    BuildState *build;
    RoomInfo *room;
    Map *cell0;
    s16 i;
    s16 best;
    s16 j;
    s16 tile;
    s16 d;
    s16 best_tile;
    s16 cx;
    s16 cy;

    build = &allhosps[owner].build;
    blocked = 0;
    othervar.f18[owner] = 0;
    room = &room_info[build->room];
    switch (build->door_state) {
    case 0:
        min_w = 3;
        min_h = 3;
        end_x = 3;
        end_y = 3;
        start_x = 1;
        start_y = 1;
        check_valid = 1;
        break;
    case 4:
        min_w = 3;
        min_h = 3;
        end_x = 2;
        end_y = 2;
        start_x = 2;
        start_y = 2;
        check_valid = 0;
        break;
    case 3:
        min_w = 3;
        min_h = 3;
        end_x = 1;
        end_y = 1;
        start_x = 1;
        start_y = 1;
        check_valid = 0;
        break;
    default:
        min_w = 3;
        min_h = 3;
        end_x = 2;
        end_y = 2;
        start_x = 1;
        start_y = 1;
        check_valid = 1;
        break;
    }
    cell0 = &map[room->pos];
    if (room->f2 > min_h) {
        for (i = start_y, best = 9999; i <= room->f2 - end_y; i++) {
            for (j = 0; j <= room->f1; j += room->f1) {
                tile = &cell0[j + (i << 7)] - map;
                if (build->door_state || (-map[tile + 128].first & 3) == 2) {
                    d = fn_000cb2db((s16)pos % 128 - tile % 128) + fn_000cb2db((s16)pos / 128 - tile / 128);
                    if (d < best) {
                        best = d;
                        best_tile = tile;
                        if (j == 0) {
                            type = 2;
                        } else {
                            type = 3;
                        }
                    }
                }
            }
        }
    }
    if (room->f1 > min_w) {
        for (i = start_x; i <= room->f1 - end_x; i++) {
            for (j = 0; j <= room->f2; j += room->f2) {
                tile = &cell0[i + (j << 7)] - map;
                if (build->door_state || (-map[tile - 1].first & 3) == 1) {
                    d = fn_000cb2db((s16)pos % 128 - tile % 128) + fn_000cb2db((s16)pos / 128 - tile / 128);
                    if (d < best) {
                        best = d;
                        best_tile = tile;
                        if (j == 0) {
                            type = 0;
                        } else {
                            type = 1;
                        }
                    }
                }
            }
        }
    }
    if (best_tile == room->pos) {
        if (room->pos % 128 == pos % 128) {
            type = 2;
        }
    }
    if (best == 9999) {
        return;
    }
    type += build->door_state << 2;
    for (j = 0; j < doortype[type].count; j++) {
        cx = (best_tile + (s16)doortype[type].tiles[j] - doortype[type].offset) % 128;
        cy = (best_tile + (s16)doortype[type].tiles[j] - doortype[type].offset) / 128;
        give_screen_xy_of_cell_at_column_row(window.unknown2eb, cx, cy);
        d = info.unknown54;
        i = info.unknown58;
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
        LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
        if (owner == player) {
            LbSpriteDraw(d, i - blocks[doortype[type].overlays[j]].info->height, blocks[doortype[type].overlays[j]].info);
        }
        cx = (best_tile + (s16)doortype[type].tiles[j]) % 128;
        cy = (best_tile + (s16)doortype[type].tiles[j]) / 128;
        if (build->door_state != 2) {
            blocked = !can_place_door((cy << 7) + cx, 3, blueprint[(s16)doortype[type % 4 + 4].sprites[0]].unknown4 - 1) ? (u8)1 : (u8)0;
        } else {
            int kind;

            kind = blueprint[find_small_thing((cy << 7) + cx)].unknown7;
            blocked = kind == 3 || kind == 0x34 || kind == 0x35 ? (u8)1 : (u8)0;
        }
        give_screen_xy_of_cell_at_column_row(window.unknown2eb, cx, cy - 1);
        d = info.unknown54;
        i = info.unknown58;
        if (check_valid || blocked) {
            if (blocked || !is_valid_door((cy << 7) + cx, owner, type)) {
                if (owner == player) {
                    draw_a_one_colour_frame(start[doortype[type].tiles[3]].frame, d, i, (u16)color.unknownD, start[doortype[type].tiles[3]].flags);
                }
                othervar.f18[owner] = 1;
            } else if (owner == player) {
                draw_a_frame(start[doortype[type].tiles[3]].frame, d, i, 0, start[doortype[type].tiles[3]].flags);
            }
        } else if (owner == player) {
            draw_a_frame(start[doortype[type].tiles[3]].frame, d, i, 0, start[doortype[type].tiles[3]].flags);
        }
    }
    build->door_tile = best_tile;
    build->door_type = type;
}

// FUNCTION: 0x0002341c
void place_bp_door(u8 owner, u8 swap)
{
    RoomInfo *room;
    BuildState *bs;
    s32 delta;
    int type;
    u16 i;
    u16 tile;
    Thing *t;
    u16 id;
    u8 piece;

    room = &room_info[allhosps[owner].build.room];
    bs = &allhosps[owner].build;
    delta = 1;
    if (othervar.f18[owner]) {
        if (othervar.ff == player) {
            play_sound(10, SFX_WRONG2, -1, -1);
        }
        return;
    }
    if (doortype[bs->door_type].f1b) {
        room->unknown4 = bs->door_tile + doortype[bs->door_type].door1;
        room->unknown6 = bs->door_tile + doortype[bs->door_type].door2;
    }
    if (!check_room_access_with_blueprint(owner)) {
        if (owner == player) {
            advisor.f2 = 6;
            play_sound(10, SFX_WRONG2, -1, -1);
        }
        return;
    }
    type = bs->door_type;
    if (swap) {
        if (blueprint[find_small_thing(bs->door_tile + doortype[type].tiles[0])].kind == 0x7e && bs->door_state == 2) {
            type %= 4;
            type += 0x14;
            delta = -1;
            play_sound(10, SFX_DE_BUILD, -1, -1);
        } else {
            return;
        }
    } else if (owner == player) {
        if (bs->door_state == 2 && blueprint[find_small_thing(bs->door_tile + doortype[type].tiles[0])].kind == 0x7e) {
            play_sound(10, SFX_WRONG2, -1, -1);
        } else {
            play_sound(10, SFX_BUILDCLK, -1, -1);
        }
    }
    for (i = 0; i < doortype[type].count; i++) {
        tile = bs->door_tile + doortype[type].tiles[i];
        if (map[tile].first > 0) {
            t = &thing[map[tile].first];
            if (t > thing) {
                for (; t->cell_next > 0; t = &thing[t->cell_next]) {
                }
            }
            id = -t->cell_next;
        } else {
            id = -map[tile].first;
        }
        piece = doortype[type].sprites[i];
        if ((blueprint[id].flags & 3) == 3) {
            piece += 6;
        }
        place_small_thing(tile, (u16)piece, 0);
        map[tile - doortype[type].offset].overlay = doortype[type].overlays[i];
    }
    othervar.f10 = 1;
    if (bs->door_state == 2) {
        room->f1b += delta;
    }
    if (bs->door_state == 1 || bs->door_state == 0) {
        if (othervar.ff == player) {
            set_edit_room_requester_mode(4);
        }
        bs->door_state = 2;
    }
}

// FUNCTION: 0x0002384c
RoomInfo *which_room(u16 tile)
{
    RoomInfo *r;
    s16 pos;
    s16 x;
    s16 y;
    s16 x0;
    s16 y0;

    x = tile % 128;
    y = tile / 128;
    for (r = room_info + 1; r < end_room_info; r++) {
        if (r->type != ROOM_FREE) {
            pos = r->pos;
            x0 = pos % 128;
            if (x0 > x || x0 + r->f1 - 1 < x) {
                continue;
            }
            y0 = pos / 128;
            if (y0 > y || y0 + r->f2 - 1 < y) {
                continue;
            }
            return r;
        }
    }
    return 0;
}

// FUNCTION: 0x00023950
u16 which_room_number(u16 tile)
{
    RoomInfo *r;
    s16 pos;
    s16 x;
    s16 y;
    s16 x0;
    s16 y0;
    u16 n;

    x = tile % 128;
    y = tile / 128;
    n = 1;
    for (r = room_info + 1; r < end_room_info; r++) {
        if (r->type != ROOM_FREE) {
            pos = r->pos;
            x0 = pos % 128;
            if (x0 > x || x0 + r->f1 - 1 < x) {
                goto next;
            }
            y0 = pos / 128;
            if (y0 > y || y0 + r->f2 - 1 < y) {
                goto next;
            }
            return n;
        }
    next:
        n++;
    }
    return 0;
}

// FUNCTION: 0x00023a64
RoomInfo *is_this_a_door(u16 tile)
{
    RoomInfo *r;

    for (r = room_info + 1; r < end_room_info; r++) {
        if (tile == r->unknown4) {
            return r;
        }
        if (tile == r->unknown6) {
            return r;
        }
    }
    return 0;
}

// FUNCTION: 0x00023ad4
s16 find_bluep_who(Map *cell)
{
    s16 id;
    Thing *t;

    id = cell->first;
    if (id > 0) {
        for (t = &thing[cell->first]; t->cell_next > 0; t = &thing[t->cell_next]) {
        }
        id = t->cell_next;
    }
    if (id < -0x4b) {
        return 0;
    }
    return id;
}

// FUNCTION: 0x00023b64
void build_all_rooms(void)
{
    u8 mask;
    RoomInfo *room;
    u8 saved;
    u16 bp;
    u8 walls[2];
    s8 col;
    s16 id;
    u16 tile;
    Thing *t;
    u8 flag;
    Map *cell;

    color.unknown4 = -1;
    color.unknown6 = -1;
    color.unknown5 = -1;
    if (statvar.unknownfd > 1) {
        switch (othervar.ff) {
        case 0:
            mask = 1;
            break;
        case 1:
            mask = 2;
            break;
        case 2:
            mask = 4;
            break;
        case 3:
            mask = 8;
            break;
        }
    } else {
        mask = 1;
    }
    for (cell = map; cell < end_map; cell++) {
        if (cell->unknown7 == mask) {
            if (find_bluep_who(cell) < 0) {
                if (cell->overlay == 0x17 || cell->overlay >= 0x20) {
                } else {
                    search_for_outer_color(cell);
                    color.unknown6 = 7;
                    if (color.unknown6 >= 0) {
                        break;
                    }
                }
            }
        }
    }
    if (color.unknown6 < 0) {
        color.unknown4 = color.unknown5;
        if (color.unknown4 < 0) {
            color.unknown4 = 7;
        }
    } else {
        color.unknown4 = color.unknown6;
    }
    for (cell = map; cell < end_map; cell++) {
        if (cell->unknown7 & mask) {
            id = find_bluep_who(cell);
            if (id < 0) {
                bp = -id;
                place_small_thing(cell - map, 0, 0);
                col = color.unknown4 * 2;
                walls[0] = walls[1] = 0;
                if (blocks[cell->unknown3].width & 0x10) {
                    walls[0] = 1;
                }
                if (blocks[cell->unknown4].width & 0x10) {
                    walls[1] = 1;
                }
                room = which_room(cell - map);
                if (room) {
                    room->owner = othervar.ff;
                }
            again:
                switch (blueprint[bp].unknown4) {
                case 0:
                    if (col >= 0) {
                        tile = cell - map;
                        if (blueprint[bp].flags < 3) {
                            (cell - 0x81)->flags |= 0x10;
                        }
                        if ((blueprint[bp].flags & 1) && walls[0] == 0) {
                            flag = 1;
                            if (cell->first) {
                                t = &thing[cell->first];
                                if (t->type == 3) {
                                    if (t->facing == 0 || t->facing == 2) {
                                        flag = 0;
                                    }
                                }
                            }
                            if (flag) {
                                switch (cell->unknown3) {
                                case 0x7e:
                                case 0x80:
                                case 0x82:
                                case 0x84:
                                case 0x86:
                                case 0x88:
                                case 0x8a:
                                    break;
                                case 0x7f:
                                case 0x81:
                                case 0x83:
                                case 0x85:
                                case 0x87:
                                case 0x89:
                                default:
                                    cell->unknown3 = blueprint[bp].kind + col;
                                    break;
                                }
                            }
                        }
                        if ((blueprint[bp].flags & 2) && walls[1] == 0) {
                            flag = 1;
                            if (cell->first) {
                                t = &thing[cell->first];
                                if (t->type == 3) {
                                    if (t->facing == 1 || t->facing == 3) {
                                        flag = 0;
                                    }
                                }
                            }
                            if (flag) {
                                switch (cell->unknown4) {
                                case 0x7f:
                                case 0x81:
                                case 0x83:
                                case 0x85:
                                case 0x87:
                                case 0x89:
                                case 0x8b:
                                    break;
                                case 0x80:
                                case 0x82:
                                case 0x84:
                                case 0x86:
                                case 0x88:
                                case 0x8a:
                                default:
                                    cell->unknown4 = blueprint[bp].kind + col + 1;
                                    break;
                                }
                            }
                        }
                        if (cell->unknown3 && (cell - 0x7f)->unknown4) {
                            (cell - 0x80)->flags |= 0x10;
                        }
                    }
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                    saved = gamevar.f28;
                    gamevar.f28 = blueprint[bp].unknown4 - 1;
                    if (place_door(cell - map, blueprint[bp].kind)) {
                        remove_blank_wall(cell, gamevar.f28 % 2);
                    } else {
                        place_small_thing(cell - map, gamevar.f28 % 2 + 1, 0);
                        cell--;
                        gamevar.f28 = saved;
                        continue;
                    }
                    gamevar.f28 = saved;
                    (cell - 0x81)->flags |= 0x10;
                    break;
                case 5:
                    if (col >= 0) {
                        if ((blueprint[bp].flags & 1) && walls[0] == 0) {
                            cell->unknown3 = 0x7e;
                        }
                        if ((blueprint[bp].flags & 2) && walls[1] == 0) {
                            cell->unknown4 = 0x7f;
                        }
                        (cell - 0x81)->flags |= 0x10;
                    }
                    break;
                case 10:
                    if (blueprint[bp].flags < 9) {
                        cell->flags |= 1;
                        if (blueprint[bp].flags == 7) {
                            cell->unknown3 = 0x97;
                            cell->unknown4 = 0x98;
                        } else {
                            (&cell->unknown3)[blueprint[bp].f6] = blueprint[bp].kind;
                        }
                    }
                    break;
                case 11:
                case 13:
                    saved = gamevar.f28;
                    gamevar.f28 = 0;
                    place_door(cell - map, (u16)blueprint[bp].kind);
                    gamevar.f28 = saved;
                    break;
                case 12:
                case 14:
                    saved = gamevar.f28;
                    gamevar.f28 = 1;
                    place_door(cell - map, (u16)blueprint[bp].kind);
                    gamevar.f28 = saved;
                    break;
                case 15:
                case 16:
                case 17:
                case 18:
                    saved = bp - 0x20;
                    switch (saved) {
                    case 0:
                    case 2:
                        place_small_thing(cell - map, 0, 0);
                        place_small_thing(cell + 1 - map, 0, 0);
                        fn_00037098(cell - map, (u16)blueprint[bp].kind, saved, othervar.ff);
                        break;
                    case 1:
                    case 3:
                        place_small_thing(cell - map, 0, 0);
                        place_small_thing(cell + 0x80 - map, 0, 0);
                        fn_00037098(cell - map, (u16)blueprint[bp].kind, saved, othervar.ff);
                        break;
                    }
                    break;
                case 21:
                    cell->unknown3 = blueprint[bp].kind;
                    break;
                case 22:
                    cell->unknown4 = blueprint[bp].kind;
                    break;
                case 23:
                case 25:
                    cell->unknown3 = 0x60;
                    bp = blueprint[bp].kind;
                    goto again;
                case 24:
                case 26:
                    cell->unknown4 = 0xc4;
                    bp = blueprint[bp].kind;
                    goto again;
                default:
                    break;
                }
            }
        } else if (cell->unknown7 & (u8)(mask | 0x10)) {
            bp = -id;
            place_small_thing(cell - map, 0, 0);
        }
    }
    for (cell = map; cell < end_map; cell++) {
        if (cell->unknown7 & mask) {
            if (blocks[cell->overlay].width == 0x20) {
                cell->overlay = 0x13;
            }
        }
    }
    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->f3 < 0) {
            room->f3 = 0;
        }
    }
    othervar.f10 = 1;
}

// FUNCTION: 0x000245fc
s8 search_for_outer_color(Map *cell)
{
    Map *m;
    s8 col;

    col = -1;
    if (find_bluep_who(cell) == 0) {
        cell += -1;
    }
    if ((-find_bluep_who(cell) & 3) == 3) {
        if (cell->unknown3) {
            col = get_wall_color(cell);
        } else if (cell->unknown4) {
            col = get_wall_color(cell);
        }
        if (cell->overlay == 0xf) {
            color.unknown5 = col;
            col = -1;
        } else {
            color.unknown6 = col;
        }
    } else if ((-find_bluep_who(cell) & 2) && color.unknown5 == -1) {
        m = cell - 0x80;
        if (m->unknown4) {
            color.unknown5 = get_wall_color(m);
        }
    } else if ((-find_bluep_who(cell) & 1) && color.unknown5 == -1) {
        m = cell - 1;
        if (m->unknown3) {
            color.unknown5 = get_wall_color(m);
        }
    }
    return 1;
}

// FUNCTION: 0x00024718
s8 get_wall_color(Map *cell)
{
    if (cell->unknown3 && !(cell->unknown3 >= 0x70 && cell->unknown3 <= 0x7c)) {
        return (cell->unknown3 - 0x50) / 2;
    }
    if (cell->unknown4 && !(cell->unknown4 >= 0x70 && cell->unknown4 <= 0x7c)) {
        return (cell->unknown4 - 0x50) / 2;
    }
    return -1;
}

// FUNCTION: 0x000247c8
void object_placed(void)
{
    gamevar.unknown33.unknown0[gamevar.unknown25]--;
    setup_place_object_requester();
    if (gamevar.unknown33.unknown0[gamevar.unknown25] == 0) {
        if (gamevar.unknown71 == 0) {
            set_mouse_mode(MM_NORMAL);
            if (gamevar.unknown26 == 2) {
                close_crequester(5);
            }
            return;
        } else {
            init_place_object(next_object_to_build(), gamevar.unknown26, gamevar.unknown2f, 0);
        }
    } else {
        init_place_object(gamevar.unknown25, gamevar.unknown26, gamevar.unknown2f, 0);
    }
}

// FUNCTION: 0x00024874
void price_all_rooms(u16 type, u8 owner)
{
    RoomInfo *room;
    u16 y;
    u16 x;
    RoomInfo *r;

    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->f3 != 3 || room->owner != owner) {
            continue;
        }
        for (y = room->pos / 128; y < room->pos / 128 + room->f2; y++) {
            for (x = room->pos % 128; x < room->pos % 128 + room->f1; x++) {
                map[(s16)x + ((s16)y << 7)].overlay = rooms[type].unknown22;
            }
        }
        recolour_all_rooms();
        if (room->owner == player) {
            if (othervar.ff == player) {
                init_place_object(next_object_to_build(), 0, room - room_info, 0);
            }
            gamevar.unknown20 = 1;
            for (r = room_info + 1; r != room; r++, gamevar.unknown20++) {
            }
        }
        break;
    }
    othervar.f10 = 1;
}

// FUNCTION: 0x00024a34
void fn_00024a34(u16 tile, u16 room, u8 side)
{
    s16 x;
    s16 y;
    u16 room_before;
    u16 room_after;
    u8 clear;

    x = tile % 128;
    y = tile / 128;
    switch (side) {
    case 0:
        room_before = which_room_number((y - 1) * 128 + x);
        room_after = which_room_number((y + 1) * 128 + x);
        clear = 1;
        if (room_before > 0 && room_before != room) {
            clear = 0;
        }
        if (room_after > 0 && room_after != room) {
            if (which_room_number(y * 128 + x) != 0) {
                clear = 0;
            }
        }
        if (blocks[map[x + (y << 7)].unknown3].width & 0x10) {
            clear = 0;
        }
        if (clear) {
            map[x + (y << 7)].unknown3 = 0;
        }
        break;
    case 1:
        room_before = which_room_number(x - 1 + y * 128);
        room_after = which_room_number((x + 1) + y * 128);
        clear = 1;
        if (room_before > 0 && room_before != room) {
            clear = 0;
        }
        if (room_after > 0 && room_after != room) {
            if (which_room_number(y * 128 + x) != 0) {
                clear = 0;
            }
        }
        if (blocks[map[x + (y << 7)].unknown4].width & 0x10) {
            clear = 0;
        }
        if (clear) {
            map[x + (y << 7)].unknown4 = 0;
        }
        break;
    }
}

// FUNCTION: 0x00024c58
void delete_room(RoomInfo *room)
{
    u16 id;
    Thing *obj;
    int x;
    int y;

    id = which_room_number(((s16)(room->pos / 128 + room->f2 / 2) << 7) + (s16)(room->pos % 128 + room->f1 / 2));
    for (y = room->pos / 128; y < room->pos / 128 + room->f2; y++) {
        for (x = room->pos % 128; x < room->pos % 128 + room->f1; x++) {
            for (obj = &thing[map[(s16)x + ((s16)y << 7)].first]; obj > thing; obj = &thing[obj->cell_next]) {
                if (obj->type == THING_OBJECT) {
                    fn_00038d04(obj, othervar.ff);
                }
            }
        }
    }
    for (y = room->pos / 128; y < room->pos / 128 + room->f2 + 1; y++) {
        for (x = room->pos % 128; x < room->pos % 128 + room->f1 + 1; x++) {
            if (x < room->pos % 128 + room->f1) {
                if (map[(s16)x + ((s16)y << 7)].unknown3) {
                    fn_00024a34((y << 7) + x, id, 0);
                }
            }
            if (y < room->pos / 128 + room->f2) {
                if (map[(s16)x + ((s16)y << 7)].unknown4) {
                    fn_00024a34((y << 7) + x, id, 1);
                }
            }
            for (obj = &thing[map[(s16)x + ((s16)y << 7)].first]; obj > thing; obj = &thing[obj->cell_next]) {
                if (obj->type == THING_DOOR || (obj->type == THING_OBJECT && (obj->subtype == OBJ_SWING_DOOR || obj->subtype == OBJ_SWING_DOOR2))) {
                    switch (obj->facing) {
                    case 0:
                    case 2:
                        if (x < room->pos % 128 + room->f1) {
                            fn_00038d04(obj, othervar.ff);
                        }
                        break;
                    case 1:
                    case 3:
                        if (y < room->pos / 128 + room->f2) {
                            fn_00038d04(obj, othervar.ff);
                        }
                        break;
                    }
                }
            }
            fn_0007d2a8(room->owner, (y << 7) + x);
        }
    }
    debug.unknown108 = 0;
    add_static_shadows();
    NRAdcounts.unknown10--;
}

// FUNCTION: 0x000250f8
void recolour_inside_room(RoomInfo *room, u16 wall_base, u16 b)
{
    u8 ok;
    u16 tile;
    u16 x;
    u16 y;
    Thing *t;

    for (x = room->pos % 128; x < room->pos % 128 + room->f1; x++) {
        ok = 1;
        if (map[(s16)x + ((s16)(room->pos / 128) << 7)].first > 0) {
            t = &thing[map[(s16)x + ((s16)(room->pos / 128) << 7)].first];
            if (t->type == 3) {
                ok = 0;
            }
        }
        if (ok) {
            tile = (s16)x + ((s16)(room->pos / 128) << 7);
            switch (map[tile].unknown3) {
            case 0x7e:
            case 0x80:
            case 0x82:
            case 0x84:
            case 0x86:
            case 0x88:
            case 0x8a:
                map[tile].unknown3 = b;
                break;
            case 0x50:
            case 0x52:
            case 0x54:
            case 0x56:
            case 0x58:
            case 0x5a:
            case 0x5c:
            case 0x5e:
                map[tile].unknown3 = wall_base;
                break;
            case 0xc3:
            case 0xc5:
            case 0xc7:
            case 0xc9:
            case 0xcb:
            case 0xcd:
            case 0xcf:
            case 0xd1:
                map[tile].unknown3 = wall_base + 0x73;
                break;
            case 0x60:
            case 0x62:
            case 0x64:
            case 0x66:
            case 0x68:
            case 0x6a:
            case 0x6c:
            case 0x6e:
                map[tile].unknown3 = wall_base + 0x10;
                break;
            }
        }
    }
    for (y = room->pos / 128; y < room->pos / 128 + room->f2; y++) {
        ok = 1;
        if (map[(s16)(room->pos % 128) + ((s16)y << 7)].first > 0) {
            t = &thing[map[(s16)(room->pos % 128) + ((s16)y << 7)].first];
            if (t->type == 3) {
                ok = 0;
            }
        }
        if (ok) {
            tile = (s16)(room->pos % 128) + ((s16)y << 7);
            switch (map[tile].unknown4) {
            case 0x7f:
            case 0x81:
            case 0x83:
            case 0x85:
            case 0x87:
            case 0x89:
            case 0x8b:
                map[tile].unknown4 = b + 1;
                break;
            case 0x51:
            case 0x53:
            case 0x55:
            case 0x57:
            case 0x59:
            case 0x5b:
            case 0x5d:
            case 0x5f:
                map[tile].unknown4 = wall_base + 1;
                break;
            case 0xc4:
            case 0xc6:
            case 0xc8:
            case 0xca:
            case 0xcc:
            case 0xce:
            case 0xd0:
            case 0xd2:
                map[tile].unknown4 = wall_base + 0x74;
                break;
            case 0x61:
            case 0x63:
            case 0x65:
            case 0x67:
            case 0x69:
            case 0x6b:
            case 0x6d:
            case 0x6f:
                map[tile].unknown4 = wall_base + 0x11;
                break;
            }
        }
    }
}

// FUNCTION: 0x0002583c
void recolour_outside_room(RoomInfo *room, u16 wall_base, u16 b)
{
    u8 ok;
    u16 x;
    u16 y;

    for (x = room->pos % 128; x < room->pos % 128 + room->f1; x++) {
        ok = 1;
        if (map[(s16)x + ((s16)(room->pos / 128 + room->f2) << 7)].first) {
            int n = map[(s16)x + ((s16)(room->pos / 128) << 7)].first;
            if (n > 0 && thing[n].type == 3) {
                ok = 0;
            }
        }
        if (ok) {
            switch (map[(s16)x + ((s16)(room->pos / 128 + room->f2) << 7)].unknown3) {
            case 0x7e:
            case 0x80:
            case 0x82:
            case 0x84:
            case 0x86:
            case 0x88:
            case 0x8a:
                map[(s16)x + ((s16)(room->pos / 128 + room->f2) << 7)].unknown3 = b;
                break;
            case 0x50:
            case 0x52:
            case 0x54:
            case 0x56:
            case 0x58:
            case 0x5a:
            case 0x5c:
            case 0x5e:
                map[(s16)x + ((s16)(room->pos / 128 + room->f2) << 7)].unknown3 = wall_base;
                break;
            case 0xc3:
            case 0xc5:
            case 0xc7:
            case 0xc9:
            case 0xcb:
            case 0xcd:
            case 0xcf:
            case 0xd1:
                map[(s16)x + ((s16)(room->pos / 128 + room->f2) << 7)].unknown3 = wall_base + 0x73;
                break;
            case 0x60:
            case 0x62:
            case 0x64:
            case 0x66:
            case 0x68:
            case 0x6a:
            case 0x6c:
            case 0x6e:
                map[(s16)x + ((s16)(room->pos / 128 + room->f2) << 7)].unknown3 = wall_base + 0x10;
                break;
            }
        }
    }
    for (y = room->pos / 128; y < room->pos / 128 + room->f2; y++) {
        ok = 1;
        if (map[((s16)y << 7) + (s16)(room->pos % 128 + room->f1)].first) {
            int n = map[(s16)x + ((s16)(room->pos / 128) << 7)].first;
            if (n > 0 && thing[n].type == 3) {
                ok = 0;
            }
        }
        if (ok) {
            switch (map[((s16)y << 7) + (s16)(room->pos % 128 + room->f1)].unknown4) {
            case 0x7f:
            case 0x81:
            case 0x83:
            case 0x85:
            case 0x87:
            case 0x89:
            case 0x8b:
                map[((s16)y << 7) + (s16)(room->pos % 128 + room->f1)].unknown4 = b + 1;
                break;
            case 0x51:
            case 0x53:
            case 0x55:
            case 0x57:
            case 0x59:
            case 0x5b:
            case 0x5d:
            case 0x5f:
                map[((s16)y << 7) + (s16)(room->pos % 128 + room->f1)].unknown4 = wall_base + 1;
                break;
            case 0xc4:
            case 0xc6:
            case 0xc8:
            case 0xca:
            case 0xcc:
            case 0xce:
            case 0xd0:
            case 0xd2:
                map[((s16)y << 7) + (s16)(room->pos % 128 + room->f1)].unknown4 = wall_base + 0x74;
                break;
            case 0x61:
            case 0x63:
            case 0x65:
            case 0x67:
            case 0x69:
            case 0x6b:
            case 0x6d:
            case 0x6f:
                map[((s16)y << 7) + (s16)(room->pos % 128 + room->f1)].unknown4 = wall_base + 0x11;
                break;
            }
        }
    }
}

// FUNCTION: 0x000260ec
void recolour_all_rooms(void)
{
    u16 inner;
    u16 b;
    u16 outer;
    RoomInfo *room;

    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->type != ROOM_FREE) {
            outer = 0;
            b = 0;
            switch (rooms[room->type].unknown2[0]) {
            case 1:
            case 2:
                outer = 0;
                b = 0;
                break;
            case 3:
                outer = 0x58;
                b = 0x7e;
                break;
            case 4:
                outer = 0x5a;
                b = 0x80;
                break;
            case 5:
                outer = 0x5c;
                b = 0x82;
                break;
            case 6:
                outer = 0x5e;
                b = 0x84;
                break;
            }
            if (b > 0 && outer > 0) {
                recolour_outside_room(room, outer, b);
            }
        }
    }
    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->pos) {
            if (room->type != ROOM_FREE) {
                inner = 0;
                b = 0;
                switch (rooms[room->type].unknown2[0]) {
                case 1:
                case 2:
                    inner = 0;
                    b = 0;
                    break;
                case 3:
                    inner = 0x50;
                    b = 0x7e;
                    break;
                case 4:
                    inner = 0x52;
                    b = 0x80;
                    break;
                case 5:
                    inner = 0x54;
                    b = 0x82;
                    break;
                case 6:
                    inner = 0x56;
                    b = 0x84;
                    break;
                }
                if (inner > 0 && b > 0) {
                    recolour_inside_room(room, inner, b);
                }
                fill_in_window(room->pos % 128, room->pos / 128, 4);
                fill_in_window(room->f1 + room->pos % 128, room->pos / 128, 1);
                fill_in_window(room->pos % 128, room->pos / 128 + room->f2, 2);
                fill_in_window(room->f1 + room->pos % 128, room->pos / 128 + room->f2, 3);
            }
        }
    }
    pixel_buffer_map();
    othervar.f10 = 1;
}

// FUNCTION: 0x00026488
u8 search_fill_in_window(u8 wall)
{
    u8 i;
    u8 fill_block;

    i = 0;
    fill_block = 0;
    while (fill_in_windows[i].wall != 0) {
        if (fill_in_windows[i].wall == wall) {
            fill_block = fill_in_windows[i].window;
            break;
        }
        i++;
    }
    return fill_block;
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

// FUNCTION: 0x000265e4
void check_inside_w_wall(u16 x, u16 y)
{
    u8 fill_block;

    fill_block = search_fill_in_window(map[(s16)x + ((s16)y << 7)].unknown4);
    if (fill_block) {
        if (map[(s16)x + ((s16)y << 7)].unknown4 == 0x7d) {
            map[(s16)x + ((s16)y << 7)].unknown4 = fill_block;
            if (map[(s16)x + ((s16)(y - 1) << 7)].unknown4 == 0x7b) {
                map[(s16)x + ((s16)(y - 1) << 7)].unknown4 = fill_block;
            }
        }
    }
}

// FUNCTION: 0x000266bc
void check_inside_n_wall(u16 x, u16 y)
{
    u8 fill_block;

    fill_block = search_fill_in_window(map[(s16)x + ((s16)y << 7)].unknown3);
    if (fill_block) {
        if (map[(s16)x + ((s16)y << 7)].unknown3 == 0x7c) {
            map[(s16)x + ((s16)y << 7)].unknown3 = fill_block;
            if (map[(s16)(x - 1) + ((s16)y << 7)].unknown3 == 0x7a) {
                map[(s16)(x - 1) + ((s16)y << 7)].unknown3 = fill_block;
            }
        }
    }
}

// FUNCTION: 0x00026798
void check_outside_s_wall(u16 x, u16 y)
{
    u8 fill_block;

    fill_block = search_fill_in_window(map[(s16)(x - 1) + ((s16)y << 7)].unknown3);
    if (fill_block) {
        if (map[(s16)(x - 1) + ((s16)y << 7)].unknown3 == 0x72) {
            map[(s16)(x - 1) + ((s16)y << 7)].unknown3 = fill_block;
            if (map[(s16)x + ((s16)y << 7)].unknown3 == 0x74) {
                map[(s16)x + ((s16)y << 7)].unknown3 = fill_block;
                if (map[(s16)(x + 1) + ((s16)y << 7)].unknown3 == 0x76) {
                    map[(s16)(x + 1) + ((s16)y << 7)].unknown3 = fill_block;
                }
            } else if (map[(s16)x + ((s16)y << 7)].unknown3 == 0x76) {
                map[(s16)x + ((s16)y << 7)].unknown3 = fill_block;
            }
        } else if (map[(s16)(x - 1) + ((s16)y << 7)].unknown3 == 0x74) {
            map[(s16)(x - 1) + ((s16)y << 7)].unknown3 = fill_block;
            if (map[(s16)(x - 2) + ((s16)y << 7)].unknown3 == 0x72) {
                map[(s16)(x - 2) + ((s16)y << 7)].unknown3 = fill_block;
            }
            if (map[(s16)x + ((s16)y << 7)].unknown3 == 0x76) {
                map[(s16)x + ((s16)y << 7)].unknown3 = fill_block;
            }
        }
    }
}

// FUNCTION: 0x000269e8
void check_outside_e_wall(u16 x, u16 y)
{
    u8 fill_block;

    fill_block = search_fill_in_window(map[(s16)x + ((s16)(y - 1) << 7)].unknown4);
    if (fill_block) {
        if (map[(s16)x + ((s16)(y - 1) << 7)].unknown4 == 0x73) {
            map[(s16)x + ((s16)(y - 1) << 7)].unknown4 = fill_block;
            if (map[(s16)x + ((s16)y << 7)].unknown4 == 0x75) {
                map[(s16)x + ((s16)y << 7)].unknown4 = fill_block;
                if (map[(s16)x + ((s16)(y + 1) << 7)].unknown4 == 0x77) {
                    map[(s16)x + ((s16)(y + 1) << 7)].unknown4 = fill_block;
                }
            } else if (map[(s16)x + ((s16)y << 7)].unknown4 == 0x77) {
                map[(s16)x + ((s16)y << 7)].unknown4 = fill_block;
            }
        } else if (map[(s16)x + ((s16)(y - 1) << 7)].unknown3 == 0x75) {
            map[(s16)x + ((s16)(y - 1) << 7)].unknown3 = fill_block;
            if (map[(s16)x + ((s16)(y - 2) << 7)].unknown3 == 0x73) {
                map[(s16)x + ((s16)(y - 2) << 7)].unknown3 = fill_block;
            }
            if (map[(s16)x + ((s16)y << 7)].unknown3 == 0x77) {
                map[(s16)x + ((s16)y << 7)].unknown3 = fill_block;
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
    Thing *obj;
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
    for (obj = &thing[room->first_object]; obj > thing; obj = &thing[obj->room_obj_next]) {
        if (obj->state != 0x12 && !check_object_nav((u16)obj->subtype, &shapes[objects[obj->subtype].shapes[obj->facing]], room, ((u16)obj->y.tile << 7) + (u16)obj->x.tile, (u16)room->type)) {
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
    s8 new_y;
    s8 new_x;
    s8 room_y;
    s8 room_x;
    s8 y;
    s8 x;
    Bounds *bounds;
    RoomInfo *room;

    room = &room_info[gamevar.unknown2f];
    bounds = &statvar.fff[kind][dir];
    x = (pos + offset) % 128;
    y = (pos + offset) / 128;
    room_x = room->pos % 128;
    room_y = room->pos / 128;
    new_x = pos % 128;
    new_y = pos / 128;
    if (x + bounds->maxX >= room_x + room->f1) {
        new_x -= x + bounds->maxX - room_x - room->f1 + 1;
    }
    if (x + bounds->minX < room_x) {
        new_x += room_x - x - bounds->minX;
    }
    if (y + bounds->maxY >= room_y + room->f2) {
        new_y -= y + bounds->maxY - room_y - room->f2 + 1;
    }
    if (y + bounds->minY < room_y) {
        new_y += room_y - y - bounds->minY;
    }
    return new_x + (new_y << 7);
}

// FUNCTION: 0x000273bc
u8 place_object_change_angle(s8 angle)
{
    Shapes *shape;
    s8 i;
    u8 changed;

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
    set_mouse_mode(MM_PLACE_OBJECT);
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
        if (door->facing == 1) {
            other = search_mapwho_for_room_door(&map[room->unknown4 - 128], room - room_info);
        } else {
            other = search_mapwho_for_room_door(&map[room->unknown4 - 1], room - room_info);
        }
    } else {
        door = search_mapwho_for_room_door(&map[room->unknown6], room - room_info);
        if (door > thing) {
            if (door->facing == 0) {
                other = search_mapwho_for_room_door(&map[room->unknown6 - 1], room - room_info);
            } else {
                other = search_mapwho_for_room_door(&map[room->unknown6 - 128], room - room_info);
            }
        }
    }
    other->type = THING_OBJECT;
    door->door_partner = other - thing;
    other->door_main = door - thing;
    if (door->facing == 1 || door->facing == 3) {
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
    if (othervar.ff == player) {
        crequester.unknown8 = room->type;
        gamevar.unknown26 = 0;
    }
    if (fn_00029a54(room)) {
        room->f3 = 4;
        if (othervar.ff == player) {
            setup_place_object_requester();
        }
    } else {
        room->f22 = 1;
        if (othervar.ff == player) {
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
void open_room(u8 owner)
{
    RoomInfo *room;
    BuildState *build;
    u8 x;
    u8 y;

    room = &room_info[allhosps[owner].unknown11b7];
    build = &allhosps[owner].build;
    if (room > room_info) {
        room->f3 = 0;
        clear_pager_responses(room - room_info);
        for (x = build->x0; x <= build->x1 + 1; x++) {
            for (y = build->y0; y <= build->y1 + 1; y++) {
                place_small_thing((s16)(x + (y << 7)), 0, 0);
            }
        }
        clear_build_objects(owner);
        if (room->unknown33 == 0) {
            change_balance(TX_BUILD_ROOM, -rooms[room->type].cost, 0, (u16)owner);
            room->unknown33 = 1;
        }
        if (owner == player && othervar.f9c5 == 0) {
            othervar.fa4b = room->type;
        }
        add_static_shadows();
        othervar.f10 = 1;
        othervar.f9c5 = 0;
        if (othervar.ff == player) {
            set_mouse_mode(MM_NORMAL);
            close_crequester(5);
            if (myplayer.f23[room->type] == 0) {
                show_help_page(0, room->type);
                myplayer.f23[room->type] = 1;
            }
        }
        allhosps[owner].unknown11b7 = 0;
        NRAdcounts.unknown10++;
    }
}

// FUNCTION: 0x00027d48
u16 compute_object_pointed(RoomInfo *room)
{
    Thing *obj;

    obj = &thing[othervar.f41];
    if (obj->owner == player) {
        if (room) {
            if (!(!(obj->type == THING_OBJECT && (u16)(room - room_info) == obj->room_id) || obj->subtype == OBJ_SWING_DOOR || obj->subtype == OBJ_SWING_DOOR2)) {
                return othervar.f41;
            } else {
                return 0;
            }
        } else {
            return othervar.f41;
        }
    } else {
        return 0;
    }
}

// FUNCTION: 0x00027e1c
void pick_up_object(u16 id, u8 both)
{
    Thing *obj;
    u16 room;
    BuildState *build;
    u8 i;
    Thing *other;
    Thing *user;
    u16 anim;

    obj = &thing[id];
    room = 0;
    build = &allhosps[obj->owner].build;
    for (i = 0; i < 20; i++) {
        if (build->objects[i] == 0) {
            build->objects[i] = id;
            break;
        }
    }
    if (both) {
        remove_object_from_map(obj);
        other = &thing[obj->pair];
        if (other > thing) {
            for (; i < 20; i++) {
                if (build->objects[i] == 0) {
                    build->objects[i] = other - thing;
                    break;
                }
            }
            remove_object_from_map(other);
            if (objects[other->subtype].unknown17 != -1) {
                obj = other;
            }
        }
        room = build->room;
    } else {
        if (obj->in_use == 1 && obj->user) {
            user = &thing[obj->user];
            if (user->type == THING_STAFF && user->subtype == STAFF_RECEPTIONIST) {
                set_complex(user, 0x1b8, 0);
                user->next_state = get_state(0, user);
                abandon_complex(user);
                anim = check_frame(staff_anim_starts[user->subtype].idle, user) + (u16)user->facing;
                user->sprite = start[anim].frame;
                user->flip = start[anim].flags > 0;
            } else {
                abandon_complex(user);
                user->complex_action = 0;
            }
        }
        remove_object_from_map(obj);
        if (othervar.ff == player) {
            gamevar.unknown26 = 2;
            crequester.unknown8 = 3;
        }
    }
    if (othervar.ff == player) {
        gamevar.unknown33.unknown0[obj->subtype]++;
        gamevar.f28 = obj->facing;
        init_place_object(obj->subtype, gamevar.unknown26, room, obj->x.tile + (obj->y.tile << 7) - othervar.f28);
        othervar.f43 = 0;
    }
}

// FUNCTION: 0x0002812c
void clear_build_objects(u8 owner)
{
    Thing *obj;
    BuildState *build;
    u8 i;
    u8 has;

    build = &allhosps[owner].build;
    for (i = 0; i < 20; i++) {
        has = build->objects[i];
        if (has) {
            obj = &thing[build->objects[i]];
            if (statvar.unknownfd > 1 && obj->subtype == OBJ_LITTER_BOMB) {
                continue;
            }
            change_balance(TX_BUY_OBJECT, objects[obj->subtype].cost * depreciation() / 100, 0, owner);
            memset(obj, 0, sizeof(Thing));
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

// FUNCTION: 0x00028288
u16 place_object_wall_hug(u16 pos, u8 kind)
{
    RoomInfo *room;
    Bounds *b;
    u8 side;
    u8 below2;
    u8 sx;
    u8 sy;
    u8 x;
    u8 y;
    u8 rx;
    u8 ry;
    u8 rx2;
    u8 ry2;
    u8 below;
    u8 angle;
    u8 corner;
    s8 dx;
    s8 dy;

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
        play_sound(10, SFX_SWOOSH, -1, -1);
    gamevar.f28 = angle;
    b = &statvar.fff[kind][angle];
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
        u16 part_kind;
        u8 part_angle;
        u32 part_pos;
        s8 ox;
        s8 oy;

        part_kind = next_part[objects[kind].unknown17].kind;
        part_angle = next_part[objects[kind].unknown17].parts[angle].angle;
        part_pos = next_part[objects[kind].unknown17].parts[angle].pos;
        b = &statvar.fff[part_kind][part_angle];
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
    if (othervar.f43) {
        edit = crequester.unknown2 == 5 || (crequester.unknown2 == 10 && crequester.unknown4 != 1);
        if (edit) {
            if (thing[othervar.f43].room_id != room)
                return;
        }
        D_001549d0[player].arg = edit;
        if (thing[othervar.f43].type == THING_OBJECT) {
            D_001549d0[player].value = othervar.f43;
        } else {
            D_001549d0[player].value = thing[othervar.f43].object;
        }
        D_001549d0[player].type = EV_PICK_UP_OBJECT;
    } else if (room) {
        D_001549d0[player].type = EV_INIT_EDIT_ROOM;
        D_001549d0[player].value = room;
        set_mouse_mode(MM_NORMAL);
    }
}

// FUNCTION: 0x000288ac
void check_object_right_click_in_room_edit(void)
{
    Thing *obj;

    obj = &thing[othervar.f41];
    if (obj > thing && obj->type == THING_OBJECT && obj->room_id == allhosps[player].unknown11b7) {
        D_001549d0[player].type = EV_PICK_UP_OBJECT;
        D_001549d0[player].value = othervar.f41;
        D_001549d0[player].arg = 1;
    }
}

// FUNCTION: 0x00028950
void init_object_drag_shape(void)
{
    othervar.f9c6 = 0xff;
    othervar.f9c7 = 0xff;
    othervar.f9c8 = 0;
    othervar.f9c9 = 0;
}

#define LINKED_OBJECT(w) ((w)->object && (thing[(w)->object].type == THING_OBJECT || thing[(w)->object].type == THING_DOOR) && (u16)((w) - thing) == thing[(w)->object].user ? (w)->object : 0)

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
        DRAG_MIN(othervar.f9c6, x);
        DRAG_MIN(othervar.f9c7, y);
        DRAG_MAX(othervar.f9c8, x);
        DRAG_MAX(othervar.f9c9, y);
        if (map[pos + (s16)shape->points[i]].flags & 7) {
            map[pos + (s16)shape->points[i]].flags |= 0x80;
        } else {
            map[pos + (s16)shape->points[i]].flags |= 0x40;
        }
    }
    for (i = 2; i >= 0; i--) {
        x = (pos + shape->unknown13[i]) % 128;
        y = (pos + shape->unknown13[i]) / 128;
        DRAG_MIN(othervar.f9c6, x);
        DRAG_MIN(othervar.f9c7, y);
        DRAG_MAX(othervar.f9c8, x);
        DRAG_MAX(othervar.f9c9, y);
        if (map[pos + shape->unknown13[i]].flags & 7) {
            map[pos + shape->unknown13[i]].flags |= 0x80;
        } else {
            map[pos + shape->unknown13[i]].flags |= 0x40;
        }
        x = (pos + shape->unknown19[i]) % 128;
        y = (pos + shape->unknown19[i]) / 128;
        DRAG_MIN(othervar.f9c6, x);
        DRAG_MIN(othervar.f9c7, y);
        DRAG_MAX(othervar.f9c8, x);
        DRAG_MAX(othervar.f9c9, y);
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

    for (x = othervar.f9c6; x <= othervar.f9c8; x++) {
        for (y = othervar.f9c7; y <= othervar.f9c9; y++) {
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
    Thing *obj;
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
                    obj = &thing[id];
                    if (obj->type == THING_OBJECT) {
                        if (obj->subtype == OBJ_RECEPTION_DESK) {
                            side = 0;
                        }
                        if (!(obj->subtype == OBJ_PLANT || obj->subtype == OBJ_LITTER_BOMB)) {
                            side = 1;
                            other_shape = &shapes[objects[obj->subtype].shapes[obj->facing]];
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
    Thing *obj;
    int i;
    int found;
    int kind;

    for (i = rooms[room->type].fc - 1; i >= 0; i--) {
        found = 0;
        kind = rooms[room->type].objects[i];
        for (obj = &thing[room->first_object]; obj > thing; obj = &thing[obj->room_obj_next]) {
            if (obj->subtype == kind) {
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
    Thing *occupant;
    Thing *obj;
    u8 ok;

    cell = &map[pos];
    shape = &shapes[objects[kind].shapes[dir]];
    ok = 1;
    if (!place_object_nav_check(room, pos, kind, dir, 0, 0, 0)) {
        return 0;
    }
    mark_object_shape(cell, shape, dir, &objects[kind], 0);
    for (occupant = &thing[room->first]; occupant > thing; occupant = &thing[occupant->room_next]) {
        obj = &thing[LINKED_OBJECT(occupant)];
        if (obj > thing) {
            if (occupant->type == THING_STAFF) {
                pos2 = obj->use_pos[3];
            } else {
                pos2 = obj->use_pos[4];
            }
        } else {
            pos2 = (u16)occupant->x.tile + ((u16)occupant->y.tile << 7);
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
    Thing *clicked;

    clicked = &thing[othervar.f41];
    if (clicked->type == THING_PATIENT && LINKED_OBJECT(clicked)) {
        clicked = &thing[LINKED_OBJECT(clicked)];
    }
    if (clicked->type == THING_OBJECT && clicked->owner == player && (!clicked->room_id || (objects[clicked->subtype].unknown20[2] & 8)) && !(objects[clicked->subtype].unknown20[2] & 1)) {
        othervar.f43 = othervar.f41;
        clicked_for_edit();
    }
}

// FUNCTION: 0x00029748
extern "C" u8 fn_00029748(void)
{
    Map *m;
    int i;
    int small;

    if (allhosps[player].build.unknownE[0] == 1) {
        return 1;
    }
    for (m = map, i = 0; m < end_map; m++, i++) {
        small = find_small_thing(i);
        if (small != 0) {
            if (small >= 1 && small <= 3) {
                return 1;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x000297e4
void repair_invis_walls_around_room(RoomInfo *room)
{
    Thing *obj;
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
                for (obj = &thing[map[(s16)x + ((s16)y << 7)].first]; obj > thing; obj = &thing[obj->cell_next]) {
                    if (obj->type == THING_OBJECT) {
                        check_invis_walls_around_object(obj);
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
        othervar.f10 = 1;
        allhosps[owner].unknown11b7 = 0;
    }
}

// FUNCTION: 0x00029a54
extern "C" u8 fn_00029a54(RoomInfo *room)
{
    Thing *door;

    if (room->first) {
        return 0;
    }
    door = fn_00019aac(&map[room->unknown4], THING_DOOR);
    if (door > thing) {
        if (door->user) {
            return 0;
        }
    }
    return 1;
}
