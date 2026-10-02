#include "hosp.h"

extern u16 ins_control[];
extern InsuranceCo insurance_co[];

void auto_available(void);
void appraise_staff(Thing *employee);
void computer_month(void);
void adjust_monthly_figures(void);
void calc_visitors_this_month(void);
void create_job_market(void);
void swap_month_end_packets(void);
void monthly_update_of_award_counts(void);
void monthly_update_of_adivsor_counts(void);
void calc_pricing_factor(const u8 owner);
void calculate_score(u8 owner);
void computer_year(void);
void swap_year_end_packets(void);
void create_chart_data(void);
void check_trophies_won(void);
void annual_update_of_award_counts(void);
void start_year_end(void);
void check_awards_won(void);
void try_to_generate_emergency(void);
void generate_mayors(void);
void generate_disaster(u8 owner);
void generate_quake(void);
void manage_rats(void);
void computer_day(void);
void do_research(void);
u32 epidemic_status(u8 owner);
void generate_inspector(u8 owner);
void create_a_peep(void);
void rat_fest_control(void);
void computer_action(void);

void process_stat_panel(void);

struct StatItem {
    u8 type;
    u16 x;
    u16 y;
    u8 unknown5[6];
};

extern "C" {
extern Event player_events[];
// demo: banktext
StatItem banktext[] = {
    {1, 32, 8},  {2, 8, 24},  {3, 2, 40},   {4, 11, 40},  {5, 40, 40},
    {6, 50, 40}, {7, 62, 40}, {8, 84, 405}, {2, 25, 405}, {9, 568, 405, {0x30, 0, 0x1e}},
    {0, 0, 0},
};
extern char **stat_text;
extern char **finance_text;
extern char **illness_text;
extern u8 redraw_panel;
void StopSample(u32 source_id, u32 number);
}

// Adds amount (negative to spend) to owner's bank balance as a transaction of
// type (TX_*), capped at 9999999. For the local player also records it in the
// bank statement (merged with an identical type on the same day; arg is the
// illness, or insurance company index, shown with some types) and in the
// month's income, expenditure and wage totals, and warns the advisor when
// money runs low.
// FUNCTION: 0x00050ef0
void change_balance(const u8 type, const s32 amount, const u16 arg, const u16 owner)
{
    u16 count;
    u16 day;
    u16 month;
    u16 i;

    if (owner == player) {
        count = (u16)mycontrol->entry_count;
        day = (u16)mycontrol->day;
        month = (u16)mycontrol->month;
        if (count && mycontrol->entries[count - 1].type == type && mycontrol->entries[count - 1].day == day &&
            mycontrol->entries[count - 1].month == month) {
            mycontrol->entries[count - 1].amount += amount;
        } else {
            mycontrol->entries[count].day = day;
            mycontrol->entries[count].month = month;
            mycontrol->entries[count].type = type;
            mycontrol->entries[count].amount = amount;
            switch (type) {
            case TX_INSURANCE:
                mycontrol->entries[count].extra = ins_control[arg];
                break;
            case TX_TREAT:
            case TX_FINAL_TREAT:
            case TX_CURE_TREATMENT:
            case TX_ADVANCE:
                mycontrol->entries[count].extra = arg;
                break;
            }
            if (count < 0x31) {
                mycontrol->entry_count++;
            } else {
                for (i = 0; i < 40; i++) {
                    if (i < 10)
                        mycontrol->statement_balance += mycontrol->entries[i].amount;
                    mycontrol->entries[i] = mycontrol->entries[i + 10];
                }
                mycontrol->entry_count += -9;
            }
        }
        switch (type) {
        case TX_WAGES:
        case TX_HIRE_STAFF:
            mycontrol->wages_this_month -= amount;
            myhosp->balance += amount;
            break;
        default:
            myhosp->balance += amount;
            break;
        }
        if (type != TX_CHEAT_MONEY) {
            if (amount > 0)
                mycontrol->income_this_month += amount;
            else if (amount < 0)
                mycontrol->expenditure_this_month -= amount;
        }
        if ((towns[myhosp->level].start_cash - myhosp->balance) * 8 > towns[myhosp->level].start_cash)
            advisor.event_flags |= 0x20;
    } else {
        allhosps[owner].balance += amount;
    }
    if (allhosps[owner].balance > 9999999)
        allhosps[owner].balance = 9999999;
}

