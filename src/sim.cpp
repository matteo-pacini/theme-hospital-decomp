#include "hosp.h"

struct ComputerBucket {
    u8 unknown0[4];
    s32 f4;
    s32 f8;
    u8 unknownc[0xc];
    s32 f18[0x2f];
    s32 fd4[0x2f];
};

extern "C" {
extern u8 D_00155d6f;
extern u32 D_0015542c;
extern u16 D_0015542a;
extern u8 D_00155e17;
extern u8 D_00155e02;
extern u16 D_00158190[];
extern u16 D_00155d89;
extern u8 D_00155e15;
extern u8 D_00155e4c;
extern u8 D_00155e4d;
extern Event D_001549d0[];
}

extern ComputerBucket (*comp_buckets)[12];
extern ScoreRatings rating[];
extern WinLose win_criteria[];
extern WinLose lose_criteria[];
extern WinLoseNet net_criteria[];
extern InsuranceCo insurance_co[];
extern u16 ins_control[];
extern s16 choice_time_out;

void evaluate_reputation(const u8 room, const u8 owner);
s32 evaluate_criteria(const u8 owner);
u32 check_criteria_values(const u32 mode, const u8 owner);
s32 check_criteria(u16 mode, u8 owner);
u8 check_net_criteria(const u8 owner);
void setup_next_net_criteria(const u8 owner);
void try_bonus_level(void);

// FUNCTION: 0x00058370
void reputation_affected(const u8 room, const u16 event, const u16 month, const u8 owner, const s32 amount)
{
    s32 delta;
    s32 count;
    AllHosps *h;

    delta = 0;
    count = 0;
    h = &allhosps[owner];
    switch (event) {
    case 0:
    case 6:
    case 21:
        delta = 1;
        break;
    case 3:
    case 25:
        delta = -1;
        break;
    case 22:
        count = 1;
        break;
    case 2:
    case 4:
    case 7:
    case 10:
        break;
    case 5:
        delta = -(h->fc86 * 5) / 100;
        break;
    case 9:
        if (room)
            delta = h->rooms[room].f8 * 5 / 100;
        else
            delta = h->fc86 * 5 / 100;
        break;
    case 11:
        delta = -(h->fc86 * 10) / 100;
        break;
    case 12:
        delta = h->fc86 * 2 / 100;
        break;
    case 13:
        delta = -(h->fc86 * (amount * 4)) / 100;
        break;
    case 14:
        delta = -(h->fc86 * 2) / 100;
        break;
    case 15:
        delta = -(h->fc86 * 10) / 100;
        break;
    case 16:
        delta = h->fc86 * 4 / 100;
        break;
    case 17:
        delta = h->fc86 * 2 / 100;
        break;
    case 18:
        delta = -(h->fc86 * 4) / 100;
        break;
    case 19:
        delta = -(h->fc86 * 20) / 100;
        break;
    case 20:
        delta = -1;
        break;
    case 23:
    case 24:
        delta += amount * h->fc86 / 100;
        break;
    case 1:
    case 8:
    default:
        break;
    }
    if (h->f2a == 1) {
        if (room) {
            h->rooms[room].f4 += delta;
            h->rooms[room].f8 += count;
            evaluate_reputation(room, owner);
        }
        h->fc82 += delta;
        h->fc86 += count;
        evaluate_reputation(0, owner);
    } else if (h->f2a == 2) {
        ComputerBucket *bucket;

        bucket = &comp_buckets[owner][month];
        if (room) {
            bucket->f18[room] += delta;
            bucket->fd4[room] += count;
        }
        bucket->f4 += delta;
        bucket->f8 += count;
    }
    if (room)
        update_single_cure_cost(room, owner);
}

// FUNCTION: 0x00058730
void evaluate_reputation(const u8 room, const u8 owner)
{
    AllHosps *h;

    h = &allhosps[owner];
    if (room) {
        if (h->rooms[room].f8 > 0)
            h->rooms[room].unknown0 = h->rooms[room].f4 * 500 / h->rooms[room].f8;
        else
            h->rooms[room].unknown0 = 500;
        if (h->rooms[room].unknown0 < 0)
            h->rooms[room].unknown0 = 0;
        else if (h->rooms[room].unknown0 > 1000)
            h->rooms[room].unknown0 = 1000;
    } else {
        if (h->fc86 > 0)
            h->unknownc7e = h->fc82 * 500 / h->fc86;
        else
            h->unknownc7e = 500;
        if (h->unknownc7e < 0)
            h->unknownc7e = 0;
        else if (h->unknownc7e > 1000)
            h->unknownc7e = 1000;
    }
}

