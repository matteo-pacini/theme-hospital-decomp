#include "hosp.h"

u8 is_tile_valid(u16 tile, u8 owner);
u8 is_valid_room(s16 x0, s16 y0, s16 x1, s16 y1, u8 owner);
void mark_blueprint_on_map(s16 x0, s16 y0, s16 x1, s16 y1, u8 flag);
void cleanse_area(u8 owner, u8 x, u8 y, u8 w, u8 h);
void remove_object_from_map(Thing *t);
void room_deleted_update_listhead(RoomInfo *room);
void delete_room(RoomInfo *room);
void repair_invis_walls_around_room(RoomInfo *room);
void remove_blank_wall(Map *cell, u8 side);
void insert_blank_wall(Map *cell, u8 side);
u8 is_room_clear_yet(RoomInfo *room);
void build_all_rooms(void);
void price_all_rooms(u16 type, u8 owner);
void init_double_doors(RoomInfo *room);
u8 try_move_peep_to_wall(Thing *w, u8 side);
u8 try_move_peep_cell(Thing *w, u8 x, u8 y);
s32 room_depreciation(void);
void remove_expertise_room(RoomInfo *room);
u8 is_valid_no_room(u16 tile);
u8 is_room_size_valid(s16 x0, s16 y0, s16 x1, s16 y1, u8 owner);
u8 check_room_access(u8 owner);
void blank_wall_around_blueprint(u8 owner, u8 blank);
u8 is_tile_near_half_wall(u8 x, u8 y, u8 x0, u8 y0, u8 x1, u8 y1);
void set_expertise_room(RoomInfo *room);

extern "C" {

extern u8 D_001553c7;
extern u8 D_001553cf;
extern u16 D_001553f9;
extern u16 D_001553fb;
extern u8 D_001553e8;
extern u8 D_001553c8;
extern u16 D_00188c74;
extern s16 D_001567b0[8];

}

// FUNCTION: 0x00045890
void draw_blueprint_area(const s16 x0, const s16 y0, const s16 x1, const s16 y1, const u8 owner, const u8 mode)
{
    u8 palette;
    u8 piece;
    u16 saved;
    u16 block;
    u16 base;
    u16 width;
    u16 height;
    s16 y;
    s16 x;
    s16 sx;
    s16 sy;
    Map *cell;
    s32 tx;
    s32 ty;

    saved = D_00188c74;
    palette = 0;
    if (owner != player)
        return;
    D_00188c74 = 4;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    block = 0x17;
    base = 0;
    width = x1 - x0 + 1;
    height = y1 - y0 + 1;
    if (mode == 2) {
        switch (game_turn % 6) {
        case 0:
            palette = 0;
            goto outline;
        case 1:
        case 5:
            palette = 4;
            goto outline;
        case 2:
        case 4:
            palette = 8;
            goto outline;
        case 3:
            return;
        }
    }
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++) {
            give_screen_xy_of_cell_at_column_row(window.unknown2eb, x, y);
            sx = info.unknown54;
            sy = info.unknown58;
            set_all_refresh_zones_in_window(sx, sy - resvars.f3, sx + resvars.f4, sy, 3);
            if (!(is_room_size_valid(x0, y0, x1, y1, owner) && is_valid_no_room((y << 7) + x)
                  && is_tile_valid((y << 7) + x, owner))
                || is_tile_near_half_wall(x, y, x0, y0, x1, y1)) {
                LbSpriteDraw(sx, sy - blocks[66].info->height, blocks[66].info);
                D_001553cf = 1;
            } else if ((height % 2 == 0 && width % 2 == 0) && width > 3 && height > 3) {
                if (y == (height - 1) / 2 + y0 && x == (width - 1) / 2 + x0)
                    LbSpriteDraw(sx, sy - blocks[38].info->height, blocks[38].info);
                else if (y == (height - 1) / 2 + y0 && x == (width + 1) / 2 + x0)
                    LbSpriteDraw(sx, sy - blocks[37].info->height, blocks[37].info);
                else if (y == (height + 1) / 2 + y0 && x == (width - 1) / 2 + x0)
                    LbSpriteDraw(sx, sy - blocks[36].info->height, blocks[36].info);
                else if (y == (height + 1) / 2 + y0 && x == (width + 1) / 2 + x0)
                    LbSpriteDraw(sx, sy - blocks[39].info->height, blocks[39].info);
                else
                    LbSpriteDraw(sx, sy - blocks[block].info->height, blocks[block].info);
            } else {
                LbSpriteDraw(sx, sy - blocks[block].info->height, blocks[block].info);
            }
        }
    }
