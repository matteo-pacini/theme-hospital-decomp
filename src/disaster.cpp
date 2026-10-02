#include "hosp.h"

extern "C" {
s32 remove_object(Thing *obj, u8 owner);
#pragma aux remove_object "W?remove_object$n(pn$Thing$$)s"
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

// Blows up machine: explosion, damage and debris around it, kills everyone in
// its room and destroys the room (ROOM_DESTROYED), costing reputation and
// counting the disaster. Its queue is sent elsewhere.
// FUNCTION: 0x00075010
void machine_disaster(Thing *machine)
{
    u16 tile;
    RoomInfo *room;

    room = &room_info[machine->room_id];
    tile = (s16)(machine->x.tile + (machine->y.tile << 7)) + objects[machine->subtype].explosion_offset;
    create_explosion(tile, 0x97);
    blast_effect(tile, room);
    explosion_kills_dudes_in_room(room);
    remove_expertise_room(room);
    room_deleted_update_listhead(room);
    room->type = ROOM_DESTROYED;
    room->state = RS_CLOSED;
    allhosps[machine->owner].mishaps++;
    reputation_affected(0, REP_MACHINE_EXPLODED, 0, machine->owner, 0);
    if (machine->owner == player)
        award_counts.machine_exploded = 1;
    restart_entire_queue(room);
}

// THING_DEBRIS: moves the piece by its velocity under gravity and drag and
// animates it; removes it when it lands, reaches the edge of its room, or its
// animation ends.
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
            if (x.tile < x0 || x.tile == x0 + room->width - 1 || y.tile < y0 || y.tile == y0 + room->height - 1)
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

// Creates a piece of debris with animation anim at tile, moving at dx,dy,dz,
// in the first free thing from id on (the last 50 are left free). Returns its
// thing index, or 0 if none is free.
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
        if (bang->type == THING_FREE) {
            bang->type = THING_DEBRIS;
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

// Throws a burst of debris (animation anim) out from tile in all directions
// and plays the bomb sound.
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
        for (heading = 0; heading < 0x800;
             heading += 0x800 / ((lbSinTable[elevation + 0x200] << 0xb >> 0x10) / 0x5f + 1)) {
            speed = old_newrand() % 50 + 100;
            horiz_speed = speed * lbSinTable[elevation + 0x200] >> 0x10;
            dx = horiz_speed * lbSinTable[heading + 0x200] >> 0x10;
            dy = horiz_speed * lbSinTable[heading] >> 0x10;
            dz = speed * lbSinTable[elevation] >> 0xa;
            id = create_bang(id, tile, dx, dy, dz, anim);
        }
    }
    play_sound(SNDSRC_GENERAL, SFX_BOMB, -1, -1);
}

// Wrecks room around tile within a random radius of 4-7 tiles: removes the
// objects there, and the nearer a tile the likelier its walls and floor are
// damaged.
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
            if (x >= rx && x < rx + room->width && y >= ry && y < ry + room->height) {
                cell = (s16)x + ((s16)y << 7);
                m = &map[(s16)x + ((s16)y << 7)];
                d2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
                blast_things(m);
                if ((old_newrand() & 0x3ff) > (d2 << 10) / r2)
                    blast_walls_and_floor(cell);
            }
        }
    }
    othervar.full_redraw = 1;
}

// Damages tile's north wall with a 70% chance, by placing a damage marker
// (small thing 0x65 or 0x67); if the north wall was not hit, tries its west
// wall the same way (0x66 or 0x68). The floor is left alone.
// FUNCTION: 0x000757a0
void blast_walls_and_floor(u16 tile)
{
    u8 flags;
    u8 block;
    Map *m;

    m = &map[tile];
    if (old_newrand() % 1000 > 300) {
        block = m->wall_n;
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
        block = m->wall_w;
        flags = blocks[block].width;
        if (flags & 2) {
            if (!(flags & 0x10))
                place_small_thing(tile, 0x68, 1);
        } else if (flags & 1) {
            place_small_thing(tile, 0x66, 1);
        }
    }
}

