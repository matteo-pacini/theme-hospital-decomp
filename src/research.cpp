#include "hosp.h"

extern "C" {

extern s32 D_0015f8e1;
extern s32 D_0015f8e5;
extern u16 D_00155376;
extern u8 D_0012ddc0;
extern u16 D_0012dd74;
extern u16 D_0012ddb8;
extern u8 D_0012ddda;
extern u8 D_0012dddb;
extern u8 D_00155e3c;
extern u16 D_00155e3d;

}

void staff_exit_room_check(Thing *w);
void item_researched(u16 item, u16 illness, u8 owner);
void drug_improved(u16 illness, u8 owner);
void drug_researched(u16 illness, u8 owner);
void item_upgraded(u16 item, u8 owner);
u16 emptiest_research_room(u8 owner);
u16 find_rand_research_room(u8 owner, u16 exclude);
u8 find_best_drug_rating(u8 illness, u8 owner);
s32 find_new_cure_cost(u8 illness, u8 owner);
s32 find_new_diag_cost(u8 illness, u8 owner);
s32 find_new_drug_cost(u8 illness, u8 owner);
s32 find_drug_improve_cost(u8 illness, u8 owner);
s32 find_hw_improve_cost(u8 illness, u8 owner);
u8 choose_next_research_illness(u8 owner, s32 (*cost)(u8, u8));
void research_category_exhausted(const u8 owner, const u8 category);
void do_illness_research(u8 owner, u8 illness, u32 amount, u8 category);

// FUNCTION: 0x0006cf80
void do_research(void)
{
    AllHosps *h;
    s32 amount;
    u16 category;
    u16 unused = 0;
    u8 illness;
    u8 owner = 0;
    u8 exhausted;

    for (owner = 0; owner < 4; owner++) {
        h = &allhosps[owner];
        for (category = 0; category < 5; category++) {
            amount = h->research_alloc[category] * h->research_points / 100;
            if (h->research_alloc[category] > 0 && amount > 0) {
                h->research_points -= amount;
                D_0015f8e5 += amount;
                exhausted = 0;
                switch (category) {
                case 0:
                    illness = choose_next_research_illness(owner, find_new_cure_cost);
                    if (illness) {
                        do_illness_research(owner, illness, amount, 0);
                    } else {
                        research_category_exhausted(owner, 0);
                        exhausted = 1;
                    }
                    break;
                case 1:
                    illness = choose_next_research_illness(owner, find_new_diag_cost);
                    if (illness) {
                        do_illness_research(owner, illness, amount, 1);
                    } else {
                        research_category_exhausted(owner, 1);
                        exhausted = 1;
                    }
                    break;
                case 2:
                    illness = choose_next_research_illness(owner, find_new_drug_cost);
                    if (illness) {
                        do_illness_research(owner, illness, amount, 2);
                    } else {
                        illness = choose_next_research_illness(owner, find_drug_improve_cost);
                        if (illness)
                            do_illness_research(owner, illness, amount, 2);
                        else
                            exhausted = 1;
                    }
                    break;
                case 3:
                    illness = choose_next_research_illness(owner, find_hw_improve_cost);
                    if (illness)
                        do_illness_research(owner, illness, amount, 3);
                    else
                        exhausted = 1;
                    break;
                case 4:
                    if ((statvar.unknownfd > 1 ? allhosps[owner].unknown1256 : D_00155376) != 0)
                        do_illness_research(owner, statvar.unknownfd > 1 ? allhosps[owner].unknown1256 : D_00155376, amount, 4);
                    break;
                }
                if (exhausted) {
                    h->research_points += amount;
                    D_0015f8e5 -= amount;
                }
            }
        }
    }
}

// FUNCTION: 0x0006d288
void item_researched(u16 item, u16 illness, u8 owner)
{
    u8 pay;
    u8 i;
    u16 date;

    pay = 1;
    if (statvar.unknownfd > 1) {
        date = mycontrol->f648 * 12 + mycontrol->unknown650;
        for (i = 0; i < 4; i++) {
            if (allhosps[i].equipment[item].unknown0 == 1 && objects[item].unknown1c == 1)
                pay = 0;
            if (owner != i) {
                if (allhosps[i].object_stats[item].available == 0)
                    allhosps[i].object_stats[item].available = date + 3;
            }
        }
        if (pay) {
            change_balance(0x15, 500, 0, (u16)owner);
            if (illness < 0x2c)
                reputation_affected(illness, 9, 0, owner, 0);
        }
    }
    allhosps[owner].equipment[item].unknown0 = 1;
    if (owner == player)
        announce_this(5, item);
}

