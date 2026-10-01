#include "hosp.h"

extern "C" {
u8 fn_0007cbdc(RoomInfo *room, s32 pos, u8 owner);
#pragma aux fn_0007cbdc "W?trapped_plant_check$n(pn$RoomInfo$$l)uc"
u8 fn_0007cc88(Thing *w);
}

extern s16 special_around[];
extern Map *map;

u8 thingpage_cancel(ThingPager *p, u16 id);
u16 thingpage_respond(ThingPager *p, Thing *w, u16 room, u8 min);
u8 thingpage_find_highest_priority(ThingPager *p);
u16 thingpage_remind_me(ThingPager *p, Thing *w);
u8 handyman_try_fix(Thing *w);
u8 handyman_try_plant(Thing *w);
u8 handyman_try_litter(Thing *w);
u8 is_next_to_plant(Thing *w, Thing *plant);

void choose_aniseq(Thing *w, u8 anim, u8 speed);
void remove_litter(u16 tile, u8 owner);

#define TILE_POS(t) ((s16)(t)->x.tile + ((s16)(t)->y.tile << 7))

// FUNCTION: 0x0007a710
void place_litter(u16 tile, s8 owner, u8 type)
{
    Map *m;
    u8 cur;

    m = &map[tile];
    if (m->unknown7 == 1 << owner) {
        cur = m->floor & 0xf;
        if (cur == 0) {
            m->floor &= 0xf0;
            m->floor |= type;
            allhosps[owner].fcaa++;
            allhosps[owner].fcac = tile;
        } else if ((type == 5 || type == 4) && !(cur == 0 || cur == 7 || cur == 8)) {
            m->floor &= 0xf0;
            m->floor |= type;
            allhosps[owner].fcac = tile;
        }
    }
}

// FUNCTION: 0x0007a814
void remove_litter(u16 tile, u8 owner)
{
    Map *m;
    u8 cur;

    m = &map[tile];
    cur = m->floor & 0xf;
    if (m->unknown7 == 1 << owner && !(cur == 0 || cur == 7 || cur == 8)) {
        m->floor &= 0xf0;
        allhosps[owner].fcaa--;
    }
}

