#include "hosp.h"

extern "C" {
}

// Pages staff with the skills in skill_mask (SKILL_* bits, one page per
// skill) to room room_id for player `owner`, unless `wanted` of them are
// already in the room (when count_present is set) or paged; posts up to
// `wanted` pages. Never pages to a reception. Returns 1 if no new page was
// needed, else 0.
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
    if (room->type != ROOM_FREE) {
    }
    room = &room_info[room_id];
    if (room->type == ROOM_RECEPTION)
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
            if (staff->type == THING_STAFF && (staff->skills & skill_mask)) {
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
        free_pager->claimed_until = 0;
        free_pager->responder = 0;
        if (owner == player) {
            if (room->type >= 0 && room->type <= 0x1e)
                play_ambient(room->type);
        }
        if (wanted > 1)
            return page_member_of_staff(skill_mask, room_id, count_present, wanted - 1, owner);
    }
    return 0;
}

// Takes the nearest page staff answers (matching their skills, not answered
// in the last 3000 turns, or already theirs) and marks it as answered by
// them. Returns the paged room, or 0 if none (dropping pages to a room that
// no longer exists).
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
        if ((pager->owner == staff->owner && (pager->mask & staff->skills) &&
             pager->claimed_until < actual_game_turn) ||
            (u16)(staff - thing) == pager->responder) {
            room = &room_info[pager->room];
            if (!(room->state == 1 || room->state == 0))
                continue;
            {
                dist = abs(staff->x.tile - room->door_tile % 128) + abs(staff->y.tile - room->door_tile / 128);
                if (best > dist) {
                    best = dist;
                    found = pager;
                }
            }
        }
    }
    if (found) {
        room_id = found->room;
        found->claimed_until = actual_game_turn + 3000;
        found->responder = staff - thing;
    } else {
        room_id = 0;
    }
    room = &room_info[room_id];
    if (room > room_info && room->type == ROOM_FREE) {
        for (pager = pagers; pager < end_pagers; pager++) {
            if (pager->room == room_id)
                memset(pager, 0, sizeof(SkillPager));
        }
        room_id = 0;
    }
    return room_id;
}

// Staff arriving in a room clear the page there that matches their skills:
// their own, else one answered by someone else, else an unanswered one.
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
        if ((pager->mask & staff->skills) && (u16)(staff->room - room_info) == pager->room &&
            pager->owner == staff->owner) {
            if ((u16)(staff - thing) == pager->responder)
                goto found;
            if (pager->responder)
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
        pager->mask = pager->room = pager->owner = pager->claimed_until = pager->responder = 0;
}

// Marks every page to room room_id as unanswered.
// FUNCTION: 0x00049e30
void clear_pager_responses(u16 room_id)
{
    SkillPager *pager;

    for (pager = pagers; pager < end_pagers; pager++) {
        if (pager->room == room_id) {
            pager->claimed_until = 0;
            pager->responder = 0;
        }
    }
}

