#include "hosp.h"

u8 is_tile_valid(u16 tile, u8 owner);
u8 is_valid_room(s16 x0, s16 y0, s16 x1, s16 y1, u8 owner);
void mark_blueprint_on_map(s16 x0, s16 y0, s16 x1, s16 y1, u8 set_overlay);
void cleanse_area(u8 owner, u8 x, u8 y, u8 w, u8 h);
void remove_object_from_map(Thing *obj);
void room_deleted_update_listhead(RoomInfo *room);
void delete_room(RoomInfo *room);
void repair_invis_walls_around_room(RoomInfo *room);
void insert_blank_wall(Map *cell, u8 side);
void remove_blank_wall(Map *cell, u8 side);
u8 is_room_clear_yet(RoomInfo *room);
void build_all_rooms(void);
void price_all_rooms(u16 type, u8 owner);
void init_double_doors(RoomInfo *room);
u8 try_move_peep_to_wall(Thing *peep, u8 side);
u8 try_move_peep_cell(Thing *peep, u8 x, u8 y);
s32 room_depreciation(void);
void remove_expertise_room(RoomInfo *room);
u8 is_valid_no_room(u16 tile);
u8 is_room_size_valid(s16 x0, s16 y0, s16 x1, s16 y1, u8 owner);
u8 check_room_access(u8 owner);
void blank_wall_around_blueprint(u8 owner, u8 blank);
u8 is_tile_near_half_wall(u8 x, u8 y, u8 x0, u8 y0, u8 x1, u8 y1);
void set_expertise_room(RoomInfo *room);

extern "C" {

extern s16 frame_bounds[8];
}

// Draws the blueprint x0,y0 to x1,y1 (inclusive tiles) on the local player's
// view: floor tiles, marked invalid where the room cannot go (also setting
// othervar.blueprint_has_invalid), and the outline, in one colour at invalid tiles. mode 2
// flashes the outline alone, as for an existing room picked for editing. Does
// nothing for other owners.
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

    saved = lbDisplay.draw_flags;
    palette = 0;
    if (owner != player)
        return;
    lbDisplay.draw_flags = 4;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
    LbTextSetWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
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
            give_screen_xy_of_cell_at_column_row(window.current_window, x, y);
            sx = info.cell_x;
            sy = info.cell_y;
            set_all_refresh_zones_in_window(sx, sy - resvars.tile_height, sx + resvars.tile_width, sy, 3);
            if (!(is_room_size_valid(x0, y0, x1, y1, owner) && is_valid_no_room((y << 7) + x) &&
                  is_tile_valid((y << 7) + x, owner)) ||
                is_tile_near_half_wall(x, y, x0, y0, x1, y1)) {
                LbSpriteDraw(sx, sy - blocks[66].info->height, blocks[66].info);
                othervar.blueprint_has_invalid = 1;
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
                give_screen_xy_of_cell_at_column_row(window.current_window, x, y - 1);
                sx = info.cell_x;
                sy = info.cell_y;
                frame_bounds[0] = frame_bounds[1] = 999;
                frame_bounds[2] = frame_bounds[3] = 0;
                tx = x;
                ty = y;
                if (mode != 2) {
                    if (!((blocks[cell->wall_n].flags & 0x10) || (blocks[cell->wall_w].flags & 0x10))) {
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
                if (mode != 2 && (!(is_valid_no_room((ty << 7) + tx) && is_tile_valid((ty << 7) + tx, owner) &&
                                    is_room_size_valid(x0, y0, x1, y1, owner)) ||
                                  is_tile_near_half_wall(x, y, x0, y0, x1, y1)))
                    draw_a_one_colour_frame(start[blueprint[piece + base].anim].frame, sx, sy, color.red,
                                            start[blueprint[piece + base].anim].flags);
                else
                    draw_a_frame(start[blueprint[piece + base].anim].frame, sx, sy, palette,
                                 start[blueprint[piece + base].anim].flags);
                LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
                LbTextSetWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
                set_all_refresh_zones_in_window(frame_bounds[0], frame_bounds[1], frame_bounds[2], frame_bounds[3], 3);
            }
        }
    }
    lbDisplay.draw_flags = saved;
}