// FUNCTION: 0x0007a8a8
void handyman_works(Thing *handyman)
{
    u8 (*funcs[3])(Thing *);
    Thing *plant;
    Thing *machine;
    RoomInfo *room;
    u32 scores[3];
    s32 litter;
    s32 plant_score;
    s32 fix;
    u32 n;
    u32 idx;
    u32 best;
    s32 dx;
    s32 dy;
    u16 pos;
    u16 id;
    u8 i;
    u8 level;

    switch (handyman->unknown103) {
    case 0:
        if (shall_i_resign(handyman))
            break;
        if (((handyman->f8e & 1) || handyman->f78 > allhosps[handyman->owner].f21d) && take_a_break(handyman))
            break;
        room = handyman->room;
        if (room) {
            for (machine = &thing[handyman->room->first_object]; machine > thing; machine = &thing[machine->unknown71]) {
                if (machine->renderer == 6 && objects[machine->subtype].unknown24) {
                    if (machine->mode > 0) {
                        thingpage_request(&allhosps[machine->owner].ff6d, machine - thing, 100);
                        break;
                    }
                }
            }
            if (thingpage_respond(&allhosps[handyman->owner].ff6d, handyman, handyman->room - room_info, 0)) {
                handyman->unknown103 = 8;
                if (room->f3 != 6)
                    room->f3 = 5;
                break;
            }
            handyman->kind = 0x36;
            break;
        }
        if (thingpage_find_highest_priority(&allhosps[handyman->owner].ff6d) && handyman_try_fix(handyman))
            break;
        litter = handyman->f98 * 100 / (handyman->f9e + 1);
        plant_score = handyman->f9a * 1000 / (handyman->f9f + 1);
        fix = handyman->f9c * 2000 / (handyman->fa0 + 1);
        scores[0] = litter;
        funcs[0] = handyman_try_litter;
        scores[1] = plant_score;
        funcs[1] = handyman_try_plant;
        scores[2] = fix;
        funcs[2] = handyman_try_fix;
        best = 1000000;
        while (best != 2000000) {
            best = 2000000;
            for (n = 0; n < 3; n++) {
                if (scores[n] < best) {
                    best = scores[n];
                    idx = n;
                }
            }
            if (best < 1000000) {
                scores[idx] = 2000000;
                if (funcs[idx](handyman))
                    return;
            }
        }
        dx = old_newrand() % 15 - 7;
        dy = old_newrand() % 15 - 7;
        pos = (u16)handyman->x.tile + ((u16)handyman->y.tile << 7) + ((dy << 7) + dx);
        if (map[pos].unknown7 == 1 << handyman->owner && can_i_get_to(handyman, pos) && !(map[pos].unknown7 & 0x10)) {
            handyman->state_b = handyman->unknown94 = 0x3c;
            set_complex(handyman, 0xbe, (u16)handyman->x.tile + ((u16)handyman->y.tile << 7) + ((dy << 7) + dx));
            fn_0003e724(handyman);
        }
        break;
    case 1:
        break;
    case 2:
        if (handyman->x.tile != handyman->unknown19 || handyman->y.tile != handyman->unknown20) {
            handyman->unknown103 = 0;
            break;
        }
        if (handyman->unknown148w == TILE_POS(handyman))
            handyman->unknown148w = 0;
        handyman->unknown17 = showanim_angle[handyman->unknown17];
        handyman->state_b = 0x3c;
        handyman->unknown103 = 3;
        set_complex(handyman, 0x46, 0);
        fn_0003e724(handyman);
        break;
    case 3:
        remove_litter((u16)handyman->x.tile + ((u16)handyman->y.tile << 7), handyman->owner);
        set_complex(handyman, 0x30c, 0);
        handyman->state_b = 0x3c;
        fn_0003e724(handyman);
        handyman->unknown103 = 0;
        handyman->f98++;
        break;
    case 4:
        handyman->unknown69 = thingpage_remind_me(&allhosps[handyman->owner].fd2b, handyman);
        set_complex(handyman, 0x26c, 0);
        handyman->unknown103 = 5;
        handyman->state_b = 0x3c;
        handyman->unknown94 = 0x36;
        fn_0003e724(handyman);
        break;
    case 5:
        id = handyman->unknown69;
        plant = &thing[id];
        thingpage_cancel(&allhosps[plant->owner].fd2b, id);
        if (plant->unknown98 == 1 || plant->renderer != 6 || plant->subtype != 0x2d || plant->kind != 10
            || !is_next_to_plant(handyman, plant)) {
            handyman->kind = 0x36;
            break;
        }
        plant->unknown98 = 1;
        handyman->unknown17 = direction_to_face(plant, handyman);
        handyman->state_b = 0x3c;
        handyman->unknown94 = 0x36;
        handyman->unknown103 = 6;
        set_complex(handyman, 0x258, 0);
        fn_0003e724(handyman);
        plant->unknown100s < 0 ? plant->unknown100s = 0 : 0;
        level = (400 - plant->unknown100s) / 80;
        {
            u16 anim;

            anim = check_frame(0x3d0, plant) + (u16)plant->unknown17;
            plant->sprite = start[anim].frame;
            plant->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
        }
        for (i = 0; i < 5 - level - 1; i++)
            plant->sprite = frame[plant->sprite].next;
        plant->kind = 0xe;
        break;
    case 6:
        id = handyman->unknown69;
        plant = &thing[id];
        if (!(actual_game_turn & 1)) {
            plant->sprite = frame[plant->sprite].next;
            if (frame[plant->sprite].flags & 1) {
                plant->unknown98 = 0;
                {
                    u16 anim2;

                    anim2 = check_frame(0x3cf, plant) + (u16)plant->unknown17;
                    plant->sprite = start[anim2].frame;
                    plant->palette = start[anim2].flags > 0 ? (u8)1 : (u8)0;
                }
                plant->kind = 10;
                if (plant->owner == player) {
                    NRAdcounts.unknownb += 0x37 - (plant->unknown100s * 2 + 0x50) / 16;
                    if (NRAdcounts.unknownb < 0)
                        NRAdcounts.unknownb = 0;
                    else if (NRAdcounts.unknown0 * 0x37 < NRAdcounts.unknownb)
                        NRAdcounts.unknownb = NRAdcounts.unknown0 * 0x37;
                }
                plant->unknown100s = 400;
                handyman->unknown103 = 0;
                handyman->f9a++;
                set_complex(handyman, 0x262, 0);
                if (handyman->room)
                    set_complex(handyman, 0x136, handyman->room->unknown6);
                handyman->state_b = 0x3c;
                fn_0003e724(handyman);
                NRAdcounts.unknown2--;
            }
        }
        break;
    case 8:
        room = handyman->room;
        if (room->f3 != 6) {
            if (!(find_person_in_room(room, 4, 1) || find_person_in_room(room, 4, 0)
                  || find_person_in_room(room, 4, 4))) {
                room->f3 = 6;
            } else {
                u16 pos2;

                pos2 = room->pos + (((s16)(old_newrand() % room->f2) << 7) + (s16)(old_newrand() % room->f1));
                if (pos2 != TILE_POS(handyman) && can_i_get_to(handyman, pos2)) {
                    handyman->state_b = handyman->unknown94 = 0x3c;
                    set_complex(handyman, 0xbe, pos2);
                    fn_0003e724(handyman);
                }
            }
        } else {
            handyman->state_b = 0x3c;
            handyman->unknown103 = 9;
            handyman->unknown94 = 0x36;
            handyman->unknown69 = thingpage_remind_me(&allhosps[handyman->owner].ff6d, handyman);
            set_complex(handyman, 0x2d0, 0);
            fn_0003e724(handyman);
        }
        break;
    case 9: {
        u16 seq;

        seq = objects[thing[handyman->unknown69].subtype].f12;
        reset_icon_value(&thing[handyman->unknown69], 0x1c);
        if (seq)
            choose_aniseq(handyman, seq, 0x3c);
        machine = &thing[handyman->unknown69];
        if (machine > thing)
            machine->smoke[old_newrand() % 3] = 0;
        handyman->unknown103 = 10;
        break;
    }
    case 10:
        machine = &thing[handyman->unknown69];
        if (machine > thing) {
            thingpage_cancel(&allhosps[handyman->owner].ff6d, handyman->unknown69);
            if (machine->value > 2) {
                if (old_newrand() % machine->value < machine->mode)
                    machine->value--;
            }
            machine->mode = 0;
            machine->icon = 0;
            machine->smoke[0] = 0;
            machine->smoke[1] = 0;
            machine->smoke[2] = 0;
        }
        handyman->room->f3 = 0;
        set_complex(handyman, 0x136, handyman->room->unknown6);
        handyman->unknown94 = 0x36;
        handyman->state_b = 0x3c;
        handyman->unknown103 = 0;
        handyman->f9c++;
        fn_0003e724(handyman);
        break;
    case 7:
    default:
        break;
    }
}