// Removes every object on map cell m, leaving its debris piece where it has
// one.
// FUNCTION: 0x000758b8
void blast_things(Map *m)
{
    u8 facing;
    u8 debris;
    Thing *obj;

restart:
    for (obj = &thing[m->first]; obj > thing; obj = &thing[obj->cell_next]) {
        if (obj->type == THING_OBJECT) {
            debris = objects[obj->subtype].debris_lo;
            facing = obj->facing;
            remove_object(obj, obj->owner);
            if (debris)
                place_small_thing(m - map, debris + facing, 0);
            goto restart;
        }
    }
}

// Fires (sack_staff mode 2) all staff in room and kills all patients in it.
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
            if (person->type == THING_STAFF) {
                sack_staff(person, 2);
                again = 1;
                break;
            }
        }
    }
    for (person = &thing[room->first]; person > thing; person = &thing[person->room_next]) {
        if (person->type == THING_PATIENT)
            person_dies(person);
    }
}

// Takes every patient out of room's queue and sends them on again, to the GP
// if the hospital cannot treat their treatment.
// FUNCTION: 0x00075aac
void restart_entire_queue(RoomInfo *room)
{
    Thing *patient;

    while ((patient = &thing[room->queue_head]) > thing) {
        if (allhosps[patient->owner].rooms[patient->treatment].cure_available != 1)
            patient->treatment = ILL_GENERAL_PRACTICE;
        leave_queue(patient);
        interrupt_person(patient);
        start_person_going(patient);
    }
}

// Daily: runs owner's random disaster (allhosps.disaster_kind). Every gbv.disaster_launch days a
// new one may start: 1 or 2 the heating fails cold or hot (when there are
// radiators and fewer than one handyman per 8 of allhosps.radiator_count), 3 everyone
// feels sick (while there are over 10 pieces of litter, allhosps.litter_count). A
// running one counts down (faster for the heating with more handymen) and then
// ends, with advisor messages at start and end.
// FUNCTION: 0x00075b54
void generate_disaster(u8 owner)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    switch (hosp->disaster_kind) {
    case 0:
        hosp->disaster_counter++;
        if (hosp->disaster_counter > gbv.disaster_launch) {
            hosp->disaster_counter = 0;
            hosp->disaster_kind = old_newrand() & 3;
            switch (hosp->disaster_kind) {
            case 0:
                break;
            case 1:
                if (listhead[owner].objects[OBJ_RADIATOR] && hosp->staff_by_kind[2] * 8 < hosp->radiator_count) {
                    hosp->saved_heating = hosp->heating_level;
                    hosp->heating_level = 0;
                    hosp->heating_broken = 1;
                    advisor.forced_advice_id = 0xb;
                    hosp->disaster_timer = old_normal_rand(10, 0x1e);
                } else {
                    hosp->disaster_kind = 0;
                }
                break;
            case 2:
                if (listhead[owner].objects[OBJ_RADIATOR] && hosp->staff_by_kind[2] * 8 < hosp->radiator_count) {
                    hosp->saved_heating = hosp->heating_level;
                    hosp->heating_level = 0xa;
                    hosp->heating_broken = 1;
                    advisor.forced_advice_id = 0xa;
                    hosp->disaster_timer = old_normal_rand(10, 0x1e);
                } else {
                    hosp->disaster_kind = 0;
                }
                break;
            case 3:
                if (hosp->litter_count > 10) {
                    disaster_peeps(0, owner);
                    advisor.forced_advice_id = 0xd;
                    hosp->disaster_timer = old_normal_rand(5, 10);
                } else {
                    hosp->disaster_kind = 0;
                }
                break;
            }
        }
        break;
    case 1:
    case 2:
        if (hosp->staff_by_kind[2] * 8 > hosp->radiator_count)
            hosp->disaster_timer--;
        if (hosp->staff_by_kind[2] * 5 > hosp->radiator_count)
            hosp->disaster_timer--;
        hosp->disaster_timer--;
        if (hosp->disaster_timer <= 0) {
            hosp->disaster_kind = 0;
            hosp->heating_level = hosp->saved_heating;
            hosp->heating_broken = 0;
            advisor.forced_advice_id = 0xc;
        }
        break;
    case 3:
        if (hosp->litter_count > 10)
            disaster_peeps(0, owner);
        else
            hosp->disaster_timer--;
        hosp->disaster_timer--;
        if (hosp->disaster_timer <= 0) {
            hosp->disaster_kind = 0;
            advisor.forced_advice_id = 0xe;
        }
        break;
    }
}

