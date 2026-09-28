#include "hosp.h"

extern "C" {
extern u8 D_001553c8;
extern s32 lbSinTable[];
s32 fn_00038d04(Thing *w, u8 owner);
#pragma aux fn_00038d04 "W?remove_object$n(pn$Thing$$)s"
}

void remove_expertise_room(RoomInfo *room);
void room_deleted_update_listhead(RoomInfo *room);
void blast_effect(u16 tile, RoomInfo *room);
void blast_walls_and_floor(u16 tile);
void blast_things(Map *m);
void explosion_kills_dudes_in_room(RoomInfo *room);
void restart_entire_queue(RoomInfo *room);
u16 create_bang(u16 id, u16 tile, s16 dx, s16 dy, s16 dz, u16 anim);
void disaster_peeps(u8 mode, s8 owner);

// FUNCTION: 0x00075010
void machine_disaster(Thing *e)
{
    u16 tile;
    RoomInfo *room;

    room = &room_info[e->unknown89];
    tile = (s16)(e->x.tile + (e->y.tile << 7)) + objects[e->subtype].f1a;
    create_explosion(tile, 0x97);
    blast_effect(tile, room);
    explosion_kills_dudes_in_room(room);
    remove_expertise_room(room);
    room_deleted_update_listhead(room);
    room->type = 6;
    room->f3 = 6;
    allhosps[e->owner].fc98++;
    reputation_affected(0, 0xb, 0, e->owner, 0);
    if (e->owner == player)
        award_counts.f144 = 1;
    restart_entire_queue(room);
}

// FUNCTION: 0x00075110
void process_bang(Thing *w)
{
    MapPos x;
    MapPos y;
    RoomInfo *room;

    x.raw = w->x.raw + w->f39;
    y.raw = w->y.raw + w->f3b;
    move_node(w, x, y);
    w->f37 += w->f3d;
    w->f3d -= 0x200;
    w->f39 = w->f39 - (w->f39 >> 2);
    w->f3b = w->f3b - (w->f3b >> 2);
    w->f3d = w->f3d - (w->f3d >> 2);
    if (w->f37 >= 0) {
        room = &room_info[w->unknown63];
        if (room > room_info) {
            u8 x0;
            u8 y0;

            x0 = room->pos % 128;
            y0 = room->pos / 128;
            if (x.tile < x0 || x.tile == x0 + room->f1 - 1 || y.tile < y0 || y.tile == y0 + room->f2 - 1)
                goto die;
        }
        if (actual_game_turn % w->anim_speed == 0) {
            u16 next;

            next = frame[w->sprite].next;
            if (frame[next].flags & 1)
                goto die;
            w->sprite = next;
        }
        return;
    }
die:
    remove_thing(w);
}

// FUNCTION: 0x000752e8
u16 create_bang(u16 id, u16 tile, s16 dx, s16 dy, s16 dz, u16 anim)
{
    RoomInfo *room;
    Thing *w;
    u8 x;
    u8 y;

    room = which_room(tile);
    x = tile % 128;
    y = tile / 128;
    for (w = &thing[id]; w < end_things - 50; w++) {
        if (w->renderer == 0) {
            w->renderer = 0xe;
            w->sprite = start[anim].frame;
            w->palette = start[anim].flags;
            w->anim_speed = (old_newrand() & 1) + 1;
            w->x.tile = x;
            w->y.tile = y;
            w->x.frac = 0x80;
            w->y.frac = 0x80;
            insert_node(w, w->x, w->y);
            w->f37 = 0;
            w->f39 = dx;
            w->f3b = dy;
            w->f3d = dz;
            if (room > room_info)
                w->unknown63 = room - room_info;
            return w - thing;
        }
    }
    return 0;
}

// FUNCTION: 0x00075484
void create_explosion(u16 tile, u16 anim)
{
    u16 id;
    s32 a;
    s32 b;
    s32 r;
    s32 h;
    s32 dx;
    s32 dy;
    s32 dz;

    id = 1;
    for (a = 0; a < 0x200; a += 0x55) {
        for (b = 0; b < 0x800; b += 0x800 / ((lbSinTable[a + 0x200] << 0xb >> 0x10) / 0x5f + 1)) {
            r = old_newrand() % 50 + 100;
            h = r * lbSinTable[a + 0x200] >> 0x10;
            dx = h * lbSinTable[b + 0x200] >> 0x10;
            dy = h * lbSinTable[b] >> 0x10;
            dz = r * lbSinTable[a] >> 0xa;
            id = create_bang(id, tile, dx, dy, dz, anim);
        }
    }
    play_sound(10, 0xd5, -1, -1);
}

