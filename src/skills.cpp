#include "hosp.h"

extern "C" {
extern u16 D_0012dd7c[];
}

// NONMATCHING: shift count load, self-call relocation
// FUNCTION: 0x00049820
u16 page_member_of_staff(u16 a, u16 room_id, u8 b, s8 c, u8 owner)
{
    RoomInfo *room;
    SkillPager *free;
    u16 count;
    u16 i;
    u32 bit;
    Thing *w;
    SkillPager *p;

    room = &room_info[room_id];
    free = 0;
    count = 0;
    if (room->type != 0) {}
    room = &room_info[room_id];
    if (room->type == 4)
        return 0;
    for (i = 0; i < 9; i++) {
        if (a & (1 << i)) {
            bit = 1 << i;
            count++;
        }
    }
    if (count > 1) {
        page_member_of_staff(~bit & a, room_id, b, c, owner);
        a = bit;
    }
    if (b) {
        for (w = &thing[room->first]; w > thing; w = &thing[w->room_next]) {
            if (w->renderer == 4 && (w->unknown124 & a)) {
                if (--c <= 0)
                    return 1;
            }
        }
    }
    for (p = pagers; p < end_pagers; p++) {
        if ((p->mask & a) && p->room == room_id) {
            if (--c <= 0)
                return 1;
        } else if (p->mask == 0) {
            free = p;
        }
    }
    if (free) {
        free->mask = a;
        free->room = room_id;
        free->owner = owner;
        free->unknown4 = 0;
        free->unknown8 = 0;
        if (owner == player) {
            if (room->type >= 0 && room->type <= 0x1e)
                play_ambient(room->type);
        }
        if (c > 1)
            return page_member_of_staff(a, room_id, b, c - 1, owner);
    }
    return 0;
}

// FUNCTION: 0x00049acc
u16 check_pager(Thing *w)
{
    SkillPager *p;
    SkillPager *found;
    RoomInfo *room;
    u16 best;
    u16 dist;
    u16 room_id;

    found = 0;
    best = 999;
    for (p = pagers; p < end_pagers; p++) {
        if ((p->owner == w->owner && (p->mask & w->unknown124) && p->unknown4 < actual_game_turn)
            || (u16)(w - thing) == p->unknown8) {
            room = &room_info[p->room];
            if (!(room->f3 == 1 || room->f3 == 0))
                continue;
            {
                dist = fn_000cb2db(w->x.tile - room->unknown4 % 128)
                     + fn_000cb2db(w->y.tile - room->unknown4 / 128);
                if (best > dist) {
                    best = dist;
                    found = p;
                }
            }
        }
    }
    if (found) {
        room_id = found->room;
        found->unknown4 = actual_game_turn + 3000;
        found->unknown8 = w - thing;
    } else {
        room_id = 0;
    }
    room = &room_info[room_id];
    if (room > room_info && room->type == 0) {
        for (p = pagers; p < end_pagers; p++) {
            if (p->room == room_id)
                memset(p, 0, sizeof(SkillPager));
        }
        room_id = 0;
    }
    return room_id;
}

// NONMATCHING: register choice
// FUNCTION: 0x00049ce0
void pager_arrival(Thing *w)
{
    SkillPager *blank;
    SkillPager *p;
    SkillPager *other;
    u16 id;

    id = w - thing;
    other = 0;
    blank = 0;
    for (p = pagers; p < end_pagers; p++) {
        if ((p->mask & w->unknown124) && (u16)(w->room - room_info) == p->room && p->owner == w->owner) {
            if ((u16)(w - thing) == p->unknown8)
                goto found;
            if (p->unknown8)
                other = p;
            else
                blank = p;
        }
    }
    if (other)
        p = other;
    else if (blank)
        p = blank;
found:
    if (p < end_pagers)
        p->mask = p->room = p->owner = p->unknown4 = p->unknown8 = 0;
}

