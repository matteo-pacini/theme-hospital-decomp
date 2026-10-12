// Diagnosing a patient and choosing its next treatment: diagnose_patient,
// cure_illness and setup_next_treatment (src/treat.cpp).

#include "core/world/rooms.h"
#include "core/world/rooms_internal.h"

#include "core/audio/sounds.h"
#include "core/ui/charts.h"
#include "core/world/arrivals.h"
#include "core/world/build.h"
#include "core/world/epidemic.h"
#include "core/world/finance.h"
#include "core/world/icons.h"
#include "core/world/queue.h"
#include "core/world/reception.h"
#include "core/world/reputation.h"
#include "core/world/trace.h"

namespace th
{

namespace
{

// Thing.person_flags bits (the game's PF_*) besides core/world/queue.h's.
constexpr uint32_t kPfDiagnosed = 0x20000;

// Illnesses (the game's ILL_*) whose cure changes a patient's looks back.
constexpr uint8_t kIllBloatyHead = 2;

constexpr uint8_t kIllHairyitis = 3;

constexpr uint8_t kIllKingComplex = 4;

constexpr uint8_t kIllInvisibility = 5;

constexpr uint8_t kIllSlackTongue = 7;

constexpr uint8_t kIllAlienDna = 8;

constexpr uint8_t kIllFracturedBones = 9;

constexpr uint8_t kIllBaldness = 10;

// The game's static gbv.diagnosis_complete and gbv.diagnosis_hopeless
// (src/array.cpp; not balance keys).
constexpr int kDiagnosisComplete = 1000;

constexpr int kDiagnosisHopeless = -1000;

} // namespace

void add_treatment_to_history(Thing &patient)
{
    if (patient.treatment == 0)
        return;
    size_t i = 0;
    for (; i < patient.treatments.size(); i++) {
        if (patient.treatments[i] == 0) {
            patient.treatments[i] = patient.treatment;
            break;
        }
    }
    if (i >= patient.treatments.size()) {
        for (i = 0; i < 9; i++)
            patient.treatments[i] = patient.treatments[i + 1];
        patient.treatments[9] = patient.treatment;
    }
}

int16_t diag_register(uint8_t illness, uint8_t treatment)
{
    if (treatment < kFirstDiagnosis || illness >= kDiagRegister.size())
        return 0;
    return kDiagRegister[illness][size_t(treatment - kFirstDiagnosis) % 8];
}

inline namespace rooms_detail
{

// treatment_already_done (0x3d89c).
bool treatment_already_done(const Thing &patient, uint8_t treatment)
{
    for (uint8_t t : patient.treatments) {
        if (t == 0)
            break;
        if (t == treatment)
            return true;
    }
    return false;
}

// best_guess_treatment (0x3e330): the patient's illness taken as its
// diagnosis (PF_GUESSED), wrongly (PF_WRONG_GUESS) with a chance falling as
// the diagnosis progresses (a draw from world.seed).
void best_guess_treatment(World &world, Thing &p)
{
    p.person_flags &= ~kPfNeedsDiagnosisRoom;
    p.person_flags |= kPfGuessed;
    p.known_illness = p.treatment = p.illness;
    p.next_treatment = 0;
    if (kExpertiseData[p.treatment].needs_ward & 1) {
        p.next_treatment = p.treatment;
        p.treatment = kIllDiagWard;
    }
    if (world.cure_available[p.illness] == 0)
        p.person_flags |= kPfAwaitingDecision;
    // A byte, as in the game.
    auto pct = uint8_t(p.diagnosis * 100 / kDiagnosisComplete);
    if (pct > 99)
        pct = 99;
    if (pct < old_newrand(world.seed) % 100)
        p.person_flags |= kPfWrongGuess;
}
} // namespace rooms_detail

namespace
{

// announce_new_illness (0x3dfcc): the illness discovered by hospital 0
// (HospitalSettings illnesses[].discovered), with a call (kind 7, expiring
// after 15 days) the first time, which the advisor hears of
// (kAdvisorEventCall).
void announce_new_illness(World &world, uint8_t illness, uint8_t owner)
{
    if (!world.hospital || owner != 0)
        return;
    uint8_t &discovered = world.hospital->illnesses[illness % kIllnessCount].discovered;
    if (discovered == 0) {
        world.advisor_events |= kAdvisorEventCall;
        make_a_playercall(world, kCallNewIllness, illness, 0xf, owner);
    }
    discovered = 1;
}

// diagnose_patient's search for a diagnosis room: picks one
// (find_possible_diagnosis) or marks that the patient needs one. True when one
// was picked.
bool diagnose_patient_find_room(World &world, uint16_t id)
{
    Thing &p = world.things[id];
    find_possible_diagnosis(world, id);
    if (p.treatment != 0)
        return true;
    p.person_flags |= kPfNeedsDiagnosisRoom;
    reputation_affected(world, p.illness, kRepNoTreatmentFound);
    return false;
}

// diagnose_patient's status icon over `target` (the member of staff, or the
// patient awaiting a decision).
void diagnose_icon(World &world, const AnimData &anims, Thing &target, int8_t icon)
{
    trace_event("diagnose_icon %u icon=%u", unsigned(&target - world.things.data()), unsigned(icon));
    set_icon_value(anims, target, icon);
}

// diagnose_patient's complete diagnosis: the illness is known; its treatment
// becomes the patient's unless the policy wants more diagnosis and a room is
// left for it.
void diagnose_patient_complete(World &world, const AnimData &anims, Thing &staff, uint16_t id, int16_t diag)
{
    Thing &p = world.things[id];
    uint8_t illness = p.illness;
    if (p.known_illness != illness) {
        staff.happiness = uint16_t(staff.happiness + 5);
        announce_new_illness(world, illness, p.owner);
        reputation_affected(world, illness, kRepIllnessFound);
        record_person_as_patient(world, id);
        p.known_illness = illness;
        if (world.market)
            world.market->diagnosed++;
    }
    if (p.contagion)
        alert_epidemic(world, p.illness, p.owner);
    p.person_flags |= kPfDiagnosed;
    if (diag < hospital_of(world).diag_threshold && diagnose_patient_find_room(world, id))
        return;
    diagnose_icon(world, anims, staff, kIconDiagnosed);
    if (world.cure_available[illness] == 0) {
        p.person_flags |= kPfAwaitingDecision | kPfWaitingForRoom;
        p.treatment = p.illness;
        diagnose_icon(world, anims, p, kIconAwaitingDecision);
    } else {
        reputation_affected(world, p.illness, kRepDiagnosed);
        p.treatment = p.illness;
    }
}

// diagnose_patient's guess for a diagnosis that is neither complete nor
// hopeless: a diagnosis room is picked (find_possible_diagnosis) or the
// patient needs one, and the staff member's icon says which.
void diagnose_patient_guess(World &world, const AnimData &anims, Thing &staff, uint16_t id)
{
    Thing &p = world.things[id];
    find_possible_diagnosis(world, id);
    if (p.treatment == 0) {
        diagnose_icon(world, anims, staff, kIconNoDiagnosis);
        p.person_flags |= kPfNeedsDiagnosisRoom;
        reputation_affected(world, p.illness, kRepNoTreatmentFound);
    } else {
        diagnose_icon(world, anims, staff, kIconGuessed);
    }
}

// diagnose_patient's hopeless diagnosis: a draw from world.seed sends the
// patient for an autopsy (when the hospital can do one) or near death.
void diagnose_patient_hopeless(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn,
                               std::source_location caller)
{
    Thing &p = world.things[id];
    uint32_t coin = old_newrand(world.seed, caller) & 1;
    if (coin == 0) {
        if (world.cure_available[kIllAutopsy] == 1)
            p.treatment = kIllAutopsy;
        else
            coin = 1;
    }
    if (coin == 1)
        change_health(world, anims, id, int16_t(-(p.health - 3)), game_turn);
    reputation_affected(world, p.illness, kRepDiagnosisFailed);
    world.mishaps++;
}

// diagnose_patient (0x3d28c): a complete diagnosis makes the illness known
// and its treatment the patient's (awaiting a decision if the hospital
// cannot give it), unless the policy's diag_threshold wants more diagnosis
// and a room is left for it; a hopeless one sends the patient for an
// autopsy or near death (a draw from world.seed); otherwise a diagnosis
// room is picked (find_possible_diagnosis), or the patient needs one. Ward
// illnesses go by the ward. A newly found illness is announced
// (announce_new_illness); the staff member's status icon says what came of
// it and a failed diagnosis counts as a mishap.
void diagnose_patient(World &world, const AnimData &anims, Thing &staff, uint16_t id, uint32_t game_turn)
{
    Thing &p = world.things[id];
    trace_event("diagnose %u staff=%u", unsigned(id), unsigned(&staff - world.things.data()));
    int16_t diag = p.diagnosis;
    if (diag >= kDiagnosisComplete)
        diagnose_patient_complete(world, anims, staff, id, diag);
    else if (diag <= kDiagnosisHopeless)
        diagnose_patient_hopeless(world, anims, id, game_turn, std::source_location::current());
    else
        diagnose_patient_guess(world, anims, staff, id);
    p.next_treatment = 0;
    if (kExpertiseData[p.treatment].needs_ward & 1) {
        p.next_treatment = p.treatment;
        p.treatment = kIllDiagWard;
    }
}

} // namespace

void cure_illness(World &world, uint16_t id)
{
    trace_event("cure %u illness=%u", unsigned(id), unsigned(world.things[id].illness));
    Thing &p = world.things[id];
    switch (p.illness) {
    case kIllBloatyHead:
        p.layers[0] = uint8_t(p.layers[0] - 5);
        break;
    case kIllHairyitis:
        transform_peep_who(p, 2);
        break;
    case kIllKingComplex:
        transform_peep_who(p, 6);
        break;
    case kIllSlackTongue:
        if (p.subtype == 1)
            p.layers[0] = uint8_t(p.layers[0] - 4);
        else
            transform_peep_who(p, 8);
        break;
    case kIllInvisibility:
        transform_peep_who(p, 0xa);
        break;
    case kIllBaldness:
        p.layers[0] = 7;
        break;
    case kIllFracturedBones:
        if (p.subtype == 1) {
            p.layers[2] = 0;
            p.layers[3] = 0;
        } else {
            transform_peep_who(p, 0xe);
        }
        break;
    case kIllAlienDna:
        p.subtype = p.orig_subtype;
        break;
    default:
        break;
    }
    if (p.owner == 0 && world.ledger)
        world.ledger->cures++;
    if (world.hospital && p.owner == 0)
        world.hospital->illnesses[p.illness % kIllnessCount].cured++;
    reputation_affected(world, p.illness, kRepCured);
    p.known_illness = p.illness = 0;
    if (p.contagion)
        clean_contagious_peep(world, id);
    if (p.owner == 0 && world.ledger)
        world.ledger->cures_this_month++;
    if (p.person_flags & kPfEmergency) {
        trace_event("emergency_cure %u", unsigned(id));
        world.emergency.cured++;
        p.person_flags &= ~kPfEmergency;
        reset_icon_value(p, kIconEmergency);
    }
    record_person_as_patient(world, id);
}

namespace
{

// setup_next_treatment_outcome's outcome 2: the patient pays for the
// treatment just given and is cured, or on a wrong guess dies; the curing
// doctor's happiness rises, and both outcomes set the patient's status icon
// and queue the sound (SFX_CHEER / SFX_BOO).
void setup_next_treatment_outcome_cure(World &world, const AnimData &anims, uint16_t staff_id, uint16_t id,
                                       uint32_t game_turn, Thing &p)
{
    pay_for_treatment(world, id, 2);
    p.treatment = 0;
    cure_illness(world, id);
    if (staff_id)
        world.things[staff_id].happiness = uint16_t(world.things[staff_id].happiness + 5);
    p.person_flags &= ~kPfAwaitingDecision;
    reset_icon_value(p, kIconAwaitingDecision);
    if (p.person_flags & kPfWrongGuess) {
        set_icon_value(anims, p, kIconKilled);
        change_health(world, anims, id, -1000, game_turn);
        world.treatment_sounds.push_back(kSfxBoo);
    } else {
        set_icon_value(anims, p, kIconCured);
        change_health(world, anims, id, 1000, game_turn);
        p.person_flags |= kPfCured;
        world.treatment_sounds.push_back(kSfxCheer);
    }
}

// setup_next_treatment's charge for the treatment just given and its outcome
// (kExpertiseData[].outcome): the next treatment, a cure or a diagnosis.
// False when the patient will not pay for what comes next.
bool setup_next_treatment_outcome(World &world, const AnimData &anims, uint16_t staff_id, uint16_t id,
                                  uint32_t game_turn)
{
    Thing &p = world.things[id];
    bool pay = true;
    switch (kExpertiseData[p.treatment].outcome) {
    case 0:
        pay_for_treatment(world, id, 2);
        p.treatment = 0;
        break;
    case 1:
        pay_for_treatment(world, id, 2);
        p.treatment = kIllGeneralPractice;
        pay = shall_peep_pay(world, anims, id);
        if (pay)
            pay_for_treatment(world, id, 3);
        break;
    case 2:
        setup_next_treatment_outcome_cure(world, anims, staff_id, id, game_turn, p);
        break;
    case 4:
        pay_for_treatment(world, id, 2);
        diagnose_patient(world, anims, world.things[staff_id], id, game_turn);
        pay = shall_peep_pay(world, anims, id);
        if (pay)
            pay_for_treatment(world, id, 3);
        break;
    case 5:
        pay_for_treatment(world, id, 2);
        if (p.next_treatment) {
            p.treatment = p.next_treatment;
            p.next_treatment = 0;
        } else {
            p.treatment = kIllGeneralPractice;
        }
        pay = shall_peep_pay(world, anims, id);
        if (pay)
            pay_for_treatment(world, id, 3);
        break;
    default:
        break;
    }
    return pay;
}

// setup_next_treatment for a patient who needs a diagnosis room: by the
// policy, a well enough diagnosed one gets the best guess at a treatment and
// a poorly diagnosed one is sent home.
void setup_next_treatment_guess_or_send_home(World &world, Thing &p)
{
    const HospitalSettings &hs = hospital_of(world);
    // Unsigned, as in the game.
    auto pct = uint32_t(p.diagnosis * 100 / kDiagnosisComplete);
    if (uint32_t(hs.guess_cure_threshold) <= pct && hs.illnesses[p.illness % kIllnessCount].discovered == 1 &&
        pct > 0) {
        best_guess_treatment(world, p);
    } else if (uint32_t(hs.send_home_threshold) >= pct) {
        p.person_flags = 0;
        p.treatment = 0;
        turned_away(world, p);
        p.person_flags |= kPfNoDiagnosis;
    }
}

} // namespace

void setup_next_treatment(World &world, const AnimData &anims, uint16_t staff_id, uint16_t id, uint32_t game_turn)
{
    trace_event("next_treatment %u staff=%u", unsigned(id), unsigned(staff_id));
    Thing &p = world.things[id];
    p.impatience = 0;
    p.waiting_time = 0;
    reputation_affected(world, p.treatment, kRepNextTreatment);
    if (!setup_next_treatment_outcome(world, anims, staff_id, id, game_turn)) {
        p.treatment = 0;
        pay_for_treatment(world, id, 2);
    }
    if (p.person_flags & kPfNeedsDiagnosisRoom)
        setup_next_treatment_guess_or_send_home(world, p);
    next_treatment_now_setup(world, anims, id, true, game_turn);
}

} // namespace th
