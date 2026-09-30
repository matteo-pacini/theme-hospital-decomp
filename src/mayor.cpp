#include "hosp.h"

extern "C" {
extern u8 D_00156910;
extern u8 D_00158192;
void fn_000c93bf(char *dst, char *src);
}

extern char D_00145330[];
#pragma aux D_00145330 "W?$Wsi0dnew_line$n[]a$:.0$:?report_mayor_result$n(ucpnpna)vnx[]a"

u16 create_mayor(u8 owner);
u8 mayor_judges_hospital(u8 owner);

// FUNCTION: 0x000a0b70
void mayor_requester(u8 idx, char **strs)
{
    int v;
    u16 a;
    u16 b;
    char buf[200];

    v = lbFontPtr ? lbFontPtr[34].height : 0;
    a = 0;
    b = v * 2;
    sprintf(buf, strs[0], mayor_names[idx]);
    LbTextDraw(a, b, buf);
    D_00156910 = 1;
}

// FUNCTION: 0x000a0c08
void process_mayor_start_buttons(u8 n)
{
    switch (n) {
    case 1:
        packet_choice_result(0x15);
        if (++allhosps[player].f1fd <= 5 || othervar.f0 >= 10)
            break;
    case 0:
        packet_choice_result(0x14);
        allhosps[player].f1fd = 0;
        break;
    }
}

// FUNCTION: 0x000a0cac
void generate_mayors(void)
{
    u8 found;
    u8 made;
    s16 count;
    s16 i;
    AllHosps *h;

    found = 0;
    made = 0;
    count = 1;
    if (statvar.unknownfd > 1)
        count = 4;
    for (i = 0; i < count; i++) {
        if (statvar.unknownfd > 1 && D_001609c4[i].active == 0)
            continue;
        if (allhosps[i].f1fa != 0) {
            found = 1;
            break;
        }
    }
    if (!found) {
        for (i = 0; i < count; i++) {
            if (statvar.unknownfd > 1 && D_001609c4[i].active == 0)
                continue;
            if (&thing[listhead[i].objects[11]] > thing) {
                h = &allhosps[i];
                h->f1ff++;
                if ((s16)h->f1ff >= gbv.f128) {
                    u8 r;
                    r = old_newrand() % 11;
                    h->f1f9 = r;
                    h->f1fa = 1;
                    h->f1ff = 0;
                    made = 1;
                    make_a_playercall(6, (u16)r, 0x14, (u8)i);
                }
            }
        }
    } else {
        for (i = 0; i < count; i++) {
            if (statvar.unknownfd > 1 && D_001609c4[i].active == 0)
                continue;
            h = &allhosps[i];
            switch (h->f1fa) {
            case 2:
                if (actual_game_turn > h->f1f3) {
                    h->f1f7 = create_mayor((u8)i);
                    if (h->f1f7) {}
                    h->f1fa = 3;
                    if (i == player) {
                        advisor.f2 = 8;
                        if (h->f1f9 >= 0 && h->f1f9 < 6)
                            play_ambient(h->f1f9 + 0xb7);
                    }
                }
                break;
            }
        }
    }
}

// FUNCTION: 0x000a0f6c
u16 create_mayor(u8 owner)
{
    Thing *w;
    u16 pos;
    u16 f;

    w = allocate_thing(0x19);
    if (w != 0) {
    Thing *obj;
    w->renderer = 0x14;
    w->owner = owner;
    pos = room_info[allhosps[owner].unknown1cf[old_newrand() & 3]].unknown4;
    w->x.tile = pos % 128;
    w->y.tile = pos / 128;
    w->x.frac = 0x80;
    w->y.frac = 0x80;
    w->unknown18 = 0x10;
    w->anim_speed = 2;
    w->unknown17 = 1;
    insert_node(w, w->x, w->y);
    f = check_frame(0x89, w) + w->unknown17;
    w->sprite = start[f].frame;
    w->palette = start[f].flags > 0 ? (u8)1 : (u8)0;
    obj = &thing[listhead[owner].objects[11]];
    if (obj > thing) {
        fn_0004195c(w, obj->use_pos[1]);
        w->kind = 2;
        w->f6e = 4;
        w->f6c = 0;
    } else {
        fn_0004195c(w, room_info[allhosps[w->owner].unknown1cf[old_newrand() & 3]].unknown4);
        w->kind = 2;
        w->f6e = 6;
        allhosps[w->owner].f1fb = 0xe;
    }
    return w - thing;
    }
    return 0;
}

// FUNCTION: 0x000a11b8
void mayor_pauses(Thing *w)
{
    RoomInfo *room;
    AllHosps *h;
    s32 value;

    if (--w->timer != 0)
        return;
    for (;;) {
        w->f6c++;
        room = &room_info[w->f6c];
        if (room >= end_room_info)
            break;
        if (room->owner == w->owner && rooms[room->type].fa) {
            if (rooms[room->type].unknown36 == 0 || !(old_newrand() & 1)) {
                fn_0004195c(w, room->unknown4);
                w->kind = 2;
                w->f6e = 4;
                return;
            }
        }
    }
    h = &allhosps[w->owner];
    fn_0004195c(w, room_info[h->unknown1cf[old_newrand() & 3]].unknown4);
    w->kind = 2;
    w->f6e = 6;
    h->f1fb = mayor_judges_hospital(w->owner);
    value = mayor_result[h->f1fb].f4;
    if (value != 0)
        change_balance(0x20, value, 0, (u16)w->owner);
    value = mayor_result[h->f1fb].f2;
    if (value != 0)
        reputation_affected(0, value, 0, w->owner, 0);
    if (value != 0x12)
        satisfied_net_criteria(2, w->owner);
}