outline:
    for (y = y0; y <= y1 + 1; y++) {
        for (x = x0; x <= x1 + 1; x++) {
            if (y == y0) {
                if (x == x0)
                    piece = 3;
                else if (x == x1 + 1)
                    piece = 2;
                else
                    piece = 1;
            } else if (y == y1 + 1) {
                if (x == x1 + 1)
                    piece = 0;
                else
                    piece = 1;
            } else if (x == x0 || x == x1 + 1) {
                piece = 2;
            } else {
                piece = 0;
            }
            cell = &map[x + (y << 7)];
            if (piece) {
                give_screen_xy_of_cell_at_column_row(window.unknown2eb, x, y - 1);
                sx = info.unknown54;
                sy = info.unknown58;
                D_001567b0[0] = D_001567b0[1] = 999;
                D_001567b0[2] = D_001567b0[3] = 0;
                tx = x;
                ty = y;
                if (mode != 2) {
                    if (!((blocks[cell->unknown3].width & 0x10) || (blocks[cell->unknown4].width & 0x10))) {
                        if (y == y1 + 1 || x == x1 + 1) {
                            if (x == x1 + 1 && y != y1 + 1) {
                                tx--;
                            } else if (x != x1 + 1 && y == y1 + 1) {
                                ty--;
                            } else {
                                tx--;
                                ty--;
                            }
                        }
                    } else {
                        if (y == y1 + 1 || x == x1 + 1) {
                            if (x == x1 + 1 && y != y1 + 1)
                                tx--;
                            else if (x != x1 + 1 && y == y1 + 1)
                                ty--;
                            else
                                ty--;
                        }
                    }
                }
                if (mode != 2
                    && (!(is_valid_no_room((ty << 7) + tx) && is_tile_valid((ty << 7) + tx, owner)
                          && is_room_size_valid(x0, y0, x1, y1, owner))
                        || is_tile_near_half_wall(x, y, x0, y0, x1, y1)))
                    draw_a_one_colour_frame(start[blueprint[piece + base].anim].frame, sx, sy, color.unknownD,
                                            start[blueprint[piece + base].anim].flags);
                else
                    draw_a_frame(start[blueprint[piece + base].anim].frame, sx, sy, palette,
                                 start[blueprint[piece + base].anim].flags);
                LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
                LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
                set_all_refresh_zones_in_window(D_001567b0[0], D_001567b0[1], D_001567b0[2], D_001567b0[3], 3);
            }
        }
    }
    D_00188c74 = saved;
}

