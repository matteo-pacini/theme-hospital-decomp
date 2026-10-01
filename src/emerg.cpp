#include "hosp.h"

struct EmergencyDef {
    u16 start;
    u16 end;
    u8 min;
    u8 max;
    u8 illness;
    u8 f7;
    u16 f8;
};

struct PlayerPopup {
    u16 call;
    u8 unknown2[0x6a];
};

extern "C" {
extern PlayerPopup D_0015f5c2[];
extern char **illness_text;
extern char **room_names;
extern char **skill_names;
void fn_000c93bf(char *dst, char *src);
extern u16 choice;
extern u8 D_00158192;
extern Event D_001549d0[];
extern EmergencyDef emergency_control[];
}

u32 check_emergency_criteria(void);
void setup_next_emergency(void);
void setup_emergency_stats(u16 call, u8 owner);
u16 summon_vehicle(u16 owner, u8 kind, u16 cell, u16 anim, u16 shadow_anim);
u32 old_newrand(void);


u16 saferand(void);
u16 count_emergency_staff(u8 owner);
u16 create_shadow(u16 tile, u16 caster, u8 owner, u16 anim);

// Daily emergency handling: sends a helicopter to each hospital that has just
// accepted an emergency (allhosps.emergency_state 5, then 3 once summoned, or 0 if no
// thing was free); when the next scheduled emergency is due (and some hospital
// has seen its illness) offers it to every active hospital by fax, then
// schedules the following one; counts down the waits (f1e2 above 5).
// FUNCTION: 0x000aeec0
void try_to_generate_emergency(void)
{
    s32 found;
    s32 i;
    s32 j;
    AllHosps *hosp;
    s32 hosp_count;

    found = 0;
    hosp_count = 1;
    if (statvar.num_players > 1)
        hosp_count = 4;
    for (i = 0; i < hosp_count; i++) {
        if (statvar.num_players <= 1 || D_001609c4[i].active != 0 || 0) {
            hosp = &allhosps[i];
            if (hosp->emergency_state_s == 5) {
                hosp->emergency_heli = summon_vehicle(i, 0, allhosps[i].heli_pad_tile + 0x80, 0x4cc, 0x7a7);
                if (hosp->emergency_heli) {
                    hosp->emergency_state = 3;
                    hosp->heli_landed = 0;
                } else {
                    hosp->emergency_to_spawn = 0;
                    hosp->emergency_state = 0;
                }
            }
        }
    }
    if (check_emergency_criteria()) {
        for (j = 0; j < hosp_count; j++) {
            if (statvar.num_players <= 1 || D_001609c4[j].active != 0 || 0) {
                if (allhosps[j].rooms[gamevar.emergency_illness].discovered == 1)
                    found = 1;
            }
        }
        if (found) {
            for (j = 0; j < hosp_count; j++) {
                if (statvar.num_players <= 1 || D_001609c4[j].active != 0 || 0)
                    setup_emergency_stats(make_a_playercall(2, 0, 0x14, j), j);
            }
        }
        setup_next_emergency();
    }
    for (i = 0; i < hosp_count; i++) {
        if (statvar.num_players <= 1 || D_001609c4[i].active != 0 || 0) {
            hosp = &allhosps[i];
            if (hosp->emergency_state_s > 5)
                hosp->emergency_state--;
        }
    }
}

