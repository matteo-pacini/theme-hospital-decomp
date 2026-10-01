#include "hosp.h"

struct ComputerBucket {
    s32 f0;
    s32 f4;
    s32 f8;
    s32 fc;
    s32 f10;
    s32 f14;
    s32 f18[47];
    s32 fd4[47];
};

extern ComputerBucket (*comp_buckets)[12];
extern u8 skillnb_to_who[];
extern s16 diag_register[][8];
u32 old_normal_rand(u32 a, u32 b);
void add_treatment_to_history(Thing *w);

void calculate_score(u8 player);
void evaluate_reputation(const u8 room, const u8 player);
void inc_staff_stats(Thing *w, u8 player);
void remove_staff_in_struct(Thing *w);
void insert_skill(u16 skills, u8 player);
void set_expertise_staff(Thing *w);
void set_expertise_room(RoomInfo *room);
void inc_hardware_stats(u8 obj, u8 player);
u32 find_strategy_start(u8 type);
void computer_decision(u8 owner);
void get_computer_bucket(u8 owner);
u8 computer_employ(u8 type, u16 skills, u8 owner);
u8 computer_build(u8 type, u8 owner);
u8 computer_place_object(u8 obj, u8 free, u8 owner);
u8 computer_loan(s8 amount, u8 owner);
u8 computer_recheck_actions(u8 a, u8 owner);
u8 computer_staff_levels(u8 owner);
u8 computer_choose_room(u8 order, u8 owner);
u8 make_a_note(u16 type, s16 arg, s16 count, u8 owner);
u8 find_a_note(u16 type, u8 owner);
void remove_a_note(u16 type, u8 owner);
void declass_a_note(u16 type, u8 owner);
void computer_diag_progress(Thing *w, u8 owner);
u8 computer_free_build(u8 owner);
u8 computer_maintain_loan(u8 owner);
u8 computer_object_levels(u8 owner);
u8 computer_buy_land(u8 owner);

extern "C" {

extern char **computer_names;

}

// Single player: fills the free hospital slots (allhosps.f2a 0) with computer
// players (f2a 2), first those computer[] marks (f8), then random unused ones,
// giving each its name, personality (f1245) and strategy (f1244, f1240).
// FUNCTION: 0x0004e760
void setup_computer_players(void)
{
    u8 j;
    u8 k;
    u8 i;
    AllHosps *hosp;
    u8 used[20];

    if (statvar.unknownfd > 1 || gamevar.f174) {
        return;
    }
    for (i = 0; i < 20; i++) {
        used[i] = 0;
    }
    for (j = 0; j < 20; j++) {
        if (computer[j].f8) {
            for (k = 0; k < 4; k++) {
                hosp = &allhosps[k];
                if (hosp->f2a == 0) {
                    fn_000bbff0((char *)hosp, computer_names[j]);
                    hosp->f2a = 2;
                    hosp->f1245 = j;
                    hosp->f1244 = computer[j].f6;
                    hosp->f1240 = find_strategy_start(hosp->f1244);
                    hosp->f1f1 = myhosp->f1f1;
                    used[j] = 1;
                    break;
                }
            }
            if (k >= 4) {
                break;
            }
        }
    }
    if (k < 4) {
        for (; k < 4; k++) {
            hosp = &allhosps[k];
            if (hosp->f2a == 0) {
                hosp->f2a = 2;
                fn_000bbff0((char *)hosp, "DEF 64");
                for (i = 0; i < 1000; i++) {
                    j = (lbDisplay.mmouse_x + lbDisplay.mmouse_y + old_newrand()) % 20;
                    hosp->f1245 = j;
                    if (used[j] == 0) {
                        fn_000bbff0((char *)hosp, computer_names[j]);
                        used[j] = 1;
                        break;
                    }
                }
                hosp->f1244 = computer[j].f6;
                hosp->f1240 = find_strategy_start(hosp->f1244);
            }
        }
    }
}