// FUNCTION: 0x00046174
void mark_blueprint_on_map(s16 x0, s16 y0, s16 x1, s16 y1, u8 flag)
{
    u16 block;
    u16 base;
    u16 width;
    u16 height;
    s16 y;
    s16 x;
    u8 piece;
    Map *cell;

    D_001553c8 = 1;
    block = 0x17;
    base = 0;
    width = x1 - x0 + 1;
    height = y1 - y0 + 1;
    for (y = y0; y <= y1 + 1; y++) {
        for (x = x0; x <= x1 + 1; x++) {
            if (y == y0) {
                if (x == x0)
                    piece = 3;
                else if (x == x1 + 1)
                    piece = 2;
                else
                    piece = 1;
            } else if (y == y1 + 1) {
                if (x == x1 + 1)
                    piece = 0;
                else
                    piece = 1;
            } else if (x == x0 || x == x1 + 1) {
                piece = 2;
            } else {
                piece = 0;
            }
            cell = &map[x + (y << 7)];
            if (flag && y != y1 + 1 && x != x1 + 1) {
                if (height % 2 == 0 && width % 2 == 0) {
                    if (y == (height - 1) / 2 + y0 && x == (width - 1) / 2 + x0)
                        cell->overlay = 0x26;
                    else if (y == (height - 1) / 2 + y0 && x == (width + 1) / 2 + x0)
                        cell->overlay = 0x25;
                    else if (y == (height + 1) / 2 + y0 && x == (width - 1) / 2 + x0)
                        cell->overlay = 0x24;
                    else if (y == (height + 1) / 2 + y0 && x == (width + 1) / 2 + x0)
                        cell->overlay = 0x27;
                    else
                        cell->overlay = block;
                } else {
                    cell->overlay = block;
                }
            }
            if (piece) {
                if (!((blocks[cell->unknown3].width & 0x10) || (blocks[cell->unknown4].width & 0x10))) {
                    place_small_thing((s16)((y << 7) + x), 0, 0);
                    place_small_thing((y << 7) + x, (u16)piece + base, 0);
                } else {
                    place_small_thing((y << 7) + x, (u16)piece + base, 0);
                }
            }
        }
    }
}

// FUNCTION: 0x000464bc
void init_blueprint(u8 owner, u8 type)
{
    BuildState *build;

    build = &allhosps[owner].build;
    memset(build, 0, 0x3d);
    build->unknownA[2] = 0;
    build->unknownD = type;
}

// NONMATCHING: frame layout (build/on_y1 and dx/dy slots swapped; declaration order does not move them)
// FUNCTION: 0x00046514
void modify_blueprint(u8 owner, u8 dragging, u16 tile)
{
    u8 on_y1;
    u8 on_x1;
    u8 x;
    u8 y;
    u8 on_x0;
    u8 on_y0;
    u8 dx;
    u8 dy;
    BuildState *build;

    build = &allhosps[owner].build;
    x = tile % 128;
    y = tile / 128;
    on_x0 = x == build->x0;
    on_y0 = y == build->y0;
    on_x1 = x == build->x1;
    on_y1 = y == build->y1;
    if (dragging) {
        switch (build->drag) {
        case 0:
            build->unknownE_ = 1;
            if (!(x >= build->x0 && x <= build->x1 && y >= build->y0 && y <= build->y1)) {
                build->anchor.x = x;
                build->anchor.y = y;
                build->x0 = x;
                build->y0 = y;
                build->x1 = x;
                build->y1 = y;
                build->drag = 1;
            } else if (on_x0 && on_y0) {
                build->anchor = build->end;
                build->drag = 1;
            } else if (on_x0 && on_y1) {
                build->anchor.x = build->x1;
                build->anchor.y = build->y0;
                build->drag = 1;
            } else if (on_x1 && on_y0) {
                build->anchor.x = build->x0;
                build->anchor.y = build->y1;
                build->drag = 1;
            } else if (on_x1 && on_y1) {
                build->anchor.x = build->x0;
                build->anchor.y = build->y0;
                build->drag = 1;
            } else if (on_x0) {
                build->anchor.x = build->x1;
                build->anchor.y = build->y0;
                build->pivot.x = build->x1;
                build->pivot.y = build->y1;
                build->drag = 2;
            } else if (on_y0) {
                build->anchor.x = build->x0;
                build->anchor.y = build->y1;
                build->pivot.x = build->x1;
                build->pivot.y = build->y1;
                build->drag = 2;
            } else if (on_x1) {
                build->anchor.x = build->x0;
                build->anchor.y = build->y0;
                build->pivot.x = build->x0;
                build->pivot.y = build->y1;
                build->drag = 2;
            } else if (on_y1) {
                build->anchor.x = build->x0;
                build->anchor.y = build->y0;
                build->pivot.x = build->x1;
                build->pivot.y = build->y0;
                build->drag = 2;
            } else {
                build->anchor.x = x;
                build->anchor.y = y;
                build->drag = 3;
            }
            break;
        case 1:
            build->x0 = build->anchor.x < x ? build->anchor.x : x;
            build->y0 = build->anchor.y < y ? build->anchor.y : y;
            build->x1 = build->anchor.x > x ? build->anchor.x : x;
            build->y1 = build->anchor.y > y ? build->anchor.y : y;
            break;
        case 2:
            if (build->anchor.y == build->pivot.y) {
                build->x0 = build->anchor.x;
                build->y0 = y < build->anchor.y ? y : build->anchor.y;
                build->x1 = build->pivot.x;
                build->y1 = y > build->anchor.y ? y : build->anchor.y;
            } else {
                build->x0 = build->anchor.x < x ? build->anchor.x : x;
                build->y0 = build->anchor.y;
                build->x1 = build->anchor.x > x ? build->anchor.x : x;
                build->y1 = build->pivot.y;
            }
            break;
        case 3:
            dx = x - build->anchor.x;
            dy = y - build->anchor.y;
            build->x0 += dx;
            build->y0 += dy;
            build->x1 += dx;
            build->y1 += dy;
            build->anchor.x = x;
            build->anchor.y = y;
            break;
        }
    } else {
        switch (allhosps[owner].build.drag) {
        case 0:
            if (D_001553c7 == player) {
                if (!(x >= build->x0 && x <= build->x1 && y >= build->y0 && y <= build->y1))
                    D_001553e8 = 3;
                else if ((((on_x0 && on_y0) || (on_x0 && on_y1)) || (on_x1 && on_y0)) || (on_x1 && on_y1))
                    D_001553e8 = 7;
                else if (on_x0 || on_x1)
                    D_001553e8 = 6;
                else if (on_y0 || on_y1)
                    D_001553e8 = 5;
                else
                    D_001553e8 = 8;
                set_mouse_sprite(D_001553e8);
            }
            break;
        case 1:
        case 2:
        case 3:
            build->drag = 0;
            break;
        default:
            break;
        }
    }
}