// Returns 1 if the scheduled emergency (gamevar.next_emergency_turn) is due, statvar.illness_kind_count is
// set, fewer than 4 offers are outstanding, and no active hospital has an
// emergency call (type 3) pending, allhosps.epidemic_active set, an emergency running
// (f1e2), or, in single player, unknown1256 clear; else 0. Never while a single
// player is viewing an epidemic fax.
// FUNCTION: 0x000af118
u32 check_emergency_criteria(void)
{
    u8 i;
    s32 hosp_count;

    hosp_count = 1;
    if (statvar.num_players > 1)
        hosp_count = 4;
    if (statvar.num_players <= 1 && display_mode == 9 && choice == 5)
        return 0;
    if (actual_game_turn >= gamevar.next_emergency_turn && gamevar.emergency_pending) {
        for (i = 0; i < hosp_count; i++) {
            if (statvar.num_players <= 1 || D_001609c4[i].active != 0 || 0) {
                if (check_playercall(3, 0, i))
                    return 0;
                if (allhosps[i].epidemic_active)
                    return 0;
                if (allhosps[i].emergency_state_s != 0)
                    return 0;
                if (!(statvar.num_players > 1 ? 1 : allhosps[i].hospital_open))
                    return 0;
            }
        }
        if (statvar.illness_kind_count && gamevar.emergencies_active < 4)
            return 1;
    }
    return 0;
}

// Schedules the next emergency from emergency_control: the first unused entry
// still ahead picks the turn (gamevar.next_emergency_turn, within its months), illness,
// patient count and terms. Clears gamevar.emergency_pending if none is left.
// FUNCTION: 0x000af2c0
void setup_next_emergency(void)
{
    u32 i;
    u32 r;
    u32 month;

    month = mycontrol->year * 12 + mycontrol->month;
    gamevar.emergency_pending = 0;
    for (i = 0; i < 20; i++) {
        if (gamevar.emergency_used[i] == 0) {
            gamevar.emergency_used[i] = 1;
            if (emergency_control[i].end > month && emergency_control[i].end > emergency_control[i].start) {
                r = old_newrand() % ((emergency_control[i].end + 1 - emergency_control[i].start) * 0x5dc);
                gamevar.next_emergency_turn = r + emergency_control[i].start * 0x5dc;
                gamevar.emergency_illness = emergency_control[i].illness;
                r = old_newrand() % (emergency_control[i].max + 1 - emergency_control[i].min);
                gamevar.emergency_patients = r + emergency_control[i].min;
                gamevar.emergency_win_percent = emergency_control[i].f7;
                gamevar.emergency_bonus = emergency_control[i].f8;
                gamevar.emergency_pending = 1;
                break;
            }
        }
    }
}

// Stores the scheduled emergency's details for the local player's offer fax
// (call is its player call), in a free gamevar.emergency_calls slot.
// FUNCTION: 0x000af448
void setup_emergency_stats(u16 call, u8 owner)
{
    u32 i;

    if (owner == player) {
        for (i = 0; i < 4; i++) {
            if (gamevar.emergency_calls[i].used == 0)
                break;
        }
        gamevar.emergency_calls[i].used = 1;
        gamevar.emergency_calls[i].call = call;
        gamevar.emergency_calls[i].bonus = gamevar.emergency_bonus;
        gamevar.emergency_calls[i].illness = gamevar.emergency_illness;
        gamevar.emergency_calls[i].count = gamevar.emergency_patients;
        gamevar.emergency_calls[i].win_percent = gamevar.emergency_win_percent;
        gamevar.emergencies_active++;
        othervar.emergency_patient = 0;
    }
}

// Gives owner the emergency stored for player call call (illness, patient
// count, bonus per patient, cure percentage required) and frees its slot.
// FUNCTION: 0x000af510
void give_emergency_stats(u16 call, u8 owner)
{
    AllHosps *hosp;
    u32 i;

    hosp = &allhosps[owner];
    for (i = 0; i < 4; i++) {
        if (gamevar.emergency_calls[i].call == call && gamevar.emergency_calls[i].used != 0)
            break;
    }
    hosp->emergency_illness = gamevar.emergency_calls[i].illness;
    hosp->emergency_patients = gamevar.emergency_calls[i].count;
    hosp->emergency_to_spawn = hosp->emergency_patients;
    hosp->emergency_bonus = gamevar.emergency_calls[i].bonus;
    hosp->emergency_win_pct = gamevar.emergency_calls[i].win_percent;
    gamevar.emergency_calls[i].used = 0;
    gamevar.emergencies_active--;
}