// Year end for computer players: recalculates each one's value and score.
// Single player only.
// FUNCTION: 0x0004e9d8
void computer_year(void)
{
    u8 i;
    AllHosps *hosp;

    if (statvar.unknownfd <= 1) {
        for (i = 0; i < 4; i++) {
            hosp = &allhosps[i];
            if (hosp->f2a == 2) {
                calculate_value(i);
                calculate_score(i);
            }
        }
    }
}

// Daily for computer players: every fifth day (mycontrol->unknown64f), adds
// each one's simulated results so far to its hospital (get_computer_bucket).
// Single player only.
// FUNCTION: 0x0004ea50
void computer_day(void)
{
    u8 i;
    AllHosps *hosp;

    if (statvar.unknownfd <= 1) {
        if (mycontrol->unknown64f % 5 == 0) {
            for (i = 0; i < 4; i++) {
                hosp = &allhosps[i];
                if (hosp->f2a == 2) {
                    get_computer_bucket(i);
                }
            }
        }
    }
}

// Month end for computer players: adds the simulated results to each one's
// hospital and pays its wages (f1234). Single player only.
// FUNCTION: 0x0004eae0
void computer_month(void)
{
    u8 i;
    AllHosps *hosp;

    if (statvar.unknownfd <= 1) {
        for (i = 0; i < 4; i++) {
            hosp = &allhosps[i];
            if (hosp->f2a == 2) {
                get_computer_bucket(i);
                hosp->f35 = hosp->f35 - hosp->f1234;
            }
        }
    }
}

// Runs each computer player whose next action time (f11f8) has come: carries
// out its pending action (employ, build, buy objects, loan) or, with none,
// takes its next strategy step (computer_decision), then sets the time of the
// next one from its personality's pace. Single player only, and not during the
// tutorial.
// FUNCTION: 0x0004eb5c
void computer_action(void)
{
    s8 pace;
    u8 i;
    AllHosps *hosp;
    ComputerPlayer *ai;

    if (statvar.unknownfd > 1 || (advisor.f11f & 1)) {
        return;
    }
    for (i = 0; i < 4; i++) {
        hosp = &allhosps[i];
        if (hosp->f2a != 2 || hosp->f11f8 > actual_game_turn) {
            continue;
        }
        ai = &computer[hosp->f1245];
        pace = ai->f3 - old_newrand() % ai->f0;
        if (pace < 1) {
            pace = 1;
        } else if (pace > 100) {
            pace = 100;
        }
        switch (hosp->action.type) {
        case 0:
            computer_decision(i);
            break;
        case 1:
            if (computer_employ(hosp->action.arg, hosp->action.count, i)) {
                if (hosp->f1202) {
                    remove_a_note(1, i);
                }
            } else if (hosp->f1202) {
                declass_a_note(1, i);
            }
            hosp->f1202 = 0;
            hosp->action.type = 0;
            hosp->f11f8 = actual_game_turn + com_decide[0].delay * pace / 10;
            break;
        case 2:
            if (computer_build(hosp->action.arg, i)) {
                if (hosp->f1202) {
                    remove_a_note(2, i);
                }
            } else if (hosp->f1202) {
                declass_a_note(2, i);
            }
            hosp->f1202 = 0;
            hosp->action.type = 0;
            hosp->f11f8 = actual_game_turn + com_decide[0].delay * pace / 10;
            break;
        case 3:
            computer_place_object(hosp->action.arg, 0, i);
            hosp->action.count--;
            if (hosp->action.count <= 0) {
                hosp->action.type = 0;
            }
            hosp->f11f8 = actual_game_turn + com_decide[hosp->action.type].delay * pace / 10;
            break;
        case 4:
            computer_loan(hosp->action.arg, i);
            hosp->action.type = 0;
            hosp->f11f8 = actual_game_turn + com_decide[0].delay * pace / 10;
            break;
        }
    }
}

