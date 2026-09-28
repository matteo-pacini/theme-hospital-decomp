#include "hosp.h"

u8 is_tile_valid(u16 tile, u8 owner);
u8 check_room_access(u8 owner);
void blank_wall_around_blueprint(u8 owner, u8 blank);
u8 is_tile_near_half_wall(u8 x, u8 y, u8 x0, u8 y0, u8 x1, u8 y1);
void set_expertise_room(RoomInfo *room);

extern "C" {

extern u8 D_0015831b;
extern u8 D_001553c7;

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

// FUNCTION: 0x00046f88
u8 is_tile_valid(u16 tile, u8 owner)
{
    Thing *t;

    for (t = &thing[map[tile].first]; t > thing; t = &thing[t->next]) {
        if (t->renderer != 2 && t->renderer != 4) {
            D_0015831b |= 2;
            return 0;
        }
    }
    if (map[tile].flags & 7) {
        return 0;
    }
    if (!(map[tile].unknown7 & 0x10) && (map[tile].unknown7 & (1 << owner))) {
        return 1;
    }
    D_0015831b |= 2;
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
    if (which_room((y0 << 7) + x0) || which_room(((y1 - 1) << 7) + (x1 - 1))) {
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
    D_0015831b |= 2;
    return 0;
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

// FUNCTION: 0x00047b88
u8 is_room_clear_yet(RoomInfo *room)
{
    u8 y0;
    u8 y1;
    u8 x;
    u8 y;
    u8 x1;
    u8 x0;

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
        D_0015831b |= 4;
        return 0;
    }
    D_0015831b &= ~4;
    return 1;
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