// Composes the emergency fax for owner from strs: who is calling, the patient
// count and illness, whether the hospital can treat it (or the room and staff
// it lacks), and the bonus. The accept button is disabled while an emergency
// is running. Always returns 0.
// FUNCTION: 0x000af600
u32 announce_emergency(u8 owner, char **strs)
{
    s32 width;
    s32 missing_room;
    s32 missing_staff;
    s32 cured;
    s32 count;
    s32 line_height;
    AllHosps *hosp;
    u16 call;
    s32 i;
    s32 staff_count;
    s32 patients;
    s32 bonus;
    s32 illness;
    Expertise *exp;
    char name[200];
    char text[1000];
    u16 x;
    u16 y;
    char buf[200];

    width = 400;
    missing_room = 0;
    missing_staff = 0;
    cured = 0;
    count = 0;
    char new_line[] = "\n";
    line_height = lbFontPtr ? lbFontPtr[34].height : 0;
    hosp = &allhosps[owner];
    if (hosp->emergency_state_s != 0)
        available_buttons &= ~1;
    call = D_0015f5c2[owner].call;
    for (i = 0; i < 4; i++) {
        if (gamevar.emergency_calls[i].call == call && gamevar.emergency_calls[i].used)
            break;
    }
    staff_count = count_emergency_staff(owner);
    patients = gamevar.emergency_calls[i].count;
    bonus = patients * gamevar.emergency_calls[i].bonus;
    illness = gamevar.emergency_calls[i].illness;
    exp = &expertise[illness];
    if (allhosps[owner].rooms[illness].cure_available == 1) {
        cured = 1;
        if (exp->kind == 0x27)
            count = hosp->rooms[illness].drug_effectiveness;
    } else {
        if (listhead[owner].rooms[exp->category] == 0 && room_available(exp->category))
            missing_room = exp->category;
        for (i = 0; i < 9; i++) {
            if ((exp->staff_mask & (1 << i)) && listhead[owner].staff[i] == 0) {
                missing_staff = i;
                break;
            }
        }
    }
    fn_000bbff0(name, strs[saferand() % 4 + 9]);
    text[0] = 0;
    x = 0;
    y = 0;
    sprintf(buf, strs[0], name);
    fn_000c93bf(text, buf);
    sprintf(buf, strs[1], patients, illness_text[illness]);
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, buf);
    if (cured) {
        if (count)
            sprintf(buf, strs[2], illness_text[illness], count);
        else
            sprintf(buf, strs[3]);
    } else if (missing_room && missing_staff) {
        sprintf(buf, strs[4], room_names[rooms[missing_room].id], skill_names[missing_staff]);
    } else if (missing_room) {
        sprintf(buf, strs[5], room_names[rooms[missing_room].id]);
    } else if (missing_staff) {
        sprintf(buf, strs[6], skill_names[missing_staff]);
    } else {
        sprintf(buf, strs[7]);
    }
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, buf);
    sprintf(buf, strs[8], bonus);
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, buf);
    LbTextDraw(x, y, text);
    return 0;
}

// Emergency offer fax button: queues the accept (0) or refuse (1) answer.
// FUNCTION: 0x000afab8
void process_emergency_start_buttons(u8 button)
{
    switch (button) {
    case 0:
        D_001549d0[player].type = 0x7d;
        D_001549d0[player].value = 0x1f;
        break;
    case 1:
        D_001549d0[player].type = 0x7d;
        D_001549d0[player].value = 0x1e;
        break;
    }
}

// Composes the emergency result fax for owner from strs: the bonus earned
// (if enough were cured) and how many of the patients were saved.
// FUNCTION: 0x000afb34
void emergency_over(u8 owner, char **strs)
{
    char new_line[] = "\n";
    s32 line_height;
    AllHosps *hosp;
    s32 bonus;
    s32 total;
    char text[2000];
    u16 x;
    u16 y;
    char buf[200];

    line_height = lbFontPtr ? lbFontPtr[34].height : 0;
    hosp = &allhosps[owner];
    if (hosp->emergency_cured * 100 / hosp->emergency_patients >= hosp->emergency_win_pct)
        bonus = hosp->emergency_bonus * hosp->emergency_cured;
    else
        bonus = 0;
    total = hosp->emergency_bonus * hosp->emergency_patients;
    text[0] = 0;
    x = 0;
    y = 0;
    sprintf(buf, strs[0], total, bonus);
    fn_000c93bf(text, buf);
    sprintf(buf, strs[1], hosp->emergency_cured, hosp->emergency_patients);
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, buf);
    LbTextDraw(x, y, text);
}

