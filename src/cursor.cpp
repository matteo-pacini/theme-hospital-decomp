#include "hosp.h"

extern "C" {
void fn_000c9be0(int x0, int y0, int x1, int y1, u8 color);
}

void draw_cell_no_editor(s32 x, s32 y, u8 colour);
u32 compute_plant_frame(Thing *w);
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
    u8 unknown3[3];
};

struct NextPart {
    u8 kind;
    NextPartDir dirs[4];
};

extern NextPart next_part[];

// FUNCTION: 0x000b10b0
void draw_cell_no(s32 cell, u8 colour)
{
    s32 b;
    s32 a;
    s32 y;
    s32 x;
    s32 col;
    s32 row;

    row = cell / 128;
    col = cell % 128;
    x = col * (resvars.f4 / 2) - resvars.f4 / 2 * row - window.f0[window.unknown2eb] + window.f90[window.unknown2eb]
        + resvars.f4 / 2;
    y = col * (resvars.f3 / 2) + resvars.f3 / 2 * row - window.fc[window.unknown2eb] + window.f9c[window.unknown2eb]
        - resvars.f3;
    find_common_window(0, 0, lbDisplay.graphics_screen_width, lbDisplay.unknown1c - 0x5c, window.zone_x[window.unknown2eb],
                       window.zone_y[window.unknown2eb], window.zone_w[window.unknown2eb], window.zone_h[window.unknown2eb]);
    find_common_window(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height, window.zone_x[window.unknown2eb], window.zone_y[window.unknown2eb],
                       window.zone_w[window.unknown2eb], window.zone_h[window.unknown2eb]);
    find_common_window(info.unknown68, info.unknown6c, info.unknown70, info.unknown74, window.unknown2c3, window.unknown2c7,
                       window.unknown2cb, window.unknown2cf);
    if (info.unknown78 == 0) {
        LbScreenSetGraphicsWindow(info.unknown68, info.unknown6c, info.unknown70 - info.unknown68,
                                  info.unknown74 - info.unknown6c);
        x -= info.unknown68;
        y -= info.unknown6c;
        a = x - resvars.f4 / 2;
        b = y + resvars.f3;
        draw_cell_no_editor(a, b, colour);
    }
}

// FUNCTION: 0x000b1318
void draw_cell_no_editor(s32 x, s32 y, u8 colour)
{
    if (-(resvars.f4 / 2) <= x && resvars.f4 / 2 + lbDisplay.graphics_screen_width > x && -resvars.f3 <= y && lbDisplay.unknown1c - 0x5c > y) {
        fn_000c9be0(x, y, (resvars.f4 - 1) / 2 + x - 1, y + (resvars.f3 - 1) / 2, colour);
        fn_000c9be0((resvars.f4 - 1) / 2 + x, y + (resvars.f3 - 1) / 2, x, y + (resvars.f3 - 1), colour);
        fn_000c9be0(x, resvars.f3 - 1 + y, x - (resvars.f4 - 1) / 2, y + (resvars.f3 - 1) / 2, colour);
        fn_000c9be0(x - (resvars.f4 - 1) / 2, y + (resvars.f3 - 1) / 2, x, y, colour);
        set_all_refresh_zones_in_window(x - (resvars.f4 - 1) / 2, y, (resvars.f4 - 1) / 2 + x - 1, resvars.f3 - 1 + y, 3);
    }
}