// With kind 2, charges patient for their current treatment: the fee quoted
// earlier (or the current price), paid by their insurance company (credited
// later) or from their cash. Otherwise just quotes the current price as their
// fee. Returns 0 if they cannot afford it, else 1.
// FUNCTION: 0x00051280
u16 pay_for_treatment(Thing *patient, u16 kind)
{
    HospitalRoom *room;
    s32 price;
    s32 fee;
    u16 ins;
    u16 type;
    u16 i;
    u16 month;

    fee = 0;
    price = allhosps[patient->owner].rooms[patient->treatment].price;
    room = &allhosps[patient->owner].rooms[patient->treatment];
    if (kind == 2) {
        if (patient->fee == 0)
            patient->fee = price;
        room->income += patient->fee;
        fee = patient->fee;
        type = TX_TREAT;
        patient->fee = 0;
    } else {
        patient->fee = price;
    }
    if (fee > 0) {
        ins = (u16)patient->insurance;
        if (ins & allhosps[patient->owner].insurance_accepted) {
            for (i = 0; i < 3; i++) {
                if (ins & (1 << i)) {
                    month = mycontrol->month + insurance_co[ins_control[i]].payment_delay;
                    if (month >= 12)
                        month -= 12;
                    if (month >= 12)
                        month = 0;
                    allhosps[patient->owner].insurance_claims[i][month] += fee;
                    create_numfx(NUMFX_MONEY, fee, patient->x.tile + (patient->y.tile << 7), color.red, 0x40);
                    break;
                }
            }
        } else {
            if (fee <= patient->cash) {
                patient->cash -= fee;
                change_balance(type, fee, (u16)patient->treatment, (u16)patient->owner);
                create_numfx(NUMFX_MONEY, fee, patient->x.tile + (patient->y.tile << 7), color.black, 0x40);
            } else {
                return 0;
            }
        }
    }
    return 1;
}

// Month end for every hospital: pays wages (appraising each employee), banks
// drinks and amusement income and the insurance payments due this month,
// charges heating, loan and overdraft interest, updates prices and each
// hospital's share of the month_visitors totals (market_share), the computer players, the monthly
// history and statistics, then moves to the next month and refreshes the job
// market.
// FUNCTION: 0x00051534
void month_end(void)
{
    u16 i;
    u16 j;
    s32 amount;
    s32 total;
    Thing *employee;
    s32 wages[4];

    total = 0;
    mycontrol->day--;
    auto_available();
    for (i = 0; i < 4; i++)
        wages[i] = 0;
    for (employee = thing + 1; employee < end_things; employee++) {
        if (employee->type == THING_STAFF && employee->state != SS_APPLICANT) {
            wages[employee->owner] += employee->salary;
            appraise_staff(employee);
        }
    }
    for (i = 0; i < 4; i++) {
        count_hospital_area(i);
        if (wages[i] != 0)
            change_balance(TX_WAGES, -wages[i], 0, i);
        for (j = 0; j < 2; j++) {
            if (allhosps[i].sales_income[j] != 0) {
                change_balance(j + TX_INCOME_DRINKS, allhosps[i].sales_income[j], 0, i);
                allhosps[i].sales_income[j] = 0;
            }
        }
        for (j = 0; j < 3; j++) {
            amount = allhosps[i].insurance_claims[j][mycontrol->month];
            if (amount != 0) {
                allhosps[i].insurance_claims[j][mycontrol->month] = 0;
                change_balance(TX_INSURANCE, amount, j, i);
            }
        }
        amount = allhosps[i].radiator_count * allhosps[i].heating_level * mycontrol->heating_price / 100;
        if (amount != 0)
            change_balance(TX_HEATING, -amount, 0, i);
        if (allhosps[i].loan != 0)
            change_balance(TX_LOAN_INTEREST, -(towns[allhosps[i].level].interest_rate * allhosps[i].loan / 10000 / 12),
                           0, i);
        if (allhosps[i].balance < 0)
            change_balance(TX_OVERDRAFT_INTEREST,
                           towns[allhosps[i].level].interest_rate * allhosps[i].balance * 2 / 10000 / 12, 0, i);
        calc_pricing_factor(i);
        total += allhosps[i].month_visitors;
    }
    computer_month();
    adjust_monthly_figures();
    for (i = 0; i < 4; i++) {
        if (total != 0)
            allhosps[i].market_share = allhosps[i].month_visitors * 100 / total;
        else
            allhosps[i].market_share = 0;
        allhosps[i].month_visitors = 0;
    }
    calc_visitors_this_month();
    mycontrol->day = 1;
    mycontrol->month++;
    othervar.game_month = mycontrol->year * 12 + mycontrol->month;
    create_job_market();
    swap_month_end_packets();
    monthly_update_of_award_counts();
    monthly_update_of_adivsor_counts();
    update_cure_costs();
}

