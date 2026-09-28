#include "hosp.h"

extern "C" {
extern u8 D_001553d0[];
}

void give_screen_xy_of_cell_at_column_row(const u8 view, const s32 column, const s32 row);
u8 can_place_door(u16 tile, u16 a, u8 b);
u8 is_valid_door(u16 tile, u8 owner, u8 type);

// FUNCTION: 0x00022b90
void temp_place_door(u16 pos, u8 owner)
{
    u8 blocked;
    u8 end_x;
    u8 end_y;
    u8 start_x;
    u8 type;
    u8 flag;
    u8 start_y;
    u8 min_w;
    u8 min_h;
    BuildState *b;
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

    b = &allhosps[owner].build;
    blocked = 0;
    D_001553d0[owner] = 0;
    room = &room_info[b->room];
    switch (b->door_state) {
    case 0:
        min_w = 3;
        min_h = 3;
        end_x = 3;
        end_y = 3;
        start_x = 1;
        start_y = 1;
        flag = 1;
        break;
    case 4:
        min_w = 3;
        min_h = 3;
        end_x = 2;
        end_y = 2;
        start_x = 2;
        start_y = 2;
        flag = 0;
        break;
    case 3:
        min_w = 3;
        min_h = 3;
        end_x = 1;
        end_y = 1;
        start_x = 1;
        start_y = 1;
        flag = 0;
        break;
    default:
        min_w = 3;
        min_h = 3;
        end_x = 2;
        end_y = 2;
        start_x = 1;
        start_y = 1;
        flag = 1;
        break;
    }
    cell0 = &map[room->pos];
    if (room->f2 > min_h) {
        for (i = start_y, best = 9999; i <= room->f2 - end_y; i++) {
            for (j = 0; j <= room->f1; j += room->f1) {
                tile = &cell0[j + (i << 7)] - map;
                if (b->door_state || (-map[tile + 128].first & 3) == 2) {
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
                if (b->door_state || (-map[tile - 1].first & 3) == 1) {
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
    type += b->door_state << 2;
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
        if (b->door_state != 2) {
            blocked = !can_place_door((cy << 7) + cx, 3, blueprint[(s16)doortype[type % 4 + 4].sprites[0]].unknown4 - 1) ? (u8)1 : (u8)0;
        } else {
            int kind;

            kind = blueprint[find_small_thing((cy << 7) + cx)].unknown7;
            blocked = kind == 3 || kind == 0x34 || kind == 0x35 ? (u8)1 : (u8)0;
        }
        give_screen_xy_of_cell_at_column_row(window.unknown2eb, cx, cy - 1);
        d = info.unknown54;
        i = info.unknown58;
        if (flag || blocked) {
            if (blocked || !is_valid_door((cy << 7) + cx, owner, type)) {
                if (owner == player) {
                    draw_a_one_colour_frame(start[doortype[type].tiles[3]].frame, d, i, (u16)color.unknownD, start[doortype[type].tiles[3]].flags);
                }
                D_001553d0[owner] = 1;
            } else if (owner == player) {
                draw_a_frame(start[doortype[type].tiles[3]].frame, d, i, 0, start[doortype[type].tiles[3]].flags);
            }
        } else if (owner == player) {
            draw_a_frame(start[doortype[type].tiles[3]].frame, d, i, 0, start[doortype[type].tiles[3]].flags);
        }
    }
    b->door_tile = best_tile;
    b->door_type = type;
}
