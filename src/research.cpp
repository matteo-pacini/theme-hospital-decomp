#include "hosp.h"

extern "C" {


}

void staff_exit_room_check(Thing *w);
void item_researched(u16 item, u16 illness, u8 owner);
void drug_improved(u16 illness, u8 owner);
void drug_researched(u16 illness, u8 owner);
void item_upgraded(u16 item, u8 owner);
u16 emptiest_research_room(u8 owner);
u16 find_rand_research_room(u8 owner, u16 exclude);
u8 find_best_drug_rating(u8 illness, u8 owner);
s32 find_new_cure_cost(u8 illness, u8 owner);
s32 find_new_diag_cost(u8 illness, u8 owner);
s32 find_new_drug_cost(u8 illness, u8 owner);
s32 find_drug_improve_cost(u8 illness, u8 owner);
s32 find_hw_improve_cost(u8 illness, u8 owner);
u8 choose_next_research_illness(u8 owner, s32 (*cost)(u8, u8));
void research_category_exhausted(const u8 owner, const u8 category);
void do_illness_research(u8 owner, u8 illness, u32 amount, u8 category);

// FUNCTION: 0x0006cf80
void do_research(void)
{
    AllHosps *hosp;
    s32 amount;
    u16 category;
    u16 unused = 0;
    u8 illness;
    u8 owner = 0;
    u8 exhausted;

    for (owner = 0; owner < 4; owner++) {
        hosp = &allhosps[owner];
        for (category = 0; category < 5; category++) {
            amount = hosp->research_alloc[category] * hosp->research_points / 100;
            if (hosp->research_alloc[category] > 0 && amount > 0) {
                hosp->research_points -= amount;
                award_counts.f149 += amount;
                exhausted = 0;
                switch (category) {
                case 0:
                    illness = choose_next_research_illness(owner, find_new_cure_cost);
                    if (illness) {
                        do_illness_research(owner, illness, amount, 0);
                    } else {
                        research_category_exhausted(owner, 0);
                        exhausted = 1;
                    }
                    break;
                case 1:
                    illness = choose_next_research_illness(owner, find_new_diag_cost);
                    if (illness) {
                        do_illness_research(owner, illness, amount, 1);
                    } else {
                        research_category_exhausted(owner, 1);
                        exhausted = 1;
                    }
                    break;
                case 2:
                    illness = choose_next_research_illness(owner, find_new_drug_cost);
                    if (illness) {
                        do_illness_research(owner, illness, amount, 2);
                    } else {
                        illness = choose_next_research_illness(owner, find_drug_improve_cost);
                        if (illness)
                            do_illness_research(owner, illness, amount, 2);
                        else
                            exhausted = 1;
                    }
                    break;
                case 3:
                    illness = choose_next_research_illness(owner, find_hw_improve_cost);
                    if (illness)
                        do_illness_research(owner, illness, amount, 3);
                    else
                        exhausted = 1;
                    break;
                case 4:
                    if ((statvar.unknownfd > 1 ? allhosps[owner].unknown1256 : gamevar.f136) != 0)
                        do_illness_research(owner, statvar.unknownfd > 1 ? allhosps[owner].unknown1256 : gamevar.f136, amount, 4);
                    break;
                }
                if (exhausted) {
                    hosp->research_points += amount;
                    award_counts.f149 -= amount;
                }
            }
        }
    }
}

// FUNCTION: 0x0006d288
void item_researched(u16 item, u16 illness, u8 owner)
{
    u8 pay;
    u8 i;
    u16 date;

    pay = 1;
    if (statvar.unknownfd > 1) {
        date = mycontrol->f648 * 12 + mycontrol->unknown650;
        for (i = 0; i < 4; i++) {
            if (allhosps[i].equipment[item].unknown0 == 1 && objects[item].unknown1c == 1)
                pay = 0;
            if (owner != i) {
                if (allhosps[i].object_stats[item].available == 0)
                    allhosps[i].object_stats[item].available = date + 3;
            }
        }
        if (pay) {
            change_balance(TX_RESEARCH_BONUS, 500, 0, (u16)owner);
            if (illness < 0x2c)
                reputation_affected(illness, 9, 0, owner, 0);
        }
    }
    allhosps[owner].equipment[item].unknown0 = 1;
    if (owner == player)
        announce_this(5, item);
}