// FUNCTION: 0x0006d428
void drug_improved(u16 illness, u8 owner)
{
    u16 effect;
    u16 unused;
    u16 cost;
    u16 done;
    AllHosps *h;

    effect = 0;
    unused = 0;
    cost = 999;
    done = 0;
    h = &allhosps[owner];
    if (!(old_newrand() & 1)) {
        cost = h->rooms[illness].drug.cost;
        if (cost > 10) {
            cost = cost * 95 / 100;
            cost < 10 ? cost = 10 : 0;
            h->rooms[illness].drug.cost = cost;
            if (old_newrand() % 7 == 0) {
                h->rooms[illness].drug.effectiveness += D_0012ddc0;
                h->rooms[illness].drug.effectiveness > 100 ? h->rooms[illness].drug.effectiveness = 100 : 0;
            }
            done = 1;
        }
    } else {
        effect = h->rooms[illness].drug.effectiveness;
        if (effect < 100) {
            effect += D_0012ddc0;
            h->rooms[illness].drug.effectiveness = effect;
            h->rooms[illness].drug.effectiveness > 100 ? h->rooms[illness].drug.effectiveness = 100 : 0;
            if (old_newrand() % 7 == 0) {
                h->rooms[illness].drug.cost--;
                h->rooms[illness].drug.cost > 10 ? h->rooms[illness].drug.cost = 10 : 0;
            }
            done = 1;
        }
    }
    if (owner == player && done)
        announce_this(7, illness);
}

// FUNCTION: 0x0006d6a0
void drug_researched(u16 illness, u8 owner)
{
    AllHosps *h;

    h = &allhosps[owner];
    h->rooms[illness].drug.effectiveness = expertise[57].unknown10[3];
    h->rooms[illness].drug.cost = *(u16 *)&expertise[57].unknown10[5];
    allhosps[owner].rooms[illness].unknown1C = 1;
    if (owner == player)
        announce_this(6, illness);
}

// NONMATCHING: case 4 builds the -= target address as item * 14 + h in ebx; here h + (item * 14)
// FUNCTION: 0x0006d748
void item_upgraded(u16 item, u8 owner)
{
    AllHosps *h;

    h = &allhosps[owner];
    switch (h->equipment[item].unknown1[0] % 6) {
    case 0:
        break;
    case 2:
        if (h->object_stats[item].unknown8 < *(u16 *)&expertise[59].unknown10[7])
            h->object_stats[item].unknown8 += D_0012dd74;
        break;
    case 4:
        h->object_stats[item].cost = (u32)h->object_stats[item].cost * 7 / 8;
        h->object_stats[item].cost -= (u32)h->object_stats[item].cost % 10;
        if (objects[item].cost / 3 > (u32)h->object_stats[item].cost)
            h->object_stats[item].cost = objects[item].cost / 3;
        break;
    }
    h->equipment[item].unknown1[0] += 2;
    if (owner == player)
        announce_this(8, item);
}

// FUNCTION: 0x0006d8fc
void auto_available(void)
{
    u8 owner;
    u8 item;
    u16 date;
    AllHosps *h;

    date = mycontrol->f648 * 12 + mycontrol->unknown650;
    for (owner = 0; owner < 4; owner++) {
        h = &allhosps[owner];
        for (item = 0; item < 0x3e; item++) {
            if (allhosps[owner].equipment[item].unknown0 == 0 && objects[item].unknown1c == 1) {
                if (h->object_stats[item].available != 0 && date >= h->object_stats[item].available) {
                    allhosps[owner].equipment[item].unknown0 = 1;
                    if (owner == player)
                        announce_this(9, item);
                }
            }
        }
    }
}

// FUNCTION: 0x0006da40
void research_contribution(Thing *w, s16 amount)
{
    if (w->f78 > *(u16 *)&expertise[59].category)
        amount += w->unknown126 / 4;
    else if (w->f78 > *(u16 *)&expertise[59].unknown7)
        amount += w->unknown126 / 2;
    else
        amount += w->unknown126;
    if (w->subtype == 1 && w->unknown148[2])
        amount = -amount / 2;
    allhosps[w->owner].research_points += amount;
    D_0015f8e1 += amount;
    if (allhosps[w->owner].research_points < 0)
        allhosps[w->owner].research_points = 0;
}