// Takes owner's next step in its com_strategy script: either sets it as the
// pending action (types 0-4) or carries it out at once (choose a room, recheck
// staff, staff and object levels, buy land, repay the loan, build freely,
// switch strategy, or announce).
// FUNCTION: 0x0004eebc
void computer_decision(u8 owner)
{
    AllHosps *hosp;
    ComputerPlayer *ai;
    int step;
    int pace;
    u32 action;

    hosp = &allhosps[owner];
    ai = &computer[hosp->f1245];
    step = hosp->f1240;
    hosp->f1240++;
    pace = ai->f3 - old_newrand() % ai->f0 + old_newrand() % 5;
    if (pace < 1) {
        pace = 1;
    } else if (pace > 100) {
        pace = 100;
    }
    action = com_strategy[step].type;
    switch (action) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        hosp->action.type = action;
        hosp->action.arg = com_strategy[step].arg;
        hosp->action.count = com_strategy[step].count;
        hosp->f11f8 = actual_game_turn + com_decide[action].delay * pace / 10;
        break;
    case 6:
        hosp->f1244 = 2;
        hosp->f1240 = find_strategy_start(2);
        break;
    case 7:
        computer_choose_room(0, owner);
        break;
    case 8:
        computer_recheck_actions(com_strategy[step].arg, owner);
        break;
    case 10:
        if (statvar.unknownfd <= 1) {
            hosp->unknown1256 = 1;
        }
        announce_this(0xc, owner);
        break;
    case 11:
        computer_staff_levels(owner);
        break;
    case 12:
        computer_object_levels(owner);
        break;
    case 13:
        computer_buy_land(owner);
        break;
    case 14:
        computer_maintain_loan(owner);
        break;
    case 15:
        hosp->f1240 = find_strategy_start(hosp->f1244);
        break;
    case 16:
        hosp->f1244 = 2;
        hosp->f1240 = find_strategy_start(2);
        break;
    case 9:
        break;
    case 17:
        computer_free_build(owner);
        break;
    case 5:
    default:
        break;
    }
}

#define ADVANCE_TIME()                                         \
    hours += step;                                             \
    if (hours > 30) {                                          \
        hours = 0;                                             \
        month++;                                               \
        month = month % 12;                                    \
        bucket = &comp_buckets[owner][month];                  \
    }