// Puts the blueprint outline x0,y0 to x1,y1 (inclusive tiles) on the map as
// small things, and with set_overlay nonzero marks its floor with the
// blueprint overlay (with a centre marker for even sizes).
// FUNCTION: 0x00046174
void mark_blueprint_on_map(s16 x0, s16 y0, s16 x1, s16 y1, u8 set_overlay)
{
    u16 block;
    u16 base;
    u16 width;
    u16 height;
    s16 y;
    s16 x;
    u8 piece;
    Map *cell;

    othervar.full_redraw = 1;
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
            if (set_overlay && y != y1 + 1 && x != x1 + 1) {
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
                if (!((blocks[cell->wall_n].flags & 0x10) || (blocks[cell->wall_w].flags & 0x10))) {
                    place_small_thing((s16)((y << 7) + x), 0, 0);
                    place_small_thing((y << 7) + x, (u16)piece + base, 0);
                } else {
                    place_small_thing((y << 7) + x, (u16)piece + base, 0);
                }
            }
        }
    }
}

// Clears owner's build state to start a blueprint for a room of type type.
// FUNCTION: 0x000464bc
void init_blueprint(u8 owner, u8 type)
{
    BuildState *build;

    build = &allhosps[owner].build;
    memset(build, 0, 0x3d);
    build->drag = 0;
    build->room_type = type;
}

// Applies the mouse at tile to owner's blueprint. While dragging, the first
// call picks what the drag does by where it starts (outside: a new rectangle;
// corner or edge: resize; inside: move) and later calls apply it. Otherwise it
// ends any drag and, for the local player, sets the cursor for what a drag
// from tile would do.
// FUNCTION: 0x00046514
void modify_blueprint(u8 owner, u8 dragging, u16 tile)
{
    BuildState *build;
    u8 x;
    u8 y;
    u8 on_x0;
    u8 on_y0;
    u8 on_x1;
    u8 on_y1;

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
            build->draw_mode = 1;
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
        case 3: {
            u8 dx;
            u8 dy;

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
        }
    } else {
        switch (allhosps[owner].build.drag) {
        case 0:
            if (othervar.cur_player == player) {
                if (!(x >= build->x0 && x <= build->x1 && y >= build->y0 && y <= build->y1))
                    othervar.pointer_sprite = PTR_RESIZE_ROOM;
                else if ((((on_x0 && on_y0) || (on_x0 && on_y1)) || (on_x1 && on_y0)) || (on_x1 && on_y1))
                    othervar.pointer_sprite = PTR_RESIZE_CORNER;
                else if (on_x0 || on_x1)
                    othervar.pointer_sprite = PTR_RESIZE_X_EDGE;
                else if (on_y0 || on_y1)
                    othervar.pointer_sprite = PTR_RESIZE_Y_EDGE;
                else
                    othervar.pointer_sprite = PTR_MOVE_ROOM;
                set_mouse_sprite(othervar.pointer_sprite);
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

// Places owner's blueprint as the room's floor plan, removes rat holes and
// litter in the area (cleanse_area), and moves to the door placing stage. Returns 0 (with an advisor
// message for the local player) if the room is invalid there or would cut off
// other rooms from the entrance, else 1.
// FUNCTION: 0x00046b54
u8 commit_blueprint(u8 owner)
{
    BuildState *build;
    u8 width;
    u8 x;
    u8 y;
    RoomInfo *room;
    u8 height;

    build = &allhosps[owner].build;
    if (!is_valid_room(build->x0, build->y0, build->x1, build->y1, owner)) {
        if (othervar.cur_player == player) {
            advisor.forced_advice_id = 4;
            set_mouse_mode(MM_BLUEPRINT);
        }
        return 0;
    }
    if (!check_room_access_with_blueprint(owner)) {
        if (othervar.cur_player == player) {
            advisor.forced_advice_id = 5;
            set_mouse_mode(MM_BLUEPRINT);
        }
        return 0;
    }
    room = &room_info[build->room];
    build->door_kind = rooms[build->room_type].door_kind;
    allhosps[owner].build.draw_mode = 0;
    build->saved_overlay = map[(s16)build->x0 + ((s16)build->y0 << 7)].overlay;
    mark_blueprint_on_map(build->x0, build->y0, build->x1, build->y1, 1);
    room->created_turn = actual_game_turn;
    room->pos = (s16)(build->x0 + (build->y0 << 7));
    room->width = build->x1 - build->x0 + 1;
    room->height = build->y1 - build->y0 + 1;
    cleanse_area(room->owner, room->pos % 128, room->pos / 128, room->width, room->height);
    blank_wall_around_blueprint(owner, 1);
    build->state = 1;
    if (othervar.cur_player == player) {
        set_edit_room_requester_mode(2);
        set_mouse_mode(MM_PLACE_DOOR);
    }
    width = build->x1 - build->x0 + 1;
    height = build->y1 - build->y0 + 1;
    for (y = build->y0; y < build->y0 + height; y++) {
        for (x = build->x0; x < build->x0 + width; x++) {
            map[(s16)x + ((s16)y << 7)].floor &= 0xaf;
        }
    }
    return 1;
}

// Takes owner's blueprint off the map (outline and overlay) and back to the
// dragging stage.
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
            map[(s16)x + ((s16)y << 7)].overlay = build->saved_overlay;
        }
    }
    build->draw_mode = 1;
    allhosps[owner].build.state = 0;
    othervar.full_redraw = 1;
}

