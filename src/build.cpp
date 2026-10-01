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

s32 remove_object(Thing *obj, u8 owner);
#pragma aux remove_object "W?remove_object$n(pn$Thing$$)s"
void delete_wall(u16 tile, u16 room, u8 side);
#pragma aux delete_wall "W?delete_wall$n(ususuc)v"
void clear_cell_of_ratholes(u8 owner, u16 tile);
#pragma aux clear_cell_of_ratholes "W?clear_cell_of_ratholes$n(ucus)v"
u8 fn_00029a54(RoomInfo *room);
Thing *search_mapwho(Map *cell, u16 kind);

}

void give_screen_xy_of_cell_at_column_row(const u8 win, const s32 x, const s32 y);
u8 can_place_door(u16 tile, u16 kind, u8 dir);
u8 is_valid_door(u16 tile, u8 owner, u8 type);
s16 place_door(u16 tile, u16 kind);
s8 search_for_outer_color(Map *cell);
void insert_blank_wall(Map *cell, u8 side);
s16 find_bluep_who(Map *cell);
s8 get_wall_color(Map *cell);
void recolour_all_rooms(void);
u16 which_room_number(u16 tile);
void add_static_shadows(void);
void recolour_inside_room(RoomInfo *room, u16 wall_base, u16 b);
void recolour_outside_room(RoomInfo *room, u16 wall_base, u16 b);
void fill_in_window(u16 x, u16 y, u8 side);
void pixel_buffer_map(void);
void mark_object_shape(Map *base_cell, Shapes *shape, u8 dir, ObjectControl *control, u8 cleanse);
void remove_object_shape(Map *cell, Shapes *shape, u8 dir, ObjectControl *control);
void mark_blueprint_on_map(s16 x0, s16 y0, s16 x1, s16 y1, u8 set_overlay);
void clear_pager_responses(u16 room_id);
u8 search_fill_in_window(u8 wall);
void check_inside_w_wall(u16 x, u16 y);
void check_inside_n_wall(u16 x, u16 y);
void check_outside_s_wall(u16 x, u16 y);
void check_outside_e_wall(u16 x, u16 y);
void remove_object_from_map(Thing *obj);
s32 depreciation(void);
s16 get_state(const u16 type, const Thing *staff);
u8 place_object_nav_check(RoomInfo *room, u16 pos, u8 kind, u8 dir, u16 pos2, u8 kind2, u8 dir2);
void check_invis_walls_around_object(Thing *obj);

