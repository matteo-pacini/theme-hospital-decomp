#include "hosp.h"

/*
 * One month of a computer player's simulated results
 * (comp_buckets[player][month]); see computer.cpp.
 */
struct ComputerBucket {
    u8 unknown0[4];
    // 0x4: Reputation points from reputation_affected.
    s32 rep_points;
    // 0x8: Number of reputation events.
    s32 rep_events;
    u8 unknownc[0xc];
    // 0x18: rep_points per treatment.
    s32 room_rep_points[0x2f];
    // 0xd4: rep_events per treatment.
    s32 room_rep_events[0x2f];
};

extern "C" {
extern Menu choice;
extern Event player_events[];
}

extern ComputerBucket (*comp_buckets)[12];
extern ScoreRatings rating[];
extern WinLose win_criteria[];
extern WinLose lose_criteria[];
extern WinLoseNet net_criteria[];
extern InsuranceCo insurance_co[];
extern u16 ins_control[];
extern s16 choice_time_out;

void evaluate_reputation(const u8 treatment, const u8 owner);
s32 evaluate_criteria(const u8 owner);
u32 check_criteria_values(const u32 mode, const u8 owner);
s32 check_criteria(u16 mode, u8 owner);
u8 check_net_criteria(const u8 owner);
void setup_next_net_criteria(const u8 owner);
void try_bonus_level(void);

// Applies reputation event `event` (0-25) to player `owner`'s hospital, for
// treatment (illness index, 0 for the hospital as a whole): each event adds a
// fixed amount or a percentage of allhosps.rep_events to the reputation totals
// (amount scales events 13, 23 and 24), and event 22 counts a case. Hospitals
// on the map (allhosps.player_type 1) update their reputation now; computer ones (player_type
// 2) bank it in comp_buckets[owner][month].
// FUNCTION: 0x00058370
void reputation_affected(const u8 treatment, const u16 event, const u16 month, const u8 owner, const s32 amount)
{
    s32 delta;
    s32 count;
    AllHosps *hosp;

    delta = 0;
    count = 0;
    hosp = &allhosps[owner];
    switch (event) {
    case REP_CURED:
    case REP_COMPUTER_DIAGNOSED:
    case REP_ILLNESS_FOUND:
        delta = 1;
        break;
    case REP_DIED:
    case 25:
        delta = -1;
        break;
    case REP_CASE:
        count = 1;
        break;
    case REP_GO_FOR_TREATMENT:
    case REP_DIAGNOSED:
    case REP_NO_TREATMENT_FOUND:
    case REP_NEXT_TREATMENT:
        break;
    case REP_DIAGNOSIS_FAILED:
        delta = -(hosp->rep_events * 5) / 100;
        break;
    case REP_RESEARCHED:
        if (treatment)
            delta = hosp->rooms[treatment].rep_events * 5 / 100;
        else
            delta = hosp->rep_events * 5 / 100;
        break;
    case REP_MACHINE_EXPLODED:
        delta = -(hosp->rep_events * 10) / 100;
        break;
    case REP_EMERGENCY_SUCCESS:
        delta = hosp->rep_events * 2 / 100;
        break;
    case REP_EMERGENCY_FAILED:
        delta = -(hosp->rep_events * (amount * 4)) / 100;
        break;
    case REP_EPIDEMIC_DECLARED:
        delta = -(hosp->rep_events * 2) / 100;
        break;
    case REP_EPIDEMIC_COVER_UP_FOUND:
        delta = -(hosp->rep_events * 10) / 100;
        break;
    case REP_VIP_GOOD:
        delta = hosp->rep_events * 4 / 100;
        break;
    case REP_VIP_FAIR:
        delta = hosp->rep_events * 2 / 100;
        break;
    case REP_VIP_BAD:
        delta = -(hosp->rep_events * 4) / 100;
        break;
    case REP_AUTOPSY:
        delta = -(hosp->rep_events * 20) / 100;
        break;
    case 20:
        delta = -1;
        break;
    case REP_TROPHY:
    case REP_AWARD:
        delta += amount * hosp->rep_events / 100;
        break;
    case 1:
    case 8:
    default:
        break;
    }
    if (hosp->player_type == 1) {
        if (treatment) {
            hosp->rooms[treatment].rep_points += delta;
            hosp->rooms[treatment].rep_events += count;
            evaluate_reputation(treatment, owner);
        }
        hosp->rep_points += delta;
        hosp->rep_events += count;
        evaluate_reputation(0, owner);
    } else if (hosp->player_type == 2) {
        ComputerBucket *bucket;

        bucket = &comp_buckets[owner][month];
        if (treatment) {
            bucket->room_rep_points[treatment] += delta;
            bucket->room_rep_events[treatment] += count;
        }
        bucket->rep_points += delta;
        bucket->rep_events += count;
    }
    if (treatment)
        update_single_cure_cost(treatment, owner);
}