// Simulates patient's whole visit to computer player owner's hospital at once:
// GP and diagnosis visits until diagnosed, sent home or fed up, then treatment
// or not by the hospital's personality; adds the income, cures, deaths and
// reputation changes to this month's bucket (comp_buckets), advancing the
// simulated date as it goes.
// FUNCTION: 0x0004f15c
void process_computer_peep(Thing *patient, u8 owner)
{
    AllHosps *hosp;
    ComputerPlayer *ai;
    u32 month;
    u32 hours;
    ComputerBucket *bucket;
    int illness;
    int busy;
    int chance;
    int patience;
    int result;
    int step;
    int pct;

    hosp = &allhosps[owner];
    ai = &computer[hosp->f1245];
    month = mycontrol->unknown650;
    hours = mycontrol->unknown64f;
    bucket = &comp_buckets[owner][month];
    illness = patient->illness;
    hosp->f123c++;
    if (listhead[owner].objects[OBJ_DRINKS_MACHINE] * 10 > hosp->f123c) {
        busy = 1;
    }
    if (hosp->f123c > 5) {
        chance = 100 - hosp->f123c / ((hosp->f1238 + 1) * 10) - hosp->f123c / ((hosp->fc9c + 1) * 10)
            - hosp->f123c / ((listhead[owner].objects[OBJ_BENCH] + 1) * 10);
        if (chance < 0) {
            chance = 0;
        } else if (chance > 100) {
            chance = 100;
        }
    } else {
        chance = 20;
    }
    patience = patient->health;
    result = 0;
    step = 5;
    for (;;) {
        ADVANCE_TIME();
        if (allhosps[owner].rooms[ILL_GENERAL_PRACTICE].unknown1C == 1) {
            update_single_cure_cost(1, owner);
            bucket->f0 += hosp->rooms[ILL_GENERAL_PRACTICE].unknownC;
            patient->treatment = ILL_GENERAL_PRACTICE;
            computer_diag_progress(patient, owner);
            reputation_affected(1, 10, month % 12, patient->owner, 0);
            if (patient->diagnosis > gbv.ffc) {
                patient->treatment = patient->illness;
                reputation_affected(illness, 6, month % 12, patient->owner, 0);
                hosp->f1ef++;
                result = 1;
                break;
            }
        } else {
            reputation_affected(illness, 7, month % 12, owner, 0);
            result = 3;
            break;
        }
        ADVANCE_TIME();
        if (busy && old_newrand() % 5 == 0) {
            ADVANCE_TIME();
            bucket->f0 += 20;
        }
        if (old_newrand() % chance == 0) {
            result = 3;
            break;
        }
        chance -= 1;
        if (chance < 5) {
            chance = 5;
        }
        if (old_newrand() % patience == 0) {
            result = 2;
            break;
        }
        patience -= 1;
        if (patience < 5) {
            patience = 5;
        }
        find_possible_diagnosis(patient);
        if (patient->treatment == 0) {
            reputation_affected(illness, 7, month % 12, owner, 0);
            pct = patient->diagnosis * 100 / gbv.ffc;
            if (pct < 0) {
                pct = 0;
            } else if (pct > 99) {
                pct = 99;
            }
            if (ai->f5 <= pct) {
                if (old_newrand() % 100 > pct) {
                    result = 2;
                } else {
                    result = 1;
                }
            } else {
                result = 3;
            }
            break;
        }
        ADVANCE_TIME();
        update_single_cure_cost(patient->treatment, owner);
        bucket->f0 += hosp->rooms[patient->treatment].unknownC;
        add_treatment_to_history(patient);
        computer_diag_progress(patient, owner);
        reputation_affected(patient->treatment, 10, month % 12, patient->owner, 0);
        if (old_newrand() % chance == 0) {
            result = 3;
            break;
        }
    }
    reputation_affected(illness, 10, month % 12, patient->owner, 0);
    switch (result) {
    case 1:
        if (allhosps[owner].rooms[illness].unknown1C == 1) {
            if (expertise[illness].unknown5[0] == 1 && old_normal_rand(0, 99) >= hosp->rooms[illness].unknown1B) {
                bucket->f10++;
                reputation_affected(illness, 3, month % 12, owner, 0);
            } else {
                bucket->fc++;
                reputation_affected(illness, 0, month % 12, owner, 0);
            }
        } else {
            reputation_affected(illness, 2, month % 12, owner, 0);
            result = 3;
        }
        break;
    case 2:
        bucket->f10++;
        reputation_affected(illness, 3, month % 12, owner, 0);
        break;
    case 3:
        reputation_affected(illness, 2, month % 12, owner, 0);
        break;
    }
    reputation_affected(illness, 0x16, month % 12, owner, 0);
    bucket->f14++;
}

// Moves the results in owner's bucket for the current month (othervar.f9d1)
// into its hospital (money, cures, deaths, room statistics), updates its
// reputations and empties the bucket.
// FUNCTION: 0x0004f900
void get_computer_bucket(u8 owner)
{
    int month;
    ComputerBucket *bucket;
    AllHosps *hosp;
    u32 i;

    month = othervar.f9d1 % 12;
    bucket = &comp_buckets[owner][month];
    hosp = &allhosps[owner];
    hosp->f35 += bucket->f0;
    hosp->f1e9 += bucket->fc;
    hosp->f1eb += bucket->f10;
    hosp->fc82 += bucket->f4;
    hosp->fc86 += bucket->f8;
    evaluate_reputation(0, owner);
    hosp->f123c -= bucket->f14;
    if (hosp->f123c < 0) {
        hosp->f123c = 0;
    }
    for (i = 0; i < 47; i++) {
        hosp->rooms[i].f4 += bucket->f18[i];
        hosp->rooms[i].f8 += bucket->fd4[i];
        evaluate_reputation(i, owner);
    }
    memset(bucket, 0, 400);
}