// Door placing stage: finds the wall tile of owner's blueprint room nearest
// tile pos where the current door kind (build.door_state; 2 is a window) can
// go, draws the preview there for the local player, and records it in
// build.door_tile and build.door_type. Sets othervar.door_invalid[owner] if the door
// cannot be placed there.
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
    othervar.door_invalid[owner] = 0;
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
    if (room->height > min_h) {
        for (i = start_y, best = 9999; i <= room->height - end_y; i++) {
            for (j = 0; j <= room->width; j += room->width) {
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
    if (room->width > min_w) {
        for (i = start_x; i <= room->width - end_x; i++) {
            for (j = 0; j <= room->height; j += room->height) {
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
        give_screen_xy_of_cell_at_column_row(window.current_window, cx, cy);
        d = info.cell_x;
        i = info.cell_y;
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
        LbTextSetWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
        if (owner == player) {
            LbSpriteDraw(d, i - blocks[doortype[type].overlays[j]].info->height, blocks[doortype[type].overlays[j]].info);
        }
        cx = (best_tile + (s16)doortype[type].tiles[j]) % 128;
        cy = (best_tile + (s16)doortype[type].tiles[j]) / 128;
        if (build->door_state != 2) {
            blocked = !can_place_door((cy << 7) + cx, 3, blueprint[(s16)doortype[type % 4 + 4].sprites[0]].build_type - 1) ? (u8)1 : (u8)0;
        } else {
            int kind;

            kind = blueprint[find_small_thing((cy << 7) + cx)].object_kind;
            blocked = kind == 3 || kind == 0x34 || kind == 0x35 ? (u8)1 : (u8)0;
        }
        give_screen_xy_of_cell_at_column_row(window.current_window, cx, cy - 1);
        d = info.cell_x;
        i = info.cell_y;
        if (check_valid || blocked) {
            if (blocked || !is_valid_door((cy << 7) + cx, owner, type)) {
                if (owner == player) {
                    draw_a_one_colour_frame(start[doortype[type].cursor_anim].frame, d, i, (u16)color.red, start[doortype[type].cursor_anim].flags);
                }
                othervar.door_invalid[owner] = 1;
            } else if (owner == player) {
                draw_a_frame(start[doortype[type].cursor_anim].frame, d, i, 0, start[doortype[type].cursor_anim].flags);
            }
        } else if (owner == player) {
            draw_a_frame(start[doortype[type].cursor_anim].frame, d, i, 0, start[doortype[type].cursor_anim].flags);
        }
    }
    build->door_tile = best_tile;
    build->door_type = type;
}

// Places the door or window previewed by temp_place_door for owner. With swap
// nonzero, removes the window there instead. Fails (with a sound for the local
// player) if the preview was invalid or the door would cut rooms off from the
// entrance. After the door, moves the build on to placing windows.
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
    if (othervar.door_invalid[owner]) {
        if (othervar.cur_player == player) {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        return;
    }
    if (doortype[bs->door_type].is_entrance) {
        room->door_tile = bs->door_tile + doortype[bs->door_type].door1;
        room->door_inside_tile = bs->door_tile + doortype[bs->door_type].door2;
    }
    if (!check_room_access_with_blueprint(owner)) {
        if (owner == player) {
            advisor.forced_advice_id = 6;
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        return;
    }
    type = bs->door_type;
    if (swap) {
        if (blueprint[find_small_thing(bs->door_tile + doortype[type].tiles[0])].kind == 0x7e && bs->door_state == 2) {
            type %= 4;
            type += 0x14;
            delta = -1;
            play_sound(SNDSRC_GENERAL, SFX_DE_BUILD, -1, -1);
        } else {
            return;
        }
    } else if (owner == player) {
        if (bs->door_state == 2 && blueprint[find_small_thing(bs->door_tile + doortype[type].tiles[0])].kind == 0x7e) {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        } else {
            play_sound(SNDSRC_GENERAL, SFX_BUILDCLK, -1, -1);
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
    othervar.full_redraw = 1;
    if (bs->door_state == 2) {
        room->windows += delta;
    }
    if (bs->door_state == 1 || bs->door_state == 0) {
        if (othervar.cur_player == player) {
            set_edit_room_requester_mode(4);
        }
        bs->door_state = 2;
    }
}

// Returns the room whose floor contains tile, or 0 for a corridor tile.
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
            if (x0 > x || x0 + r->width - 1 < x) {
                continue;
            }
            y0 = pos / 128;
            if (y0 > y || y0 + r->height - 1 < y) {
                continue;
            }
            return r;
        }
    }
    return 0;
}

// As which_room, but returns the room's room_info index (0 if none).
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
            if (x0 > x || x0 + r->width - 1 < x) {
                goto next;
            }
            y0 = pos / 128;
            if (y0 > y || y0 + r->height - 1 < y) {
                goto next;
            }
            return n;
        }
    next:
        n++;
    }
    return 0;
}

// Returns the room with a door tile (unknown4 or unknown6) at tile, or 0.
// FUNCTION: 0x00023a64
RoomInfo *is_this_a_door(u16 tile)
{
    RoomInfo *r;

    for (r = room_info + 1; r < end_room_info; r++) {
        if (tile == r->door_tile) {
            return r;
        }
        if (tile == r->door_inside_tile) {
            return r;
        }
    }
    return 0;
}

// Returns the blueprint small thing on cell (a negative id), or 0 if there is
// none.
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

// Turns the blueprint pieces on the current player's land (othervar.cur_player) into
// real walls, doors, windows and objects, in a wall colour matched to the
// surroundings, then resets any negative room f3 to 0.
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

    color.wall_colour = -1;
    color.corner_wall_colour = -1;
    color.side_wall_colour = -1;
    if (statvar.num_players > 1) {
        switch (othervar.cur_player) {
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
        if (cell->owner_flags == mask) {
            if (find_bluep_who(cell) < 0) {
                if (cell->overlay == 0x17 || cell->overlay >= 0x20) {
                } else {
                    search_for_outer_color(cell);
                    color.corner_wall_colour = 7;
                    if (color.corner_wall_colour >= 0) {
                        break;
                    }
                }
            }
        }
    }
    if (color.corner_wall_colour < 0) {
        color.wall_colour = color.side_wall_colour;
        if (color.wall_colour < 0) {
            color.wall_colour = 7;
        }
    } else {
        color.wall_colour = color.corner_wall_colour;
    }
    for (cell = map; cell < end_map; cell++) {
        if (cell->owner_flags & mask) {
            id = find_bluep_who(cell);
            if (id < 0) {
                bp = -id;
                place_small_thing(cell - map, 0, 0);
                col = color.wall_colour * 2;
                walls[0] = walls[1] = 0;
                if (blocks[cell->wall_n].width & 0x10) {
                    walls[0] = 1;
                }
                if (blocks[cell->wall_w].width & 0x10) {
                    walls[1] = 1;
                }
                room = which_room(cell - map);
                if (room) {
                    room->owner = othervar.cur_player;
                }
            again:
                switch (blueprint[bp].build_type) {
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
                                switch (cell->wall_n) {
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
                                    cell->wall_n = blueprint[bp].kind + col;
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
                                switch (cell->wall_w) {
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
                                    cell->wall_w = blueprint[bp].kind + col + 1;
                                    break;
                                }
                            }
                        }
                        if (cell->wall_n && (cell - 0x7f)->wall_w) {
                            (cell - 0x80)->flags |= 0x10;
                        }
                    }
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                    saved = gamevar.place_dir;
                    gamevar.place_dir = blueprint[bp].build_type - 1;
                    if (place_door(cell - map, blueprint[bp].kind)) {
                        insert_blank_wall(cell, gamevar.place_dir % 2);
                    } else {
                        place_small_thing(cell - map, gamevar.place_dir % 2 + 1, 0);
                        cell--;
                        gamevar.place_dir = saved;
                        continue;
                    }
                    gamevar.place_dir = saved;
                    (cell - 0x81)->flags |= 0x10;
                    break;
                case 5:
                    if (col >= 0) {
                        if ((blueprint[bp].flags & 1) && walls[0] == 0) {
                            cell->wall_n = 0x7e;
                        }
                        if ((blueprint[bp].flags & 2) && walls[1] == 0) {
                            cell->wall_w = 0x7f;
                        }
                        (cell - 0x81)->flags |= 0x10;
                    }
                    break;
                case 10:
                    if (blueprint[bp].flags < 9) {
                        cell->flags |= 1;
                        if (blueprint[bp].flags == 7) {
                            cell->wall_n = 0x97;
                            cell->wall_w = 0x98;
                        } else {
                            (&cell->wall_n)[blueprint[bp].wall_layer] = blueprint[bp].kind;
                        }
                    }
                    break;
                case 11:
                case 13:
                    saved = gamevar.place_dir;
                    gamevar.place_dir = 0;
                    place_door(cell - map, (u16)blueprint[bp].kind);
                    gamevar.place_dir = saved;
                    break;
                case 12:
                case 14:
                    saved = gamevar.place_dir;
                    gamevar.place_dir = 1;
                    place_door(cell - map, (u16)blueprint[bp].kind);
                    gamevar.place_dir = saved;
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
                        place_object(cell - map, (u16)blueprint[bp].kind, saved, othervar.cur_player);
                        break;
                    case 1:
                    case 3:
                        place_small_thing(cell - map, 0, 0);
                        place_small_thing(cell + 0x80 - map, 0, 0);
                        place_object(cell - map, (u16)blueprint[bp].kind, saved, othervar.cur_player);
                        break;
                    }
                    break;
                case 21:
                    cell->wall_n = blueprint[bp].kind;
                    break;
                case 22:
                    cell->wall_w = blueprint[bp].kind;
                    break;
                case 23:
                case 25:
                    cell->wall_n = 0x60;
                    bp = blueprint[bp].kind;
                    goto again;
                case 24:
                case 26:
                    cell->wall_w = 0xc4;
                    bp = blueprint[bp].kind;
                    goto again;
                default:
                    break;
                }
            }
        } else if (cell->owner_flags & (u8)(mask | MAP_OUTDOOR)) {
            bp = -id;
            place_small_thing(cell - map, 0, 0);
        }
    }
    for (cell = map; cell < end_map; cell++) {
        if (cell->owner_flags & mask) {
            if (blocks[cell->overlay].width == 0x20) {
                cell->overlay = 0x13;
            }
        }
    }
    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->state < 0) {
            room->state = 0;
        }
    }
    othervar.full_redraw = 1;
}

// Looks for the wall colour next to blueprint cell cell, storing it in
// color.corner_wall_colour (or color.side_wall_colour for a corridor wall). Always returns 1.
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
        if (cell->wall_n) {
            col = get_wall_color(cell);
        } else if (cell->wall_w) {
            col = get_wall_color(cell);
        }
        if (cell->overlay == 0xf) {
            color.side_wall_colour = col;
            col = -1;
        } else {
            color.corner_wall_colour = col;
        }
    } else if ((-find_bluep_who(cell) & 2) && color.side_wall_colour == -1) {
        m = cell - 0x80;
        if (m->wall_w) {
            color.side_wall_colour = get_wall_color(m);
        }
    } else if ((-find_bluep_who(cell) & 1) && color.side_wall_colour == -1) {
        m = cell - 1;
        if (m->wall_n) {
            color.side_wall_colour = get_wall_color(m);
        }
    }
    return 1;
}

