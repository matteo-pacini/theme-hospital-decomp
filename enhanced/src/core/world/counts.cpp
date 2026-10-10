#include "core/world/counts.h"

#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/object_tables.h"
#include "core/world/queue.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/year_end.h"

namespace th
{

namespace
{

// RoomInfo.type of a training room (ROOM_TRAINING).
constexpr uint8_t kRoomTraining = 22;

bool counting(uint32_t game_turn)
{
    return (game_turn & 0x1f) == 0;
}

bool outdoors(const World &world, const Thing &t)
{
    return world.map[size_t(t.x.tile + (t.y.tile << 7))].owner_flags & kCellOutdoor;
}

// person_count's figures of one of hospital 0's patients indoors.
void person_count_figures(AdCounts &c, const Thing &p)
{
    c.patients++;
    c.impatience_sum = uint16_t(c.impatience_sum + p.impatience);
    c.waiting_sum += p.waiting_time;
    for (size_t k = 0; k < c.wait_sums.size(); k++) {
        c.wait_sums[k] += p.waiting_time;
        c.wait_counts[k]++;
    }
    c.happiness_sum = int16_t(c.happiness_sum + p.happiness);
    if (int16_t(p.happiness) > c.max_happiness)
        c.max_happiness = int16_t(p.happiness);
    if (int16_t(p.happiness) < c.min_happiness)
        c.min_happiness = int16_t(p.happiness);
    c.refreshment_sum = int16_t(c.refreshment_sum + p.refreshment);
    if (p.refreshment < c.min_refreshment)
        c.min_refreshment = p.refreshment;
    if (int16_t(p.warmth) > c.max_warmth)
        c.max_warmth = int16_t(p.warmth);
    if (int16_t(p.warmth) < c.min_warmth)
        c.min_warmth = int16_t(p.warmth);
    c.warmth_sum = int16_t(c.warmth_sum + p.warmth);
    c.bladder_sum = int16_t(c.bladder_sum + p.bladder);
    if (p.bladder > c.max_bladder)
        c.max_bladder = p.bladder;
    if (p.illness < c.illness_counts.size())
        c.illness_counts[p.illness]++;
}

// staff_count's figures of a member of staff by kind.
void staff_count_kind(AdCounts &c, const Thing &s)
{
    auto add = [&](uint8_t kind) {
        c.staff_tiredness_by_kind[kind] = int16_t(c.staff_tiredness_by_kind[kind] + s.tiredness);
        c.staff_happiness_by_kind[kind] = int16_t(c.staff_happiness_by_kind[kind] + s.happiness);
    };
    switch (s.subtype) {
    case kStaffSurgeon:
    case kStaffDoctor:
        if (s.subtype == kStaffSurgeon)
            c.surgeons++;
        c.doctors_and_nurses++;
        add(kStaffDoctor);
        c.consultants += (s.skills & kSkillConsultant) != 0;
        c.researchers += (s.skills & kSkillResearcher) != 0;
        c.psychiatrists += (s.skills & kSkillPsychiatrist) != 0;
        break;
    case kStaffNurse:
        c.doctors_and_nurses++;
        add(kStaffNurse);
        break;
    case kStaffHandyman:
    case kStaffReceptionist:
        add(s.subtype);
        break;
    default:
        break;
    }
}

} // namespace

void reset_all_count(World &world, uint32_t game_turn)
{
    if (!counting(game_turn))
        return;
    // Both live outside the game's adcounts, which its memset clears:
    // advisor.has_patients and NRAdcounts.last_quake_size.
    bool has_patients = world.counts.has_patients;
    uint8_t last_quake_size = world.counts.last_quake_size;
    world.counts = {};
    world.counts.has_patients = has_patients;
    world.counts.last_quake_size = last_quake_size;
    world.emergency.present = 0;
    world.epidemic.contagious_count = 0;
}

void person_count(World &world, uint16_t id, uint32_t game_turn)
{
    if (!counting(game_turn))
        return;
    Thing &p = world.things[id];
    world.counts.has_patients = true;
    if (outdoors(world, p)) {
        if ((p.person_flags & kPfContagious) && p.owner == 0)
            world.epidemic.timer = 0;
        return;
    }
    if (p.owner == 0)
        person_count_figures(world.counts, p);
    if ((p.person_flags & kPfEmergency) && p.owner == 0)
        world.emergency.present++;
    if (int8_t(p.contagion) != 0 && p.owner == 0)
        world.epidemic.contagious_count++;
    if (!(p.person_flags & kPfCountedVisitor)) {
        // Only hospital 0's history is kept; the game adds every
        // hospital's patients to it.
        if (world.ledger)
            world.ledger->history[kHistoryVisitors][11]++;
        p.person_flags |= kPfCountedVisitor;
    }
}

void staff_count(World &world, uint16_t id, uint32_t game_turn)
{
    if (!counting(game_turn))
        return;
    const Thing &s = world.things[id];
    if (s.state == kStaffApplicant || s.owner != 0)
        return;
    AdCounts &c = world.counts;
    staff_count_kind(c, s);
    c.staff_happiness_sum = uint16_t(c.staff_happiness_sum + s.happiness);
    c.staff_tiredness_sum = int16_t(c.staff_tiredness_sum + s.tiredness);
    if (s.tiredness > c.max_staff_tiredness)
        c.max_staff_tiredness = s.tiredness;
    const RoomInfo *room = s.room ? &world.rooms[s.room] : nullptr;
    if (s.state == kStaffWanders ||
        (room && room->type != kRoomTraining && room->queue_length == 0 && room->queue_incoming == 0))
        c.idle_staff[s.subtype % c.idle_staff.size()]++;
    c.staff_warmth_sum = int16_t(c.staff_warmth_sum + s.warmth);
}

void object_count(World &world, uint16_t id, uint32_t game_turn)
{
    if (!counting(game_turn))
        return;
    const Thing &obj = world.things[id];
    if (obj.subtype >= kObjectKinds || !kObjectIsMachine[obj.subtype] || obj.strength == 0)
        return;
    AdCounts &c = world.counts;
    int32_t wear = (obj.times_used << 9) / (obj.strength * 3);
    if (obj.owner == 0) {
        c.machines++;
        c.machine_wear_sum += wear;
    }
    if (obj.times_used > obj.strength && wear > c.max_machine_wear && obj.owner == 0) {
        c.max_machine_wear = wear;
        c.worst_machine = obj.subtype;
    }
}

void process_things_alone_begin(World &world, uint16_t id, uint32_t game_turn)
{
    reset_all_count(world, game_turn);
    // process_things re-entered for one thing checks the win status too.
    if (world.game)
        check_win_status(*world.game);
    if (world.things[id].type == kThingPatient)
        person_count(world, id, game_turn);
    else if (world.things[id].type == kThingStaff)
        staff_count(world, id, game_turn);
}

void final_count(const World &world, uint32_t game_turn)
{
    if (!counting(game_turn))
        return;
    const AdCounts &c = world.counts;
    trace_event("counts patients=%u impatience=%u waiting=%ld happy=%d refresh=%d warm=%d bladder=%d staff=%u "
                "tired=%d,%d,%d,%d,%d happy=%u warm=%d idle=%u,%u,%u,%u,%u machines=%u wear=%ld,%ld worst=%u "
                "crackups=%u visitors=%ld",
                unsigned(c.patients), unsigned(c.impatience_sum), long(c.waiting_sum), int(c.happiness_sum),
                int(c.refreshment_sum), int(c.warmth_sum), int(c.bladder_sum), unsigned(c.doctors_and_nurses),
                int(c.staff_tiredness_sum), int(c.staff_tiredness_by_kind[0]), int(c.staff_tiredness_by_kind[1]),
                int(c.staff_tiredness_by_kind[2]), int(c.staff_tiredness_by_kind[3]), unsigned(c.staff_happiness_sum),
                int(c.staff_warmth_sum), unsigned(c.idle_staff[0]), unsigned(c.idle_staff[1]),
                unsigned(c.idle_staff[2]), unsigned(c.idle_staff[3]), unsigned(c.idle_staff[4]), unsigned(c.machines),
                long(c.machine_wear_sum), long(c.max_machine_wear), unsigned(c.worst_machine), unsigned(c.crackups),
                world.ledger ? long(world.ledger->history[kHistoryVisitors][11]) : 0L);
}

} // namespace th
