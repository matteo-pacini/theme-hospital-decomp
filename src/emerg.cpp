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
extern char D_00149628[];
extern PlayerPopup D_0015f5c2[];
extern char **D_001602a4;
extern char **D_001602cc;
extern char **D_00160318;
void fn_000c93bf(char *dst, char *src);
extern u16 D_00155e40;
extern u16 D_00158190;
extern u8 D_00158192;
extern Event D_001549d0[];
extern EmergencyDef D_0015fc3c[];
}

u32 check_emergency_criteria(void);
void setup_next_emergency(void);
void setup_emergency_stats(u16 call, u8 owner);
u16 summon_vehicle(u16 owner, u8 a, u16 b, u16 c, u16 d);
u32 old_newrand(void);

extern char D_0014962a[];
#pragma aux D_0014962a "W?$Wsi0dnew_line$n[]a$:.0$:?emergency_over$n(ucpnpna)vnx[]a"

u16 saferand(void);
u16 count_emergency_staff(u8 owner);
u16 create_shadow(u16 tile, u16 height, u8 owner, u16 anim);

// FUNCTION: 0x000aeec0
void try_to_generate_emergency(void)
{
    s32 found;
    s32 i;
    s32 j;
    AllHosps *h;
    s32 n;

    found = 0;
    n = 1;
    if (statvar.unknownfd > 1)
        n = 4;
    for (i = 0; i < n; i++) {
        if (statvar.unknownfd <= 1 || D_001609c4[i].active != 0 || 0) {
            h = &allhosps[i];
            if (h->f1e2s == 5) {
                h->f1df = summon_vehicle(i, 0, allhosps[i].f1d7 + 0x80, 0x4cc, 0x7a7);
                if (h->f1df) {
                    h->f1e2 = 3;
                    h->f1e1 = 0;
                } else {
                    h->f1e3 = 0;
                    h->f1e2 = 0;
                }
            }
        }
    }
    if (check_emergency_criteria()) {
        for (j = 0; j < n; j++) {
            if (statvar.unknownfd <= 1 || D_001609c4[j].active != 0 || 0) {
                if (allhosps[j].rooms[gamevar.f124].unknown1D == 1)
                    found = 1;
            }
        }
        if (found) {
            for (j = 0; j < n; j++) {
                if (statvar.unknownfd <= 1 || D_001609c4[j].active != 0 || 0)
                    setup_emergency_stats(make_a_playercall(2, 0, 0x14, j), j);
            }
        }
        setup_next_emergency();
    }
    for (i = 0; i < n; i++) {
        if (statvar.unknownfd <= 1 || D_001609c4[i].active != 0 || 0) {
            h = &allhosps[i];
            if (h->f1e2s > 5)
                h->f1e2--;
        }
    }
}

