#include "hosp.h"

extern "C" {
u8 trapped_plant_check(RoomInfo *room, s32 pos, u8 owner);
#pragma aux trapped_plant_check "W?trapped_plant_check$n(pn$RoomInfo$$l)uc"
u8 is_plant_trapped(Thing *w);
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

void choose_aniseq(Thing *w, u8 group, u8 next_state);
void remove_litter(u16 tile, u8 owner);

#define TILE_POS(t) ((s16)(t)->x.tile + ((s16)(t)->y.tile << 7))

// Drops litter of kind type (floor kind 1-15) at tile if it is owner's land:
// on a clean tile, counting it (allhosps.fcaa); kinds 4 and 5 also replace
// existing mess. Records the tile in allhosps.fcac.
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

// Cleans the mess off tile (on owner's land), uncounting it.
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

// SS_HANDYMAN_WORKS: runs handyman by substate. 0 decides what to do next:
// resign or rest if due; in a room, page and repair its machines; otherwise
// repair urgent machines, then the job (sweeping, watering, repairing) whose
// done count is lowest for its priority, else stroll nearby. Later substates
// sweep litter (2-3), water a plant (4-6) and repair a machine once its room
// is clear of staff (8-10).
// FUNCTION: 0x0007a8a8
void handyman_works(Thing *handyman)
{
    u8 (*funcs[3])(Thing *);
    Thing *plant;
    Thing *machine;
    RoomInfo *room;
    u32 scores[3];
    s32 litter_score;
    s32 plant_score;
    s32 fix_score;
    u32 n;
    u32 idx;
    u32 best;
    s32 dx;
    s32 dy;
    u16 pos;
    u16 id;
    u8 i;
    u8 stage;

    switch (handyman->substate) {
    case 0:
        if (shall_i_resign(handyman))
            break;
        if (((handyman->orders & ORD_REST) || handyman->tiredness > allhosps[handyman->owner].f21d) && take_a_break(handyman))
            break;
        room = handyman->room;
        if (room) {
            for (machine = &thing[handyman->room->first_object]; machine > thing; machine = &thing[machine->room_obj_next]) {
                if (machine->type == THING_OBJECT && objects[machine->subtype].unknown24) {
                    if (machine->times_used > 0) {
                        thingpage_request(&allhosps[machine->owner].ff6d, machine - thing, 100);
                        break;
                    }
                }
            }
            if (thingpage_respond(&allhosps[handyman->owner].ff6d, handyman, handyman->room - room_info, 0)) {
                handyman->substate = 8;
                if (room->f3 != 6)
                    room->f3 = 5;
                break;
            }
            handyman->state = SS_PROCESS_COMPLEX;
            break;
        }
        if (thingpage_find_highest_priority(&allhosps[handyman->owner].ff6d) && handyman_try_fix(handyman))
            break;
        litter_score = handyman->litter_count * 100 / (handyman->prio_litter + 1);
        plant_score = handyman->plant_count * 1000 / (handyman->prio_plants + 1);
        fix_score = handyman->repair_count * 2000 / (handyman->prio_repair + 1);
        scores[0] = litter_score;
        funcs[0] = handyman_try_litter;
        scores[1] = plant_score;
        funcs[1] = handyman_try_plant;
        scores[2] = fix_score;
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
            handyman->next_state = handyman->resume_state = SS_HANDYMAN_WORKS;
            set_complex(handyman, 0xbe, (u16)handyman->x.tile + ((u16)handyman->y.tile << 7) + ((dy << 7) + dx));
            process_staff_complex(handyman);
        }
        break;
    case 1:
        break;
    case 2:
        if (handyman->x.tile != handyman->dest_x || handyman->y.tile != handyman->dest_y) {
            handyman->substate = 0;
            break;
        }
        if (handyman->work_target == TILE_POS(handyman))
            handyman->work_target = 0;
        handyman->facing = showanim_angle[handyman->facing];
        handyman->next_state = SS_HANDYMAN_WORKS;
        handyman->substate = 3;
        set_complex(handyman, 0x46, 0);
        process_staff_complex(handyman);
        break;
    case 3:
        remove_litter((u16)handyman->x.tile + ((u16)handyman->y.tile << 7), handyman->owner);
        set_complex(handyman, 0x30c, 0);
        handyman->next_state = SS_HANDYMAN_WORKS;
        process_staff_complex(handyman);
        handyman->substate = 0;
        handyman->litter_count++;
        break;
    case 4:
        handyman->object = thingpage_remind_me(&allhosps[handyman->owner].fd2b, handyman);
        set_complex(handyman, 0x26c, 0);
        handyman->substate = 5;
        handyman->next_state = SS_HANDYMAN_WORKS;
        handyman->resume_state = SS_PROCESS_COMPLEX;
        process_staff_complex(handyman);
        break;
    case 5:
        id = handyman->object;
        plant = &thing[id];
        thingpage_cancel(&allhosps[plant->owner].fd2b, id);
        if (plant->in_use == 1 || plant->type != THING_OBJECT || plant->subtype != OBJ_PLANT || plant->state != OS_PLANT_AGES
            || !is_next_to_plant(handyman, plant)) {
            handyman->state = SS_PROCESS_COMPLEX;
            break;
        }
        plant->in_use = 1;
        handyman->facing = direction_to_face(plant, handyman);
        handyman->next_state = SS_HANDYMAN_WORKS;
        handyman->resume_state = SS_PROCESS_COMPLEX;
        handyman->substate = 6;
        set_complex(handyman, 0x258, 0);
        process_staff_complex(handyman);
        plant->plant_health < 0 ? plant->plant_health = 0 : 0;
        stage = (400 - plant->plant_health) / 80;
        {
            u16 anim;

            anim = check_frame(0x3d0, plant) + (u16)plant->facing;
            plant->sprite = start[anim].frame;
            plant->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
        }
        for (i = 0; i < 5 - stage - 1; i++)
            plant->sprite = frame[plant->sprite].next;
        plant->state = 0xe;
        break;
    case 6:
        id = handyman->object;
        plant = &thing[id];
        if (!(actual_game_turn & 1)) {
            plant->sprite = frame[plant->sprite].next;
            if (frame[plant->sprite].flags & 1) {
                plant->in_use = 0;
                {
                    u16 anim2;

                    anim2 = check_frame(0x3cf, plant) + (u16)plant->facing;
                    plant->sprite = start[anim2].frame;
                    plant->flip = start[anim2].flags > 0 ? (u8)1 : (u8)0;
                }
                plant->state = OS_PLANT_AGES;
                if (plant->owner == player) {
                    NRAdcounts.unknownb += 0x37 - (plant->plant_health * 2 + 0x50) / 16;
                    if (NRAdcounts.unknownb < 0)
                        NRAdcounts.unknownb = 0;
                    else if (NRAdcounts.unknown0 * 0x37 < NRAdcounts.unknownb)
                        NRAdcounts.unknownb = NRAdcounts.unknown0 * 0x37;
                }
                plant->plant_health = 400;
                handyman->substate = 0;
                handyman->plant_count++;
                set_complex(handyman, 0x262, 0);
                if (handyman->room)
                    set_complex(handyman, 0x136, handyman->room->unknown6);
                handyman->next_state = SS_HANDYMAN_WORKS;
                process_staff_complex(handyman);
                NRAdcounts.unknown2--;
            }
        }
        break;
    case 8:
        room = handyman->room;
        if (room->f3 != 6) {
            if (!(find_person_in_room(room, THING_STAFF, STAFF_DOCTOR) || find_person_in_room(room, THING_STAFF, STAFF_NURSE)
                  || find_person_in_room(room, THING_STAFF, STAFF_SURGEON))) {
                room->f3 = 6;
            } else {
                u16 pos2;

                pos2 = room->pos + (((s16)(old_newrand() % room->f2) << 7) + (s16)(old_newrand() % room->f1));
                if (pos2 != TILE_POS(handyman) && can_i_get_to(handyman, pos2)) {
                    handyman->next_state = handyman->resume_state = SS_HANDYMAN_WORKS;
                    set_complex(handyman, 0xbe, pos2);
                    process_staff_complex(handyman);
                }
            }
        } else {
            handyman->next_state = SS_HANDYMAN_WORKS;
            handyman->substate = 9;
            handyman->resume_state = SS_PROCESS_COMPLEX;
            handyman->object = thingpage_remind_me(&allhosps[handyman->owner].ff6d, handyman);
            set_complex(handyman, 0x2d0, 0);
            process_staff_complex(handyman);
        }
        break;
    case 9: {
        u16 seq;

        seq = objects[thing[handyman->object].subtype].f12;
        reset_icon_value(&thing[handyman->object], ICON_NEEDS_REPAIR);
        if (seq)
            choose_aniseq(handyman, seq, 0x3c);
        machine = &thing[handyman->object];
        if (machine > thing)
            machine->smoke[old_newrand() % 3] = 0;
        handyman->substate = 10;
        break;
    }
    case 10:
        machine = &thing[handyman->object];
        if (machine > thing) {
            thingpage_cancel(&allhosps[handyman->owner].ff6d, handyman->object);
            if (machine->strength > 2) {
                if (old_newrand() % machine->strength < machine->times_used)
                    machine->strength--;
            }
            machine->times_used = 0;
            machine->overlay_frame = 0;
            machine->smoke[0] = 0;
            machine->smoke[1] = 0;
            machine->smoke[2] = 0;
        }
        handyman->room->f3 = 0;
        set_complex(handyman, 0x136, handyman->room->unknown6);
        handyman->resume_state = SS_PROCESS_COMPLEX;
        handyman->next_state = SS_HANDYMAN_WORKS;
        handyman->substate = 0;
        handyman->repair_count++;
        process_staff_complex(handyman);
        break;
    case 7:
    default:
        break;
    }
}

