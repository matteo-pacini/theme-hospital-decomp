// Treating a patient: paying for it, the treatment machines, health changes,
// drugs and forced cures (src/treat.cpp, pay_for_treatment in src/finance.cpp).

#include "core/world/rooms.h"
#include "core/world/rooms_internal.h"

#include "core/audio/sounds.h"
#include "core/ui/charts.h"
#include "core/world/arrivals.h"
#include "core/world/build.h"
#include "core/world/complex.h"
#include "core/world/complex_tables.h"
#include "core/world/disaster.h"
#include "core/world/epidemic.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/mayor.h"
#include "core/world/patient_tables.h"
#include "core/world/queue.h"
#include "core/world/reaper.h"
#include "core/world/reception.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

#include <algorithm>
#include <cstdlib>

namespace th
{

namespace
{

// Room types (the game's ROOM_*).
constexpr uint8_t kRoomDestroyed = 6;

// The complexes that walk a patient out of its room (P_COMP.LEX), or just
// end what it does there.
constexpr uint16_t kComplexLeaveRoom = 90;

constexpr uint16_t kComplexStandStill = 240;

// allhosps.insurance_accepted, which init_player sets to all three
// companies and nothing changes.
constexpr uint8_t kInsuranceAccepted = 7;

// insurance_co[].payment_delay (src/tables.cpp): months before an insurance
// company pays.
constexpr std::array<uint8_t, 9> kPaymentDelay = {0, 3, 6, 8, 2, 9, 10, 4, 3};

// The game's possible_diagnoses (src/tables.cpp): per set, the count and the
// diagnosis treatments a doctor may send a patient for.
struct PossibleDiag {
    uint8_t count;
    std::array<uint8_t, 9> treatments;
};

constexpr std::array<PossibleDiag, 11> kPossibleDiagnoses = {{
    {1, {}},
    {9, {36, 37, 38, 39, 40, 41, 42, 43, 42}},
    {4, {36, 38, 39, 41}},
    {2, {36, 39}},
    {2, {36, 39}},
    {2, {36, 39}},
    {8, {36, 37, 38, 39, 40, 41, 42, 43}},
    {2, {36, 39}},
    {1, {36}},
    {2, {36, 37, 39}},
    {1, {36}},
}};

const Balance &balance_of(const World &world)
{
    static const Balance statics;
    return world.settings ? *world.settings : statics;
}

// add_health_to_history (0x3d188).
void add_health_to_history(Thing &p)
{
    int last = 0;
    size_t i = 0;
    for (; i < p.health_history.size(); i++) {
        if (p.health_history[i] == 0)
            break;
        last = p.health_history[i] * 10;
    }
    if (std::abs(last - p.health) >= 50) {
        if (i >= p.health_history.size()) {
            // As the game: entry 0 stays, entries 2-9 move down one.
            for (i = 1; i < 9; i++)
                p.health_history[i] = p.health_history[i + 1];
        }
        p.health_history[i] = int8_t(p.health / 10);
    }
}

// calc_ability (0x3d658): the member of staff's diagnosis this time: their
// ability plus a random 250 either way (world.seed), their grade, their
// tiredness, a cracked-up doctor's madness and the room (calc_room_value).
int32_t calc_ability(World &world, const Thing &staff)
{
    const Balance::Gbv &gbv = balance_of(world).gbv;
    int32_t random_adj = 250 - int32_t(old_normal_rand(world.seed, 0, 500));
    int32_t skill_adj = 0;
    if (staff.skills & kSkillConsultant)
        skill_adj = 200;
    else if (staff.skills & kSkillJunior)
        skill_adj = -100;
    int32_t tired_adj = 0;
    if (staff.tiredness < gbv.not_tired)
        tired_adj = 100;
    else if (staff.tiredness >= gbv.tired)
        tired_adj = -100;
    else if (staff.tiredness >= gbv.very_tired)
        tired_adj = -200;
    int32_t quack_adj = staff.subtype == kStaffDoctor && staff.crazy ? -400 : 0;
    return staff.ability + random_adj + skill_adj + tired_adj + quack_adj + calc_room_value(world, staff.room);
}

// pay_for_treatment by insurance: `fee` is claimed from the first of the
// patient's insurers, to be paid after its delay.
void pay_for_treatment_claim_insurance(World &world, const Thing &p, int32_t fee)
{
    for (size_t i = 0; i < 3; i++) {
        if (!(p.insurance & (1 << i)))
            continue;
        if (world.hospital && world.date && p.owner == 0) {
            int month = world.date->month + kPaymentDelay[world.ins_control[i] % kPaymentDelay.size()];
            if (month >= 12)
                month -= 12;
            if (month >= 12)
                month = 0;
            world.hospital->insurance_claims[i][size_t(month)] += fee;
        }
        create_numfx(world, kNumfxMoney, int16_t(fee), uint16_t(p.x.tile + (p.y.tile << 7)), colour_red(world), 0x40);
        break;
    }
}

} // namespace

bool pay_for_treatment(World &world, uint16_t id, int kind)
{
    Thing &p = world.things[id];
    trace_event("pay %u kind=%d fee=%d cash=%d", unsigned(id), kind, int(p.fee), int(p.cash));

    static HospitalMarket no_market;
    const HospitalMarket &market = world.market ? *world.market : no_market;
    int32_t price = market.price[p.treatment % market.price.size()];
    int32_t fee = 0;
    if (kind == 2) {
        if (p.fee == 0)
            p.fee = price;
        if (world.hospital && p.owner == 0)
            world.hospital->illnesses[p.treatment % kIllnessCount].income += p.fee;
        fee = p.fee;
        p.fee = 0;
    } else {
        p.fee = price;
    }
    if (fee <= 0)
        return true;
    if (p.insurance & kInsuranceAccepted) {
        pay_for_treatment_claim_insurance(world, p, fee);
        return true;
    }
    if (fee > p.cash)
        return false;
    p.cash -= fee;
    if (world.ledger && world.balance && world.date && p.owner == 0)
        change_balance(*world.ledger, *world.balance, world.date->day, world.date->month, {kTxTreat, fee, p.treatment});
    create_numfx(world, kNumfxMoney, int16_t(fee), uint16_t(p.x.tile + (p.y.tile << 7)), kColourBlack, 0x40);
    return true;
}

void done_treatment(World &world, const AnimData &anims, uint16_t staff, uint16_t patient, uint32_t game_turn)
{
    trace_event("done_treatment %u staff=%u", unsigned(patient), unsigned(staff));

    Thing &p = world.things[patient];
    if (p.room != 0 && world.rooms[p.room].type == kRoomDestroyed)
        return;
    affect_diagnosis(world, anims, staff, patient, game_turn);
    add_treatment_to_history(p);
    setup_next_treatment(world, anims, staff, patient, game_turn);
}

void next_treatment_now_setup(World &world, const AnimData &anims, uint16_t id, bool in_room, uint32_t game_turn)
{
    Thing &p = world.things[id];
    if (p.person_flags & kPfNeedsDiagnosisRoom) {
        if (check_playercall(world, kCallNoDiagnosis, 0, p.owner) <= 5) {
            p.person_flags |= kPfAwaitingDecision;
            p.next_state = kPsWaitsForDecision;
            p.timer = int16_t(game_turn);
            p.playercall = make_a_playercall(world, kCallNoDiagnosis, id, 0, p.owner);
            if (p.owner == 0)
                world.advisor_events |= kAdvisorEventCall;
        } else {
            p.treatment = 0;
            p.next_state = kPsGoForTreatment;
            turned_away(world, p);
            p.person_flags |= kPfNoDiagnosis;
        }
    } else if (p.person_flags & kPfAwaitingDecision) {
        if (check_playercall(world, kCallNoTreatment, p.illness, p.owner) <= 5) {
            p.next_state = kPsWaitsForDecision;
            p.timer = int16_t(game_turn);
            p.playercall = make_a_playercall(world, kCallNoTreatment, id, 0, p.owner);
            if (p.owner == 0)
                world.advisor_events |= kAdvisorEventCall;
        } else {
            p.treatment = 0;
            p.next_state = kPsGoForTreatment;
            turned_away(world, p);
            p.person_flags |= kPfNoTreatment;
        }
    } else {
        p.next_state = kPsGoForTreatment;
    }
    if (p.room != 0) {
        const PersonComplexes &complexes = person_complexes_of(world);
        bool go = in_room ? set_complex(world, id, kComplexLeaveRoom, world.rooms[p.room].door_inside_tile)
                          : set_complex(world, id, kComplexStandStill, 0);
        if (go)
            process_person_complex(world, anims, complexes, id, game_turn);
    } else {
        p.state = p.next_state;
        p.next_state = 0;
    }
}

void affect_diagnosis(World &world, const AnimData &anims, uint16_t staff_id, uint16_t patient_id, uint32_t game_turn)
{
    Thing &staff = world.things[staff_id];
    Thing &patient = world.things[patient_id];
    if (patient.treatment >= kFirstDiagnosis || patient.treatment == kIllGeneralPractice) {
        int32_t total = patient.diagnosis - patient.diagnosis_handicap;
        total += calc_ability(world, staff);
        int16_t bonus = 0;
        if (patient.treatment >= kFirstDiagnosis && patient.illness < kDiagRegister.size())
            bonus = kDiagRegister[patient.illness][size_t(patient.treatment - kFirstDiagnosis) % 8];
        total += bonus;
        if (patient.diagnosis < total)
            patient.diagnosis = int16_t(total);
        if (bonus < 0)
            change_health(world, anims, patient_id, -5, game_turn);
    }
    staff.ability = uint16_t(std::clamp(int(staff.ability) + 1, 30, 1000));
}

bool treatment_machine_used(World &world, const AnimData &anims, uint8_t illness, uint16_t tile, uint32_t game_turn)
{
    uint8_t kind = kExpertiseData[illness % kExpertiseData.size()].kind;
    if (!kObjectUse[kind].overlay_anim)
        return false;
    uint16_t id = find_object(world, tile, kind, false, false, 0);
    if (!id)
        return false;
    Thing &machine = world.things[id];
    if (machine.strength * 2 > machine.times_used)
        machine.times_used++;
    if (machine.times_used > machine.strength / 3 && machine.owner == 0) {
        int32_t percent = machine.times_used * 100 / (machine.strength + 1);
        thingpage_request(world.repair_pager, id, uint8_t(percent < 100 ? percent : 100));
    }
    if (machine.strength * 75 / 100 <= machine.times_used)
        return modify_machine(world, anims, id, game_turn);
    return false;
}

uint16_t find_possible_diagnosis(uint32_t &seed, Thing &p, const std::array<uint8_t, 47> &cure_available)
{
    const PossibleDiag &diag = kPossibleDiagnoses[kExpertiseData[p.illness].diag_set % kPossibleDiagnoses.size()];
    uint16_t result = 0;
    p.treatment = 0;
    if (diag.count == 0)
        return 0;
    std::array<uint8_t, 9> list{};
    uint16_t count = 0;
    for (size_t i = 0; i < diag.count; i++) {
        uint8_t treatment = diag.treatments[i];
        if (cure_available[treatment] == 1)
            list[count++] = treatment;
    }
    if (count == 0)
        return 0;
    for (int i = 0; i < 9; i++) {
        uint16_t pick = uint16_t(old_newrand(seed) % count);
        uint8_t treatment = list[pick];
        if (cure_available[treatment] != 0) {
            if (!treatment_already_done(p, treatment)) {
                result = treatment;
                break;
            }
            for (uint16_t j = pick; j + 1 < count; j++)
                list[j] = list[j + 1];
            if (--count == 0)
                break;
        }
    }
    p.treatment = uint8_t(result);
    return result;
}

uint16_t find_possible_diagnosis(World &world, uint16_t id)
{
    return find_possible_diagnosis(world.seed, world.things[id], world.cure_available);
}

void change_health(World &world, const AnimData &anims, uint16_t id, int16_t amount, uint32_t game_turn)
{
    Thing &p = world.things[id];
    int health = p.health;
    // change_health's health icon, by the current health (set whatever it is;
    // set_icon_value leaves a dead patient's alone).
    if (health <= 20)
        set_icon_value(anims, p, kIconHealth5);
    else if (health <= 40)
        set_icon_value(anims, p, kIconHealth4);
    else if (health <= 60)
        set_icon_value(anims, p, kIconHealth3);
    else if (health <= 80)
        set_icon_value(anims, p, kIconHealth2);
    else if (health <= 100) {
        reset_icon_value(p, p.icon_id);
        set_icon_value(anims, p, kIconHealth1);
    } else if (p.icon_id >= kIconHealth1 && p.icon_id <= kIconHealth5) {
        reset_icon_value(p, p.icon_id);
    }
    if (health <= 0)
        return;
    if (p.room != 0) {
        p.health_delta = int16_t(p.health_delta + amount);
        return;
    }
    amount = int16_t(amount + p.health_delta);
    p.health_delta = 0;
    health += amount;
    p.health = int16_t(health < 0 ? 0 : health > 1000 ? 1000 : health);
    add_health_to_history(p);
    if (p.health <= 0)
        person_dies(world, anims, id, game_turn);
}

bool pre_administer_drugs(const World &world, uint16_t id)
{
    const Thing &p = world.things[id];
    return p.owner == 0 && hospital_of(world).illnesses[p.illness % kIllnessCount].drug_effectiveness != 0;
}

void administer_drugs(World &world, const AnimData &anims, uint16_t staff_id, uint16_t id, uint32_t game_turn)
{
    trace_event("drugs %u staff=%u", unsigned(id), unsigned(staff_id));
    Thing &p = world.things[id];
    Thing &staff = world.things[staff_id];
    p.impatience = 0;
    p.waiting_time = 0;
    add_treatment_to_history(p);
    const IllnessFigures &drug = hospital_of(world).illnesses[p.illness % kIllnessCount];
    int32_t effect = p.owner == 0 ? drug.drug_effectiveness : 0;
    if (p.person_flags & kPfWrongGuess)
        effect = 0;
    if (int32_t(old_normal_rand(world.seed, 0, 99)) < effect) {
        pay_for_treatment(world, id, 2);
        p.treatment = 0;
        cure_illness(world, id);
        staff.happiness = uint16_t(staff.happiness + 5);
        p.person_flags &= ~kPfAwaitingDecision;
        reset_icon_value(p, kIconAwaitingDecision);
        p.person_flags |= kPfCured;
        change_health(world, anims, id, 1000, game_turn);
        world.treatment_sounds.push_back(kSfxCheer);
    } else {
        pay_for_treatment(world, id, 2);
        if ((p.person_flags & kPfEmergency) || kNotContagious[p.illness % kNotContagious.size()]) {
            change_health(world, anims, id, -1000, game_turn);
            world.treatment_sounds.push_back(kSfxBoo);
        } else {
            if (p.health <= 1000 - effect * 10)
                world.treatment_sounds.push_back(kSfxBoo);
            change_health(world, anims, id, int16_t(-(1000 - effect * 10)), game_turn);
        }
        p.treatment = kIllGeneralPractice;
        pay_for_treatment(world, id, 3);
    }
    p.next_state = kPsGoForTreatment;
    if (set_complex(world, id, kComplexLeaveRoom, world.rooms[staff.room].door_inside_tile))
        process_person_complex(world, anims, person_complexes_of(world), id, game_turn);
    int32_t cost = p.owner == 0 ? drug.drug_cost : 0;
    if (staff.owner == 0 && world.ledger && world.balance && world.date)
        change_balance(*world.ledger, *world.balance, world.date->day, world.date->month, {kTxDrugCost, -cost});
}

void force_peep_to_be_cured(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    trace_event("force_cure %u", unsigned(id));
    Thing &p = world.things[id];
    p.next_state = 0;
    p.person_flags = 0;
    leave_queue(world, id);
    interrupt_person(world, anims, person_complexes_of(world), id, game_turn);
    best_guess_treatment(world, p);
    next_treatment_now_setup(world, anims, id, false, game_turn);
}

} // namespace th