// Returns 1 if owner may build on tile: only people stand on it, its map
// flags & 7 are clear, and it is owner's land not marked MAP_OUTDOOR.
// Otherwise returns 0 (setting advisor.flags flag 2 unless the flags failed).
// FUNCTION: 0x00046f88
u8 is_tile_valid(u16 tile, u8 owner)
{
    Thing *occupant;

    for (occupant = &thing[map[tile].first]; occupant > thing; occupant = &thing[occupant->cell_next]) {
        if (occupant->type != THING_PATIENT && occupant->type != THING_STAFF) {
            advisor.flags |= 2;
            return 0;
        }
    }
    if (map[tile].flags & 7) {
        return 0;
    }
    if (!(map[tile].owner_flags & MAP_OUTDOOR) && (map[tile].owner_flags & (1 << owner))) {
        return 1;
    }
    advisor.flags |= 2;
    return 0;
}

// Returns 1 if the blueprint x0,y0 to x1,y1 (inclusive) is a valid room for
// owner: at least the room type's minimum size, overlapping no room, on
// buildable tiles, and away from half-height walls.
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
    if (width < rooms[type].min_width || height < rooms[type].min_height) {
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

// Returns nonzero if a door of door type type at tile would open onto a tile
// outside any room that is not marked MAP_OUTDOOR.
// FUNCTION: 0x00047248
u8 is_valid_door(u16 tile, u8 owner, u8 type)
{
    s32 pos;

    pos = tile + (s16)doortype[type % 4 + 4].door_offset;
    return !(map[pos].owner_flags & MAP_OUTDOOR) && which_room(pos) == 0;
}

// Returns 1 if tile is not in a room; otherwise returns 0 and sets
// advisor.flags flag 2.
// FUNCTION: 0x000472c8
u8 is_valid_no_room(u16 tile)
{
    if (which_room(tile) == 0) {
        return 1;
    }
    advisor.flags |= 2;
    return 0;
}

// Turns owner's room being edited (build.room) back into a blueprint: lifts
// its objects into the build list (counting them back into the local player's
// stock), deletes the room from the map and puts its outline back up for
// dragging.
// FUNCTION: 0x00047308
void edit_blueprint(u8 owner)
{
    BuildState *build;
    RoomInfo *room;
    u8 i;
    Thing *obj;

    build = &allhosps[owner].build;
    room = &room_info[allhosps[owner].build.room];
    i = 0;
    while ((obj = &thing[room->first_object]) > thing) {
        if (othervar.cur_player == player && (!obj->pair || objects[thing[obj->pair].subtype].next_part_index == -1))
            gamevar.buy_list.to_place[obj->subtype]++;
        remove_object_from_map(obj);
        for (; i < 20; i++) {
            if (build->objects[i] == 0) {
                build->objects[i] = obj - thing;
                break;
            }
        }
    }
    if (room->door_tile && room->door_tile < 0x4000)
        build->saved_overlay = map[room->door_tile].overlay;
    room_deleted_update_listhead(room);
    delete_room(room);
    uncommit_blueprint(owner);
    repair_invis_walls_around_room(room);
    room->pos = 0;
    room->door_tile = 0;
    room->door_inside_tile = 0;
    room->width = 0;
    room->height = 0;
    room->state = RS_BLUEPRINT;
    if (othervar.cur_player == player) {
        set_edit_room_requester_mode(0);
        set_mouse_mode(MM_BLUEPRINT);
    }
}

// Hover in edit-room mode: records the local player's room at tile for
// highlighting (build.edit_room), or, outside rooms, records the pointed thing
// (othervar.pointed_thing) in othervar.pickup_thing if it is a movable object of the player's in
// the corridor, or a person using one.
// FUNCTION: 0x0004750c
void compute_room_pointed(u16 tile)
{
    RoomInfo *room;
    BuildState *build;
    Thing *pointed;

    room = which_room(tile);
    build = &allhosps[player].build;
    if (room && room->type != ROOM_DESTROYED) {
        build->edit_room = room - room_info;
        build->draw_mode = 2;
        build->x0 = room->pos % 128;
        build->y0 = room->pos / 128;
        build->x1 = room->pos % 128 + room->width - 1;
        build->y1 = room->pos / 128 + room->height - 1;
    } else {
        build->edit_room = 0;
        build->draw_mode = 0;
        pointed = &thing[othervar.pointed_thing];
        if (pointed > thing) {
            switch (pointed->type) {
            case THING_OBJECT:
                if (pointed->room_id == 0 && !(objects[pointed->subtype].flags & 1) && pointed->owner == player)
                    othervar.pickup_thing = othervar.pointed_thing;
                break;
            case THING_PATIENT:
            case THING_STAFF:
                if (pointed->object && (u16)(pointed - thing) == thing[pointed->object].user &&
                    thing[pointed->object].room_id == 0 && pointed->owner == player)
                    othervar.pickup_thing = othervar.pointed_thing;
                break;
            }
        }
    }
}

// Returns 1 if every finished room of owner's (except receptions) and both
// sides of each reception desk can be reached from the entrance, else 0.
// FUNCTION: 0x00047778
u8 check_room_access(u8 owner)
{
    u16 pos;
    RoomInfo *room;
    Thing *obj;

    pos = room_info[allhosps[owner].entrances[0]].door_tile;
    for (room = room_info + 1; room < end_room_info; room++) {
        if (((!(room->type == ROOM_FREE || room->state == RS_BLUEPRINT) && room->owner == owner) &&
             room->type != ROOM_ENTRANCE) &&
            room->type != ROOM_RECEPTION) {
            if (!check_points_connected(pos, room->door_tile))
                return 0;
            pos = room->door_tile;
        }
    }
    for (obj = thing + 1; obj < end_things; obj++) {
        if ((obj->type == THING_OBJECT && obj->subtype == OBJ_RECEPTION_DESK) && obj->owner == owner) {
            if (!check_points_connected(pos, obj->use_pos[0]))
                return 0;
            if (!check_points_connected(pos, obj->use_pos[1]))
                return 0;
        }
    }
    return 1;
}

// check_room_access with owner's blueprint walled off.
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

// Adds (blank nonzero) or removes invisible walls around owner's blueprint,
// for path checks.
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
            insert_blank_wall(&map[(s16)x + ((s16)y0 << 7)], 0);
            insert_blank_wall(&map[(s16)x + ((s16)(y1 + 1) << 7)], 0);
        } else {
            remove_blank_wall(&map[(s16)x + ((s16)y0 << 7)], 0);
            remove_blank_wall(&map[(s16)x + ((s16)(y1 + 1) << 7)], 0);
        }
    }
    for (y = y0; y <= y1; y++) {
        if (blank) {
            insert_blank_wall(&map[(s16)x0 + ((s16)y << 7)], 1);
            insert_blank_wall(&map[(s16)(x1 + 1) + ((s16)y << 7)], 1);
        } else {
            remove_blank_wall(&map[(s16)x0 + ((s16)y << 7)], 1);
            remove_blank_wall(&map[(s16)(x1 + 1) + ((s16)y << 7)], 1);
        }
    }
}

