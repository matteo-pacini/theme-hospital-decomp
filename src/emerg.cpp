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
extern char **D_001602a4;
extern char **D_001602cc;
extern char **D_00160318;
void fn_000c93bf(char *dst, char *src);
extern u16 D_00158190;
extern u8 D_00158192;
extern Event D_001549d0[];
extern EmergencyDef D_0015fc3c[];
}

u32 check_emergency_criteria(void);
void setup_next_emergency(void);
void setup_emergency_stats(u16 call, u8 owner);
u16 summon_vehicle(u16 owner, u8 kind, u16 cell, u16 anim, u16 shadow_anim);
u32 old_newrand(void);


u16 saferand(void);
u16 count_emergency_staff(u8 owner);
u16 create_shadow(u16 tile, u16 height, u8 owner, u16 anim);

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
    if (statvar.unknownfd > 1)
        hosp_count = 4;
    for (i = 0; i < hosp_count; i++) {
        if (statvar.unknownfd <= 1 || D_001609c4[i].active != 0 || 0) {
            hosp = &allhosps[i];
            if (hosp->f1e2s == 5) {
                hosp->f1df = summon_vehicle(i, 0, allhosps[i].f1d7 + 0x80, 0x4cc, 0x7a7);
                if (hosp->f1df) {
                    hosp->f1e2 = 3;
                    hosp->f1e1 = 0;
                } else {
                    hosp->f1e3 = 0;
                    hosp->f1e2 = 0;
                }
            }
        }
    }
    if (check_emergency_criteria()) {
        for (j = 0; j < hosp_count; j++) {
            if (statvar.unknownfd <= 1 || D_001609c4[j].active != 0 || 0) {
                if (allhosps[j].rooms[gamevar.f124].unknown1D == 1)
                    found = 1;
            }
        }
        if (found) {
            for (j = 0; j < hosp_count; j++) {
                if (statvar.unknownfd <= 1 || D_001609c4[j].active != 0 || 0)
                    setup_emergency_stats(make_a_playercall(2, 0, 0x14, j), j);
            }
        }
        setup_next_emergency();
    }
    for (i = 0; i < hosp_count; i++) {
        if (statvar.unknownfd <= 1 || D_001609c4[i].active != 0 || 0) {
            hosp = &allhosps[i];
            if (hosp->f1e2s > 5)
                hosp->f1e2--;
        }
    }
}

// FUNCTION: 0x000af118
u32 check_emergency_criteria(void)
{
    u8 i;
    s32 hosp_count;

    hosp_count = 1;
    if (statvar.unknownfd > 1)
        hosp_count = 4;
    if (statvar.unknownfd <= 1 && display_mode == 9 && D_00158190 == 5)
        return 0;
    if (actual_game_turn >= gamevar.f11d && gamevar.f123) {
        for (i = 0; i < hosp_count; i++) {
            if (statvar.unknownfd <= 1 || D_001609c4[i].active != 0 || 0) {
                if (check_playercall(3, 0, i))
                    return 0;
                if (allhosps[i].f20d)
                    return 0;
                if (allhosps[i].f1e2s != 0)
                    return 0;
                if (!(statvar.unknownfd > 1 ? 1 : allhosps[i].unknown1256))
                    return 0;
            }
        }
        if (statvar.f6a && gamevar.f13e < 4)
            return 1;
    }
    return 0;
}

// FUNCTION: 0x000af2c0
void setup_next_emergency(void)
{
    u32 i;
    u32 r;
    u32 month;

    month = mycontrol->f648 * 12 + mycontrol->unknown650;
    gamevar.f123 = 0;
    for (i = 0; i < 20; i++) {
        if (gamevar.f15b[i] == 0) {
            gamevar.f15b[i] = 1;
            if (D_0015fc3c[i].end > month && D_0015fc3c[i].end > D_0015fc3c[i].start) {
                r = old_newrand() % ((D_0015fc3c[i].end + 1 - D_0015fc3c[i].start) * 0x5dc);
                gamevar.f11d = r + D_0015fc3c[i].start * 0x5dc;
                gamevar.f124 = D_0015fc3c[i].illness;
                r = old_newrand() % (D_0015fc3c[i].max + 1 - D_0015fc3c[i].min);
                gamevar.f125 = r + D_0015fc3c[i].min;
                gamevar.f126 = D_0015fc3c[i].f7;
                gamevar.f121 = D_0015fc3c[i].f8;
                gamevar.f123 = 1;
                break;
            }
        }
    }
}

// FUNCTION: 0x000af448
void setup_emergency_stats(u16 call, u8 owner)
{
    u32 i;

    if (owner == player) {
        for (i = 0; i < 4; i++) {
            if (gamevar.ffd[i].used == 0)
                break;
        }
        gamevar.ffd[i].used = 1;
        gamevar.ffd[i].call = call;
        gamevar.ffd[i].f2 = gamevar.f121;
        gamevar.ffd[i].illness = gamevar.f124;
        gamevar.ffd[i].count = gamevar.f125;
        gamevar.ffd[i].f7 = gamevar.f126;
        gamevar.f13e++;
        othervar.fa88 = 0;
    }
}

