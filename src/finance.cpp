#include "hosp.h"

extern u16 ins_control[];
extern InsuranceCo insurance_co[];

void auto_available(void);
void appraise_staff(Thing *w);
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

struct StatPanelLayout {
    u8 unknown0[0x40];
    struct {
        s32 x;
        s32 y;
        s32 w;
        s32 h;
    } boxes[6];
};

struct StatTextLayout {
    u8 unknown0[0x238];
    Point pos[7];
};

struct StatItem {
    u8 type;
    u16 x;
    u16 y;
    u8 unknown5[6];
};

extern "C" {
extern u8 D_00158324;
extern u16 D_00155d89;
extern u16 D_00188c74;
extern s32 D_00188c38;
extern Event D_001549d0[];
extern StatPanelLayout *D_0015f430;
extern StatTextLayout *D_0015f434;
extern StatItem D_00136388[];
extern char **D_0016032c;
extern char **D_001602b4;
extern char **D_001602a4;
extern char D_0012042c[];
extern char D_0012042f[];
extern char D_00120432[];
extern char D_00120438[];
extern char D_0012043e[];
extern char D_00120444[];
extern char D_00120447[];
extern char D_0012044d[];
extern char D_00120453[];
extern char D_00120459[];
extern volatile s16 D_00188c6c, D_00188c70;
extern u8 D_00188c7e, D_00188c81;
extern Button *D_0015f43c;
extern u8 D_00156910;
void StopSample(u32 a, u32 b);
}

// FUNCTION: 0x00050ef0
void change_balance(const u8 type, const s32 amount, const u16 arg, const u16 owner)
{
    u16 n;
    u16 day;
    u16 month;
    u16 i;

    if (owner == player) {
        n = (u16)mycontrol->f64e;
        day = (u16)mycontrol->unknown64f;
        month = (u16)mycontrol->unknown650;
        if (n && mycontrol->entries[n - 1].type == type && mycontrol->entries[n - 1].day == day
            && mycontrol->entries[n - 1].month == month) {
            mycontrol->entries[n - 1].amount += amount;
        } else {
            mycontrol->entries[n].day = day;
            mycontrol->entries[n].month = month;
            mycontrol->entries[n].type = type;
            mycontrol->entries[n].amount = amount;
            switch (type) {
            case 17:
                mycontrol->entries[n].extra = ins_control[arg];
                break;
            case 7:
            case 8:
            case 9:
            case 11:
                mycontrol->entries[n].extra = arg;
                break;
            }
            if (n < 0x31) {
                mycontrol->f64e++;
            } else {
                for (i = 0; i < 40; i++) {
                    if (i < 10)
                        mycontrol->f4b4 += mycontrol->entries[i].amount;
                    mycontrol->entries[i] = mycontrol->entries[i + 10];
                }
                mycontrol->f64e += -9;
            }
        }
        switch (type) {
        case 1:
        case 2:
            mycontrol->f0 -= amount;
            myhosp->f35 += amount;
            break;
        default:
            myhosp->f35 += amount;
            break;
        }
        if (type != 15) {
            if (amount > 0)
                mycontrol->f4 += amount;
            else if (amount < 0)
                mycontrol->f8 -= amount;
        }
        if ((towns[myhosp->f29].f0 - myhosp->f35) * 8 > towns[myhosp->f29].f0)
            D_00158324 |= 0x20;
    } else {
        allhosps[owner].f35 += amount;
    }
    if (allhosps[owner].f35 > 9999999)
        allhosps[owner].f35 = 9999999;
}

