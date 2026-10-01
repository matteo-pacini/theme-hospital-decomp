#include "hosp.h"

extern "C" {
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
void machine_disaster(Thing *machine)
{
    u16 tile;
    RoomInfo *room;

    room = &room_info[machine->room_id];
    tile = (s16)(machine->x.tile + (machine->y.tile << 7)) + objects[machine->subtype].f1a;
    create_explosion(tile, 0x97);
    blast_effect(tile, room);
    explosion_kills_dudes_in_room(room);
    remove_expertise_room(room);
    room_deleted_update_listhead(room);
    room->type = 6;
    room->f3 = 6;
    allhosps[machine->owner].fc98++;
    reputation_affected(0, 0xb, 0, machine->owner, 0);
    if (machine->owner == player)
        award_counts.f144 = 1;
    restart_entire_queue(room);
}

// FUNCTION: 0x00075110
void process_bang(Thing *bang)
{
    MapPos x;
    MapPos y;
    RoomInfo *room;

    x.raw = bang->x.raw + bang->vel_x;
    y.raw = bang->y.raw + bang->vel_y;
    move_node(bang, x, y);
    bang->altitude += bang->vel_z;
    bang->vel_z -= 0x200;
    bang->vel_x = bang->vel_x - (bang->vel_x >> 2);
    bang->vel_y = bang->vel_y - (bang->vel_y >> 2);
    bang->vel_z = bang->vel_z - (bang->vel_z >> 2);
    if (bang->altitude >= 0) {
        room = &room_info[bang->bang_room];
        if (room > room_info) {
            u8 x0;
            u8 y0;

            x0 = room->pos % 128;
            y0 = room->pos / 128;
            if (x.tile < x0 || x.tile == x0 + room->f1 - 1 || y.tile < y0 || y.tile == y0 + room->f2 - 1)
                goto die;
        }
        if (actual_game_turn % bang->anim_speed == 0) {
            u16 next;

            next = frame[bang->sprite].next;
            if (frame[next].flags & 1)
                goto die;
            bang->sprite = next;
        }
        return;
    }
die:
    remove_thing(bang);
}

// FUNCTION: 0x000752e8
u16 create_bang(u16 id, u16 tile, s16 dx, s16 dy, s16 dz, u16 anim)
{
    RoomInfo *room;
    Thing *bang;
    u8 x;
    u8 y;

    room = which_room(tile);
    x = tile % 128;
    y = tile / 128;
    for (bang = &thing[id]; bang < end_things - 50; bang++) {
        if (bang->type == 0) {
            bang->type = 0xe;
            bang->sprite = start[anim].frame;
            bang->flip = start[anim].flags;
            bang->anim_speed = (old_newrand() & 1) + 1;
            bang->x.tile = x;
            bang->y.tile = y;
            bang->x.frac = 0x80;
            bang->y.frac = 0x80;
            insert_node(bang, bang->x, bang->y);
            bang->altitude = 0;
            bang->vel_x = dx;
            bang->vel_y = dy;
            bang->vel_z = dz;
            if (room > room_info)
                bang->bang_room = room - room_info;
            return bang - thing;
        }
    }
    return 0;
}

// FUNCTION: 0x00075484
void create_explosion(u16 tile, u16 anim)
{
    u16 id;
    s32 elevation;
    s32 heading;
    s32 speed;
    s32 horiz_speed;
    s32 dx;
    s32 dy;
    s32 dz;

    id = 1;
    for (elevation = 0; elevation < 0x200; elevation += 0x55) {
        for (heading = 0; heading < 0x800; heading += 0x800 / ((lbSinTable[elevation + 0x200] << 0xb >> 0x10) / 0x5f + 1)) {
            speed = old_newrand() % 50 + 100;
            horiz_speed = speed * lbSinTable[elevation + 0x200] >> 0x10;
            dx = horiz_speed * lbSinTable[heading + 0x200] >> 0x10;
            dy = horiz_speed * lbSinTable[heading] >> 0x10;
            dz = speed * lbSinTable[elevation] >> 0xa;
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
    u16 cell;
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
                cell = (s16)x + ((s16)y << 7);
                m = &map[(s16)x + ((s16)y << 7)];
                d2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
                blast_things(m);
                if ((old_newrand() & 0x3ff) > (d2 << 10) / r2)
                    blast_walls_and_floor(cell);
            }
        }
    }
    othervar.f10 = 1;
}

