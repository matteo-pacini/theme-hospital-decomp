#include "hosp.h"

extern "C" {
extern u8 D_00156910;
extern u8 D_00158192;
void fn_000c93bf(char *dst, char *src);
}


u16 create_mayor(u8 owner);
u8 mayor_judges_hospital(u8 owner);

// FUNCTION: 0x000a0b70
void mayor_requester(u8 idx, char **strs)
{
    int line_height;
    u16 text_x;
    u16 text_y;
    char buf[200];

    line_height = lbFontPtr ? lbFontPtr[34].height : 0;
    text_x = 0;
    text_y = line_height * 2;
    sprintf(buf, strs[0], mayor_names[idx]);
    LbTextDraw(text_x, text_y, buf);
    D_00156910 = 1;
}

// FUNCTION: 0x000a0c08
void process_mayor_start_buttons(u8 button)
{
    switch (button) {
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
    AllHosps *hosp;

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
                hosp = &allhosps[i];
                hosp->f1ff++;
                if ((s16)hosp->f1ff >= gbv.f128) {
                    u8 mayor_idx;
                    mayor_idx = old_newrand() % 11;
                    hosp->f1f9 = mayor_idx;
                    hosp->f1fa = 1;
                    hosp->f1ff = 0;
                    made = 1;
                    make_a_playercall(6, (u16)mayor_idx, 0x14, (u8)i);
                }
            }
        }
    } else {
        for (i = 0; i < count; i++) {
            if (statvar.unknownfd > 1 && D_001609c4[i].active == 0)
                continue;
            hosp = &allhosps[i];
            switch (hosp->f1fa) {
            case 2:
                if (actual_game_turn > hosp->f1f3) {
                    hosp->f1f7 = create_mayor((u8)i);
                    if (hosp->f1f7) {}
                    hosp->f1fa = 3;
                    if (i == player) {
                        advisor.f2 = 8;
                        if (hosp->f1f9 >= 0 && hosp->f1f9 < 6)
                            play_ambient(hosp->f1f9 + 0xb7);
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
    Thing *mayor;
    u16 pos;
    u16 anim;

    mayor = allocate_thing(0x19);
    if (mayor != 0) {
    Thing *reception_desk;
    mayor->type = 0x14;
    mayor->owner = owner;
    pos = room_info[allhosps[owner].unknown1cf[old_newrand() & 3]].unknown4;
    mayor->x.tile = pos % 128;
    mayor->y.tile = pos / 128;
    mayor->x.frac = 0x80;
    mayor->y.frac = 0x80;
    mayor->speed = 0x10;
    mayor->anim_speed = 2;
    mayor->facing = 1;
    insert_node(mayor, mayor->x, mayor->y);
    anim = check_frame(0x89, mayor) + mayor->facing;
    mayor->sprite = start[anim].frame;
    mayor->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
    reception_desk = &thing[listhead[owner].objects[11]];
    if (reception_desk > thing) {
        fn_0004195c(mayor, reception_desk->use_pos[1]);
        mayor->state = 2;
        mayor->goto_state = 4;
        mayor->inspect_room = 0;
    } else {
        fn_0004195c(mayor, room_info[allhosps[mayor->owner].unknown1cf[old_newrand() & 3]].unknown4);
        mayor->state = 2;
        mayor->goto_state = 6;
        allhosps[mayor->owner].f1fb = 0xe;
    }
    return mayor - thing;
    }
    return 0;
}

// FUNCTION: 0x000a11b8
void mayor_pauses(Thing *mayor)
{
    RoomInfo *room;
    AllHosps *hosp;
    s32 value;

    if (--mayor->timer != 0)
        return;
    for (;;) {
        mayor->inspect_room++;
        room = &room_info[mayor->inspect_room];
        if (room >= end_room_info)
            break;
        if (room->owner == mayor->owner && rooms[room->type].fa) {
            if (rooms[room->type].unknown36 == 0 || !(old_newrand() & 1)) {
                fn_0004195c(mayor, room->unknown4);
                mayor->state = 2;
                mayor->goto_state = 4;
                return;
            }
        }
    }
    hosp = &allhosps[mayor->owner];
    fn_0004195c(mayor, room_info[hosp->unknown1cf[old_newrand() & 3]].unknown4);
    mayor->state = 2;
    mayor->goto_state = 6;
    hosp->f1fb = mayor_judges_hospital(mayor->owner);
    value = mayor_result[hosp->f1fb].f4;
    if (value != 0)
        change_balance(0x20, value, 0, (u16)mayor->owner);
    value = mayor_result[hosp->f1fb].f2;
    if (value != 0)
        reputation_affected(0, value, 0, mayor->owner, 0);
    if (value != 0x12)
        satisfied_net_criteria(2, mayor->owner);
}

// FUNCTION: 0x000a13a4
u8 mayor_judges_hospital(u8 owner)
{
    int score;
    int doctors;
    int juniors;
    int consultants;
    int nrooms;
    int nstaff;
    int fatigue_sum;
    int max_fatigue;
    int npatients;
    int sum2;
    Thing *person;
    AllHosps *hosp;
    int avg;
    RoomInfo *room;

    score = 0;
    doctors = 0;
    juniors = 0;
    consultants = 0;
    nrooms = 0;
    if (statvar.unknownfd > 1) {
        nstaff = 0;
        fatigue_sum = 0;
        max_fatigue = 0;
        npatients = 0;
        sum2 = 0;
        for (person = thing + 1; person < end_things; person++) {
            if (person->owner != owner)
                continue;
            switch (person->type) {
            case 4:
                fatigue_sum += person->tiredness;
                person->tiredness > max_fatigue ? max_fatigue = person->tiredness : 0;
                nstaff++;
                break;
            case 2:
                sum2 += (s16)person->warmth;
                npatients++;
                break;
            }
        }
    } else {
        fatigue_sum = adcounts.f2;
        max_fatigue = adcounts.f8;
        npatients = adcounts.f0;
        sum2 = adcounts.f14;
    }
    hosp = &allhosps[owner];
    if (hosp->fcaa > 10)
        ++score;
    else if (hosp->fcaa > 20)
        score += 2;
    else
        --score;
    if (hosp->fc9c > 2) {
        avg = fatigue_sum / hosp->fc9c;
        if (gbv.fea < max_fatigue)
            score += 2;
        else if (gbv.fe8 < avg)
            ++score;
        else
            --score;
    } else {
        score += 4;
    }
    if (npatients > 10) {
        avg = sum2 / npatients;
        if (avg > 0x50 || avg < 0x14)
            ++score;
        else
            --score;
    }
    for (person = thing + 1; person < end_things; person++) {
        if ((person->type == 4 && person->subtype == 1) && person->owner == owner) {
            doctors++;
            if (person->skills & 8)
                juniors++;
            else if (person->skills & 0x80)
                consultants++;
        }
    }
    if (doctors / 2 < juniors)
        score += 2;
    if (doctors / 2 < consultants)
        score += -2;
    if (doctors == 0)
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
void mayor_gone(Thing *mayor)
{
    if (mayor->type == 0x14) {
        allhosps[mayor->owner].f1fa = 4;
        make_a_playercall(5, 2, 0, mayor->owner);
    } else {
        allhosps[mayor->owner].unknown209[1] = 0;
    }
    remove_thing(mayor);
}

// FUNCTION: 0x000a17e8
void report_mayor_result(u8 owner, char **strs)
{
    int value;
    char buf[200];
    char text[2000];
    u16 text_x;
    u16 text_y;
    int line_height;
    char new_line[] = "\n";

    line_height = lbFontPtr ? lbFontPtr[34].height : 0;
    text[0] = 0;
    text_x = 0;
    text_y = 0;
    sprintf(buf, strs[0], allhosps[othervar.ff].unknown0);
    fn_000c93bf(text, buf);
    sprintf(buf, strs[1], mayor_names[allhosps[owner].f1f9]);
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, buf);
    sprintf(buf, "%s", strs[allhosps[owner].f1fb + 5]);
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, buf);
    fn_000c93bf(text, new_line);
    value = mayor_result[allhosps[owner].f1fb].f4;
    if (value != 0) {
        sprintf(buf, strs[2], value);
        fn_000c93bf(text, new_line);
        fn_000c93bf(text, buf);
    }
    value = mayor_result[allhosps[owner].f1fb].f2;
    if (value == 0x10) {
        sprintf(buf, strs[3], value);
        fn_000c93bf(text, new_line);
        fn_000c93bf(text, buf);
    } else if (value == 0x12) {
        sprintf(buf, strs[4], value);
        fn_000c93bf(text, new_line);
        fn_000c93bf(text, buf);
    }
    LbTextDraw(text_x, text_y, text);
    D_00156910 = 1;
}

// FUNCTION: 0x000a1ab4
void process_mayor_end_buttons(u8 button)
{
    switch (button) {
    case 0:
        D_00158192 = 0x5c;
        break;
    }
}

// FUNCTION: 0x000a1af0
void mayor_inspects(Thing *mayor)
{
    Thing *door;
    Thing *reception_desk;

    if (mayor->inspect_room == 0) {
        reception_desk = &thing[listhead[mayor->owner].objects[11]];
        if (reception_desk > thing)
            mayor->facing = opposite_angle[reception_desk->facing];
    } else {
        door = find_door(mayor);
        if (door > thing)
            mayor->facing = opposite_angle[door->facing];
    }
    {
        u16 anim = check_frame(0x89, mayor) + mayor->facing;
        mayor->sprite = start[anim].frame;
        mayor->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
    }
    mayor->timer = 0x32;
}
