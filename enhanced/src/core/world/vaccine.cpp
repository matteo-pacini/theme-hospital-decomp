#include "core/world/vaccine.h"

#include "core/world/complex.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/nav.h"
#include "core/world/queue.h"
#include "core/world/staff.h"
#include "core/world/staff_complex.h"

#include <array>
#include <cstdlib>

namespace th
{

namespace
{

// The game's around (src/tables.cpp): the cells beside a cell.
constexpr std::array<int, 4> kAround = {-kMapSize, 1, kMapSize, -1};

// Complex actions the original vaccine.cpp names by number.
constexpr uint16_t kComplexLeaveRoom = 0x136;
constexpr uint16_t kComplexWalkTo = 0xbe;

uint16_t cell_of(const Thing &t)
{
    return uint16_t(t.x.tile + (t.y.tile << 7));
}

// check_points_connected is core/world/nav.h's points_connected.

// can_i_go_near_point (0x1ab90): of the four cells beside `target`, the
// nearest to the person that a path reaches; 0 if none.
uint16_t can_i_go_near_point(const World &world, const Thing &person, uint16_t target)
{
    uint16_t best = 0xffff;
    uint16_t result = 0;
    uint16_t pos = cell_of(person);
    for (int offset : kAround) {
        uint16_t cand = uint16_t(target + offset);
        uint16_t dist =
            uint16_t(std::abs(pos % kMapSize - cand % kMapSize) + std::abs(pos / kMapSize - cand / kMapSize));
        if (dist >= best)
            continue;
        OrthonavPath path;
        orthonav_init(world, path, person.x.tile, person.y.tile, cand % kMapSize, cand / kMapSize);
        if (path.count != 0) {
            best = dist;
            result = cand;
        }
    }
    return result;
}

// am_i_near_point (0x1ad58): whether the person stands on a cell beside
// `pos`.
bool am_i_near_point(const Thing &person, uint16_t pos)
{
    uint16_t cell = cell_of(person);
    for (int offset : kAround)
        if (cell == uint16_t(pos + offset))
            return true;
    return false;
}

// change_balance for hospital `owner` (src/finance.cpp), at the current
// date.
void pay(World &world, uint8_t type, int32_t amount)
{
    if (!world.ledger || !world.balance || !world.date)
        return;
    change_balance(*world.ledger, *world.balance, world.date->day, world.date->month,
                   BalanceChange{type, amount, 0, 0});
}

} // namespace

bool is_person_highlighted_for_vaccination(const World &world, uint16_t pointed, uint16_t &target)
{
    target = 0;
    if (pointed == 0 || pointed >= world.things.size())
        return false;
    const Thing &person = world.things[pointed];
    if (person.type != kThingPatient || person.owner != 0 || int8_t(person.contagion) <= 0 ||
        world.epidemic.illness != person.illness)
        return false;
    if (is_person_in_vaccine_queue(world, pointed) == 0) {
        target = pointed;
        return true;
    }
    return false;
}

void request_vaccination(World &world, const AnimData &anims, uint16_t id)
{
    insert_person_in_vaccination_queue(world, id);
    Thing &person = world.things[id];
    person.person_flags |= kPfVaccineRequested;
    set_icon_value(anims, person, kIconVaccineRequested);
    world.epidemic.vaccine_requests++;
}

void cancel_vaccination_request(World &world, uint16_t id)
{
    Thing &person = world.things[id];
    if (is_person_in_vaccine_queue(world, id) && !(person.person_flags & kPfVaccineNurseSent))
        world.epidemic.vaccine_requests--;
    remove_person_from_vaccination_queue(world, id);
    person.person_flags &= ~(kPfVaccineRequested | kPfVaccineNurseSent);
    reset_icon_value(person, kIconVaccineRequested);
    reset_icon_value(person, kIconVaccineNurseSent);
}

void insert_person_in_vaccination_queue(World &world, uint16_t id)
{
    Thing &person = world.things[id];
    person.vaccine_next = 0;
    if (world.epidemic.vaccine_queue) {
        Thing *last = &world.things[world.epidemic.vaccine_queue];
        while (last->vaccine_next)
            last = &world.things[last->vaccine_next];
        last->vaccine_next = id;
    } else {
        world.epidemic.vaccine_queue = id;
    }
}

uint8_t remove_person_from_vaccination_queue(World &world, uint16_t id)
{
    Thing &person = world.things[id];
    if (id == world.epidemic.vaccine_queue) {
        world.epidemic.vaccine_queue = person.vaccine_next;
        person.vaccine_next = 0;
        return 1;
    }
    for (uint16_t queued = world.epidemic.vaccine_queue; queued;) {
        Thing &q = world.things[queued];
        if (q.vaccine_next == id) {
            q.vaccine_next = person.vaccine_next;
            person.vaccine_next = 0;
            return 1;
        }
        queued = q.vaccine_next;
    }
    return 0;
}

uint8_t is_person_in_vaccine_queue(const World &world, uint16_t id)
{
    for (uint16_t queued = world.epidemic.vaccine_queue; queued; queued = world.things[queued].vaccine_next)
        if (queued == id)
            return 1;
    return 0;
}

bool should_nurse_vaccinate(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                            const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn)
{
    if (world.epidemic.vaccine_requests == 0)
        return false;
    Thing &nurse = world.things[id];
    for (uint16_t pid = world.epidemic.vaccine_queue; pid;) {
        Thing &patient = world.things[pid];
        uint16_t next = patient.vaccine_next;
        if ((patient.person_flags & kPfVaccineRequested) && !(patient.person_flags & kPfVaccineNurseSent) &&
            !patient.room) {
            uint16_t pos = can_i_go_near_point(world, nurse, cell_of(patient));
            if (pos && !(world.map[pos].owner_flags & kCellOutdoor)) {
                if (nurse.room)
                    set_complex(world, id, kComplexLeaveRoom, world.rooms[nurse.room].door_inside_tile);
                set_complex(world, id, kComplexWalkTo, pos);
                nurse.next_state = kStaffNurseVaccinates;
                nurse.resume_state = kStaffNurseVaccinates;
                process_staff_complex(world, anims, person_complexes, staff_complexes, id, game_turn);
                nurse.slave = pid;
                patient.person_flags |= kPfVaccineNurseSent;
                reset_icon_value(patient, kIconVaccineRequested);
                set_icon_value(anims, patient, kIconVaccineNurseSent);
                world.epidemic.vaccine_requests--;
                return true;
            }
        }
        pid = next;
    }
    return false;
}

void nurse_vaccinates(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                      const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn)
{
    Thing &nurse = world.things[id];
    uint16_t pid = nurse.slave;
    nurse.slave = 0;
    if (pid == 0 || pid >= world.things.size()) {
        nurse.state = kStaffInCorridor;
        return;
    }
    Thing &patient = world.things[pid];
    if (!is_person_in_vaccine_queue(world, pid)) {
        nurse.state = kStaffInCorridor;
    } else if (am_i_near_point(nurse, cell_of(patient))) {
        cancel_vaccination_request(world, pid);
        nurse.state = kStaffInCorridor;
        reset_icon_value(patient, kIconVaccineNurseSent);
        set_icon_value(anims, patient, kIconVaccinated);
        if (int8_t(patient.contagion) > 0)
            patient.contagion = 0xff;
        pay(world, kTxVaccination, -int32_t(world.settings->gbv.vac_cost));
    } else {
        uint16_t pos = can_i_go_near_point(world, nurse, cell_of(patient));
        if (pos && !(world.map[pos].owner_flags & kCellOutdoor)) {
            set_complex(world, id, kComplexWalkTo, pos);
            nurse.next_state = kStaffNurseVaccinates;
            nurse.resume_state = kStaffNurseVaccinates;
            process_staff_complex(world, anims, person_complexes, staff_complexes, id, game_turn);
            nurse.slave = pid;
        } else {
            nurse.state = kStaffInCorridor;
            patient.person_flags &= ~kPfVaccineNurseSent;
            world.epidemic.vaccine_requests++;
        }
    }
}

bool could_nurse_vaccinate(const World &world, uint16_t id)
{
    const Thing &nurse = world.things[id];
    if (world.epidemic.vaccine_requests == 0)
        return false;
    uint16_t from = nurse.room ? world.rooms[nurse.room].door_tile : cell_of(nurse);
    for (uint16_t pid = world.epidemic.vaccine_queue; pid; pid = world.things[pid].vaccine_next) {
        const Thing &patient = world.things[pid];
        if (!(patient.person_flags & kPfVaccineRequested) || (patient.person_flags & kPfVaccineNurseSent) ||
            patient.room)
            continue;
        uint16_t pos = cell_of(patient);
        if (!(world.map[pos].owner_flags & kCellOutdoor) && points_connected(world, pos, from))
            return true;
    }
    return false;
}

} // namespace th
