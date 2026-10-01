#include "hosp.h"

extern "C" {
}

u16 create_inspector(u8 owner);
void send_peeps_home(u8 all, u8 owner);

// Creates the health inspector for owner at one of the hospital's entrances,
// walking to the reception desk to inspect (goto_state MAYOR_INSPECTS), or straight back
// out if there is none. Returns its thing index, or 0 if no thing is free.
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
    pos = room_info[allhosps[owner].entrances[old_newrand() & 3]].door_tile;
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
        inspector->goto_state = MAYOR_INSPECTS;
        inspector->inspect_room = 0;
    } else {
        set_destination_coords(inspector, room_info[allhosps[inspector->owner].entrances[old_newrand() & 3]].door_tile);
        inspector->state = MAYOR_GOTO;
        inspector->goto_state = MAYOR_GONE;
    }
    return inspector - thing;
    }
    return 0;
}

// Sends the health inspector to owner after an epidemic cover-up, unless
// one is already on the way.
// FUNCTION: 0x000a1e3c
void generate_inspector(u8 owner)
{
    if (allhosps[owner].inspection[1] != 0)
        return;
    if (allhosps[owner].inspector = create_inspector(owner)) {}
    lib_strcpy(debug.msg_text, "Inspector Generated");
    debug.msg_timer = 0xc8;
    allhosps[owner].inspection[1] = 3;
}

// The inspector's visit: after the pause, leaves and judges the epidemic by
// the contagious patients still inside (allhosps.inspection[0], harsher the
// more there are): 0 (always when none, rarely otherwise) pays compensation;
// 1 fines gbv.epidemic_fine * 2 per contagious patient and sends them home; 2 also
// costs reputation; 3 costs reputation and sends every patient home. Ends the
// epidemic and raises the result fax.
// FUNCTION: 0x000a1ed4
void inspector_pauses(Thing *inspector)
{
    AllHosps *hosp;
    u32 contagious;

    if (--inspector->timer != 0)
        return;
    hosp = &allhosps[inspector->owner];
    set_destination_coords(inspector, room_info[hosp->entrances[old_newrand() & 3]].door_tile);
    inspector->state = MAYOR_GOTO;
    inspector->goto_state = MAYOR_GONE;
    contagious = count_and_flag_contagious_peeps(hosp->epidemic_illness, 2, inspector->owner);
    if (contagious == 0) {
        hosp->inspection[0] = 0;
    } else if (contagious <= 2) {
        if (old_newrand() % 5 == 0)
            hosp->inspection[0] = 0;
        else
            hosp->inspection[0] = 1;
    } else if (contagious <= 5) {
        if (old_newrand() % 20 == 0)
            hosp->inspection[0] = 0;
        else if (old_newrand() % 10 == 0)
            hosp->inspection[0] = 1;
        else
            hosp->inspection[0] = 2;
    } else if (contagious < 10) {
        if (old_newrand() % 50 == 0)
            hosp->inspection[0] = 0;
        else if (old_newrand() % 25 == 0)
            hosp->inspection[0] = 1;
        else
            hosp->inspection[0] = 2;
    } else {
        if (old_newrand() % 50 == 0)
            hosp->inspection[0] = 0;
        else
            hosp->inspection[0] = 3;
    }
    hosp->epidemic_fine = gbv.epidemic_fine * contagious * 2;
    make_a_playercall(5, 3, 0x14, inspector->owner);
    hosp->inspected_illness = hosp->epidemic_illness;
    hosp->epidemic_illness = 0;
    hosp->inspection[1] = 0;
    hosp->epidemic_over = 0;
    switch (hosp->inspection[0]) {
    case 0:
        hosp->epidemic_fine = old_normal_rand(gbv.epidemic_comp_lo, gbv.epidemic_comp_hi);
        change_balance(TX_GOVERNMENT_COMPENSATION, hosp->epidemic_fine, 0, (u16)inspector->owner);
        break;
    case 1:
        change_balance(TX_EPIDEMIC_COVERUP_FINE, -hosp->epidemic_fine, 0, (u16)inspector->owner);
        send_peeps_home(0, inspector->owner);
        break;
    case 2:
        reputation_affected(0, REP_EPIDEMIC_COVER_UP_FOUND, 0, inspector->owner, 0);
        change_balance(TX_EPIDEMIC_COVERUP_FINE, -hosp->epidemic_fine, 0, (u16)inspector->owner);
        send_peeps_home(0, inspector->owner);
        break;
    case 3:
        reputation_affected(0, REP_EPIDEMIC_COVER_UP_FOUND, 0, inspector->owner, 0);
        send_peeps_home(1, inspector->owner);
        break;
    }
}
