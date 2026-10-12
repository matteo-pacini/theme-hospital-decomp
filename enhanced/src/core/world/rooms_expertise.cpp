// The skills a room's staff provide: the room and staff expertise lists (src/skills.cpp).

#include "core/world/rooms.h"
#include "core/world/rooms_internal.h"

#include "core/ui/charts.h"
#include "core/world/build.h"
#include "core/world/finance.h"
#include "core/world/icons.h"
#include "core/world/queue.h"
#include "core/world/staff.h"

namespace th
{

namespace
{

// Room types (the game's ROOM_*).
constexpr uint8_t kRoomResearch = 28;

// listhead[owner].objects[kind] != 0: only hospital 0's are listed.
bool has_object(const World &world, uint8_t owner, uint8_t kind)
{
    return owner == 0 && world.first_of_kind[kind] != 0;
}

// set_expertise_ward (0x4aae4). Member of staff `hired`, if any, is not
// counted yet: set_expertise_staff runs before the game counts their skills.
void set_expertise_ward(World &world, uint8_t owner, uint16_t hired = 0)
{
    for (size_t i = 1; i < 0x2f; i++) {
        const ExpertiseInfo &exp = kExpertiseData[i];
        if (world.cure_available[i] != 0 || !exp.needs_ward || owner != 0 || !world.room_of_type[exp.category])
            continue;
        if ((exp.staff_mask & kSkillSurgeon) && surgeon_count(world, owner) < 2)
            continue;
        int j = 0;
        for (; j < 9; j++)
            if ((exp.staff_mask & (1 << j)) &&
                staff_with_skill(world, owner, j) - (hired && (world.things[hired].skills & (1 << j))) == 0)
                break;
        if (j >= 9)
            world.cure_available[i] = 1;
    }
}

// remove_expertise_ward (0x4ac4c).
void remove_expertise_ward(World &world)
{
    for (size_t i = 1; i < 0x2f; i++)
        if (world.cure_available[i] == 1 && kExpertiseData[i].needs_ward)
            world.cure_available[i] = 0;
}

// set_expertise_room: whether hospital `owner` employs staff with every skill
// in `mask` (two surgeons for surgery).
bool set_expertise_room_has_staff(const World &world, uint8_t owner, uint16_t mask)
{
    for (int j = 0; j < 9; j++) {
        if (((1 << j) & mask) == kSkillSurgeon && surgeon_count(world, owner) < 2)
            return false;
        if ((mask & (1 << j)) && staff_with_skill(world, owner, j) == 0)
            return false;
    }
    return true;
}

} // namespace

void set_expertise_room(World &world, uint16_t room_id)
{
    const RoomInfo &room = world.rooms[room_id];
    // Only hospital 0's treatments are kept (World::cure_available).
    if (room.owner != 0)
        return;
    bool ward = world.cure_available[kIllDiagWard] == 1;
    for (size_t i = 1; i < 0x2f; i++) {
        if (world.cure_available[i] != 0)
            continue;
        const ExpertiseInfo &exp = kExpertiseData[i];
        if (exp.needs_ward && !ward)
            continue;
        if (exp.category != room.type)
            continue;
        uint16_t mask = exp.staff_mask;
        if (mask == 0) {
            if (room.type != kRoomResearch)
                world.cure_available[i] = 1;
            continue;
        }
        if (set_expertise_room_has_staff(world, room.owner, mask))
            world.cure_available[i] = 1;
    }
    if (!ward && world.cure_available[kIllDiagWard] == 1)
        set_expertise_ward(world, room.owner);
}

void set_expertise_staff(World &world, uint16_t id)
{
    const Thing &staff = world.things[id];
    if (staff.owner != 0) {
        if (staff.owner < world.rivals.size())
            set_expertise_staff(world.rivals[staff.owner], staff);
        return;
    }
    bool ward = world.cure_available[kIllDiagWard] == 1;
    for (size_t i = 1; i < 0x2f; i++) {
        if (world.cure_available[i] != 0)
            continue;
        const ExpertiseInfo &exp = kExpertiseData[i];
        if (!world.room_of_type[exp.category])
            continue;
        if ((exp.staff_mask & kSkillSurgeon) && surgeon_count(world, staff.owner) < 2)
            continue;
        if (!(staff.skills & exp.staff_mask))
            continue;
        if (exp.needs_ward && !ward)
            continue;
        if (exp.kind == 0 || has_object(world, staff.owner, exp.kind))
            world.cure_available[i] = 1;
    }
    if (!ward && world.cure_available[kIllDiagWard] == 1)
        set_expertise_ward(world, staff.owner, id);
}

void remove_expertise_staff(World &world, uint16_t id)
{
    const Thing &staff = world.things[id];
    if (staff.owner != 0)
        return;
    bool ward = world.cure_available[kIllDiagWard] == 1;
    for (int i = 0; i < 9; i++) {
        if (!(staff.skills & (1 << i)))
            continue;
        // The game has taken their skills off its counts (remove_skill) but
        // not yet the surgeon off allhosps.surgeon_count.
        if (staff_with_skill(world, staff.owner, i) - 1 != 0 && !(i == 5 && surgeon_count(world, staff.owner) < 3))
            continue;
        for (size_t j = 1; j < 0x2f; j++)
            if (world.cure_available[j] == 1 && ((1 << i) & kExpertiseData[j].staff_mask))
                world.cure_available[j] = 0;
    }
    if (ward && world.cure_available[kIllDiagWard] != 1)
        remove_expertise_ward(world);
}

void remove_expertise_room(World &world, uint16_t room_id)
{
    const RoomInfo &room = world.rooms[room_id];
    if (room.owner != 0)
        return;
    bool ward = world.cure_available[kIllDiagWard] == 1;
    for (uint16_t r = 1; r < kRoomCount; r++)
        if (world.rooms[r].owner == room.owner && world.rooms[r].type == room.type && r != room_id)
            return;
    for (size_t i = 1; i < 0x2f; i++)
        if (world.cure_available[i] == 1 && kExpertiseData[i].category == room.type)
            world.cure_available[i] = 0;
    if (ward && world.cure_available[kIllDiagWard] != 1)
        remove_expertise_ward(world);
}

void room_deleted_update_listhead(World &world, uint16_t room_id)
{
    const RoomInfo &room = world.rooms[room_id];
    uint16_t id = 0;
    for (uint16_t r = 1; r < kRoomCount; r++) {
        if (world.rooms[r].type == room.type && r != room_id && world.rooms[r].owner == room.owner) {
            id = r;
            break;
        }
    }
    if (room.owner == 0)
        world.room_of_type[room.type % world.room_of_type.size()] = id;
}

} // namespace th