// FUNCTION: 0x0006d428
void drug_improved(u16 illness, u8 owner)
{
    u16 effect;
    u16 unused;
    u16 cost;
    u16 done;
    AllHosps *hosp;

    effect = 0;
    unused = 0;
    cost = 999;
    done = 0;
    hosp = &allhosps[owner];
    if (!(old_newrand() & 1)) {
        cost = hosp->rooms[illness].drug.cost;
        if (cost > 10) {
            cost = cost * 95 / 100;
            cost < 10 ? cost = 10 : 0;
            hosp->rooms[illness].drug.cost = cost;
            if (old_newrand() % 7 == 0) {
                hosp->rooms[illness].drug.effectiveness += gbv.f140;
                hosp->rooms[illness].drug.effectiveness > 100 ? hosp->rooms[illness].drug.effectiveness = 100 : 0;
            }
            done = 1;
        }
    } else {
        effect = hosp->rooms[illness].drug.effectiveness;
        if (effect < 100) {
            effect += gbv.f140;
            hosp->rooms[illness].drug.effectiveness = effect;
            hosp->rooms[illness].drug.effectiveness > 100 ? hosp->rooms[illness].drug.effectiveness = 100 : 0;
            if (old_newrand() % 7 == 0) {
                hosp->rooms[illness].drug.cost--;
                hosp->rooms[illness].drug.cost > 10 ? hosp->rooms[illness].drug.cost = 10 : 0;
            }
            done = 1;
        }
    }
    if (owner == player && done)
        announce_this(7, illness);
}

// FUNCTION: 0x0006d6a0
void drug_researched(u16 illness, u8 owner)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    hosp->rooms[illness].drug.effectiveness = gbv.fc8;
    hosp->rooms[illness].drug.cost = gbv.fca;
    allhosps[owner].rooms[illness].unknown1C = 1;
    if (owner == player)
        announce_this(6, illness);
}

// FUNCTION: 0x0006d748
void item_upgraded(u16 item, u8 owner)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    switch (hosp->equipment[item].unknown1[0] % 6) {
    case 0:
        break;
    case 2:
        if (hosp->object_stats[item].unknown8 < gbv.ff2)
            hosp->object_stats[item].unknown8 += gbv.ff4;
        break;
    case 4:
        hosp->object_stats[item].cost = (u32)hosp->object_stats[item].cost * 7 / 8;
        hosp->object_stats[item].cost -= (u32)hosp->object_stats[item].cost % 10;
        if ((u32)hosp->object_stats[item].cost < objects[item].cost / 3)
            hosp->object_stats[item].cost = objects[item].cost / 3;
        break;
    }
    hosp->equipment[item].unknown1[0] += 2;
    if (owner == player)
        announce_this(8, item);
}

// FUNCTION: 0x0006d8fc
void auto_available(void)
{
    u8 owner;
    u8 item;
    u16 date;
    AllHosps *hosp;

    date = mycontrol->f648 * 12 + mycontrol->unknown650;
    for (owner = 0; owner < 4; owner++) {
        hosp = &allhosps[owner];
        for (item = 0; item < 0x3e; item++) {
            if (allhosps[owner].equipment[item].unknown0 == 0 && objects[item].unknown1c == 1) {
                if (hosp->object_stats[item].available != 0 && date >= hosp->object_stats[item].available) {
                    allhosps[owner].equipment[item].unknown0 = 1;
                    if (owner == player)
                        announce_this(9, item);
                }
            }
        }
    }
}

// FUNCTION: 0x0006da40
void research_contribution(Thing *researcher, s16 amount)
{
    if (researcher->tiredness > gbv.fea)
        amount += researcher->ability / 4;
    else if (researcher->tiredness > gbv.fe8)
        amount += researcher->ability / 2;
    else
        amount += researcher->ability;
    if (researcher->subtype == STAFF_DOCTOR && researcher->crazy)
        amount = -amount / 2;
    allhosps[researcher->owner].research_points += amount;
    award_counts.f145 += amount;
    if (allhosps[researcher->owner].research_points < 0)
        allhosps[researcher->owner].research_points = 0;
}

// FUNCTION: 0x0006db78
void send_person_for_research(Thing *patient)
{
    u16 idx;
    RoomInfo *room;

    room = get_expertise_dest(0x2c, patient);
    room->unknown32++;
    page_staff_i_need(patient, room);
    patient->dest_room = room - room_info;
    idx = check_frame(anim_starts[patient->subtype].idle, patient) + patient->facing;
    patient->sprite = start[idx].frame;
    patient->flip = start[idx].flags > 0 ? (u8)1 : (u8)0;
    patient->state = PS_GOTO_QUEUE;
    set_destination_coords(patient, room->unknown4);
}