// Shifts the local player's 12-month history one month and stores this
// month's figures (balance, income, expenditure, wages, deaths, cures,
// reputation), then clears the month's totals.
// FUNCTION: 0x00051a84
void adjust_monthly_figures(void)
{
    u16 i;

    for (i = 0; i < 3; i++) {
        lib_memcpy(mycontrol->history[1][i], &mycontrol->history[1][i][1], 0x2c);
        lib_memcpy(mycontrol->history[2][i], &mycontrol->history[2][i][1], 0x2c);
        lib_memcpy(mycontrol->history[0][i], &mycontrol->history[0][i][1], 0x2c);
        lib_memcpy(mycontrol->history[3][i], &mycontrol->history[3][i][1], 0x2c);
        lib_memcpy(mycontrol->history[4][i], &mycontrol->history[4][i][1], 0x2c);
        lib_memcpy(mycontrol->history[5][i], &mycontrol->history[5][i][1], 0x2c);
        lib_memcpy(mycontrol->history[6][i], &mycontrol->history[6][i][1], 0x2c);
        lib_memcpy(mycontrol->history[7][i], &mycontrol->history[7][i][1], 0x2c);
        mycontrol->history[3][i][11] = myhosp->balance;
        mycontrol->history[1][i][11] = mycontrol->income_this_month;
        mycontrol->history[2][i][11] = mycontrol->expenditure_this_month;
        mycontrol->history[0][i][11] = mycontrol->wages_this_month;
        mycontrol->history[5][i][11] = myhosp->deaths;
        mycontrol->history[6][i][11] = myhosp->cures;
        mycontrol->history[7][i][11] = myhosp->reputation;
    }
    mycontrol->income_this_month = 0;
    mycontrol->expenditure_this_month = 0;
    mycontrol->wages_this_month = 0;
    mycontrol->deaths_this_month = 0;
    mycontrol->cures_this_month = 0;
}