// Returns the colour index of cell's north or west wall block, or -1 if it has
// none (or only plain pieces 0x70-0x7c).
// FUNCTION: 0x00024718
s8 get_wall_color(Map *cell)
{
    if (cell->wall_n && !(cell->wall_n >= 0x70 && cell->wall_n <= 0x7c)) {
        return (cell->wall_n - 0x50) / 2;
    }
    if (cell->wall_w && !(cell->wall_w >= 0x70 && cell->wall_w <= 0x7c)) {
        return (cell->wall_w - 0x50) / 2;
    }
    return -1;
}

// Called after an object of kind gamevar.place_kind is placed: uses one up from
// stock and starts placing the next one, or ends object placing (closing the
// requester when moving objects) when none are left.
// FUNCTION: 0x000247c8
void object_placed(void)
{
    gamevar.buy_list.to_place[gamevar.place_kind]--;
    setup_place_object_requester();
    if (gamevar.buy_list.to_place[gamevar.place_kind] == 0) {
        if (gamevar.buy_kind_count == 0) {
            set_mouse_mode(MM_NORMAL);
            if (gamevar.place_mode == 2) {
                close_crequester(REQ_EDIT_ROOM);
            }
            return;
        } else {
            init_place_object(next_object_to_build(), gamevar.place_mode, gamevar.place_room, 0);
        }
    } else {
        init_place_object(gamevar.place_kind, gamevar.place_mode, gamevar.place_room, 0);
    }
}

// Gives owner's room under construction the floor of room type type and its
// wall colours; for the local player starts placing its objects and sets
// gamevar.new_room_index to the room's index.
// FUNCTION: 0x00024874
void price_all_rooms(u16 type, u8 owner)
{
    RoomInfo *room;
    u16 y;
    u16 x;
    RoomInfo *r;

    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->state != RS_BLUEPRINT || room->owner != owner) {
            continue;
        }
        for (y = room->pos / 128; y < room->pos / 128 + room->height; y++) {
            for (x = room->pos % 128; x < room->pos % 128 + room->width; x++) {
                map[(s16)x + ((s16)y << 7)].overlay = rooms[type].overlay;
            }
        }
        recolour_all_rooms();
        if (room->owner == player) {
            if (othervar.cur_player == player) {
                init_place_object(next_object_to_build(), 0, room - room_info, 0);
            }
            gamevar.new_room_index = 1;
            for (r = room_info + 1; r != room; r++, gamevar.new_room_index++) {
            }
        }
        break;
    }
    othervar.full_redraw = 1;
}

// Removes the north (side 0) or west (side 1) wall of tile, unless it also
// bounds another room than room (a room_info index) or its block has flag
// 0x10.
// FUNCTION: 0x00024a34
void delete_wall(u16 tile, u16 room, u8 side)
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
        if (blocks[map[x + (y << 7)].wall_n].width & 0x10) {
            clear = 0;
        }
        if (clear) {
            map[x + (y << 7)].wall_n = 0;
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
        if (blocks[map[x + (y << 7)].wall_w].width & 0x10) {
            clear = 0;
        }
        if (clear) {
            map[x + (y << 7)].wall_w = 0;
        }
        break;
    }
}

// Removes room from the map: its objects, the walls it does not share, its
// doors and the rat holes around it. The room_info entry itself is left.
// FUNCTION: 0x00024c58
void delete_room(RoomInfo *room)
{
    u16 id;
    Thing *obj;
    int x;
    int y;

    id = which_room_number(((s16)(room->pos / 128 + room->height / 2) << 7) + (s16)(room->pos % 128 + room->width / 2));
    for (y = room->pos / 128; y < room->pos / 128 + room->height; y++) {
        for (x = room->pos % 128; x < room->pos % 128 + room->width; x++) {
            for (obj = &thing[map[(s16)x + ((s16)y << 7)].first]; obj > thing; obj = &thing[obj->cell_next]) {
                if (obj->type == THING_OBJECT) {
                    remove_object(obj, othervar.cur_player);
                }
            }
        }
    }
    for (y = room->pos / 128; y < room->pos / 128 + room->height + 1; y++) {
        for (x = room->pos % 128; x < room->pos % 128 + room->width + 1; x++) {
            if (x < room->pos % 128 + room->width) {
                if (map[(s16)x + ((s16)y << 7)].wall_n) {
                    delete_wall((y << 7) + x, id, 0);
                }
            }
            if (y < room->pos / 128 + room->height) {
                if (map[(s16)x + ((s16)y << 7)].wall_w) {
                    delete_wall((y << 7) + x, id, 1);
                }
            }
            for (obj = &thing[map[(s16)x + ((s16)y << 7)].first]; obj > thing; obj = &thing[obj->cell_next]) {
                if (obj->type == THING_DOOR || (obj->type == THING_OBJECT && (obj->subtype == OBJ_SWING_DOOR || obj->subtype == OBJ_SWING_DOOR2))) {
                    switch (obj->facing) {
                    case 0:
                    case 2:
                        if (x < room->pos % 128 + room->width) {
                            remove_object(obj, othervar.cur_player);
                        }
                        break;
                    case 1:
                    case 3:
                        if (y < room->pos / 128 + room->height) {
                            remove_object(obj, othervar.cur_player);
                        }
                        break;
                    }
                }
            }
            clear_cell_of_ratholes(room->owner, (y << 7) + x);
        }
    }
    debug.unknown108 = 0;
    add_static_shadows();
    NRAdcounts.open_rooms--;
}

