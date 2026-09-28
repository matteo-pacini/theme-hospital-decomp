#include "hosp.h"

extern "C" {

extern u8 D_001553c7;
extern u8 D_001553c8;

s32 fn_00038d04(Thing *w, u8 owner);
#pragma aux fn_00038d04 "W?remove_object$n(pn$Thing$$)s"
void fn_00024a34(u16 tile, u16 room, u8 side);
#pragma aux fn_00024a34 "W?clear_cell_of_ratholes$n(ucus)v"
void fn_0007d2a8(u8 owner, u16 tile);
#pragma aux fn_0007d2a8 "W?delete_wall$n(ususuc)v"

}

s16 find_bluep_who(Map *cell);
s8 get_wall_color(Map *cell);
void recolour_all_rooms(void);
u16 which_room_number(u16 tile);
void add_static_shadows(void);
void recolour_inside_room(RoomInfo *room, u16 a, u16 b);
void recolour_outside_room(RoomInfo *room, u16 a, u16 b);
void fill_in_window(u16 x, u16 y, u8 side);
void pixel_buffer_map(void);

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
            set_mouse_mode(0);
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
            if (D_001553c7 == player) {
                init_place_object(next_object_to_build(), 0, room - room_info, 0);
            }
            gamevar.unknown20 = 1;
            for (r = room_info + 1; r != room; r++, gamevar.unknown20++) {
            }
        }
        break;
    }
    D_001553c8 = 1;
}

// FUNCTION: 0x00024c58
void delete_room(RoomInfo *room)
{
    u16 id;
    Thing *t;
    int x;
    int y;

    id = which_room_number(((s16)(room->pos / 128 + room->f2 / 2) << 7) + (s16)(room->pos % 128 + room->f1 / 2));
    for (y = room->pos / 128; y < room->pos / 128 + room->f2; y++) {
        for (x = room->pos % 128; x < room->pos % 128 + room->f1; x++) {
            for (t = &thing[map[(s16)x + ((s16)y << 7)].first]; t > thing; t = &thing[t->next]) {
                if (t->renderer == 6) {
                    fn_00038d04(t, D_001553c7);
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
            for (t = &thing[map[(s16)x + ((s16)y << 7)].first]; t > thing; t = &thing[t->next]) {
                if (t->renderer == 8 || (t->renderer == 6 && (t->subtype == 0x34 || t->subtype == 0x35))) {
                    switch (t->unknown17) {
                    case 0:
                    case 2:
                        if (x < room->pos % 128 + room->f1) {
                            fn_00038d04(t, D_001553c7);
                        }
                        break;
                    case 1:
                    case 3:
                        if (y < room->pos / 128 + room->f2) {
                            fn_00038d04(t, D_001553c7);
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
void recolour_inside_room(RoomInfo *room, u16 a, u16 b)
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
            if (t->renderer == 3) {
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
                map[tile].unknown3 = a;
                break;
            case 0xc3:
            case 0xc5:
            case 0xc7:
            case 0xc9:
            case 0xcb:
            case 0xcd:
            case 0xcf:
            case 0xd1:
                map[tile].unknown3 = a + 0x73;
                break;
            case 0x60:
            case 0x62:
            case 0x64:
            case 0x66:
            case 0x68:
            case 0x6a:
            case 0x6c:
            case 0x6e:
                map[tile].unknown3 = a + 0x10;
                break;
            }
        }
    }
    for (y = room->pos / 128; y < room->pos / 128 + room->f2; y++) {
        ok = 1;
        if (map[(s16)(room->pos % 128) + ((s16)y << 7)].first > 0) {
            t = &thing[map[(s16)(room->pos % 128) + ((s16)y << 7)].first];
            if (t->renderer == 3) {
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
                map[tile].unknown4 = a + 1;
                break;
            case 0xc4:
            case 0xc6:
            case 0xc8:
            case 0xca:
            case 0xcc:
            case 0xce:
            case 0xd0:
            case 0xd2:
                map[tile].unknown4 = a + 0x74;
                break;
            case 0x61:
            case 0x63:
            case 0x65:
            case 0x67:
            case 0x69:
            case 0x6b:
            case 0x6d:
            case 0x6f:
                map[tile].unknown4 = a + 0x11;
                break;
            }
        }
    }
}

// FUNCTION: 0x0002583c
void recolour_outside_room(RoomInfo *room, u16 a, u16 b)
{
    u8 ok;
    u16 x;
    u16 y;

    for (x = room->pos % 128; x < room->pos % 128 + room->f1; x++) {
        ok = 1;
        if (map[(s16)x + ((s16)(room->pos / 128 + room->f2) << 7)].first) {
            int n = map[(s16)x + ((s16)(room->pos / 128) << 7)].first;
            if (n > 0 && thing[n].renderer == 3) {
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
                map[(s16)x + ((s16)(room->pos / 128 + room->f2) << 7)].unknown3 = a;
                break;
            case 0xc3:
            case 0xc5:
            case 0xc7:
            case 0xc9:
            case 0xcb:
            case 0xcd:
            case 0xcf:
            case 0xd1:
                map[(s16)x + ((s16)(room->pos / 128 + room->f2) << 7)].unknown3 = a + 0x73;
                break;
            case 0x60:
            case 0x62:
            case 0x64:
            case 0x66:
            case 0x68:
            case 0x6a:
            case 0x6c:
            case 0x6e:
                map[(s16)x + ((s16)(room->pos / 128 + room->f2) << 7)].unknown3 = a + 0x10;
                break;
            }
        }
    }
    for (y = room->pos / 128; y < room->pos / 128 + room->f2; y++) {
        ok = 1;
        if (map[((s16)y << 7) + (s16)(room->pos % 128 + room->f1)].first) {
            int n = map[(s16)x + ((s16)(room->pos / 128) << 7)].first;
            if (n > 0 && thing[n].renderer == 3) {
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
                map[((s16)y << 7) + (s16)(room->pos % 128 + room->f1)].unknown4 = a + 1;
                break;
            case 0xc4:
            case 0xc6:
            case 0xc8:
            case 0xca:
            case 0xcc:
            case 0xce:
            case 0xd0:
            case 0xd2:
                map[((s16)y << 7) + (s16)(room->pos % 128 + room->f1)].unknown4 = a + 0x74;
                break;
            case 0x61:
            case 0x63:
            case 0x65:
            case 0x67:
            case 0x69:
            case 0x6b:
            case 0x6d:
            case 0x6f:
                map[((s16)y << 7) + (s16)(room->pos % 128 + room->f1)].unknown4 = a + 0x11;
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
        if (room->type != 0) {
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
            if (room->type != 0) {
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
    D_001553c8 = 1;
}
