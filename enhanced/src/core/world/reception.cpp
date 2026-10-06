#include "core/world/reception.h"

#include "core/world/arrivals.h"
#include "core/world/reputation.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

#include <array>

namespace th
{

namespace
{

// insurance_co[].extra_pay_pct (src/tables.cpp): per insurance company, the
// percentage its patients pay over the most others will.
constexpr std::array<uint8_t, 9> kInsuranceExtraPay = {0, 15, 10, 5, 8, 9, 40, 12, 3};

// allhosps[0].insurance_accepted, which init_player sets to all three
// companies and nothing changes.
constexpr uint8_t kInsuranceAccepted = 7;

// The complexes interrupt_person gives a patient in a room (P_COMP.LEX):
// those of patient kinds 4-5 and 9-10 that put their looks back (action
// 34, orig_subtype), then out
// by the room's door.
constexpr uint16_t kComplexLooksBackKinds45 = 0xb4;
constexpr uint16_t kComplexLooksBackKinds910 = 0x276;
constexpr uint16_t kComplexLeaveRoom = 90;

HospitalMarket &market_of(World &world)
{
    static HospitalMarket none;
    return world.market ? *world.market : none;
}

const Balance &balance_of(const World &world)
{
    static const Balance statics;
    return world.settings ? *world.settings : statics;
}

} // namespace

void update_single_cure_cost(World &world, uint16_t treatment)
{
    update_single_cure_cost(market_of(world), balance_of(world), treatment);
}

uint16_t reception_queue_head(const World &world, uint16_t room)
{
    return world.rooms[room].queue_head;
}

uint16_t reception_queue_next(const World &world, uint16_t patient)
{
    return world.things[patient].queue_next;
}

bool reception_emergency(const World &world, uint16_t patient)
{
    return world.things[patient].person_flags & kPfEmergency;
}

uint16_t call_person_to_room(const PersonRun &run, uint16_t caller, uint16_t id, uint16_t action, uint16_t arg,
                             bool follow)
{
    World &world = run.world;
    trace_event("call %u id=%u action=%u arg=%u", unsigned(caller), unsigned(id), unsigned(action), unsigned(arg));
    for (;;) {

        Thing &callee = world.things[id];
        if (callee.health <= 0) {
            uint16_t next = callee.queue_next;
            if (next == 0)
                break;
            leave_queue(world, id);
            id = next;
            continue;
        }
        if (person_ok_to_call(world, id)) {
            Thing &c = world.things[caller];
            c.slave = id;
            c.timer = 200;
            c.call_retries = int16_t((old_newrand(world.seed) & 3) + 3);
            leave_queue(world, id);
            if (set_complex(world, id, action, arg))
                process_person_complex(world, run.anims, run.complexes, id, run.game_turn);
            callee.next_state = kPsArrivedFromCall;
            return id;
        }
        if (!follow || callee.queue_next == 0)
            break;
        id = callee.queue_next;
    }
    return 0;
}

uint8_t call_person_again(const PersonRun &run, uint16_t caller, uint16_t action, uint16_t arg)
{
    World &world = run.world;
    Thing &c = world.things[caller];
    if (c.slave == 0)
        return 0;
    uint16_t id = c.slave;
    Thing &callee = world.things[id];
    if (callee.health <= 0 || callee.type != kThingPatient || callee.queue_room)
        return 0;
    if (!person_ok_to_call(world, id)) {
        c.timer = 200;
        return 2;
    }
    if (callee.complex_pc / 10 * 10 != action) {
        if (callee.complex_pc == 0 || callee.complex_queued == 0) {
            if (set_complex(world, id, action, arg))
                process_person_complex(world, run.anims, run.complexes, id, run.game_turn);
            callee.next_state = kPsArrivedFromCall;
        } else if (callee.complex_queued != action) {
            return 0;
        }
    } else if (uint16_t(callee.dest_x + (callee.dest_y << 7)) != arg) {
        return 0;
    }
    callee.next_state = kPsArrivedFromCall;
    c.timer = 200;
    if (--c.call_retries <= 0)
        return 1;
    return 2;
}

bool forget_about_my_slave(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t caller,
                           uint16_t action, uint32_t game_turn)
{
    Thing &w = world.things[caller];
    if (w.slave == 0)
        return true;
    const Thing &slave = world.things[w.slave];
    if (slave.health <= 0 || slave.type != kThingPatient || slave.queue_room)
        return true;
    if (slave.next_state == kPsArrivedFromCall)
        return false;
    if (slave.next_state != kPsEntertainsThemself)
        return true;
    if (w.subtype == kStaffReceptionist) {
        if (w.object)
            call_person_to_room({world, anims, complexes, game_turn}, caller, w.slave, action,
                                world.things[w.object].use_pos[1], false);
    } else if (uint16_t tile = world.rooms[w.room].door_tile) {
        call_person_to_room({world, anims, complexes, game_turn}, caller, w.slave, action, tile, false);
    }
    return false;
}

void interrupt_person(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t person,
                      uint32_t game_turn)
{
    Thing &p = world.things[person];
    bool was_deciding = p.state == kPsWaitsForDecision || p.next_state == kPsWaitsForDecision;
    uint8_t state;
    if (p.queue_room) {
        state = kPsReturnToQueueArea;
    } else if (p.dest_room) {
        state = kPsGotoQueue;
    } else {
        leave_queue(world, person);
        state = kPsGoForTreatment;
    }
    interrupt_complex(world, anims, complexes, person, game_turn);
    if (p.room) {
        for (size_t i = 1; i < world.things.size(); i++) {
            Thing &staff = world.things[i];
            if (staff.type == kThingStaff && staff.room == p.room && staff.slave == person) {
                staff.slave = 0;
                interrupt_staff(world, anims, uint16_t(i), game_turn);
                break;
            }
        }
        switch (p.subtype) {
        case 4:
        case 5:
            set_complex(world, person, kComplexLooksBackKinds45, 0);
            break;
        case 9:
        case 10:
            p.resume_state = 0;
            set_complex(world, person, kComplexLooksBackKinds910, 0);
            break;
        default:
            break;
        }
        set_complex(world, person, kComplexLeaveRoom, world.rooms[p.room].door_inside_tile);
        process_person_complex(world, anims, complexes, person, game_turn);
        p.next_state = state;
    } else {
        p.state = was_deciding ? kPsWaitsForDecision : state;
    }
}

bool reception_cure_available(const World &world, uint16_t patient)
{
    const Thing &p = world.things[patient];
    return p.owner == 0 && world.cure_available[p.treatment] == 1;
}

bool shall_peep_pay(World &world, uint16_t id)
{
    Thing &p = world.things[id];
    HospitalMarket &hosp = market_of(world);
    uint16_t treatment = p.treatment;
    world.unpaid_treatment = p.treatment;
    bool pays = false;
    for (int attempt = 0; attempt < 2; attempt++) {
        pays = false;
        update_single_cure_cost(world, treatment);
        // The game reads the price into 16 bits.
        auto price = uint16_t(hosp.price[treatment]);
        int32_t max = max_price_for_treatment(hosp, balance_of(world), treatment);
        if (uint8_t ins = p.insurance & kInsuranceAccepted) {
            uint16_t company = world.ins_control[ins];
            max += (company < kInsuranceExtraPay.size() ? kInsuranceExtraPay[company] : 0) * max / 100;
        } else if (price > p.cash) {
            max = 0;
        }
        max = int32_t(uint32_t(max) + old_newrand(world.seed) % uint32_t(price / 2 + 1));
        if (price <= max)
            pays = true;
        treatment = p.next_treatment;
        if (treatment == 0 || !pays)
            break;
    }
    if (!pays) {
        p.person_flags |= kPfTooExpensive;
        record_person_as_patient(world, id);
        if (p.owner == 0)
            world.too_expensive = true;
    }
    trace_event("shall_pay %u pays=%d", unsigned(id), int(pays));
    return pays;
}

void reception_serves_patient(World &world, uint16_t receptionist, uint16_t patient)
{
    trace_event("checkin %u receptionist=%u", unsigned(patient), unsigned(receptionist));
    Thing &p = world.things[patient];
    p.impatience = 0;
    p.waiting_time = 0;
    if (shall_peep_pay(world, patient))
        pay_for_treatment(world, patient, 3);
    else

        p.treatment = 0;
    p.state = kPsGoForTreatment;
}

} // namespace th