// Returns 1 once no things are on the room's floor tiles, else 0.
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
    x1 = x0 + room->width;
    y1 = y0 + room->height;
    for (x = x0; x < x1; x++) {
        for (y = y0; y < y1; y++) {
            if (map[((s16)y << 7) + (s16)x].first > 0) {
                return 0;
            }
        }
    }
    return 1;
}

// For each active player whose room is waiting to be cleared (build state 2),
// retries commit_doors.
// FUNCTION: 0x00047c58
void process_blueprint_zones(void)
{
    s32 count;
    s8 i;

    count = 1;
    if (statvar.num_players > 1) {
        count = 4;
    }
    for (i = count - 1; i >= 0; i--) {
        if (statvar.num_players > 1 && player_info[i].active == 0) {
            continue;
        }
        othervar.cur_player = i;
        if (allhosps[i].build.state == 2) {
            commit_doors(i);
        }
    }
}

// Finishes owner's new room once no things are left on its floor: builds it,
// prices it and makes it the player's room of its kind, then for the local
// player opens the object placing stage. Until then leaves the build waiting
// (state 2).
// FUNCTION: 0x00047cf8
void commit_doors(u8 owner)
{
    allhosps[owner].build.state = 2;
    if (is_room_clear_yet(&room_info[allhosps[owner].build.room])) {
        allhosps[owner].build.state = 0;
        build_all_rooms();
        price_all_rooms((u16)allhosps[owner].build.room_type, owner);
        if (rooms[allhosps[owner].build.room_type].door_kind == 0)
            init_double_doors(&room_info[allhosps[owner].build.room]);
        init_edit_room(&room_info[allhosps[owner].build.room]);
        listhead[owner].rooms[allhosps[owner].build.room_type] = allhosps[owner].build.room;
        if (owner == player)
            init_place_object(next_object_to_build(), 0, allhosps[owner].build.room, 0);
    }
}