// Year end: values and scores the hospitals, totals the year's income and
// expenditure, moves to the next year, records the year's starting figures,
// then works out the league tables, trophies and awards and opens the
// year-end screens.
// FUNCTION: 0x00051d98
void year_end(void)
{
    u16 i;

    calculate_value(player);
    calculate_score(player);
    for (i = mycontrol->income_last_year = mycontrol->expenditure_last_year = 0; i < 12; i++) {
        mycontrol->income_last_year += mycontrol->history[1][0][i];
        mycontrol->expenditure_last_year += mycontrol->history[2][0][i];
    }
    computer_year();
    mycontrol->month = 0;
    if (++mycontrol->year > 999)
        mycontrol->year = 0;
    othervar.game_month = mycontrol->year * 12 + mycontrol->month;
    mycontrol->entry_count = 0;
    swap_year_end_packets();
    for (i = 0; i < 4; i++) {
        allhosps[i].year_start_balance = allhosps[i].balance;
        allhosps[i].year_start_visitors = allhosps[i].visitors;
    }
    mycontrol->statement_balance = myhosp->year_start_balance;
    mycontrol->year_start_reputation = myhosp->reputation;
    mycontrol->year_start_area = myhosp->area;
    mycontrol->year_start_cures = myhosp->cures;
    mycontrol->year_start_deaths = myhosp->deaths;
    mycontrol->year_start_staff = myhosp->staff_count;
    mycontrol->year_start_staff2 = myhosp->staff_count;
    create_chart_data();
    check_trophies_won();
    annual_update_of_award_counts();
    start_year_end();
    check_awards_won();
    swap_year_end_packets();
    mycontrol->statement_balance = myhosp->year_start_balance;
    mycontrol->year_start_reputation = myhosp->reputation;
}

// Runs one frame of the bank statement screen (bank.state: 0 loads it, 1
// shows it, 2 asks to return to the game): the latest entries that fit, with
// the running balance.
// FUNCTION: 0x00052038
void bank_statement(void)
{
    u16 row;
    u16 line_h;
    u16 char_w;
    u16 i;
    u16 first;
    u16 k;
    s32 balance;
    char buf[100];

    compute_help_text(0, 0, 0xf);
    switch (bank.state) {
    case 0:
        load_ingame_pic(SCR_STATEMENT, 1, 0x1a, 0, (TbScreenMode)0xd);
        bank.state = 1;
        break;
    case 1:
        copy_screen_to_wscreen(bscreen);
        process_stat_panel();
        break;
    case 2:
        player_events[player].type = EV_RETURN_FROM_SCREEN;
        player_events[player].value = MM_NORMAL;
        break;
    }
    set_font_from_cache(0x1c);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0x100;
    for (i = 0; i < 6; i++) {
        LbTextSetWindow(resvars.screen_boxes_Boxp[i + 4].x, resvars.screen_boxes_Boxp[i + 4].y,
                        resvars.screen_boxes_Boxp[i + 4].w, resvars.screen_boxes_Boxp[i + 4].h);
        sprintf(buf, "%s", stat_text[i]);
        LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y * 2, buf);
    }
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    line_h = lbFontPtr ? lbFontPtr[34].height : 0;
    char_w = lbFontPtr ? lbFontPtr[34].width : 0;
    for (i = 0; banktext[i].type; i++) {
        switch (banktext[i].type) {
        case 1:
            sprintf(buf, "%s", myhosp->name + 9);
            LbTextDraw(banktext[i].x, banktext[i].y, buf);
            break;
        case 2:
            break;
        case 3:
            row = 0;
            first = (banktext[7].y - banktext[i].y) / line_h;
            first = mycontrol->entry_count - first > 0 ? mycontrol->entry_count - first : 0;
            balance = mycontrol->statement_balance;
            for (k = 0; k < first; k++)
                balance += mycontrol->entries[k].amount;
            for (; first < mycontrol->entry_count && row < banktext[7].y - line_h; first++) {
                if (row + resvars.screen_coords[72].y > resvars.screen_coords[72].y + line_h * 0x1b)
                    break;
                sprintf(buf, "%d %s", mycontrol->entries[first].day, month_text[mycontrol->entries[first].month]);
                LbTextDraw(resvars.screen_coords[72].x, row + resvars.screen_coords[72].y, buf);
                switch (mycontrol->entries[first].type) {
                case TX_INSURANCE:
                    sprintf(buf, "%s %s", finance_text[mycontrol->entries[first].type],
                            insurance_names[mycontrol->entries[first].extra]);
                    break;
                case TX_TREAT:
                case TX_FINAL_TREAT:
                case TX_CURE_TREATMENT:
                case TX_ADVANCE:
                    sprintf(buf, "%s %s", finance_text[mycontrol->entries[first].type],
                            illness_text[mycontrol->entries[first].extra]);
                    break;
                default:
                    sprintf(buf, "%s", finance_text[mycontrol->entries[first].type]);
                    break;
                }
                LbTextDraw(resvars.screen_coords[73].x, row + resvars.screen_coords[73].y, buf);
                if (mycontrol->entries[first].amount < 0) {
                    sprintf(buf, "$ %7d", -mycontrol->entries[first].amount);
                    LbTextDraw(resvars.screen_coords[74].x, row + resvars.screen_coords[74].y, buf);
                } else if (mycontrol->entries[first].amount != 0) {
                    sprintf(buf, "$ %7d", mycontrol->entries[first].amount);
                    LbTextDraw(resvars.screen_coords[75].x, row + resvars.screen_coords[75].y, buf);
                }
                balance += mycontrol->entries[first].amount;
                sprintf(buf, "$ %7d", balance);
                if (balance < 0)
                    LbTextDraw(resvars.screen_coords[76].x, row + resvars.screen_coords[76].y, buf);
                else
                    LbTextDraw(resvars.screen_coords[76].x, row + resvars.screen_coords[76].y, buf);
                row += line_h;
            }
            break;
        case 8:
            sprintf(buf, "$ %7d", balance);
            if (balance < 0)
                LbTextDraw(resvars.screen_coords[77].x, resvars.screen_coords[77].y, buf);
            else
                LbTextDraw(resvars.screen_coords[77].x, resvars.screen_coords[77].y, buf);
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        default:
            break;
        }
    }
    lbDisplay.draw_flags = 0;
    if (exit_quick_requester())
        bank.state = 2;
}