// FUNCTION: 0x000755c8
void blast_effect(u16 tile, RoomInfo *room)
{
    s8 cy;
    s8 x;
    s8 y;
    s8 radius;
    s8 cx;
    s8 rx;
    s8 ry;
    s32 r2;
    u16 t;
    Map *m;
    s32 d2;

    radius = (old_newrand() & 3) + 4;
    cx = tile % 128;
    cy = tile / 128;
    rx = room->pos % 128;
    ry = room->pos / 128;
    r2 = radius * radius;
    for (x = cx - radius; x <= cx + radius; x++) {
        for (y = cy - radius; y <= cy + radius; y++) {
            if (x >= rx && x < rx + room->f1 && y >= ry && y < ry + room->f2) {
                t = (s16)x + ((s16)y << 7);
                m = &map[(s16)x + ((s16)y << 7)];
                d2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
                blast_things(m);
                if ((old_newrand() & 0x3ff) > (d2 << 10) / r2)
                    blast_walls_and_floor(t);
            }
        }
    }
    D_001553c8 = 1;
}

// FUNCTION: 0x000757a0
void blast_walls_and_floor(u16 tile)
{
    u8 flags;
    u8 b;
    Map *m;

    m = &map[tile];
    if (old_newrand() % 1000 > 300) {
        b = m->unknown3;
        flags = blocks[b].width;
        if (flags & 2) {
            if (!(flags & 0x10))
                place_small_thing(tile, 0x67, 1);
            return;
        }
        if (flags & 1) {
            place_small_thing(tile, 0x65, 1);
            return;
        }
    }
    if (old_newrand() % 1000 > 300) {
        b = m->unknown4;
        flags = blocks[b].width;
        if (flags & 2) {
            if (!(flags & 0x10))
                place_small_thing(tile, 0x68, 1);
        } else if (flags & 1) {
            place_small_thing(tile, 0x66, 1);
        }
    }
}

// FUNCTION: 0x000758b8
void blast_things(Map *m)
{
    u8 pal;
    u8 obj;
    Thing *w;

restart:
    for (w = &thing[m->first]; w > thing; w = &thing[w->next]) {
        if (w->renderer == 6) {
            obj = objects[w->subtype].unknown20[0];
            pal = w->unknown17;
            fn_00038d04(w, w->owner);
            if (obj)
                place_small_thing(m - map, obj + pal, 0);
            goto restart;
        }
    }
}

// FUNCTION: 0x0007599c
void explosion_kills_dudes_in_room(RoomInfo *room)
{
    u8 again;
    s32 tries;
    Thing *w;

    again = 1;
    tries = 50;
    while (again && --tries >= 0) {
        again = 0;
        for (w = &thing[room->first]; w > thing; w = &thing[w->room_next]) {
            if (w->renderer == 4) {
                sack_staff(w, 2);
                again = 1;
                break;
            }
        }
    }
    for (w = &thing[room->first]; w > thing; w = &thing[w->room_next]) {
        if (w->renderer == 2)
            person_dies(w);
    }
}

// FUNCTION: 0x00075aac
void restart_entire_queue(RoomInfo *room)
{
    Thing *w;

    while ((w = &thing[room->queue_head]) > thing) {
        if (allhosps[w->owner].rooms[w->unknown171].unknown1C != 1)
            w->unknown171 = 1;
        leave_queue(w);
        interrupt_person(w);
        start_person_going(w);
    }
}

