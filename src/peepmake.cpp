#include "hosp.h"

/* One bidder in who_owns_peep: a weighted score and the hospital it is for. */
struct PeepBid {
    signed int score : 27;
    u8 player : 5;
};

/* A kind of visitor: its sprite set, head and palette choices, and weight. */
struct PeepChoice {
    u8 kind;
    u8 head;
    u8 palette;
};

extern "C" {

extern u16 visuals_available[];
extern PeepType D_0012b7d4[];
extern PeepPalette D_0012b818[];

s8 nth_active_player(int player);
void sort_peep_bids(PeepBid *bids);
void nav_init(Thing *w);

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
        if (popn[i].month == othervar.f9d1) {
            gamevar.f145 = popn[i].count;
            break;
        }
        if (popn[i].month > othervar.f9d1)
            break;
    }
    gamevar.f147 += gamevar.f145;
    if (gamevar.f147)
        gamevar.f1c = 1500 / gamevar.f147;
    else
        gamevar.f1c = 0;
}

// FUNCTION: 0x0006ffec
void create_a_peep(void)
{
    s8 i;
    u8 illness;
    Thing *vehicle;
    Thing *patient;
    AllHosps *hosp;
    RoomInfo *room;
    s32 nplayers;

    nplayers = 1;
    if (statvar.unknownfd > 1)
        nplayers = 4;
    if (gamevar.f174)
        return;
    for (i = 0; i < nplayers; i++) {
        if (statvar.unknownfd > 1 && D_001609c4[i].active == 0)
            continue;
        hosp = &allhosps[i];
        if (hosp->f1e3 > 0 && hosp->f1e1 == 1) {
            patient = make_a_peep(hosp->f1d7, hosp->f1e4, i);
            if (patient) {
                patient->known_illness = patient->illness;
                patient->treatment = patient->known_illness;
                if (shall_peep_pay(patient))
                    pay_for_treatment(patient, 3);
                else
                    patient->treatment = 0;
                patient->state = PS_GO_FOR_TREATMENT;
                patient->speed = 0x20;
                patient->anim_speed = 1;
                patient->flags |= PF_EMERGENCY;
                hosp->f1ed++;
                hosp->f1cd++;
                set_icon_value(patient, ICON_EMERGENCY);
            }
            hosp->f1e3--;
            if (hosp->f1e3 == 0) {
                vehicle = &thing[hosp->f1df];
                if (vehicle > thing)
                    vehicle->state = HELI_LEAVES;
                hosp->f1e1 = 0;
            }
        }
    }
    if (gamevar.f1c) {
        if (actual_game_turn % gamevar.f1c >= 25)
            return;
        illness = create_an_illness();
        i = who_owns_peep(illness);
        if (i >= 0) {
            hosp = &allhosps[i];
            if (hosp->f2a == 1) {
                room = &room_info[hosp->unknown1cf[old_newrand() & 3]];
                patient = make_a_peep(room->unknown4, illness, i);
                if (patient)
                    start_person_going(patient);
            } else {
                patient = make_a_peep(0, illness, i);
                if (patient) {
                    process_computer_peep(patient, i);
                    remove_thing(patient);
                }
            }
            gamevar.f1a++;
            hosp->f1ed++;
            hosp->f1cd++;
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
        if (gbv.fd6 && othervar.f9d1 < gbv.fd6)
            ok = 0;
        else if (gbv.fd8 && gamevar.f1a < gbv.fd8)
            ok = 0;
    }
    if (ok && statvar.f8) {
        r = old_newrand() % statvar.f8;
        for (i = 0; i < 14; i++) {
            if (r < visuals[i]) {
                if (othervar.f9d1 < visuals_available[i])
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
            if (r < non_visuals[i]) {
                illness = i + 0x10;
                break;
            }
        }
    }
    return illness;
}

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
    s32 roll;

    best = -9999;
    owner = -1;
    if (statvar.unknownfd > 1)
        players = network_num_players();
    else
        players = 4;
    if (gbv.f13f && othervar.f9d1 < gbv.f13f) {
        owner = gamevar.f149 % players;
        gamevar.f149++;
        if (statvar.unknownfd > 1)
            owner = nth_active_player(owner + 1);
    } else if (old_newrand() % gbv.f13c == 0) {
        owner = old_newrand() % players;
        if (statvar.unknownfd > 1)
            owner = nth_active_player(owner + 1);
    } else {
        for (i = 0; i < 4; i++) {
            bids[i].player = i;
            bids[i].score = 0;
            if (statvar.unknownfd > 1) {
                if (D_001609c4[i].active == 0)
                    continue;
            }
            bids[i].score += allhosps[i].rooms[illness].unknown0 * gbv.f13e;
            update_single_cure_cost(illness, i);
            price[i] = max_price_for_treatment(illness, i);
            pct[i] = price[i] * 100 / allhosps[i].rooms[illness].unknownC;
            if (pct[i] < 50)
                pct[i] = 50;
            else if (pct[i] > 150)
                pct[i] = 150;
            bids[i].score = bids[i].score * pct[i] / 100;
        }
        sort_peep_bids(bids);
        total = 0;
        for (j = 0; j < 4; j++) {
            total += bids[j].score;
            bids[j].score = total;
        }
        if (total) {
            roll = old_newrand() % total;
            for (j = 0; j < 4; j++) {
                if (bids[j].score > roll) {
                    owner = bids[j].player;
                    break;
                }
            }
        }
        if (owner < 0) {
            owner = old_newrand() % players;
            if (statvar.unknownfd > 1)
                owner = nth_active_player(owner + 1);
        }
    }
    if (statvar.unknownfd > 1 ? 1 : allhosps[owner].unknown1256)
        return owner;
    return -1;
}

// FUNCTION: 0x000707f4
void add_contagiousness(Thing *patient)
{
    u8 chance;
    u8 boost;

    if (!expertise[patient->illness].unknown10[1]) {
        chance = expertise[patient->illness].unknown10[0];
        if (chance) {
            boost = 0;
            if (gbv.fd0 && othervar.f9d1 < gbv.fd0)
                boost = 1;
            else if (gbv.fd2 && gamevar.f1a < gbv.fd2)
                boost = 1;
            if (boost)
                chance = chance * gbv.fd4 / 100;
            if (allhosps[patient->owner].f20d)
                chance = 0;
            if (chance && old_newrand() % chance == 0)
                patient->contagion = old_newrand() % gbv.fce + 1;
        }
    }
}

// FUNCTION: 0x00070918
Thing *make_a_peep(u16 pos, u8 illness, u8 owner)
{
    Thing *peep;
    s32 total;
    s32 i;
    s32 r;
    s32 k;
    PeepPalette *pal;
    s32 nchoices;
    PeepChoice choices[10];
    u32 pick;
    s32 j;

    peep = allocate_thing(0x32);
    if (peep) {
        peep->owner = owner;
        peep->type = THING_PATIENT;
        peep->x.tile = pos % 128;
        peep->y.tile = pos / 128;
        peep->x.frac = 0x80;
        peep->y.frac = 0x80;
        peep->dest_x = peep->x.tile;
        peep->dest_y = peep->y.tile;
        peep->speed = gbv.f14c;
        peep->anim_speed = gbv.f14e;
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
            peep->subtype = D_0012b7d4[i].kind;
            k = old_newrand() % D_0012b7d4[i].count;
            peep->layers[0] = D_0012b7d4[i].head[k];
            pal = &D_0012b818[D_0012b7d4[i].palette[k]];
        } else {
            nchoices = 0;
            for (i = 4; i >= 0; i--) {
                if (illness >= 16 || suitable_illness[illness][i]) {
                    for (k = D_0012b7d4[i].count - 1; k >= 0; k--) {
                        choices[nchoices].kind = D_0012b7d4[i].kind;
                        choices[nchoices].head = D_0012b7d4[i].head[k];
                        choices[nchoices].palette = D_0012b7d4[i].palette[k];
                        nchoices++;
                    }
                }
            }
            pick = old_newrand() % nchoices;
            peep->subtype = choices[pick].kind;
            peep->layers[0] = choices[pick].head;
            pal = &D_0012b818[choices[pick].palette];
        }
        if (statvar.unknownfd > 1 && pal->count1 == 4)
            peep->layers[1] = pal->layer1[owner];
        else
            peep->layers[1] = pal->layer1[old_newrand() % pal->count1];
        peep->layers[2] = pal->layer2[old_newrand() % pal->count2];
        peep->layers[3] = pal->layer3[old_newrand() % pal->count3];
        peep->layers[4] = 0;
        effect_illness(peep, illness, 0);
        add_contagiousness(peep);
        peep->icon_id = 0;
        {
            u16 idx;

            idx = check_frame(anim_starts[peep->subtype].idle, peep) + peep->facing;
            peep->sprite = start[idx].frame;
            peep->flip = start[idx].flags > 0 ? (u8)1 : (u8)0;
        }
        peep->happiness = 100;
        peep->warmth = 50;
        peep->refreshment = 100;
        if (illness == ILL_THE_SQUITS)
            peep->bladder = gbv.f116 / 2;
        else
            peep->bladder = 0;
        peep->health = old_normal_rand(200, 1000);
        for (j = 0; j < 10; j++)
            peep->health_history[j] = peep->health;
        peep->treatment = ILL_GENERAL_PRACTICE;
        peep->known_illness = 1;
        peep->diagnosis_handicap = old_normal_rand(1, *(u16 *)&expertise[illness].unknown10[6]);
        if (old_newrand() % *(u16 *)&towns[myhosp->f29].unknown4[0xe] == 0)
            peep->insurance = 1 << old_newrand() % 3;
        else
            peep->insurance = 0;
        peep->cash = old_newrand() % 95000 + 5000;
        if (allhosps[owner].f2a == 1) {
            nav_init(peep);
            insert_node(peep, peep->x, peep->y);
        }
        return peep;
    }
    return 0;
}

// FUNCTION: 0x00070e50
void sort_peep_bids(PeepBid *bids)
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