// Picks a random tile near handyman (closer the more able they are) and, if it
// has litter on their land, keeps it as their nearest or second nearest litter
// target (work_target, work_target2).
// FUNCTION: 0x0007b554
void handyman_check_for_litter(Thing *handyman)
{
    u8 range;
    u16 pos;
    Map *m;
    s32 dx;
    s32 dy;

    range = 10 - handyman->ability / 100 < 1 ? 1 : 10 - handyman->ability / 100;
    dx = old_newrand() % (range * 2) - range;
    dy = old_newrand() % (range * 2) - range;
    pos = ((u16)handyman->y.tile << 7) + (u16)handyman->x.tile + ((dy << 7) + dx);
    m = &map[pos];
    if (!(m->floor & 0xf) || (m->floor & 0xf) == 7 || (m->floor & 0xf) == 8 || m->unknown7 != (1 << handyman->owner))
        return;
    {
        u16 nearest;
        u16 second;
        u32 dist_new;
        u32 dist_second;
        u32 dist_nearest;

        nearest = handyman->work_target;
        second = handyman->work_target2;
        dist_nearest = (nearest % 128 - handyman->x.tile) * (nearest % 128 - handyman->x.tile) + (nearest / 128 - handyman->y.tile) * (nearest / 128 - handyman->y.tile);
        dist_second = (second % 128 - handyman->x.tile) * (second % 128 - handyman->x.tile) + (second / 128 - handyman->y.tile) * (second / 128 - handyman->y.tile);
        dist_new = (pos % 128 - handyman->x.tile) * (pos % 128 - handyman->x.tile) + (pos / 128 - handyman->y.tile) * (pos / 128 - handyman->y.tile);
        if (pos == nearest || pos == handyman->work_target2)
            return;
        if (dist_new < dist_nearest) {
            handyman->work_target2 = nearest;
            handyman->work_target = pos;
        } else if (dist_new < dist_second) {
            handyman->work_target2 = pos;
        }
    }
}