// FUNCTION: 0x00051280
u16 pay_for_treatment(Thing *w, u16 kind)
{
    HospitalRoom *room;
    s32 price;
    s32 fee;
    u16 ins;
    u16 type;
    u16 i;
    u16 month;

    fee = 0;
    price = allhosps[w->owner].rooms[w->unknown171].unknownC;
    room = &allhosps[w->owner].rooms[w->unknown171];
    if (kind == 2) {
        if (w->unknown108 == 0)
            w->unknown108 = price;
        room->unknown14 += w->unknown108;
        fee = w->unknown108;
        type = 7;
        w->unknown108 = 0;
    } else {
        w->unknown108 = price;
    }
    if (fee > 0) {
        ins = (u16)w->unknown172[1];
        if (ins & allhosps[w->owner].f216) {
            for (i = 0; i < 3; i++) {
                if (ins & (1 << i)) {
                    month = mycontrol->unknown650 + insurance_co[ins_control[i]].f1;
                    if (month >= 12)
                        month -= 12;
                    if (month >= 12)
                        month = 0;
                    allhosps[w->owner].f45[i][month] += fee;
                    create_numfx(3, fee, w->x.tile + (w->y.tile << 7), color.unknownD, 0x40);
                    break;
                }
            }
        } else {
            if (fee <= w->f70) {
                w->f70 -= fee;
                change_balance(type, fee, (u16)w->unknown171, (u16)w->owner);
                create_numfx(3, fee, w->x.tile + (w->y.tile << 7), color.unknown8, 0x40);
            } else {
                return 0;
            }
        }
    }
    return 1;
}

// FUNCTION: 0x00051534
void month_end(void)
{
    u16 i;
    u16 j;
    s32 amount;
    s32 total;
    Thing *w;
    s32 wages[4];

    total = 0;
    mycontrol->unknown64f--;
    auto_available();
    for (i = 0; i < 4; i++)
        wages[i] = 0;
    for (w = thing + 1; w < end_things; w++) {
        if (w->renderer == 4 && w->kind != 0x2e) {
            wages[w->owner] += w->unknown108;
            appraise_staff(w);
        }
    }
    for (i = 0; i < 4; i++) {
        count_hospital_area(i);
        if (wages[i] != 0)
            change_balance(1, -wages[i], 0, i);
        for (j = 0; j < 2; j++) {
            if (allhosps[i].fd5s[j] != 0) {
                change_balance(j + 13, allhosps[i].fd5s[j], 0, i);
                allhosps[i].fd5s[j] = 0;
            }
        }
        for (j = 0; j < 3; j++) {
            amount = allhosps[i].f45[j][mycontrol->unknown650];
            if (amount != 0) {
                allhosps[i].f45[j][mycontrol->unknown650] = 0;
                change_balance(17, amount, j, i);
            }
        }
        amount = allhosps[i].fc92 * allhosps[i].fc94 * mycontrol->f64c / 100;
        if (amount != 0)
            change_balance(16, -amount, 0, i);
        if (allhosps[i].f3d != 0)
            change_balance(20, -(towns[allhosps[i].f29].f10 * allhosps[i].f3d / 10000 / 12), 0, i);
        if (allhosps[i].f35 < 0)
            change_balance(23, towns[allhosps[i].f29].f10 * allhosps[i].f35 * 2 / 10000 / 12, 0, i);
        calc_pricing_factor(i);
        total += allhosps[i].f1cd;
    }
    computer_month();
    adjust_monthly_figures();
    for (i = 0; i < 4; i++) {
        if (total != 0)
            allhosps[i].f1cb = allhosps[i].f1cd * 100 / total;
        else
            allhosps[i].f1cb = 0;
        allhosps[i].f1cd = 0;
    }
    calc_visitors_this_month();
    mycontrol->unknown64f = 1;
    mycontrol->unknown650++;
    D_00155d89 = mycontrol->f648 * 12 + mycontrol->unknown650;
    create_job_market();
    swap_month_end_packets();
    monthly_update_of_award_counts();
    monthly_update_of_adivsor_counts();
    update_cure_costs();
}