// FUNCTION: 0x000a13a4
u8 mayor_judges_hospital(u8 owner)
{
    int score;
    int staff;
    int tired;
    int angry;
    int nrooms;
    int npatients;
    int sum;
    int max;
    int n;
    int sum2;
    Thing *t;
    AllHosps *h;
    int avg;
    RoomInfo *room;

    score = 0;
    staff = 0;
    tired = 0;
    angry = 0;
    nrooms = 0;
    if (statvar.unknownfd > 1) {
        npatients = 0;
        sum = 0;
        max = 0;
        n = 0;
        sum2 = 0;
        for (t = thing + 1; t < end_things; t++) {
            if (t->owner != owner)
                continue;
            switch (t->renderer) {
            case 4:
                sum += t->f78;
                t->f78 > max ? max = t->f78 : 0;
                npatients++;
                break;
            case 2:
                sum2 += (s16)t->unknown83w;
                n++;
                break;
            }
        }
    } else {
        sum = adcounts.f2;
        max = adcounts.f8;
        n = adcounts.f0;
        sum2 = adcounts.f14;
    }
    h = &allhosps[owner];
    if (h->fcaa > 10)
        ++score;
    else if (h->fcaa > 20)
        score += 2;
    else
        --score;
    if (h->fc9c > 2) {
        avg = sum / h->fc9c;
        if (gbv.fea < max)
            score += 2;
        else if (gbv.fe8 < avg)
            ++score;
        else
            --score;
    } else {
        score += 4;
    }
    if (n > 10) {
        avg = sum2 / n;
        if (avg > 0x50 || avg < 0x14)
            ++score;
        else
            --score;
    }
    for (t = thing + 1; t < end_things; t++) {
        if ((t->renderer == 4 && t->subtype == 1) && t->owner == owner) {
            staff++;
            if (t->unknown124 & 8)
                tired++;
            else if (t->unknown124 & 0x80)
                angry++;
        }
    }
    if (staff / 2 < tired)
        score += 2;
    if (staff / 2 < angry)
        score += -2;
    if (staff == 0)
        score += 4;
    for (room = room_info + 1; room < end_room_info; room++) {
        if ((room->type != 0 && room->owner == owner) && room->type != 1)
            nrooms++;
    }
    if (nrooms == 0)
        score += 4;
    else if (nrooms < 3)
        ++score;
    score += 2 - old_normal_rand(0, 5);
    if (owner == player) {
        award_counts.f13a += score;
        award_counts.f13e++;
    }
    {
        int i;

        for (i = 0; i < 15; i++) {
            if (mayor_result[i].f0 >= score)
                return i;
        }
    }
    return 14;
}

// FUNCTION: 0x000a1754
void mayor_gone(Thing *w)
{
    if (w->renderer == 0x14) {
        allhosps[w->owner].f1fa = 4;
        make_a_playercall(5, 2, 0, w->owner);
    } else {
        allhosps[w->owner].unknown209[1] = 0;
    }
    remove_thing(w);
}

// FUNCTION: 0x000a17e8
void report_mayor_result(u8 owner, char **strs)
{
    int value;
    char buf[200];
    char text[2000];
    u16 a;
    u16 b;
    int v;
    u16 sep;

    sep = *(u16 *)D_00145330;
    v = lbFontPtr ? lbFontPtr[34].height : 0;
    text[0] = 0;
    a = 0;
    b = 0;
    sprintf(buf, strs[0], allhosps[othervar.ff].unknown0);
    fn_000c93bf(text, buf);
    sprintf(buf, strs[1], mayor_names[allhosps[owner].f1f9]);
    fn_000c93bf(text, (char *)&sep);
    fn_000c93bf(text, buf);
    sprintf(buf, "%s", strs[allhosps[owner].f1fb + 5]);
    fn_000c93bf(text, (char *)&sep);
    fn_000c93bf(text, (char *)&sep);
    fn_000c93bf(text, buf);
    fn_000c93bf(text, (char *)&sep);
    value = mayor_result[allhosps[owner].f1fb].f4;
    if (value != 0) {
        sprintf(buf, strs[2], value);
        fn_000c93bf(text, (char *)&sep);
        fn_000c93bf(text, buf);
    }
    value = mayor_result[allhosps[owner].f1fb].f2;
    if (value == 0x10) {
        sprintf(buf, strs[3], value);
        fn_000c93bf(text, (char *)&sep);
        fn_000c93bf(text, buf);
    } else if (value == 0x12) {
        sprintf(buf, strs[4], value);
        fn_000c93bf(text, (char *)&sep);
        fn_000c93bf(text, buf);
    }
    LbTextDraw(a, b, text);
    D_00156910 = 1;
}

// FUNCTION: 0x000a1ab4
void process_mayor_end_buttons(u8 n)
{
    switch (n) {
    case 0:
        D_00158192 = 0x5c;
        break;
    }
}

// FUNCTION: 0x000a1af0
void mayor_inspects(Thing *w)
{
    Thing *door;
    Thing *obj;

    if (w->f6c == 0) {
        obj = &thing[listhead[w->owner].objects[11]];
        if (obj > thing)
            w->unknown17 = opposite_angle[obj->unknown17];
    } else {
        door = find_door(w);
        if (door > thing)
            w->unknown17 = opposite_angle[door->unknown17];
    }
    {
        u16 f = check_frame(0x89, w) + w->unknown17;
        w->sprite = start[f].frame;
        w->palette = start[f].flags > 0 ? (u8)1 : (u8)0;
    }
    w->timer = 0x32;
}