// OS_PLANT_AGES: every 16 turns, half the time, plant loses health; every 80
// points it wilts a stage and pages a handyman (unless it cannot be reached),
// and once dead it pages urgently every 40 points more. Updates the advisor's
// plant counts.
// FUNCTION: 0x0007b88c
void plant_ages(Thing *plant)
{
    u8 stage;
    u32 next;

    if (!othervar.f32 || (old_newrand() & 1))
        return;
    plant->plant_health--;
    if (plant->plant_health > 0) {
        if (plant->plant_health % 80 == 0) {
            stage = (400 - plant->plant_health) / 80;
            next = frame[plant->sprite].next;
            if (!(frame[next].flags & 1))
                plant->sprite = next;
            if (!is_plant_trapped(plant))
                thingpage_request(&allhosps[plant->owner].fd2b, plant - thing, stage);
            NRAdcounts.unknown2++;
            NRAdcounts.unknownb -= 11;
        }
    } else if (plant->plant_health == -40) {
        if (!is_plant_trapped(plant))
            thingpage_request(&allhosps[plant->owner].fd2b, plant - thing, 10);
        plant->plant_health = 0;
        NRAdcounts.unknown2 += 2;
    }
}

// Returns the frame showing plant at its wilting stage.
// FUNCTION: 0x0007ba18
u32 compute_plant_frame(Thing *plant)
{
    u32 f;
    s32 stage;

    f = start[0x3cf].frame;
    stage = (400 - plant->plant_health) / 80;
    stage > 4 ? stage = 4 : 0;
    for (; stage > 0; stage--)
        f = frame[f].next;
    return f;
}

// Pages for thing id at priority on pager p, raising the priority of an
// existing page. Returns 0 if the pager is full (64 pages), else 1.
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

// Removes the page for thing id from p. Returns 1 if there was one.
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

