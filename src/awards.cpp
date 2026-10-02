#include "hosp.h"

u8 year_check(s32 *values, const s32 limit, const u8 above);
void set_trophy_value(const u8 trophy_no, const s16 value, u8 type);
void general_award(s32 *field, s32 value, const u8 award, const s32 high, const s32 low, const u8 size,
                   const u8 is_signed, const u8 type, const s16 good, const s16 bad);

// Year end: decides the player's 13 awards (all_awards, cleared first) from
// this year's figures, the monthly award_counts and the level's thresholds
// (awards_trophies), paying their money or reputation. Gives none unless the
// year's history[4] total exceeds three times myhosp->level.
// FUNCTION: 0x000b3430
void check_awards_won(void)
{
    s8 i;
    s32 a;
    s32 cures;
    s32 unused;

    i = 12;
    a = 0;
    cures = 0;
    unused = 0;
    LbMemorySet(all_awards, 0, 0x4e);
    while (--i >= 0) {
        a += mycontrol->history[4][0][i];
        cures += mycontrol->history[6][0][i];
    }
    if (myhosp->level * 3 < a) {
        for (i = 0; i < 13; i++) {
            switch (i) {
            case 0:
                if (myhosp->level * 2 <= cures)
                    general_award(&myhosp->reputation, myhosp->reputation, 0, awards_trophies.reputation_award,
                                  awards_trophies.reputation_poor, 4, 0, 2, awards_trophies.award_reputation_bonus,
                                  awards_trophies.award_reputation_penalty);
                break;
            case 1:
                if (myhosp->visitors > myhosp->year_start_visitors)
                    general_award((s32 *)&myhosp->cures, myhosp->cures, 1,
                                  awards_trophies.cures_award * (myhosp->visitors - myhosp->year_start_visitors),
                                  awards_trophies.cures_poor * (myhosp->visitors - myhosp->year_start_visitors), 2, 0,
                                  2, awards_trophies.cures_bonus, awards_trophies.cures_penalty);
                break;
            case 2:
                if (myhosp->visitors > myhosp->year_start_visitors && myhosp->cures)
                    general_award((s32 *)&myhosp->deaths, myhosp->deaths, 2,
                                  awards_trophies.deaths_award * (myhosp->visitors - myhosp->year_start_visitors),
                                  awards_trophies.deaths_poor * (myhosp->visitors - myhosp->year_start_visitors), 2, 0,
                                  2, awards_trophies.deaths_bonus, awards_trophies.deaths_penalty);
                break;
            case 3:
                if (myhosp->level * 2 <= cures)
                    general_award((s32 *)&myhosp->market_share, myhosp->market_share, 3,
                                  awards_trophies.population_percentage_award,
                                  awards_trophies.population_percentage_poor, 2, 0, 1,
                                  awards_trophies.population_percentage_bonus, awards_trophies.population_pct_penalty);
                break;
            case 4:
                if (myhosp->deaths > 0) {
                    a = myhosp->cures * 0x6400 / myhosp->deaths;
                    if (awards_trophies.cures_v_deaths_award << 8 < a)
                        set_award_value(i, 1, 0, awards_trophies.cures_v_deaths_bonus, 2);
                    else if (awards_trophies.cures_v_deaths_poor << 8 > a)
                        set_award_value(i, -1, 0, awards_trophies.cures_v_deaths_penalty, 2);
                } else if (myhosp->visitors > myhosp->year_start_visitors && awards_trophies.cures_bonus <= cures) {
                    set_award_value(i, 1, 0, awards_trophies.cures_v_deaths_bonus, 2);
                }
                break;
            case 5:
                if (year_check(award_counts.peep_happiness, awards_trophies.peep_happiness_award, 1))
                    set_award_value(i, 1, 0, awards_trophies.bonus_penalty[6], 1);
                else if (year_check(award_counts.peep_happiness, awards_trophies.peep_happiness_poor, 0))
                    set_award_value(i, -1, 0, awards_trophies.bonus_penalty[7], 1);
                break;
            case 6:
                general_award(&myhosp->hospital_value, myhosp->hospital_value, 6, awards_trophies.hosp_value_award,
                              awards_trophies.hosp_value_poor, 4, 0, 1, awards_trophies.hosp_value_bonus,
                              awards_trophies.hosp_value_penalty);
                break;
            case 7:
                if (year_check(award_counts.staff_happiness, awards_trophies.staff_happiness_award, 1))
                    set_award_value(i, 1, 0, awards_trophies.bonus_penalty[4], 1);
                else if (year_check(award_counts.staff_happiness, awards_trophies.staff_happiness_poor, 0))
                    set_award_value(i, -1, 0, awards_trophies.bonus_penalty[5], 1);
                break;
            case 8:
                if (myhosp->area > 0) {
                    a = (myhosp->litter_count + myhosp->rathole_count * 2 << 14) / myhosp->area;
                    if ((awards_trophies.cleanliness_award << 14) / 100 > a)
                        set_award_value(i, 1, 0, awards_trophies.bonus_penalty[0], 1);
                    else if (0x4000 / awards_trophies.cleanliness_poor < a)
                        set_award_value(i, -1, 0, awards_trophies.bonus_penalty[1], 1);
                }
                break;
            case 9:
                if (year_check(award_counts.waiting_times, awards_trophies.waiting_times_award, 0))
                    set_award_value(i, 1, 0, awards_trophies.bonus_penalty[8], 1);
                else if (year_check(award_counts.waiting_times, awards_trophies.waiting_times_poor, 1))
                    set_award_value(i, -1, 0, awards_trophies.bonus_penalty[9], 1);
                break;
            case 10:
                if (award_counts.emergencies > 2) {
                    if (awards_trophies.emergency_award < award_counts.emergency_saved_pct)
                        set_award_value(i, 1, 0, awards_trophies.bonus_penalty[2], 1);
                    else if (awards_trophies.emergency_poor > award_counts.emergency_saved_pct)
                        set_award_value(i, -1, 0, awards_trophies.bonus_penalty[3], 1);
                }
                break;
            case 11:
                if (award_counts.research_spent > 100) {
                    if (award_counts.research_spent > awards_trophies.new_tech_award)
                        set_award_value(i, 1, 0, awards_trophies.bonus_penalty[12], 1);
                    else if (award_counts.research_spent < awards_trophies.new_tech_poor)
                        set_award_value(i, -1, 0, awards_trophies.bonus_penalty[13], 1);
                }
                break;
            case 12:
                if (year_check(award_counts.machines, 1, 1) &&
                    year_check(award_counts.machine_wear, awards_trophies.well_kept_tech_award, 0))
                    set_award_value(i, 1, 0, awards_trophies.bonus_penalty[10], 1);
                else if (award_counts.machine_exploded ||
                         (year_check(award_counts.machines, 1, 1) &&
                          year_check(award_counts.machine_wear, awards_trophies.well_kept_tech_poor, 1)))
                    set_award_value(i, -1, 0, awards_trophies.bonus_penalty[11], 1);
                break;
            default:
                break;
            }
        }
    }
}

