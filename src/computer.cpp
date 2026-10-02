#include "hosp.h"

/*
 * One month of a computer player's simulated results
 * (comp_buckets[player][month]), added to its hospital by get_computer_bucket.
 */
struct ComputerBucket {
    // 0x0: Money taken.
    s32 income;
    // 0x4: Reputation points from reputation_affected.
    s32 rep_points;
    // 0x8: Number of reputation events.
    s32 rep_events;
    // 0xc: Patients cured.
    s32 cures;
    // 0x10: Patients who died.
    s32 deaths;
    // 0x14: Patients whose visit was simulated; taken off the hospital's
    // patient load.
    s32 patients_done;
    // 0x18: rep_points per treatment.
    s32 room_rep_points[47];
    // 0xd4: rep_events per treatment.
    s32 room_rep_events[47];
};

extern ComputerBucket (*comp_buckets)[12];
extern u8 skillnb_to_who[];
extern s16 diag_register[][8];
u32 old_normal_rand(u32 low, u32 high);
void add_treatment_to_history(Thing *patient);

void calculate_score(u8 owner);
void evaluate_reputation(const u8 treatment, const u8 owner);
void inc_staff_stats(Thing *staff, u8 owner);
void remove_staff_in_struct(Thing *staff);
void insert_skill(u16 mask, u8 owner);
void set_expertise_staff(Thing *staff);
void set_expertise_room(RoomInfo *room);
void inc_hardware_stats(u8 kind, u8 owner);
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
void computer_diag_progress(Thing *patient, u8 owner);
u8 computer_free_build(u8 owner);
u8 computer_maintain_loan(u8 owner);
u8 computer_object_levels(u8 owner);
u8 computer_buy_land(u8 owner);

extern "C" {

extern char **computer_names;
}