// Picks for handyman the page on p (for things in room, if not 0) of the
// highest priority (at least min), nearest first, that nobody else has taken
// in the last 3000 turns and is not in a room being edited; takes it for
// 3000 turns. Returns its thing, or 0.
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
        if (room && thing[p->pages[i].id].room_id != room)
            continue;
        if (p->pages[i].responder != (u16)(handyman - thing) && p->pages[i].time > actual_game_turn)
            continue;
        if (p->pages[i].priority < min)
            continue;
        room_id = thing[p->pages[i].id].room_id;
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

// Returns the highest priority among p's pages (0xff if there are none).
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

// Returns the thing of a page on p that handyman has taken, or 0.
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

// Releases every page on p that handyman had taken.
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

// Sends handyman to repair the most urgent machine paged (by their repair
// priority), into its room (substate 8). Returns 1 if one was taken.
// FUNCTION: 0x0007c488
u8 handyman_try_fix(Thing *handyman)
{
    u16 id;
    RoomInfo *room;

    id = thingpage_respond(&allhosps[handyman->owner].ff6d, handyman, 0, (100 - handyman->prio_repair) * 3 / 4);
    if (id) {
        room = &room_info[thing[id].room_id];
        if (room > room_info) {
            set_complex(handyman, 0x96, room->unknown4);
            handyman->next_state = SS_HANDYMAN_WORKS;
            handyman->substate = 8;
            handyman->resume_state = SS_PROCESS_COMPLEX;
            handyman->dest_room = room - room_info;
            process_staff_complex(handyman);
            if (room->f3 != 6)
                room->f3 = 5;
            return 1;
        }
    }
    return 0;
}

// Sends handyman to water the most urgent plant paged, through its room's
// door if it is in a room. Returns 1 if one was taken.
// FUNCTION: 0x0007c5ac
u8 handyman_try_plant(Thing *handyman)
{
    u16 id;
    u16 room_id;

    id = thingpage_respond(&allhosps[handyman->owner].fd2b, handyman, 0, 0);
    if (id) {
        room_id = thing[id].room_id;
        if (room_id) {
            handyman->dest_room = room_id;
            set_complex(handyman, 0x96, room_info[room_id].unknown4);
            handyman->substate = 4;
        } else {
            set_complex(handyman, 0x26c, 0);
            handyman->object = id;
            handyman->substate = 5;
        }
        handyman->next_state = SS_HANDYMAN_WORKS;
        handyman->resume_state = SS_PROCESS_COMPLEX;
        process_staff_complex(handyman);
        return 1;
    }
    return 0;
}

// Sends handyman to sweep litter on or next to them, or else their nearest
// remembered litter target (moving on to the next). Returns 1 if they set
// off.
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
    if (target == 0 && handyman->work_target) {
        if (!(!(map[handyman->work_target].floor & 0xf) || (map[handyman->work_target].floor & 0xf) == 7
              || (map[handyman->work_target].floor & 0xf) == 8))
            target = handyman->work_target;
        handyman->work_target = handyman->work_target2;
        handyman->work_target2 = 0;
    }
    if (target) {
        handyman->next_state = SS_HANDYMAN_WORKS;
        handyman->resume_state = SS_HANDYMAN_WORKS;
        set_complex(handyman, 0xbe, target);
        process_staff_complex(handyman);
        handyman->substate = 2;
        return 1;
    }
    return 0;
}

// Returns 1 if handyman stands on a tile beside plant (not diagonally) with an
// invisible wall (blocks 0x8e-0x91) on the edge between them, else 0.
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

// Returns 1 if tile pos can be reached: from room's door when room is a room
// (and pos is in it), else from owner's entrance. Otherwise 0.
// FUNCTION: 0x0007cbdc
u8 trapped_plant_check(RoomInfo *room, s32 pos, u8 owner)
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

// Returns 1 if none of the four tiles beside plant can be reached (see
// trapped_plant_check), else 0.
// FUNCTION: 0x0007cc88
u8 is_plant_trapped(Thing *plant)
{
    RoomInfo *room;
    s32 pos;

    room = &room_info[plant->room_id];
    pos = (s16)plant->x.tile + ((s16)plant->y.tile << 7);
    if (trapped_plant_check(room, pos + 1, plant->owner))
        return 0;
    if (trapped_plant_check(room, pos + 0x80, plant->owner))
        return 0;
    if (trapped_plant_check(room, pos - 1, plant->owner))
        return 0;
    if (trapped_plant_check(room, pos - 0x80, plant->owner))
        return 0;
    return 1;
}
