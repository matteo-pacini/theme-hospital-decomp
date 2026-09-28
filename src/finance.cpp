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

extern "C" {
extern u8 D_00158324;
extern u16 D_00155d89;
}

// NONMATCHING: register choice
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

// NONMATCHING: register choice
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

// NONMATCHING: register choice
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