// FUNCTION: 0x0006dc64
u16 emptiest_research_room(u8 owner)
{
    s32 best;
    u32 found;
    RoomInfo *room;
    s32 count;
    Thing *t;
    u8 k;

    best = -10000;
    found = 0;
    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->type == ROOM_RESEARCH && room->owner == owner) {
            count = 0;
            for (t = &thing[room->first_object]; t > thing; t = &thing[t->room_obj_next]) {
                for (k = 0; k < research_workstations.count; k++) {
                    if (t->subtype == research_workstations.kinds[k])
                        count++;
                }
            }
            for (t = &thing[room->first]; t > thing; t = &thing[t->room_next]) {
                if (t->type == THING_STAFF)
                    count--;
            }
            if (count > best) {
                best = count;
                found = room - room_info;
            }
        }
    }
    return found;
}

// FUNCTION: 0x0006ddf0
u16 find_rand_research_room(u8 owner, u16 exclude)
{
    u8 count;
    RoomInfo *room;
    u16 list[30];

    count = 0;
    for (room = room_info + 1; room < end_room_info; room++) {
        if ((room->type == ROOM_RESEARCH && room->owner == owner) && room - room_info != exclude) {
            list[count++] = room - room_info;
            if (count == 30)
                break;
        }
    }
    if (count == 0)
        return 0;
    return list[old_newrand() % count];
}

// FUNCTION: 0x0006dedc
u8 send_doctor_to_research(Thing *doctor)
{
    u16 id;

    if (doctor->room != 0 && doctor->room->type == ROOM_RESEARCH)
        id = find_rand_research_room(doctor->owner, doctor->room - room_info);
    else
        id = emptiest_research_room(doctor->owner);
    if (!id)
        return 0;
    staff_exit_room_check(doctor);
    if (doctor->room)
        set_complex(doctor, 0x136, doctor->room->unknown6);
    set_complex(doctor, 0x96, room_info[id].unknown4);
    doctor->next_state = SS_DOCTOR_RESEARCHES;
    doctor->resume_state = SS_PROCESS_COMPLEX;
    doctor->dest_room = id;
    process_staff_complex(doctor);
    return 1;
}

// FUNCTION: 0x0006dfe8
void person_autopsied(Thing *patient)
{
    u8 bad;
    Thing *staff;
    AllHosps *hosp;
    s32 staff_happiness;

    hosp = &allhosps[patient->owner];
    staff_happiness = 0;
    for (staff = thing + 1; staff < end_things; staff++) {
        if ((staff->type == THING_STAFF && staff->state != SS_APPLICANT) && staff->owner == patient->owner)
            staff_happiness += staff->happiness_s;
    }
    do_illness_research(patient->owner, patient->illness, gbv.f138 * expertise[patient->illness].cost / 100, 4);
    if ((staff_happiness << 8) / hosp->fc9c < 0x1900)
        bad = old_newrand() % 3 != 0;
    else
        bad = old_newrand() % 3 == 0;
    if (bad) {
        reputation_affected(0, 0x13, 0, patient->owner, 0);
        if (patient->owner == player)
            announce_this(2, 1);
    }
}

// FUNCTION: 0x0006e184
u8 find_best_drug_rating(u8 illness, u8 owner)
{
    return allhosps[owner].rooms[illness].drug.effectiveness;
}

// FUNCTION: 0x0006e1c8
s32 find_new_cure_cost(u8 illness, u8 owner)
{
    if ((allhosps[owner].equipment[expertise[illness].kind].unknown0 == 0 && objects[expertise[illness].kind].unknown1c == 1)
        && expertise[illness].unknown5[0] == 2)
        return expertise[illness].cost;
    return -1;
}

// FUNCTION: 0x0006e27c
s32 find_new_diag_cost(u8 illness, u8 owner)
{
    if ((allhosps[owner].equipment[expertise[illness].kind].unknown0 == 0 && objects[expertise[illness].kind].unknown1c == 1)
        && expertise[illness].unknown5[0] == 3)
        return expertise[illness].cost;
    return -1;
}

// FUNCTION: 0x0006e330
s32 find_new_drug_cost(u8 illness, u8 owner)
{
    if ((expertise[illness].kind == 0x27 && allhosps[owner].rooms[illness].unknown1D == 1)
        && allhosps[owner].rooms[illness].unknown1C == 0)
        return expertise[illness].cost;
    return -1;
}

