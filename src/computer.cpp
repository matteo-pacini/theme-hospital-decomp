#include "hosp.h"

struct ComputerPlayer {
    u8 f0;
    u8 f1;
    u8 unknown2;
    u8 f3;
    u8 f4;
    u8 f5;
    u8 f6;
    u8 unknown7;
    u8 f8;
};

struct ComputerDecisions {
    u16 delay;
};

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

extern ComputerPlayer computer[];
extern ComputerDecisions com_decide[];
extern ComputerStrategy com_strategy[];
extern ComputerBucket (*comp_buckets)[12];
extern u16 staff_max_nbs[][10];
extern u16 object_nbs[][11];
extern u8 com_room_orders[][14];
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
void computer_decision(u8 player);
void get_computer_bucket(u8 player);
u8 computer_employ(u8 type, u16 skills, u8 player);
u8 computer_build(u8 type, u8 player);
u8 computer_place_object(u8 type, u8 a, u8 player);
u8 computer_loan(s8 amount, u8 player);
u8 computer_recheck_actions(u8 a, u8 player);
u8 computer_staff_levels(u8 player);
u8 computer_choose_room(u8 a, u8 player);
u8 make_a_note(u16 type, s16 arg, s16 count, u8 player);
u8 find_a_note(u16 type, u8 player);
void remove_a_note(u16 type, u8 player);
void declass_a_note(u16 type, u8 player);
void computer_diag_progress(Thing *w, u8 player);
u8 computer_free_build(u8 player);
u8 computer_maintain_loan(u8 player);
u8 computer_object_levels(u8 player);
u8 computer_buy_land(u8 player);

extern "C" {

extern u8 D_001553b4;
extern u8 D_0015831b;
extern u16 D_00155d89;
extern char **D_001602a8;
extern char D_00120424[];
extern u16 D_0012dd7c[];

}