// FUNCTION: 0x00051a84
void adjust_monthly_figures(void)
{
    u16 i;

    for (i = 0; i < 3; i++) {
        LbDrawBoxCoords(mycontrol->history[1][i], &mycontrol->history[1][i][1], 0x2c);
        LbDrawBoxCoords(mycontrol->history[2][i], &mycontrol->history[2][i][1], 0x2c);
        LbDrawBoxCoords(mycontrol->history[0][i], &mycontrol->history[0][i][1], 0x2c);
        LbDrawBoxCoords(mycontrol->history[3][i], &mycontrol->history[3][i][1], 0x2c);
        LbDrawBoxCoords(mycontrol->history[4][i], &mycontrol->history[4][i][1], 0x2c);
        LbDrawBoxCoords(mycontrol->history[5][i], &mycontrol->history[5][i][1], 0x2c);
        LbDrawBoxCoords(mycontrol->history[6][i], &mycontrol->history[6][i][1], 0x2c);
        LbDrawBoxCoords(mycontrol->history[7][i], &mycontrol->history[7][i][1], 0x2c);
        mycontrol->history[3][i][11] = myhosp->f35;
        mycontrol->history[1][i][11] = mycontrol->f4;
        mycontrol->history[2][i][11] = mycontrol->f8;
        mycontrol->history[0][i][11] = mycontrol->f0;
        mycontrol->history[5][i][11] = myhosp->f1eb;
        mycontrol->history[6][i][11] = myhosp->f1e9;
        mycontrol->history[7][i][11] = myhosp->unknownc7e;
    }
    mycontrol->f4 = 0;
    mycontrol->f8 = 0;
    mycontrol->f0 = 0;
    mycontrol->unknown10 = 0;
    mycontrol->fc = 0;
}

