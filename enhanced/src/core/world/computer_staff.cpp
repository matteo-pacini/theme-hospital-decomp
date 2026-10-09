// The computer players' hiring and building (src/computer.cpp: computer_employ,
// computer_build, computer_place_object) and the counts and treatments they
// change (inc_staff_stats, insert_skill, inc_hardware_stats, set_expertise_*).

#include "core/data/balance.h"
#include "core/world/build.h"
#include "core/world/computer.h"
#include "core/world/object_tables.h"
#include "core/world/queue.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

namespace th
{

namespace
{

constexpr uint8_t kRoomResearch = 28;
// objects[].category of a reception desk.
constexpr uint8_t kCategoryReceptionDesk = 7;
// Staff skill bits the game counts (insert_skill, computer_employ).
constexpr int kSkillBits = 9;

// set_expertise_ward (0x4aae4) for a computer player: makes available its ward
// treatments whose room and staff it has.
void set_expertise_ward(RivalHospital &rival)
{
    for (size_t i = 1; i < 0x2f; i++) {
        const ExpertiseInfo &exp = kExpertiseData[i];
        if (rival.cure_available[i] != 0 || !exp.needs_ward || rival.rooms[exp.category] == 0)
            continue;
        if ((exp.staff_mask & kSkillSurgeon) && rival.surgeon_count < 2)
            continue;
        int j = 0;
        for (; j < kSkillBits; j++)
            if ((exp.staff_mask & (1 << j)) && rival.staff_by_skill[size_t(j)] == 0)
                break;
        if (j >= kSkillBits)
            rival.cure_available[i] = 1;
    }
}

// inc_staff_stats (0x2a094): counts the hire `staff` in `rival`'s staff
// statistics (total, per kind and surgeons).
void inc_staff_stats(RivalHospital &rival, const Thing &staff)
{
    rival.staff_by_kind[staff.subtype == 4 ? 1 : staff.subtype]++;
    if (staff.skills & kSkillSurgeon)
        rival.surgeon_count++;
    rival.staff_count++;
}

// insert_skill (0x4a0d0): counts a hire with skill bits `mask` in `rival`'s
// per-skill staff counts.
void insert_skill(RivalHospital &rival, uint16_t mask)
{
    for (int i = 0; i < kSkillBits; i++)
        if (mask & (1 << i))
            rival.staff_by_skill[size_t(i)]++;
}

// inc_hardware_stats (0x39f8c): counts a new object of kind `kind` in
// `rival`'s hardware statistics and the value of its objects.
void inc_hardware_stats(RivalHospital &rival, uint8_t kind)
{
    RivalObjectStats &stats = rival.object_stats[kind];
    switch (kObjects[kind].category) {
    case 1:
    case 2:
        rival.machine_count++;
        rival.object_value += stats.cost;
        break;
    case 3:
        rival.leisure_count++;
        rival.object_value += stats.cost;
        break;
    case 4:
        rival.amenity_count++;
        break;
    case 5:
        rival.extinguisher_count++;
        break;
    case 6:
        rival.radiator_count++;
        break;
    default:
        break;
    }
    rival.object_count++;
    stats.count++;
}

// computer_employ's hire of applicant `id` for `owner`, who has paid `pay`
// for them.
void computer_employ_hire(World &world, uint16_t id, uint8_t owner, int32_t pay)
{
    RivalHospital &rival = world.rivals[owner];
    Thing &c = world.things[id];
    inc_staff_stats(rival, c);
    rival.balance -= pay;
    rival.wage_bill += c.salary;
    rival.staff_ability_sum[c.subtype] = int16_t(rival.staff_ability_sum[c.subtype] + c.ability);
    c.owner = owner;
    trace_event("computer_employ %u thing=%u subtype=%u balance=%d", unsigned(owner), unsigned(id), unsigned(c.subtype),
                int(rival.balance));
    set_expertise_staff(world, id);
    insert_skill(rival, c.skills);
    remove_staff_in_struct(world, c);
}

} // namespace

void set_expertise_staff(RivalHospital &rival, const Thing &staff)
{
    bool ward = rival.cure_available[kIllDiagWard] == 1;
    for (size_t i = 1; i < 0x2f; i++) {
        const ExpertiseInfo &exp = kExpertiseData[i];
        if (rival.cure_available[i] != 0 || rival.rooms[exp.category] == 0)
            continue;
        if ((exp.staff_mask & kSkillSurgeon) && rival.surgeon_count < 2)
            continue;
        if (!(staff.skills & exp.staff_mask) || (exp.needs_ward && !ward))
            continue;
        if (exp.kind == 0 || rival.objects[exp.kind] != 0)
            rival.cure_available[i] = 1;
    }
    if (!ward && rival.cure_available[kIllDiagWard] == 1)
        set_expertise_ward(rival);
}

void set_expertise_room(RivalHospital &rival, uint8_t type)
{
    bool ward = rival.cure_available[kIllDiagWard] == 1;
    for (size_t i = 1; i < 0x2f; i++) {
        const ExpertiseInfo &exp = kExpertiseData[i];
        if (rival.cure_available[i] != 0 || (exp.needs_ward && !ward) || exp.category != type)
            continue;
        uint16_t mask = exp.staff_mask;
        if (mask == 0) {
            if (type != kRoomResearch)
                rival.cure_available[i] = 1;
            continue;
        }
        int j = 0;
        for (; j < kSkillBits; j++) {
            if (((1 << j) & mask) == kSkillSurgeon && rival.surgeon_count < 2)
                break;
            if ((mask & (1 << j)) && rival.staff_by_skill[size_t(j)] == 0)
                break;
        }
        if (j >= kSkillBits)
            rival.cure_available[i] = 1;
    }
    if (!ward && rival.cure_available[kIllDiagWard] == 1)
        set_expertise_ward(rival);
}

bool computer_employ(World &world, uint8_t type, uint16_t skills, uint8_t owner)
{
    const RivalHospital &rival = world.rivals[owner];
    for (size_t id = 1; id < world.things.size(); id++) {
        const Thing &c = world.things[id];
        if (c.type != kThingStaff || c.state != kStaffApplicant || id == world.held_staff || c.subtype != type)
            continue;
        if (skills & ~c.skills & ((1 << kSkillBits) - 1))
            continue;
        int32_t pay = c.salary * 12 / 10;
        if (rival.balance < pay || rival.balance <= 0)
            return false;
        computer_employ_hire(world, uint16_t(id), owner, pay);
        return true;
    }
    return false;
}

bool computer_place_object(World &world, uint8_t obj, bool free, uint8_t owner)
{
    RivalHospital &rival = world.rivals[owner];
    // The game compares the balance as an unsigned number.
    bool affordable = uint32_t(rival.balance) >= uint32_t(rival.object_stats[obj].cost) && rival.balance > 0;
    if (!affordable && !free)
        return false;
    if (!free)
        rival.balance -= rival.object_stats[obj].cost;
    inc_hardware_stats(rival, obj);
    if (kObjects[obj].category == kCategoryReceptionDesk)
        rival.rooms[kRoomReception]++;
    rival.objects[obj]++;
    trace_event("computer_place_object %u obj=%u balance=%d", unsigned(owner), unsigned(obj), int(rival.balance));
    return true;
}

bool computer_build(World &world, const Balance &settings, uint8_t type, uint8_t owner)
{
    RivalHospital &rival = world.rivals[owner];
    int32_t cost = settings.rooms[type].cost;
    if (rival.balance < cost || rival.balance <= 0)
        return false;
    const RoomTypeInfo &info = kRoomTypeInfo[type];
    rival.balance -= cost;
    rival.rooms_built++;
    for (int i = 0; i < info.compulsory_count; i++)
        computer_place_object(world, info.objects[size_t(i)], false, owner);
    rival.rooms[type]++;
    set_expertise_room(rival, type);
    trace_event("computer_build %u room=%u balance=%d", unsigned(owner), unsigned(type), int(rival.balance));
    return true;
}

} // namespace th
