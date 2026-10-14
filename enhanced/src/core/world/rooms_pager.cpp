// Paging staff to rooms, the player calls and the handymen's thing pages
// (src/skills.cpp, src/handyman.cpp).

#include "core/world/rooms.h"
#include "core/world/rooms_internal.h"

#include "core/ui/charts.h"
#include "core/world/build.h"
#include "core/world/disaster.h"
#include "core/world/epidemic.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/mayor.h"
#include "core/world/popups.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

#include <algorithm>
#include <bit>
#include <cstdlib>

namespace th
{

namespace
{

// Room types (the game's ROOM_*).
constexpr uint8_t kRoomTheatre = 10;

// RoomInfo.state values (RS_*) of rooms staff answer pages to: open, in
// use; and of a room being edited.
constexpr int8_t kRoomStateOpen = 0;

constexpr int8_t kRoomStateInUse = 1;

constexpr int8_t kRoomEdited = 4;

// The announcements (play_ambient's Ambients indexes) of the epidemic call
// and the inspector's result: a base plus a draw from 0 to 2.
constexpr uint8_t kAnnounceEpidemic = 0xaf;
constexpr uint8_t kAnnounceInspector = 0xb2;
constexpr uint16_t kInspectorSubject = 3;

// The last room type a page announces (play_ambient(room->type)).
constexpr uint8_t kLastPagedRoomType = 0x1e;

// make_a_playercall's announcement of an epidemic or of the inspector's
// result for the player. The draw comes from saferand, the generator the
// shell shares (Game::safeseed), and is made whether or not the sound plays.
void announce_playercall(World &world, uint16_t type, uint16_t subject, uint8_t owner)
{
    return;
    uint8_t base = kAnnounceEpidemic;
    if (type == kCallEpidemicOver && subject == kInspectorSubject)
        base = kAnnounceInspector;
    else if (type != kCallEpidemic)
        return;
    if (owner != 0)
        return;
    uint16_t pick = 0;
    if (world.game) {
        uint32_t &seed = world.game->safeseed;
        seed = std::rotr(seed * 0x24a1u + 0x24dfu, 13);
        pick = uint16_t(uint16_t(seed) % 3);
    }
    world.machine_announcements.push_back(uint8_t(base + pick));
}

// page_member_of_staff: counts the staff with a skill in `skill_mask` already
// in the room off `wanted`. True when no more are wanted.
bool page_member_of_staff_count_present(const World &world, uint16_t skill_mask, uint16_t room_id, int8_t &wanted)
{
    for (const Thing &t : world.things) {
        if (t.type == kThingStaff && t.room == room_id && (t.skills & skill_mask)) {
            if (--wanted <= 0)
                return true;
        }
    }
    return false;
}

} // namespace

bool page_member_of_staff(World &world, uint16_t skill_mask, uint16_t room_id, bool count_present, int8_t wanted,
                          uint8_t owner)
{
    trace_event("page room=%u mask=%x wanted=%d", unsigned(room_id), unsigned(skill_mask), int(wanted));
    if (world.rooms[room_id].type == kRoomReception)
        return false;
    int count = 0;
    uint16_t bit = 0;
    for (int i = 0; i < 9; i++) {
        if (skill_mask & (1 << i)) {
            bit = uint16_t(1 << i);
            count++;
        }
    }
    if (count > 1) {
        page_member_of_staff(world, uint16_t(~bit & skill_mask), room_id, count_present, wanted, owner);
        skill_mask = bit;
    }
    if (count_present && page_member_of_staff_count_present(world, skill_mask, room_id, wanted))
        return true;
    World::Pager *free_pager = nullptr;
    for (World::Pager &pager : world.pagers) {
        if ((pager.mask & skill_mask) && pager.room == room_id) {
            if (--wanted <= 0)
                return true;
        } else if (pager.mask == 0) {
            free_pager = &pager;
        }
    }
    if (free_pager) {
        *free_pager = {skill_mask, room_id, 0, 0, owner};
        if (owner == 0 && world.rooms[room_id].type <= kLastPagedRoomType)
            world.machine_announcements.push_back(world.rooms[room_id].type);
        if (wanted > 1)
            return page_member_of_staff(world, skill_mask, room_id, count_present, int8_t(wanted - 1), owner);
    }
    return false;
}

void page_staff_i_need(World &world, uint16_t person, uint16_t room)
{
    const Thing &p = world.things[person];
    int8_t count = world.rooms[room].type == kRoomTheatre ? 2 : 1;
    page_member_of_staff(world, kExpertiseData[p.treatment].staff_mask, room, true, count, p.owner);
}

uint16_t check_pager(World &world, uint16_t id, uint32_t game_turn)
{
    const Thing &staff = world.things[id];
    World::Pager *found = nullptr;
    uint16_t best = 999;
    for (World::Pager &pager : world.pagers) {
        if ((pager.owner == staff.owner && (pager.mask & staff.skills) && pager.claimed_until < game_turn) ||
            id == pager.responder) {
            const RoomInfo &room = world.rooms[pager.room];
            if (!(room.state == kRoomStateInUse || room.state == kRoomStateOpen))
                continue;
            auto dist = uint16_t(std::abs(staff.x.tile - room.door_tile % kMapSize) +
                                 std::abs(staff.y.tile - room.door_tile / kMapSize));
            if (best > dist) {
                best = dist;
                found = &pager;
            }
        }
    }
    uint16_t room_id = 0;
    if (found) {
        room_id = found->room;
        found->claimed_until = game_turn + 3000;
        found->responder = id;
    }
    if (room_id > 0 && world.rooms[room_id].type == kRoomFree) {
        for (World::Pager &pager : world.pagers)
            if (pager.room == room_id)
                pager = {};
        room_id = 0;
    }
    if (room_id)
        trace_event("check_pager %u room=%u", unsigned(id), unsigned(room_id));
    return room_id;
}

void pager_arrival(World &world, uint16_t id)
{
    trace_event("pager_arrival %u", unsigned(id));
    const Thing &staff = world.things[id];
    World::Pager *other = nullptr;
    World::Pager *blank = nullptr;
    for (World::Pager &pager : world.pagers) {
        if ((pager.mask & staff.skills) && staff.room == pager.room && pager.owner == staff.owner) {
            if (id == pager.responder) {
                pager = {};
                return;
            }
            if (pager.responder)
                other = &pager;
            else
                blank = &pager;
        }
    }
    if (other)
        *other = {};
    else if (blank)
        *blank = {};
}

void clear_pager_responses(World &world, uint16_t room)
{
    trace_event("clear_pagers room=%u", unsigned(room));
    for (World::Pager &pager : world.pagers) {
        if (pager.room == room) {
            pager.claimed_until = 0;
            pager.responder = 0;
        }
    }
}

void staff_abandon_pager_response(World &world, uint16_t id)
{
    trace_event("abandon_pager %u", unsigned(id));
    for (World::Pager &pager : world.pagers) {
        if (pager.responder == id) {
            pager.claimed_until = 0;
            pager.responder = 0;
        }
    }
}

uint16_t make_a_playercall(World &world, uint16_t type, uint16_t subject, int16_t timeout, uint8_t owner)
{
    // The rat game has no calls.
    if (world.rats.fest_level)
        return 0;
    for (size_t i = 1; i < world.calls.size(); i++) {
        World::PlayerCall &call = world.calls[i];
        if (call.type == 0) {
            announce_playercall(world, type, subject, owner);
            // The panel slot and slide step are left as they were until
            // add_panel_popup sets them, as in the game.
            call.type = type;
            call.id = world.next_call_id++;
            call.subject = subject;
            call.timer = timeout;
            call.owner = owner;
            add_panel_popup(world, uint16_t(i));
            uint16_t &head = world.first_call[owner & 3][type % 9];
            if (head == 0)
                head = uint16_t(i);
            trace_event("playercall %u type=%u subject=%u owner=%u", unsigned(call.id), unsigned(type),
                        unsigned(subject), unsigned(owner));
            return call.id;
        }
    }
    return 0;
}

void expire_playercalls(World &world)
{
    bool changed = false;
    for (size_t i = 1; i < world.calls.size(); i++) {
        World::PlayerCall &call = world.calls[i];
        if (!call.timer || --call.timer > 0)
            continue;
        trace_event("expire_call %u type=%u subject=%u", unsigned(call.id), unsigned(call.type),
                    unsigned(call.subject));
        if (call.type == kCallEmergencyOver && call.subject == 1 && call.owner == 0)
            world.emergency.state = 0;
        else if (call.type == kCallVip && call.owner == 0) {
            world.mayor.arrival_turn = (world.game ? world.game->turn_count : 0) + 1500;
            world.mayor.state = 2;
        } else if (call.type == kCallEpidemic && call.owner == 0) {
            world.epidemic.cover_up_expired = true;
        }
        uint16_t &head = world.first_call[call.owner & 3][call.type % 9];
        if (head == i) {
            head = 0;
            changed = true;
        }
        delete_panel_popup(world, uint16_t(i));
        call.type = 0;
    }
    if (!changed)
        return;
    // The heads lost go to the first call of their kind in the table.
    for (size_t i = 1; i < world.calls.size(); i++) {
        const World::PlayerCall &call = world.calls[i];
        uint16_t &head = world.first_call[call.owner & 3][call.type % 9];
        if (call.type && head == 0)
            head = uint16_t(i);
    }
}

uint16_t playercall_subject(const World &world, uint16_t type, uint16_t id, uint8_t owner)
{
    for (size_t i = 1; i < world.calls.size(); i++) {
        const World::PlayerCall &call = world.calls[i];
        if (call.type == type && call.owner == owner && (id == 0 || id == call.id))
            return call.subject;
    }
    return 0;
}

uint16_t get_playercall(World &world, uint16_t type, uint16_t id, uint8_t owner)
{
    trace_event("get_playercall %u type=%u owner=%u", unsigned(id), unsigned(type), unsigned(owner));
    for (size_t i = 1; i < world.calls.size(); i++) {
        World::PlayerCall &call = world.calls[i];
        if (call.type != type || call.owner != owner)
            continue;
        if (id == 0 || id == call.id) {
            uint16_t subject = call.subject;
            call.type = 0;
            delete_panel_popup(world, uint16_t(i));
            uint16_t &head = world.first_call[owner & 3][type % 9];
            if (head == i) {
                head = 0;
                get_new_popup_head(world, type, owner);
            }
            return subject;
        }
    }
    return 0;
}

uint16_t check_playercall(const World &world, uint16_t type, uint16_t subject, uint8_t owner)
{
    uint16_t count = 0;
    for (size_t i = 1; i < world.calls.size(); i++) {
        const World::PlayerCall &call = world.calls[i];
        if (call.type != type || call.owner != owner)
            continue;
        if (type == 2 || type == 3 || type == 4 || type == 8 || call.subject == subject)
            count++;
    }
    return count;
}

bool thingpage_request(World::ThingPager &pager, uint16_t id, uint8_t priority)
{
    if (pager.count == pager.pages.size())
        return false;
    for (int i = pager.count - 1; i >= 0; i--) {
        World::ThingPage &page = pager.pages[size_t(i)];
        if (page.id == id) {
            if (page.priority < priority)
                page.priority = priority;
            return true;
        }
    }
    pager.pages[pager.count++] = {id, priority, 0, 0};
    return true;
}

bool thingpage_cancel(World::ThingPager &pager, uint16_t id)
{
    for (int i = pager.count - 1; i >= 0; i--) {
        if (pager.pages[size_t(i)].id == id) {
            pager.pages[size_t(i)] = pager.pages[pager.count - 1u];
            pager.count--;
            return true;
        }
    }
    return false;
}

uint16_t thingpage_respond(World &world, World::ThingPager &pager, uint16_t handyman, uint16_t room, uint8_t min,
                           uint32_t game_turn)
{
    const Thing &h = world.things[handyman];
    int best_priority = -1;
    int best = -1;
    uint32_t best_dist = 0x4000;
    auto dist = [&](const Thing &t) {
        int dx = cell_of(h) % kMapSize - cell_of(t) % kMapSize;
        int dy = cell_of(h) / kMapSize - cell_of(t) / kMapSize;
        return uint32_t(dx * dx + dy * dy);
    };
    for (int i = pager.count - 1; i >= 0; i--) {
        const World::ThingPage &page = pager.pages[size_t(i)];
        const Thing &t = world.things[page.id];
        if (room && t.room_id != room)
            continue;
        if (page.responder != handyman && page.claimed_until > game_turn)
            continue;
        if (page.priority < min)
            continue;
        if (t.room_id != 0) {
            const RoomInfo &r = world.rooms[t.room_id];
            if (r.being_edited || r.state == kRoomEdited || (h.owner == 0 && world.building.build.room == t.room_id))
                continue;
        }
        if (page.priority == best_priority) {
            if (dist(t) < best_dist) {
                best = i;
                best_dist = dist(t);
            }
        } else if (page.priority > best_priority) {
            best = i;
            best_dist = dist(t);
            best_priority = page.priority;
        }
    }
    if (best == -1)
        return 0;
    World::ThingPage &page = pager.pages[size_t(best)];
    page.responder = handyman;
    page.claimed_until = game_turn + 3000;
    return page.id;
}

uint8_t thingpage_find_highest_priority(const World::ThingPager &pager)
{
    int best = -1;
    for (int i = pager.count - 1; i >= 0; i--)
        best = std::max(best, int(pager.pages[size_t(i)].priority));
    return uint8_t(best);
}

uint16_t thingpage_remind_me(const World::ThingPager &pager, uint16_t handyman)
{
    for (int i = pager.count - 1; i >= 0; i--)
        if (pager.pages[size_t(i)].responder == handyman)
            return pager.pages[size_t(i)].id;
    return 0;
}

void thingpage_cancel_my_response(World::ThingPager &pager, uint16_t handyman)
{
    for (int i = pager.count - 1; i >= 0; i--) {
        World::ThingPage &page = pager.pages[size_t(i)];
        if (page.responder == handyman) {
            page.responder = 0;
            page.claimed_until = 0;
        }
    }
}

} // namespace th
