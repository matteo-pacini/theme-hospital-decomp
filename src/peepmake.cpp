#include "hosp.h"

/* One bidder in who_owns_peep: a weighted score and the hospital it is for. */
struct PeepBid {
    signed int score : 27;
    u8 player : 5;
};

/* A kind of visitor: its sprite set, head and palette choices, and weight. */
struct PeepType {
    u8 kind;
    u8 count;
    u8 head[5];
    u8 palette[5];
    u8 weight;
};

struct PeepPalette {
    u8 count1;
    u8 layer1[5];
    u8 count2;
    u8 layer2[5];
    u8 count3;
    u8 layer3[11];
};

struct PeepChoice {
    u8 kind;
    u8 head;
    u8 palette;
};

extern "C" {

extern u16 D_00155385;
extern u16 D_00155387;
extern u16 D_00155389;
extern u16 D_00155d89;
extern u8 D_001553b4;
extern u16 D_0012b8c8[];
extern u16 D_0012b8e4[];
extern u16 D_0015fc20[];
extern u8 D_0012ddbc;
extern u8 D_0012ddbe;
extern u8 D_0012ddbf;
extern u8 D_0012ddcc;
extern u8 D_0012ddce;
extern u16 D_0012dd7c[];
extern PeepType D_0012b7d4[];
extern PeepPalette D_0012b818[];
extern u8 D_0012b878[][5];

s8 fn_000856c8(int player);
void fn_00070e50(PeepBid *bids);
void fn_000444e0(Thing *w);

}

u8 create_an_illness(void);
s8 who_owns_peep(u8 illness);
void add_contagiousness(Thing *w);
Thing *make_a_peep(u16 pos, u8 illness, u8 owner);
void process_computer_peep(Thing *w, u8 player);

// FUNCTION: 0x0006ff30
void calc_visitors_this_month(void)
{
    s16 i;
    u16 unused;

    unused = 0;
    for (i = 0; i < 30; i++) {
        if (popn[i].month == D_00155d89) {
            D_00155385 = popn[i].count;
            break;
        }
        if (popn[i].month > D_00155d89)
            break;
    }
    D_00155387 += D_00155385;
    if (D_00155387)
        gamevar.f1c = 1500 / D_00155387;
    else
        gamevar.f1c = 0;
}

// FUNCTION: 0x0006ffec
void create_a_peep(void)
{
    s8 i;
    u8 illness;
    Thing *t;
    Thing *p;
    AllHosps *h;
    RoomInfo *room;
    s32 n;

    n = 1;
    if (statvar.unknownfd > 1)
        n = 4;
    if (D_001553b4)
        return;
    for (i = 0; i < n; i++) {
        if (statvar.unknownfd > 1 && D_001609c4[i].active == 0)
            continue;
        h = &allhosps[i];
        if (h->f1e3 > 0 && h->f1e1 == 1) {
            p = make_a_peep(h->f1d7, h->f1e4, i);
            if (p) {
                p->unknown169 = p->style;
                p->unknown171 = p->unknown169;
                if (shall_peep_pay(p))
                    pay_for_treatment(p, 3);
                else
                    p->unknown171 = 0;
                p->kind = 0x26;
                p->unknown18 = 0x20;
                p->anim_speed = 1;
                p->flags |= 0x40;
                h->f1ed++;
                h->f1cd++;
                set_icon_value(p, 6);
            }
            h->f1e3--;
            if (h->f1e3 == 0) {
                t = &thing[h->f1df];
                if (t > thing)
                    t->kind = 6;
                h->f1e1 = 0;
            }
        }
    }
    if (gamevar.f1c) {
        if (actual_game_turn % gamevar.f1c >= 25)
            return;
        illness = create_an_illness();
        i = who_owns_peep(illness);
        if (i >= 0) {
            h = &allhosps[i];
            if (h->f2a == 1) {
                room = &room_info[h->unknown1cf[old_newrand() & 3]];
                p = make_a_peep(room->unknown4, illness, i);
                if (p)
                    start_person_going(p);
            } else {
                p = make_a_peep(0, illness, i);
                if (p) {
                    process_computer_peep(p, i);
                    remove_thing(p);
                }
            }
            gamevar.f1a++;
            h->f1ed++;
            h->f1cd++;
        }
    }
}