// Year end: decides the player's 9 trophies (trophy, cleared first) from this
// year's figures and the level's thresholds (awards_trophies), paying their
// bonuses. Gives none unless the year's history[4] total exceeds 5.
// FUNCTION: 0x000b3ce4
void check_trophies_won(void)
{
    s8 i;
    s32 visitors;

    i = 12;
    visitors = 0;
    LbMemorySet(trophy, 0, 0x2d);
    while (--i >= 0)
        visitors += mycontrol->history[4][0][i];
    if (visitors > 5) {
        for (i = 0; i < 9; i++) {
            switch (i) {
            case 0:
                if (myhosp->visitors > myhosp->year_start_visitors) {
                    if ((myhosp->cures - mycontrol->year_start_cures << 8) /
                            (myhosp->visitors - myhosp->year_start_visitors) >
                        0xf3) {
                        set_trophy_value(i, awards_trophies.trophy_cures_bonus, 2);
                        change_balance(TX_TROPHY_BONUS, (u16)awards_trophies.trophy_cures_bonus, 0, player);
                    }
                } else if (myhosp->visitors == myhosp->cures) {
                    set_trophy_value(i, awards_trophies.trophy_cures_bonus, 2);
                    change_balance(TX_TROPHY_BONUS, (u16)awards_trophies.trophy_cures_bonus * 2, 0, player);
                }
                break;
            case 1:
                if (year_check(mycontrol->history[7][0], awards_trophies.reputation, 1))
                    set_trophy_value(i, awards_trophies.trophy_reputation_bonus, 2);
                break;
            case 2:
                if (year_check(award_counts.staff_happiness, awards_trophies.trophy_staff_happiness, 1))
                    set_trophy_value(i, awards_trophies.trophy_staff_happiness_bonus, 1);
                break;
            case 3:
                if (award_counts.mayor_visits > 1) {
                    if ((award_counts.mayor_score_total << 8) / award_counts.mayor_visits < awards_trophies.trophy_mayor
                                                                                                << 8)
                        set_trophy_value(i, awards_trophies.trophy_mayor_bonus, 1);
                }
                break;
            case 4:
                if (gamevar.prev_year_deaths == myhosp->deaths)
                    set_trophy_value(i, awards_trophies.trophy_death_bonus, 2);
                gamevar.prev_year_deaths = myhosp->deaths;
                break;
            case 5:
                if (awards_trophies.rat_kills_absolute < award_counts.rats_killed) {
                    set_trophy_value(i, awards_trophies.rat_kills_absolute_bonus, 1);
                    othervar.plaque_rat_kills = award_counts.rats_killed;
                }
                break;
            case 6:
                if (award_counts.rat_shots > 0 && awards_trophies.rat_kills_absolute / 2 < award_counts.rats_killed) {
                    if ((award_counts.rats_killed << 8) / award_counts.rat_shots >
                        (awards_trophies.rat_kills_percentage << 8) / 100) {
                        set_trophy_value(i, awards_trophies.rat_kills_percentage_bonus, 2);
                        othervar.plaque_rat_kill_pct = award_counts.rats_killed * 100 / award_counts.rat_shots;
                    }
                }
                break;
            case 7:
                if (year_check(award_counts.plant_health, awards_trophies.plant << 14, 1))
                    set_trophy_value(i, awards_trophies.plant_bonus, 1);
                break;
            case 8:
                if (award_counts.cans_of_coke > awards_trophies.cans_of_coke)
                    set_trophy_value(i, awards_trophies.cans_of_coke_bonus, 2);
                break;
            default:
                break;
            }
        }
    }
}

