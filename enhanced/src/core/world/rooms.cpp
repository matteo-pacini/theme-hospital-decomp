#include "core/world/rooms.h"

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
#include "core/world/nav.h"
#include "core/world/patient_tables.h"
#include "core/world/popups.h"
#include "core/world/queue.h"
#include "core/world/reaper.h"
#include "core/world/reception.h"
#include "core/world/reputation.h"
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
constexpr uint8_t kRoomWard = 9;
constexpr uint8_t kRoomTheatre = 10;
constexpr uint8_t kRoomResearch = 28;

// RoomInfo.state values (RS_*) of rooms staff answer pages to: open, in
// use; and of a room being edited.
constexpr int8_t kRoomStateOpen = 0;
constexpr int8_t kRoomStateInUse = 1;
constexpr int8_t kRoomEdited = 4;

// Illnesses past the cures: the diagnosis rooms' treatments start here.
constexpr uint8_t kFirstDiagnosis = 0x24;

// Thing.person_flags bits (the game's PF_*) besides core/world/queue.h's.
constexpr uint32_t kPfWrongGuess = 0x800;
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

// The complexes that walk a patient out of its room (P_COMP.LEX), or just
// end what it does there.
constexpr uint16_t kComplexLeaveRoom = 90;
constexpr uint16_t kComplexStandStill = 240;

// The player call kinds about patients: a missing treatment room (4) and an
// unfinished diagnosis (8).
constexpr uint16_t kCallNoTreatment = 4;
constexpr uint16_t kCallNoDiagnosis = 8;
// The call announcing an illness discovered.
constexpr uint16_t kCallNewIllness = 7;

// allhosps.insurance_accepted, which init_player sets to all three
// companies and nothing changes.
constexpr uint8_t kInsuranceAccepted = 7;

// insurance_co[].payment_delay (src/tables.cpp): months before an insurance
// company pays.
constexpr std::array<uint8_t, 9> kPaymentDelay = {0, 3, 6, 8, 2, 9, 10, 4, 3};

// The game's special_around (src/tables.cpp), first five: a cell and its
// four neighbours, as cell offsets.
constexpr std::array<int16_t, 5> kSpecialAround = {0, 1, 128, -1, -128};

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

// The game's diag_register (src/tables.cpp): per illness, the diagnosis
// progress each diagnosis room (from treatment 0x24 on) adds; a room that
// takes some away also costs 5 health.
constexpr std::array<std::array<int16_t, 8>, 36> kDiagRegister = {{
    {},
    {},
    {400, 0, 0, 0, 0, 0, 200},
    {200, 0, 0, 0, 0, 0, 400},
    {0, 0, 0, 0, 0, 0, 200, 400},
    {0, 400, 0, -200},
    {-200, 400, 0, -400},
    {400, 0, 200, 0, 0, 0, 0, -200},
    {200, 400, 0, 200, 200, 0, 0, -400},
    {0, 0, 200, 400},
    {0, 400, 0, 0, 0, 0, 200, -200},
    {0, 400},
    {0, 0, 0, 0, 0, 400},
    {400, 0, 0, 0, 0, 0, 200},
    {},
    {0, 0, 0, 0, 0, 0, 400},
    {0, 0, 0, 0, 0, 400},
    {0, 0, 0, 0, 400},
    {0, 0, 0, 400},
    {0, 0, 0, 200, 400},
    {0, 0, 400, 0, 200},
    {400, 0, 0, 0, 200},
    {0, 0, 0, 0, 0, 0, 0, 400},
    {0, 200, 0, 0, 0, 0, 0, 400},
    {0, 0, 400},
    {400, 0, 0, 0, 0, 0, 200},
    {0, 0, 0, 0, 0, 0, 0, 400},
    {0, 400},
    {0, 0, 0, 0, 0, 0, 400},
    {0, 0, 0, 0, 0, 400, 200},
    {200, 0, 0, 400},
    {0, 0, -400, 0, 0, 0, 0, 400},
    {0, 0, 0, 0, 0, 0, 400},
    {0, 0, 0, 0, 400},
    {0, 0, 0, 200, 400},
    {400},
}};