// Recolours room's north and west walls (seen from inside) to wall colour
// wall_base, using b for the 0x7e-0x8b pieces.
// FUNCTION: 0x000250f8
void recolour_inside_room(RoomInfo *room, u16 wall_base, u16 b)
{
    u8 ok;
    u16 tile;
    u16 x;
    u16 y;
    Thing *t;

    for (x = room->pos % 128; x < room->pos % 128 + room->width; x++) {
        ok = 1;
        if (map[(s16)x + ((s16)(room->pos / 128) << 7)].first > 0) {
            t = &thing[map[(s16)x + ((s16)(room->pos / 128) << 7)].first];
            if (t->type == 3) {
                ok = 0;
            }
        }
        if (ok) {
            tile = (s16)x + ((s16)(room->pos / 128) << 7);
            switch (map[tile].wall_n) {
            case 0x7e:
            case 0x80:
            case 0x82:
            case 0x84:
            case 0x86:
            case 0x88:
            case 0x8a:
                map[tile].wall_n = b;
                break;
            case 0x50:
            case 0x52:
            case 0x54:
            case 0x56:
            case 0x58:
            case 0x5a:
            case 0x5c:
            case 0x5e:
                map[tile].wall_n = wall_base;
                break;
            case 0xc3:
            case 0xc5:
            case 0xc7:
            case 0xc9:
            case 0xcb:
            case 0xcd:
            case 0xcf:
            case 0xd1:
                map[tile].wall_n = wall_base + 0x73;
                break;
            case 0x60:
            case 0x62:
            case 0x64:
            case 0x66:
            case 0x68:
            case 0x6a:
            case 0x6c:
            case 0x6e:
                map[tile].wall_n = wall_base + 0x10;
                break;
            }
        }
    }
    for (y = room->pos / 128; y < room->pos / 128 + room->height; y++) {
        ok = 1;
        if (map[(s16)(room->pos % 128) + ((s16)y << 7)].first > 0) {
            t = &thing[map[(s16)(room->pos % 128) + ((s16)y << 7)].first];
            if (t->type == 3) {
                ok = 0;
            }
        }
        if (ok) {
            tile = (s16)(room->pos % 128) + ((s16)y << 7);
            switch (map[tile].wall_w) {
            case 0x7f:
            case 0x81:
            case 0x83:
            case 0x85:
            case 0x87:
            case 0x89:
            case 0x8b:
                map[tile].wall_w = b + 1;
                break;
            case 0x51:
            case 0x53:
            case 0x55:
            case 0x57:
            case 0x59:
            case 0x5b:
            case 0x5d:
            case 0x5f:
                map[tile].wall_w = wall_base + 1;
                break;
            case 0xc4:
            case 0xc6:
            case 0xc8:
            case 0xca:
            case 0xcc:
            case 0xce:
            case 0xd0:
            case 0xd2:
                map[tile].wall_w = wall_base + 0x74;
                break;
            case 0x61:
            case 0x63:
            case 0x65:
            case 0x67:
            case 0x69:
            case 0x6b:
            case 0x6d:
            case 0x6f:
                map[tile].wall_w = wall_base + 0x11;
                break;
            }
        }
    }
}

// Recolours room's south and east walls (seen from outside) to wall colour
// wall_base, using b for the 0x7e-0x8b pieces. Its skip test (a type 3 thing on
// the tile) reads the room's top row, and a stale x for the east wall.
// FUNCTION: 0x0002583c
void recolour_outside_room(RoomInfo *room, u16 wall_base, u16 b)
{
    u8 ok;
    u16 x;
    u16 y;

    for (x = room->pos % 128; x < room->pos % 128 + room->width; x++) {
        ok = 1;
        if (map[(s16)x + ((s16)(room->pos / 128 + room->height) << 7)].first) {
            int n = map[(s16)x + ((s16)(room->pos / 128) << 7)].first;
            if (n > 0 && thing[n].type == 3) {
                ok = 0;
            }
        }
        if (ok) {
            switch (map[(s16)x + ((s16)(room->pos / 128 + room->height) << 7)].wall_n) {
            case 0x7e:
            case 0x80:
            case 0x82:
            case 0x84:
            case 0x86:
            case 0x88:
            case 0x8a:
                map[(s16)x + ((s16)(room->pos / 128 + room->height) << 7)].wall_n = b;
                break;
            case 0x50:
            case 0x52:
            case 0x54:
            case 0x56:
            case 0x58:
            case 0x5a:
            case 0x5c:
            case 0x5e:
                map[(s16)x + ((s16)(room->pos / 128 + room->height) << 7)].wall_n = wall_base;
                break;
            case 0xc3:
            case 0xc5:
            case 0xc7:
            case 0xc9:
            case 0xcb:
            case 0xcd:
            case 0xcf:
            case 0xd1:
                map[(s16)x + ((s16)(room->pos / 128 + room->height) << 7)].wall_n = wall_base + 0x73;
                break;
            case 0x60:
            case 0x62:
            case 0x64:
            case 0x66:
            case 0x68:
            case 0x6a:
            case 0x6c:
            case 0x6e:
                map[(s16)x + ((s16)(room->pos / 128 + room->height) << 7)].wall_n = wall_base + 0x10;
                break;
            }
        }
    }
    for (y = room->pos / 128; y < room->pos / 128 + room->height; y++) {
        ok = 1;
        if (map[((s16)y << 7) + (s16)(room->pos % 128 + room->width)].first) {
            int n = map[(s16)x + ((s16)(room->pos / 128) << 7)].first;
            if (n > 0 && thing[n].type == 3) {
                ok = 0;
            }
        }
        if (ok) {
            switch (map[((s16)y << 7) + (s16)(room->pos % 128 + room->width)].wall_w) {
            case 0x7f:
            case 0x81:
            case 0x83:
            case 0x85:
            case 0x87:
            case 0x89:
            case 0x8b:
                map[((s16)y << 7) + (s16)(room->pos % 128 + room->width)].wall_w = b + 1;
                break;
            case 0x51:
            case 0x53:
            case 0x55:
            case 0x57:
            case 0x59:
            case 0x5b:
            case 0x5d:
            case 0x5f:
                map[((s16)y << 7) + (s16)(room->pos % 128 + room->width)].wall_w = wall_base + 1;
                break;
            case 0xc4:
            case 0xc6:
            case 0xc8:
            case 0xca:
            case 0xcc:
            case 0xce:
            case 0xd0:
            case 0xd2:
                map[((s16)y << 7) + (s16)(room->pos % 128 + room->width)].wall_w = wall_base + 0x74;
                break;
            case 0x61:
            case 0x63:
            case 0x65:
            case 0x67:
            case 0x69:
            case 0x6b:
            case 0x6d:
            case 0x6f:
                map[((s16)y << 7) + (s16)(room->pos % 128 + room->width)].wall_w = wall_base + 0x11;
                break;
            }
        }
    }
}

// Repaints every room's walls in the style of its type (rooms[].wall_style)
// and fills in the window corners, then refreshes the wall flags.
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
            switch (rooms[room->type].wall_style) {
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
                switch (rooms[room->type].wall_style) {
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
                fill_in_window(room->width + room->pos % 128, room->pos / 128, 1);
                fill_in_window(room->pos % 128, room->pos / 128 + room->height, 2);
                fill_in_window(room->width + room->pos % 128, room->pos / 128 + room->height, 3);
            }
        }
    }
    pixel_buffer_map();
    othervar.full_redraw = 1;
}

// Returns the filled-in window block for wall block wall (fill_in_windows), or
// 0 if it has none.
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

// Replaces the window pieces meeting at room corner x,y with their filled-in
// blocks: side 1 top right, 2 bottom left, 3 bottom right, 4 top left.
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

// Fills in a window piece on the west wall at x,y (and the one above it).
// FUNCTION: 0x000265e4
void check_inside_w_wall(u16 x, u16 y)
{
    u8 fill_block;

    fill_block = search_fill_in_window(map[(s16)x + ((s16)y << 7)].wall_w);
    if (fill_block) {
        if (map[(s16)x + ((s16)y << 7)].wall_w == 0x7d) {
            map[(s16)x + ((s16)y << 7)].wall_w = fill_block;
            if (map[(s16)x + ((s16)(y - 1) << 7)].wall_w == 0x7b) {
                map[(s16)x + ((s16)(y - 1) << 7)].wall_w = fill_block;
            }
        }
    }
}