// FUNCTION: 0x00075b54
void generate_disaster(u8 owner)
{
    AllHosps *h;

    h = &allhosps[owner];
    switch (h->f213) {
    case 0:
        h->f20f++;
        if (h->f20f > gbv.f12c) {
            h->f20f = 0;
            h->f213 = old_newrand() & 3;
            switch (h->f213) {
            case 0:
                break;
            case 1:
                if (listhead[owner].objects[0x2c] && h->fc9e[2] * 8 < h->fc92) {
                    h->f211 = h->fc94;
                    h->fc94 = 0;
                    h->fc95 = 1;
                    advisor.f2 = 0xb;
                    h->f214 = old_normal_rand(10, 0x1e);
                } else {
                    h->f213 = 0;
                }
                break;
            case 2:
                if (listhead[owner].objects[0x2c] && h->fc9e[2] * 8 < h->fc92) {
                    h->f211 = h->fc94;
                    h->fc94 = 0xa;
                    h->fc95 = 1;
                    advisor.f2 = 0xa;
                    h->f214 = old_normal_rand(10, 0x1e);
                } else {
                    h->f213 = 0;
                }
                break;
            case 3:
                if (h->fcaa > 10) {
                    disaster_peeps(0, owner);
                    advisor.f2 = 0xd;
                    h->f214 = old_normal_rand(5, 10);
                } else {
                    h->f213 = 0;
                }
                break;
            }
        }
        break;
    case 1:
    case 2:
        if (h->fc9e[2] * 8 > h->fc92)
            h->f214--;
        if (h->fc9e[2] * 5 > h->fc92)
            h->f214--;
        h->f214--;
        if (h->f214 <= 0) {
            h->f213 = 0;
            h->fc94 = h->f211;
            h->fc95 = 0;
            advisor.f2 = 0xc;
        }
        break;
    case 3:
        if (h->fcaa > 10)
            disaster_peeps(0, owner);
        else
            h->f214--;
        h->f214--;
        if (h->f214 <= 0) {
            h->f213 = 0;
            advisor.f2 = 0xe;
        }
        break;
    }
}

// FUNCTION: 0x00075ed0
void disaster_peeps(u8 mode, s8 owner)
{
    Thing *w;

    for (w = thing + 1; w < end_things; w++) {
        if (w->renderer == 2 && (w->owner == owner || owner == -1)) {
            if (mode == 0)
                w->unknown138w = (u16)gbv.f11a + 10;
            else if (mode == 1 && abilities[w->subtype][5])
                w->flags |= 0x200;
            else if (mode == 2 && abilities[w->subtype][5])
                w->flags &= ~0x200;
            else if (mode == 3)
                change_health(w, -w->unknown130);
        }
    }
}

// FUNCTION: 0x00075fe4
void what_is_required(u8 room, u8 owner)
{
    s32 unused;
    Expertise *e;
    s32 i;

    unused = 0;
    memset(&required, 0, 10);
    e = &expertise[room];
    if (allhosps[owner].rooms[room].unknown1C == 1) {
        required.unknown5 = 1;
        if (e->kind == 0x27)
            required.unknown0[2] = allhosps[owner].rooms[room].unknown1B;
        return;
    }
    if (listhead[owner].rooms[e->category] == 0) {
        required.unknown6 = e->category;
        if (room_available((u16)e->category) == 0)
            required.unknown7 = 1;
    }
    if (e->unknown0 & 0x20) {
        if (listhead[owner].staff[5] == 0) {
            required.flags = 0x20;
            required.unknown3 = 5;
            required.unknown4 = 2;
        } else if (allhosps[owner].fca6 < 2) {
            required.unknown4 = 2 - allhosps[owner].fca6;
            required.flags = 0x20;
            required.unknown3 = 5;
        }
    } else {
        for (i = 0; i < 9; i++) {
            if ((e->unknown0 & (1 << i)) && listhead[owner].staff[i] == 0) {
                required.flags = e->unknown0;
                required.unknown3 = i;
                break;
            }
        }
    }
    if (e->unknown7) {
        if (allhosps[owner].rooms[0x2a].unknown1C == 0) {
            if (listhead[owner].rooms[9] == 0)
                required.unknown8 = 1;
            if (listhead[owner].staff[2] == 0)
                required.unknown9 = 1;
        }
    }
}

// FUNCTION: 0x0007628c
extern "C" u8 fn_0007628c(Thing *w)
{
    return 0;
}

// FUNCTION: 0x000762b0
void saucer_abducts(Thing *w)
{
}

// FUNCTION: 0x000762cc
u16 create_shadow(u16 tile, u16 height, u8 owner, u16 anim)
{
    Thing *w;

    w = allocate_thing(0x32);
    if (w) {
        w->renderer = 0x18;
        w->owner = owner;
        w->x.tile = tile % 128;
        w->y.tile = tile / 128;
        w->x.frac = 0x80;
        w->y.frac = 0x80;
        insert_node(w, w->x, w->y);
        w->sprite = start[anim].frame;
        w->palette = start[anim].flags;
        w->f37 = height;
        w->depth = -8;
        return w - thing;
    }
    return 0;
}