// FUNCTION: 0x0006e3e0
s32 find_drug_improve_cost(u8 illness, u8 owner)
{
    u8 rating;

    if ((expertise[illness].kind == 0x27 && allhosps[owner].rooms[illness].unknown1D == 1)
        && allhosps[owner].rooms[illness].unknown1C == 1) {
        rating = find_best_drug_rating(illness, owner);
        if (rating < 100)
            return (gbv.f15a + rating / 25 * gbv.f15b) * expertise[illness].cost / 100;
    }
    return -1;
}

// FUNCTION: 0x0006e4dc
s32 find_hw_improve_cost(u8 illness, u8 owner)
{
    u8 kind;
    s32 level;

    kind = expertise[illness].kind;
    if ((allhosps[owner].equipment[kind].unknown0 == 1 && objects[kind].unknown1c == 1)
        && (expertise[illness].unknown5[0] == 2 || expertise[illness].unknown5[0] == 3)
        && allhosps[owner].equipment[kind].unknown1[0] < 40) {
        level = allhosps[owner].equipment[kind].unknown1[0] / 2;
        return (gbv.f15a + gbv.f15b * level) * expertise[illness].cost / 100;
    }
    return -1;
}

// FUNCTION: 0x0006e624
u8 choose_next_research_illness(u8 owner, s32 (*cost)(u8, u8))
{
    u8 found;
    u32 best;
    s32 i;
    s32 item_cost;

    found = 0;
    best = -1;
    for (i = 0x2e; i >= 0; i--) {
        item_cost = cost(i, owner);
        if (item_cost > 0) {
            if (item_cost < best) {
                best = item_cost;
                found = i;
            }
        }
    }
    return found;
}

// FUNCTION: 0x0006e694
void research_category_exhausted(const u8 owner, const u8 category)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    hosp->research_alloc[category] = -1;
    if (owner == player) {
        announce_this(3, category);
        othervar.fa84 = 1;
        othervar.fa85 = 10;
        play_sound(10, 0xdb, -1, -1);
    }
}

// FUNCTION: 0x0006e720
void do_illness_research(u8 owner, u8 illness, u32 amount, u8 category)
{
    AllHosps *hosp;
    s32 i;
    s32 cost;
    s32 rating;
    u8 kind;
    s32 level;

    hosp = &allhosps[owner];
    kind = expertise[illness].kind;
    if (expertise[illness].unknown5[0] == 2 || expertise[illness].unknown5[0] == 3) {
        for (i = 0; i < 0x2f; i++) {
            if (expertise[i].kind == expertise[illness].kind && expertise[i].cost != 0) {
                illness = i;
                break;
            }
        }
    }
    hosp->illness_research[illness] += amount;
    if ((expertise[illness].unknown5[0] == 2 || expertise[illness].unknown5[0] == 3)
        && (allhosps[owner].equipment[kind].unknown0 == 0 && objects[kind].unknown1c == 1)) {
        cost = hosp->research_cost[category] = expertise[illness].cost;
        if (hosp->illness_research[illness] >= cost) {
            hosp->illness_research[illness] -= cost;
            item_researched(expertise[illness].kind, illness, owner);
        }
    } else if ((expertise[illness].kind == 0x27 && allhosps[owner].rooms[illness].unknown1D == 1)
               && allhosps[owner].rooms[illness].unknown1C == 0) {
        cost = hosp->research_cost[category] = expertise[illness].cost;
        if (hosp->illness_research[illness] >= cost) {
            hosp->illness_research[illness] -= cost;
            drug_researched(illness, owner);
        }
    } else if ((expertise[illness].kind == 0x27 && allhosps[owner].rooms[illness].unknown1D == 1)
               && allhosps[owner].rooms[illness].unknown1C == 1) {
        rating = find_best_drug_rating(illness, owner);
        cost = (gbv.f15a + rating / 25 * gbv.f15b) * expertise[illness].cost / 100;
        if (hosp->illness_research[illness] >= cost) {
            hosp->illness_research[illness] -= cost;
            drug_improved(illness, owner);
        }
    } else if (((expertise[illness].unknown5[0] == 2 || expertise[illness].unknown5[0] == 3)
                && (allhosps[owner].equipment[kind].unknown0 == 1 && objects[kind].unknown1c == 1))
               && hosp->equipment[kind].unknown1[0] < 40) {
        level = hosp->equipment[kind].unknown1[0] / 2;
        cost = (gbv.f15a + gbv.f15b * level) * expertise[illness].cost / 100;
        if (hosp->illness_research[illness] >= cost) {
            hosp->illness_research[illness] -= cost;
            item_upgraded(kind, owner);
        }
    }
}