// Returns 1 if all 12 monthly values are at least limit (above nonzero) or at
// most limit (above zero), else 0.
// FUNCTION: 0x000b40ec
u8 year_check(s32 *values, const s32 limit, const u8 above)
{
    s8 i;

    i = 0;
    if (above) {
        for (; i < 12; i++) {
            if (values[i] < limit)
                return 0;
        }
    } else {
        for (; i < 12; i++) {
            if (values[i] > limit)
                return 0;
        }
    }
    return 1;
}

// Month end: stores this month's ratios (from adcounts and NRAdcounts) in the
// award_counts arrays at index mycontrol->month, for year_check.
// FUNCTION: 0x000b4170
void monthly_update_of_award_counts(void)
{
    if (myhosp->staff_count > 3)
        award_counts.staff_happiness[mycontrol->month] = (adcounts.staff_happiness_sum_s << 8) / myhosp->staff_count;
    else
        award_counts.staff_happiness[mycontrol->month] = 0;
    if (adcounts.patients > 3) {
        award_counts.peep_happiness[mycontrol->month] = (adcounts.happiness_sum << 8) / adcounts.patients;
        award_counts.waiting_times[mycontrol->month] = (adcounts.waiting_sum << 8) / adcounts.patients;
    } else {
        award_counts.peep_happiness[mycontrol->month] = 0;
        award_counts.waiting_times[mycontrol->month] = gbv.leave_max << 8;
    }
    if (NRAdcounts.plants > 7)
        award_counts.plant_health[mycontrol->month] = NRAdcounts.plant_health_sum * 0x190000 / NRAdcounts.plants;
    else
        award_counts.plant_health[mycontrol->month] = 0;
    award_counts.machine_wear[mycontrol->month] = adcounts.machine_wear_sum;
    award_counts.machines[mycontrol->month] = adcounts.machines;
}

// Clears the award_counts totals kept over a year (not the monthly arrays).
// FUNCTION: 0x000b4358
void annual_update_of_award_counts(void)
{
    award_counts.cans_of_coke = 0;
    award_counts.mayor_score_total = 0;
    award_counts.mayor_visits = 0;
    award_counts.emergencies = 0;
    award_counts.emergency_saved_pct = 0;
    award_counts.research_earned = 0;
    award_counts.research_spent = 0;
    award_counts.rats_killed = 0;
    award_counts.rat_shots = 0;
}

// Awards trophy trophy_no: picks one of its three texts at random and pays
// value as reputation (type 1) or money (type 2, TX_TROPHY_BONUS). If the
// reputation cannot change, pays 5000 instead.
// FUNCTION: 0x000b43c0
void set_trophy_value(const u8 trophy_no, const s16 value, u8 type)
{
    trophy[trophy_no].id = trophy_no;
    trophy[trophy_no].text_index = trophy_no * 3 + saferand() % 3 + 1;
    if (type == 1) {
        s32 rep;

        rep = myhosp->reputation;
        reputation_affected(0, REP_TROPHY, 0, player, value);
        if (myhosp->reputation != rep) {
            trophy[trophy_no].value = myhosp->reputation - rep;
        } else {
            change_balance(TX_TROPHY_BONUS, 5000, 0, player);
            trophy[trophy_no].value = 5000;
            trophy[trophy_no].id = 2;
        }
    } else if (type == 2) {
        change_balance(TX_TROPHY_BONUS, value, 0, player);
        trophy[trophy_no].value = value;
    }
    trophy[trophy_no].reward_kind = saferand() % 1 + type - 1;
}