// FUNCTION: 0x000702c4
u8 create_an_illness(void)
{
    u8 illness;
    u8 ok;
    u16 i;
    u16 r;

    illness = 0;
    ok = 0;
    if (towns[myhosp->f29].unknown4[3] && old_newrand() % towns[myhosp->f29].unknown4[3] == 0) {
        ok = 1;
        if (*(u16 *)&expertise[58].unknown8 && D_00155d89 < *(u16 *)&expertise[58].unknown8)
            ok = 0;
        else if (*(u16 *)&expertise[58].unknown10[0] && gamevar.f1a < *(u16 *)&expertise[58].unknown10[0])
            ok = 0;
    }
    if (ok && statvar.f8) {
        r = old_newrand() % statvar.f8;
        for (i = 0; i < 14; i++) {
            if (r < D_0012b8c8[i]) {
                if (D_00155d89 < D_0015fc20[i])
                    ok = 0;
                else
                    illness = i + 2;
                break;
            }
        }
    }
    if (ok == 0) {
        r = old_newrand() % statvar.fa;
        for (i = 0; i < 20; i++) {
            if (r < D_0012b8e4[i]) {
                illness = i + 0x10;
                break;
            }
        }
    }
    return illness;
}

// NONMATCHING: the target loads D_0012ddbc before calling old_newrand() for the % test
// FUNCTION: 0x00070458
s8 who_owns_peep(u8 illness)
{
    s32 best;
    s8 owner;
    s32 players;
    s8 i;
    PeepBid bids[4];
    s32 price[4];
    s32 pct[4];
    s32 total;
    s32 j;
    s32 r;

    best = -9999;
    owner = -1;
    if (statvar.unknownfd > 1)
        players = network_num_players();
    else
        players = 4;
    if (D_0012ddbf && D_00155d89 < D_0012ddbf) {
        owner = D_00155389 % players;
        D_00155389++;
        if (statvar.unknownfd > 1)
            owner = fn_000856c8(owner + 1);
    } else if (old_newrand() % D_0012ddbc == 0) {
        owner = old_newrand() % players;
        if (statvar.unknownfd > 1)
            owner = fn_000856c8(owner + 1);
    } else {
        for (i = 0; i < 4; i++) {
            bids[i].player = i;
            bids[i].score = 0;
            if (statvar.unknownfd > 1) {
                if (D_001609c4[i].active == 0)
                    continue;
            }
            bids[i].score += allhosps[i].rooms[illness].unknown0 * D_0012ddbe;
            update_single_cure_cost(illness, i);
            price[i] = max_price_for_treatment(illness, i);
            pct[i] = price[i] * 100 / allhosps[i].rooms[illness].unknownC;
            if (pct[i] < 50)
                pct[i] = 50;
            else if (pct[i] > 150)
                pct[i] = 150;
            bids[i].score = bids[i].score * pct[i] / 100;
        }
        fn_00070e50(bids);
        total = 0;
        for (j = 0; j < 4; j++) {
            total += bids[j].score;
            bids[j].score = total;
        }
        if (total) {
            r = old_newrand() % total;
            for (j = 0; j < 4; j++) {
                if (bids[j].score > r) {
                    owner = bids[j].player;
                    break;
                }
            }
        }
        if (owner < 0) {
            owner = old_newrand() % players;
            if (statvar.unknownfd > 1)
                owner = fn_000856c8(owner + 1);
        }
    }
    if (statvar.unknownfd > 1 ? 1 : allhosps[owner].unknown1256)
        return owner;
    return -1;
}

// FUNCTION: 0x000707f4
void add_contagiousness(Thing *w)
{
    u8 chance;
    u8 boost;

    if (!expertise[w->style].unknown10[1]) {
        chance = expertise[w->style].unknown10[0];
        if (chance) {
            boost = 0;
            if (expertise[58].f2 && D_00155d89 < expertise[58].f2)
                boost = 1;
            else if (*(u16 *)&expertise[58].kind && gamevar.f1a < *(u16 *)&expertise[58].kind)
                boost = 1;
            if (boost)
                chance = chance * *(u16 *)&expertise[58].unknown5[1] / 100;
            if (allhosps[w->owner].f20d)
                chance = 0;
            if (chance && old_newrand() % chance == 0)
                w->unknown174 = old_newrand() % expertise[58].unknown0 + 1;
        }
    }
}

