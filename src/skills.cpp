#include "hosp.h"

extern "C" {
}

// FUNCTION: 0x00049820
u16 page_member_of_staff(u16 skill_mask, u16 room_id, u8 count_present, s8 wanted, u8 owner)
{
    RoomInfo *room;
    SkillPager *free_pager;
    u16 count;
    u16 i;
    u16 bit;
    Thing *staff;
    SkillPager *pager;

    room = &room_info[room_id];
    free_pager = 0;
    count = 0;
    if (room->type != 0) {}
    room = &room_info[room_id];
    if (room->type == 4)
        return 0;
    for (i = 0; i < 9; i++) {
        if (skill_mask & (1 << i)) {
            bit = 1 << i;
            count++;
        }
    }
    if (count > 1) {
        page_member_of_staff(~bit & skill_mask, room_id, count_present, wanted, owner);
        skill_mask = bit;
    }
    if (count_present) {
        for (staff = &thing[room->first]; staff > thing; staff = &thing[staff->room_next]) {
            if (staff->renderer == 4 && (staff->unknown124 & skill_mask)) {
                if (--wanted <= 0)
                    return 1;
            }
        }
    }
    for (pager = pagers; pager < end_pagers; pager++) {
        if ((pager->mask & skill_mask) && pager->room == room_id) {
            if (--wanted <= 0)
                return 1;
        } else if (pager->mask == 0) {
            free_pager = pager;
        }
    }
    if (free_pager) {
        free_pager->mask = skill_mask;
        free_pager->room = room_id;
        free_pager->owner = owner;
        free_pager->unknown4 = 0;
        free_pager->unknown8 = 0;
        if (owner == player) {
            if (room->type >= 0 && room->type <= 0x1e)
                play_ambient(room->type);
        }
        if (wanted > 1)
            return page_member_of_staff(skill_mask, room_id, count_present, wanted - 1, owner);
    }
    return 0;
}

// FUNCTION: 0x00049acc
u16 check_pager(Thing *staff)
{
    SkillPager *pager;
    SkillPager *found;
    RoomInfo *room;
    u16 best;
    u16 dist;
    u16 room_id;

    found = 0;
    best = 999;
    for (pager = pagers; pager < end_pagers; pager++) {
        if ((pager->owner == staff->owner && (pager->mask & staff->unknown124) && pager->unknown4 < actual_game_turn)
            || (u16)(staff - thing) == pager->unknown8) {
            room = &room_info[pager->room];
            if (!(room->f3 == 1 || room->f3 == 0))
                continue;
            {
                dist = fn_000cb2db(staff->x.tile - room->unknown4 % 128)
                     + fn_000cb2db(staff->y.tile - room->unknown4 / 128);
                if (best > dist) {
                    best = dist;
                    found = pager;
                }
            }
        }
    }
    if (found) {
        room_id = found->room;
        found->unknown4 = actual_game_turn + 3000;
        found->unknown8 = staff - thing;
    } else {
        room_id = 0;
    }
    room = &room_info[room_id];
    if (room > room_info && room->type == 0) {
        for (pager = pagers; pager < end_pagers; pager++) {
            if (pager->room == room_id)
                memset(pager, 0, sizeof(SkillPager));
        }
        room_id = 0;
    }
    return room_id;
}

// FUNCTION: 0x00049ce0
void pager_arrival(Thing *staff)
{
    SkillPager *blank;
    SkillPager *pager;
    SkillPager *other;
    u16 id;

    id = staff - thing;
    other = 0;
    blank = 0;
    for (pager = pagers; pager < end_pagers; pager++) {
        if ((pager->mask & staff->unknown124) && (u16)(staff->room - room_info) == pager->room && pager->owner == staff->owner) {
            if ((u16)(staff - thing) == pager->unknown8)
                goto found;
            if (pager->unknown8)
                other = pager;
            else
                blank = pager;
        }
    }
    if (other)
        pager = other;
    else if (blank)
        pager = blank;
found:
    if (pager < end_pagers)
        pager->mask = pager->room = pager->owner = pager->unknown4 = pager->unknown8 = 0;
}

// FUNCTION: 0x00049e30
void clear_pager_responses(u16 room_id)
{
    SkillPager *pager;

    for (pager = pagers; pager < end_pagers; pager++) {
        if (pager->room == room_id) {
            pager->unknown4 = 0;
            pager->unknown8 = 0;
        }
    }
}

