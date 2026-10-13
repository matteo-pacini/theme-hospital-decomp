// Rooms in use: what a room needs, finding objects and doors, a room's value and
// the call and patient events. The rest is in rooms_*.cpp, which share
// rooms_internal.h.

#include "core/world/rooms.h"
#include "core/world/rooms_internal.h"

#include "core/ui/charts.h"
#include "core/world/build.h"
#include "core/world/complex.h"
#include "core/world/disaster.h"
#include "core/world/epidemic.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/mayor.h"
#include "core/world/nav.h"
#include "core/world/queue.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

#include <algorithm>
#include <cstdlib>

namespace th
{

namespace
{

// Room types (the game's ROOM_*).
constexpr uint8_t kRoomWard = 9;

// The game's special_around (src/tables.cpp), first five: a cell and its
// four neighbours, as cell offsets.
constexpr std::array<int16_t, 5> kSpecialAround = {0, 1, 128, -1, -128};

} // namespace

inline namespace rooms_detail
{

// allhosps[owner].rooms[illness].turned_away++, kept for hospital 0.
void turned_away(World &world, const Thing &patient)
{
    if (world.hospital && patient.owner == 0)
        world.hospital->illnesses[patient.illness % kIllnessCount].turned_away++;
}

// allhosps.surgeon_count (inc_staff_stats): the hospital's hired surgeons.
int surgeon_count(const World &world, uint8_t owner)
{
    int n = 0;
    for (const Thing &t : world.things)
        n += t.type == kThingStaff && t.owner == owner && (t.skills & kSkillSurgeon);
    return n;
}
} // namespace rooms_detail

void start_rooms_in_use(Game &game, const PersonComplexes &person_complexes, const StaffComplexes &staff_complexes)
{
    World &world = game.world;
    world.pagers = {};
    world.room_of_type = {};
    world.calls = {};
    world.next_call_id = 1;
    world.repair_pager = {};
    world.plant_pager = {};
    world.hospital = &game.hospital;
    world.ledger = &game.finance;
    world.balance = &game.balance;
    world.date = &game.date;
    world.person_complexes = &person_complexes;
    world.staff_complexes = &staff_complexes;
}

int staff_with_skill(const World &world, uint8_t owner, int skill)
{
    int n = 0;
    for (const Thing &t : world.things)
        n += t.type == kThingStaff && t.owner == owner && (t.skills & (1 << skill));
    return n;
}

Required what_is_required(const World &world, uint8_t illness)
{
    constexpr uint8_t kNurseSkill = 2;
    constexpr uint8_t kSurgeonSkill = 5;
    Required r;
    if (world.cure_available[illness] == 1) {
        r.treatable = true;
        return r;
    }
    const ExpertiseInfo &exp = kExpertiseData[illness];
    if (world.room_of_type[exp.category] == 0) {
        r.missing_room = exp.category;
        r.room_unavailable = !room_available(world, exp.category);
    }
    if (exp.staff_mask & kSkillSurgeon) {
        const int surgeons = surgeon_count(world, 0);
        if (staff_with_skill(world, 0, kSurgeonSkill) == 0) {
            r.staff_mask = kSkillSurgeon;
            r.missing_skill = kSurgeonSkill;
            r.surgeons_needed = 2;
        } else if (surgeons < 2) {
            r.surgeons_needed = uint8_t(2 - surgeons);
            r.staff_mask = kSkillSurgeon;
            r.missing_skill = kSurgeonSkill;
        }
    } else {
        for (uint8_t i = 0; i < 9; i++) {
            if ((exp.staff_mask & (1 << i)) && staff_with_skill(world, 0, i) == 0) {
                r.staff_mask = exp.staff_mask;
                r.missing_skill = i;
                break;
            }
        }
    }
    if (exp.needs_ward && world.cure_available[kIllDiagWard] == 0) {
        r.needs_ward = world.room_of_type[kRoomWard] == 0;
        r.needs_nurse = staff_with_skill(world, 0, kNurseSkill) == 0;
    }
    return r;
}

namespace
{

// find_object's candidates so far.
struct FoundObjects {
    std::array<uint16_t, 30> ids{};
    uint8_t count = 0;
};

// find_object in a room: the room's objects of `kind`. Returns the one to
// take at once (0 to pick among `found`).
uint16_t find_object_in_room(const World &world, uint16_t room, uint8_t kind, bool random, bool free_only,
                             FoundObjects &found)
{
    for (uint16_t o = world.rooms[room].first_object; o; o = world.things[o].room_obj_next) {
        const Thing &obj = world.things[o];
        if (obj.subtype != kind || (free_only && obj.in_use != 0))
            continue;
        if (!random || found.count >= found.ids.size())
            return o;
        found.ids[found.count++] = o;
    }
    return 0;
}

// find_object in a corridor: the corridor objects of `kind` within 7 tiles of
// `tile` that can be walked to. Returns the one to take at once (0 to pick
// among `found`).
uint16_t find_object_nearby(World &world, uint16_t tile, uint8_t kind, bool random, bool free_only, FoundObjects &found)
{
    int x = tile % kMapSize;
    int y = tile / kMapSize;
    for (uint16_t o = world.first_of_kind[kind]; o; o = world.things[o].type_obj_next) {
        const Thing &obj = world.things[o];
        if (free_only && obj.in_use != 0)
            continue;
        if (std::abs(x - obj.x.tile) > 7 || std::abs(y - obj.y.tile) > 7)
            continue;
        OrthonavPath path;
        orthonav_init(world, path, obj.use_pos[1] % kMapSize, obj.use_pos[1] / kMapSize, x, y);
        if (path.count == 0 || obj.room_id)
            continue;
        if (!random || found.count >= found.ids.size())
            return o;
        found.ids[found.count++] = o;
    }
    return 0;
}

} // namespace

uint16_t find_object(World &world, uint16_t tile, uint8_t kind, bool random, bool free_only, uint8_t owner)
{
    FoundObjects found;
    uint16_t at_once = 0;
    if (uint16_t room = room_at(world, tile))
        at_once = find_object_in_room(world, room, kind, random, free_only, found);
    else if (owner == 0)
        at_once = find_object_nearby(world, tile, kind, random, free_only, found);
    if (at_once)
        return at_once;
    return found.count ? found.ids[old_newrand(world.seed) % found.count] : 0;
}

uint16_t is_this_a_door(const World &world, uint16_t tile)
{
    for (uint16_t r = 1; r < kRoomCount; r++)
        if (tile == world.rooms[r].door_tile || tile == world.rooms[r].door_inside_tile)
            return r;
    return 0;
}

uint16_t find_door(const World &world, uint16_t id)
{
    const Thing &person = world.things[id];
    uint16_t tile = cell_of(person);
    for (int16_t offset : kSpecialAround) {
        int cell = tile + offset;
        if (cell < 0 || cell >= kMapCells)
            continue;
        for (int16_t i = world.map[size_t(cell)].first; i > 0; i = world.things[size_t(i)].cell_next) {
            const Thing &door = world.things[size_t(i)];
            if (door.type != kThingDoor)
                continue;
            if (person.room || tile == world.rooms[door.room_id].door_tile)
                return uint16_t(i);
            break;
        }
    }
    return 0;
}

int32_t calc_room_value(const World &world, uint16_t room_id)
{
    if (room_id == 0 || room_id >= kRoomCount)
        return 0;
    const RoomInfo &room = world.rooms[room_id];
    int32_t base = (room.width + room.height) * 5;
    int32_t count = 0;
    for (uint16_t o = room.first_object; o; o = world.things[o].room_obj_next)
        count++;
    count -= kRoomTypeInfo[room.type % kRoomTypes].compulsory_count;
    count += room.windows / 2;
    int32_t score = base + count * 5;
    return score < 0 ? 0 : score;
}

namespace
{

// EV_CHOICE_RESULT kChoiceTreatResearch and EV_PATIENT_TO_RESEARCH: the
// patient goes for an autopsy.
void send_for_autopsy(Thing &p)
{
    p.treatment = kIllAutopsy;
    p.person_flags &= ~kPfAwaitingDecision;
    reset_icon_value(p, kIconAwaitingDecision);
    p.state = kPsGoForTreatment;
}

// EV_PATIENT_WAITS_FOR_ROOM and the choice screens' wait answers: `id` stops
// awaiting a decision and waits for a room (the game does not check that
// the thing is a patient).
void patient_waits_for_room(Game &game, uint16_t id)
{
    if (id >= game.world.things.size())
        return;
    Thing &p = game.world.things[id];
    p.person_flags &= ~kPfAwaitingDecision;
    reset_icon_value(p, kIconAwaitingDecision);
    p.person_flags |= kPfWaitingForRoom;
    p.timer = int16_t(game.turn_count);
}

// apply_call_event's EV_CHOICE_RESULT for patient `id` awaiting a decision.
void apply_call_event_decision(Game &game, uint16_t id, uint16_t choice)
{
    World &world = game.world;
    Thing &p = world.things[id];
    const PersonComplexes &complexes = person_complexes_of(world);
    switch (choice) {
    case kChoiceTreatSendHome:
    case kChoiceDiagSendHome:
        p.treatment = 0;
        turned_away(world, p);
        person_go_home(world, game.anims(), complexes, id, game.turn_count);
        p.person_flags |= choice == kChoiceTreatSendHome ? kPfNoTreatment : kPfNoDiagnosis;
        break;
    case kChoiceTreatWait:
    case kChoiceDiagWait:
        patient_waits_for_room(game, id);
        break;
    case kChoiceTreatResearch:
        send_for_autopsy(p);
        break;
    case kChoiceDiagGuess:
        p.person_flags &= ~kPfAwaitingDecision;
        reset_icon_value(p, kIconAwaitingDecision);
        interrupt_complex(world, game.anims(), complexes, id, game.turn_count);
        best_guess_treatment(world, p);
        next_treatment_now_setup(world, game.anims(), id, false, game.turn_count);
        break;
    default:
        break;
    }
}

// apply_call_event's EV_ANSWER_PLAYERCALL.
bool apply_call_event_answer(Game &game, const Event &event)
{
    uint16_t subject = 0;
    switch (event.value) {
    case kCallNoTreatment:
        subject = get_playercall(game.world, kCallNoTreatment, event.arg, 0);
        if (subject)
            game.choice_screen = {kFaxNoTreatment, subject, 0};
        return true;
    case kCallNewIllness:
        subject = get_playercall(game.world, kCallNewIllness, 0, 0);
        game.choice_screen = {kFaxNewIllness, subject, 0};
        return true;
    case kCallNoDiagnosis:
        subject = get_playercall(game.world, kCallNoDiagnosis, event.arg, 0);
        if (subject)
            game.choice_screen = {kFaxNoDiagnosis, subject, 0};
        return true;
    default:
        return false;
    }
}

} // namespace

bool apply_call_event(Game &game, const Event &event)
{
    if (event.type == EV_ANSWER_PLAYERCALL)
        return apply_call_event_answer(game, event);
    if (event.type != EV_CHOICE_RESULT)
        return false;
    constexpr std::array<uint16_t, 6> kChoices = {kChoiceTreatSendHome, kChoiceTreatWait, kChoiceTreatResearch,
                                                  kChoiceDiagSendHome,  kChoiceDiagGuess, kChoiceDiagWait};
    if (std::find(kChoices.begin(), kChoices.end(), event.value) == kChoices.end())
        return false;
    if (event.arg >= game.world.things.size())
        return true;
    Thing &p = game.world.things[event.arg];
    if (p.type == kThingPatient && (p.person_flags & kPfAwaitingDecision))
        apply_call_event_decision(game, event.arg, event.value);
    if (p.type == kThingPatient)
        p.playercall = 0;
    return true;
}

bool apply_patient_event(Game &game, const Event &event)
{
    World &world = game.world;
    switch (event.type) {
    case EV_QUEUE_CAPACITY_UP:
        if (event.value < world.rooms.size() && world.rooms[event.value].queue_capacity < 0x1e)
            world.rooms[event.value].queue_capacity++;
        return true;
    case EV_QUEUE_CAPACITY_DOWN:
        if (event.value < world.rooms.size() && world.rooms[event.value].queue_capacity)
            world.rooms[event.value].queue_capacity--;
        return true;
    case EV_FRONT_QUEUE:
        front_queue(world, event.value, event.arg);
        return true;
    case EV_BACK_QUEUE:
        back_queue(world, event.value, event.arg);
        return true;
    case EV_QUEUE_PERSON_GO_HOME:
        queue_person_go_home(world, game.anims(), person_complexes_of(world), event.value, event.arg, game.turn_count);
        return true;
    case EV_QUEUE_PERSON_GO_RECEPTION:
        queue_person_go_reception(world, game.anims(), person_complexes_of(world), event.value, event.arg,
                                  game.turn_count);
        return true;
    case EV_PATIENT_TO_RESEARCH:
        if (event.value < world.things.size())
            send_for_autopsy(world.things[event.value]);
        return true;
    case EV_PATIENT_WAITS_FOR_ROOM:
        patient_waits_for_room(game, event.value);
        return true;
    case EV_QUEUE_PERSON_TO_NEW_QUEUE:
        queue_person_to_new_queue(world, game.anims(), person_complexes_of(world), event.value, event.arg,
                                  game.turn_count);
        return true;
    default:
        break;
    }
    if (event.type != EV_PERSON_GO_HOME && event.type != EV_FORCE_CURE)
        return false;
    if (event.value >= world.things.size())
        return true;
    if (event.type == EV_FORCE_CURE) {
        force_peep_to_be_cured(world, game.anims(), event.value, game.turn_count);
        return true;
    }
    Thing &p = world.things[event.value];
    p.treatment = 0;
    turned_away(world, p);
    person_go_home(world, game.anims(), person_complexes_of(world), event.value, game.turn_count);
    p.person_flags |= kPfSentHome;
    if (p.person_flags & kPfContagious)
        world.epidemic.timer = 0;
    return true;
}

} // namespace th