// Hires for owner the first job applicant of staff kind type with all the
// skill bits in skills (0 for any), paying 1.2 times their salary. Returns 1
// if hired, 0 if there is no such applicant or not enough money.
// FUNCTION: 0x0004fa50
u8 computer_employ(u8 type, u16 skills, u8 owner)
{
    Thing *candidate;
    int ok;
    u32 i;
    AllHosps *hosp;

    for (candidate = thing + 1; candidate < end_things; candidate++) {
        if (candidate->type == THING_STAFF && candidate->state == SS_APPLICANT) {
            if ((u16)(candidate - thing) != gamevar.unknown75) {
                if (type == candidate->subtype) {
            ok = 1;
            if (skills) {
                for (i = 0; i < 9; i++) {
                    if (skills & (1 << i)) {
                        if ((candidate->skills & (1 << i)) == 0) {
                            ok = 0;
                            break;
                        }
                    }
                }
            }
            if (ok) {
                if (allhosps[owner].f35 >= candidate->salary * 12 / 10 && allhosps[owner].f35 > 0) {
                    hosp = &allhosps[owner];
                    inc_staff_stats(candidate, owner);
                    hosp->f35 -= candidate->salary * 12 / 10;
                    hosp->f1234 += candidate->salary;
                    hosp->f1246[candidate->subtype] += candidate->ability;
                    candidate->owner = owner;
                    set_expertise_staff(candidate);
                    insert_skill(candidate->skills, owner);
                    remove_staff_in_struct(candidate);
                    return 1;
                }
                return 0;
            }
                }
            }
        }
    }
    return 0;
}

// Builds a room of type type for owner (in the abstract: counts and
// statistics), with its required objects for free. Returns 1, or 0 if owner
// cannot afford it.
// FUNCTION: 0x0004fc48
u8 computer_build(u8 type, u8 owner)
{
    AllHosps *hosp;
    int i;
    RoomInfo room;

    if (allhosps[owner].f35 >= rooms[type].cost && allhosps[owner].f35 > 0) {
        hosp = &allhosps[owner];
        hosp->f35 -= rooms[type].cost;
        hosp->f1238++;
        for (i = 0; i < rooms[type].fc; i++) {
            computer_place_object(rooms[type].objects[i], 0, owner);
        }
        listhead[owner].rooms[type]++;
        room.type = type;
        room.owner = owner;
        set_expertise_room(&room);
        return 1;
    }
    return 0;
}

// Buys object obj for owner (in the abstract), free of charge if free is set.
// Returns 1, or 0 if owner cannot afford it.
// FUNCTION: 0x0004fd6c
u8 computer_place_object(u8 obj, u8 free, u8 owner)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    if ((allhosps[owner].f35 >= (u32)hosp->object_stats[obj].cost && allhosps[owner].f35 > 0) || free) {
        if (!free) {
            hosp->f35 -= hosp->object_stats[obj].cost;
        }
        inc_hardware_stats(obj, owner);
        if (objects[obj].unknown14 == 7) {
            listhead[owner].rooms[ROOM_RECEPTION]++;
        }
        listhead[owner].objects[obj]++;
        return 1;
    }
    return 0;
}

// Changes owner's loan by 5000: amount negative adds 5000 to the balance and to
// f3d if the balance is at least 5000 and f3d is positive; amount positive
// takes 5000 from both if f3d stays at least 5000 below the limit f41. Returns
// 1 if done.
// FUNCTION: 0x0004fe78
u8 computer_loan(s8 amount, u8 owner)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    if ((amount < 0 && hosp->f35 >= 5000) && hosp->f3d > 0) {
        hosp->f35 += 5000;
        hosp->f3d += 5000;
        return 1;
    }
    if (amount > 0 && hosp->f41 - 5000 >= hosp->f3d) {
        hosp->f35 -= 5000;
        hosp->f3d -= 5000;
        return 1;
    }
    return 0;
}