// Emergency result fax button 0 closes the fax.
// FUNCTION: 0x000afcd0
void process_emergency_end_buttons(u8 button)
{
    switch (button) {
    case 0:
        D_00158192 = 0x5c;
        break;
    }
}

// Creates a vehicle (helicopter, kind 0; saucer, kind 1) for owner at cell,
// descending to land (HELI_LANDS), with its shadow. For the local player
// plays an arrival sound. Returns its thing index, or 0 if none is free.
// FUNCTION: 0x000afd0c
u16 summon_vehicle(u16 owner, u8 kind, u16 cell, u16 anim, u16 shadow_anim)
{
    Thing *vehicle;

    vehicle = allocate_thing(0x19);
    if (vehicle) {
        vehicle->type = THING_HELICOPTER;
        vehicle->subtype = kind;
        vehicle->owner = owner;
        vehicle->x.tile = cell % 128;
        vehicle->y.tile = cell / 128;
        vehicle->x.frac = 0x80;
        vehicle->y.frac = 0x80;
        vehicle->anim_speed = 2;
        vehicle->facing = 1;
        insert_node(vehicle, vehicle->x, vehicle->y);
        vehicle->speed = 8;
        vehicle->altitude = 0x190;
        vehicle->sprite = start[anim].frame;
        vehicle->flip = start[anim].flags;
        vehicle->state = HELI_LANDS;
        vehicle->shadow = create_shadow(cell, vehicle - thing, owner, shadow_anim);
        if (owner == player) {
            if (anim == 0x792)
                play_ambient(saferand() % 5 + 0xc7);
            else if (myhosp->emergency_illness >= 0 && myhosp->emergency_illness < 0x24)
                play_ambient(myhosp->emergency_illness + 0x1f);
        }
        return vehicle - thing;
    }
    return 0;
}

// HELI_LANDS: animates heli and lowers it; on landing a helicopter waits
// (state 4) and marks the hospital's emergency as landed, a saucer starts
// abducting (HELI_ABDUCTS).
// FUNCTION: 0x000afef4
void helicopter_lands(Thing *heli)
{
    heli->sprite = frame[heli->sprite].next;
    if (heli->altitude > 0) {
        heli->altitude -= heli->speed;
    } else {
        switch (heli->subtype) {
        case 0:
            heli->altitude = 0;
            heli->state = 4;
            allhosps[heli->owner].heli_landed = 1;
            if (heli->owner == player)
                advisor.heli_landed = 1;
            break;
        case 1:
            heli->sprite = start[0x7a8].frame;
            heli->flip = start[0x7a8].flags;
            heli->altitude = 0;
            heli->state = HELI_ABDUCTS;
            break;
        }
    }
}

// HELI_LEAVES: animates heli and raises it; at the top removes it and its
// shadow and clears the emergency's (or the saucer's) records.
// FUNCTION: 0x000b0000
void helicopter_leaves(Thing *heli)
{
    AllHosps *hosp;

    hosp = &allhosps[heli->owner];
    heli->sprite = frame[heli->sprite].next;
    if (heli->altitude < 0x190) {
        heli->altitude += heli->speed;
    } else {
        switch (heli->subtype) {
        case 0:
            hosp->heli_landed = 0;
            hosp->emergency_heli = 0;
            break;
        case 1:
            gamevar.ff5 = 0;
            gamevar.ff7 = 0;
            break;
        }
        if (heli->shadow)
            remove_thing(&thing[(u16)heli->shadow]);
        remove_thing(heli);
    }
}