// FUNCTION: 0x000af510
void give_emergency_stats(u16 call, u8 owner)
{
    AllHosps *hosp;
    u32 i;

    hosp = &allhosps[owner];
    for (i = 0; i < 4; i++) {
        if (gamevar.ffd[i].call == call && gamevar.ffd[i].used != 0)
            break;
    }
    hosp->f1e4 = gamevar.ffd[i].illness;
    hosp->f1e6 = gamevar.ffd[i].count;
    hosp->f1e3 = hosp->f1e6;
    hosp->f1dd = gamevar.ffd[i].f2;
    hosp->f1e8 = gamevar.ffd[i].f7;
    gamevar.ffd[i].used = 0;
    gamevar.f13e--;
}

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
    if (hosp->f1e2s != 0)
        available_buttons &= ~1;
    call = D_0015f5c2[owner].call;
    for (i = 0; i < 4; i++) {
        if (gamevar.ffd[i].call == call && gamevar.ffd[i].used)
            break;
    }
    staff_count = count_emergency_staff(owner);
    patients = gamevar.ffd[i].count;
    bonus = patients * gamevar.ffd[i].f2;
    illness = gamevar.ffd[i].illness;
    exp = &expertise[illness];
    if (allhosps[owner].rooms[illness].unknown1C == 1) {
        cured = 1;
        if (exp->kind == 0x27)
            count = hosp->rooms[illness].unknown1B;
    } else {
        if (listhead[owner].rooms[exp->category] == 0 && room_available(exp->category))
            missing_room = exp->category;
        for (i = 0; i < 9; i++) {
            if ((exp->unknown0 & (1 << i)) && listhead[owner].staff[i] == 0) {
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
    sprintf(buf, strs[1], patients, D_001602a4[illness]);
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, buf);
    if (cured) {
        if (count)
            sprintf(buf, strs[2], D_001602a4[illness], count);
        else
            sprintf(buf, strs[3]);
    } else if (missing_room && missing_staff) {
        sprintf(buf, strs[4], D_001602cc[rooms[missing_room].id], D_00160318[missing_staff]);
    } else if (missing_room) {
        sprintf(buf, strs[5], D_001602cc[rooms[missing_room].id]);
    } else if (missing_staff) {
        sprintf(buf, strs[6], D_00160318[missing_staff]);
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
    if (hosp->f1e5 * 100 / hosp->f1e6 >= hosp->f1e8)
        bonus = hosp->f1dd * hosp->f1e5;
    else
        bonus = 0;
    total = hosp->f1dd * hosp->f1e6;
    text[0] = 0;
    x = 0;
    y = 0;
    sprintf(buf, strs[0], total, bonus);
    fn_000c93bf(text, buf);
    sprintf(buf, strs[1], hosp->f1e5, hosp->f1e6);
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, buf);
    LbTextDraw(x, y, text);
}

// FUNCTION: 0x000afcd0
void process_emergency_end_buttons(u8 button)
{
    switch (button) {
    case 0:
        D_00158192 = 0x5c;
        break;
    }
}

// FUNCTION: 0x000afd0c
u16 summon_vehicle(u16 owner, u8 kind, u16 cell, u16 anim, u16 shadow_anim)
{
    Thing *vehicle;

    vehicle = allocate_thing(0x19);
    if (vehicle) {
        vehicle->renderer = 0x10;
        vehicle->subtype = kind;
        vehicle->owner = owner;
        vehicle->x.tile = cell % 128;
        vehicle->y.tile = cell / 128;
        vehicle->x.frac = 0x80;
        vehicle->y.frac = 0x80;
        vehicle->anim_speed = 2;
        vehicle->unknown17 = 1;
        insert_node(vehicle, vehicle->x, vehicle->y);
        vehicle->unknown18 = 8;
        vehicle->f37 = 0x190;
        vehicle->sprite = start[anim].frame;
        vehicle->palette = start[anim].flags;
        vehicle->kind = 2;
        vehicle->f39 = create_shadow(cell, vehicle - thing, owner, shadow_anim);
        if (owner == player) {
            if (anim == 0x792)
                play_ambient(saferand() % 5 + 0xc7);
            else if (myhosp->f1e4 >= 0 && myhosp->f1e4 < 0x24)
                play_ambient(myhosp->f1e4 + 0x1f);
        }
        return vehicle - thing;
    }
    return 0;
}

// FUNCTION: 0x000afef4
void helicopter_lands(Thing *heli)
{
    heli->sprite = frame[heli->sprite].next;
    if (heli->f37 > 0) {
        heli->f37 -= heli->unknown18;
    } else {
        switch (heli->subtype) {
        case 0:
            heli->f37 = 0;
            heli->kind = 4;
            allhosps[heli->owner].f1e1 = 1;
            if (heli->owner == player)
                advisor.f127 = 1;
            break;
        case 1:
            heli->sprite = start[0x7a8].frame;
            heli->palette = start[0x7a8].flags;
            heli->f37 = 0;
            heli->kind = 8;
            break;
        }
    }
}

// FUNCTION: 0x000b0000
void helicopter_leaves(Thing *heli)
{
    AllHosps *hosp;

    hosp = &allhosps[heli->owner];
    heli->sprite = frame[heli->sprite].next;
    if (heli->f37 < 0x190) {
        heli->f37 += heli->unknown18;
    } else {
        switch (heli->subtype) {
        case 0:
            hosp->f1e1 = 0;
            hosp->f1df = 0;
            break;
        case 1:
            gamevar.ff5 = 0;
            gamevar.ff7 = 0;
            break;
        }
        if (heli->f39)
            remove_thing(&thing[(u16)heli->f39]);
        remove_thing(heli);
    }
}
