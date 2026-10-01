#include "hosp.h"

extern "C" {
extern u8 D_00156910;
extern u8 D_00158192;
void fn_000c93bf(char *dst, char *src);
}


u16 create_mayor(u8 owner);
u8 mayor_judges_hospital(u8 owner);

// Draws the VIP visit request's text: strs[0] formatted with VIP idx's name
// (mayor_names).
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

// VIP request buttons: 0 accepts the visit (choice 0x14), 1 refuses it
// (0x15). After more than five refusals in a row, while othervar.rand_pct < 10, a
// refusal is followed by an acceptance.
// FUNCTION: 0x000a0c08
void process_mayor_start_buttons(u8 button)
{
    switch (button) {
    case 1:
        packet_choice_result(CH_VIP_REFUSE);
        if (++allhosps[player].mayor_postponed <= 5 || othervar.rand_pct >= 10)
            break;
    case 0:
        packet_choice_result(CH_VIP_ACCEPT);
        allhosps[player].mayor_postponed = 0;
        break;
    }
}

// Daily VIP check: once a hospital with a reception desk has waited gbv.mayor_launch
// days, asks it to accept a random VIP (allhosps.mayor_state = 1). An accepted
// visit (f1fa 2) starts at its time with create_mayor (f1fa 3).
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
    if (statvar.num_players > 1)
        count = 4;
    for (i = 0; i < count; i++) {
        if (statvar.num_players > 1 && D_001609c4[i].active == 0)
            continue;
        if (allhosps[i].mayor_state != 0) {
            found = 1;
            break;
        }
    }
    if (!found) {
        for (i = 0; i < count; i++) {
            if (statvar.num_players > 1 && D_001609c4[i].active == 0)
                continue;
            if (&thing[listhead[i].objects[OBJ_RECEPTION_DESK]] > thing) {
                hosp = &allhosps[i];
                hosp->mayor_launch_count++;
                if ((s16)hosp->mayor_launch_count >= gbv.mayor_launch) {
                    u8 mayor_idx;
                    mayor_idx = old_newrand() % 11;
                    hosp->mayor_name = mayor_idx;
                    hosp->mayor_state = 1;
                    hosp->mayor_launch_count = 0;
                    made = 1;
                    make_a_playercall(6, (u16)mayor_idx, 0x14, (u8)i);
                }
            }
        }
    } else {
        for (i = 0; i < count; i++) {
            if (statvar.num_players > 1 && D_001609c4[i].active == 0)
                continue;
            hosp = &allhosps[i];
            switch (hosp->mayor_state) {
            case 2:
                if (actual_game_turn > hosp->mayor_arrival_turn) {
                    hosp->mayor = create_mayor((u8)i);
                    if (hosp->mayor) {}
                    hosp->mayor_state = 3;
                    if (i == player) {
                        advisor.forced_advice_id = 8;
                        if (hosp->mayor_name >= 0 && hosp->mayor_name < 6)
                            play_ambient(hosp->mayor_name + 0xb7);
                    }
                }
                break;
            }
        }
    }
}

// Creates the visiting VIP for player `owner` at one of the hospital's four
// entry points (unknown1cf), walking to the reception desk, or straight back
// out with verdict 14 if there is none. Returns the thing index, 0 if no
// thing was free.
// FUNCTION: 0x000a0f6c
u16 create_mayor(u8 owner)
{
    Thing *mayor;
    u16 pos;
    u16 anim;

    mayor = allocate_thing(0x19);
    if (mayor != 0) {
    Thing *reception_desk;
    mayor->type = THING_MAYOR;
    mayor->owner = owner;
    pos = room_info[allhosps[owner].entrances[old_newrand() & 3]].door_tile;
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
    reception_desk = &thing[listhead[owner].objects[OBJ_RECEPTION_DESK]];
    if (reception_desk > thing) {
        set_destination_coords(mayor, reception_desk->use_pos[1]);
        mayor->state = MAYOR_GOTO;
        mayor->goto_state = MAYOR_PAUSES;
        mayor->inspect_room = 0;
    } else {
        set_destination_coords(mayor, room_info[allhosps[mayor->owner].entrances[old_newrand() & 3]].door_tile);
        mayor->state = MAYOR_GOTO;
        mayor->goto_state = MAYOR_GONE;
        allhosps[mayor->owner].mayor_verdict = 0xe;
    }
    return mayor - thing;
    }
    return 0;
}

// VIP waiting state: when the timer runs out, walks to the next room of
// interest (some at random), or, after the last room, leaves and applies the
// verdict of mayor_judges_hospital (cash award, reputation).
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
        if (room->owner == mayor->owner && rooms[room->type].main_object) {
            if (rooms[room->type].door_kind == 0 || !(old_newrand() & 1)) {
                set_destination_coords(mayor, room->door_tile);
                mayor->state = MAYOR_GOTO;
                mayor->goto_state = MAYOR_PAUSES;
                return;
            }
        }
    }
    hosp = &allhosps[mayor->owner];
    set_destination_coords(mayor, room_info[hosp->entrances[old_newrand() & 3]].door_tile);
    mayor->state = MAYOR_GOTO;
    mayor->goto_state = MAYOR_GONE;
    hosp->mayor_verdict = mayor_judges_hospital(mayor->owner);
    value = mayor_result[hosp->mayor_verdict].cash_bonus;
    if (value != 0)
        change_balance(TX_VIP_AWARD, value, 0, (u16)mayor->owner);
    value = mayor_result[hosp->mayor_verdict].reputation_event;
    if (value != 0)
        reputation_affected(0, value, 0, mayor->owner, 0);
    if (value != REP_VIP_BAD)
        satisfied_net_criteria(2, mayor->owner);
}