// Records award award as won (dir 1) or lost (dir -1), with text variant tier
// (0-2), and applies value as reputation (type 1) or money (type 2,
// TX_YEAR_END_BONUS). If the reputation cannot change, 2500 is paid or taken
// instead.
// FUNCTION: 0x000b4510
void set_award_value(const u8 award, const s8 dir, const u8 tier, const s16 value, u8 type)
{
    all_awards[award].direction = dir;
    all_awards[award].id = award;
    all_awards[award].text_index = tier + award * 3 + saferand() % 1 + 1;
    if (type == 2) {
        change_balance(TX_YEAR_END_BONUS, value, 0, player);
        all_awards[award].value = value;
    } else if (type == 1) {
        s32 rep;

        rep = myhosp->reputation;
        reputation_affected(0, REP_AWARD, 0, player, value);
        if (myhosp->reputation != rep) {
            all_awards[award].value = myhosp->reputation - rep;
        } else {
            if (dir > 0) {
                change_balance(TX_YEAR_END_BONUS, 2500, 0, player);
                all_awards[award].value = 2500;
            } else if (dir < 0) {
                change_balance(TX_YEAR_END_BONUS, -2500, 0, player);
                all_awards[award].value = -2500;
            }
            type = 2;
        }
    }
    all_awards[award].reward_kind = saferand() % 1 + type - 1;
}

// Gives award award from a statistic compared with the other active hospitals:
// field points at the player's AllHosps field (size bytes, signed if
// is_signed) and value is its value. Above high: good, doubled if no other
// hospital beats it, tripled if it also beats their total. Below low: bad,
// doubled if every other hospital is above it. type as for set_award_value.
// FUNCTION: 0x000b4698
void general_award(s32 *field, s32 value, const u8 award, const s32 high, const s32 low, const u8 size,
                   const u8 is_signed, const u8 type, const s16 good, const s16 bad)
{
    s32 hosp_no;
    s32 total;
    s8 top;
    s8 bottom;
    s8 best;

    total = 0;
    best = 1;
    top = 1;
    bottom = 1;
    hosp_no = 4;
    while (--hosp_no >= 0) {
        if (statvar.num_players > 1 && player_info[hosp_no].active == 0)
            continue;
        if (&allhosps[hosp_no] == myhosp)
            continue;
        if (size == 4) {
            if (is_signed) {
                if (value < *(s32 *)((u8 *)field + hosp_no * 0x1259)) {
                    best = 0;
                    top = 0;
                } else {
                    total += *(s32 *)(hosp_no * 0x1259 + (u32)field);
                    bottom = 0;
                }
            } else {
                if ((u32)value < *(u32 *)((u8 *)field + hosp_no * 0x1259)) {
                    best = 0;
                    top = 0;
                } else {
                    total += *(u32 *)(hosp_no * 0x1259 + (u32)field);
                    bottom = 0;
                }
            }
        } else if (size == 2) {
            if (is_signed) {
                if (*(s16 *)(hosp_no * 0x1259 + (u32)field) > value) {
                    best = 0;
                    top = 0;
                } else {
                    total += *(s16 *)(hosp_no * 0x1259 + (u32)field);
                    bottom = 0;
                }
            } else {
                if (*(u16 *)(hosp_no * 0x1259 + (u32)field) > value) {
                    best = 0;
                    top = 0;
                } else {
                    total += *(u16 *)(hosp_no * 0x1259 + (u32)field);
                    bottom = 0;
                }
            }
        } else if (size == 1) {
            if (is_signed) {
                if (*(s8 *)(hosp_no * 0x1259 + (u32)field) > value) {
                    best = 0;
                    top = 0;
                } else {
                    total += *(s8 *)(hosp_no * 0x1259 + (u32)field);
                    bottom = 0;
                }
            } else {
                if (*(u8 *)(hosp_no * 0x1259 + (u32)field) > value) {
                    best = 0;
                    top = 0;
                } else {
                    total += *(u8 *)(hosp_no * 0x1259 + (u32)field);
                    bottom = 0;
                }
            }
        }
    }
    if (value > high) {
        if (top) {
            if (value > total && !bottom)
                set_award_value(award, 1, 2, good * 3, type);
            else if (top == 1)
                set_award_value(award, 1, 1, good * 2, type);
        } else {
            set_award_value(award, 1, 0, good, type);
        }
    } else if (value < low) {
        if (!bottom)
            set_award_value(award, -1, 0, bad, type);
        else
            set_award_value(award, -1, 1, bad * 2, type);
    }
}