// FUNCTION: 0x00046b54
u8 commit_blueprint(u8 owner)
{
    BuildState *build;
    u8 w;
    u8 x;
    u8 y;
    RoomInfo *room;
    u8 h;

    build = &allhosps[owner].build;
    if (!is_valid_room(build->x0, build->y0, build->x1, build->y1, owner)) {
        if (D_001553c7 == player) {
            advisor.f2 = 4;
            set_mouse_mode(1);
        }
        return 0;
    }
    if (!check_room_access_with_blueprint(owner)) {
        if (D_001553c7 == player) {
            advisor.f2 = 5;
            set_mouse_mode(1);
        }
        return 0;
    }
    room = &room_info[build->room];
    build->door_state = rooms[build->unknownD].unknown36;
    allhosps[owner].build.unknownE_ = 0;
    build->unknown3c = map[(s16)build->x0 + ((s16)build->y0 << 7)].overlay;
    mark_blueprint_on_map(build->x0, build->y0, build->x1, build->y1, 1);
    room->unknown22 = actual_game_turn;
    room->pos = (s16)(build->x0 + (build->y0 << 7));
    room->f1 = build->x1 - build->x0 + 1;
    room->f2 = build->y1 - build->y0 + 1;
    cleanse_area(room->owner, room->pos % 128, room->pos / 128, room->f1, room->f2);
    blank_wall_around_blueprint(owner, 1);
    build->state = 1;
    if (D_001553c7 == player) {
        set_edit_room_requester_mode(2);
        set_mouse_mode(2);
    }
    w = build->x1 - build->x0 + 1;
    h = build->y1 - build->y0 + 1;
    for (y = build->y0; y < build->y0 + h; y++) {
        for (x = build->x0; x < build->x0 + w; x++) {
            map[(s16)x + ((s16)y << 7)].floor &= 0xaf;
        }
    }
    return 1;
}