// FUNCTION: 0x00049e88
RoomInfo *get_expertise_dest(u16 type, Thing *patient)
{
    RoomInfo *best;
    RoomInfo *fallback;
    u32 best_score;
    Expertise *exp;
    RoomInfo *room;
    u32 score;
    Thing *occupant;
    u8 owner;
    u8 kind;

    best = 0;
    fallback = 0;
    best_score = 9999;
    owner = patient->owner;
    exp = &expertise[type];
    kind = exp->category;
    if (allhosps[owner].rooms[type].unknown1C == 1 && listhead[owner].rooms[kind]) {
        for (room = room_info + 1; room < end_room_info; room++) {
            if (room->owner == owner && room->type == kind && room->unknown4 != 0) {
                fallback = room;
                if (spare_queue_capacity(0, 1, room, owner)) {
                    score = room->unknown31 * gbv.f100;
                    score += (fn_000cb2db(patient->x.tile - room->unknown4 % 128)
                            + fn_000cb2db(patient->y.tile - room->unknown4 / 128)) * gbv.f102;
                    score += gbv.f104;
                    for (occupant = &thing[room->first]; occupant > thing; occupant = &thing[occupant->room_next]) {
                        if (occupant->renderer == 4) {
                            score -= gbv.f104;
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
void staff_abandon_pager_response(Thing *staff)
{
    SkillPager *pager;
    u16 id;

    id = staff - thing;
    for (pager = pagers; pager < end_pagers; pager++) {
        if (pager->unknown8 == id) {
            pager->unknown4 = 0;
            pager->unknown8 = 0;
        }
    }
}

// FUNCTION: 0x0004a244
void room_deleted_update_listhead(RoomInfo *room)
{
    u32 id;
    RoomInfo *other;

    id = 0;
    for (other = room_info + 1; other < end_room_info; other++) {
        if (other->type == room->type && other != room && other->owner == room->owner) {
            id = (u16)(other - room_info);
            break;
        }
    }
    listhead[room->owner].rooms[room->type] = id;
}

// FUNCTION: 0x0004a2fc
void set_expertise_staff(Thing *staff)
{
    Expertise *exp;
    u32 i;
    u32 kind;
    s32 ward;

    ward = allhosps[staff->owner].rooms[42].unknown1C == 1;
    for (i = 1; i < 0x2f; i++) {
        if (allhosps[staff->owner].rooms[i].unknown1C == 0) {
            exp = &expertise[i];
            if (listhead[staff->owner].rooms[exp->category] != 0) {
                if ((exp->unknown0 & 0x20) && allhosps[staff->owner].fca6 < 2)
                    continue;
                if (staff->unknown124 & exp->unknown0) {
                    if (exp->unknown7 && !ward)
                        continue;
                    kind = exp->kind;
                    if (kind == 0)
                        allhosps[staff->owner].rooms[i].unknown1C = 1;
                    else if (listhead[staff->owner].objects[kind])
                        allhosps[staff->owner].rooms[i].unknown1C = 1;
                }
            }
        }
    }
    if (!ward && allhosps[staff->owner].rooms[42].unknown1C == 1)
        set_expertise_ward(staff->owner);
}

// FUNCTION: 0x0004a520
void set_expertise_room(RoomInfo *room)
{
    Expertise *exp;
    u32 i;
    u32 mask;
    u32 j;
    s32 ward;

    ward = allhosps[room->owner].rooms[42].unknown1C == 1;
    for (i = 1; i < 0x2f; i++) {
        if (allhosps[room->owner].rooms[i].unknown1C == 0) {
            exp = &expertise[i];
            if (exp->unknown7 && !ward)
                continue;
            if (exp->category == room->type) {
                mask = exp->unknown0;
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

// FUNCTION: 0x0004a754
void remove_expertise_staff(Thing *staff)
{
    u16 j;
    u16 i;
    u16 mask;
    s32 ward;

    ward = allhosps[staff->owner].rooms[42].unknown1C == 1;
    remove_skill(staff->unknown124, staff->owner);
    mask = staff->unknown124;
    for (i = 0; i < 9; i++) {
        if (mask & (1 << i)) {
            if (listhead[staff->owner].staff[i] == 0 || (i == 5 && allhosps[staff->owner].fca6 < 3)) {
                for (j = 1; j < 0x2f; j++) {
                    if (allhosps[staff->owner].rooms[j].unknown1C == 1) {
                        if ((1 << i) & expertise[j].unknown0)
                            allhosps[staff->owner].rooms[j].unknown1C = 0;
                    }
                }
            }
        }
    }
    if (ward && allhosps[staff->owner].rooms[42].unknown1C != 1)
        remove_expertise_ward(staff->owner);
}

// FUNCTION: 0x0004a964
void remove_expertise_room(RoomInfo *room)
{
    u16 i;
    RoomInfo *other;
    s32 ward;

    ward = allhosps[room->owner].rooms[42].unknown1C == 1;
    for (other = room_info + 1; other < end_room_info; other++) {
        if (other->owner == room->owner && room->type == other->type && other != room)
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

// FUNCTION: 0x0004aae4
void set_expertise_ward(u8 owner)
{
    u32 i;
    Expertise *exp;
    u32 mask;
    u32 j;

    for (i = 1; i < 0x2f; i++) {
        if (allhosps[owner].rooms[i].unknown1C == 0) {
            exp = &expertise[i];
            if (exp->unknown7) {
                if (listhead[owner].rooms[exp->category] != 0) {
                    if ((exp->unknown0 & 0x20) && allhosps[owner].fca6 < 2)
                        continue;
                    mask = exp->unknown0;
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