// FUNCTION: 0x0006db78
void send_person_for_research(Thing *w)
{
    u16 idx;
    RoomInfo *room;

    room = get_expertise_dest(0x2c, w);
    room->unknown32++;
    page_staff_i_need(w, room);
    w->unknown89 = room - room_info;
    idx = check_frame(anim_starts[w->subtype].idle, w) + w->unknown17;
    w->sprite = start[idx].frame;
    w->palette = start[idx].flags > 0 ? (u8)1 : (u8)0;
    w->kind = 0x24;
    fn_0004195c(w, room->unknown4);
}

// FUNCTION: 0x0006dc64
u16 emptiest_research_room(u8 owner)
{
    s32 best;
    u32 found;
    RoomInfo *room;
    s32 count;
    Thing *t;
    u8 k;

    best = -10000;
    found = 0;
    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->type == 0x1c && room->owner == owner) {
            count = 0;
            for (t = &thing[room->first_object]; t > thing; t = &thing[t->unknown71]) {
                for (k = 0; k < research_workstations.count; k++) {
                    if (t->subtype == research_workstations.kinds[k])
                        count++;
                }
            }
            for (t = &thing[room->first]; t > thing; t = &thing[t->room_next]) {
                if (t->renderer == 4)
                    count--;
            }
            if (count > best) {
                best = count;
                found = room - room_info;
            }
        }
    }
    return found;
}

// FUNCTION: 0x0006ddf0
u16 find_rand_research_room(u8 owner, u16 exclude)
{
    u8 n;
    RoomInfo *room;
    u16 list[30];

    n = 0;
    for (room = room_info + 1; room < end_room_info; room++) {
        if ((room->type == 0x1c && room->owner == owner) && room - room_info != exclude) {
            list[n++] = room - room_info;
            if (n == 30)
                break;
        }
    }
    if (n == 0)
        return 0;
    return list[old_newrand() % n];
}

// FUNCTION: 0x0006dedc
u8 send_doctor_to_research(Thing *w)
{
    u16 id;

    if (w->room != 0 && w->room->type == 0x1c)
        id = find_rand_research_room(w->owner, w->room - room_info);
    else
        id = emptiest_research_room(w->owner);
    if (!id)
        return 0;
    staff_exit_room_check(w);
    if (w->room)
        set_complex(w, 0x136, w->room->unknown6);
    set_complex(w, 0x96, room_info[id].unknown4);
    w->state_b = 0x3e;
    w->state_a = 0x36;
    w->unknown89 = id;
    fn_0003e724(w);
    return 1;
}

// FUNCTION: 0x0006dfe8
void person_autopsied(Thing *w)
{
    u8 bad;
    Thing *t;
    AllHosps *h;
    s32 count;

    h = &allhosps[w->owner];
    count = 0;
    for (t = thing + 1; t < end_things; t++) {
        if ((t->renderer == 4 && t->kind != 0x2e) && t->owner == w->owner)
            count += t->unknown81s;
    }
    do_illness_research(w->owner, w->style, D_0012ddb8 * expertise[w->style].cost / 100, 4);
    if ((count << 8) / h->fc9c < 0x1900)
        bad = old_newrand() % 3 != 0;
    else
        bad = old_newrand() % 3 == 0;
    if (bad) {
        reputation_affected(0, 0x13, 0, w->owner, 0);
        if (w->owner == player)
            announce_this(2, 1);
    }
}

// FUNCTION: 0x0006e184
u8 find_best_drug_rating(u8 illness, u8 owner)
{
    return allhosps[owner].rooms[illness].drug.effectiveness;
}

// FUNCTION: 0x0006e1c8
s32 find_new_cure_cost(u8 illness, u8 owner)
{
    if ((allhosps[owner].equipment[expertise[illness].kind].unknown0 == 0 && objects[expertise[illness].kind].unknown1c == 1)
        && expertise[illness].unknown5[0] == 2)
        return expertise[illness].cost;
    return -1;
}

// FUNCTION: 0x0006e27c
s32 find_new_diag_cost(u8 illness, u8 owner)
{
    if ((allhosps[owner].equipment[expertise[illness].kind].unknown0 == 0 && objects[expertise[illness].kind].unknown1c == 1)
        && expertise[illness].unknown5[0] == 3)
        return expertise[illness].cost;
    return -1;
}

// FUNCTION: 0x0006e330
s32 find_new_drug_cost(u8 illness, u8 owner)
{
    if ((expertise[illness].kind == 0x27 && allhosps[owner].rooms[illness].unknown1D == 1)
        && allhosps[owner].rooms[illness].unknown1C == 0)
        return expertise[illness].cost;
    return -1;
}