// FUNCTION: 0x00046e50
void uncommit_blueprint(u8 owner)
{
    BuildState *build;
    u8 x;
    u8 y;

    build = &allhosps[owner].build;
    blank_wall_around_blueprint(owner, 0);
    for (x = build->x0; x <= build->x1 + 1; x++) {
        for (y = build->y0; y <= build->y1 + 1; y++) {
            place_small_thing((s16)(x + (y << 7)), 0, 0);
        }
    }
    for (x = build->x0; x <= build->x1; x++) {
        for (y = build->y0; y <= build->y1; y++) {
            map[(s16)x + ((s16)y << 7)].overlay = build->unknown3c;
        }
    }
    build->unknownE_ = 1;
    allhosps[owner].build.state = 0;
    D_001553c8 = 1;
}

// FUNCTION: 0x00046f88
u8 is_tile_valid(u16 tile, u8 owner)
{
    Thing *t;

    for (t = &thing[map[tile].first]; t > thing; t = &thing[t->next]) {
        if (t->renderer != 2 && t->renderer != 4) {
            advisor.f11f |= 2;
            return 0;
        }
    }
    if (map[tile].flags & 7) {
        return 0;
    }
    if (!(map[tile].unknown7 & 0x10) && (map[tile].unknown7 & (1 << owner))) {
        return 1;
    }
    advisor.f11f |= 2;
    return 0;
}

// FUNCTION: 0x00047098
u8 is_valid_room(s16 x0, s16 y0, s16 x1, s16 y1, u8 owner)
{
    u16 width;
    u16 height;
    u16 type;
    u16 y;
    u16 x;

    width = x1 - x0 + 1;
    height = y1 - y0 + 1;
    type = room_info[allhosps[owner].build.room].type;
    if (width < rooms[type].unknown27[0] || height < rooms[type].unknown27[1]) {
        return 0;
    }
    if (which_room((y0 << 7) + x0) || which_room((x1 - 1) + ((y1 - 1) << 7))) {
        return 0;
    }
    for (y = y0; y < y0 + height; y++) {
        for (x = x0; x < x0 + width; x++) {
            if (which_room((y << 7) + x) || !is_tile_valid((y << 7) + x, owner)) {
                return 0;
            }
            if (is_tile_near_half_wall(x, y, x0, y0, x1, y1)) {
                return 0;
            }
        }
    }
    return 1;
}

// FUNCTION: 0x00047248
u8 is_valid_door(u16 tile, u8 owner, u8 type)
{
    s32 pos;

    pos = tile + (s16)doortype[type % 4 + 4].door1;
    return !(map[pos].unknown7 & 0x10) && which_room(pos) == 0;
}

// FUNCTION: 0x000472c8
u8 is_valid_no_room(u16 tile)
{
    if (which_room(tile) == 0) {
        return 1;
    }
    advisor.f11f |= 2;
    return 0;
}

// FUNCTION: 0x00047308
void edit_blueprint(u8 owner)
{
    BuildState *build;
    RoomInfo *room;
    u8 i;
    Thing *t;

    build = &allhosps[owner].build;
    room = &room_info[allhosps[owner].build.room];
    i = 0;
    while ((t = &thing[room->first_object]) > thing) {
        if (D_001553c7 == player && (!t->group || objects[thing[t->group].subtype].unknown17 == -1))
            gamevar.unknown33.unknown0[t->subtype]++;
        remove_object_from_map(t);
        for (; i < 20; i++) {
            if (build->objects[i] == 0) {
                build->objects[i] = t - thing;
                break;
            }
        }
    }
    if (room->unknown4 && room->unknown4 < 0x4000)
        build->unknown3c = map[room->unknown4].overlay;
    room_deleted_update_listhead(room);
    delete_room(room);
    uncommit_blueprint(owner);
    repair_invis_walls_around_room(room);
    room->pos = 0;
    room->unknown4 = 0;
    room->unknown6 = 0;
    room->f1 = 0;
    room->f2 = 0;
    room->f3 = 3;
    if (D_001553c7 == player) {
        set_edit_room_requester_mode(0);
        set_mouse_mode(1);
    }
}