// FUNCTION: 0x00051d98
void year_end(void)
{
    u16 i;

    calculate_value(player);
    calculate_score(player);
    for (i = mycontrol->f14 = mycontrol->f18 = 0; i < 12; i++) {
        mycontrol->f14 += mycontrol->history[1][0][i];
        mycontrol->f18 += mycontrol->history[2][0][i];
    }
    computer_year();
    mycontrol->unknown650 = 0;
    if (++mycontrol->f648 > 999)
        mycontrol->f648 = 0;
    D_00155d89 = mycontrol->f648 * 12 + mycontrol->unknown650;
    mycontrol->f64e = 0;
    swap_year_end_packets();
    for (i = 0; i < 4; i++) {
        allhosps[i].f1c3 = allhosps[i].f35;
        allhosps[i].f1c7 = allhosps[i].f1ed;
    }
    mycontrol->f4b4 = myhosp->f1c3;
    mycontrol->f4a0 = myhosp->unknownc7e;
    mycontrol->f64a = myhosp->f1f1;
    mycontrol->f4a4 = myhosp->f1e9;
    mycontrol->f4a8 = myhosp->f1eb;
    mycontrol->f4ac = myhosp->fc9c;
    mycontrol->f4b0 = myhosp->fc9c;
    create_chart_data();
    check_trophies_won();
    annual_update_of_award_counts();
    start_year_end();
    check_awards_won();
    swap_year_end_packets();
    mycontrol->f4b4 = myhosp->f1c3;
    mycontrol->f4a0 = myhosp->unknownc7e;
}

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
    switch (bank.unknown2) {
    case 0:
        load_ingame_pic(3, 1, 0x1a, 0, (TbScreenMode)0xd);
        bank.unknown2 = 1;
        break;
    case 1:
        copy_screen_to_wscreen(bscreen);
        process_stat_panel();
        break;
    case 2:
        D_001549d0[player].type = 0x70;
        D_001549d0[player].value = 0;
        break;
    }
    set_font_from_cache(0x1c);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0x100;
    for (i = 0; i < 6; i++) {
        LbTextSetWindow(D_0015f430->boxes[i].x, D_0015f430->boxes[i].y, D_0015f430->boxes[i].w,
                        D_0015f430->boxes[i].h);
        sprintf(buf, D_0012042c, D_0016032c[i]);
        LbTextDraw(D_0015f434->pos[0].x, D_0015f434->pos[0].y * 2, buf);
    }
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    line_h = lbFontPtr ? lbFontPtr[34].height : 0;
    char_w = lbFontPtr ? lbFontPtr[34].width : 0;
    for (i = 0; D_00136388[i].type; i++) {
        switch (D_00136388[i].type) {
        case 1:
            sprintf(buf, D_0012042f, myhosp->unknown0 + 9);
            LbTextDraw(D_00136388[i].x, D_00136388[i].y, buf);
            break;
        case 2:
            break;
        case 3:
            row = 0;
            first = (D_00136388[7].y - D_00136388[i].y) / line_h;
            first = mycontrol->f64e - first > 0 ? mycontrol->f64e - first : 0;
            balance = mycontrol->f4b4;
            for (k = 0; k < first; k++)
                balance += mycontrol->entries[k].amount;
            for (; first < mycontrol->f64e && row < D_00136388[7].y - line_h; first++) {
                if (row + D_0015f434->pos[1].y > D_0015f434->pos[1].y + line_h * 0x1b)
                    break;
                sprintf(buf, D_00120432, mycontrol->entries[first].day, month_text[mycontrol->entries[first].month]);
                LbTextDraw(D_0015f434->pos[1].x, row + D_0015f434->pos[1].y, buf);
                switch (mycontrol->entries[first].type) {
                case 17:
                    sprintf(buf, D_00120438, D_001602b4[mycontrol->entries[first].type],
                            insurance_names[mycontrol->entries[first].extra]);
                    break;
                case 7:
                case 8:
                case 9:
                case 11:
                    sprintf(buf, D_0012043e, D_001602b4[mycontrol->entries[first].type],
                            D_001602a4[mycontrol->entries[first].extra]);
                    break;
                default:
                    sprintf(buf, D_00120444, D_001602b4[mycontrol->entries[first].type]);
                    break;
                }
                LbTextDraw(D_0015f434->pos[2].x, row + D_0015f434->pos[2].y, buf);
                if (mycontrol->entries[first].amount < 0) {
                    sprintf(buf, D_00120447, -mycontrol->entries[first].amount);
                    LbTextDraw(D_0015f434->pos[3].x, row + D_0015f434->pos[3].y, buf);
                } else if (mycontrol->entries[first].amount != 0) {
                    sprintf(buf, D_0012044d, mycontrol->entries[first].amount);
                    LbTextDraw(D_0015f434->pos[4].x, row + D_0015f434->pos[4].y, buf);
                }
                balance += mycontrol->entries[first].amount;
                sprintf(buf, D_00120453, balance);
                if (balance < 0)
                    LbTextDraw(D_0015f434->pos[5].x, row + D_0015f434->pos[5].y, buf);
                else
                    LbTextDraw(D_0015f434->pos[5].x, row + D_0015f434->pos[5].y, buf);
                row += line_h;
            }
            break;
        case 8:
            sprintf(buf, D_00120459, balance);
            if (balance < 0)
                LbTextDraw(D_0015f434->pos[6].x, D_0015f434->pos[6].y, buf);
            else
                LbTextDraw(D_0015f434->pos[6].x, D_0015f434->pos[6].y, buf);
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        default:
            break;
        }
    }
    D_00188c74 = 0;
    if (exit_quick_requester())
        bank.unknown2 = 2;
}

// FUNCTION: 0x000528cc
void process_stat_panel(void)
{
    u8 i;

    if (D_00188c81) {
        for (i = 0; D_0015f43c[i].width != 0 && D_0015f43c[i].height != 0; i++) {
            if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i].x, D_0015f43c[i].y, D_0015f43c[i].width,
                             D_0015f43c[i].height)) {
                D_0015f43c[i].callback(i);
                reqspr_draw(D_0015f43c[i].unknownC, D_0015f43c[i].unknownE,
                            &screen_sprites_tab_start[D_0015f43c[i].sprite]);
            }
        }
    }
    if (D_00188c7e) {
        for (i = 0; D_0015f43c[i].width != 0 && D_0015f43c[i].height != 0; i++) {
            if (over_hotspot(mouse_dx, mouse_dy, D_0015f43c[i].x, D_0015f43c[i].y, D_0015f43c[i].width,
                             D_0015f43c[i].height)) {
                reqspr_draw(D_0015f43c[i].unknownC, D_0015f43c[i].unknownE,
                            &screen_sprites_tab_start[D_0015f43c[i].sprite]);
                break;
            }
        }
    }
    D_00188c81 = 0;
}