// Fills in a window piece on the north wall at x,y (and the one left of it).
// FUNCTION: 0x000266bc
void check_inside_n_wall(u16 x, u16 y)
{
    u8 fill_block;

    fill_block = search_fill_in_window(map[(s16)x + ((s16)y << 7)].wall_n);
    if (fill_block) {
        if (map[(s16)x + ((s16)y << 7)].wall_n == 0x7c) {
            map[(s16)x + ((s16)y << 7)].wall_n = fill_block;
            if (map[(s16)(x - 1) + ((s16)y << 7)].wall_n == 0x7a) {
                map[(s16)(x - 1) + ((s16)y << 7)].wall_n = fill_block;
            }
        }
    }
}

// Fills in the window pieces along the wall left of corner x,y.
// FUNCTION: 0x00026798
void check_outside_s_wall(u16 x, u16 y)
{
    u8 fill_block;

    fill_block = search_fill_in_window(map[(s16)(x - 1) + ((s16)y << 7)].wall_n);
    if (fill_block) {
        if (map[(s16)(x - 1) + ((s16)y << 7)].wall_n == 0x72) {
            map[(s16)(x - 1) + ((s16)y << 7)].wall_n = fill_block;
            if (map[(s16)x + ((s16)y << 7)].wall_n == 0x74) {
                map[(s16)x + ((s16)y << 7)].wall_n = fill_block;
                if (map[(s16)(x + 1) + ((s16)y << 7)].wall_n == 0x76) {
                    map[(s16)(x + 1) + ((s16)y << 7)].wall_n = fill_block;
                }
            } else if (map[(s16)x + ((s16)y << 7)].wall_n == 0x76) {
                map[(s16)x + ((s16)y << 7)].wall_n = fill_block;
            }
        } else if (map[(s16)(x - 1) + ((s16)y << 7)].wall_n == 0x74) {
            map[(s16)(x - 1) + ((s16)y << 7)].wall_n = fill_block;
            if (map[(s16)(x - 2) + ((s16)y << 7)].wall_n == 0x72) {
                map[(s16)(x - 2) + ((s16)y << 7)].wall_n = fill_block;
            }
            if (map[(s16)x + ((s16)y << 7)].wall_n == 0x76) {
                map[(s16)x + ((s16)y << 7)].wall_n = fill_block;
            }
        }
    }
}

// Fills in the window pieces along the wall above corner x,y.
// FUNCTION: 0x000269e8
void check_outside_e_wall(u16 x, u16 y)
{
    u8 fill_block;

    fill_block = search_fill_in_window(map[(s16)x + ((s16)(y - 1) << 7)].wall_w);
    if (fill_block) {
        if (map[(s16)x + ((s16)(y - 1) << 7)].wall_w == 0x73) {
            map[(s16)x + ((s16)(y - 1) << 7)].wall_w = fill_block;
            if (map[(s16)x + ((s16)y << 7)].wall_w == 0x75) {
                map[(s16)x + ((s16)y << 7)].wall_w = fill_block;
                if (map[(s16)x + ((s16)(y + 1) << 7)].wall_w == 0x77) {
                    map[(s16)x + ((s16)(y + 1) << 7)].wall_w = fill_block;
                }
            } else if (map[(s16)x + ((s16)y << 7)].wall_w == 0x77) {
                map[(s16)x + ((s16)y << 7)].wall_w = fill_block;
            }
        } else if (map[(s16)x + ((s16)(y - 1) << 7)].wall_n == 0x75) {
            map[(s16)x + ((s16)(y - 1) << 7)].wall_n = fill_block;
            if (map[(s16)x + ((s16)(y - 2) << 7)].wall_n == 0x73) {
                map[(s16)x + ((s16)(y - 2) << 7)].wall_n = fill_block;
            }
            if (map[(s16)x + ((s16)y << 7)].wall_n == 0x77) {
                map[(s16)x + ((s16)y << 7)].wall_n = fill_block;
            }
        }
    }
}

// Adds an invisible wall on cell's north (side 0) or west (side 1) edge, if it
// has no wall there; nests twice.
// FUNCTION: 0x00026c28
void insert_blank_wall(Map *cell, u8 side)
{
    if ((&cell->wall_n)[side] == 0) {
        (&cell->wall_n)[side] = side + 0x8e;
    } else if ((&cell->wall_n)[side] == side + 0x8e) {
        (&cell->wall_n)[side] = side + 0x90;
    }
}

// Removes one level of insert_blank_wall's invisible wall from cell's side.
// FUNCTION: 0x00026c98
void remove_blank_wall(Map *cell, u8 side)
{
    if ((&cell->wall_n)[side] == side + 0x8e) {
        (&cell->wall_n)[side] = 0;
    } else if ((&cell->wall_n)[side] == side + 0x90) {
        (&cell->wall_n)[side] = side + 0x8e;
    }
}

// Returns nonzero if tile pos, and tile other unless the same, can reach
// target.
// FUNCTION: 0x00026d08
u8 check_object_accessible(u16 pos, u16 other, u16 target)
{
    if (pos == other) {
        return check_points_connected(pos, target);
    }
    return check_points_connected(pos, target) && check_points_connected(other, target);
}