// FUNCTION: 0x000588a8
void calculate_score(u8 owner)
{
    AllHosps *h;
    s32 score;
    s32 i;

    h = &allhosps[owner];
    score = evaluate_criteria(owner);
    for (i = 0; rating[i].threshold; i++) {
        if (rating[i].threshold >= score) {
            if (h->f2d > 1000000)
                h->f2d += rating[i].f4 * h->f2d / 10000;
            else if (gbv.f124 * h->f31 / 100 < h->f2d)
                h->f2d += rating[i].f4 * h->f2d / 10000;
            else
                h->f2d += rating[i].f2 * h->f2d / 10000;
            break;
        }
    }
    if (h->f2d > 200000000)
        h->f2d = 200000000;
}

// FUNCTION: 0x000589f0
void check_game_over(void)
{
    s32 score;
    s32 pct;

    if (statvar.unknownfd > 1) {
        check_net_criteria(player);
    } else {
        score = evaluate_criteria(player);
        if (score >= 100) {
            D_00155d6f = 1;
            pct = myhosp->f2c / 2;
            pct += old_normal_rand(0x37, 0x3c);
            D_0015542c = myhosp->f2d * pct / 100;
            if (D_0015542c > 200000000)
                D_0015542c = 200000000;
            year_end_type = 5;
            D_0015542a = 1;
            winlose.unknown2 = D_0015542a;
            D_00155e17 = 0;
            try_bonus_level();
        } else if (check_criteria(1, player)) {
            year_end_type = 7;
            D_0015542a = 2;
            winlose.unknown2 = D_0015542a;
        }
    }
}

// FUNCTION: 0x00058b00
void check_quarterly_game_over(void)
{
    s32 pct;

    if (statvar.unknownfd > 1) {
        check_net_criteria(player);
    } else if (myhosp->f29 < 12) {
        if (evaluate_criteria(player) < 100)
            return;
        if (!D_00155e17) {
            D_00155d6f = 1;
            pct = myhosp->f2c / 2;
            pct += old_normal_rand(0x37, 0x3c);
            D_0015542c = myhosp->f2d * pct / 100;
            if (D_0015542c > 200000000)
                D_0015542c = 200000000;
            if (display_mode == 9) {
                LbDrawBoxCoords(&year_end_backup.unknown2, D_00158190, 10);
                year_end_backup.mode = display_mode;
                year_end_backup.f6 = choice_spare_data;
                year_end_backup.f8 = choice_time_out;
            } else {
                LbDrawBoxCoords(&year_end_backup, &choice_backup, 10);
            }
            year_end_type = 5;
            display_mode = 0xc;
            winlose.unknown2 = 1;
            try_bonus_level();
        }
        D_00155e17 = 0;
    }
}

// FUNCTION: 0x00058c54
u16 shall_peep_pay(Thing *w)
{
    u16 room;
    u16 pays;
    u16 attempt;
    u16 ins;
    u16 price;
    s32 max;

    room = w->unknown171;
    advisor.fc_u16 = w->unknown171;
    for (attempt = 0; attempt < 2; attempt++) {
        pays = 0;
        update_single_cure_cost(room, w->owner);
        price = allhosps[w->owner].rooms[room].unknownC;
        max = max_price_for_treatment(room, w->owner);
        ins = w->unknown172[1] & allhosps[w->owner].f216;
        if (ins)
            max += insurance_co[ins_control[ins]].f3 * max / 100;
        else if (price > w->f70)
            max = 0;
        max += old_newrand() % (price / 2 + 1);
        if (price <= max)
            pays = 1;
        room = w->unknown172[0];
        if (room == 0 || pays == 0)
            break;
    }
    if (!pays) {
        set_icon_value(w, 0x13);
        w->unknown118 |= 0x80;
        record_person_as_patient(w);
        if (w->owner == player)
            advisor.f128 |= 2;
    }
    return pays;
}

