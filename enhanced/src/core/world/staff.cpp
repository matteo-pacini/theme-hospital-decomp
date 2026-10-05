#include "core/world/staff.h"

#include "core/world/events.h"
#include "core/world/game.h"
#include "core/world/nav.h"

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

// staff_states[rooms[0].staff_state_row] (src/tables.cpp, row 0): the state
// of staff placed in the corridor, by kind.
constexpr std::array<uint8_t, 5> kCorridorStates = {kStaffInCorridor, kStaffInCorridor, kStaffHandymanWorks,
                                                    kStaffReceptionistFindsReception, kStaffInCorridor};

// The player whose hospital the engine plays.
constexpr uint8_t kPlayer = 0;

// calculate_speed (0x2c268), for staff at a cell's middle outside rooms.
void calculate_speed(Thing &staff, const Balance &settings)
{
    if (staff.x.frac != 0x80 || staff.y.frac != 0x80)
        return;
    const Balance::Gbv &gbv = settings.gbv;
    uint32_t speed = 3;
    if (staff.tiredness > int(gbv.crack_up_tired))
        speed = 1;
    else if (staff.tiredness > int(gbv.very_tired))
        speed = 2;
    if (staff.ability >= gbv.consultant_threshold)
        speed += 30;
    else if (staff.ability > gbv.doctor_threshold)
        speed += 20;
    else
        speed += 10;
    switch (speed) {
    case 11:
    case 12:
    case 13:
    case 21:
    case 22:
    case 31:
        staff.speed = 0x10;
        staff.anim_speed = 2;
        break;
    case 23:
    case 32:
        staff.speed = 0x20;
        staff.anim_speed = 1;
        break;
    case 33:
        staff.speed = 0x40;
        staff.anim_speed = 1;
        break;
    }
}

// staff_in_corridor (0x2abb0): wanders from where they stand.
void staff_in_corridor(Thing &staff, const AnimData &anims)
{
    staff.dest_x = staff.x.tile;
    staff.dest_y = staff.y.tile;
    staff.state = kStaffWanders;
    staff.substate = 6;
    staff.countdown = 1;
    play_frame(staff, anims, kStaffAnims[staff.subtype].stand);
}

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

void appraise_staff(Thing &staff, const Balance &settings)
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
    int happiness = int16_t(staff.happiness);
    happiness += staff.salary < value ? gbv.salary_too_low : gbv.salary_too_high;
    staff.happiness = uint16_t(std::clamp(int(int16_t(happiness)), 0, 100));
}

uint16_t place_staff_in_struct(World &world, StaffMarket &market, const Balance &settings, uint8_t kind,
                               uint16_t extra_skills, uint32_t game_turn)
{
    uint16_t id = 0;
    for (size_t i = 1; i < world.things.size() - kStaffReserve; i++) {
        if (world.things[i].type == kThingFree) {
            id = uint16_t(i);
            break;
        }
    }
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
    s.hire_expiry = game_turn + 2500 + staffrand(seed) % 2500;
    s.work_delay = uint8_t(normal_staffrand(seed, 1, 10));
    s.happiness = 100;
    s.warmth = 50;
    s.state = kStaffApplicant;

    const Balance::Gbv &gbv = settings.gbv;
    uint32_t ability_min = 30, ability_max = 1000;
    switch (kind) {
    case kStaffNurse:
        s.skills = kSkillNurse;
        s.female = 1;
        s.face_head = 12;
        s.face_body = uint8_t(staffrand(seed) % 5 + 10);
        s.face_legs = uint8_t(staffrand(seed) % 5 + 10);
        break;
    case kStaffDoctor:
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
        break;
    case kStaffHandyman:
        s.skills = kSkillHandyman;
        s.female = 0;
        s.prio_litter = 0x21;
        s.prio_plants = 0x21;
        s.prio_repair = 0x22;
        s.face_head = uint8_t(staffrand(seed) % 3 + 15);
        s.face_body = uint8_t(staffrand(seed) % 3 + 15);
        s.face_legs = uint8_t(staffrand(seed) % 3 + 15);
        break;
    case kStaffReceptionist:
        s.skills = kSkillReceptionist;
        s.female = 1;
        s.face_head = uint8_t(staffrand(seed) % 4 + 10);
        s.face_body = uint8_t(staffrand(seed) % 5 + 10);
        s.face_legs = uint8_t(staffrand(seed) % 5 + 10);
        if (s.face_head == 12)
            s.face_head = 14;
        break;
    }
    s.skills |= extra_skills;
    s.ability = uint16_t(normal_staffrand(seed, ability_min, ability_max));
    appraise_staff(s, settings);
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
        int have = count_applicants(world, kind);
        for (int j = have; j < market.wanted[kind]; j++)
            place_staff_in_struct(world, market, settings, kind, 0, game_turn);
    }
}