// FUNCTION: 0x000b14f0
void draw_object_cursor(void)
{
    u8 flag;
    u8 flip;
    u8 flip2;
    u8 dir;
    s8 part;
    u8 dir2;
    u16 tile2;
    u16 kind;
    u16 tile;
    u16 kind2;
    u32 frame2;
    RoomInfo *room;
    s32 row;
    s32 col;
    s32 x;
    s32 y;
    s32 x2;
    s32 y2;
    u32 frame;
    u8 *remap;

    tile2 = 0;
    kind2 = 0;
    kind = gamevar.unknown25;
    dir2 = 0;
    dir = gamevar.f28;
    part = objects[gamevar.unknown25].unknown17;
    flag = gamevar.f29;
    if (othervar.f28 == othervar.f2a && !flag)
        goto draw;
    gamevar.f29 = 0;
    switch (gamevar.unknown26) {
    case 1:
        tile = place_object_wall_hug(othervar.f28 + gamevar.f31, kind);
        dir = gamevar.f28;
        break;
    case 0:
        tile = place_object_restrict_to_room(othervar.f28 + gamevar.f31, 0, kind, dir);
        if (part != -1)
            tile = place_object_restrict_to_room(tile, next_part[part].dirs[dir].offset, next_part[part].kind,
                                                 next_part[part].dirs[dir].dir);
        break;
    case 2:
        tile = othervar.f28 + gamevar.f31;
        break;
    default:
        break;
    }
    if (tile == gamevar.f23 && !flag)
        goto draw;
    gamevar.f23 = tile;
    gamevar.unknown2a = can_place_object(tile, kind, dir, player);
    if (!gamevar.unknown2a)
        goto draw;
    if (part != -1) {
        kind2 = next_part[part].kind;
        tile2 = tile + next_part[part].dirs[dir].offset;
        dir2 = next_part[part].dirs[dir].dir;
        gamevar.unknown2a = can_place_object(tile2, kind2, dir2, player);
        if (!gamevar.unknown2a)
            goto draw;
    }
    if (gamevar.unknown26 == 2) {
        room = which_room(tile);
        if (room) {
            if ((objects[kind].unknown20[2] & 8) && room->type != 6) {
                gamevar.unknown2a = corridor_object_room_nav_check(tile, kind, dir, room);
            } else {
                gamevar.f23 = tile;
                gamevar.unknown2a = 0;
            }
        } else {
            gamevar.unknown2a = corridor_object_nav_check(tile, kind, dir);
        }
    } else {
        gamevar.unknown2a = place_object_nav_check(&room_info[gamevar.unknown2f], tile, kind, dir, tile2, kind2, dir2);
    }
draw:
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    if (gamevar.unknown2a) {
        remap = 0;
        advisor.f11f &= ~2;
    } else {
        remap = bw_remap;
        advisor.f11f |= 2;
    }
    row = gamevar.f23 / 128;
    col = gamevar.f23 % 128;
    x = resvars.f4 * col / 2 - resvars.f4 * row / 2 - window.f0[window.unknown2eb] + window.f90[window.unknown2eb];
    y = resvars.f3 * col / 2 + resvars.f3 * row / 2 - window.fc[window.unknown2eb] + window.f9c[window.unknown2eb]
        + resvars.f3 / 2;
    switch (kind) {
    case 0x11:
        frame = get_frame_for_placing_bomb();
        flip = 0;
        break;
    case 0x2d:
        frame = get_frame_for_placing_plant();
        flip = 0;
        break;
    default:
        frame = start[objects[kind].unknownE + dir].frame;
        flip = start[objects[kind].unknownE + dir].flags;
        break;
    }
    init_object_drag_shape();
    compute_object_drag_shape(gamevar.f23, kind, dir);
    if (part != -1) {
        u16 tile3;

        tile3 = gamevar.f23 + next_part[part].dirs[dir].offset;
        kind = next_part[part].kind;
        dir2 = next_part[part].dirs[dir].dir;
        compute_object_drag_shape(tile3, kind, dir2);
        row = tile3 / 128;
        col = tile3 % 128;
        x2 = resvars.f4 * col / 2 - resvars.f4 * row / 2 - window.f0[window.unknown2eb] + window.f90[window.unknown2eb];
        y2 = resvars.f3 * col / 2 + resvars.f3 * row / 2 - window.fc[window.unknown2eb] + window.f9c[window.unknown2eb]
            + resvars.f3 / 2;
        frame2 = start[objects[kind].unknownE + dir2].frame;
        flip2 = start[objects[kind].unknownE + dir2].flags;
        if (next_part[part].dirs[dir].unknown3[2]) {
            draw_from_frame(frame2, x2, y2, remap, flip2);
            draw_from_frame(frame, x, y, remap, flip);
        } else {
            draw_from_frame(frame, x, y, remap, flip);
            draw_from_frame(frame2, x2, y2, remap, flip2);
        }
    } else {
        draw_from_frame(frame, x, y, remap, flip);
    }
}

// FUNCTION: 0x000b1c68
u32 get_frame_for_placing_bomb(void)
{
    u8 i;
    BuildState *bs;
    Thing *w;

    bs = &allhosps[othervar.ff].build;
    for (i = 0; i < 20; i++) {
        w = &thing[bs->objects[i]];
        if (w > thing) {
            if (w->subtype == 0x11 && w->owner == player)
                return start[w->f64 + 0x9bf].frame;
        }
    }
    return start[0x9bf].frame;
}

// FUNCTION: 0x000b1d44
u32 get_frame_for_placing_plant(void)
{
    u8 i;
    BuildState *bs;
    Thing *w;

    bs = &allhosps[othervar.ff].build;
    for (i = 0; i < 20; i++) {
        w = &thing[bs->objects[i]];
        if (w > thing) {
            if (w->subtype == 0x2d && w->owner == player)
                return compute_plant_frame(w);
        }
    }
    return start[0x3cf].frame;
}
