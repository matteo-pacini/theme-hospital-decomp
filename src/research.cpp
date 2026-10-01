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

// Spends each hospital's research points by its allocation to the five
// categories (0 cures, 1 diagnosis, 2 drugs, 3 improvements, 4 the
// specialisation): each goes to the cheapest open item of its category.
// Points for a category with nothing left are kept, and categories 0 and 1
// are then closed (research_category_exhausted).
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
                award_counts.research_spent += amount;
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
                    if ((statvar.num_players > 1 ? allhosps[owner].hospital_open : gamevar.research_illness) != 0)
                        do_illness_research(owner, statvar.num_players > 1 ? allhosps[owner].hospital_open : gamevar.research_illness, amount, 4);
                    break;
                }
                if (exhausted) {
                    hosp->research_points += amount;
                    award_counts.research_spent -= amount;
                }
            }
        }
    }
}

// Makes object item available to player `owner` (researched through
// illness), announcing it. In a network game the first to research it gets
// a 500 bonus and reputation, and the others get it three months later.
// FUNCTION: 0x0006d288
void item_researched(u16 item, u16 illness, u8 owner)
{
    u8 pay;
    u8 i;
    u16 date;

    pay = 1;
    if (statvar.num_players > 1) {
        date = mycontrol->year * 12 + mycontrol->month;
        for (i = 0; i < 4; i++) {
            if (allhosps[i].equipment[item].available == 1 && objects[item].available_for_level == 1)
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
    allhosps[owner].equipment[item].available = 1;
    if (owner == player)
        announce_this(5, item);
}

// Improves player `owner`'s drug for illness: at random, its cost falls by
// 5% (to at least 10, sometimes also raising its effectiveness) or its
// effectiveness rises by gbv.drug_improve_rate (to at most 100; sometimes the cost then
// drops, but the code caps it at 10 instead). Announced for this player.
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
                hosp->rooms[illness].drug.effectiveness += gbv.drug_improve_rate;
                hosp->rooms[illness].drug.effectiveness > 100 ? hosp->rooms[illness].drug.effectiveness = 100 : 0;
            }
            done = 1;
        }
    } else {
        effect = hosp->rooms[illness].drug.effectiveness;
        if (effect < 100) {
            effect += gbv.drug_improve_rate;
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

// Gives player `owner` the drug for illness at the starting effectiveness
// and cost (gbv.start_rating, gbv.start_cost), announced for this player.
// FUNCTION: 0x0006d6a0
void drug_researched(u16 illness, u8 owner)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    hosp->rooms[illness].drug.effectiveness = gbv.start_rating;
    hosp->rooms[illness].drug.cost = gbv.start_cost;
    allhosps[owner].rooms[illness].cure_available = 1;
    if (owner == player)
        announce_this(6, illness);
}

// Upgrades player `owner`'s object item by one level: every third level
// raises its strength (up to gbv.max_object_strength) and every third lowers its price by an
// eighth (not below a third of the base), announced for this player.
// FUNCTION: 0x0006d748
void item_upgraded(u16 item, u8 owner)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    switch (hosp->equipment[item].upgrade_level[0] % 6) {
    case 0:
        break;
    case 2:
        if (hosp->object_stats[item].strength < gbv.max_object_strength)
            hosp->object_stats[item].strength += gbv.research_increment;
        break;
    case 4:
        hosp->object_stats[item].cost = (u32)hosp->object_stats[item].cost * 7 / 8;
        hosp->object_stats[item].cost -= (u32)hosp->object_stats[item].cost % 10;
        if ((u32)hosp->object_stats[item].cost < objects[item].cost / 3)
            hosp->object_stats[item].cost = objects[item].cost / 3;
        break;
    }
    hosp->equipment[item].upgrade_level[0] += 2;
    if (owner == player)
        announce_this(8, item);
}

// Monthly: makes available each object whose availability date has come
// (object_stats.available, in months), announcing it to this player.
// FUNCTION: 0x0006d8fc
void auto_available(void)
{
    u8 owner;
    u8 item;
    u16 date;
    AllHosps *hosp;

    date = mycontrol->year * 12 + mycontrol->month;
    for (owner = 0; owner < 4; owner++) {
        hosp = &allhosps[owner];
        for (item = 0; item < 0x3e; item++) {
            if (allhosps[owner].equipment[item].available == 0 && objects[item].available_for_level == 1) {
                if (hosp->object_stats[item].available != 0 && date >= hosp->object_stats[item].available) {
                    allhosps[owner].equipment[item].available = 1;
                    if (owner == player)
                        announce_this(9, item);
                }
            }
        }
    }
}