// Recomputes player `owner`'s reputation (0-1000) for treatment, or the
// hospital's overall reputation (allhosps.reputation) for treatment 0, from its
// running totals; 500 when nothing is counted yet.
// FUNCTION: 0x00058730
void evaluate_reputation(const u8 treatment, const u8 owner)
{
    AllHosps *hosp;

    hosp = &allhosps[owner];
    if (treatment) {
        if (hosp->rooms[treatment].rep_events > 0)
            hosp->rooms[treatment].reputation =
                hosp->rooms[treatment].rep_points * 500 / hosp->rooms[treatment].rep_events;
        else
            hosp->rooms[treatment].reputation = 500;
        if (hosp->rooms[treatment].reputation < 0)
            hosp->rooms[treatment].reputation = 0;
        else if (hosp->rooms[treatment].reputation > 1000)
            hosp->rooms[treatment].reputation = 1000;
    } else {
        if (hosp->rep_events > 0)
            hosp->reputation = hosp->rep_points * 500 / hosp->rep_events;
        else
            hosp->reputation = 500;
        if (hosp->reputation < 0)
            hosp->reputation = 0;
        else if (hosp->reputation > 1000)
            hosp->reputation = 1000;
    }
}

// Grows player `owner`'s score (allhosps.score) by the rating[] rate for
// how close they are to winning, capped at 200,000,000.
// FUNCTION: 0x000588a8
void calculate_score(u8 owner)
{
    AllHosps *hosp;
    s32 score;
    s32 i;

    hosp = &allhosps[owner];
    score = evaluate_criteria(owner);
    for (i = 0; rating[i].threshold; i++) {
        if (rating[i].threshold >= score) {
            if (hosp->score > 1000000)
                hosp->score += rating[i].capped_growth * hosp->score / 10000;
            else if (gbv.score_max_inc * hosp->start_score / 100 < hosp->score)
                hosp->score += rating[i].capped_growth * hosp->score / 10000;
            else
                hosp->score += rating[i].growth * hosp->score / 10000;
            break;
        }
    }
    if (hosp->score > 200000000)
        hosp->score = 200000000;
}

// Single player: ends the level if the player has met the win criteria
// (winlose state 1, with the final score) or a lose criterion (state 2).
// Network game: checks the current criterion.
// FUNCTION: 0x000589f0
void check_game_over(void)
{
    s32 score;
    s32 pct;

    if (statvar.num_players > 1) {
        check_net_criteria(player);
    } else {
        score = evaluate_criteria(player);
        if (score >= 100) {
            othervar.level_won = 1;
            pct = myhosp->win_progress / 2;
            pct += old_normal_rand(0x37, 0x3c);
            othervar.next_level_offer = myhosp->score * pct / 100;
            if (othervar.next_level_offer > 200000000)
                othervar.next_level_offer = 200000000;
            year_end_type = 5;
            othervar.winlose_result = 1;
            winlose.state = othervar.winlose_result;
            othervar.win_declined = 0;
            try_bonus_level();
        } else if (check_criteria(1, player)) {
            year_end_type = 7;
            othervar.winlose_result = 2;
            winlose.state = othervar.winlose_result;
        }
    }
}

// Single player, levels before 12: offers the win as soon as the criteria
// are met (saving the screen in progress to return to). Network game:
// checks the current criterion.
// Saving the fax copies 10 bytes from choice_state on, two past the end of
// year_end_backup (into numbuttons and numtext).
// FUNCTION: 0x00058b00
void check_quarterly_game_over(void)
{
    s32 pct;

    if (statvar.num_players > 1) {
        check_net_criteria(player);
    } else if (myhosp->level < 12) {
        if (evaluate_criteria(player) < 100)
            return;
        if (!othervar.win_declined) {
            othervar.level_won = 1;
            pct = myhosp->win_progress / 2;
            pct += old_normal_rand(0x37, 0x3c);
            othervar.next_level_offer = myhosp->score * pct / 100;
            if (othervar.next_level_offer > 200000000)
                othervar.next_level_offer = 200000000;
            if (display_mode == DM_CHOICE) {
                lib_memcpy(&year_end_backup.choice_state, &choice, 10);
                year_end_backup.mode = display_mode;
                year_end_backup.spare_data = choice_spare_data;
                year_end_backup.time_out = choice_time_out;
            } else {
                lib_memcpy(&year_end_backup, &choice_backup, 10);
            }
            year_end_type = 5;
            display_mode = DM_YEAR_END_WINLOSE;
            winlose.state = 1;
            try_bonus_level();
        }
        othervar.win_declined = 0;
    }
}