// FUNCTION: 0x000757a0
void blast_walls_and_floor(u16 tile)
{
    u8 flags;
    u8 block;
    Map *m;

    m = &map[tile];
    if (old_newrand() % 1000 > 300) {
        block = m->unknown3;
        flags = blocks[block].width;
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
        block = m->unknown4;
        flags = blocks[block].width;
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
    u8 facing;
    u8 debris;
    Thing *obj;

restart:
    for (obj = &thing[m->first]; obj > thing; obj = &thing[obj->cell_next]) {
        if (obj->type == 6) {
            debris = objects[obj->subtype].unknown20[0];
            facing = obj->facing;
            fn_00038d04(obj, obj->owner);
            if (debris)
                place_small_thing(m - map, debris + facing, 0);
            goto restart;
        }
    }
}

// FUNCTION: 0x0007599c
void explosion_kills_dudes_in_room(RoomInfo *room)
{
    u8 again;
    s32 tries;
    Thing *person;

    again = 1;
    tries = 50;
    while (again && --tries >= 0) {
        again = 0;
        for (person = &thing[room->first]; person > thing; person = &thing[person->room_next]) {
            if (person->type == 4) {
                sack_staff(person, 2);
                again = 1;
                break;
            }
        }
    }
    for (person = &thing[room->first]; person > thing; person = &thing[person->room_next]) {
        if (person->type == 2)
            person_dies(person);
    }
}

// FUNCTION: 0x00075aac
void restart_entire_queue(RoomInfo *room)
{
    Thing *patient;

    while ((patient = &thing[room->queue_head]) > thing) {
        if (allhosps[patient->owner].rooms[patient->treatment].unknown1C != 1)
            patient->treatment = 1;
        leave_queue(patient);
        interrupt_person(patient);
        start_person_going(patient);
    }
}

// FUNCTION: 0x00075b54
void generate_disaster(u8 owner)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    switch (hosp->f213) {
    case 0:
        hosp->f20f++;
        if (hosp->f20f > gbv.f12c) {
            hosp->f20f = 0;
            hosp->f213 = old_newrand() & 3;
            switch (hosp->f213) {
            case 0:
                break;
            case 1:
                if (listhead[owner].objects[0x2c] && hosp->fc9e[2] * 8 < hosp->fc92) {
                    hosp->f211 = hosp->fc94;
                    hosp->fc94 = 0;
                    hosp->fc95 = 1;
                    advisor.f2 = 0xb;
                    hosp->f214 = old_normal_rand(10, 0x1e);
                } else {
                    hosp->f213 = 0;
                }
                break;
            case 2:
                if (listhead[owner].objects[0x2c] && hosp->fc9e[2] * 8 < hosp->fc92) {
                    hosp->f211 = hosp->fc94;
                    hosp->fc94 = 0xa;
                    hosp->fc95 = 1;
                    advisor.f2 = 0xa;
                    hosp->f214 = old_normal_rand(10, 0x1e);
                } else {
                    hosp->f213 = 0;
                }
                break;
            case 3:
                if (hosp->fcaa > 10) {
                    disaster_peeps(0, owner);
                    advisor.f2 = 0xd;
                    hosp->f214 = old_normal_rand(5, 10);
                } else {
                    hosp->f213 = 0;
                }
                break;
            }
        }
        break;
    case 1:
    case 2:
        if (hosp->fc9e[2] * 8 > hosp->fc92)
            hosp->f214--;
        if (hosp->fc9e[2] * 5 > hosp->fc92)
            hosp->f214--;
        hosp->f214--;
        if (hosp->f214 <= 0) {
            hosp->f213 = 0;
            hosp->fc94 = hosp->f211;
            hosp->fc95 = 0;
            advisor.f2 = 0xc;
        }
        break;
    case 3:
        if (hosp->fcaa > 10)
            disaster_peeps(0, owner);
        else
            hosp->f214--;
        hosp->f214--;
        if (hosp->f214 <= 0) {
            hosp->f213 = 0;
            advisor.f2 = 0xe;
        }
        break;
    }
}

// FUNCTION: 0x00075ed0
void disaster_peeps(u8 mode, s8 owner)
{
    Thing *peep;

    for (peep = thing + 1; peep < end_things; peep++) {
        if (peep->type == 2 && (peep->owner == owner || owner == -1)) {
            if (mode == 0)
                peep->nausea = (u16)gbv.f11a + 10;
            else if (mode == 1 && abilities[peep->subtype][5])
                peep->flags |= 0x200;
            else if (mode == 2 && abilities[peep->subtype][5])
                peep->flags &= ~0x200;
            else if (mode == 3)
                change_health(peep, -peep->health);
        }
    }
}

// FUNCTION: 0x00075fe4
void what_is_required(u8 illness, u8 owner)
{
    s32 unused;
    Expertise *exp;
    s32 i;

    unused = 0;
    memset(&required, 0, 10);
    exp = &expertise[illness];
    if (allhosps[owner].rooms[illness].unknown1C == 1) {
        required.unknown5 = 1;
        if (exp->kind == 0x27)
            required.unknown0[2] = allhosps[owner].rooms[illness].unknown1B;
        return;
    }
    if (listhead[owner].rooms[exp->category] == 0) {
        required.unknown6 = exp->category;
        if (room_available((u16)exp->category) == 0)
            required.unknown7 = 1;
    }
    if (exp->unknown0 & 0x20) {
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
            if ((exp->unknown0 & (1 << i)) && listhead[owner].staff[i] == 0) {
                required.flags = exp->unknown0;
                required.unknown3 = i;
                break;
            }
        }
    }
    if (exp->unknown7) {
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
u16 create_shadow(u16 tile, u16 caster, u8 owner, u16 anim)
{
    Thing *shadow;

    shadow = allocate_thing(0x32);
    if (shadow) {
        shadow->type = 0x18;
        shadow->owner = owner;
        shadow->x.tile = tile % 128;
        shadow->y.tile = tile / 128;
        shadow->x.frac = 0x80;
        shadow->y.frac = 0x80;
        insert_node(shadow, shadow->x, shadow->y);
        shadow->sprite = start[anim].frame;
        shadow->flip = start[anim].flags;
        shadow->caster = caster;
        shadow->depth = -8;
        return shadow - thing;
    }
    return 0;
}