// Returns the room of player `owner`'s for treatment type (an expertise
// index) that patient should go to: the one with the best score from queue
// length, distance and staff present among those with space, else any
// room of the kind, or 0 if the treatment is unavailable.
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
    if (allhosps[owner].rooms[type].cure_available == 1 && listhead[owner].rooms[kind]) {
        for (room = room_info + 1; room < end_room_info; room++) {
            if (room->owner == owner && room->type == kind && room->door_tile != 0) {
                fallback = room;
                if (spare_queue_capacity(0, 1, room, owner)) {
                    score = room->queue_length * gbv.q_points;
                    score +=
                        (abs(patient->x.tile - room->door_tile % 128) + abs(patient->y.tile - room->door_tile / 128)) *
                        gbv.dist_points;
                    score += gbv.no_staff_points;
                    for (occupant = &thing[room->first]; occupant > thing; occupant = &thing[occupant->room_next]) {
                        if (occupant->type == THING_STAFF) {
                            score -= gbv.no_staff_points;
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

// Counts a staff member with skill bits mask in player `owner`'s per-skill
// staff counts (listhead.staff).
// FUNCTION: 0x0004a0d0
void insert_skill(u16 mask, u8 owner)
{
    u16 i;

    for (i = 0; i < 9; i++) {
        if (mask & (1 << i))
            listhead[owner].staff[i]++;
    }
}

// Uncounts a staff member with skill bits mask from player `owner`'s
// per-skill staff counts.
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

// Drops staff's answer to any page (the page becomes unanswered).
// FUNCTION: 0x0004a1d4
void staff_abandon_pager_response(Thing *staff)
{
    SkillPager *pager;
    u16 id;

    id = staff - thing;
    for (pager = pagers; pager < end_pagers; pager++) {
        if (pager->responder == id) {
            pager->claimed_until = 0;
            pager->responder = 0;
        }
    }
}

// Before room is removed: points the owner's room list for its kind
// (listhead.rooms) at another room of the kind, or 0.
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

// A staff member joins: makes available each treatment that their skills
// complete (it already has its room and machine; surgery needs two
// surgeons, counted in allhosps.surgeon_count), and those that need a ward.
// FUNCTION: 0x0004a2fc
void set_expertise_staff(Thing *staff)
{
    Expertise *exp;
    u32 i;
    u32 kind;
    s32 ward;

    ward = allhosps[staff->owner].rooms[ILL_DIAG_WARD].cure_available == 1;
    for (i = 1; i < 0x2f; i++) {
        if (allhosps[staff->owner].rooms[i].cure_available == 0) {
            exp = &expertise[i];
            if (listhead[staff->owner].rooms[exp->category] != 0) {
                if ((exp->staff_mask & 0x20) && allhosps[staff->owner].surgeon_count < 2)
                    continue;
                if (staff->skills & exp->staff_mask) {
                    if (exp->needs_ward && !ward)
                        continue;
                    kind = exp->kind;
                    if (kind == 0)
                        allhosps[staff->owner].rooms[i].cure_available = 1;
                    else if (listhead[staff->owner].objects[kind])
                        allhosps[staff->owner].rooms[i].cure_available = 1;
                }
            }
        }
    }
    if (!ward && allhosps[staff->owner].rooms[ILL_DIAG_WARD].cure_available == 1)
        set_expertise_ward(staff->owner);
}

// A room is built: makes available each treatment done in it whose skills
// the hospital has (none for research rooms without skills), and those that
// need a ward.
// FUNCTION: 0x0004a520
void set_expertise_room(RoomInfo *room)
{
    Expertise *exp;
    u32 i;
    u32 mask;
    u32 j;
    s32 ward;

    ward = allhosps[room->owner].rooms[ILL_DIAG_WARD].cure_available == 1;
    for (i = 1; i < 0x2f; i++) {
        if (allhosps[room->owner].rooms[i].cure_available == 0) {
            exp = &expertise[i];
            if (exp->needs_ward && !ward)
                continue;
            if (exp->category == room->type) {
                mask = exp->staff_mask;
                if (mask == 0) {
                    if (room->type != ROOM_RESEARCH)
                        allhosps[room->owner].rooms[i].cure_available = 1;
                } else {
                    for (j = 0; j < 9; j++) {
                        if (((1 << j) & mask) == 0x20 && allhosps[room->owner].surgeon_count < 2)
                            break;
                        if ((mask & (1 << j)) && listhead[room->owner].staff[j] == 0)
                            break;
                    }
                    if (j >= 9)
                        allhosps[room->owner].rooms[i].cure_available = 1;
                }
            }
        }
    }
    if (!ward && allhosps[room->owner].rooms[ILL_DIAG_WARD].cure_available == 1)
        set_expertise_ward(room->owner);
}

// A staff member leaves: uncounts their skills and makes unavailable the
// treatments that needed the last of a skill (and surgery whenever the
// surgeon count, allhosps.surgeon_count, is below 3), and those that need a ward.
// FUNCTION: 0x0004a754
void remove_expertise_staff(Thing *staff)
{
    u16 j;
    u16 i;
    u16 mask;
    s32 ward;

    ward = allhosps[staff->owner].rooms[ILL_DIAG_WARD].cure_available == 1;
    remove_skill(staff->skills, staff->owner);
    mask = staff->skills;
    for (i = 0; i < 9; i++) {
        if (mask & (1 << i)) {
            if (listhead[staff->owner].staff[i] == 0 || (i == 5 && allhosps[staff->owner].surgeon_count < 3)) {
                for (j = 1; j < 0x2f; j++) {
                    if (allhosps[staff->owner].rooms[j].cure_available == 1) {
                        if ((1 << i) & expertise[j].staff_mask)
                            allhosps[staff->owner].rooms[j].cure_available = 0;
                    }
                }
            }
        }
    }
    if (ward && allhosps[staff->owner].rooms[ILL_DIAG_WARD].cure_available != 1)
        remove_expertise_ward(staff->owner);
}

// A room is removed: if it was the owner's last of its kind, makes the
// treatments done in it unavailable, and those that need a ward.
// FUNCTION: 0x0004a964
void remove_expertise_room(RoomInfo *room)
{
    u16 i;
    RoomInfo *other;
    s32 ward;

    ward = allhosps[room->owner].rooms[ILL_DIAG_WARD].cure_available == 1;
    for (other = room_info + 1; other < end_room_info; other++) {
        if (other->owner == room->owner && room->type == other->type && other != room)
            return;
    }
    for (i = 1; i < 0x2f; i++) {
        if (allhosps[room->owner].rooms[i].cure_available == 1) {
            if (room->type == expertise[i].category)
                allhosps[room->owner].rooms[i].cure_available = 0;
        }
    }
    if (ward && allhosps[room->owner].rooms[ILL_DIAG_WARD].cure_available != 1)
        remove_expertise_ward(room->owner);
}

// Makes available player `owner`'s ward treatments whose room and staff
// they have.
// FUNCTION: 0x0004aae4
void set_expertise_ward(u8 owner)
{
    u32 i;
    Expertise *exp;
    u32 mask;
    u32 j;

    for (i = 1; i < 0x2f; i++) {
        if (allhosps[owner].rooms[i].cure_available == 0) {
            exp = &expertise[i];
            if (exp->needs_ward) {
                if (listhead[owner].rooms[exp->category] != 0) {
                    if ((exp->staff_mask & 0x20) && allhosps[owner].surgeon_count < 2)
                        continue;
                    mask = exp->staff_mask;
                    for (j = 0; j < 9; j++) {
                        if ((mask & (1 << j)) && listhead[owner].staff[j] == 0)
                            break;
                    }
                    if (j >= 9)
                        allhosps[owner].rooms[i].cure_available = 1;
                }
            }
        }
    }
}

// Makes all of player `owner`'s ward treatments unavailable.
// FUNCTION: 0x0004ac4c
void remove_expertise_ward(u8 owner)
{
    u32 i;

    for (i = 1; i < 0x2f; i++) {
        if (allhosps[owner].rooms[i].cure_available == 1 && expertise[i].needs_ward)
            allhosps[owner].rooms[i].cure_available = 0;
    }
}