// Returns 1 if patient will pay for their treatment (and the next, if
// any) at the hospital's price, given its maximum price, their insurance or
// cash and a random allowance. If not, marks them PF_TOO_EXPENSIVE with its
// icon and records them.
// FUNCTION: 0x00058c54
u16 shall_peep_pay(Thing *patient)
{
    u16 treatment;
    u16 pays;
    u16 attempt;
    u16 ins;
    u16 price;
    s32 max;

    treatment = patient->treatment;
    advisor.unpaid_treatment_u = patient->treatment;
    for (attempt = 0; attempt < 2; attempt++) {
        pays = 0;
        update_single_cure_cost(treatment, patient->owner);
        price = allhosps[patient->owner].rooms[treatment].price;
        max = max_price_for_treatment(treatment, patient->owner);
        ins = patient->insurance & allhosps[patient->owner].insurance_accepted;
        if (ins)
            max += insurance_co[ins_control[ins]].extra_pay_pct * max / 100;
        else if (price > patient->cash)
            max = 0;
        max += old_newrand() % (price / 2 + 1);
        if (price <= max)
            pays = 1;
        treatment = patient->next_treatment;
        if (treatment == 0 || pays == 0)
            break;
    }
    if (!pays) {
        set_icon_value(patient, ICON_TOO_EXPENSIVE);
        patient->flags2 |= (PF_TOO_EXPENSIVE >> 16);
        record_person_as_patient(patient);
        if (patient->owner == player)
            advisor.event_flags |= 2;
    }
    return pays;
}

// Returns the most patients will pay player `owner` for treatment: the
// base price scaled by the reputation for it (or the hospital's), plus the
// base again in a network game when no rival offers it; never below the
// base.
// FUNCTION: 0x00058e34
s32 max_price_for_treatment(const u16 treatment, const u8 owner)
{
    u8 monopoly;
    u16 base;
    s32 rep;
    u16 i;
    s32 price;

    monopoly = 0;
    base = expertise[treatment].start_price;
    if (treatment >= 0x24 || treatment == ILL_GENERAL_PRACTICE)
        rep = allhosps[owner].reputation;
    else
        rep = allhosps[owner].rooms[treatment].reputation;
    if (statvar.num_players > 1) {
        monopoly = 1;
        for (i = 0; i < 4; i++) {
            if (i != owner && allhosps[i].rooms[treatment].cure_available == 1)
                monopoly = 0;
        }
    }
    price = base * rep / 500 + monopoly * base;
    if (base > price)
        return base;
    return price;
}

// Sets player `owner`'s overall price level (allhosps.price_level, 0-300%) from
// their prices against the maximums for their known treatments.
// FUNCTION: 0x00058f7c
void calc_pricing_factor(const u8 owner)
{
    u32 max_total;
    s32 price_total;
    u32 i;

    max_total = 0;
    price_total = 0;
    for (i = 0; i < 0x2f; i++) {
        if (allhosps[owner].rooms[i].cure_available == 1) {
            max_total += max_price_for_treatment(i, owner);
            price_total += allhosps[owner].rooms[i].price;
        }
    }
    if (max_total > 0)
        allhosps[owner].price_level = price_total * 100 / max_total;
    else
        allhosps[owner].price_level = 100;
    if (allhosps[owner].price_level < 0)
        allhosps[owner].price_level = 0;
    else if (allhosps[owner].price_level > 300)
        allhosps[owner].price_level = 300;
}

// Returns the first group of win (mode 0) or lose (mode 1) criteria whose
// conditions are all met for player `owner`, or 0.
// FUNCTION: 0x000590e0
s32 check_criteria(u16 mode, u8 owner)
{
    s32 count;
    s32 result;
    WinLose *table;
    s32 crit;
    s32 all;
    s32 any;
    s32 i;

    count = 0;
    result = 0;
    count = check_criteria_values(mode, owner);
    if (mode == 0)
        table = win_criteria;
    else
        table = lose_criteria;
    for (crit = 1; crit <= count; crit++) {
        all = 1;
        any = 0;
        for (i = 0; i < 10; i++) {
            if (table[i].group == crit) {
                any = 1;
                if (table[i].met == 0) {
                    all = 0;
                    break;
                }
            }
        }
        if (all && any) {
            result = crit;
            break;
        }
    }
    return result;
}