// FUNCTION: 0x0004750c
void compute_room_pointed(u16 tile)
{
    RoomInfo *room;
    BuildState *build;
    Thing *t;

    room = which_room(tile);
    build = &allhosps[player].build;
    if (room && room->type != 6) {
        build->edit_room = room - room_info;
        build->unknownE_ = 2;
        build->x0 = room->pos % 128;
        build->y0 = room->pos / 128;
        build->x1 = room->pos % 128 + room->f1 - 1;
        build->y1 = room->pos / 128 + room->f2 - 1;
    } else {
        build->edit_room = 0;
        build->unknownE_ = 0;
        t = &thing[D_001553f9];
        if (t > thing) {
            switch (t->renderer) {
            case 6:
                if (t->unknown89 == 0 && !(objects[t->subtype].unknown20[2] & 1) && t->owner == player)
                    D_001553fb = D_001553f9;
                break;
            case 2:
            case 4:
                if (t->unknown69 && (u16)(t - thing) == thing[t->unknown69].unknown91
                    && thing[t->unknown69].unknown89 == 0 && t->owner == player)
                    D_001553fb = D_001553f9;
                break;
            }
        }
    }
}

// FUNCTION: 0x00047778
u8 check_room_access(u8 owner)
{
    u16 pos;
    RoomInfo *room;
    Thing *t;

    pos = room_info[allhosps[owner].unknown1cf[0]].unknown4;
    for (room = room_info + 1; room < end_room_info; room++) {
        if (((!(room->type == 0 || room->f3 == 3) && room->owner == owner) && room->type != 1) && room->type != 4) {
            if (!check_points_connected(pos, room->unknown4))
                return 0;
            pos = room->unknown4;
        }
    }
    for (t = thing + 1; t < end_things; t++) {
        if ((t->renderer == 6 && t->subtype == 0xb) && t->owner == owner) {
            if (!check_points_connected(pos, t->use_pos[0]))
                return 0;
            if (!check_points_connected(pos, t->use_pos[1]))
                return 0;
        }
    }
    return 1;
}

// FUNCTION: 0x0004790c
u8 check_room_access_with_blueprint(u8 owner)
{
    BuildState *build;
    u8 result;

    build = &allhosps[owner].build;
    blank_wall_around_blueprint(owner, 1);
    result = check_room_access(owner);
    blank_wall_around_blueprint(owner, 0);
    return result;
}

// FUNCTION: 0x00047974
void blank_wall_around_blueprint(u8 owner, u8 blank)
{
    u8 x0;
    u8 y0;
    u8 x1;
    u8 y1;
    u8 x;
    u8 y;

    x0 = allhosps[owner].build.x0;
    y0 = allhosps[owner].build.y0;
    x1 = allhosps[owner].build.x1;
    y1 = allhosps[owner].build.y1;
    for (x = x0; x <= x1; x++) {
        if (blank) {
            remove_blank_wall(&map[(s16)x + ((s16)y0 << 7)], 0);
            remove_blank_wall(&map[(s16)x + ((s16)(y1 + 1) << 7)], 0);
        } else {
            insert_blank_wall(&map[(s16)x + ((s16)y0 << 7)], 0);
            insert_blank_wall(&map[(s16)x + ((s16)(y1 + 1) << 7)], 0);
        }
    }
    for (y = y0; y <= y1; y++) {
        if (blank) {
            remove_blank_wall(&map[(s16)x0 + ((s16)y << 7)], 1);
            remove_blank_wall(&map[(s16)(x1 + 1) + ((s16)y << 7)], 1);
        } else {
            insert_blank_wall(&map[(s16)x0 + ((s16)y << 7)], 1);
            insert_blank_wall(&map[(s16)(x1 + 1) + ((s16)y << 7)], 1);
        }
    }
}

// FUNCTION: 0x00047b88
u8 is_room_clear_yet(RoomInfo *room)
{
    u8 x;
    u8 y;
    u8 x0;
    u8 x1;
    u8 y0;
    u8 y1;

    x0 = room->pos % 128;
    y0 = room->pos / 128;
    x1 = x0 + room->f1;
    y1 = y0 + room->f2;
    for (x = x0; x < x1; x++) {
        for (y = y0; y < y1; y++) {
            if (map[((s16)y << 7) + (s16)x].first > 0) {
                return 0;
            }
        }
    }
    return 1;
}

