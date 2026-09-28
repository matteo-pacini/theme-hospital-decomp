#include "hosp.h"

extern "C" {

extern u8 D_00155e1e;
extern u16 D_00155e1f;
extern u8 D_0015fe84;
extern u8 D_0015fe85;
extern u8 D_001553d0[];
extern u8 D_001553c7;
extern u8 D_001553c8;

u8 fn_00029748(void);
void fn_000228e0(void);

}

s16 place_door(u16 tile, u16 kind);
s8 search_for_outer_color(Map *cell);
void remove_blank_wall(Map *cell, u8 side);

// FUNCTION: 0x00022854
u8 fn_00022854(u8 warn)
{
    if (gamevar.f12e != 0) {
        if (warn) {
            D_00155e1e = 0x19;
            D_00155e1f = 0x14;
        }
        return 0;
    }
    if (fn_00029748() || allhosps[player].unknown11b7) {
        if (warn) {
            D_00155e1e = 0x19;
            D_00155e1f = 0x14;
        }
        return 0;
    }
    return 1;
}

// FUNCTION: 0x000228e0
void fn_000228e0(void)
{
    if (myplayer.f22) {
        if (statvar.unknownfd <= 1) {
            if (mycontrol->unknown64f == 0x1c || D_0015fe85) {
                if (mycontrol->unknown650 + mycontrol->unknown64f != D_0015fe84 || D_0015fe85) {
                    if (fn_00022854(0)) {
                        fn_00021818(8, -1);
                        D_0015fe84 = mycontrol->unknown650 + mycontrol->unknown64f;
                        D_0015fe85 = 0;
                    } else {
                        D_0015fe85 = 1;
                    }
                }
            }
        }
    }
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
    u8 sprite;

    room = &room_info[allhosps[owner].build.room];
    bs = &allhosps[owner].build;
    delta = 1;
    if (D_001553d0[owner]) {
        if (D_001553c7 == player) {
            play_sound(10, 8, -1, -1);
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
            play_sound(10, 8, -1, -1);
        }
        return;
    }
    type = bs->door_type;
    if (swap) {
        if (blueprint[find_small_thing(bs->door_tile + doortype[type].tiles[0])].kind == 0x7e && bs->door_state == 2) {
            type %= 4;
            type += 0x14;
            delta = -1;
            play_sound(10, 0x97, -1, -1);
        } else {
            return;
        }
    } else if (owner == player) {
        if (bs->door_state == 2 && blueprint[find_small_thing(bs->door_tile + doortype[type].tiles[0])].kind == 0x7e) {
            play_sound(10, 8, -1, -1);
        } else {
            play_sound(10, 0x1e, -1, -1);
        }
    }
    for (i = 0; i < doortype[type].count; i++) {
        tile = bs->door_tile + doortype[type].tiles[i];
        if (map[tile].first > 0) {
            t = &thing[map[tile].first];
            if (t > thing) {
                for (; t->next > 0; t = &thing[t->next]) {
                }
            }
            id = -t->next;
        } else {
            id = -map[tile].first;
        }
        sprite = doortype[type].sprites[i];
        if ((blueprint[id].flags & 3) == 3) {
            sprite += 6;
        }
        place_small_thing(tile, (u16)sprite, 0);
        map[tile - doortype[type].offset].overlay = doortype[type].overlays[i];
    }
    D_001553c8 = 1;
    if (bs->door_state == 2) {
        room->f1b += delta;
    }
    if (bs->door_state == 1 || bs->door_state == 0) {
        if (D_001553c7 == player) {
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
        if (r->type != 0) {
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
        if (r->type != 0) {
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
        for (t = &thing[cell->first]; t->next > 0; t = &thing[t->next]) {
        }
        id = t->next;
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
        switch (D_001553c7) {
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
                    room->owner = D_001553c7;
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
                                if (t->renderer == 3) {
                                    if (t->unknown17 == 0 || t->unknown17 == 2) {
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
                                if (t->renderer == 3) {
                                    if (t->unknown17 == 1 || t->unknown17 == 3) {
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
                        fn_00037098(cell - map, (u16)blueprint[bp].kind, saved, D_001553c7);
                        break;
                    case 1:
                    case 3:
                        place_small_thing(cell - map, 0, 0);
                        place_small_thing(cell + 0x80 - map, 0, 0);
                        fn_00037098(cell - map, (u16)blueprint[bp].kind, saved, D_001553c7);
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
    D_001553c8 = 1;
}
