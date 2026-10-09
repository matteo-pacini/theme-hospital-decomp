#include "core/world/computer.h"

#include "core/world/arrivals.h"
#include "core/world/reputation.h"
#include "core/world/rooms.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <algorithm>
#include <source_location>

namespace th
{

namespace
{

constexpr uint8_t kIllGeneralPractice = 1;
constexpr uint8_t kObjBench = 4;
constexpr uint8_t kObjDrinksMachine = 7;
constexpr uint8_t kObjPharmacyCabinet = 39;

// The game's static gbv.diagnosis_complete (src/array.cpp; not a balance key).
constexpr int kDiagnosisComplete = 1000;

// skillnb_to_who (src/tables.cpp): the staff kind (RivalHospital::staff_by_kind)
// that a treatment's lowest skill bit asks for, by that bit; index 9 is no bit.
constexpr std::array<uint8_t, 10> kSkillNbToWho = {3, 2, 0, 1, 1, 1, 1, 1, 1, 1};

// The days a stage of a visit takes (ADVANCE_TIME's step) and the days in a
// month the simulated clock counts.
constexpr int kStepDays = 5;
constexpr int kClockMonthDays = 30;

// One run of process_computer_peep: what it works on and its state.
struct Visit {
    World &world;
    const Balance &settings;
    RivalHospital &rival;
    HospitalMarket &market;
    Thing &patient;
    // process_computer_peep, to which its draws are traced.
    std::source_location where;
    uint8_t owner;
    // The simulated date: the month (0-11) and the days into it.
    int month;
    int hours;
    ComputerBucket *bucket;
    bool has_drinks;
    // The chance (percent) of leaving, and the health, that fall as the visit goes on.
    int chance;
    int health;
    // 1 diagnosed or guessed right, 2 died, 3 left.
    int result = 0;
};

// ADVANCE_TIME.
void advance_time(Visit &v)
{
    v.hours += kStepDays;
    if (v.hours > kClockMonthDays) {
        v.hours = 0;
        v.month = (v.month + 1) % 12;
        v.bucket = &v.rival.buckets[size_t(v.month)];
    }
}

void reputation(Visit &v, uint8_t treatment, RepEvent event)
{
    rival_reputation_affected(v.rival, v.market, v.settings, uint8_t(v.month), treatment, event);
}

// old_newrand() % `range`, drawn as process_computer_peep.
uint32_t roll(Visit &v, int range)
{
    return old_newrand(v.world.seed, v.where) % uint32_t(range);
}

// process_computer_peep's chance of leaving at the first stage. The game
// divides unsigned and signed alike here: every operand is positive.
int first_chance(const RivalHospital &rival)
{
    if (rival.patient_load <= 5)
        return 20;
    int load = rival.patient_load;
    int chance = 100 - load / int((rival.rooms_built + 1) * 10) - load / ((rival.staff_count + 1) * 10) -
                 load / ((rival.objects[kObjBench] + 1) * 10);
    return std::clamp(chance, 0, 100);
}

// A diagnosis visit to the GP: true if it ends the visit (diagnosed, or the
// hospital has no GP).
bool process_computer_peep_gp_visit(Visit &v)
{
    uint8_t illness = v.patient.illness;
    if (v.rival.cure_available[kIllGeneralPractice] != 1) {
        reputation(v, illness, kRepNoTreatmentFound);
        v.result = 3;
        return true;
    }
    update_single_cure_cost(v.market, v.settings, kIllGeneralPractice);
    v.bucket->income += v.market.price[kIllGeneralPractice];
    v.patient.treatment = kIllGeneralPractice;
    computer_diag_progress(v.world.seed, v.rival, v.patient);
    reputation(v, kIllGeneralPractice, kRepNextTreatment);
    if (v.patient.diagnosis <= kDiagnosisComplete)
        return false;
    v.patient.treatment = illness;
    reputation(v, illness, kRepComputerDiagnosed);
    v.market.diagnosed++;
    v.result = 1;
    return true;
}

// With no diagnosis room left to try, the computer player's guess at the cure:
// right with the odds of the diagnosis so far, if its personality guesses at
// that much.
void process_computer_peep_guess(Visit &v)
{
    reputation(v, v.patient.illness, kRepNoTreatmentFound);
    int pct = std::clamp(v.patient.diagnosis * 100 / kDiagnosisComplete, 0, 99);
    if (v.settings.computer[v.rival.ai_index].guess_at > pct)
        v.result = 3;
    else
        v.result = roll(v, 100) > uint32_t(pct) ? 2 : 1;
}

// A visit to a diagnosis room, or the guess if none is left: true if it ends
// the visit.
bool process_computer_peep_diagnosis_visit(Visit &v)
{
    Thing &patient = v.patient;
    find_possible_diagnosis(v.world.seed, patient, v.rival.cure_available);
    if (patient.treatment == 0) {
        process_computer_peep_guess(v);
        return true;
    }
    advance_time(v);
    update_single_cure_cost(v.market, v.settings, patient.treatment);
    v.bucket->income += v.market.price[patient.treatment];
    add_treatment_to_history(patient);
    computer_diag_progress(v.world.seed, v.rival, patient);
    reputation(v, patient.treatment, kRepNextTreatment);
    if (roll(v, v.chance) == 0) {
        v.result = 3;
        return true;
    }
    return false;
}

// The visit's stages until the patient is diagnosed, leaves or dies: sets
// Visit::result.
void process_computer_peep_stages(Visit &v)
{
    for (;;) {
        advance_time(v);
        if (process_computer_peep_gp_visit(v))
            return;
        advance_time(v);
        if (v.has_drinks && roll(v, 5) == 0) {
            advance_time(v);
            v.bucket->income += 20;
        }
        // The chance is not 0: the first is 20, or 100 less three quotients of the patient
        // load that cancel it only from a load of 334, and it falls to 5 at least.
        if (roll(v, v.chance) == 0) {
            v.result = 3;
            return;
        }
        v.chance = std::max(v.chance - 1, 5);
        // The health is not 0: it starts at 200 or more and falls to 5 at least.
        if (roll(v, v.health) == 0) {
            v.result = 2;
            return;
        }
        v.health = std::max(v.health - 1, 5);
        if (process_computer_peep_diagnosis_visit(v))
            return;
    }
}

// The cure of a diagnosed patient: a drug cure can fail, and the patient
// dies.
void process_computer_peep_cure(Visit &v)
{
    uint8_t illness = v.patient.illness;
    if (v.rival.cure_available[illness] != 1) {
        reputation(v, illness, kRepGoForTreatment);
        v.result = 3;
        return;
    }
    // The game tests expertise.research_type == 1; the illnesses with it (5, 11, 13, 15, 16, 17, 24, 25,
    // 28, 29, 32, 33) are those cured with the pharmacy cabinet.
    bool drug = kExpertiseData[illness].kind == kObjPharmacyCabinet;
    if (drug && old_normal_rand(v.world.seed, 0, 99, v.where) >= v.rival.drug_effectiveness[illness]) {
        v.bucket->deaths++;
        reputation(v, illness, kRepDied);
    } else {
        v.bucket->cures++;
        reputation(v, illness, kRepCured);
    }
}

// What the visit came to, by Visit::result.
void process_computer_peep_result(Visit &v)
{
    uint8_t illness = v.patient.illness;
    reputation(v, illness, kRepNextTreatment);
    switch (v.result) {
    case 1:
        process_computer_peep_cure(v);
        break;
    case 2:
        v.bucket->deaths++;
        reputation(v, illness, kRepDied);
        break;
    case 3:
        reputation(v, illness, kRepGoForTreatment);
        break;
    default:
        break;
    }
    reputation(v, illness, kRepCase);
}

} // namespace

void computer_diag_progress(uint32_t &seed, const RivalHospital &rival, Thing &patient)
{
    uint32_t skill = kExpertiseData[patient.treatment].staff_mask;
    size_t bit = 0;
    while (bit < 9 && !(skill & (1u << bit)))
        bit++;
    uint8_t who = kSkillNbToWho[bit];
    // A hospital that gives a treatment has hired the staff it needs
    // (set_expertise_staff, set_expertise_room), so the divisor is not 0.
    int val = patient.diagnosis - patient.diagnosis_handicap;
    val += rival.staff_ability_sum[who] / rival.staff_by_kind[who];
    val += 250 - int(old_normal_rand(seed, 0, 500));
    val += diag_register(patient.illness, patient.treatment);
    if (patient.diagnosis < val)
        patient.diagnosis = int16_t(val);
}

void process_computer_peep(World &world, ArrivalState &state, const Balance &settings, uint16_t patient_id,
                           const RivalPeepVisit &visit)
{
    RivalHospital &rival = world.rivals[visit.owner];
    Thing &patient = world.things[patient_id];
    int month = visit.month % 12;
    rival.patient_load++;
    bool drinks = rival.objects[kObjDrinksMachine] * 10 > rival.patient_load;
    Visit v{world,
            settings,
            rival,
            state.hospitals[visit.owner],
            patient,
            std::source_location::current(),
            visit.owner,
            month,
            visit.day,
            &rival.buckets[size_t(month)],
            drinks || visit.stale_drinks,
            first_chance(rival),
            patient.health};
    process_computer_peep_stages(v);
    process_computer_peep_result(v);
    trace_event("computer_peep %u illness=%u result=%d load=%d", unsigned(visit.owner), unsigned(patient.illness),
                v.result, int(rival.patient_load));
    v.bucket->patients_done++;
}

} // namespace th