// FUNCTION: 0x00049e30
void clear_pager_responses(u16 room_id)
{
    SkillPager *p;

    for (p = pagers; p < end_pagers; p++) {
        if (p->room == room_id) {
            p->unknown4 = 0;
            p->unknown8 = 0;
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x00049e88
RoomInfo *get_expertise_dest(u16 type, Thing *w)
{
    RoomInfo *best;
    RoomInfo *fallback;
    u32 best_score;
    Expertise *e;
    RoomInfo *room;
    u32 score;
    Thing *t;
    u8 owner;
    u8 kind;

    best = 0;
    fallback = 0;
    best_score = 9999;
    owner = w->owner;
    e = &expertise[type];
    kind = e->category;
    if (allhosps[owner].rooms[type].unknown1C == 1 && listhead[owner].rooms[kind]) {
        for (room = room_info + 1; room < end_room_info; room++) {
            if (room->owner == owner && room->type == kind && room->unknown4 != 0) {
                fallback = room;
                if (spare_queue_capacity(0, 1, room, owner)) {
                    score = room->unknown31 * D_0012dd7c[2];
                    score += (fn_000cb2db(w->x.tile - room->unknown4 % 128)
                            + fn_000cb2db(w->y.tile - room->unknown4 / 128)) * D_0012dd7c[3];
                    score += D_0012dd7c[4];
                    for (t = &thing[room->first]; t > thing; t = &thing[t->room_next]) {
                        if (t->renderer == 4) {
                            score -= D_0012dd7c[4];
                            break;
                        }
                    }
                    if (score < best_score) {
                        best_score = score;
                        best = room;
                    }
                }
            }
        }
    }
    if (best)
        return best;
    else
        return fallback;
}

// FUNCTION: 0x0004a0d0
void insert_skill(u16 mask, u8 owner)
{
    u16 i;

    for (i = 0; i < 9; i++) {
        if (mask & (1 << i))
            listhead[owner].staff[i]++;
    }
}

// FUNCTION: 0x0004a140
void remove_skill(u16 mask, u8 owner)
{
    u16 i;

    for (i = 0; i < 9; i++) {
        if (mask & (1 << i)) {
            if (listhead[owner].staff[i])
                listhead[owner].staff[i]--;
        }
    }
}

// FUNCTION: 0x0004a1d4
void staff_abandon_pager_response(Thing *w)
{
    SkillPager *p;
    u16 id;

    id = w - thing;
    for (p = pagers; p < end_pagers; p++) {
        if (p->unknown8 == id) {
            p->unknown4 = 0;
            p->unknown8 = 0;
        }
    }
}

// FUNCTION: 0x0004a244
void room_deleted_update_listhead(RoomInfo *room)
{
    u32 id;
    RoomInfo *r;

    id = 0;
    for (r = room_info + 1; r < end_room_info; r++) {
        if (r->type == room->type && r != room && r->owner == room->owner) {
            id = (u16)(r - room_info);
            break;
        }
    }
    listhead[room->owner].rooms[room->type] = id;
}

// NONMATCHING: register choice
// FUNCTION: 0x0004a2fc
void set_expertise_staff(Thing *w)
{
    Expertise *e;
    u32 i;
    u32 kind;
    s32 ward;

    ward = allhosps[w->owner].rooms[42].unknown1C == 1;
    for (i = 1; i < 0x2f; i++) {
        if (allhosps[w->owner].rooms[i].unknown1C == 0) {
            e = &expertise[i];
            if (listhead[w->owner].rooms[e->category] != 0) {
                if ((e->unknown0 & 0x20) && allhosps[w->owner].fca6 < 2)
                    continue;
                if (w->unknown124 & e->unknown0) {
                    if (e->unknown7 && !ward)
                        continue;
                    kind = e->kind;
                    if (kind == 0)
                        allhosps[w->owner].rooms[i].unknown1C = 1;
                    else if (listhead[w->owner].objects[kind])
                        allhosps[w->owner].rooms[i].unknown1C = 1;
                }
            }
        }
    }
    if (!ward && allhosps[w->owner].rooms[42].unknown1C == 1)
        set_expertise_ward(w->owner);
}

// NONMATCHING: register choice
// FUNCTION: 0x0004a520
void set_expertise_room(RoomInfo *room)
{
    Expertise *e;
    u32 i;
    u32 mask;
    u32 j;
    s32 ward;

    ward = allhosps[room->owner].rooms[42].unknown1C == 1;
    for (i = 1; i < 0x2f; i++) {
        if (allhosps[room->owner].rooms[i].unknown1C == 0) {
            e = &expertise[i];
            if (e->unknown7 && !ward)
                continue;
            if (e->category == room->type) {
                mask = e->unknown0;
                if (mask == 0) {
                    if (room->type != 0x1c)
                        allhosps[room->owner].rooms[i].unknown1C = 1;
                } else {
                    for (j = 0; j < 9; j++) {
                        if (((1 << j) & mask) == 0x20 && allhosps[room->owner].fca6 < 2)
                            break;
                        if ((mask & (1 << j)) && listhead[room->owner].staff[j] == 0)
                            break;
                    }
                    if (j >= 9)
                        allhosps[room->owner].rooms[i].unknown1C = 1;
                }
            }
        }
    }
    if (!ward && allhosps[room->owner].rooms[42].unknown1C == 1)
        set_expertise_ward(room->owner);
}

// NONMATCHING: argument zero-extension
// FUNCTION: 0x0004a754
void remove_expertise_staff(Thing *w)
{
    u16 j;
    u16 i;
    u16 mask;
    s32 ward;

    ward = allhosps[w->owner].rooms[42].unknown1C == 1;
    remove_skill(w->unknown124, w->owner);
    mask = w->unknown124;
    for (i = 0; i < 9; i++) {
        if (mask & (1 << i)) {
            if (listhead[w->owner].staff[i] == 0 || (i == 5 && allhosps[w->owner].fca6 < 3)) {
                for (j = 1; j < 0x2f; j++) {
                    if (allhosps[w->owner].rooms[j].unknown1C == 1) {
                        if ((1 << i) & expertise[j].unknown0)
                            allhosps[w->owner].rooms[j].unknown1C = 0;
                    }
                }
            }
        }
    }
    if (ward && allhosps[w->owner].rooms[42].unknown1C != 1)
        remove_expertise_ward(w->owner);
}

// NONMATCHING: register choice
// FUNCTION: 0x0004a964
void remove_expertise_room(RoomInfo *room)
{
    u16 i;
    RoomInfo *r;
    s32 ward;

    ward = allhosps[room->owner].rooms[42].unknown1C == 1;
    for (r = room_info + 1; r < end_room_info; r++) {
        if (r->owner == room->owner && room->type == r->type && r != room)
            return;
    }
    for (i = 1; i < 0x2f; i++) {
        if (allhosps[room->owner].rooms[i].unknown1C == 1) {
            if (room->type == expertise[i].category)
                allhosps[room->owner].rooms[i].unknown1C = 0;
        }
    }
    if (ward && allhosps[room->owner].rooms[42].unknown1C != 1)
        remove_expertise_ward(room->owner);
}

// NONMATCHING: register choice
// FUNCTION: 0x0004aae4
void set_expertise_ward(u8 owner)
{
    u32 i;
    Expertise *e;
    u32 mask;
    u32 j;

    for (i = 1; i < 0x2f; i++) {
        if (allhosps[owner].rooms[i].unknown1C == 0) {
            e = &expertise[i];
            if (e->unknown7) {
                if (listhead[owner].rooms[e->category] != 0) {
                    if ((e->unknown0 & 0x20) && allhosps[owner].fca6 < 2)
                        continue;
                    mask = e->unknown0;
                    for (j = 0; j < 9; j++) {
                        if ((mask & (1 << j)) && listhead[owner].staff[j] == 0)
                            break;
                    }
                    if (j >= 9)
                        allhosps[owner].rooms[i].unknown1C = 1;
                }
            }
        }
    }
}

// FUNCTION: 0x0004ac4c
void remove_expertise_ward(u8 owner)
{
    u32 i;

    for (i = 1; i < 0x2f; i++) {
        if (allhosps[owner].rooms[i].unknown1C == 1 && expertise[i].unknown7)
            allhosps[owner].rooms[i].unknown1C = 0;
    }
}