// FUNCTION: 0x0006e3e0
s32 find_drug_improve_cost(u8 illness, u8 owner)
{
    u8 rating;

    if ((expertise[illness].kind == 0x27 && allhosps[owner].rooms[illness].unknown1D == 1)
        && allhosps[owner].rooms[illness].unknown1C == 1) {
        rating = find_best_drug_rating(illness, owner);
        if (rating < 100)
            return (D_0012ddda + D_0012dddb * (rating / 25)) * expertise[illness].cost / 100;
    }
    return -1;
}

// FUNCTION: 0x0006e4dc
s32 find_hw_improve_cost(u8 illness, u8 owner)
{
    u8 kind;
    s32 level;

    kind = expertise[illness].kind;
    if ((allhosps[owner].equipment[kind].unknown0 == 1 && objects[kind].unknown1c == 1)
        && (expertise[illness].unknown5[0] == 2 || expertise[illness].unknown5[0] == 3)
        && allhosps[owner].equipment[kind].unknown1[0] < 40) {
        level = allhosps[owner].equipment[kind].unknown1[0] / 2;
        return (D_0012ddda + D_0012dddb * level) * expertise[illness].cost / 100;
    }
    return -1;
}

// FUNCTION: 0x0006e624
u8 choose_next_research_illness(u8 owner, s32 (*cost)(u8, u8))
{
    u8 found;
    u32 best;
    s32 i;
    s32 c;

    found = 0;
    best = -1;
    for (i = 0x2e; i >= 0; i--) {
        c = cost(i, owner);
        if (c > 0) {
            if (c < best) {
                best = c;
                found = i;
            }
        }
    }
    return found;
}

// FUNCTION: 0x0006e694
void research_category_exhausted(const u8 owner, const u8 category)
{
    AllHosps *h;

    h = &allhosps[owner];
    h->research_alloc[category] = -1;
    if (owner == player) {
        announce_this(3, category);
        D_00155e3c = 1;
        D_00155e3d = 10;
        play_sound(10, 0xdb, -1, -1);
    }
}

// NONMATCHING: in the drug branch the target loads D_0012dddb (into ecx) before dividing rating by 25
// FUNCTION: 0x0006e720
void do_illness_research(u8 owner, u8 illness, u32 amount, u8 category)
{
    AllHosps *h;
    s32 i;
    s32 cost;
    s32 rating;
    u8 kind;
    s32 level;

    h = &allhosps[owner];
    kind = expertise[illness].kind;
    if (expertise[illness].unknown5[0] == 2 || expertise[illness].unknown5[0] == 3) {
        for (i = 0; i < 0x2f; i++) {
            if (expertise[i].kind == expertise[illness].kind && expertise[i].cost != 0) {
                illness = i;
                break;
            }
        }
    }
    h->illness_research[illness] += amount;
    if ((expertise[illness].unknown5[0] == 2 || expertise[illness].unknown5[0] == 3)
        && (allhosps[owner].equipment[kind].unknown0 == 0 && objects[kind].unknown1c == 1)) {
        cost = h->research_cost[category] = expertise[illness].cost;
        if (h->illness_research[illness] >= cost) {
            h->illness_research[illness] -= cost;
            item_researched(expertise[illness].kind, illness, owner);
        }
    } else if ((expertise[illness].kind == 0x27 && allhosps[owner].rooms[illness].unknown1D == 1)
               && allhosps[owner].rooms[illness].unknown1C == 0) {
        cost = h->research_cost[category] = expertise[illness].cost;
        if (h->illness_research[illness] >= cost) {
            h->illness_research[illness] -= cost;
            drug_researched(illness, owner);
        }
    } else if ((expertise[illness].kind == 0x27 && allhosps[owner].rooms[illness].unknown1D == 1)
               && allhosps[owner].rooms[illness].unknown1C == 1) {
        rating = find_best_drug_rating(illness, owner);
        cost = (D_0012ddda + D_0012dddb * (rating / 25)) * expertise[illness].cost / 100;
        if (h->illness_research[illness] >= cost) {
            h->illness_research[illness] -= cost;
            drug_improved(illness, owner);
        }
    } else if (((expertise[illness].unknown5[0] == 2 || expertise[illness].unknown5[0] == 3)
                && (allhosps[owner].equipment[kind].unknown0 == 1 && objects[kind].unknown1c == 1))
               && h->equipment[kind].unknown1[0] < 40) {
        level = h->equipment[kind].unknown1[0] / 2;
        cost = (D_0012ddda + D_0012dddb * level) * expertise[illness].cost / 100;
        if (h->illness_research[illness] >= cost) {
            h->illness_research[illness] -= cost;
            item_upgraded(kind, owner);
        }
    }
}