// FUNCTION: 0x00070918
Thing *make_a_peep(u16 pos, u8 illness, u8 owner)
{
    Thing *p;
    s32 total;
    s32 i;
    s32 r;
    s32 k;
    PeepPalette *pal;
    s32 n;
    PeepChoice choices[10];
    u32 pick;
    s32 j;

    p = allocate_thing(0x32);
    if (p) {
        p->owner = owner;
        p->renderer = 2;
        p->x.tile = pos % 128;
        p->y.tile = pos / 128;
        p->x.frac = 0x80;
        p->y.frac = 0x80;
        p->unknown19 = p->x.tile;
        p->unknown20 = p->y.tile;
        p->unknown18 = D_0012ddcc;
        p->anim_speed = D_0012ddce;
        if (illness >= 16) {
            total = 0;
            for (i = 4; i >= 0; i--)
                total += D_0012b7d4[i].weight;
            r = old_newrand() % total;
            for (i = 4; i >= 0; i--) {
                total -= D_0012b7d4[i].weight;
                if (r >= total)
                    break;
            }
            p->subtype = D_0012b7d4[i].kind;
            k = old_newrand() % D_0012b7d4[i].count;
            p->layers[0] = D_0012b7d4[i].head[k];
            pal = &D_0012b818[D_0012b7d4[i].palette[k]];
        } else {
            n = 0;
            for (i = 4; i >= 0; i--) {
                if (illness >= 16 || D_0012b878[illness][i]) {
                    for (k = D_0012b7d4[i].count - 1; k >= 0; k--) {
                        choices[n].kind = D_0012b7d4[i].kind;
                        choices[n].head = D_0012b7d4[i].head[k];
                        choices[n].palette = D_0012b7d4[i].palette[k];
                        n++;
                    }
                }
            }
            pick = old_newrand() % n;
            p->subtype = choices[pick].kind;
            p->layers[0] = choices[pick].head;
            pal = &D_0012b818[choices[pick].palette];
        }
        if (statvar.unknownfd > 1 && pal->count1 == 4)
            p->unknown30[0] = pal->layer1[owner];
        else
            p->unknown30[0] = pal->layer1[old_newrand() % pal->count1];
        p->unknown30[1] = pal->layer2[old_newrand() % pal->count2];
        p->unknown30[2] = pal->layer3[old_newrand() % pal->count3];
        p->unknown30[3] = 0;
        effect_illness(p, illness, 0);
        add_contagiousness(p);
        p->unknown24 = 0;
        {
            u16 idx;

            idx = check_frame(anim_starts[p->subtype].idle, p) + p->unknown17;
            p->sprite = start[idx].frame;
            p->palette = start[idx].flags > 0 ? (u8)1 : (u8)0;
        }
        p->unknown81 = 100;
        p->unknown83w = 50;
        p->unknown134 = 100;
        if (illness == 0x1d)
            p->unknown136 = (s16)D_0012dd7c[13] / 2;
        else
            p->unknown136 = 0;
        p->unknown130 = old_normal_rand(200, 1000);
        for (j = 0; j < 10; j++)
            p->unknown158[j] = p->unknown130;
        p->unknown171 = 1;
        p->unknown169 = 1;
        p->unknown144 = old_normal_rand(1, *(u16 *)&expertise[illness].unknown10[6]);
        if (old_newrand() % *(u16 *)&towns[myhosp->f29].unknown4[0xe] == 0)
            p->unknown172[1] = 1 << old_newrand() % 3;
        else
            p->unknown172[1] = 0;
        p->f70 = old_newrand() % 95000 + 5000;
        if (allhosps[owner].f2a == 1) {
            fn_000444e0(p);
            insert_node(p, p->x, p->y);
        }
        return p;
    }
    return 0;
}

// FUNCTION: 0x00070e50
void fn_00070e50(PeepBid *bids)
{
    u8 swapped;
    s32 tmp;
    s32 i;

    swapped = 1;
    while (swapped) {
        swapped = 0;
        for (i = 0; i < 2; i++) {
            if (bids[i].score > bids[i + 1].score) {
                tmp = bids[i].score;
                bids[i].score = bids[i + 1].score;
                bids[i + 1].score = tmp;
                tmp = bids[i].player;
                bids[i].player = bids[i + 1].player;
                bids[i + 1].player = tmp;
                swapped = 1;
                break;
            }
        }
    }
}