// Draws pressed and runs the callbacks of the bank statement's buttons under
// the mouse.
// FUNCTION: 0x000528cc
void process_stat_panel(void)
{
    u8 i;

    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.screen_buttons[i].width != 0 && resvars.screen_buttons[i].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_buttons[i].x,
                             resvars.screen_buttons[i].y, resvars.screen_buttons[i].width,
                             resvars.screen_buttons[i].height)) {
                resvars.screen_buttons[i].callback(i);
                reqspr_draw(resvars.screen_buttons[i].sprite_x, resvars.screen_buttons[i].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i].sprite]);
            }
        }
    }
    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.screen_buttons[i].width != 0 && resvars.screen_buttons[i].height != 0; i++) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_buttons[i].x,
                             resvars.screen_buttons[i].y, resvars.screen_buttons[i].width,
                             resvars.screen_buttons[i].height)) {
                reqspr_draw(resvars.screen_buttons[i].sprite_x, resvars.screen_buttons[i].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i].sprite]);
                break;
            }
        }
    }
    lbDisplay.rleft_button = 0;
}

// Bank statement button: 0 closes it, 1 acts as key F7 (process_keys).
// FUNCTION: 0x00052b48
void process_stat_button(u8 button)
{
    switch (button) {
    case 0:
        bank.state = 2;
        play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        break;
    case 1:
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        process_keys(0, KC_F7);
        break;
    }
}

// Returns 1 (consuming the key, with a sound) if Escape or Space was pressed
// to leave a screen, else 0.
// FUNCTION: 0x00052bcc
u16 exit_quick_requester(void)
{
    if (lbInkey >= 0x80)
        lbInkey = 0;
    if (lbInkey == KC_ESCAPE || lbInkey == KC_SPACE) {
        lbInkey = 0;
        play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        return 1;
    }
    return 0;
}