// Applies mode to every patient of owner (-1 for all): 0 makes them about to
// vomit, 1 and 2 set and clear PF_QUAKE_SHAKEN for those whose kind allows it,
// 3 kills them.
// FUNCTION: 0x00075ed0
void disaster_peeps(u8 mode, s8 owner)
{
    Thing *peep;

    for (peep = thing + 1; peep < end_things; peep++) {
        if (peep->type == THING_PATIENT && (peep->owner == owner || owner == -1)) {
            if (mode == 0)
                peep->nausea = (u16)gbv.vomit_limit + 10;
            else if (mode == 1 && abilities[peep->subtype][5])
                peep->flags |= PF_QUAKE_SHAKEN;
            else if (mode == 2 && abilities[peep->subtype][5])
                peep->flags &= ~PF_QUAKE_SHAKEN;
            else if (mode == 3)
                change_health(peep, -peep->health);
        }
    }
}

// Fills required with what owner lacks to handle illness (an expertise index):
// the room, whether it is available, staff with the needed skill (two
// surgeons for surgery), a ward and nurse, or that nothing is lacking
// (required.treatable).
// FUNCTION: 0x00075fe4
void what_is_required(u8 illness, u8 owner)
{
    s32 unused;
    Expertise *exp;
    s32 i;

    unused = 0;
    memset(&required, 0, 10);
    exp = &expertise[illness];
    if (allhosps[owner].rooms[illness].cure_available == 1) {
        required.treatable = 1;
        if (exp->kind == 0x27)
            required.raw[2] = allhosps[owner].rooms[illness].drug_effectiveness;
        return;
    }
    if (listhead[owner].rooms[exp->category] == 0) {
        required.missing_room = exp->category;
        if (room_available((u16)exp->category) == 0)
            required.room_unavailable = 1;
    }
    if (exp->staff_mask & 0x20) {
        if (listhead[owner].staff[5] == 0) {
            required.flags = 0x20;
            required.missing_skill = 5;
            required.surgeons_needed = 2;
        } else if (allhosps[owner].surgeon_count < 2) {
            required.surgeons_needed = 2 - allhosps[owner].surgeon_count;
            required.flags = 0x20;
            required.missing_skill = 5;
        }
    } else {
        for (i = 0; i < 9; i++) {
            if ((exp->staff_mask & (1 << i)) && listhead[owner].staff[i] == 0) {
                required.flags = exp->staff_mask;
                required.missing_skill = i;
                break;
            }
        }
    }
    if (exp->needs_ward) {
        if (allhosps[owner].rooms[ILL_DIAG_WARD].cure_available == 0) {
            if (listhead[owner].rooms[ROOM_WARD] == 0)
                required.needs_ward = 1;
            if (listhead[owner].staff[2] == 0)
                required.needs_nurse = 1;
        }
    }
}

// Abductions are disabled: always returns 0.
// FUNCTION: 0x0007628c
extern "C" u8 shall_i_be_abducted(Thing *w)
{
    return 0;
}

// Does nothing in this build.
// FUNCTION: 0x000762b0
void saucer_abducts(Thing *w)
{
}

// Creates a shadow thing at tile for thing caster, drawn with animation anim
// below other things. Returns its index, or 0 if no thing is free.
// FUNCTION: 0x000762cc
u16 create_shadow(u16 tile, u16 caster, u8 owner, u16 anim)
{
    Thing *shadow;

    shadow = allocate_thing(0x32);
    if (shadow) {
        shadow->type = THING_SHADOW;
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
