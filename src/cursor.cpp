#include "hosp.h"

extern "C" {
void fn_000c9be0(int x1, int y1, int x2, int y2, u8 colour);
}

void draw_cell_no_editor(s32 x, s32 y, u8 colour);
u32 compute_plant_frame(Thing *plant);
u32 get_frame_for_placing_bomb(void);
u32 get_frame_for_placing_plant(void);
u16 place_object_wall_hug(u16 pos, u8 kind);
u16 place_object_restrict_to_room(u16 pos, u16 offset, u8 kind, u8 dir);
u8 can_place_object(u16 tile, u16 kind, u8 dir, u8 owner);
u8 corridor_object_room_nav_check(u16 pos, u8 kind, u8 dir, RoomInfo *room);
u8 corridor_object_nav_check(u16 pos, u8 kind, u8 dir);
u8 place_object_nav_check(RoomInfo *room, u16 pos, u8 kind, u8 dir, u16 pos2, u8 kind2, u8 dir2);
void init_object_drag_shape(void);
void compute_object_drag_shape(u16 pos, u8 kind, u8 angle);

struct NextPartDir {
    u8 dir;
    u16 offset;
    u8 unknown3[2];
    // 0x5: Nonzero to draw the second part before the first.
    u8 draw_part_first;
};

struct NextPart {
    u8 kind;
    NextPartDir dirs[4];
};

extern NextPart next_part[];

// Outlines map tile cell (y * 128 + x) in colour colour on the current view,
// clipped to the view.
// FUNCTION: 0x000b10b0
void draw_cell_no(s32 cell, u8 colour)
{
    s32 top_y;
    s32 top_x;
    s32 y;
    s32 x;
    s32 col;
    s32 row;

    row = cell / 128;
    col = cell % 128;
    x = col * (resvars.tile_width / 2) - resvars.tile_width / 2 * row - window.scroll_x[window.current_window] + window.center_x[window.current_window]
        + resvars.tile_width / 2;
    y = col * (resvars.tile_height / 2) + resvars.tile_height / 2 * row - window.scroll_y[window.current_window] + window.center_y[window.current_window]
        - resvars.tile_height;
    find_common_window(0, 0, lbDisplay.graphics_screen_width, lbDisplay.physical_screen_height - 0x5c, window.zone_x[window.current_window],
                       window.zone_y[window.current_window], window.zone_x1[window.current_window], window.zone_y1[window.current_window]);
    find_common_window(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height, window.zone_x[window.current_window], window.zone_y[window.current_window],
                       window.zone_x1[window.current_window], window.zone_y1[window.current_window]);
    find_common_window(info.clip_x0, info.clip_y0, info.clip_x1, info.clip_y1, window.clip_x0, window.clip_y0,
                       window.clip_x1, window.clip_y1);
    if (info.clip_empty == 0) {
        LbScreenSetGraphicsWindow(info.clip_x0, info.clip_y0, info.clip_x1 - info.clip_x0,
                                  info.clip_y1 - info.clip_y0);
        x -= info.clip_x0;
        y -= info.clip_y0;
        top_x = x - resvars.tile_width / 2;
        top_y = y + resvars.tile_height;
        draw_cell_no_editor(top_x, top_y, colour);
    }
}

// Draws a tile-sized diamond outline in colour colour with its top corner at
// screen x,y, if on screen, and flags the area for redraw.
// FUNCTION: 0x000b1318
void draw_cell_no_editor(s32 x, s32 y, u8 colour)
{
    if (-(resvars.tile_width / 2) <= x && resvars.tile_width / 2 + lbDisplay.graphics_screen_width > x && -resvars.tile_height <= y && lbDisplay.physical_screen_height - 0x5c > y) {
        fn_000c9be0(x, y, (resvars.tile_width - 1) / 2 + x - 1, y + (resvars.tile_height - 1) / 2, colour);
        fn_000c9be0((resvars.tile_width - 1) / 2 + x, y + (resvars.tile_height - 1) / 2, x, y + (resvars.tile_height - 1), colour);
        fn_000c9be0(x, resvars.tile_height - 1 + y, x - (resvars.tile_width - 1) / 2, y + (resvars.tile_height - 1) / 2, colour);
        fn_000c9be0(x - (resvars.tile_width - 1) / 2, y + (resvars.tile_height - 1) / 2, x, y, colour);
        set_all_refresh_zones_in_window(x - (resvars.tile_width - 1) / 2, y, (resvars.tile_width - 1) / 2 + x - 1, resvars.tile_height - 1 + y, 3);
    }
}