// Evaluates every win (mode 0) or lose (mode 1) criterion for player
// `owner`: its current value (current), whether it is met (met) and its progress
// (progress, 0-200%). Returns the highest criteria group number, 0 for owner
// 0xff.
// FUNCTION: 0x000591d4
u32 check_criteria_values(const u32 mode, const u8 owner)
{
    AllHosps *hosp;
    s32 max;
    WinLose *table;

    s32 i;
    s32 val;
    s32 pct;
    s32 range;

    hosp = &allhosps[owner];
    max = 0;
    if (owner == 0xff)
        return 0;
    if (mode == 0)
        table = win_criteria;
    else
        table = lose_criteria;
    for (i = 0; i < 10; i++) {
        table[i].met = 0;
        if (table[i].group > max)
            max = table[i].group;
        switch (table[i].criterion) {
        case 0:
            table[i].met = 1;
            val = 0;
            break;
        case 1:
            val = hosp->reputation;
            break;
        case 2:
            if (mode == 0)
                val = hosp->balance - hosp->loan;
            else
                val = hosp->balance;
            break;
        case 3:
            if (hosp->visitors > 10)
                val = hosp->diagnosed * 100 / hosp->visitors;
            else
                val = 0;
            break;
        case 4:
            val = hosp->cures;
            break;
        case 5:
            if (hosp->visitors > 10)
                val = hosp->deaths * 100 / hosp->visitors;
            else
                val = 0;
            break;
        case 6:
            val = hosp->hospital_value;
            break;
        }
        if (table[i].op == 1) {
            if (val >= table[i].value)
                table[i].met = 1;
        } else if (val <= table[i].value) {
            table[i].met = 1;
        }
        table[i].progress = pct = 0;
        range = table[i].value - table[i].base;
        if (range != 0)
            pct = (val - table[i].base) * 100 / range;
        if (pct < 0)
            pct = 0;
        else if (pct > 200)
            pct = 200;
        table[i].progress = pct;
        table[i].current = val;
    }
    return max;
}

#define ABS(x) ((x) < 0 ? -(x) : (x))

// Returns player `owner`'s progress towards winning (percent, the best of
// the win criteria groups; 100 = won), stores it in allhosps.win_progress, the
// average over the groups in adcounts.win_progress, and for this player the
// best group in othervar.best_win_group.
// FUNCTION: 0x000594d0
s32 evaluate_criteria(const u8 owner)
{
    AllHosps *hosp;
    s32 best;
    s32 best_group;
    s32 total;
    s32 groups;
    s32 group;
    s32 count;
    s32 sum;
    s32 i;
    s32 pct;

    hosp = &allhosps[owner];
    best = 0;
    best_group = 0;
    total = 0;
    groups = check_criteria_values(0, owner);
    for (group = 1; group <= groups; group++) {
        count = 0;
        sum = 0;
        for (i = 0; i < 10; i++) {
            if (win_criteria[i].group != group)
                continue;
            count++;
            if (win_criteria[i].met == 1) {
                sum += 100;
                continue;
            }
            if (win_criteria[i].op == 1) {
                if (win_criteria[i].value != 0) {
                    pct = ABS(win_criteria[i].current * 100 / win_criteria[i].value);
                    if (win_criteria[i].criterion == 2 && win_criteria[i].current < 0)
                        pct = 0;
                } else {
                    pct = 0;
                }
            } else if (win_criteria[i].current != 0) {
                pct = ABS(win_criteria[i].value * 100 / win_criteria[i].current);
            } else {
                pct = 0;
            }
            if (othervar.level_won == 0) {
                if (pct < 0)
                    pct = 0;
                else if (pct > 100)
                    pct = 100;
            } else {
                if (pct < 0)
                    pct = 0;
                else if (pct > 200)
                    pct = 200;
            }
            sum += pct;
        }
        if (sum / count > best) {
            best = sum / count;
            best_group = group;
        }
        total += sum / count;
    }
    if (total > 0 && groups > 0)
        total /= groups;
    else
        total = 0;
    adcounts.win_progress = total;
    hosp->win_progress = best;
    if (owner == player)
        othervar.best_win_group = best_group;
    return best;
}