uint16_t cell_of(const Thing &t)
{
    return uint16_t(t.x.tile + (t.y.tile << 7));
}

// The player's settings, or the game's starting ones before
// start_rooms_in_use.
const HospitalSettings &hospital_of(const World &world)
{
    static const HospitalSettings statics;
    return world.hospital ? *world.hospital : statics;
}

const Balance &balance_of(const World &world)
{
    static const Balance statics;
    return world.settings ? *world.settings : statics;
}

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

// listhead[owner].objects[kind] != 0: only hospital 0's are listed.
bool has_object(const World &world, uint8_t owner, uint8_t kind)
{
    return owner == 0 && world.first_of_kind[kind] != 0;
}

// set_expertise_ward (0x4aae4). Member of staff `hired`, if any, is not
// counted yet: set_expertise_staff runs before the game counts their skills.
void set_expertise_ward(World &world, uint8_t owner, uint16_t hired = 0)
{
    for (size_t i = 1; i < 0x2f; i++) {
        const ExpertiseInfo &exp = kExpertiseData[i];
        if (world.cure_available[i] != 0 || !exp.needs_ward || owner != 0 || !world.room_of_type[exp.category])
            continue;
        if ((exp.staff_mask & kSkillSurgeon) && surgeon_count(world, owner) < 2)
            continue;
        int j = 0;
        for (; j < 9; j++)
            if ((exp.staff_mask & (1 << j)) &&
                staff_with_skill(world, owner, j) - (hired && (world.things[hired].skills & (1 << j))) == 0)
                break;
        if (j >= 9)
            world.cure_available[i] = 1;
    }
}

// remove_expertise_ward (0x4ac4c).
void remove_expertise_ward(World &world)
{
    for (size_t i = 1; i < 0x2f; i++)
        if (world.cure_available[i] == 1 && kExpertiseData[i].needs_ward)
            world.cure_available[i] = 0;
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

namespace
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

namespace
{

const PersonComplexes &person_complexes_of(const World &world)
{
    static const PersonComplexes none;
    return world.person_complexes ? *world.person_complexes : none;
}

} // namespace

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

// set_expertise_room: whether hospital `owner` employs staff with every skill
// in `mask` (two surgeons for surgery).
bool set_expertise_room_has_staff(const World &world, uint8_t owner, uint16_t mask)
{
    for (int j = 0; j < 9; j++) {
        if (((1 << j) & mask) == kSkillSurgeon && surgeon_count(world, owner) < 2)
            return false;
        if ((mask & (1 << j)) && staff_with_skill(world, owner, j) == 0)
            return false;
    }
    return true;
}

} // namespace

void set_expertise_room(World &world, uint16_t room_id)
{
    const RoomInfo &room = world.rooms[room_id];
    // Only hospital 0's treatments are kept (World::cure_available).
    if (room.owner != 0)
        return;
    bool ward = world.cure_available[kIllDiagWard] == 1;
    for (size_t i = 1; i < 0x2f; i++) {
        if (world.cure_available[i] != 0)
            continue;
        const ExpertiseInfo &exp = kExpertiseData[i];
        if (exp.needs_ward && !ward)
            continue;
        if (exp.category != room.type)
            continue;
        uint16_t mask = exp.staff_mask;
        if (mask == 0) {
            if (room.type != kRoomResearch)
                world.cure_available[i] = 1;
            continue;
        }
        if (set_expertise_room_has_staff(world, room.owner, mask))
            world.cure_available[i] = 1;
    }
    if (!ward && world.cure_available[kIllDiagWard] == 1)
        set_expertise_ward(world, room.owner);
}