// FUNCTION: 0x00058e34
s32 max_price_for_treatment(const u16 room, const u8 owner)
{
    u8 flag;
    u16 base;
    s32 rep;
    u16 i;
    s32 price;

    flag = 0;
    base = expertise[room].f2;
    if (room >= 0x24 || room == 1)
        rep = allhosps[owner].unknownc7e;
    else
        rep = allhosps[owner].rooms[room].unknown0;
    if (statvar.unknownfd > 1) {
        flag = 1;
        for (i = 0; i < 4; i++) {
            if (i != owner && allhosps[i].rooms[room].unknown1C == 1)
                flag = 0;
        }
    }
    price = base * rep / 500 + flag * base;
    if (base > price)
        return base;
    return price;
}

// FUNCTION: 0x00058f7c
void calc_pricing_factor(const u8 owner)
{
    u32 max_total;
    s32 price_total;
    u32 i;

    max_total = 0;
    price_total = 0;
    for (i = 0; i < 0x2f; i++) {
        if (allhosps[owner].rooms[i].unknown1C == 1) {
            max_total += max_price_for_treatment(i, owner);
            price_total += allhosps[owner].rooms[i].unknownC;
        }
    }
    if (max_total > 0)
        allhosps[owner].fc8e = price_total * 100 / max_total;
    else
        allhosps[owner].fc8e = 100;
    if (allhosps[owner].fc8e < 0)
        allhosps[owner].fc8e = 0;
    else if (allhosps[owner].fc8e > 300)
        allhosps[owner].fc8e = 300;
}

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
            if (table[i].fc == crit) {
                any = 1;
                if (table[i].fe == 0) {
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

// FUNCTION: 0x000591d4
u32 check_criteria_values(const u32 mode, const u8 owner)
{
    AllHosps *h;
    s32 max;
    WinLose *table;

    s32 i;
    s32 val;
    s32 pct;
    s32 range;

    h = &allhosps[owner];
    max = 0;
    if (owner == 0xff)
        return 0;
    if (mode == 0)
        table = win_criteria;
    else
        table = lose_criteria;
    for (i = 0; i < 10; i++) {
        table[i].fe = 0;
        if (table[i].fc > max)
            max = table[i].fc;
        switch (table[i].criterion) {
        case 0:
            table[i].fe = 1;
            val = 0;
            break;
        case 1:
            val = h->unknownc7e;
            break;
        case 2:
            if (mode == 0)
                val = h->f35 - h->f3d;
            else
                val = h->f35;
            break;
        case 3:
            if (h->f1ed > 10)
                val = h->f1ef * 100 / h->f1ed;
            else
                val = 0;
            break;
        case 4:
            val = h->f1e9;
            break;
        case 5:
            if (h->f1ed > 10)
                val = h->f1eb * 100 / h->f1ed;
            else
                val = 0;
            break;
        case 6:
            val = h->f1bb;
            break;
        }
        if (table[i].op == 1) {
            if (val >= table[i].value)
                table[i].fe = 1;
        } else if (val <= table[i].value) {
            table[i].fe = 1;
        }
        table[i].f14 = pct = 0;
        range = table[i].value - table[i].base;
        if (range != 0)
            pct = (val - table[i].base) * 100 / range;
        if (pct < 0)
            pct = 0;
        else if (pct > 200)
            pct = 200;
        table[i].f14 = pct;
        table[i].f10 = val;
    }
    return max;
}

#define ABS(x) ((x) < 0 ? -(x) : (x))

// FUNCTION: 0x000594d0
s32 evaluate_criteria(const u8 owner)
{
    AllHosps *h;
    s32 best;
    s32 best_group;
    s32 total;
    s32 groups;
    s32 group;
    s32 n;
    s32 sum;
    s32 i;
    s32 pct;

    h = &allhosps[owner];
    best = 0;
    best_group = 0;
    total = 0;
    groups = check_criteria_values(0, owner);
    for (group = 1; group <= groups; group++) {
        n = 0;
        sum = 0;
        for (i = 0; i < 10; i++) {
            if (win_criteria[i].fc != group)
                continue;
            n++;
            if (win_criteria[i].fe == 1) {
                sum += 100;
                continue;
            }
            if (win_criteria[i].op == 1) {
                if (win_criteria[i].value != 0) {
                    pct = ABS(win_criteria[i].f10 * 100 / win_criteria[i].value);
                    if (win_criteria[i].criterion == 2 && win_criteria[i].f10 < 0)
                        pct = 0;
                } else {
                    pct = 0;
                }
            } else if (win_criteria[i].f10 != 0) {
                pct = ABS(win_criteria[i].value * 100 / win_criteria[i].f10);
            } else {
                pct = 0;
            }
            if (D_00155d6f == 0) {
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
        if (sum / n > best) {
            best = sum / n;
            best_group = group;
        }
        total += sum / n;
    }
    if (total > 0 && groups > 0)
        total /= groups;
    else
        total = 0;
    adcounts.f9e = total;
    h->f2c = best;
    if (owner == player)
        D_00155e02 = best_group;
    return best;
}

// FUNCTION: 0x000597c4
u8 check_net_criteria(const u8 owner)
{
    s32 i;
    s32 val;
    AllHosps *h;
    u8 kind;
    u8 fail;
    u8 better;
    u8 active;

    h = &allhosps[owner];
    kind = 0;
    fail = 0;
    better = 0;
    active = 0;
    if (current_net_criteria.unknown0) {
        if (D_00155d89 >= current_net_criteria.start) {
            active = 1;
            switch (current_net_criteria.unknown0) {
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
                val = h->f1e9 - current_net_criteria.fa;
                kind = 1;
                break;
            case 5:
                val = h->f35 - h->f3d;
                kind = 1;
                break;
            case 6:
                val = h->f1bb;
                kind = 1;
                break;
            case 7:
                val = h->unknownc7e;
                kind = 1;
                break;
            case 8:
                for (i = 0; i < 4; i++) {
                    if (D_001609c4[i].active) {
                        if (i != player && h->unknownc7e >= allhosps[i].unknownc7e)
                            better = 1;
                    }
                }
                kind = 3;
                break;
            case 9:
                for (i = 0; i < 4; i++) {
                    if (D_001609c4[i].active) {
                        if (i != player && h->f35 - h->f3d >= allhosps[i].f35 - allhosps[i].f3d)
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
    fn_0008493c(fail, active);
    if (owner == player) {
        if (evaluate_criteria(owner) >= 100) {
            D_001549d0[player].type = 0x90;
            D_00155e4c = 2;
            D_00155e4d = 1;
            return 0;
        }
        if (check_criteria(1, owner)) {
            D_001549d0[player].type = 0x7e;
            D_001549d0[player].value = 1;
            D_00155e4c = 1;
            return 0;
        }
    }
    return fail;
}

// FUNCTION: 0x00059af8
void setup_next_net_criteria(const u8 owner)
{
    u8 i;

    if (statvar.unknownfd > 1 && owner == player) {
        current_net_criteria.unknown0 = 0;
        for (i = 0; i < 20; i++) {
            if (net_criteria[i].done == 0 && net_criteria[i].unknown0) {
                if (D_00155d89 >= net_criteria[i].start) {
                    net_criteria[i].done = 1;
                    current_net_criteria.unknown0 = net_criteria[i].unknown0;
                    current_net_criteria.value = net_criteria[i].value;
                    current_net_criteria.start = D_00155d89 + net_criteria[i].fa;
                    if (current_net_criteria.unknown0 == 4)
                        current_net_criteria.fa = allhosps[owner].f1e9;
                    current_net_criteria.done = 0;
                    D_00155e15 = 1;
                    break;
                }
            }
        }
    }
}

// FUNCTION: 0x00059c38
void satisfied_net_criteria(const u8 criterion, const u8 owner)
{
    if (statvar.unknownfd > 1 && owner == player) {
        if (criterion == current_net_criteria.unknown0)
            current_net_criteria.done = 1;
    }
}

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

// FUNCTION: 0x00059ce0
void force_win_lose_criteria(const u32 mode, const u8 owner)
{
}