// FUNCTION: 0x0007b554
void handyman_check_for_litter(Thing *handyman)
{
    u8 range;
    u16 pos;
    Map *m;
    s32 dx;
    s32 dy;

    range = 10 - handyman->unknown126 / 100 < 1 ? 1 : 10 - handyman->unknown126 / 100;
    dx = old_newrand() % (range * 2) - range;
    dy = old_newrand() % (range * 2) - range;
    pos = ((u16)handyman->y.tile << 7) + (u16)handyman->x.tile + ((dy << 7) + dx);
    m = &map[pos];
    if (!(m->floor & 0xf) || (m->floor & 0xf) == 7 || (m->floor & 0xf) == 8 || m->unknown7 != (1 << handyman->owner))
        return;
    {
        u16 nearest;
        u16 second;
        u32 dn;
        u32 db;
        u32 da;

        nearest = handyman->unknown148w;
        second = handyman->unknown150;
        da = (nearest % 128 - handyman->x.tile) * (nearest % 128 - handyman->x.tile) + (nearest / 128 - handyman->y.tile) * (nearest / 128 - handyman->y.tile);
        db = (second % 128 - handyman->x.tile) * (second % 128 - handyman->x.tile) + (second / 128 - handyman->y.tile) * (second / 128 - handyman->y.tile);
        dn = (pos % 128 - handyman->x.tile) * (pos % 128 - handyman->x.tile) + (pos / 128 - handyman->y.tile) * (pos / 128 - handyman->y.tile);
        if (pos == nearest || pos == handyman->unknown150)
            return;
        if (dn < da) {
            handyman->unknown150 = nearest;
            handyman->unknown148w = pos;
        } else if (dn < db) {
            handyman->unknown150 = pos;
        }
    }
}