// NONMATCHING: register choice
// FUNCTION: 0x0004e760
void setup_computer_players(void)
{
    u8 j;
    u8 k;
    u8 i;
    AllHosps *h;
    u8 used[20];

    if (statvar.unknownfd > 1 || D_001553b4) {
        return;
    }
    for (i = 0; i < 20; i++) {
        used[i] = 0;
    }
    for (j = 0; j < 20; j++) {
        if (computer[j].f8) {
            for (k = 0; k < 4; k++) {
                h = &allhosps[k];
                if (h->f2a == 0) {
                    LbDataLoadSetModifyFilenameFunction((char *)h, D_001602a8[j]);
                    h->f2a = 2;
                    h->f1245 = j;
                    h->f1244 = computer[j].f6;
                    h->f1240 = find_strategy_start(h->f1244);
                    h->f1f1 = myhosp->f1f1;
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
            h = &allhosps[k];
            if (h->f2a == 0) {
                h->f2a = 2;
                LbDataLoadSetModifyFilenameFunction((char *)h, D_00120424);
                for (i = 0; i < 1000; i++) {
                    j = (mouse_dx + mouse_dy + old_newrand()) % 20;
                    h->f1245 = j;
                    if (used[j] == 0) {
                        LbDataLoadSetModifyFilenameFunction((char *)h, D_001602a8[j]);
                        used[j] = 1;
                        break;
                    }
                }
                h->f1244 = computer[j].f6;
                h->f1240 = find_strategy_start(h->f1244);
            }
        }
    }
}

// FUNCTION: 0x0004e9d8
void computer_year(void)
{
    u8 i;
    AllHosps *h;

    if (statvar.unknownfd <= 1) {
        for (i = 0; i < 4; i++) {
            h = &allhosps[i];
            if (h->f2a == 2) {
                calculate_value(i);
                calculate_score(i);
            }
        }
    }
}

// FUNCTION: 0x0004ea50
void computer_day(void)
{
    u8 i;
    AllHosps *h;

    if (statvar.unknownfd <= 1) {
        if (mycontrol->unknown64f % 5 == 0) {
            for (i = 0; i < 4; i++) {
                h = &allhosps[i];
                if (h->f2a == 2) {
                    get_computer_bucket(i);
                }
            }
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0004eae0
void computer_month(void)
{
    u8 i;
    AllHosps *h;

    if (statvar.unknownfd <= 1) {
        for (i = 0; i < 4; i++) {
            h = &allhosps[i];
            if (h->f2a == 2) {
                get_computer_bucket(i);
                h->f35 = h->f35 - h->f1234;
            }
        }
    }
}

// NONMATCHING: register choice (u8 field loads, com_decide index)
// FUNCTION: 0x0004eb5c
void computer_action(void)
{
    s8 r;
    u8 i;
    AllHosps *h;
    ComputerPlayer *c;

    if (statvar.unknownfd > 1 || (D_0015831b & 1)) {
        return;
    }
    for (i = 0; i < 4; i++) {
        h = &allhosps[i];
        if (h->f2a != 2 || h->f11f8 > actual_game_turn) {
            continue;
        }
        c = &computer[h->f1245];
        r = c->f3 - old_newrand() % c->f0;
        if (r < 1) {
            r = 1;
        } else if (r > 100) {
            r = 100;
        }
        switch (h->action.type) {
        case 0:
            computer_decision(i);
            break;
        case 1:
            if (computer_employ(h->action.arg, h->action.count, i)) {
                if (h->f1202) {
                    remove_a_note(1, i);
                }
            } else if (h->f1202) {
                declass_a_note(1, i);
            }
            h->f1202 = 0;
            h->action.type = 0;
            h->f11f8 = actual_game_turn + com_decide[0].delay * r / 10;
            break;
        case 2:
            if (computer_build(h->action.arg, i)) {
                if (h->f1202) {
                    remove_a_note(2, i);
                }
            } else if (h->f1202) {
                declass_a_note(2, i);
            }
            h->f1202 = 0;
            h->action.type = 0;
            h->f11f8 = actual_game_turn + com_decide[0].delay * r / 10;
            break;
        case 3:
            computer_place_object(h->action.arg, 0, i);
            h->action.count--;
            if (h->action.count <= 0) {
                h->action.type = 0;
            }
            h->f11f8 = actual_game_turn + com_decide[h->action.type].delay * r / 10;
            break;
        case 4:
            computer_loan(h->action.arg, i);
            h->action.type = 0;
            h->f11f8 = actual_game_turn + com_decide[0].delay * r / 10;
            break;
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0004eebc
void computer_decision(u8 player)
{
    AllHosps *h;
    ComputerPlayer *c;
    int s;
    int r;
    u32 action;

    h = &allhosps[player];
    c = &computer[h->f1245];
    s = h->f1240;
    h->f1240++;
    r = c->f3 - old_newrand() % c->f0 + old_newrand() % 5;
    if (r < 1) {
        r = 1;
    } else if (r > 100) {
        r = 100;
    }
    action = com_strategy[s].type;
    switch (action) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        h->action.type = action;
        h->action.arg = com_strategy[s].arg;
        h->action.count = com_strategy[s].count;
        h->f11f8 = actual_game_turn + com_decide[action].delay * r / 10;
        break;
    case 6:
        h->f1244 = 2;
        h->f1240 = find_strategy_start(2);
        break;
    case 7:
        computer_choose_room(0, player);
        break;
    case 8:
        computer_recheck_actions(com_strategy[s].arg, player);
        break;
    case 10:
        if (statvar.unknownfd <= 1) {
            h->unknown1256 = 1;
        }
        announce_this(0xc, player);
        break;
    case 11:
        computer_staff_levels(player);
        break;
    case 12:
        computer_object_levels(player);
        break;
    case 13:
        computer_buy_land(player);
        break;
    case 14:
        computer_maintain_loan(player);
        break;
    case 15:
        h->f1240 = find_strategy_start(h->f1244);
        break;
    case 16:
        h->f1244 = 2;
        h->f1240 = find_strategy_start(2);
        break;
    case 9:
        break;
    case 17:
        computer_free_build(player);
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
        bucket = &comp_buckets[player][month];                 \
    }

// NONMATCHING: register choice (u8 field loads through a pointer)
// FUNCTION: 0x0004f15c
void process_computer_peep(Thing *w, u8 player)
{
    AllHosps *h;
    ComputerPlayer *c;
    u32 month;
    u32 hours;
    ComputerBucket *bucket;
    int kind;
    int busy;
    int chance;
    int patience;
    int result;
    int step;
    int pct;

    h = &allhosps[player];
    c = &computer[h->f1245];
    month = mycontrol->unknown650;
    hours = mycontrol->unknown64f;
    bucket = &comp_buckets[player][month];
    kind = w->style;
    h->f123c++;
    if (listhead[player].objects[7] * 10 > h->f123c) {
        busy = 1;
    }
    if (h->f123c > 5) {
        chance = 100 - h->f123c / ((h->f1238 + 1) * 10) - h->f123c / ((h->fc9c + 1) * 10)
            - h->f123c / ((listhead[player].objects[4] + 1) * 10);
        if (chance < 0) {
            chance = 0;
        } else if (chance > 100) {
            chance = 100;
        }
    } else {
        chance = 20;
    }
    patience = w->unknown130;
    result = 0;
    step = 5;
    for (;;) {
        ADVANCE_TIME();
        if (allhosps[player].rooms[1].unknown1C == 1) {
            update_single_cure_cost(1, player);
            bucket->f0 += h->rooms[1].unknownC;
            w->unknown171 = 1;
            computer_diag_progress(w, player);
            reputation_affected(1, 10, month % 12, w->owner, 0);
            if (w->unknown146 > D_0012dd7c[0]) {
                w->unknown171 = w->style;
                reputation_affected(kind, 6, month % 12, w->owner, 0);
                h->f1ef++;
                result = 1;
                break;
            }
        } else {
            reputation_affected(kind, 7, month % 12, player, 0);
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
        find_possible_diagnosis(w);
        if (w->unknown171 == 0) {
            reputation_affected(kind, 7, month % 12, player, 0);
            pct = w->unknown146 * 100 / D_0012dd7c[0];
            if (pct < 0) {
                pct = 0;
            } else if (pct > 99) {
                pct = 99;
            }
            if (c->f5 <= pct) {
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
        update_single_cure_cost(w->unknown171, player);
        bucket->f0 += h->rooms[w->unknown171].unknownC;
        add_treatment_to_history(w);
        computer_diag_progress(w, player);
        reputation_affected(w->unknown171, 10, month % 12, w->owner, 0);
        if (old_newrand() % chance == 0) {
            result = 3;
            break;
        }
    }
    reputation_affected(kind, 10, month % 12, w->owner, 0);
    switch (result) {
    case 1:
        if (allhosps[player].rooms[kind].unknown1C == 1) {
            if (expertise[kind].unknown5[0] == 1 && old_normal_rand(0, 99) >= h->rooms[kind].unknown1B) {
                bucket->f10++;
                reputation_affected(kind, 3, month % 12, player, 0);
            } else {
                bucket->fc++;
                reputation_affected(kind, 0, month % 12, player, 0);
            }
        } else {
            reputation_affected(kind, 2, month % 12, player, 0);
            result = 3;
        }
        break;
    case 2:
        bucket->f10++;
        reputation_affected(kind, 3, month % 12, player, 0);
        break;
    case 3:
        reputation_affected(kind, 2, month % 12, player, 0);
        break;
    }
    reputation_affected(kind, 0x16, month % 12, player, 0);
    bucket->f14++;
}

// NONMATCHING: register choice
// FUNCTION: 0x0004f900
void get_computer_bucket(u8 player)
{
    int month;
    ComputerBucket *b;
    AllHosps *h;
    u32 i;

    month = D_00155d89 % 12;
    b = &comp_buckets[player][month];
    h = &allhosps[player];
    h->f35 += b->f0;
    h->f1e9 += b->fc;
    h->f1eb += b->f10;
    h->fc82 += b->f4;
    h->fc86 += b->f8;
    evaluate_reputation(0, player);
    h->f123c -= b->f14;
    if (h->f123c < 0) {
        h->f123c = 0;
    }
    for (i = 0; i < 47; i++) {
        h->rooms[i].f4 += b->f18[i];
        h->rooms[i].f8 += b->fd4[i];
        evaluate_reputation(i, player);
    }
    memset(b, 0, 400);
}

// NONMATCHING: register choice
// FUNCTION: 0x0004fa50
u8 computer_employ(u8 type, u16 skills, u8 player)
{
    Thing *w;
    int ok;
    u32 i;
    AllHosps *h;

    for (w = thing + 1; w < end_things; w++) {
        if (w->renderer == 4 && w->kind == 0x2e) {
            if ((u16)(w - thing) != gamevar.unknown75) {
                if (type == w->subtype) {
            ok = 1;
            if (skills) {
                for (i = 0; i < 9; i++) {
                    if (skills & (1 << i)) {
                        if ((w->unknown124 & (1 << i)) == 0) {
                            ok = 0;
                            break;
                        }
                    }
                }
            }
            if (ok) {
                if (allhosps[player].f35 >= w->unknown108 * 12 / 10 && allhosps[player].f35 > 0) {
                    h = &allhosps[player];
                    inc_staff_stats(w, player);
                    h->f35 -= w->unknown108 * 12 / 10;
                    h->f1234 += w->unknown108;
                    h->f1246[w->subtype] += w->unknown126;
                    w->owner = player;
                    set_expertise_staff(w);
                    insert_skill(w->unknown124, player);
                    remove_staff_in_struct(w);
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

// NONMATCHING: register choice
// FUNCTION: 0x00050528
u8 make_a_note(u16 type, s16 arg, s16 count, u8 player)
{
    AllHosps *h;
    int i;

    h = &allhosps[player];
    for (i = 0; i < 8; i++) {
        if (h->notes[i].type == 0) {
            h->notes[i].type = type;
            h->notes[i].arg = arg;
            h->notes[i].count = count;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x000505d0
u8 find_a_note(u16 type, u8 player)
{
    AllHosps *h;
    int i;

    h = &allhosps[player];
    for (i = 0; i < 8; i++) {
        if (h->notes[i].type == type) {
            h->action.type = type;
            h->action.arg = h->notes[i].arg;
            h->action.count = h->notes[i].count;
            h->f1202 = 1;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x00050684
void remove_a_note(u16 type, u8 player)
{
    AllHosps *h;
    int i;

    h = &allhosps[player];
    for (i = 0; i < 8; i++) {
        if (h->notes[i].type == type) {
            h->notes[i].type = 0;
            break;
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x000506f4
void declass_a_note(u16 type, u8 player)
{
    AllHosps *h;
    int i;

    h = &allhosps[player];
    find_a_note(type, player);
    remove_a_note(type, player);
    for (i = 7; i >= 0; i--) {
        if (h->notes[i].type == 0) {
            h->notes[i].type = h->action.type;
            h->notes[i].arg = h->action.arg;
            h->notes[i].count = h->action.count;
            break;
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0004fc48
u8 computer_build(u8 type, u8 player)
{
    AllHosps *h;
    int i;
    RoomInfo room;

    if (allhosps[player].f35 >= rooms[type].cost && allhosps[player].f35 > 0) {
        h = &allhosps[player];
        h->f35 -= rooms[type].cost;
        h->f1238++;
        for (i = 0; i < rooms[type].fc; i++) {
            computer_place_object(rooms[type].objects[i], 0, player);
        }
        listhead[player].rooms[type]++;
        room.type = type;
        room.owner = player;
        set_expertise_room(&room);
        return 1;
    }
    return 0;
}

// NONMATCHING: register choice; `|| free` tests with and+test instead of cmp byte ptr
// FUNCTION: 0x0004fd6c
u8 computer_place_object(u8 obj, u8 free, u8 player)
{
    AllHosps *h;

    h = &allhosps[player];
    if ((allhosps[player].f35 >= (u32)h->object_stats[obj].cost && allhosps[player].f35 > 0) || free) {
        if (free == 0) {
            h->f35 -= h->object_stats[obj].cost;
        }
        inc_hardware_stats(obj, player);
        if (objects[obj].unknown14 == 7) {
            listhead[player].rooms[4]++;
        }
        listhead[player].objects[obj]++;
        return 1;
    }
    return 0;
}

// FUNCTION: 0x0004fe78
u8 computer_loan(s8 amount, u8 player)
{
    AllHosps *h;

    h = &allhosps[player];
    if ((amount < 0 && h->f35 >= 5000) && h->f3d > 0) {
        h->f35 += 5000;
        h->f3d += 5000;
        return 1;
    }
    if (amount > 0 && h->f41 - 5000 >= h->f3d) {
        h->f35 -= 5000;
        h->f3d -= 5000;
        return 1;
    }
    return 0;
}

// NONMATCHING: register choice
// FUNCTION: 0x0004ff24
u8 computer_recheck_actions(u8 a, u8 player)
{
    AllHosps *h;
    int counts[5];
    int i;

    h = &allhosps[player];
    memset(counts, 0, 20);
    i = find_strategy_start(h->f1244);
    if (h->f1242) {
        for (; i < h->f1242; i++) {
            if (com_strategy[i].type == 1) {
                counts[com_strategy[i].arg]++;
            }
        }
    }
    for (; i < h->f1240; i++) {
        if (com_strategy[i].type == 1) {
            counts[com_strategy[i].arg]++;
            if (h->fc9e[com_strategy[i].arg] < counts[com_strategy[i].arg]) {
                h->action.type = 1;
                h->action.arg = com_strategy[i].arg;
                h->action.count = com_strategy[i].count;
                h->f11f8 = actual_game_turn;
                if (a == 1) {
                    h->f1240--;
                } else {
                    h->f1242 = i + 1;
                }
                break;
            }
        }
    }
    if (a == 0 && i < h->f1240) {
        h->f1240--;
    }
    if (a == 1 || h->f1240 == i) {
        h->f1242 = 0;
    }
    return 0;
}

// NONMATCHING: register choice (u8 field loads through a pointer)
// FUNCTION: 0x0005010c
u8 computer_staff_levels(u8 player)
{
    AllHosps *h;
    ComputerPlayer *c;
    int r;
    int i;
    int max;

    h = &allhosps[player];
    c = &computer[h->f1245];
    if (find_a_note(1, player)) {
        r = c->f3 - old_newrand() % c->f0;
        if (r < 1) {
            r = 1;
        } else if (r > 100) {
            r = 100;
        }
        h->f11f8 = actual_game_turn + com_decide[1].delay * r / 10;
        return 1;
    }
    for (i = 0; i <= 3; i++) {
        max = h->f123c / staff_max_nbs[i][c->f1];
        if (h->fc9e[i] < max) {
            r = c->f3 - old_newrand() % c->f0;
            if (r < 1) {
                r = 1;
            } else if (r > 100) {
                r = 100;
            }
            h->action.type = 1;
            h->action.arg = i;
            h->f11f8 = actual_game_turn + com_decide[1].delay * r / 10;
            return 1;
        }
    }
    return 0;
}

// NONMATCHING: register choice
// FUNCTION: 0x00050360
u8 computer_choose_room(u8 idx, u8 player)
{
    int failed;
    AllHosps *h;
    ComputerPlayer *c;
    int r;
    int i;
    int room;

    failed = 0;
    h = &allhosps[player];
    c = &computer[h->f1245];
    r = c->f3 - old_newrand() % c->f0;
    if (r < 1) {
        r = 1;
    } else if (r > 100) {
        r = 100;
    }
    for (i = 1; i <= com_room_orders[idx][0]; i++) {
        room = com_room_orders[idx][i];
        if (listhead[player].rooms[room] == 0 && room_available(room)) {
            if (allhosps[player].f35 >= rooms[room].cost && allhosps[player].f35 > 0) {
                h->action.type = 2;
                h->action.arg = room;
                h->f11f8 = actual_game_turn + com_decide[2].delay * r / 10;
                return room;
            } else {
                failed = 1;
            }
        }
    }
    if (failed) {
        make_a_note(4, 0, 0, player);
    }
    return 0;
}

// NONMATCHING: register choice; target reloc is change_person_who+0x98 rather than diag_register-0x4a
// FUNCTION: 0x000507c4
void computer_diag_progress(Thing *w, u8 player)
{
    AllHosps *h;
    u32 skill;
    int i;
    int who;
    int val;

    h = &allhosps[player];
    skill = expertise[w->unknown171].unknown0;
    for (i = 0; i < 9; i++) {
        if (skill & (1 << i)) {
            break;
        }
    }
    who = skillnb_to_who[i];
    val = w->unknown146 - w->unknown144;
    val += h->f1246[who] / h->fc9e[who];
    val += 250 - old_normal_rand(0, 500);
    if (w->unknown171 >= 0x24) {
        val += diag_register[w->style][w->unknown171 - 0x24];
    }
    if (w->unknown146 < val) {
        w->unknown146 = val;
    }
}

// NONMATCHING: register choice (u8 field loads through a pointer)
// FUNCTION: 0x0005090c
u8 computer_free_build(u8 player)
{
    AllHosps *h;
    ComputerPlayer *c;
    int r;
    int room;
    int arg;
    int count;

    h = &allhosps[player];
    c = &computer[h->f1245];
    if (find_a_note(2, player)) {
        r = c->f3 - old_newrand() % c->f0;
        if (r < 1) {
            r = 1;
        } else if (r > 100) {
            r = 100;
        }
        h->f11f8 = actual_game_turn + com_decide[2].delay * r / 10;
        return 1;
    }
    room = computer_choose_room(0, player);
    if (room != 0) {
        arg = rooms[room].f26;
        if (arg >= 0) {
            count = rooms[room].unknown8;
            make_a_note(1, arg, count, player);
        }
        return 1;
    }
    room = computer_choose_room(1, player);
    if (room != 0) {
        arg = rooms[room].f26;
        if (arg >= 0) {
            count = rooms[room].unknown8;
            make_a_note(1, arg, count, player);
        }
        return 1;
    }
    return 0;
}

// NONMATCHING: register choice (u8 field loads through a pointer)
// FUNCTION: 0x00050aa8
u8 computer_maintain_loan(u8 player)
{
    AllHosps *h;
    ComputerPlayer *c;
    int r;

    h = &allhosps[player];
    c = &computer[h->f1245];
    if (find_a_note(4, player)) {
        r = c->f3 - old_newrand() % c->f0;
        if (r < 1) {
            r = 1;
        } else if (r > 100) {
            r = 100;
        }
        h->f11f8 = actual_game_turn + com_decide[4].delay * r / 10;
        h->f1202 = 0;
        remove_a_note(4, player);
        return 1;
    }
    if (h->f3d != 0 && h->f35 > 6000) {
        r = c->f3 - old_newrand() % c->f0;
        if (r < 1) {
            r = 1;
        } else if (r > 100) {
            r = 100;
        }
        h->action.type = 4;
        h->action.arg = -1;
        h->f11f8 = actual_game_turn + com_decide[4].delay * r / 10;
        return 1;
    }
    return 0;
}

// NONMATCHING: register choice (u8 field loads through a pointer)
// FUNCTION: 0x00050c44
u8 computer_object_levels(u8 player)
{
    AllHosps *h;
    ComputerPlayer *c;
    int i;
    int obj;
    int max;
    int r;
    int n;

    h = &allhosps[player];
    c = &computer[h->f1245];
    for (i = 0; i < 4; i++) {
        obj = object_nbs[i][0];
        max = h->f123c / object_nbs[i][c->f4 + 1];
        if (h->object_stats[obj].fA < max) {
            r = c->f3 - old_newrand() % c->f0;
            if (r < 1) {
                r = 1;
            } else if (r > 100) {
                r = 100;
            }
            h->action.type = 3;
            h->action.arg = obj;
            n = max - h->object_stats[obj].fA;
            if (n < 1) {
                n = 1;
            } else if (n > 4) {
                n = 4;
            }
            h->action.count = n;
            h->f11f8 = actual_game_turn + com_decide[3].delay * r / 10;
            return 1;
        }
    }
    return 0;
}

// NONMATCHING: register choice
// FUNCTION: 0x00050de4
u8 computer_buy_land(u8 player)
{
    AllHosps *h;
    ComputerPlayer *c;
    int n;

    h = &allhosps[player];
    c = &computer[h->f1245];
    if (myhosp->f1f1 > h->f1f1) {
        n = myhosp->f1f1 - h->f1f1;
        if (*(u16 *)&expertise[53].unknown10[3] * n <= allhosps[player].f35 && allhosps[player].f35 > 0) {
            h->f35 -= n * *(u16 *)&expertise[53].unknown10[3];
            h->f1f1 += n;
            return 1;
        }
        make_a_note(4, 0, 0, player);
    }
    return 0;
}