// FUNCTION: 0x00047c58
void process_blueprint_zones(void)
{
    s32 count;
    s8 i;

    count = 1;
    if (statvar.unknownfd > 1) {
        count = 4;
    }
    for (i = count - 1; i >= 0; i--) {
        if (statvar.unknownfd > 1 && D_001609c4[i].active == 0) {
            continue;
        }
        D_001553c7 = i;
        if (allhosps[i].build.state == 2) {
            commit_doors(i);
        }
    }
}

// FUNCTION: 0x00047cf8
void commit_doors(u8 owner)
{
    allhosps[owner].build.state = 2;
    if (is_room_clear_yet(&room_info[allhosps[owner].build.room])) {
        allhosps[owner].build.state = 0;
        build_all_rooms();
        price_all_rooms((u16)allhosps[owner].build.unknownD, owner);
        if (rooms[allhosps[owner].build.unknownD].unknown36 == 0)
            init_double_doors(&room_info[allhosps[owner].build.room]);
        init_edit_room(&room_info[allhosps[owner].build.room]);
        listhead[owner].rooms[allhosps[owner].build.unknownD] = allhosps[owner].build.room;
        if (owner == player)
            init_place_object(next_object_to_build(), 0, allhosps[owner].build.room, 0);
    }
}

// FUNCTION: 0x00047ed0
u8 move_peep_from_blueprint(Thing *w)
{
    s8 i;
    u8 x;
    u8 y;
    u8 side;
    u8 best;
    u8 dist[4];
    BuildState *build;

    build = &allhosps[w->owner].build;
    x = w->x.tile;
    y = w->y.tile;
    if (!(x >= build->x0 && x <= build->x1 && y >= build->y0 && y <= build->y1))
        return 0;
    dist[0] = y - build->y0;
    dist[1] = build->x1 - x;
    dist[2] = build->y1 - y;
    dist[3] = x - build->x0;
    do {
        best = 0xff;
        for (i = 3; i >= 0; i--) {
            if (dist[i] < best) {
                best = dist[i];
                side = i;
            }
        }
        if (best != 0xff) {
            dist[side] = 0xff;
            if (try_move_peep_to_wall(w, side))
                return 1;
        }
    } while (best != 0xff);
    return 0;
}

// FUNCTION: 0x00047ffc
u8 try_move_peep_to_wall(Thing *w, u8 side)
{
    u8 i;
    BuildState *build;
    u8 count;
    u8 base;
    u8 idx;
    u8 cells[128];

    build = &allhosps[w->owner].build;
    if (side == 0 || side == 2) {
        count = build->x1 - build->x0 + 1;
        base = build->x0;
        idx = w->x.tile - base;
    } else {
        count = build->y1 - build->y0 + 1;
        base = build->y0;
        idx = w->y.tile - base;
    }
    for (i = 0; i < count; i++)
        cells[i] = base + i;
    for (;;) {
        switch (side) {
        case 0:
            if (try_move_peep_cell(w, cells[idx], build->y0 - 1))
                return 1;
            break;
        case 1:
            if (try_move_peep_cell(w, build->x1 + 1, cells[idx]))
                return 1;
            break;
        case 2:
            if (try_move_peep_cell(w, cells[idx], build->y1 + 1))
                return 1;
            break;
        case 3:
            if (try_move_peep_cell(w, build->x0 - 1, cells[idx]))
                return 1;
            break;
        }
        --count;
        cells[idx] = cells[count];
        if (count == 0)
            break;
        idx = old_newrand() % count;
    }
    return 0;
}

// FUNCTION: 0x00048224
u8 try_move_peep_cell(Thing *w, u8 x, u8 y)
{
    u16 pos;
    u16 tile;

    pos = room_info[allhosps[w->owner].unknown1cf[0]].unknown4;
    tile = (s16)(x + (y << 7));
    if ((!(map[tile].unknown7 & 0x10) && !which_room(tile)) && check_points_connected(tile, pos)) {
        w->unknown105 = w->kind;
        w->unknown106 = x;
        w->unknown107 = y;
        if (w->renderer == 2)
            w->kind = 0x40;
        else
            w->kind = 0x4c;
        w->height = 0;
        return 1;
    }
    return 0;
}