// Scores player `owner`'s hospital for a VIP visit (staff tiredness, room
// warmth, doctor grades, rooms, a random part) and returns the verdict, a
// mayor_result index 0-14.
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
    int warmth_sum;
    Thing *person;
    AllHosps *hosp;
    int avg;
    RoomInfo *room;

    score = 0;
    doctors = 0;
    juniors = 0;
    consultants = 0;
    nrooms = 0;
    if (statvar.num_players > 1) {
        nstaff = 0;
        fatigue_sum = 0;
        max_fatigue = 0;
        npatients = 0;
        warmth_sum = 0;
        for (person = thing + 1; person < end_things; person++) {
            if (person->owner != owner)
                continue;
            switch (person->type) {
            case THING_STAFF:
                fatigue_sum += person->tiredness;
                person->tiredness > max_fatigue ? max_fatigue = person->tiredness : 0;
                nstaff++;
                break;
            case THING_PATIENT:
                warmth_sum += (s16)person->warmth;
                npatients++;
                break;
            }
        }
    } else {
        fatigue_sum = adcounts.staff_tiredness_sum;
        max_fatigue = adcounts.max_staff_tiredness;
        npatients = adcounts.patients;
        warmth_sum = adcounts.warmth_sum;
    }
    hosp = &allhosps[owner];
    if (hosp->litter_count > 10)
        ++score;
    else if (hosp->litter_count > 20)
        score += 2;
    else
        --score;
    if (hosp->staff_count > 2) {
        avg = fatigue_sum / hosp->staff_count;
        if (gbv.very_tired < max_fatigue)
            score += 2;
        else if (gbv.tired < avg)
            ++score;
        else
            --score;
    } else {
        score += 4;
    }
    if (npatients > 10) {
        avg = warmth_sum / npatients;
        if (avg > 0x50 || avg < 0x14)
            ++score;
        else
            --score;
    }
    for (person = thing + 1; person < end_things; person++) {
        if ((person->type == THING_STAFF && person->subtype == STAFF_DOCTOR) && person->owner == owner) {
            doctors++;
            if (person->skills & SKILL_JUNIOR)
                juniors++;
            else if (person->skills & SKILL_CONSULTANT)
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
        if ((room->type != ROOM_FREE && room->owner == owner) && room->type != ROOM_ENTRANCE)
            nrooms++;
    }
    if (nrooms == 0)
        score += 4;
    else if (nrooms < 3)
        ++score;
    score += 2 - old_normal_rand(0, 5);
    if (owner == player) {
        award_counts.mayor_score_total += score;
        award_counts.mayor_visits++;
    }
    {
        int i;

        for (i = 0; i < 15; i++) {
            if (mayor_result[i].max_score >= score)
                return i;
        }
    }
    return 14;
}

// A VIP who has left the map: removes the thing and, for the mayor, marks
// the visit over and queues the report call.
// FUNCTION: 0x000a1754
void mayor_gone(Thing *mayor)
{
    if (mayor->type == THING_MAYOR) {
        allhosps[mayor->owner].mayor_state = 4;
        make_a_playercall(5, 2, 0, mayor->owner);
    } else {
        allhosps[mayor->owner].inspection[1] = 0;
    }
    remove_thing(mayor);
}

// Draws the VIP report: strs[0-4] and the verdict text strs[f1fb + 5], with
// the cash award and reputation change when there are any.
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
    sprintf(buf, strs[0], allhosps[othervar.cur_player].name);
    fn_000c93bf(text, buf);
    sprintf(buf, strs[1], mayor_names[allhosps[owner].mayor_name]);
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, buf);
    sprintf(buf, "%s", strs[allhosps[owner].mayor_verdict + 5]);
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, buf);
    fn_000c93bf(text, new_line);
    value = mayor_result[allhosps[owner].mayor_verdict].cash_bonus;
    if (value != 0) {
        sprintf(buf, strs[2], value);
        fn_000c93bf(text, new_line);
        fn_000c93bf(text, buf);
    }
    value = mayor_result[allhosps[owner].mayor_verdict].reputation_event;
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

// VIP report button 0 closes the report (D_00158192 = 0x5c).
// FUNCTION: 0x000a1ab4
void process_mayor_end_buttons(u8 button)
{
    switch (button) {
    case 0:
        D_00158192 = 0x5c;
        break;
    }
}

// A VIP arriving at a stop: faces the reception desk (first stop) or the
// room's door and waits 50 turns.
// FUNCTION: 0x000a1af0
void mayor_inspects(Thing *mayor)
{
    Thing *door;
    Thing *reception_desk;

    if (mayor->inspect_room == 0) {
        reception_desk = &thing[listhead[mayor->owner].objects[OBJ_RECEPTION_DESK]];
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