// Looks back over owner's strategy for the first staff hire step whose kind
// owner now has fewer of than the strategy has hired, and makes it the pending
// action. a 1 repeats this step next time; otherwise the position reached is
// kept (f1242) to resume from. Always returns 0.
// FUNCTION: 0x0004ff24
u8 computer_recheck_actions(u8 a, u8 owner)
{
    AllHosps *hosp;
    int counts[5];
    int i;

    hosp = &allhosps[owner];
    memset(counts, 0, 20);
    i = find_strategy_start(hosp->f1244);
    if (hosp->f1242) {
        for (; i < hosp->f1242; i++) {
            if (com_strategy[i].type == 1) {
                counts[com_strategy[i].arg]++;
            }
        }
    }
    for (; i < hosp->f1240; i++) {
        if (com_strategy[i].type == 1) {
            counts[com_strategy[i].arg]++;
            if (hosp->fc9e[com_strategy[i].arg] < counts[com_strategy[i].arg]) {
                hosp->action.type = 1;
                hosp->action.arg = com_strategy[i].arg;
                hosp->action.count = com_strategy[i].count;
                hosp->f11f8 = actual_game_turn;
                if (a == 1) {
                    hosp->f1240--;
                } else {
                    hosp->f1242 = i + 1;
                }
                break;
            }
        }
    }
    if (a == 0 && i < hosp->f1240) {
        hosp->f1240--;
    }
    if (a == 1 || hosp->f1240 == i) {
        hosp->f1242 = 0;
    }
    return 0;
}

// Makes hiring staff owner's pending action: a noted hire if there is one,
// else the first kind (nurse, doctor, handyman, receptionist) below the level
// its patient load calls for. Returns 1 if an action was set.
// FUNCTION: 0x0005010c
u8 computer_staff_levels(u8 owner)
{
    AllHosps *hosp;
    ComputerPlayer *ai;
    int pace;
    int i;
    int max;

    hosp = &allhosps[owner];
    ai = &computer[hosp->f1245];
    if (find_a_note(1, owner)) {
        pace = ai->f3 - old_newrand() % ai->f0;
        if (pace < 1) {
            pace = 1;
        } else if (pace > 100) {
            pace = 100;
        }
        hosp->f11f8 = actual_game_turn + com_decide[1].delay * pace / 10;
        return 1;
    }
    for (i = 0; i <= 3; i++) {
        max = hosp->f123c / staff_max_nbs[i][ai->f1];
        if (hosp->fc9e[i] < max) {
            pace = ai->f3 - old_newrand() % ai->f0;
            if (pace < 1) {
                pace = 1;
            } else if (pace > 100) {
                pace = 100;
            }
            hosp->action.type = 1;
            hosp->action.arg = i;
            hosp->f11f8 = actual_game_turn + com_decide[1].delay * pace / 10;
            return 1;
        }
    }
    return 0;
}

// Returns the com_strategy index just after the start marker of strategy type,
// or 0 if there is none.
// FUNCTION: 0x000502e0
u32 find_strategy_start(u8 type)
{
    int i;

    for (i = 0;; i++) {
        if (com_strategy[i].type == 9) {
            break;
        }
        if (com_strategy[i].type == 5 && com_strategy[i].arg == type) {
            return i + 1;
        }
    }
    return 0;
}

// Makes building the first room in com_room_orders[order] that owner lacks,
// may build and can afford its pending action, and returns that room type. If
// only unaffordable rooms remain, adds note 4. Returns 0 if none was
// chosen.
// FUNCTION: 0x00050360
u8 computer_choose_room(u8 order, u8 owner)
{
    int failed;
    AllHosps *hosp;
    ComputerPlayer *ai;
    int pace;
    int i;
    int room;

    failed = 0;
    hosp = &allhosps[owner];
    ai = &computer[hosp->f1245];
    pace = ai->f3 - old_newrand() % ai->f0;
    if (pace < 1) {
        pace = 1;
    } else if (pace > 100) {
        pace = 100;
    }
    for (i = 1; i <= com_room_orders[order][0]; i++) {
        room = com_room_orders[order][i];
        if (listhead[owner].rooms[room] == 0 && room_available(room)) {
            if (allhosps[owner].f35 >= rooms[room].cost && allhosps[owner].f35 > 0) {
                hosp->action.type = 2;
                hosp->action.arg = room;
                hosp->f11f8 = actual_game_turn + com_decide[2].delay * pace / 10;
                return room;
            } else {
                failed = 1;
            }
        }
    }
    if (failed) {
        make_a_note(4, 0, 0, owner);
    }
    return 0;
}