// FUNCTION: 0x0004831c
u8 is_room_size_valid(s16 x0, s16 y0, s16 x1, s16 y1, u8 owner)
{
    u16 width;
    u16 height;
    u16 type;

    width = x1 - x0 + 1;
    height = y1 - y0 + 1;
    type = allhosps[owner].build.unknownD;
    if (width < rooms[type].unknown27[0] || height < rooms[type].unknown27[1]) {
        advisor.f11f |= 4;
        return 0;
    }
    advisor.f11f &= ~4;
    return 1;
}

// FUNCTION: 0x000483c4
void delete_blueprint_room(u8 owner)
{
    BuildState *build;
    RoomInfo *room;

    build = &allhosps[owner].build;
    room = &room_info[allhosps[owner].build.room];
    if (room->unknown33) {
        change_balance(4, rooms[room->type].cost * room_depreciation() / 100, 0, (u16)room->owner);
        room->unknown33 = 0;
    }
    if (allhosps[owner].build.unknownE_ == 0)
        uncommit_blueprint(owner);
    allhosps[owner].build.unknownE_ = 0;
    allhosps[owner].build.state = 0;
    restart_all_queue_peeps(room);
    remove_expertise_room(room);
    memset(room, 0, sizeof(RoomInfo));
    memset(build, 0, sizeof(BuildState));
}

// FUNCTION: 0x00048504
void draw_overdrawn_blueprints(void)
{
    u32 i;

    for (i = 0; i < 4; i++) {
        BuildState *build;
        u8 mode;

        build = &allhosps[i].build;
        mode = allhosps[i].build.unknownE_;
        switch (mode) {
        case 2:
            allhosps[i].build.unknownE_ = 0;
        case 1:
            draw_blueprint_area(build->x0, build->y0, build->x1, build->y1, i, mode);
            break;
        }
    }
}

// FUNCTION: 0x000485e0
void allocate_roominfo(const u8 owner, const u8 type)
{
    BuildState *build;
    RoomInfo *room;

    build = &allhosps[owner].build;
    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->type == 0) {
            build->room = room - room_info;
            memset(room, 0, sizeof(RoomInfo));
            room->owner = owner;
            room->type = type;
            room->f3 = 3;
            room->unknown22 = actual_game_turn;
            room->f1e = 6;
            set_expertise_room(room);
            break;
        }
    }
}

// FUNCTION: 0x000486a8
RoomInfo *allocate_reception_roominfo(u8 owner)
{
    RoomInfo *room;

    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->type == 0) {
            room->type = 4;
            room->pos = 0x3fff;
            room->f1 = room->f2 = 1;
            room->f3 = 0;
            room->owner = owner;
            room->f1e = 0x14;
            return room;
        }
    }
    return 0;
}

// FUNCTION: 0x0004873c
u8 check_near_half_wall(u8 x, u8 y, u8 x0, u8 y0, u8 x1, u8 y1)
{
    if (y != y1 + 1 && (blocks[map[(s16)x + ((s16)y << 7)].unknown4].width & 4))
        return 1;
    if (x != x1 + 1 && (blocks[map[(s16)x + ((s16)y << 7)].unknown3].width & 4))
        return 1;
    return 0;
}

// FUNCTION: 0x000487f4
u8 is_tile_near_half_wall(u8 x, u8 y, u8 x0, u8 y0, u8 x1, u8 y1)
{
    if (check_near_half_wall(x, y, x0, y0, x1, y1))
        return 1;
    if (x == x1 && check_near_half_wall(x + 1, y, x0, y0, x1, y1))
        return 1;
    if (x == x1 + 1 && check_near_half_wall(x - 1, y, x0, y0, x1, y1))
        return 1;
    if (y == y1 && check_near_half_wall(x, y + 1, x0, y0, x1, y1))
        return 1;
    if (y == y1 + 1 && check_near_half_wall(x, y - 1, x0, y0, x1, y1))
        return 1;
    return 0;
}
