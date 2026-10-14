// Hiring: the job market's applicants, their appraisal and wage demand, and
// placing staff on the map (src/staff.cpp).

#include "core/world/staff.h"

#include "core/world/icons.h"
#include "core/world/nav.h"
#include "core/world/research.h"
#include "core/world/rooms.h"
#include "core/world/staff_complex.h"
#include "core/world/staff_internal.h"
#include "core/world/trace.h"
#include "core/world/training.h"

#include <algorithm>
#include <bit>

namespace th
{

namespace
{

// allocate_thing's reserve for new staff (place_staff_in_struct).
constexpr size_t kStaffReserve = 0x19;

// staff[kind].layer5_count (src/tables.cpp): the layer-5 variants an
// applicant picks from, 0 for none.
constexpr std::array<uint8_t, 5> kLayer5Count = {0, 2, 1, 0, 1};

// staff[kind].anim (src/tables.cpp): the staff_anim_starts entry shown
// placed or held, 5 (standing) or 0 (walking).
constexpr std::array<uint8_t, 5> kHeldAnim = {5, 5, 5, 0, 5};

// gamevar.announce's kinds a promotion is announced with (ANN_*), and the
// value of ANN_PROMOTED for a training room's skill.
constexpr uint8_t kAnnJuniorToDoctor = 0x0a;
constexpr uint8_t kAnnDoctorToConsultant = 0x0d;
} // namespace

uint16_t staffrand(uint32_t &seed)
{
    seed = std::rotr(seed * 0x24a1u + 0x24dfu, 13);
    return uint16_t(seed);
}

uint32_t normal_staffrand(uint32_t &seed, uint32_t low, uint32_t high)
{
    int32_t range = int32_t((high - low) / 2 + 1);
    if (range <= 0)
        range = 1;
    uint32_t sum = uint32_t(staffrand(seed) % range);
    sum += uint32_t(staffrand(seed) % range);
    return sum + low;
}

std::array<uint8_t, 3> reference_counts(const LanguageText &text)
{
    std::array<uint8_t, 3> counts{};
    const TextSection sections[3] = {TextSection::RefGeneral, TextSection::RefGood, TextSection::RefBad};
    for (size_t s = 0; s < 3; s++) {
        // The game's counter is a byte, and the list ends at a string starting
        // with '.'.
        uint8_t i = 0;
        while (i < text.size(sections[s]) && !text.at(sections[s], i).starts_with('.'))
            i++;
        counts[s] = i;
    }
    return counts;
}

int count_applicants(const World &world, uint8_t kind)
{
    int n = 0;
    for (const Thing &t : world.things)
        n += t.type == kThingStaff && t.subtype == kind && t.owner == kNoOwner &&
             (t.state == kStaffApplicant || t.state == kStaffHeld);
    return n;
}

namespace
{

// appraise_staff's junior promotion: past their service time, a junior
// becomes a doctor, announced for player 0.
void appraise_promote_junior(World &world, Thing &staff, const Balance::Gbv &gbv, uint32_t served)
{
    if (served < uint32_t(gbv.promo_doc) * 1500)
        return;
    uint16_t id = uint16_t(&staff - world.things.data());
    trace_event("appraise %u skill=%u", unsigned(id), unsigned(kSkillJunior));
    staff.skills &= uint16_t(~kSkillJunior);
    if (staff.owner == 0)
        announce_this(world, kAnnJuniorToDoctor, 1);
}

// appraise_staff's consultant promotion: past their service time, a doctor
// becomes a consultant, announced for player 0, and leaves a training room
// they are in.
void appraise_promote_consultant(World &world, Thing &staff, const Balance::Gbv &gbv, uint32_t served,
                                 const AnimData *anims, uint32_t game_turn)
{
    if (served < uint32_t(gbv.promo_con) * 1500)
        return;
    uint16_t id = uint16_t(&staff - world.things.data());
    trace_event("appraise %u skill=%u", unsigned(id), unsigned(kSkillConsultant));
    staff.skills |= kSkillConsultant;
    if (staff.owner == 0)
        announce_this(world, kAnnDoctorToConsultant, 1);
    if (staff.room == 0 || world.rooms[staff.room].type != kRoomTraining)
        return;
    staff.next_state = kStaffInCorridor;
    set_complex(world, id, kComplexLeaveRoom, world.rooms[staff.room].door_inside_tile);
    if (!anims)
        return;
    static const PersonComplexes no_person;
    static const StaffComplexes no_staff;
    process_staff_complex(world, *anims, world.person_complexes ? *world.person_complexes : no_person,
                          world.staff_complexes ? *world.staff_complexes : no_staff, id, game_turn);
}

// appraise_staff's promotions: a junior or doctor past the grade's threshold
// and their service time becomes a doctor or consultant. Draws nothing
// itself; the complex a promoted doctor's training room gives may.
void appraise_promote(World &world, Thing &staff, const Balance::Gbv &gbv, uint32_t game_turn, const AnimData *anims)
{
    if (!(staff.skills & kSkillDoctor))
        return;
    uint32_t served = game_turn - uint16_t(staff.hire_time);
    if ((staff.skills & kSkillJunior) && staff.ability > gbv.doctor_threshold)
        appraise_promote_junior(world, staff, gbv, served);
    else if (!(staff.skills & kSkillConsultant) && staff.ability > gbv.consultant_threshold)
        appraise_promote_consultant(world, staff, gbv, served, anims, game_turn);
}
} // namespace

void appraise_staff(World &world, Thing &staff, const Balance &settings, uint32_t game_turn, const AnimData *anims)
{
    const Balance::Gbv &gbv = settings.gbv;
    int32_t value = staff.ability / gbv.salary_ability_divisor + settings.staff[staff.subtype].min_salary;
    // The game reads SalaryAdd[3..8] (gbv.salary_add, include/hosp.h).
    const uint16_t bits[6] = {kSkillJunior,       kSkillDoctor,     kSkillSurgeon,
                              kSkillPsychiatrist, kSkillConsultant, kSkillResearcher};
    for (size_t i = 0; i < 6; i++)
        if (staff.skills & bits[i])
            value += gbv.salary_add[i + 3];
    staff.wage_demand = value;
    appraise_promote(world, staff, gbv, game_turn, anims);
    int happiness = int16_t(staff.happiness);
    happiness += staff.salary < value ? gbv.salary_too_low : gbv.salary_too_high;
    staff.happiness = uint16_t(std::clamp(int(int16_t(happiness)), 0, 100));
}

namespace
{

// place_staff_in_struct's slot: the first free thing short of the
// kStaffReserve last, 0 if none.
uint16_t place_staff_in_struct_free_thing(const World &world)
{
    for (size_t i = 1; i < world.things.size() - kStaffReserve; i++)
        if (world.things[i].type == kThingFree)
            return uint16_t(i);
    return 0;
}

// place_staff_in_struct's nurse: her skill and looks, drawn from `seed`.
void place_staff_in_struct_nurse(Thing &s, uint32_t &seed)
{
    s.skills = kSkillNurse;
    s.female = 1;
    s.face_head = 12;
    s.face_body = uint8_t(staffrand(seed) % 5 + 10);
    s.face_legs = uint8_t(staffrand(seed) % 5 + 10);
}

// place_staff_in_struct's doctor: their skills, each by the market's rate,
// the range of their ability by gbv's thresholds (a consultant's above a
// doctor's, a junior's below) into `ability_min` and `ability_max`, and
// their looks, all drawn from the market's seed.
void place_staff_in_struct_doctor(Thing &s, StaffMarket &market, const Balance::Gbv &gbv, uint32_t &ability_min,
                                  uint32_t &ability_max)
{
    uint32_t &seed = market.seed;
    ability_min = gbv.doctor_threshold;
    ability_max = gbv.consultant_threshold;
    s.skills |= kSkillDoctor;
    if (market.shrink_rate > 0 && staffrand(seed) % market.shrink_rate == 0)
        s.skills |= kSkillPsychiatrist;
    if (market.surgeon_rate > 0 && staffrand(seed) % market.surgeon_rate == 0)
        s.skills |= kSkillSurgeon;
    if (market.researcher_rate > 0 && staffrand(seed) % market.researcher_rate == 0)
        s.skills |= kSkillResearcher;
    if (market.consultant_rate > 0 && staffrand(seed) % market.consultant_rate == 0) {
        s.skills |= kSkillConsultant;
        ability_min = gbv.consultant_threshold;
        ability_max = 1000;
    } else if (market.junior_rate > 0 && staffrand(seed) % market.junior_rate == 0) {
        s.skills |= kSkillJunior;
        ability_min = 30;
        ability_max = gbv.doctor_threshold;
    }
    s.female = staffrand(seed) % 100 < 53;
    s.face_head = uint8_t(staffrand(seed) % 5);
    s.face_body = uint8_t(staffrand(seed) % 5);
    s.face_legs = uint8_t(staffrand(seed) % 5);
    if (s.layers[5] == 2) {
        s.face_legs += 5;
        s.face_body += 5;
        s.face_head += 5;
    }
}

// place_staff_in_struct's handyman: his skill, priorities and looks, drawn
// from `seed`.
void place_staff_in_struct_handyman(Thing &s, uint32_t &seed)
{
    s.skills = kSkillHandyman;
    s.female = 0;
    s.prio_litter = 0x21;
    s.prio_plants = 0x21;
    s.prio_repair = 0x22;
    s.face_head = uint8_t(staffrand(seed) % 3 + 15);
    s.face_body = uint8_t(staffrand(seed) % 3 + 15);
    s.face_legs = uint8_t(staffrand(seed) % 3 + 15);
}

// place_staff_in_struct's receptionist: her skill and looks, drawn from
// `seed`.
void place_staff_in_struct_receptionist(Thing &s, uint32_t &seed)
{
    s.skills = kSkillReceptionist;
    s.female = 1;
    s.face_head = uint8_t(staffrand(seed) % 4 + 10);
    s.face_body = uint8_t(staffrand(seed) % 5 + 10);
    s.face_legs = uint8_t(staffrand(seed) % 5 + 10);
    if (s.face_head == 12)
        s.face_head = 14;
}
} // namespace

uint16_t place_staff_in_struct(World &world, StaffMarket &market, const Balance &settings, uint8_t kind,
                               uint16_t extra_skills, uint32_t game_turn)
{
    uint16_t id = place_staff_in_struct_free_thing(world);
    if (!id)
        return 0;
    uint32_t &seed = market.seed;
    Thing &s = world.things[id];
    s.subtype = kind;
    s.type = kThingStaff;
    s.facing = 0;
    s.owner = kNoOwner;
    s.layers[5] = kLayer5Count[kind] ? uint8_t(staffrand(seed) % kLayer5Count[kind] + 1) : 0;
    for (size_t layer = 6; layer <= 9; layer++)
        s.layers[layer] = uint8_t(staffrand(seed) & 3);
    if (kind == kStaffNurse || kind == kStaffReceptionist)
        s.layers[5] = 1;
    set_applicant_expiry(s, game_turn + 2500 + staffrand(seed) % 2500);
    s.work_delay = uint8_t(normal_staffrand(seed, 1, 10));
    s.happiness = 100;
    s.warmth = 50;
    s.state = kStaffApplicant;
    market.applicant_count++;
    market.applicants[kind]++;

    uint32_t ability_min = 30, ability_max = 1000;
    switch (kind) {
    case kStaffNurse:
        place_staff_in_struct_nurse(s, seed);
        break;
    case kStaffDoctor:
        place_staff_in_struct_doctor(s, market, settings.gbv, ability_min, ability_max);
        break;
    case kStaffHandyman:
        place_staff_in_struct_handyman(s, seed);
        break;
    case kStaffReceptionist:
        place_staff_in_struct_receptionist(s, seed);
        break;
    }
    s.skills |= extra_skills;
    s.ability = uint16_t(normal_staffrand(seed, ability_min, ability_max));
    appraise_staff(world, s, settings, game_turn, nullptr);
    s.salary = s.wage_demand;
    s.initial = uint8_t(staffrand(seed) % 26 + 'A');
    s.surname_a = uint8_t(staffrand(seed) % 33);
    s.surname_b = uint8_t(staffrand(seed) % 26);
    // The game divides by the counts unchecked; a language without
    // references would crash it.
    s.ref_general = uint8_t(staffrand(seed) % std::max<uint8_t>(market.ref_counts[0], 1));
    s.ref_good = uint8_t(staffrand(seed) % std::max<uint8_t>(market.ref_counts[1], 1));
    s.ref_bad = uint8_t(staffrand(seed) % std::max<uint8_t>(market.ref_counts[2], 1));
    return id;
}

void create_job_market(World &world, StaffMarket &market, const Balance &settings, int game_month, uint32_t game_turn)
{
    for (const Balance::StaffLevel &level : settings.staff_levels) {
        if (level.month == game_month) {
            market.seed = level.seed;
            market.wanted = {level.nurses, level.doctors, level.handymen, level.receptionists};
            market.shrink_rate = level.shrk_rate;
            market.surgeon_rate = level.surg_rate;
            market.researcher_rate = level.rsch_rate;
            market.consultant_rate = level.cons_rate;
            market.junior_rate = level.jr_rate;
            break;
        }
        if (level.month > game_month)
            break;
    }
    for (uint8_t kind = 0; kind <= 3; kind++) {
        for (int j = market.applicants[kind]; j < market.wanted[kind]; j++)
            place_staff_in_struct(world, market, settings, kind, 0, game_turn);
    }
}

uint16_t staff_held_anim(uint8_t kind)
{
    const PersonAnims &a = kStaffAnims[kind];
    return kHeldAnim[kind] == 0 ? a.walk : a.stand;
}

bool staff_place_ok(const World &world, int cell, uint8_t kind)
{
    if (cell < 0 || cell >= kMapCells || world.map[size_t(cell)].owner_flags != 1 << kPlayer)
        return false;
    // check_points_connected to the room's inside door cell, or to the door
    // of the first entrance (allhosps[player].entrances[0]).
    int door = world.rooms[world.entrances[kPlayer][0]].door_tile;
    uint8_t row = 0;
    if (uint16_t r = room_at(world, uint16_t(cell))) {
        const RoomInfo &room = world.rooms[r];
        if (room.state != 0 && room.state != 1 && kind != kStaffHandyman)
            return false;
        row = kRoomStaffStateRow[room.type % kRoomTypes];
        door = room.door_inside_tile;
    }
    if (kStaffStates[row][kind % 5] == kStaffNone)
        return false;
    OrthonavPath path;
    orthonav_init(world, path, cell % kMapSize, cell / kMapSize, door % kMapSize, door / kMapSize);
    return path.count != 0;
}

namespace
{

// place_staff_on_map's new hire, hired at `game_turn`: no orders,
// discontent or resignations yet, their skills now count (staff_with_skill)
// and they join the hospital's staff count (inc_staff_stats; the kind and
// surgeon counts are derived from the things).
void place_staff_on_map_hire(World &world, uint16_t id, uint32_t game_turn)
{
    Thing &s = world.things[id];
    s.resign_count = 0;
    s.resign_time = 0;
    s.orders = 0;
    s.discontent = 0;
    clear_peep_icon(s);
    set_applicant_expiry(s, 0);
    s.hire_time = int16_t(game_turn);
    set_expertise_staff(world, id);
    world.staff_count++;
    if (world.staff_market) {
        world.staff_market->applicant_count--;
        world.staff_market->applicants[s.subtype]--;
    }
}

// place_staff_on_map's member of staff put down in `room`: listed there, in
// the state the room gives them. Where that is evacuating, and they are not
// a handyman, the advisor is told; otherwise they take the room's lock if
// they work in it.
void place_staff_on_map_room(World &world, const AnimData &anims, uint32_t game_turn, uint16_t id, uint16_t room)
{
    Thing &s = world.things[id];
    uint8_t type = world.rooms[room].type;
    insert_person_in_room_list(world, room, id);
    s.state = get_state(type, s);
    if (s.state == kStaffEvacuate) {
        if (s.subtype != kStaffHandyman) {
            world.new_staff.pending = true;
            world.new_staff.kind = s.subtype;
            world.new_staff.room = type;
        }
    } else if (s.subtype != kStaffHandyman &&
               (type != kRoomTraining || s.subtype != kStaffDoctor || (s.skills & kSkillConsultant)) &&
               type != kRoomResearch) {
        change_lockholder(ctx_of(world, anims, game_turn), id, room);
    }
}
} // namespace

bool place_staff_on_map(World &world, const StaffSetup &setup, int cell, uint16_t id, bool force)
{
    const AnimData &anims = setup.anims;
    const uint32_t game_turn = setup.game_turn;
    trace_event("place_staff %u tile=%d", unsigned(id), cell);
    if (id == 0 || id >= world.things.size() || cell < 0 || cell >= kMapCells)
        return false;

    if (!force && world.map[size_t(cell)].owner_flags != 1 << kPlayer)
        return false;
    Thing &s = world.things[id];
    s.owner = kPlayer;
    if (s.facing > 3)
        s.facing = 0;
    play_frame(s, anims, staff_held_anim(s.subtype));
    s.x = {0x80, uint8_t(cell % kMapSize)};
    s.y = {0x80, uint8_t(cell / kMapSize)};
    s.path.x1 = 0xff;
    if (uint16_t room = room_at(world, uint16_t(cell))) {
        place_staff_on_map_room(world, anims, game_turn, id, room);
    } else {
        s.state = get_state(0, s);
    }
    s.substate = 0;
    s.complex_action = 0;
    calculate_speed(world, s, setup.settings);
    insert_node(world, id, s.x, s.y);
    // With no one held, a new hire.
    if (world.held_staff == 0)
        place_staff_on_map_hire(world, id, game_turn);
    if (s.subtype == kStaffNurse)
        reinit_nurse(world, id);
    else if (s.subtype == kStaffHandyman)
        s.work_target = s.work_target2 = 0;
    pager_arrival(world, id);
    if (s.room)
        world.rooms[s.room].staff_free = 1;
    world.held_staff = 0;
    world.held_staff_tile = 0;
    return true;
}

uint32_t applicant_expiry(const Thing &t)
{
    // Thing+0x96-0x97 is work_target2 for a handyman and crazy with
    // surgeon_training for a doctor (include/hosp.h); neither kind uses the
    // other's fields, so the union's bytes are whichever applies.
    uint32_t high = t.type == kThingStaff && t.subtype == kStaffHandyman
                        ? t.work_target2
                        : uint32_t(t.crazy) | (uint32_t(t.surgeon_training) << 8);
    return uint32_t(t.work_target) | (high << 16);
}

void set_applicant_expiry(Thing &t, uint32_t turn)
{
    t.work_target = uint16_t(turn);
    if (t.type == kThingStaff && t.subtype == kStaffHandyman) {
        t.work_target2 = uint16_t(turn >> 16);
    } else {
        t.crazy = uint8_t(turn >> 16);
        t.surgeon_training = uint8_t(turn >> 24);
    }
}

// remove_staff_in_struct (0x2a130): deletes an applicant, taking them off the
// job market's counts.
void remove_staff_in_struct(World &world, Thing &staff)
{
    if (world.staff_market) {
        world.staff_market->applicants[staff.subtype]--;
        world.staff_market->applicant_count--;
    }
    staff = Thing{};
}

} // namespace th