// Stores a note (a postponed action type, arg, count) in a free slot of
// owner's notes. Returns 1, or 0 if all 8 are used.
// FUNCTION: 0x00050528
u8 make_a_note(u16 type, s16 arg, s16 count, u8 owner)
{
    AllHosps *hosp;
    int i;

    hosp = &allhosps[owner];
    for (i = 0; i < 8; i++) {
        if (hosp->notes[i].type == 0) {
            hosp->notes[i].type = type;
            hosp->notes[i].arg = arg;
            hosp->notes[i].count = count;
            return 1;
        }
    }
    return 0;
}

// If owner has a note of type, makes it the pending action (flagging it as
// from a note, f1202) and returns 1; else 0.
// FUNCTION: 0x000505d0
u8 find_a_note(u16 type, u8 owner)
{
    AllHosps *hosp;
    int i;

    hosp = &allhosps[owner];
    for (i = 0; i < 8; i++) {
        if (hosp->notes[i].type == type) {
            hosp->action.type = type;
            hosp->action.arg = hosp->notes[i].arg;
            hosp->action.count = hosp->notes[i].count;
            hosp->f1202 = 1;
            return 1;
        }
    }
    return 0;
}

// Removes owner's first note of type.
// FUNCTION: 0x00050684
void remove_a_note(u16 type, u8 owner)
{
    AllHosps *hosp;
    int i;

    hosp = &allhosps[owner];
    for (i = 0; i < 8; i++) {
        if (hosp->notes[i].type == type) {
            hosp->notes[i].type = 0;
            break;
        }
    }
}

// Moves owner's note of type to the last free slot, behind the others.
// FUNCTION: 0x000506f4
void declass_a_note(u16 type, u8 owner)
{
    AllHosps *hosp;
    int i;

    hosp = &allhosps[owner];
    find_a_note(type, owner);
    remove_a_note(type, owner);
    for (i = 7; i >= 0; i--) {
        if (hosp->notes[i].type == 0) {
            hosp->notes[i].type = hosp->action.type;
            hosp->notes[i].arg = hosp->action.arg;
            hosp->notes[i].count = hosp->action.count;
            break;
        }
    }
}

// Raises patient's diagnosis progress by a simulated visit to their current
// treatment room in owner's hospital: by the average ability of the staff kind
// it needs, a random spread, and the room's diagnosis bonus.
// FUNCTION: 0x000507c4
void computer_diag_progress(Thing *patient, u8 owner)
{
    AllHosps *hosp;
    u32 skill;
    int i;
    int who;
    int val;

    hosp = &allhosps[owner];
    skill = expertise[patient->treatment].unknown0;
    for (i = 0; i < 9; i++) {
        if (skill & (1 << i)) {
            break;
        }
    }
    who = skillnb_to_who[i];
    val = patient->diagnosis - patient->diagnosis_handicap;
    val += hosp->f1246[who] / hosp->fc9e[who];
    val += 250 - old_normal_rand(0, 500);
    if (patient->treatment >= 0x24) {
        val += diag_register[patient->illness][patient->treatment - 0x24];
    }
    if (patient->diagnosis < val) {
        patient->diagnosis = val;
    }
}