// FUNCTION: 0x00052b48
void process_stat_button(u8 n)
{
    switch (n) {
    case 0:
        bank.unknown2 = 2;
        play_sound(10, 0x42, -1, -1);
        break;
    case 1:
        play_sound(10, 0x43, -1, -1);
        fn_00074220(0, 0x41);
        break;
    }
}

// FUNCTION: 0x00052bcc
u16 exit_quick_requester(void)
{
    if (lbInkey >= 0x80)
        lbInkey = 0;
    if (lbInkey == 1 || lbInkey == 0x39) {
        lbInkey = 0;
        play_sound(10, 0x42, -1, -1);
        return 1;
    }
    return 0;
}

// FUNCTION: 0x00052c44
void daily_admin(void)
{
    s32 i;
    s32 n;

    mycontrol->unknown64f++;
    D_00156910 = 1;
    try_to_generate_emergency();
    generate_mayors();
    n = 1;
    if (statvar.unknownfd > 1)
        n = 4;
    for (i = 0; i < n; i++) {
        if (statvar.unknownfd > 1 && D_001609c4[i].active == 0)
            continue;
        if (allhosps[i].f1e2s == 2) {
            if (allhosps[i].f1e7 == 0) {
                allhosps[i].f1e2 = 1;
                if (allhosps[i].f1e5 * 100 / allhosps[i].f1e6 >= allhosps[i].f1e8) {
                    change_balance(0x1c, allhosps[i].f1dd * allhosps[i].f1e5, 0, i);
                    reputation_affected(0, 0xc, 0, i, 0);
                    satisfied_net_criteria(1, i);
                } else {
                    reputation_affected(0, 0xd, 0, i, allhosps[i].f1e6 - allhosps[i].f1e5);
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

// FUNCTION: 0x00052e70
void twice_daily_admin(void)
{
    u8 n;
    u8 i;
    AllHosps *h;

    h = allhosps;
    n = 1;
    if (statvar.unknownfd > 1)
        n = 4;
    for (i = 0; i < n; i++, h++) {
        if (statvar.unknownfd > 1 && D_001609c4[i].active == 0)
            continue;
        if (h->f1d9s > 0) {
            h->f1d9--;
            if (h->f1d9s == 0 && h->f1e2s > 0)
                h->f1e2--;
            if (myhosp->f1d9s == 0)
                StopSample(0xd, 0xa9);
        }
        if (epidemic_status(i))
            generate_inspector(i);
    }
    create_a_peep();
    rat_fest_control();
    computer_action();
}

// FUNCTION: 0x00052f7c
void calculate_value(const u8 owner)
{
    s32 total;
    AllHosps *h;
    s32 base;
    s32 bank;
    RoomInfo *room;
    s32 staff;

    total = 0;
    h = &allhosps[owner];
    base = h->f1bf;
    bank = h->unknownc7e / 50 * 1000;
    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->owner == owner)
            total += rooms[room->type].cost;
    }
    count_hospital_area((u16)owner);
    staff = h->f1f1 * *(u16 *)&expertise[53].unknown10[3];
    h->f1bb = staff + (base + bank + total) + 10000;
    if (h->f1bb < 0)
        h->f1bb = 0;
    h->f41 = h->f1bb / 3 + 10000;
    if (h->f41 < 0)
        h->f41 = 0;
    else if (h->f41 > 100000)
        h->f41 = 100000;
}