// FUNCTION: 0x0007b88c
void plant_ages(Thing *plant)
{
    u8 level;
    u32 next;

    if (!othervar.f32 || (old_newrand() & 1))
        return;
    plant->unknown100s--;
    if (plant->unknown100s > 0) {
        if (plant->unknown100s % 80 == 0) {
            level = (400 - plant->unknown100s) / 80;
            next = frame[plant->sprite].next;
            if (!(frame[next].flags & 1))
                plant->sprite = next;
            if (!fn_0007cc88(plant))
                thingpage_request(&allhosps[plant->owner].fd2b, plant - thing, level);
            NRAdcounts.unknown2++;
            NRAdcounts.unknownb -= 11;
        }
    } else if (plant->unknown100s == -40) {
        if (!fn_0007cc88(plant))
            thingpage_request(&allhosps[plant->owner].fd2b, plant - thing, 10);
        plant->unknown100s = 0;
        NRAdcounts.unknown2 += 2;
    }
}

// FUNCTION: 0x0007ba18
u32 compute_plant_frame(Thing *plant)
{
    u32 f;
    s32 stage;

    f = start[0x3cf].frame;
    stage = (400 - plant->unknown100s) / 80;
    stage > 4 ? stage = 4 : 0;
    for (; stage > 0; stage--)
        f = frame[f].next;
    return f;
}

// FUNCTION: 0x0007baac
u8 thingpage_request(ThingPager *p, u16 id, u8 priority)
{
    s32 i;

    if (p->count == 0x40)
        return 0;
    for (i = p->count - 1; i >= 0; i--) {
        if (p->pages[i].id == id) {
            p->pages[i].priority < priority ? p->pages[i].priority = priority : 0;
            return 1;
        }
    }
    p->pages[p->count].id = id;
    p->pages[p->count].priority = priority;
    p->pages[p->count].responder = 0;
    p->pages[p->count].time = 0;
    p->count++;
    return 1;
}