// For an object of kind object that room type type needs to stay usable
// (room_nav), returns 0 if its use tiles, for shape at pos, cannot reach the
// room's door (room->door_inside_tile); else 1.
// FUNCTION: 0x00026d84
u8 check_object_nav(u16 object, Shapes *shape, RoomInfo *room, u16 pos, u16 type)
{
    RoomNavControl *nav;
    u8 i;

    nav = &room_nav[type];
    for (i = 0; i < nav->door_count; i++) {
        if (object == nav->door_objects[i]) {
            if (!check_object_accessible(pos + shape->points[10], pos + shape->points[13], room->door_inside_tile)) {
                return 0;
            }
            break;
        }
    }
    for (i = 0; i < nav->count; i++) {
        if (object == nav->objects[i]) {
            if (!check_object_accessible(pos + shape->points[9], pos + shape->points[12], room->door_inside_tile)) {
                return 0;
            }
            if (!check_object_accessible(pos + shape->points[11], pos + shape->points[14], room->door_inside_tile)) {
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Returns 1 if placing an object of kind at pos facing dir (and its second
// part kind2 at pos2, dir2, unless pos2 is 0) in room leaves every object in
// the room usable; else 0. The map is left unchanged.
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
        if (obj->state != OS_REMOVED && !check_object_nav((u16)obj->subtype, &shapes[objects[obj->subtype].shapes[obj->facing]], room, ((u16)obj->y.tile << 7) + (u16)obj->x.tile, (u16)room->type)) {
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

// Returns tile pos moved so that an object of kind facing dir, tested at
// pos + offset, lies inside the room being furnished (gamevar.place_room).
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

    room = &room_info[gamevar.place_room];
    bounds = &statvar.object_bounds[kind][dir];
    x = (pos + offset) % 128;
    y = (pos + offset) / 128;
    room_x = room->pos % 128;
    room_y = room->pos / 128;
    new_x = pos % 128;
    new_y = pos / 128;
    if (x + bounds->maxX >= room_x + room->width) {
        new_x -= x + bounds->maxX - room_x - room->width + 1;
    }
    if (x + bounds->minX < room_x) {
        new_x += room_x - x - bounds->minX;
    }
    if (y + bounds->maxY >= room_y + room->height) {
        new_y -= y + bounds->maxY - room_y - room->height + 1;
    }
    if (y + bounds->minY < room_y) {
        new_y += room_y - y - bounds->minY;
    }
    return new_x + (new_y << 7);
}

// Sets the placing angle (gamevar.place_dir) to angle, or to the next angle the
// object has a shape for if angle is -1 (0 if it has none for angle), and
// recomputes its tile extent (gamevar.place_max_x to f2e). Returns 1 if the angle
// changed.
// FUNCTION: 0x000273bc
u8 place_object_change_angle(s8 angle)
{
    Shapes *shape;
    s8 i;
    u8 changed;

    if (angle == -1) {
        angle = next_angle[gamevar.place_dir];
        while (objects[gamevar.place_kind].shapes[angle] == 0) {
            angle = next_angle[angle];
        }
    } else if (!objects[gamevar.place_kind].shapes[angle]) {
        angle = 0;
    }
    changed = gamevar.place_dir != angle;
    gamevar.place_dir = angle;
    gamevar.place_redraw = 1;
    shape = &shapes[objects[gamevar.place_kind].shapes[angle]];
    gamevar.place_max_x = gamevar.place_max_y = 0;
    gamevar.place_min_x = gamevar.place_min_y = 0;
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
        gamevar.place_max_x = gamevar.place_max_x > x ? gamevar.place_max_x : x;
        gamevar.place_min_x = gamevar.place_min_x < x ? gamevar.place_min_x : x;
        gamevar.place_max_y = gamevar.place_max_y > y ? gamevar.place_max_y : y;
        gamevar.place_min_y = gamevar.place_min_y < y ? gamevar.place_min_y : y;
    }
    return changed;
}

// Starts placing an object of kind: in room (room_info index, 0 for the
// corridor), freely (mode 0), against a wall (when the object hugs walls), or
// moving one already placed (mode 2). offset is kept in gamevar.place_offset.
// FUNCTION: 0x000275c8
void init_place_object(u8 kind, u8 mode, u16 room, s16 offset)
{
    if (mode == 1) {
        mode = 0;
    }
    gamevar.place_wall_hug = objects[kind].wall_hug;
    if (mode == 0 && gamevar.place_wall_hug != -1) {
        mode = 1;
    }
    gamevar.place_kind = kind;
    gamevar.place_mode = mode;
    gamevar.place_room = room;
    gamevar.place_offset = offset;
    place_object_change_angle((s8)gamevar.place_dir);
    set_mouse_mode(MM_PLACE_OBJECT);
    if (mode == 2) {
        crequester.kind = 3;
    }
    setup_place_object_requester();
    crequester.arg3 = kind;
    if (mode == 2) {
        crequester.kind = 3;
        set_edit_room_requester_mode(10);
    }
}

// Links the two halves of room's double door (door_partner, door_main),
// turning the second half into an object.
// FUNCTION: 0x00027694
void init_double_doors(RoomInfo *room)
{
    Thing *door;
    Thing *other;

    door = 0;
    other = 0;
    door = search_mapwho_for_room_door(&map[room->door_tile], room - room_info);
    if (door > thing) {
        if (door->facing == 1) {
            other = search_mapwho_for_room_door(&map[room->door_tile - 128], room - room_info);
        } else {
            other = search_mapwho_for_room_door(&map[room->door_tile - 1], room - room_info);
        }
    } else {
        door = search_mapwho_for_room_door(&map[room->door_inside_tile], room - room_info);
        if (door > thing) {
            if (door->facing == 0) {
                other = search_mapwho_for_room_door(&map[room->door_inside_tile - 1], room - room_info);
            } else {
                other = search_mapwho_for_room_door(&map[room->door_inside_tile - 128], room - room_info);
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

// Makes room the build state's room for editing and puts its outline up. If
// nobody is inside and its door is free, its f3 is set to 4 (editing);
// otherwise it is flagged to wait (f22) and the local player's mouse mode set
// to 14.
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
    x1 = x + room->width - 1;
    y1 = y + room->height - 1;
    mark_blueprint_on_map(x, y, x1, y1, 0);
    build->room_type = room->type;
    build->room = room - room_info;
    build->x0 = x;
    build->y0 = y;
    build->x1 = x1;
    build->y1 = y1;
    if (othervar.cur_player == player) {
        crequester.kind = room->type;
        gamevar.place_mode = 0;
    }
    if (fn_00029a54(room)) {
        room->state = RS_EDITED;
        if (othervar.cur_player == player) {
            setup_place_object_requester();
        }
    } else {
        room->being_edited = 1;
        if (othervar.cur_player == player) {
            set_mouse_mode(MM_EDIT_ROOM_WAIT);
        }
    }
}

// Counts the object kinds still to place (gamevar.buy_kind_count) and shows the
// object placing requester.
// FUNCTION: 0x00027a90
void setup_place_object_requester(void)
{
    u8 count;
    u8 i;

    gamevar.buy_kind_count = 0;
    for (i = 0; i < 0x3e; i++) {
        count = gamevar.buy_list.to_place[i];
        if (count != 0) {
            gamevar.buy_kind_count++;
        }
    }
    set_edit_room_requester_mode(6);
}

// Returns the first object kind with any left to place, or 0.
// FUNCTION: 0x00027af0
u8 next_object_to_build(void)
{
    u8 i;

    for (i = 0; i < 0x3e; i++) {
        if (gamevar.buy_list.to_place[i] != 0) {
            return i;
        }
    }
    return 0;
}

// Opens owner's room being edited (allhosps.edited_room) for use: takes down
// its outline, refunds objects lifted but not put back, charges for the room
// if not yet paid, and for the local player shows the room's help page the
// first time.
// FUNCTION: 0x00027b44
void open_room(u8 owner)
{
    RoomInfo *room;
    BuildState *build;
    u8 x;
    u8 y;

    room = &room_info[allhosps[owner].edited_room];
    build = &allhosps[owner].build;
    if (room > room_info) {
        room->state = 0;
        clear_pager_responses(room - room_info);
        for (x = build->x0; x <= build->x1 + 1; x++) {
            for (y = build->y0; y <= build->y1 + 1; y++) {
                place_small_thing((s16)(x + (y << 7)), 0, 0);
            }
        }
        clear_build_objects(owner);
        if (room->paid_for == 0) {
            change_balance(TX_BUILD_ROOM, -rooms[room->type].cost, 0, (u16)owner);
            room->paid_for = 1;
        }
        if (owner == player && othervar.editing_room == 0) {
            othervar.last_built_room = room->type;
        }
        add_static_shadows();
        othervar.full_redraw = 1;
        othervar.editing_room = 0;
        if (othervar.cur_player == player) {
            set_mouse_mode(MM_NORMAL);
            close_crequester(REQ_EDIT_ROOM);
            if (myplayer.room_help_seen[room->type] == 0) {
                show_help_page(0, room->type);
                myplayer.room_help_seen[room->type] = 1;
            }
        }
        allhosps[owner].edited_room = 0;
        NRAdcounts.open_rooms++;
    }
}

// Returns the pointed thing (othervar.pointed_thing) if it is the local player's and,
// when room is given, an object of that room other than a swing door; else 0.
// FUNCTION: 0x00027d48
u16 compute_object_pointed(RoomInfo *room)
{
    Thing *obj;

    obj = &thing[othervar.pointed_thing];
    if (obj->owner == player) {
        if (room) {
            if (!(!(obj->type == THING_OBJECT && (u16)(room - room_info) == obj->room_id) || obj->subtype == OBJ_SWING_DOOR || obj->subtype == OBJ_SWING_DOOR2)) {
                return othervar.pointed_thing;
            } else {
                return 0;
            }
        } else {
            return othervar.pointed_thing;
        }
    } else {
        return 0;
    }
}

// Lifts object id off the map into its owner's build list to be placed again.
// With both, also lifts its other half (editing a room). Otherwise stops
// anyone using it, as for moving a corridor object. For the local player,
// starts placing it again from where it was.
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
            if (objects[other->subtype].next_part_index != -1) {
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
                anim = check_frame(staff_anim_starts[user->subtype].walk, user) + (u16)user->facing;
                user->sprite = start[anim].frame;
                user->flip = start[anim].flags > 0;
            } else {
                abandon_complex(user);
                user->complex_action = 0;
            }
        }
        remove_object_from_map(obj);
        if (othervar.cur_player == player) {
            gamevar.place_mode = 2;
            crequester.kind = 3;
        }
    }
    if (othervar.cur_player == player) {
        gamevar.buy_list.to_place[obj->subtype]++;
        gamevar.place_dir = obj->facing;
        init_place_object(obj->subtype, gamevar.place_mode, room, obj->x.tile + (obj->y.tile << 7) - othervar.mouse_cell);
        othervar.pickup_thing = 0;
    }
}

// Refunds owner's lifted objects (at depreciation() percent) and deletes
// them; for the local player also clears the stock left to place. Litter bombs
// are kept in multiplayer games.
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
            if (statvar.num_players > 1 && obj->subtype == OBJ_LITTER_BOMB) {
                continue;
            }
            change_balance(TX_BUY_OBJECT, objects[obj->subtype].cost * depreciation() / 100, 0, owner);
            memset(obj, 0, sizeof(Thing));
            build->objects[i] = 0;
        }
    }
    if (owner == player) {
        for (i = 0; i < 0x3e; i++) {
            if (statvar.num_players > 1 && i == 0x11) {
                continue;
            }
            gamevar.buy_list.to_place[i] = 0;
        }
    }
}

// Returns the tile to place a wall-hugging object of kind at for mouse tile
// pos: against the room wall nearest pos, turned to face away from it, and
// kept inside the room with its second part.
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

    room = &room_info[gamevar.place_room];
    x = pos % 128;
    y = pos / 128;
    rx = room->pos % 128;
    ry = room->pos / 128;
    corner = wall_hug[gamevar.place_wall_hug].kind == 4;
    rx2 = rx + room->width;
    ry2 = ry + room->height;
    dx = x - rx;
    dy = y - ry;
    below = dy < room->height * dx / room->width;
    below2 = dy < room->height - dx * room->height / room->width;
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
    angle = wall_hug[gamevar.place_wall_hug].angle[side];
    if (angle != gamevar.place_dir)
        play_sound(SNDSRC_GENERAL, SFX_SWOOSH, -1, -1);
    gamevar.place_dir = angle;
    b = &statvar.object_bounds[kind][angle];
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
    if (objects[kind].next_part_index != -1) {
        u16 part_kind;
        u8 part_angle;
        u32 part_pos;
        s8 ox;
        s8 oy;

        part_kind = next_part[objects[kind].next_part_index].kind;
        part_angle = next_part[objects[kind].next_part_index].parts[angle].angle;
        part_pos = next_part[objects[kind].next_part_index].parts[angle].pos;
        b = &statvar.object_bounds[part_kind][part_angle];
        convert_MW_to_xy(next_part[objects[kind].next_part_index].parts[angle].pos, &ox, &oy);
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

// Percentage of an object's price refunded when it is sold back.
// FUNCTION: 0x00028710
s32 depreciation(void)
{
    return 100;
}

// Percentage of a room's price refunded when it is abandoned.
// FUNCTION: 0x00028734
s32 room_depreciation(void)
{
    return 50;
}

// Queues the local player's click in edit mode: picks up the object chosen
// (othervar.pickup_thing, or the object its user uses) or starts editing the room
// highlighted (build.edit_room).
// FUNCTION: 0x00028758
void clicked_for_edit(void)
{
    u8 edit;
    u16 room;

    room = allhosps[player].build.edit_room;
    if (othervar.pickup_thing) {
        edit = crequester.id == REQ_EDIT_ROOM || (crequester.id == REQ_BUY_OBJECTS && crequester.arg != 1);
        if (edit) {
            if (thing[othervar.pickup_thing].room_id != room)
                return;
        }
        D_001549d0[player].arg = edit;
        if (thing[othervar.pickup_thing].type == THING_OBJECT) {
            D_001549d0[player].value = othervar.pickup_thing;
        } else {
            D_001549d0[player].value = thing[othervar.pickup_thing].object;
        }
        D_001549d0[player].type = EV_PICK_UP_OBJECT;
    } else if (room) {
        D_001549d0[player].type = EV_INIT_EDIT_ROOM;
        D_001549d0[player].value = room;
        set_mouse_mode(MM_NORMAL);
    }
}

// Right click while editing a room: queues picking up the pointed object if it
// is in the room being edited.
// FUNCTION: 0x000288ac
void check_object_right_click_in_room_edit(void)
{
    Thing *obj;

    obj = &thing[othervar.pointed_thing];
    if (obj > thing && obj->type == THING_OBJECT && obj->room_id == allhosps[player].edited_room) {
        D_001549d0[player].type = EV_PICK_UP_OBJECT;
        D_001549d0[player].value = othervar.pointed_thing;
        D_001549d0[player].arg = 1;
    }
}

// Resets the extent of the object drag marks to empty.
// FUNCTION: 0x00028950
void init_object_drag_shape(void)
{
    othervar.drag_x0 = 0xff;
    othervar.drag_y0 = 0xff;
    othervar.drag_x1 = 0;
    othervar.drag_y1 = 0;
}

#define LINKED_OBJECT(w) ((w)->object && (thing[(w)->object].type == THING_OBJECT || thing[(w)->object].type == THING_DOOR) && (u16)((w) - thing) == thing[(w)->object].user ? (w)->object : 0)

#define DRAG_MIN(a, b) ((a) > (b) ? ((a) = (b)) : 0)
#define DRAG_MAX(a, b) ((a) < (b) ? ((a) = (b)) : 0)

// Marks the tiles an object of kind at pos facing angle would cover, and its
// use tiles: map flag 0x80 where blocked, else 0x40. Grows the extent that
// clear_object_drag_shape clears.
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
        DRAG_MIN(othervar.drag_x0, x);
        DRAG_MIN(othervar.drag_y0, y);
        DRAG_MAX(othervar.drag_x1, x);
        DRAG_MAX(othervar.drag_y1, y);
        if (map[pos + (s16)shape->points[i]].flags & 7) {
            map[pos + (s16)shape->points[i]].flags |= 0x80;
        } else {
            map[pos + (s16)shape->points[i]].flags |= 0x40;
        }
    }
    for (i = 2; i >= 0; i--) {
        x = (pos + shape->use_offsets[i]) % 128;
        y = (pos + shape->use_offsets[i]) / 128;
        DRAG_MIN(othervar.drag_x0, x);
        DRAG_MIN(othervar.drag_y0, y);
        DRAG_MAX(othervar.drag_x1, x);
        DRAG_MAX(othervar.drag_y1, y);
        if (map[pos + shape->use_offsets[i]].flags & 7) {
            map[pos + shape->use_offsets[i]].flags |= 0x80;
        } else {
            map[pos + shape->use_offsets[i]].flags |= 0x40;
        }
        x = (pos + shape->extra_use_offsets[i]) % 128;
        y = (pos + shape->extra_use_offsets[i]) / 128;
        DRAG_MIN(othervar.drag_x0, x);
        DRAG_MIN(othervar.drag_y0, y);
        DRAG_MAX(othervar.drag_x1, x);
        DRAG_MAX(othervar.drag_y1, y);
        if (map[pos + shape->extra_use_offsets[i]].flags & 7) {
            map[pos + shape->extra_use_offsets[i]].flags |= 0x80;
        } else {
            map[pos + shape->extra_use_offsets[i]].flags |= 0x40;
        }
    }
}