void set_expertise_staff(World &world, uint16_t id)
{
    const Thing &staff = world.things[id];
    if (staff.owner != 0) {
        if (staff.owner < world.rivals.size())
            set_expertise_staff(world.rivals[staff.owner], staff);
        return;
    }
    bool ward = world.cure_available[kIllDiagWard] == 1;
    for (size_t i = 1; i < 0x2f; i++) {
        if (world.cure_available[i] != 0)
            continue;
        const ExpertiseInfo &exp = kExpertiseData[i];
        if (!world.room_of_type[exp.category])
            continue;
        if ((exp.staff_mask & kSkillSurgeon) && surgeon_count(world, staff.owner) < 2)
            continue;
        if (!(staff.skills & exp.staff_mask))
            continue;
        if (exp.needs_ward && !ward)
            continue;
        if (exp.kind == 0 || has_object(world, staff.owner, exp.kind))
            world.cure_available[i] = 1;
    }
    if (!ward && world.cure_available[kIllDiagWard] == 1)
        set_expertise_ward(world, staff.owner, id);
}

void remove_expertise_staff(World &world, uint16_t id)
{
    const Thing &staff = world.things[id];
    if (staff.owner != 0)
        return;
    bool ward = world.cure_available[kIllDiagWard] == 1;
    for (int i = 0; i < 9; i++) {
        if (!(staff.skills & (1 << i)))
            continue;
        // The game has taken their skills off its counts (remove_skill) but
        // not yet the surgeon off allhosps.surgeon_count.
        if (staff_with_skill(world, staff.owner, i) - 1 != 0 && !(i == 5 && surgeon_count(world, staff.owner) < 3))
            continue;
        for (size_t j = 1; j < 0x2f; j++)
            if (world.cure_available[j] == 1 && ((1 << i) & kExpertiseData[j].staff_mask))
                world.cure_available[j] = 0;
    }
    if (ward && world.cure_available[kIllDiagWard] != 1)
        remove_expertise_ward(world);
}

void remove_expertise_room(World &world, uint16_t room_id)
{
    const RoomInfo &room = world.rooms[room_id];
    if (room.owner != 0)
        return;
    bool ward = world.cure_available[kIllDiagWard] == 1;
    for (uint16_t r = 1; r < kRoomCount; r++)
        if (world.rooms[r].owner == room.owner && world.rooms[r].type == room.type && r != room_id)
            return;
    for (size_t i = 1; i < 0x2f; i++)
        if (world.cure_available[i] == 1 && kExpertiseData[i].category == room.type)
            world.cure_available[i] = 0;
    if (ward && world.cure_available[kIllDiagWard] != 1)
        remove_expertise_ward(world);
}

void room_deleted_update_listhead(World &world, uint16_t room_id)
{
    const RoomInfo &room = world.rooms[room_id];
    uint16_t id = 0;
    for (uint16_t r = 1; r < kRoomCount; r++) {
        if (world.rooms[r].type == room.type && r != room_id && world.rooms[r].owner == room.owner) {
            id = r;
            break;
        }
    }
    if (room.owner == 0)
        world.room_of_type[room.type % world.room_of_type.size()] = id;
}

namespace
{

// page_member_of_staff: counts the staff with a skill in `skill_mask` already
// in the room off `wanted`. True when no more are wanted.
bool page_member_of_staff_count_present(const World &world, uint16_t skill_mask, uint16_t room_id, int8_t &wanted)
{
    for (const Thing &t : world.things) {
        if (t.type == kThingStaff && t.room == room_id && (t.skills & skill_mask)) {
            if (--wanted <= 0)
                return true;
        }
    }
    return false;
}

} // namespace