// Makes building owner's pending action: a noted build if any, else the next
// room from room orders 0 then 1, noting a hire for its staff. Returns 1 if an
// action was set.
// FUNCTION: 0x0005090c
u8 computer_free_build(u8 owner)
{
    AllHosps *hosp;
    ComputerPlayer *ai;
    int pace;
    int room;
    int arg;
    int count;

    hosp = &allhosps[owner];
    ai = &computer[hosp->f1245];
    if (find_a_note(2, owner)) {
        pace = ai->f3 - old_newrand() % ai->f0;
        if (pace < 1) {
            pace = 1;
        } else if (pace > 100) {
            pace = 100;
        }
        hosp->f11f8 = actual_game_turn + com_decide[2].delay * pace / 10;
        return 1;
    }
    room = computer_choose_room(0, owner);
    if (room != 0) {
        arg = rooms[room].f26;
        if (arg >= 0) {
            count = rooms[room].unknown8;
            make_a_note(1, arg, count, owner);
        }
        return 1;
    }
    room = computer_choose_room(1, owner);
    if (room != 0) {
        arg = rooms[room].f26;
        if (arg >= 0) {
            count = rooms[room].unknown8;
            make_a_note(1, arg, count, owner);
        }
        return 1;
    }
    return 0;
}

// Makes a loan change (action 4, arg -1) owner's pending action if it has a
// loan (f3d) and more than 6000; a pending note 4 is taken (with its arg 0)
// and removed instead. Returns 1 if an action was set.
// FUNCTION: 0x00050aa8
u8 computer_maintain_loan(u8 owner)
{
    AllHosps *hosp;
    ComputerPlayer *ai;
    int pace;

    hosp = &allhosps[owner];
    ai = &computer[hosp->f1245];
    if (find_a_note(4, owner)) {
        pace = ai->f3 - old_newrand() % ai->f0;
        if (pace < 1) {
            pace = 1;
        } else if (pace > 100) {
            pace = 100;
        }
        hosp->f11f8 = actual_game_turn + com_decide[4].delay * pace / 10;
        hosp->f1202 = 0;
        remove_a_note(4, owner);
        return 1;
    }
    if (hosp->f3d != 0 && hosp->f35 > 6000) {
        pace = ai->f3 - old_newrand() % ai->f0;
        if (pace < 1) {
            pace = 1;
        } else if (pace > 100) {
            pace = 100;
        }
        hosp->action.type = 4;
        hosp->action.arg = -1;
        hosp->f11f8 = actual_game_turn + com_decide[4].delay * pace / 10;
        return 1;
    }
    return 0;
}

// Makes buying objects owner's pending action for the first of four object
// kinds (object_nbs) owner has fewer of than its patient load calls for (1 to
// 4 of them). Returns 1 if an action was set.
// FUNCTION: 0x00050c44
u8 computer_object_levels(u8 owner)
{
    AllHosps *hosp;
    ComputerPlayer *ai;
    int i;
    int obj;
    int max;
    int pace;
    int n;

    hosp = &allhosps[owner];
    ai = &computer[hosp->f1245];
    for (i = 0; i < 4; i++) {
        obj = object_nbs[i][0];
        max = hosp->f123c / object_nbs[i][ai->f4 + 1];
        if (hosp->object_stats[obj].fA < max) {
            pace = ai->f3 - old_newrand() % ai->f0;
            if (pace < 1) {
                pace = 1;
            } else if (pace > 100) {
                pace = 100;
            }
            hosp->action.type = 3;
            hosp->action.arg = obj;
            n = max - hosp->object_stats[obj].fA;
            if (n < 1) {
                n = 1;
            } else if (n > 4) {
                n = 4;
            }
            hosp->action.count = n;
            hosp->f11f8 = actual_game_turn + com_decide[3].delay * pace / 10;
            return 1;
        }
    }
    return 0;
}

// Buys owner as many plots as the local player has more of, if it can afford
// them (else adds note 4). Returns 1 if bought.
// FUNCTION: 0x00050de4
u8 computer_buy_land(u8 owner)
{
    AllHosps *hosp;
    ComputerPlayer *ai;
    int n;

    hosp = &allhosps[owner];
    ai = &computer[hosp->f1245];
    if (myhosp->f1f1 > hosp->f1f1) {
        n = myhosp->f1f1 - hosp->f1f1;
        if (gbv.f7c * n <= allhosps[owner].f35 && allhosps[owner].f35 > 0) {
            hosp->f35 -= n * gbv.f7c;
            hosp->f1f1 += n;
            return 1;
        }
        make_a_note(4, 0, 0, owner);
    }
    return 0;
}