// Clears the drag marks within the recorded extent and resets it.
// FUNCTION: 0x00028e08
void clear_object_drag_shape(void)
{
    int x;
    int y;

    for (x = othervar.drag_x0; x <= othervar.drag_x1; x++) {
        for (y = othervar.drag_y0; y <= othervar.drag_y1; y++) {
            map[(s16)x + ((s16)y << 7)].flags &= 0x3f;
        }
    }
    init_object_drag_shape();
}

// Recomputes map flag 0x10 on the cells next to visible walls.
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
            if ((m->wall_n && m->wall_n < 0x8e) || m->wall_n > 0x91) {
                m[-129].flags |= 0x10;
                m[-128].flags |= 0x10;
                m[-127].flags |= 0x10;
            }
            if ((m->wall_w && m->wall_w < 0x8e) || m->wall_w > 0x91) {
                m[-129].flags |= 0x10;
                m[-1].flags |= 0x10;
                m[127].flags |= 0x10;
            }
        }
        m += 2;
    }
}

// Returns 1 if a corridor object of kind at pos facing dir would leave its own
// use tile (except plants and litter bombs) and those of nearby corridor
// objects reachable from the entrance, else 0.
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
    entrance = room_info[myhosp->entrances[0]].door_tile;
    ok = 1;
    side = 1;
    count = 0x18;
    if (kind == 0xb) {
        side = 0;
        count = 0x18;
    }
    mark_object_shape(cell, shape, dir, &objects[kind], 0);
    if (!(kind == 0x2d || kind == 0x11 || check_points_connected(entrance, pos + shape->use_offsets[side]))) {
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
                            if (check_points_connected(entrance, tile + other_shape->use_offsets[side]) == 0) {
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

// Returns 1 if room has every object its type requires, else 0.
// FUNCTION: 0x000291e8
u8 does_room_contain_compulsory_objects(RoomInfo *room)
{
    Thing *obj;
    int i;
    int found;
    int kind;

    for (i = rooms[room->type].compulsory_count - 1; i >= 0; i--) {
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

// Returns 1 if placing an object of kind at pos facing dir in room keeps the
// room's objects usable and everyone inside able to reach its door, else 0.
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
        if (!check_points_connected(pos2, room->door_inside_tile)) {
            ok = 0;
            break;
        }
    }
    remove_object_shape(cell, shape, dir, &objects[kind]);
    return ok;
}

// Right click: picks up the pointed object (or the object a patient is using)
// if it belongs to the local player and may be moved from where it is.
// FUNCTION: 0x00029520
void pick_up_object_right_click(void)
{
    Thing *clicked;

    clicked = &thing[othervar.pointed_thing];
    if (clicked->type == THING_PATIENT && LINKED_OBJECT(clicked)) {
        clicked = &thing[LINKED_OBJECT(clicked)];
    }
    if (clicked->type == THING_OBJECT && clicked->owner == player && (!clicked->room_id || (objects[clicked->subtype].flags & 8)) && !(objects[clicked->subtype].flags & 1)) {
        othervar.pickup_thing = othervar.pointed_thing;
        clicked_for_edit();
    }
}

// Returns 1 if the local player is dragging a blueprint or any blueprint
// outline is on the map, else 0.
// FUNCTION: 0x00029748
extern "C" u8 is_player_building(void)
{
    Map *m;
    int i;
    int small;

    if (allhosps[player].build.draw_mode == 1) {
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

// Rechecks the invisible walls of the objects on the ring of tiles around
// room.
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
    x1 = room->pos % 128 + room->width;
    y1 = room->pos / 128 + room->height;
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

// Cancels editing owner's room (allhosps.edited_room): clears its waiting flag
// and takes its outline off the map.
// FUNCTION: 0x0002994c
extern "C" void fn_0002994c(u8 owner)
{
    RoomInfo *room;
    BuildState *build;
    u8 x;
    u8 y;

    room = &room_info[allhosps[owner].edited_room];
    build = &allhosps[owner].build;
    if (room > room_info) {
        room->being_edited = 0;
        for (x = build->x0; x <= build->x1 + 1; x++) {
            for (y = build->y0; y <= build->y1 + 1; y++) {
                place_small_thing((s16)(x + (y << 7)), 0, 0);
            }
        }
        othervar.full_redraw = 1;
        allhosps[owner].edited_room = 0;
    }
}

// Returns 1 if nobody is in room and its door is not in use, else 0.
// FUNCTION: 0x00029a54
extern "C" u8 fn_00029a54(RoomInfo *room)
{
    Thing *door;

    if (room->first) {
        return 0;
    }
    door = search_mapwho(&map[room->door_tile], THING_DOOR);
    if (door > thing) {
        if (door->user) {
            return 0;
        }
    }
    return 1;
}