bool page_member_of_staff(World &world, uint16_t skill_mask, uint16_t room_id, bool count_present, int8_t wanted,
                          uint8_t owner)
{
    trace_event("page room=%u mask=%x wanted=%d", unsigned(room_id), unsigned(skill_mask), int(wanted));
    if (world.rooms[room_id].type == kRoomReception)
        return false;
    int count = 0;
    uint16_t bit = 0;
    for (int i = 0; i < 9; i++) {
        if (skill_mask & (1 << i)) {
            bit = uint16_t(1 << i);
            count++;
        }
    }
    if (count > 1) {
        page_member_of_staff(world, uint16_t(~bit & skill_mask), room_id, count_present, wanted, owner);
        skill_mask = bit;
    }
    if (count_present && page_member_of_staff_count_present(world, skill_mask, room_id, wanted))
        return true;
    World::Pager *free_pager = nullptr;
    for (World::Pager &pager : world.pagers) {
        if ((pager.mask & skill_mask) && pager.room == room_id) {
            if (--wanted <= 0)
                return true;
        } else if (pager.mask == 0) {
            free_pager = &pager;
        }
    }
    if (free_pager) {
        *free_pager = {skill_mask, room_id, 0, 0, owner};
        if (wanted > 1)
            return page_member_of_staff(world, skill_mask, room_id, count_present, int8_t(wanted - 1), owner);
    }
    return false;
}

void page_staff_i_need(World &world, uint16_t person, uint16_t room)
{
    const Thing &p = world.things[person];
    int8_t count = world.rooms[room].type == kRoomTheatre ? 2 : 1;
    page_member_of_staff(world, kExpertiseData[p.treatment].staff_mask, room, true, count, p.owner);
}

uint16_t check_pager(World &world, uint16_t id, uint32_t game_turn)
{
    const Thing &staff = world.things[id];
    World::Pager *found = nullptr;
    uint16_t best = 999;
    for (World::Pager &pager : world.pagers) {
        if ((pager.owner == staff.owner && (pager.mask & staff.skills) && pager.claimed_until < game_turn) ||
            id == pager.responder) {
            const RoomInfo &room = world.rooms[pager.room];
            if (!(room.state == kRoomStateInUse || room.state == kRoomStateOpen))
                continue;
            auto dist = uint16_t(std::abs(staff.x.tile - room.door_tile % kMapSize) +
                                 std::abs(staff.y.tile - room.door_tile / kMapSize));
            if (best > dist) {
                best = dist;
                found = &pager;
            }
        }
    }
    uint16_t room_id = 0;
    if (found) {
        room_id = found->room;
        found->claimed_until = game_turn + 3000;
        found->responder = id;
    }
    if (room_id > 0 && world.rooms[room_id].type == kRoomFree) {
        for (World::Pager &pager : world.pagers)
            if (pager.room == room_id)
                pager = {};
        room_id = 0;
    }
    if (room_id)
        trace_event("check_pager %u room=%u", unsigned(id), unsigned(room_id));
    return room_id;
}

void pager_arrival(World &world, uint16_t id)
{
    trace_event("pager_arrival %u", unsigned(id));
    const Thing &staff = world.things[id];
    World::Pager *other = nullptr;
    World::Pager *blank = nullptr;
    for (World::Pager &pager : world.pagers) {
        if ((pager.mask & staff.skills) && staff.room == pager.room && pager.owner == staff.owner) {
            if (id == pager.responder) {
                pager = {};
                return;
            }
            if (pager.responder)
                other = &pager;
            else
                blank = &pager;
        }
    }
    if (other)
        *other = {};
    else if (blank)
        *blank = {};
}

void clear_pager_responses(World &world, uint16_t room)
{
    trace_event("clear_pagers room=%u", unsigned(room));
    for (World::Pager &pager : world.pagers) {
        if (pager.room == room) {
            pager.claimed_until = 0;
            pager.responder = 0;
        }
    }
}

void staff_abandon_pager_response(World &world, uint16_t id)
{
    trace_event("abandon_pager %u", unsigned(id));
    for (World::Pager &pager : world.pagers) {
        if (pager.responder == id) {
            pager.claimed_until = 0;
            pager.responder = 0;
        }
    }
}