// Sends peep standing inside its owner's blueprint to a reachable tile just
// outside it, trying the nearest sides first. Returns 1 if a tile was found
// (peep is then in PS_BP_GOTO or SS_BP_GOTO), 0 if peep is not inside or none
// was found.
// FUNCTION: 0x00047ed0
u8 move_peep_from_blueprint(Thing *peep)
{
    s8 i;
    u8 x;
    u8 y;
    u8 side;
    u8 best;
    u8 dist[4];
    BuildState *build;

    build = &allhosps[peep->owner].build;
    x = peep->x.tile;
    y = peep->y.tile;
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
            if (try_move_peep_to_wall(peep, side))
                return 1;
        }
    } while (best != 0xff);
    return 0;
}

// Tries tiles along side side (0 top, 1 right, 2 bottom, 3 left) outside the
// blueprint, starting level with peep, then at random, sending peep to the
// first one that works. Returns 1 on success.
// FUNCTION: 0x00047ffc
u8 try_move_peep_to_wall(Thing *peep, u8 side)
{
    u8 i;
    BuildState *build;
    u8 count;
    u8 base;
    u8 idx;
    u8 cells[128];

    build = &allhosps[peep->owner].build;
    if (side == 0 || side == 2) {
        count = build->x1 - build->x0 + 1;
        base = build->x0;
        idx = peep->x.tile - base;
    } else {
        count = build->y1 - build->y0 + 1;
        base = build->y0;
        idx = peep->y.tile - base;
    }
    for (i = 0; i < count; i++)
        cells[i] = base + i;
    for (;;) {
        switch (side) {
        case 0:
            if (try_move_peep_cell(peep, cells[idx], build->y0 - 1))
                return 1;
            break;
        case 1:
            if (try_move_peep_cell(peep, build->x1 + 1, cells[idx]))
                return 1;
            break;
        case 2:
            if (try_move_peep_cell(peep, cells[idx], build->y1 + 1))
                return 1;
            break;
        case 3:
            if (try_move_peep_cell(peep, build->x0 - 1, cells[idx]))
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

// If tile x,y is outside any room and reachable from the entrance, sends peep
// there (PS_BP_GOTO or SS_BP_GOTO, saving its state) and returns 1; else 0.
// FUNCTION: 0x00048224
u8 try_move_peep_cell(Thing *peep, u8 x, u8 y)
{
    u16 pos;
    u16 tile;

    pos = room_info[allhosps[peep->owner].entrances[0]].door_tile;
    tile = (s16)(x + (y << 7));
    if ((!(map[tile].owner_flags & MAP_OUTDOOR) && !which_room(tile)) && check_points_connected(tile, pos)) {
        peep->saved_state = peep->state;
        peep->saved_dest_x = x;
        peep->saved_dest_y = y;
        if (peep->type == THING_PATIENT)
            peep->state = PS_BP_GOTO;
        else
            peep->state = SS_BP_GOTO;
        peep->countdown = 0;
        return 1;
    }
    return 0;
}

// Returns 1 if a blueprint x0,y0 to x1,y1 (inclusive) meets the minimum size
// of owner's room type, else 0; tracks the result in advisor.flags flag 4.
// FUNCTION: 0x0004831c
u8 is_room_size_valid(s16 x0, s16 y0, s16 x1, s16 y1, u8 owner)
{
    u16 width;
    u16 height;
    u16 type;

    width = x1 - x0 + 1;
    height = y1 - y0 + 1;
    type = allhosps[owner].build.room_type;
    if (width < rooms[type].min_width || height < rooms[type].min_height) {
        advisor.flags |= 4;
        return 0;
    }
    advisor.flags &= ~4;
    return 1;
}

// Abandons owner's room under construction: refunds its depreciated cost if
// it was paid for, takes down the blueprint, restarts its queue and frees
// the room_info entry and build state.
// FUNCTION: 0x000483c4
void delete_blueprint_room(u8 owner)
{
    BuildState *build;
    RoomInfo *room;

    build = &allhosps[owner].build;
    room = &room_info[allhosps[owner].build.room];
    if (room->paid_for) {
        change_balance(TX_BUILD_ROOM, rooms[room->type].cost * room_depreciation() / 100, 0, (u16)room->owner);
        room->paid_for = 0;
    }
    if (allhosps[owner].build.draw_mode == 0)
        uncommit_blueprint(owner);
    allhosps[owner].build.draw_mode = 0;
    allhosps[owner].build.state = 0;
    restart_all_queue_peeps(room);
    remove_expertise_room(room);
    memset(room, 0, sizeof(RoomInfo));
    memset(build, 0, sizeof(BuildState));
}

// Draws each player's blueprint (build mode 1), or flashes the room picked for
// editing (mode 2, for one frame).
// FUNCTION: 0x00048504
void draw_overdrawn_blueprints(void)
{
    u32 i;

    for (i = 0; i < 4; i++) {
        BuildState *build;
        u8 mode;

        build = &allhosps[i].build;
        mode = allhosps[i].build.draw_mode;
        switch (mode) {
        case 2:
            allhosps[i].build.draw_mode = 0;
        case 1:
            draw_blueprint_area(build->x0, build->y0, build->x1, build->y1, i, mode);
            break;
        }
    }
}

// Takes a free room_info entry for a new room of type type for owner and makes
// it the build state's room (build.room). Does nothing if there is none.
// FUNCTION: 0x000485e0
void allocate_roominfo(const u8 owner, const u8 type)
{
    BuildState *build;
    RoomInfo *room;

    build = &allhosps[owner].build;
    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->type == ROOM_FREE) {
            build->room = room - room_info;
            memset(room, 0, sizeof(RoomInfo));
            room->owner = owner;
            room->type = type;
            room->state = RS_BLUEPRINT;
            room->created_turn = actual_game_turn;
            room->queue_capacity = 6;
            set_expertise_room(room);
            break;
        }
    }
}

// Takes a free room_info entry as a reception (a 1x1 room at tile 0x3fff) for
// owner. Returns it, or 0 if room_info is full.
// FUNCTION: 0x000486a8
RoomInfo *allocate_reception_roominfo(u8 owner)
{
    RoomInfo *room;

    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->type == ROOM_FREE) {
            room->type = ROOM_RECEPTION;
            room->pos = 0x3fff;
            room->width = room->height = 1;
            room->state = 0;
            room->owner = owner;
            room->queue_capacity = 0x14;
            return room;
        }
    }
    return 0;
}

// Returns 1 if tile x,y has a half-height wall on the sides inside the
// rectangle whose far corner is x1,y1 (x0 and y0 unused).
// FUNCTION: 0x0004873c
u8 check_near_half_wall(u8 x, u8 y, u8 x0, u8 y0, u8 x1, u8 y1)
{
    if (y != y1 + 1 && (blocks[map[(s16)x + ((s16)y << 7)].wall_w].flags & 4))
        return 1;
    if (x != x1 + 1 && (blocks[map[(s16)x + ((s16)y << 7)].wall_n].flags & 4))
        return 1;
    return 0;
}

// Returns 1 if tile x,y of the blueprint x0,y0 to x1,y1, or its neighbour
// across the blueprint's far edges, has a half-height wall.
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