// Single player: fills the free hospital slots (allhosps.player_type 0) with computer
// players (player_type 2), first those computer[].playing marks, then random
// unused ones, giving each its name, computer[] entry (ai_index) and strategy
// (strategy, strategy_step).
// FUNCTION: 0x0004e760
void setup_computer_players(void)
{
    u8 j;
    u8 k;
    u8 i;
    AllHosps *hosp;
    u8 used[20];

    if (statvar.num_players > 1 || gamevar.rat_fest_level) {
        return;
    }
    for (i = 0; i < 20; i++) {
        used[i] = 0;
    }
    for (j = 0; j < 20; j++) {
        if (computer[j].playing) {
            for (k = 0; k < 4; k++) {
                hosp = &allhosps[k];
                if (hosp->player_type == 0) {
                    lib_strcpy((char *)hosp, computer_names[j]);
                    hosp->player_type = 2;
                    hosp->ai_index = j;
                    hosp->strategy = computer[j].strategy;
                    hosp->strategy_step = find_strategy_start(hosp->strategy);
                    hosp->area = myhosp->area;
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
            if (hosp->player_type == 0) {
                hosp->player_type = 2;
                lib_strcpy((char *)hosp, "DEF 64");
                for (i = 0; i < 1000; i++) {
                    j = (lbDisplay.mmouse_x + lbDisplay.mmouse_y + old_newrand()) % 20;
                    hosp->ai_index = j;
                    if (used[j] == 0) {
                        lib_strcpy((char *)hosp, computer_names[j]);
                        used[j] = 1;
                        break;
                    }
                }
                hosp->strategy = computer[j].strategy;
                hosp->strategy_step = find_strategy_start(hosp->strategy);
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

    if (statvar.num_players <= 1) {
        for (i = 0; i < 4; i++) {
            hosp = &allhosps[i];
            if (hosp->player_type == 2) {
                calculate_value(i);
                calculate_score(i);
            }
        }
    }
}

// Daily for computer players: every fifth day (mycontrol->day), adds
// each one's simulated results so far to its hospital (get_computer_bucket).
// Single player only.
// FUNCTION: 0x0004ea50
void computer_day(void)
{
    u8 i;
    AllHosps *hosp;

    if (statvar.num_players <= 1) {
        if (mycontrol->day % 5 == 0) {
            for (i = 0; i < 4; i++) {
                hosp = &allhosps[i];
                if (hosp->player_type == 2) {
                    get_computer_bucket(i);
                }
            }
        }
    }
}

// Month end for computer players: adds the simulated results to each one's
// hospital and pays its wages (wage_bill). Single player only.
// FUNCTION: 0x0004eae0
void computer_month(void)
{
    u8 i;
    AllHosps *hosp;

    if (statvar.num_players <= 1) {
        for (i = 0; i < 4; i++) {
            hosp = &allhosps[i];
            if (hosp->player_type == 2) {
                get_computer_bucket(i);
                hosp->balance = hosp->balance - hosp->wage_bill;
            }
        }
    }
}

// Runs each computer player whose next action time (next_action_turn) has come: carries
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

    if (statvar.num_players > 1 || (advisor.flags & 1)) {
        return;
    }
    for (i = 0; i < 4; i++) {
        hosp = &allhosps[i];
        if (hosp->player_type != 2 || hosp->next_action_turn > actual_game_turn) {
            continue;
        }
        ai = &computer[hosp->ai_index];
        pace = ai->speed - old_newrand() % ai->skill;
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
                if (hosp->action_from_note) {
                    remove_a_note(1, i);
                }
            } else if (hosp->action_from_note) {
                declass_a_note(1, i);
            }
            hosp->action_from_note = 0;
            hosp->action.type = 0;
            hosp->next_action_turn = actual_game_turn + com_decide[0].delay * pace / 10;
            break;
        case 2:
            if (computer_build(hosp->action.arg, i)) {
                if (hosp->action_from_note) {
                    remove_a_note(2, i);
                }
            } else if (hosp->action_from_note) {
                declass_a_note(2, i);
            }
            hosp->action_from_note = 0;
            hosp->action.type = 0;
            hosp->next_action_turn = actual_game_turn + com_decide[0].delay * pace / 10;
            break;
        case 3:
            computer_place_object(hosp->action.arg, 0, i);
            hosp->action.count--;
            if (hosp->action.count <= 0) {
                hosp->action.type = 0;
            }
            hosp->next_action_turn = actual_game_turn + com_decide[hosp->action.type].delay * pace / 10;
            break;
        case 4:
            computer_loan(hosp->action.arg, i);
            hosp->action.type = 0;
            hosp->next_action_turn = actual_game_turn + com_decide[0].delay * pace / 10;
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
    ai = &computer[hosp->ai_index];
    step = hosp->strategy_step;
    hosp->strategy_step++;
    pace = ai->speed - old_newrand() % ai->skill + old_newrand() % 5;
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
        hosp->next_action_turn = actual_game_turn + com_decide[action].delay * pace / 10;
        break;
    case 6:
        hosp->strategy = 2;
        hosp->strategy_step = find_strategy_start(2);
        break;
    case 7:
        computer_choose_room(0, owner);
        break;
    case 8:
        computer_recheck_actions(com_strategy[step].arg, owner);
        break;
    case 10:
        if (statvar.num_players <= 1) {
            hosp->hospital_open = 1;
        }
        announce_this(ANN_RIVAL_OPENED, owner);
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
        hosp->strategy_step = find_strategy_start(hosp->strategy);
        break;
    case 16:
        hosp->strategy = 2;
        hosp->strategy_step = find_strategy_start(2);
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

#define ADVANCE_TIME()                                                                                                 \
    hours += step;                                                                                                     \
    if (hours > 30) {                                                                                                  \
        hours = 0;                                                                                                     \
        month++;                                                                                                       \
        month = month % 12;                                                                                            \
        bucket = &comp_buckets[owner][month];                                                                          \
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
    ai = &computer[hosp->ai_index];
    month = mycontrol->month;
    hours = mycontrol->day;
    bucket = &comp_buckets[owner][month];
    illness = patient->illness;
    hosp->patient_load++;
    if (listhead[owner].objects[OBJ_DRINKS_MACHINE] * 10 > hosp->patient_load) {
        busy = 1;
    }
    if (hosp->patient_load > 5) {
        chance = 100 - hosp->patient_load / ((hosp->rooms_built + 1) * 10) -
                 hosp->patient_load / ((hosp->staff_count + 1) * 10) -
                 hosp->patient_load / ((listhead[owner].objects[OBJ_BENCH] + 1) * 10);
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
        if (allhosps[owner].rooms[ILL_GENERAL_PRACTICE].cure_available == 1) {
            update_single_cure_cost(1, owner);
            bucket->income += hosp->rooms[ILL_GENERAL_PRACTICE].price;
            patient->treatment = ILL_GENERAL_PRACTICE;
            computer_diag_progress(patient, owner);
            reputation_affected(1, REP_NEXT_TREATMENT, month % 12, patient->owner, 0);
            if (patient->diagnosis > gbv.diagnosis_complete) {
                patient->treatment = patient->illness;
                reputation_affected(illness, REP_COMPUTER_DIAGNOSED, month % 12, patient->owner, 0);
                hosp->diagnosed++;
                result = 1;
                break;
            }
        } else {
            reputation_affected(illness, REP_NO_TREATMENT_FOUND, month % 12, owner, 0);
            result = 3;
            break;
        }
        ADVANCE_TIME();
        if (busy && old_newrand() % 5 == 0) {
            ADVANCE_TIME();
            bucket->income += 20;
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
            reputation_affected(illness, REP_NO_TREATMENT_FOUND, month % 12, owner, 0);
            pct = patient->diagnosis * 100 / gbv.diagnosis_complete;
            if (pct < 0) {
                pct = 0;
            } else if (pct > 99) {
                pct = 99;
            }
            if (ai->guess_at <= pct) {
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
        bucket->income += hosp->rooms[patient->treatment].price;
        add_treatment_to_history(patient);
        computer_diag_progress(patient, owner);
        reputation_affected(patient->treatment, REP_NEXT_TREATMENT, month % 12, patient->owner, 0);
        if (old_newrand() % chance == 0) {
            result = 3;
            break;
        }
    }
    reputation_affected(illness, REP_NEXT_TREATMENT, month % 12, patient->owner, 0);
    switch (result) {
    case 1:
        if (allhosps[owner].rooms[illness].cure_available == 1) {
            if (expertise[illness].research_type == 1 &&
                old_normal_rand(0, 99) >= hosp->rooms[illness].drug_effectiveness) {
                bucket->deaths++;
                reputation_affected(illness, REP_DIED, month % 12, owner, 0);
            } else {
                bucket->cures++;
                reputation_affected(illness, REP_CURED, month % 12, owner, 0);
            }
        } else {
            reputation_affected(illness, REP_GO_FOR_TREATMENT, month % 12, owner, 0);
            result = 3;
        }
        break;
    case 2:
        bucket->deaths++;
        reputation_affected(illness, REP_DIED, month % 12, owner, 0);
        break;
    case 3:
        reputation_affected(illness, REP_GO_FOR_TREATMENT, month % 12, owner, 0);
        break;
    }
    reputation_affected(illness, REP_CASE, month % 12, owner, 0);
    bucket->patients_done++;
}

// Moves the results in owner's bucket for the current month (othervar.game_month)
// into its hospital (money, cures, deaths, room statistics), updates its
// reputations and empties the bucket.
// FUNCTION: 0x0004f900
void get_computer_bucket(u8 owner)
{
    int month;
    ComputerBucket *bucket;
    AllHosps *hosp;
    u32 i;

    month = othervar.game_month % 12;
    bucket = &comp_buckets[owner][month];
    hosp = &allhosps[owner];
    hosp->balance += bucket->income;
    hosp->cures += bucket->cures;
    hosp->deaths += bucket->deaths;
    hosp->rep_points += bucket->rep_points;
    hosp->rep_events += bucket->rep_events;
    evaluate_reputation(0, owner);
    hosp->patient_load -= bucket->patients_done;
    if (hosp->patient_load < 0) {
        hosp->patient_load = 0;
    }
    for (i = 0; i < 47; i++) {
        hosp->rooms[i].rep_points += bucket->room_rep_points[i];
        hosp->rooms[i].rep_events += bucket->room_rep_events[i];
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
            if ((u16)(candidate - thing) != gamevar.held_staff) {
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
                        if (allhosps[owner].balance >= candidate->salary * 12 / 10 && allhosps[owner].balance > 0) {
                            hosp = &allhosps[owner];
                            inc_staff_stats(candidate, owner);
                            hosp->balance -= candidate->salary * 12 / 10;
                            hosp->wage_bill += candidate->salary;
                            hosp->staff_ability_sum[candidate->subtype] += candidate->ability;
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

    if (allhosps[owner].balance >= rooms[type].cost && allhosps[owner].balance > 0) {
        hosp = &allhosps[owner];
        hosp->balance -= rooms[type].cost;
        hosp->rooms_built++;
        for (i = 0; i < rooms[type].compulsory_count; i++) {
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
    if ((allhosps[owner].balance >= (u32)hosp->object_stats[obj].cost && allhosps[owner].balance > 0) || free) {
        if (!free) {
            hosp->balance -= hosp->object_stats[obj].cost;
        }
        inc_hardware_stats(obj, owner);
        if (objects[obj].category == 7) {
            listhead[owner].rooms[ROOM_RECEPTION]++;
        }
        listhead[owner].objects[obj]++;
        return 1;
    }
    return 0;
}

// Changes owner's loan by 5000: amount negative adds 5000 to the balance and to
// loan if the balance is at least 5000 and the loan is positive; amount
// positive takes 5000 from both if the loan stays at least 5000 below max_loan. Returns
// 1 if done.
// FUNCTION: 0x0004fe78
u8 computer_loan(s8 amount, u8 owner)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    if ((amount < 0 && hosp->balance >= 5000) && hosp->loan > 0) {
        hosp->balance += 5000;
        hosp->loan += 5000;
        return 1;
    }
    if (amount > 0 && hosp->max_loan - 5000 >= hosp->loan) {
        hosp->balance -= 5000;
        hosp->loan -= 5000;
        return 1;
    }
    return 0;
}

// Looks back over owner's strategy for the first staff hire step whose kind
// owner now has fewer of than the strategy has hired, and makes it the pending
// action. a 1 repeats this step next time; otherwise the position reached is
// kept (recheck_step) to resume from. Always returns 0.
// FUNCTION: 0x0004ff24
u8 computer_recheck_actions(u8 a, u8 owner)
{
    AllHosps *hosp;
    int counts[5];
    int i;

    hosp = &allhosps[owner];
    memset(counts, 0, 20);
    i = find_strategy_start(hosp->strategy);
    if (hosp->recheck_step) {
        for (; i < hosp->recheck_step; i++) {
            if (com_strategy[i].type == 1) {
                counts[com_strategy[i].arg]++;
            }
        }
    }
    for (; i < hosp->strategy_step; i++) {
        if (com_strategy[i].type == 1) {
            counts[com_strategy[i].arg]++;
            if (hosp->staff_by_kind[com_strategy[i].arg] < counts[com_strategy[i].arg]) {
                hosp->action.type = 1;
                hosp->action.arg = com_strategy[i].arg;
                hosp->action.count = com_strategy[i].count;
                hosp->next_action_turn = actual_game_turn;
                if (a == 1) {
                    hosp->strategy_step--;
                } else {
                    hosp->recheck_step = i + 1;
                }
                break;
            }
        }
    }
    if (a == 0 && i < hosp->strategy_step) {
        hosp->strategy_step--;
    }
    if (a == 1 || hosp->strategy_step == i) {
        hosp->recheck_step = 0;
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
    ai = &computer[hosp->ai_index];
    if (find_a_note(1, owner)) {
        pace = ai->speed - old_newrand() % ai->skill;
        if (pace < 1) {
            pace = 1;
        } else if (pace > 100) {
            pace = 100;
        }
        hosp->next_action_turn = actual_game_turn + com_decide[1].delay * pace / 10;
        return 1;
    }
    for (i = 0; i <= 3; i++) {
        max = hosp->patient_load / staff_max_nbs[i][ai->staff_levels];
        if (hosp->staff_by_kind[i] < max) {
            pace = ai->speed - old_newrand() % ai->skill;
            if (pace < 1) {
                pace = 1;
            } else if (pace > 100) {
                pace = 100;
            }
            hosp->action.type = 1;
            hosp->action.arg = i;
            hosp->next_action_turn = actual_game_turn + com_decide[1].delay * pace / 10;
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
    ai = &computer[hosp->ai_index];
    pace = ai->speed - old_newrand() % ai->skill;
    if (pace < 1) {
        pace = 1;
    } else if (pace > 100) {
        pace = 100;
    }
    for (i = 1; i <= com_room_orders[order][0]; i++) {
        room = com_room_orders[order][i];
        if (listhead[owner].rooms[room] == 0 && room_available(room)) {
            if (allhosps[owner].balance >= rooms[room].cost && allhosps[owner].balance > 0) {
                hosp->action.type = 2;
                hosp->action.arg = room;
                hosp->next_action_turn = actual_game_turn + com_decide[2].delay * pace / 10;
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
// from a note, action_from_note) and returns 1; else 0.
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
            hosp->action_from_note = 1;
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
    skill = expertise[patient->treatment].staff_mask;
    for (i = 0; i < 9; i++) {
        if (skill & (1 << i)) {
            break;
        }
    }
    who = skillnb_to_who[i];
    val = patient->diagnosis - patient->diagnosis_handicap;
    val += hosp->staff_ability_sum[who] / hosp->staff_by_kind[who];
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
    ai = &computer[hosp->ai_index];
    if (find_a_note(2, owner)) {
        pace = ai->speed - old_newrand() % ai->skill;
        if (pace < 1) {
            pace = 1;
        } else if (pace > 100) {
            pace = 100;
        }
        hosp->next_action_turn = actual_game_turn + com_decide[2].delay * pace / 10;
        return 1;
    }
    room = computer_choose_room(0, owner);
    if (room != 0) {
        arg = rooms[room].staff_kind;
        if (arg >= 0) {
            count = rooms[room].doctor_skills;
            make_a_note(1, arg, count, owner);
        }
        return 1;
    }
    room = computer_choose_room(1, owner);
    if (room != 0) {
        arg = rooms[room].staff_kind;
        if (arg >= 0) {
            count = rooms[room].doctor_skills;
            make_a_note(1, arg, count, owner);
        }
        return 1;
    }
    return 0;
}

// Makes a loan change (action 4, arg -1) owner's pending action if it has a
// loan and more than 6000; a pending note 4 is taken (with its arg 0)
// and removed instead. Returns 1 if an action was set.
// FUNCTION: 0x00050aa8
u8 computer_maintain_loan(u8 owner)
{
    AllHosps *hosp;
    ComputerPlayer *ai;
    int pace;

    hosp = &allhosps[owner];
    ai = &computer[hosp->ai_index];
    if (find_a_note(4, owner)) {
        pace = ai->speed - old_newrand() % ai->skill;
        if (pace < 1) {
            pace = 1;
        } else if (pace > 100) {
            pace = 100;
        }
        hosp->next_action_turn = actual_game_turn + com_decide[4].delay * pace / 10;
        hosp->action_from_note = 0;
        remove_a_note(4, owner);
        return 1;
    }
    if (hosp->loan != 0 && hosp->balance > 6000) {
        pace = ai->speed - old_newrand() % ai->skill;
        if (pace < 1) {
            pace = 1;
        } else if (pace > 100) {
            pace = 100;
        }
        hosp->action.type = 4;
        hosp->action.arg = -1;
        hosp->next_action_turn = actual_game_turn + com_decide[4].delay * pace / 10;
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
    ai = &computer[hosp->ai_index];
    for (i = 0; i < 4; i++) {
        obj = object_nbs[i][0];
        max = hosp->patient_load / object_nbs[i][ai->comfort + 1];
        if (hosp->object_stats[obj].count < max) {
            pace = ai->speed - old_newrand() % ai->skill;
            if (pace < 1) {
                pace = 1;
            } else if (pace > 100) {
                pace = 100;
            }
            hosp->action.type = 3;
            hosp->action.arg = obj;
            n = max - hosp->object_stats[obj].count;
            if (n < 1) {
                n = 1;
            } else if (n > 4) {
                n = 4;
            }
            hosp->action.count = n;
            hosp->next_action_turn = actual_game_turn + com_decide[3].delay * pace / 10;
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
    ai = &computer[hosp->ai_index];
    if (myhosp->area > hosp->area) {
        n = myhosp->area - hosp->area;
        if (gbv.land_cost_per_tile * n <= allhosps[owner].balance && allhosps[owner].balance > 0) {
            hosp->balance -= n * gbv.land_cost_per_tile;
            hosp->area += n;
            return 1;
        }
        make_a_note(4, 0, 0, owner);
    }
    return 0;
}