uint16_t make_a_playercall(World &world, uint16_t type, uint16_t subject, int16_t timeout, uint8_t owner)
{
    // The rat game has no calls.
    if (world.rats.fest_level)
        return 0;
    for (size_t i = 1; i < world.calls.size(); i++) {
        World::PlayerCall &call = world.calls[i];
        if (call.type == 0) {
            // The panel slot and slide step are left as they were until
            // add_panel_popup sets them, as in the game.
            call.type = type;
            call.id = world.next_call_id++;
            call.subject = subject;
            call.timer = timeout;
            call.owner = owner;
            add_panel_popup(world, uint16_t(i));
            uint16_t &head = world.first_call[owner & 3][type % 9];
            if (head == 0)
                head = uint16_t(i);
            trace_event("playercall %u type=%u subject=%u owner=%u", unsigned(call.id), unsigned(type),
                        unsigned(subject), unsigned(owner));
            return call.id;
        }
    }
    return 0;
}

void expire_playercalls(World &world)
{
    bool changed = false;
    for (size_t i = 1; i < world.calls.size(); i++) {
        World::PlayerCall &call = world.calls[i];
        if (!call.timer || --call.timer > 0)
            continue;
        trace_event("expire_call %u type=%u subject=%u", unsigned(call.id), unsigned(call.type),
                    unsigned(call.subject));
        if (call.type == kCallEmergencyOver && call.subject == 1 && call.owner == 0)
            world.emergency.state = 0;
        else if (call.type == kCallVip && call.owner == 0) {
            world.mayor.arrival_turn = (world.game ? world.game->turn_count : 0) + 1500;
            world.mayor.state = 2;
        } else if (call.type == kCallEpidemic && call.owner == 0) {
            world.epidemic.cover_up_expired = true;
        }
        uint16_t &head = world.first_call[call.owner & 3][call.type % 9];
        if (head == i) {
            head = 0;
            changed = true;
        }
        delete_panel_popup(world, uint16_t(i));
        call.type = 0;
    }
    if (!changed)
        return;
    // The heads lost go to the first call of their kind in the table.
    for (size_t i = 1; i < world.calls.size(); i++) {
        const World::PlayerCall &call = world.calls[i];
        uint16_t &head = world.first_call[call.owner & 3][call.type % 9];
        if (call.type && head == 0)
            head = uint16_t(i);
    }
}

uint16_t playercall_subject(const World &world, uint16_t type, uint16_t id, uint8_t owner)
{
    for (size_t i = 1; i < world.calls.size(); i++) {
        const World::PlayerCall &call = world.calls[i];
        if (call.type == type && call.owner == owner && (id == 0 || id == call.id))
            return call.subject;
    }
    return 0;
}

uint16_t get_playercall(World &world, uint16_t type, uint16_t id, uint8_t owner)
{
    trace_event("get_playercall %u type=%u owner=%u", unsigned(id), unsigned(type), unsigned(owner));
    for (size_t i = 1; i < world.calls.size(); i++) {
        World::PlayerCall &call = world.calls[i];
        if (call.type != type || call.owner != owner)
            continue;
        if (id == 0 || id == call.id) {
            uint16_t subject = call.subject;
            call.type = 0;
            delete_panel_popup(world, uint16_t(i));
            uint16_t &head = world.first_call[owner & 3][type % 9];
            if (head == i) {
                head = 0;
                get_new_popup_head(world, type, owner);
            }
            return subject;
        }
    }
    return 0;
}