// Draws the object being placed (gamevar.place_kind, and its second part) at the
// mouse tile, greyed where it cannot go. When the mouse tile or angle changed,
// first recomputes its tile (gamevar.place_tile) for the placing mode
// (gamevar.place_mode 0 free in the room, 1 against a wall, 2 moving) and
// whether it may be placed there (gamevar.place_ok), and marks the tiles it
// covers.
// FUNCTION: 0x000b14f0
void draw_object_cursor(void)
{
    u8 force;
    u8 flip;
    u8 flip2;
    u8 dir;
    s8 part;
    u8 dir2;
    u16 tile2;
    u16 kind;
    u16 tile;
    u16 kind2;
    u32 part_frame;
    RoomInfo *room;
    s32 row;
    s32 col;
    s32 x;
    s32 y;
    s32 x2;
    s32 y2;
    u32 obj_frame;
    u8 *remap;

    tile2 = 0;
    kind2 = 0;
    kind = gamevar.place_kind;
    dir2 = 0;
    dir = gamevar.place_dir;
    part = objects[gamevar.place_kind].next_part_index;
    force = gamevar.place_redraw;
    if (othervar.mouse_cell == othervar.prev_mouse_cell && !force)
        goto draw;
    gamevar.place_redraw = 0;
    switch (gamevar.place_mode) {
    case 1:
        tile = place_object_wall_hug(othervar.mouse_cell + gamevar.place_offset, kind);
        dir = gamevar.place_dir;
        break;
    case 0:
        tile = place_object_restrict_to_room(othervar.mouse_cell + gamevar.place_offset, 0, kind, dir);
        if (part != -1)
            tile = place_object_restrict_to_room(tile, next_part[part].dirs[dir].offset, next_part[part].kind,
                                                 next_part[part].dirs[dir].dir);
        break;
    case 2:
        tile = othervar.mouse_cell + gamevar.place_offset;
        break;
    default:
        break;
    }
    if (tile == gamevar.place_tile && !force)
        goto draw;
    gamevar.place_tile = tile;
    gamevar.place_ok = can_place_object(tile, kind, dir, player);
    if (!gamevar.place_ok)
        goto draw;
    if (part != -1) {
        kind2 = next_part[part].kind;
        tile2 = tile + next_part[part].dirs[dir].offset;
        dir2 = next_part[part].dirs[dir].dir;
        gamevar.place_ok = can_place_object(tile2, kind2, dir2, player);
        if (!gamevar.place_ok)
            goto draw;
    }
    if (gamevar.place_mode == 2) {
        room = which_room(tile);
        if (room) {
            if ((objects[kind].flags & 8) && room->type != ROOM_DESTROYED) {
                gamevar.place_ok = corridor_object_room_nav_check(tile, kind, dir, room);
            } else {
                gamevar.place_tile = tile;
                gamevar.place_ok = 0;
            }
        } else {
            gamevar.place_ok = corridor_object_nav_check(tile, kind, dir);
        }
    } else {
        gamevar.place_ok = place_object_nav_check(&room_info[gamevar.place_room], tile, kind, dir, tile2, kind2, dir2);
    }
draw:
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
    LbTextSetWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
    if (gamevar.place_ok) {
        remap = 0;
        advisor.flags &= ~2;
    } else {
        remap = bw_remap;
        advisor.flags |= 2;
    }
    row = gamevar.place_tile / 128;
    col = gamevar.place_tile % 128;
    x = resvars.tile_width * col / 2 - resvars.tile_width * row / 2 - window.scroll_x[window.current_window] + window.center_x[window.current_window];
    y = resvars.tile_height * col / 2 + resvars.tile_height * row / 2 - window.scroll_y[window.current_window] + window.center_y[window.current_window]
        + resvars.tile_height / 2;
    switch (kind) {
    case 0x11:
        obj_frame = get_frame_for_placing_bomb();
        flip = 0;
        break;
    case 0x2d:
        obj_frame = get_frame_for_placing_plant();
        flip = 0;
        break;
    default:
        obj_frame = start[objects[kind].anim_start + dir].frame;
        flip = start[objects[kind].anim_start + dir].flags;
        break;
    }
    init_object_drag_shape();
    compute_object_drag_shape(gamevar.place_tile, kind, dir);
    if (part != -1) {
        u16 tile3;

        tile3 = gamevar.place_tile + next_part[part].dirs[dir].offset;
        kind = next_part[part].kind;
        dir2 = next_part[part].dirs[dir].dir;
        compute_object_drag_shape(tile3, kind, dir2);
        row = tile3 / 128;
        col = tile3 % 128;
        x2 = resvars.tile_width * col / 2 - resvars.tile_width * row / 2 - window.scroll_x[window.current_window] + window.center_x[window.current_window];
        y2 = resvars.tile_height * col / 2 + resvars.tile_height * row / 2 - window.scroll_y[window.current_window] + window.center_y[window.current_window]
            + resvars.tile_height / 2;
        part_frame = start[objects[kind].anim_start + dir2].frame;
        flip2 = start[objects[kind].anim_start + dir2].flags;
        if (next_part[part].dirs[dir].draw_part_first) {
            draw_from_frame(part_frame, x2, y2, remap, flip2);
            draw_from_frame(obj_frame, x, y, remap, flip);
        } else {
            draw_from_frame(obj_frame, x, y, remap, flip);
            draw_from_frame(part_frame, x2, y2, remap, flip2);
        }
    } else {
        draw_from_frame(obj_frame, x, y, remap, flip);
    }
}

// Returns the frame to draw for a litter bomb being placed: that of the local
// player's bomb in the build list (by its fuse), else the first.
// FUNCTION: 0x000b1c68
u32 get_frame_for_placing_bomb(void)
{
    u8 i;
    BuildState *bs;
    Thing *obj;

    bs = &allhosps[othervar.cur_player].build;
    for (i = 0; i < 20; i++) {
        obj = &thing[bs->objects[i]];
        if (obj > thing) {
            if (obj->subtype == OBJ_LITTER_BOMB && obj->owner == player)
                return start[obj->fuse + 0x9bf].frame;
        }
    }
    return start[0x9bf].frame;
}

// Returns the frame to draw for a plant being placed: that of the local
// player's plant in the build list (by its health), else a healthy plant.
// FUNCTION: 0x000b1d44
u32 get_frame_for_placing_plant(void)
{
    u8 i;
    BuildState *bs;
    Thing *obj;

    bs = &allhosps[othervar.cur_player].build;
    for (i = 0; i < 20; i++) {
        obj = &thing[bs->objects[i]];
        if (obj > thing) {
            if (obj->subtype == OBJ_PLANT && obj->owner == player)
                return compute_plant_frame(obj);
        }
    }
    return start[0x3cf].frame;
}
