#include "hosp.h"

extern "C" {
}

u16 create_inspector(u8 owner);
void send_peeps_home(u8 all, u8 owner);

// FUNCTION: 0x000a1c10
u16 create_inspector(u8 owner)
{
    Thing *inspector;
    u16 pos;
    u16 entry;

    inspector = allocate_thing(0x19);
    if (inspector != 0) {
    Thing *obj;
    inspector->type = THING_INSPECTOR;
    inspector->owner = owner;
    pos = room_info[allhosps[owner].unknown1cf[old_newrand() & 3]].unknown4;
    inspector->x.tile = pos % 128;
    inspector->y.tile = pos / 128;
    inspector->x.frac = 0x80;
    inspector->y.frac = 0x80;
    inspector->speed = 0x20;
    inspector->anim_speed = 1;
    inspector->facing = 1;
    insert_node(inspector, inspector->x, inspector->y);
    entry = check_frame(0x89, inspector) + inspector->facing;
    inspector->sprite = start[entry].frame;
    inspector->flip = start[entry].flags > 0 ? (u8)1 : (u8)0;
    obj = &thing[listhead[owner].objects[OBJ_RECEPTION_DESK]];
    if (obj > thing) {
        set_destination_coords(inspector, obj->use_pos[1]);
        inspector->state = MAYOR_GOTO;
        inspector->goto_state = 8;
        inspector->inspect_room = 0;
    } else {
        set_destination_coords(inspector, room_info[allhosps[inspector->owner].unknown1cf[old_newrand() & 3]].unknown4);
        inspector->state = MAYOR_GOTO;
        inspector->goto_state = MAYOR_GONE;
    }
    return inspector - thing;
    }
    return 0;
}

// FUNCTION: 0x000a1e3c
void generate_inspector(u8 owner)
{
    if (allhosps[owner].unknown209[1] != 0)
        return;
    if (allhosps[owner].f201 = create_inspector(owner)) {}
    fn_000bbff0(debug.unknown116, "Inspector Generated");
    debug.unknown1de = 0xc8;
    allhosps[owner].unknown209[1] = 3;
}

// FUNCTION: 0x000a1ed4
void inspector_pauses(Thing *inspector)
{
    AllHosps *hosp;
    u32 contagious;

    if (--inspector->timer != 0)
        return;
    hosp = &allhosps[inspector->owner];
    set_destination_coords(inspector, room_info[hosp->unknown1cf[old_newrand() & 3]].unknown4);
    inspector->state = MAYOR_GOTO;
    inspector->goto_state = MAYOR_GONE;
    contagious = count_and_flag_contagious_peeps(hosp->f20b, 2, inspector->owner);
    if (contagious == 0) {
        hosp->unknown209[0] = 0;
    } else if (contagious <= 2) {
        if (old_newrand() % 5 == 0)
            hosp->unknown209[0] = 0;
        else
            hosp->unknown209[0] = 1;
    } else if (contagious <= 5) {
        if (old_newrand() % 20 == 0)
            hosp->unknown209[0] = 0;
        else if (old_newrand() % 10 == 0)
            hosp->unknown209[0] = 1;
        else
            hosp->unknown209[0] = 2;
    } else if (contagious < 10) {
        if (old_newrand() % 50 == 0)
            hosp->unknown209[0] = 0;
        else if (old_newrand() % 25 == 0)
            hosp->unknown209[0] = 1;
        else
            hosp->unknown209[0] = 2;
    } else {
        if (old_newrand() % 50 == 0)
            hosp->unknown209[0] = 0;
        else
            hosp->unknown209[0] = 3;
    }
    hosp->f203 = gbv.f12e * contagious * 2;
    make_a_playercall(5, 3, 0x14, inspector->owner);
    hosp->f20c = hosp->f20b;
    hosp->f20b = 0;
    hosp->unknown209[1] = 0;
    hosp->f20e = 0;
    switch (hosp->unknown209[0]) {
    case 0:
        hosp->f203 = old_normal_rand(gbv.f130, gbv.f132);
        change_balance(TX_GOVERNMENT_COMPENSATION, hosp->f203, 0, (u16)inspector->owner);
        break;
    case 1:
        change_balance(TX_EPIDEMIC_COVERUP_FINE, -hosp->f203, 0, (u16)inspector->owner);
        send_peeps_home(0, inspector->owner);
        break;
    case 2:
        reputation_affected(0, 0xf, 0, inspector->owner, 0);
        change_balance(TX_EPIDEMIC_COVERUP_FINE, -hosp->f203, 0, (u16)inspector->owner);
        send_peeps_home(0, inspector->owner);
        break;
    case 3:
        reputation_affected(0, 0xf, 0, inspector->owner, 0);
        send_peeps_home(1, inspector->owner);
        break;
    }
}