uint16_t staff_held_anim(uint8_t kind)
{
    const PersonAnims &a = kStaffAnims[kind];
    return kHeldAnim[kind] == 0 ? a.walk : a.stand;
}

bool staff_place_ok(const World &world, int cell)
{
    if (cell < 0 || cell >= kMapCells || world.map[size_t(cell)].owner_flags != 1 << kPlayer)
        return false;
    // check_points_connected to the door of the first entrance
    // (allhosps[player].entrances[0]).
    int door = world.entrances[kPlayer][0];
    OrthonavPath path;
    orthonav_init(world, path, cell % kMapSize, cell / kMapSize, door % kMapSize, door / kMapSize);
    return path.count != 0;
}

bool place_staff_on_map(World &world, const AnimData &anims, const Balance &settings, int cell, uint16_t id, bool force,
                        uint32_t game_turn)
{
    if (id == 0 || id >= world.things.size() || cell < 0 || cell >= kMapCells)
        return false;
    if (!force && world.map[size_t(cell)].owner_flags != 1 << kPlayer)
        return false;
    Thing &s = world.things[id];
    bool new_hire = s.owner == kNoOwner;
    s.owner = kPlayer;
    if (s.facing > 3)
        s.facing = 0;
    play_frame(s, anims, staff_held_anim(s.subtype));
    s.x = {0x80, uint8_t(cell % kMapSize)};
    s.y = {0x80, uint8_t(cell / kMapSize)};
    s.path.x1 = 0xff;
    // No rooms exist yet: which_room finds none, so the corridor's state.
    s.state = kCorridorStates[s.subtype];
    s.substate = 0;
    s.complex_action = 0;
    calculate_speed(s, settings);
    insert_node(world, id, s.x, s.y);
    // The game tests that no picked-up member of staff is held
    // (allhosps[owner].held_staff); picking staff up is not ported, so this
    // is a new hire. The hospital's staff counts and skills, nurses' and
    // handymen's work and the pager are not ported.
    if (new_hire) {
        s.icon_id = 0;
        s.hire_expiry = 0;
        s.hire_time = int16_t(game_turn);
    }
    return true;
}

void staff_turn(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &s = world.things[id];
    switch (s.state) {
    case kStaffApplicant:
        // remove_staff_in_struct.
        if (game_turn > s.hire_expiry)
            s = Thing{};
        break;
    case kStaffInCorridor:
        staff_in_corridor(s, anims);
        break;
    case kStaffReceptionistFindsReception:
        // receptionist_finds_reception with no reception room.
        staff_in_corridor(s, anims);
        break;
    default:
        break;
    }
}

bool apply_staff_event(Game &game, const Event &event)
{
    World &world = game.world;
    switch (event.type) {
    case EV_MARK_EMPLOYING:
        // req2_mark_employing; the advisor's staff happiness total is not
        // ported.
        if (event.arg && event.arg < world.things.size())
            world.things[event.arg].state = kStaffHeld;
        return true;
    case EV_PLACE_STAFF:
        if (event.arg && event.arg < world.things.size()) {
            Thing &s = world.things[event.arg];
            if (place_staff_on_map(world, game.anims(), game.settings(), event.value, event.arg, false,
                                   game.turn_count))
                // change_balance(TX_HIRE_STAFF), as the game computes it.
                game.balance += -(s.salary * 12) / 10;
        }
        return true;
    case EV_CANCEL_BUILD:
        // req2_exit_employing.
        if (event.value && event.value < world.things.size())
            world.things[event.value].state = kStaffApplicant;
        return false;
    default:
        return false;
    }
}

} // namespace th