// Adds researcher's work (ability, less when tired) plus amount to their
// hospital's research points; a crazy doctor takes half instead. Points
// never go below 0.
// FUNCTION: 0x0006da40
void research_contribution(Thing *researcher, s16 amount)
{
    if (researcher->tiredness > gbv.very_tired)
        amount += researcher->ability / 4;
    else if (researcher->tiredness > gbv.tired)
        amount += researcher->ability / 2;
    else
        amount += researcher->ability;
    if (researcher->subtype == STAFF_DOCTOR && researcher->crazy)
        amount = -amount / 2;
    allhosps[researcher->owner].research_points += amount;
    award_counts.research_earned += amount;
    if (allhosps[researcher->owner].research_points < 0)
        allhosps[researcher->owner].research_points = 0;
}

// Sends a dead patient to the nearest autopsy room (via queue) and pages
// its staff.
// FUNCTION: 0x0006db78
void send_person_for_research(Thing *patient)
{
    u16 idx;
    RoomInfo *room;

    room = get_expertise_dest(0x2c, patient);
    room->queue_incoming++;
    page_staff_i_need(patient, room);
    patient->dest_room = room - room_info;
    idx = check_frame(anim_starts[patient->subtype].idle, patient) + patient->facing;
    patient->sprite = start[idx].frame;
    patient->flip = start[idx].flags > 0 ? (u8)1 : (u8)0;
    patient->state = PS_GOTO_QUEUE;
    set_destination_coords(patient, room->door_tile);
}

// Returns player `owner`'s research room with the most free workstations
// (desks and machines minus staff in it), or 0 if none.
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

// Returns a random research room of player `owner` other than exclude, or 0
// if none.
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

// Sends doctor to research: to another research room if already in one,
// else the emptiest. Returns 0 if there is none to go to.
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
        set_complex(doctor, 0x136, doctor->room->door_inside_tile);
    set_complex(doctor, 0x96, room_info[id].door_tile);
    doctor->next_state = SS_DOCTOR_RESEARCHES;
    doctor->resume_state = SS_PROCESS_COMPLEX;
    doctor->dest_room = id;
    process_staff_complex(doctor);
    return 1;
}

// A patient autopsied: their illness gains research points, and with a
// chance that is higher when staff are unhappy, the hospital loses
// reputation (announced for this player).
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
    do_illness_research(patient->owner, patient->illness, gbv.autopsy_rsch_percent * expertise[patient->illness].cost / 100, 4);
    if ((staff_happiness << 8) / hosp->staff_count < 0x1900)
        bad = old_newrand() % 3 != 0;
    else
        bad = old_newrand() % 3 == 0;
    if (bad) {
        reputation_affected(0, 0x13, 0, patient->owner, 0);
        if (patient->owner == player)
            announce_this(2, 1);
    }
}

// Returns player `owner`'s drug effectiveness for illness (0-100).
// FUNCTION: 0x0006e184
u8 find_best_drug_rating(u8 illness, u8 owner)
{
    return allhosps[owner].rooms[illness].drug.effectiveness;
}

// Research cost of the treatment machine for illness if player `owner` lacks
// it and it is a cure, else -1.
// FUNCTION: 0x0006e1c8
s32 find_new_cure_cost(u8 illness, u8 owner)
{
    if ((allhosps[owner].equipment[expertise[illness].kind].available == 0 && objects[expertise[illness].kind].available_for_level == 1)
        && expertise[illness].unknown5[0] == 2)
        return expertise[illness].cost;
    return -1;
}

// Research cost of the diagnosis machine for illness if player `owner` lacks
// it, else -1.
// FUNCTION: 0x0006e27c
s32 find_new_diag_cost(u8 illness, u8 owner)
{
    if ((allhosps[owner].equipment[expertise[illness].kind].available == 0 && objects[expertise[illness].kind].available_for_level == 1)
        && expertise[illness].unknown5[0] == 3)
        return expertise[illness].cost;
    return -1;
}

// Research cost of the drug for illness if player `owner` has the illness
// known (rooms[illness].discovered) and no drug yet, else -1.
// FUNCTION: 0x0006e330
s32 find_new_drug_cost(u8 illness, u8 owner)
{
    if ((expertise[illness].kind == 0x27 && allhosps[owner].rooms[illness].discovered == 1)
        && allhosps[owner].rooms[illness].cure_available == 0)
        return expertise[illness].cost;
    return -1;
}