// FUNCTION: 0x000af118
u32 check_emergency_criteria(void)
{
    u8 i;
    s32 n;

    n = 1;
    if (statvar.unknownfd > 1)
        n = 4;
    if (statvar.unknownfd <= 1 && display_mode == 9 && D_00158190 == 5)
        return 0;
    if (actual_game_turn >= gamevar.f11d && gamevar.f123) {
        for (i = 0; i < n; i++) {
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
        D_00155e40 = 0;
    }
}

// FUNCTION: 0x000af510
void give_emergency_stats(u16 call, u8 owner)
{
    AllHosps *h;
    u32 i;

    h = &allhosps[owner];
    for (i = 0; i < 4; i++) {
        if (gamevar.ffd[i].call == call && gamevar.ffd[i].used != 0)
            break;
    }
    h->f1e4 = gamevar.ffd[i].illness;
    h->f1e6 = gamevar.ffd[i].count;
    h->f1e3 = h->f1e6;
    h->f1dd = gamevar.ffd[i].f2;
    h->f1e8 = gamevar.ffd[i].f7;
    gamevar.ffd[i].used = 0;
    gamevar.f13e--;
}

// FUNCTION: 0x000af600
u32 announce_emergency(u8 owner, char **strs)
{
    s32 width;
    s32 room;
    s32 staff;
    s32 cured;
    s32 count;
    u16 sep;
    s32 v;
    AllHosps *h;
    u16 call;
    s32 i;
    s32 staff_count;
    s32 patients;
    s32 bonus;
    s32 illness;
    Expertise *exp;
    char name[200];
    char text[1000];
    u16 a;
    u16 b;
    char buf[200];

    width = 400;
    room = 0;
    staff = 0;
    cured = 0;
    count = 0;
    sep = *(u16 *)D_00149628;
    v = lbFontPtr ? lbFontPtr[34].height : 0;
    h = &allhosps[owner];
    if (h->f1e2s != 0)
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
            count = h->rooms[illness].unknown1B;
    } else {
        if (listhead[owner].rooms[exp->category] == 0 && room_available(exp->category))
            room = exp->category;
        for (i = 0; i < 9; i++) {
            if ((exp->unknown0 & (1 << i)) && listhead[owner].staff[i] == 0) {
                staff = i;
                break;
            }
        }
    }
    fn_000bbff0(name, strs[saferand() % 4 + 9]);
    text[0] = 0;
    a = 0;
    b = 0;
    sprintf(buf, strs[0], name);
    fn_000c93bf(text, buf);
    sprintf(buf, strs[1], patients, D_001602a4[illness]);
    fn_000c93bf(text, (char *)&sep);
    fn_000c93bf(text, buf);
    if (cured) {
        if (count)
            sprintf(buf, strs[2], D_001602a4[illness], count);
        else
            sprintf(buf, strs[3]);
    } else if (room && staff) {
        sprintf(buf, strs[4], D_001602cc[rooms[room].id], D_00160318[staff]);
    } else if (room) {
        sprintf(buf, strs[5], D_001602cc[rooms[room].id]);
    } else if (staff) {
        sprintf(buf, strs[6], D_00160318[staff]);
    } else {
        sprintf(buf, strs[7]);
    }
    fn_000c93bf(text, (char *)&sep);
    fn_000c93bf(text, buf);
    sprintf(buf, strs[8], bonus);
    fn_000c93bf(text, (char *)&sep);
    fn_000c93bf(text, buf);
    LbTextDraw(a, b, text);
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
    u16 sep;
    s32 v;
    AllHosps *h;
    s32 bonus;
    s32 total;
    char text[2000];
    u16 a;
    u16 b;
    char buf[200];

    sep = *(u16 *)D_0014962a;
    v = lbFontPtr ? lbFontPtr[34].height : 0;
    h = &allhosps[owner];
    if (h->f1e5 * 100 / h->f1e6 >= h->f1e8)
        bonus = h->f1dd * h->f1e5;
    else
        bonus = 0;
    total = h->f1dd * h->f1e6;
    text[0] = 0;
    a = 0;
    b = 0;
    sprintf(buf, strs[0], total, bonus);
    fn_000c93bf(text, buf);
    sprintf(buf, strs[1], h->f1e5, h->f1e6);
    fn_000c93bf(text, (char *)&sep);
    fn_000c93bf(text, buf);
    LbTextDraw(a, b, text);
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
u16 summon_vehicle(u16 owner, u8 kind, u16 cell, u16 anim, u16 e)
{
    Thing *w;

    w = allocate_thing(0x19);
    if (w) {
        w->renderer = 0x10;
        w->subtype = kind;
        w->owner = owner;
        w->x.tile = cell % 128;
        w->y.tile = cell / 128;
        w->x.frac = 0x80;
        w->y.frac = 0x80;
        w->anim_speed = 2;
        w->unknown17 = 1;
        insert_node(w, w->x, w->y);
        w->unknown18 = 8;
        w->f37 = 0x190;
        w->sprite = start[anim].frame;
        w->palette = start[anim].flags;
        w->kind = 2;
        w->f39 = create_shadow(cell, w - thing, owner, e);
        if (owner == player) {
            if (anim == 0x792)
                play_ambient(saferand() % 5 + 0xc7);
            else if (myhosp->f1e4 >= 0 && myhosp->f1e4 < 0x24)
                play_ambient(myhosp->f1e4 + 0x1f);
        }
        return w - thing;
    }
    return 0;
}

// FUNCTION: 0x000afef4
void helicopter_lands(Thing *w)
{
    w->sprite = frame[w->sprite].next;
    if (w->f37 > 0) {
        w->f37 -= w->unknown18;
    } else {
        switch (w->subtype) {
        case 0:
            w->f37 = 0;
            w->kind = 4;
            allhosps[w->owner].f1e1 = 1;
            if (w->owner == player)
                advisor.f127 = 1;
            break;
        case 1:
            w->sprite = start[0x7a8].frame;
            w->palette = start[0x7a8].flags;
            w->f37 = 0;
            w->kind = 8;
            break;
        }
    }
}

// FUNCTION: 0x000b0000
void helicopter_leaves(Thing *w)
{
    AllHosps *h;

    h = &allhosps[w->owner];
    w->sprite = frame[w->sprite].next;
    if (w->f37 < 0x190) {
        w->f37 += w->unknown18;
    } else {
        switch (w->subtype) {
        case 0:
            h->f1e1 = 0;
            h->f1df = 0;
            break;
        case 1:
            gamevar.ff5 = 0;
            gamevar.ff7 = 0;
            break;
        }
        if (w->f39)
            remove_thing(&thing[(u16)w->f39]);
        remove_thing(w);
    }
}