// FUNCTION: 0x0007bbbc
u8 thingpage_cancel(ThingPager *p, u16 id)
{
    s32 i;

    for (i = p->count - 1; i >= 0; i--) {
        if (p->pages[i].id == id) {
            fn_000c21e6(&p->pages[i], &p->pages[p->count - 1], 9);
            p->count--;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0007bc4c
u16 thingpage_respond(ThingPager *p, Thing *handyman, u16 room, u8 min)
{
    s32 best_priority;
    s32 best;
    u32 best_dist;
    s32 i;
    u32 room_id;
    u32 dist;

    best_priority = -1;
    best = -1;
    best_dist = 0x4000;
    for (i = p->count - 1; i >= 0; i--) {
        if (room && thing[p->pages[i].id].unknown89 != room)
            continue;
        if (p->pages[i].responder != (u16)(handyman - thing) && p->pages[i].time > actual_game_turn)
            continue;
        if (p->pages[i].priority < min)
            continue;
        room_id = thing[p->pages[i].id].unknown89;
        if (room_id != 0
            && (room_info[room_id].f22 || room_info[room_id].f3 == 4
                || allhosps[handyman->owner].unknown11b7 == room_id))
            continue;
        if (p->pages[i].priority == best_priority) {
            dist = (TILE_POS(handyman) % 128 - TILE_POS(&thing[p->pages[i].id]) % 128)
                     * (TILE_POS(handyman) % 128 - TILE_POS(&thing[p->pages[i].id]) % 128)
                 + (TILE_POS(handyman) / 128 - TILE_POS(&thing[p->pages[i].id]) / 128)
                     * (TILE_POS(handyman) / 128 - TILE_POS(&thing[p->pages[i].id]) / 128);
            if (dist < best_dist) {
                best = i;
                best_dist = dist;
            }
        } else if (p->pages[i].priority > best_priority) {
            best = i;
            best_dist = (TILE_POS(handyman) % 128 - TILE_POS(&thing[p->pages[i].id]) % 128)
                          * (TILE_POS(handyman) % 128 - TILE_POS(&thing[p->pages[i].id]) % 128)
                      + (TILE_POS(handyman) / 128 - TILE_POS(&thing[p->pages[i].id]) / 128)
                          * (TILE_POS(handyman) / 128 - TILE_POS(&thing[p->pages[i].id]) / 128);
            best_priority = p->pages[i].priority;
        }
    }
    if (best == -1)
        return 0;
    p->pages[best].responder = handyman - thing;
    p->pages[best].time = actual_game_turn + 3000;
    return p->pages[best].id;
}

// FUNCTION: 0x0007c2f8
u8 thingpage_find_highest_priority(ThingPager *p)
{
    s32 i;
    s32 best;

    best = -1;
    for (i = p->count - 1; i >= 0; i--)
        p->pages[i].priority > best ? best = p->pages[i].priority : 0;
    return best;
}

// FUNCTION: 0x0007c37c
u16 thingpage_remind_me(ThingPager *p, Thing *handyman)
{
    s32 i;

    for (i = p->count - 1; i >= 0; i--) {
        if (p->pages[i].responder == (u16)(handyman - thing))
            return p->pages[i].id;
    }
    return 0;
}

// FUNCTION: 0x0007c3f8
void thingpage_cancel_my_response(ThingPager *p, Thing *handyman)
{
    u32 id;
    s32 i;

    id = (u16)(handyman - thing);
    for (i = p->count - 1; i >= 0; i--) {
        if (p->pages[i].responder == id) {
            p->pages[i].responder = 0;
            p->pages[i].time = 0;
        }
    }
}

// FUNCTION: 0x0007c488
u8 handyman_try_fix(Thing *handyman)
{
    u16 id;
    RoomInfo *room;

    id = thingpage_respond(&allhosps[handyman->owner].ff6d, handyman, 0, (100 - handyman->fa0) * 3 / 4);
    if (id) {
        room = &room_info[thing[id].unknown89];
        if (room > room_info) {
            set_complex(handyman, 0x96, room->unknown4);
            handyman->state_b = 0x3c;
            handyman->unknown103 = 8;
            handyman->unknown94 = 0x36;
            handyman->unknown89 = room - room_info;
            fn_0003e724(handyman);
            if (room->f3 != 6)
                room->f3 = 5;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0007c5ac
u8 handyman_try_plant(Thing *handyman)
{
    u16 id;
    u16 room_id;

    id = thingpage_respond(&allhosps[handyman->owner].fd2b, handyman, 0, 0);
    if (id) {
        room_id = thing[id].unknown89;
        if (room_id) {
            handyman->unknown89 = room_id;
            set_complex(handyman, 0x96, room_info[room_id].unknown4);
            handyman->unknown103 = 4;
        } else {
            set_complex(handyman, 0x26c, 0);
            handyman->unknown69 = id;
            handyman->unknown103 = 5;
        }
        handyman->state_b = 0x3c;
        handyman->unknown94 = 0x36;
        fn_0003e724(handyman);
        return 1;
    }
    return 0;
}

// FUNCTION: 0x0007c6a0
u8 handyman_try_litter(Thing *handyman)
{
    u16 target;
    u16 pos;
    u32 i;

    target = 0;
    pos = (u16)handyman->x.tile + ((u16)handyman->y.tile << 7);
    for (i = 0; i < 9; i++) {
        if (!(!(map[pos + special_around[i]].floor & 0xf) || (map[pos + special_around[i]].floor & 0xf) == 7
              || (map[pos + special_around[i]].floor & 0xf) == 8)) {
            target = pos + special_around[i];
            break;
        }
    }
    if (target == 0 && handyman->unknown148w) {
        if (!(!(map[handyman->unknown148w].floor & 0xf) || (map[handyman->unknown148w].floor & 0xf) == 7
              || (map[handyman->unknown148w].floor & 0xf) == 8))
            target = handyman->unknown148w;
        handyman->unknown148w = handyman->unknown150;
        handyman->unknown150 = 0;
    }
    if (target) {
        handyman->state_b = 0x3c;
        handyman->unknown94 = 0x3c;
        set_complex(handyman, 0xbe, target);
        fn_0003e724(handyman);
        handyman->unknown103 = 2;
        return 1;
    }
    return 0;
}

// FUNCTION: 0x0007c884
u8 is_next_to_plant(Thing *handyman, Thing *plant)
{
    s32 here;
    s32 there;

    here = TILE_POS(handyman);
    there = TILE_POS(plant);
    if ((TILE_POS(plant) % 128 - TILE_POS(handyman) % 128) * (TILE_POS(plant) % 128 - TILE_POS(handyman) % 128)
            + (TILE_POS(plant) / 128 - TILE_POS(handyman) / 128) * (TILE_POS(plant) / 128 - TILE_POS(handyman) / 128)
        > 1)
        return 0;
    if (handyman->y.tile == plant->y.tile + 1)
        return map[here].unknown3 >= 0x8e && map[here].unknown3 <= 0x91 ? (u8)1 : (u8)0;
    if (handyman->x.tile == plant->x.tile + 1)
        return map[here].unknown4 >= 0x8e && map[here].unknown4 <= 0x91 ? (u8)1 : (u8)0;
    if (handyman->y.tile == plant->y.tile - 1)
        return map[there].unknown3 >= 0x8e && map[there].unknown3 <= 0x91 ? (u8)1 : (u8)0;
    if (handyman->x.tile == plant->x.tile - 1)
        return map[there].unknown4 >= 0x8e && map[there].unknown4 <= 0x91 ? (u8)1 : (u8)0;
    return 0;
}

// FUNCTION: 0x0007cbdc
u8 fn_0007cbdc(RoomInfo *room, s32 pos, u8 owner)
{
    if (room > room_info) {
        if (which_room(pos) == room) {
            if (check_points_connected(room->unknown6, pos))
                return 1;
        }
    } else {
        s32 door;

        door = room_info[allhosps[owner].unknown1cf[0]].unknown4;
        if (check_points_connected(door, pos))
            return 1;
    }
    return 0;
}

// FUNCTION: 0x0007cc88
u8 fn_0007cc88(Thing *plant)
{
    RoomInfo *room;
    s32 pos;

    room = &room_info[plant->unknown89];
    pos = (s16)plant->x.tile + ((s16)plant->y.tile << 7);
    if (fn_0007cbdc(room, pos + 1, plant->owner))
        return 0;
    if (fn_0007cbdc(room, pos + 0x80, plant->owner))
        return 0;
    if (fn_0007cbdc(room, pos - 1, plant->owner))
        return 0;
    if (fn_0007cbdc(room, pos - 0x80, plant->owner))
        return 0;
    return 1;
}