// Cost of improving player `owner`'s drug for illness (rising with its
// effectiveness), or -1 if there is none or it is at 100.
// FUNCTION: 0x0006e3e0
s32 find_drug_improve_cost(u8 illness, u8 owner)
{
    u8 rating;

    if ((expertise[illness].kind == 0x27 && allhosps[owner].rooms[illness].discovered == 1)
        && allhosps[owner].rooms[illness].cure_available == 1) {
        rating = find_best_drug_rating(illness, owner);
        if (rating < 100)
            return (gbv.rsch_improve_cost_percent + rating / 25 * gbv.rsch_improve_inc_percent) * expertise[illness].cost / 100;
    }
    return -1;
}

// Cost of upgrading player `owner`'s machine for illness (rising with its
// level), or -1 if they lack it or it is at the top level.
// FUNCTION: 0x0006e4dc
s32 find_hw_improve_cost(u8 illness, u8 owner)
{
    u8 kind;
    s32 level;

    kind = expertise[illness].kind;
    if ((allhosps[owner].equipment[kind].available == 1 && objects[kind].available_for_level == 1)
        && (expertise[illness].unknown5[0] == 2 || expertise[illness].unknown5[0] == 3)
        && allhosps[owner].equipment[kind].upgrade_level[0] < 40) {
        level = allhosps[owner].equipment[kind].upgrade_level[0] / 2;
        return (gbv.rsch_improve_cost_percent + gbv.rsch_improve_inc_percent * level) * expertise[illness].cost / 100;
    }
    return -1;
}

// Returns the illness (0-0x2e) with the lowest positive cost for player
// `owner` by cost(illness, owner), or 0 if none has one.
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

// Closes research category `category` for player `owner`
// (research_alloc -1), announcing it to this player.
// FUNCTION: 0x0006e694
void research_category_exhausted(const u8 owner, const u8 category)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    hosp->research_alloc[category] = -1;
    if (owner == player) {
        announce_this(3, category);
        othervar.research_exhausted = 1;
        othervar.fa85 = 10;
        play_sound(10, 0xdb, -1, -1);
    }
}

// Adds amount research points to illness for player `owner` (for a machine,
// to the first illness that uses it), and once enough are banked completes
// whatever is next for it: a new machine, a drug, a drug improvement or a
// machine upgrade.
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
        && (allhosps[owner].equipment[kind].available == 0 && objects[kind].available_for_level == 1)) {
        cost = hosp->research_cost[category] = expertise[illness].cost;
        if (hosp->illness_research[illness] >= cost) {
            hosp->illness_research[illness] -= cost;
            item_researched(expertise[illness].kind, illness, owner);
        }
    } else if ((expertise[illness].kind == 0x27 && allhosps[owner].rooms[illness].discovered == 1)
               && allhosps[owner].rooms[illness].cure_available == 0) {
        cost = hosp->research_cost[category] = expertise[illness].cost;
        if (hosp->illness_research[illness] >= cost) {
            hosp->illness_research[illness] -= cost;
            drug_researched(illness, owner);
        }
    } else if ((expertise[illness].kind == 0x27 && allhosps[owner].rooms[illness].discovered == 1)
               && allhosps[owner].rooms[illness].cure_available == 1) {
        rating = find_best_drug_rating(illness, owner);
        cost = (gbv.rsch_improve_cost_percent + rating / 25 * gbv.rsch_improve_inc_percent) * expertise[illness].cost / 100;
        if (hosp->illness_research[illness] >= cost) {
            hosp->illness_research[illness] -= cost;
            drug_improved(illness, owner);
        }
    } else if (((expertise[illness].unknown5[0] == 2 || expertise[illness].unknown5[0] == 3)
                && (allhosps[owner].equipment[kind].available == 1 && objects[kind].available_for_level == 1))
               && hosp->equipment[kind].upgrade_level[0] < 40) {
        level = hosp->equipment[kind].upgrade_level[0] / 2;
        cost = (gbv.rsch_improve_cost_percent + gbv.rsch_improve_inc_percent * level) * expertise[illness].cost / 100;
        if (hosp->illness_research[illness] >= cost) {
            hosp->illness_research[illness] -= cost;
            item_upgraded(kind, owner);
        }
    }
}