uint16_t check_playercall(const World &world, uint16_t type, uint16_t subject, uint8_t owner)
{
    uint16_t count = 0;
    for (size_t i = 1; i < world.calls.size(); i++) {
        const World::PlayerCall &call = world.calls[i];
        if (call.type != type || call.owner != owner)
            continue;
        if (type == 2 || type == 3 || type == 4 || type == 8 || call.subject == subject)
            count++;
    }
    return count;
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

bool thingpage_request(World::ThingPager &pager, uint16_t id, uint8_t priority)
{
    if (pager.count == pager.pages.size())
        return false;
    for (int i = pager.count - 1; i >= 0; i--) {
        World::ThingPage &page = pager.pages[size_t(i)];
        if (page.id == id) {
            if (page.priority < priority)
                page.priority = priority;
            return true;
        }
    }
    pager.pages[pager.count++] = {id, priority, 0, 0};
    return true;
}

bool thingpage_cancel(World::ThingPager &pager, uint16_t id)
{
    for (int i = pager.count - 1; i >= 0; i--) {
        if (pager.pages[size_t(i)].id == id) {
            pager.pages[size_t(i)] = pager.pages[pager.count - 1u];
            pager.count--;
            return true;
        }
    }
    return false;
}

uint16_t thingpage_respond(World &world, World::ThingPager &pager, uint16_t handyman, uint16_t room, uint8_t min,
                           uint32_t game_turn)
{
    const Thing &h = world.things[handyman];
    int best_priority = -1;
    int best = -1;
    uint32_t best_dist = 0x4000;
    auto dist = [&](const Thing &t) {
        int dx = cell_of(h) % kMapSize - cell_of(t) % kMapSize;
        int dy = cell_of(h) / kMapSize - cell_of(t) / kMapSize;
        return uint32_t(dx * dx + dy * dy);
    };
    for (int i = pager.count - 1; i >= 0; i--) {
        const World::ThingPage &page = pager.pages[size_t(i)];
        const Thing &t = world.things[page.id];
        if (room && t.room_id != room)
            continue;
        if (page.responder != handyman && page.claimed_until > game_turn)
            continue;
        if (page.priority < min)
            continue;
        if (t.room_id != 0) {
            const RoomInfo &r = world.rooms[t.room_id];
            if (r.being_edited || r.state == kRoomEdited || (h.owner == 0 && world.building.build.room == t.room_id))
                continue;
        }
        if (page.priority == best_priority) {
            if (dist(t) < best_dist) {
                best = i;
                best_dist = dist(t);
            }
        } else if (page.priority > best_priority) {
            best = i;
            best_dist = dist(t);
            best_priority = page.priority;
        }
    }
    if (best == -1)
        return 0;
    World::ThingPage &page = pager.pages[size_t(best)];
    page.responder = handyman;
    page.claimed_until = game_turn + 3000;
    return page.id;
}

uint8_t thingpage_find_highest_priority(const World::ThingPager &pager)
{
    int best = -1;
    for (int i = pager.count - 1; i >= 0; i--)
        best = std::max(best, int(pager.pages[size_t(i)].priority));
    return uint8_t(best);
}

uint16_t thingpage_remind_me(const World::ThingPager &pager, uint16_t handyman)
{
    for (int i = pager.count - 1; i >= 0; i--)
        if (pager.pages[size_t(i)].responder == handyman)
            return pager.pages[size_t(i)].id;
    return 0;
}

void thingpage_cancel_my_response(World::ThingPager &pager, uint16_t handyman)
{
    for (int i = pager.count - 1; i >= 0; i--) {
        World::ThingPage &page = pager.pages[size_t(i)];
        if (page.responder == handyman) {
            page.responder = 0;
            page.claimed_until = 0;
        }
    }
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

namespace
{

// EV_CHOICE_RESULT kChoiceTreatResearch and EV_PATIENT_TO_RESEARCH: the
// patient goes for an autopsy.
void send_for_autopsy(Thing &p)
{
    p.treatment = kIllAutopsy;
    p.person_flags &= ~kPfAwaitingDecision;
    p.state = kPsGoForTreatment;
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
        p.person_flags &= ~kPfAwaitingDecision;
        p.person_flags |= kPfWaitingForRoom;
        p.timer = int16_t(game.turn_count);
        break;
    case kChoiceTreatResearch:
        send_for_autopsy(p);
        break;
    case kChoiceDiagGuess:
        p.person_flags &= ~kPfAwaitingDecision;
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