// Daily: advances the day, then runs the emergencies (ending finished ones
// with their bonus or penalty), the mayor's visits, disasters, earthquakes,
// expired calls, rats, the computer players' day and research.
// FUNCTION: 0x00052c44
void daily_admin(void)
{
    s32 i;
    s32 hosp_count;

    mycontrol->day++;
    redraw_panel = 1;
    try_to_generate_emergency();
    generate_mayors();
    hosp_count = 1;
    if (statvar.num_players > 1)
        hosp_count = 4;
    for (i = 0; i < hosp_count; i++) {
        if (statvar.num_players > 1 && player_info[i].active == 0)
            continue;
        if (allhosps[i].emergency_state_s == 2) {
            if (allhosps[i].emergency_present == 0) {
                allhosps[i].emergency_state = 1;
                if (allhosps[i].emergency_cured * 100 / allhosps[i].emergency_patients >=
                    allhosps[i].emergency_win_pct) {
                    change_balance(TX_EMERGENCY_BONUS, allhosps[i].emergency_bonus * allhosps[i].emergency_cured, 0, i);
                    reputation_affected(0, REP_EMERGENCY_SUCCESS, 0, i, 0);
                    satisfied_net_criteria(1, i);
                } else {
                    reputation_affected(0, REP_EMERGENCY_FAILED, 0, i,
                                        allhosps[i].emergency_patients - allhosps[i].emergency_cured);
                }
                make_a_playercall(5, 1, 0x14, i);
            }
        }
        generate_disaster(i);
    }
    generate_quake();
    expire_playercalls();
    manage_rats();
    computer_day();
    do_research();
}

// Twice a day: counts down each hospital's emergency clock (allhosps.emergency_timer),
// ends cover-ups and sends the inspector, then admits new patients, runs the
// rat level and the computer players' actions.
// FUNCTION: 0x00052e70
void twice_daily_admin(void)
{
    u8 hosp_count;
    u8 i;
    AllHosps *hosp;

    hosp = allhosps;
    hosp_count = 1;
    if (statvar.num_players > 1)
        hosp_count = 4;
    for (i = 0; i < hosp_count; i++, hosp++) {
        if (statvar.num_players > 1 && player_info[i].active == 0)
            continue;
        if (hosp->emergency_timer_s > 0) {
            hosp->emergency_timer--;
            if (hosp->emergency_timer_s == 0 && hosp->emergency_state_s > 0)
                hosp->emergency_state--;
            if (myhosp->emergency_timer_s == 0)
                StopSample(SNDSRC_CLOCK, 0xa9);
        }
        if (epidemic_status(i))
            generate_inspector(i);
    }
    create_a_peep();
    rat_fest_control();
    computer_action();
}

// Recalculates owner's hospital value (allhosps.hospital_value: rooms, land,
// reputation, object_value and a base 10000) and from it the loan limit (max_loan, a third
// of it plus 10000, at most 100000).
// FUNCTION: 0x00052f7c
void calculate_value(const u8 owner)
{
    s32 room_value;
    AllHosps *hosp;
    s32 object_value;
    s32 rep_value;
    RoomInfo *room;
    s32 area_value;

    room_value = 0;
    hosp = &allhosps[owner];
    object_value = hosp->object_value;
    rep_value = hosp->reputation / 50 * 1000;
    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->owner == owner)
            room_value += rooms[room->type].cost;
    }
    count_hospital_area((u16)owner);
    area_value = hosp->area * gbv.land_cost_per_tile;
    hosp->hospital_value = area_value + (object_value + rep_value + room_value) + 10000;
    if (hosp->hospital_value < 0)
        hosp->hospital_value = 0;
    hosp->max_loan = hosp->hospital_value / 3 + 10000;
    if (hosp->max_loan < 0)
        hosp->max_loan = 0;
    else if (hosp->max_loan > 100000)
        hosp->max_loan = 100000;
}