// Network game: once the current criterion's time is up, checks player
// `owner` against it and sets up the next one, shares the result, and ends
// the game for this player when they win or lose. Returns 1 if they failed
// the criterion (0 when the local player's game ends here).
// FUNCTION: 0x000597c4
u8 check_net_criteria(const u8 owner)
{
    s32 i;
    s32 val;
    AllHosps *hosp;
    u8 kind;
    u8 fail;
    u8 better;
    u8 active;

    hosp = &allhosps[owner];
    kind = 0;
    fail = 0;
    better = 0;
    active = 0;
    if (current_net_criteria.criterion) {
        if (othervar.game_month >= current_net_criteria.start) {
            active = 1;
            switch (current_net_criteria.criterion) {
            case 0:
                current_net_criteria.done = 1;
                active = 0;
                break;
            case 1:
            case 2:
            case 3:
                kind = 4;
                break;
            case 4:
                val = hosp->cures - current_net_criteria.time_to_do;
                kind = 1;
                break;
            case 5:
                val = hosp->balance - hosp->loan;
                kind = 1;
                break;
            case 6:
                val = hosp->hospital_value;
                kind = 1;
                break;
            case 7:
                val = hosp->reputation;
                kind = 1;
                break;
            case 8:
                for (i = 0; i < 4; i++) {
                    if (player_info[i].active) {
                        if (i != player && hosp->reputation >= allhosps[i].reputation)
                            better = 1;
                    }
                }
                kind = 3;
                break;
            case 9:
                for (i = 0; i < 4; i++) {
                    if (player_info[i].active) {
                        if (i != player && hosp->balance - hosp->loan >= allhosps[i].balance - allhosps[i].loan)
                            better = 1;
                    }
                }
                kind = 3;
                break;
            }
            switch (kind) {
            case 1:
                if (val < current_net_criteria.value)
                    fail = 1;
                break;
            case 2:
                if (val > current_net_criteria.value)
                    fail = 1;
                break;
            case 3:
                if (better == 0)
                    fail = 1;
                break;
            case 4:
                if (current_net_criteria.done == 0)
                    fail = 1;
                break;
            }
            setup_next_net_criteria(owner);
        }
    } else {
        setup_next_net_criteria(owner);
    }
    swap_criteria_packets(fail, active);
    if (owner == player) {
        if (evaluate_criteria(owner) >= 100) {
            player_events[player].type = EV_NET_WIN;
            othervar.net_result = 2;
            othervar.net_won = 1;
            return 0;
        }
        if (check_criteria(1, owner)) {
            player_events[player].type = EV_LEAVE_NET_GAME;
            player_events[player].value = 1;
            othervar.net_result = 1;
            return 0;
        }
    }
    return fail;
}

// Network game, this player only: makes the next criterion whose start
// month has come the current one.
// FUNCTION: 0x00059af8
void setup_next_net_criteria(const u8 owner)
{
    u8 i;

    if (statvar.num_players > 1 && owner == player) {
        current_net_criteria.criterion = 0;
        for (i = 0; i < 20; i++) {
            if (net_criteria[i].done == 0 && net_criteria[i].criterion) {
                if (othervar.game_month >= net_criteria[i].start) {
                    net_criteria[i].done = 1;
                    current_net_criteria.criterion = net_criteria[i].criterion;
                    current_net_criteria.value = net_criteria[i].value;
                    current_net_criteria.start = othervar.game_month + net_criteria[i].time_to_do;
                    if (current_net_criteria.criterion == 4)
                        current_net_criteria.time_to_do = allhosps[owner].cures;
                    current_net_criteria.done = 0;
                    othervar.show_net_criteria = 1;
                    break;
                }
            }
        }
    }
}

// Network game: marks the current criterion met if it is `criterion` and
// owner is this player.
// FUNCTION: 0x00059c38
void satisfied_net_criteria(const u8 criterion, const u8 owner)
{
    if (statvar.num_players > 1 && owner == player) {
        if (criterion == current_net_criteria.criterion)
            current_net_criteria.done = 1;
    }
}

// Returns the index of the last entry of table (10 criteria) for criterion,
// or 0 if there is none (indistinguishable from entry 0).
// FUNCTION: 0x00059c88
u8 is_relevant_criterion(const u8 criterion, const WinLose *const table)
{
    s8 i;

    for (i = 9; i >= 0; i--) {
        if (table[i].criterion == criterion)
            return i;
    }
    return 0;
}

// Does nothing.
// FUNCTION: 0x00059ce0
void force_win_lose_criteria(const u32 mode, const u8 owner)
{
}
