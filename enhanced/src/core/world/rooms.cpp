#include "core/world/rooms.h"

#include "core/ui/charts.h"
#include "core/world/arrivals.h"
#include "core/world/build.h"
#include "core/world/complex.h"
#include "core/world/complex_tables.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/nav.h"
#include "core/world/queue.h"
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
constexpr uint32_t kPfGuessed = 0x1000;
constexpr uint32_t kPfCured = 0x2000;
constexpr uint32_t kPfDiagnosed = 0x20000;
constexpr uint32_t kPfContagious = 0x20;

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

// change_health (0x3cb5c), without its status icons, the health history and
// dying.
void change_health(Thing &p, int16_t amount)
{
    int health = p.health;
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

// add_treatment_to_history (0x3d7c0).
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
// after 15 days) the first time. The advisor's word on it is not ported.
void announce_new_illness(World &world, uint8_t illness, uint8_t owner)
{
    if (!world.hospital || owner != 0)
        return;
    uint8_t &discovered = world.hospital->illnesses[illness % kIllnessCount].discovered;
    if (discovered == 0)
        make_a_playercall(world, kCallNewIllness, illness, 0xf, owner);
    discovered = 1;
}

// diagnose_patient (0x3d28c): a complete diagnosis makes the illness known
// and its treatment the patient's (awaiting a decision if the hospital
// cannot give it), unless the policy's diag_threshold wants more diagnosis
// and a room is left for it; a hopeless one sends the patient for an
// autopsy or near death (a draw from world.seed); otherwise a diagnosis
// room is picked (find_possible_diagnosis), or the patient needs one. Ward
// illnesses go by the ward. A newly found illness is announced
// (announce_new_illness). The status icons, epidemics and the diagnosed and
// mishap counts are not ported.
void diagnose_patient(World &world, Thing &staff, Thing &p, uint16_t id)
{
    trace_event("diagnose %u staff=%u", unsigned(id), unsigned(&staff - world.things.data()));
    bool guess = true;
    int16_t diag = p.diagnosis;
    if (diag >= kDiagnosisComplete) {
        guess = false;
        uint8_t illness = p.illness;
        if (p.known_illness != illness) {
            staff.happiness = uint16_t(staff.happiness + 5);
            announce_new_illness(world, illness, p.owner);
            reputation_affected(world, illness, kRepIllnessFound);
            record_person_as_patient(world, id);
            p.known_illness = illness;
        }
        p.person_flags |= kPfDiagnosed;
        bool ok = true;
        if (diag < hospital_of(world).diag_threshold) {
            find_possible_diagnosis(world, id);
            if (p.treatment == 0) {
                p.person_flags |= kPfNeedsDiagnosisRoom;
                reputation_affected(world, p.illness, kRepNoTreatmentFound);
            } else {
                ok = false;
            }
        }
        if (ok) {
            if (world.cure_available[illness] == 0) {
                p.person_flags |= kPfAwaitingDecision | kPfWaitingForRoom;
                p.treatment = p.illness;
            } else {
                reputation_affected(world, p.illness, kRepDiagnosed);
                p.treatment = p.illness;
            }
        }
    } else if (diag <= kDiagnosisHopeless) {
        uint32_t coin = old_newrand(world.seed) & 1;
        if (coin == 0) {
            if (world.cure_available[kIllAutopsy] == 1)
                p.treatment = kIllAutopsy;
            else
                coin = 1;
        }
        if (coin == 1)
            change_health(p, int16_t(-(p.health - 3)));
        reputation_affected(world, p.illness, kRepDiagnosisFailed);
        guess = false;
    }
    if (guess) {
        find_possible_diagnosis(world, id);
        if (p.treatment == 0) {
            p.person_flags |= kPfNeedsDiagnosisRoom;
            reputation_affected(world, p.illness, kRepNoTreatmentFound);
        }
    }
    p.next_treatment = 0;
    if (kExpertiseData[p.treatment].needs_ward & 1) {
        p.next_treatment = p.treatment;
        p.treatment = kIllDiagWard;
    }
}

// effect_illness (0x54444) with cured 1: the patient's looks back to
// health, its cure counted (hospital 0's illnesses[].cured), the
// reputation for it, its illness gone, contagion cleared
// (clean_contagious_peep, without the vaccination queue), no longer an
// emergency patient, and recorded (record_person_as_patient). The
// hospital's total cures, the month's, the emergency's and the advisor's
// word are not kept.
void cure_illness(World &world, uint16_t id)
{
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
    if (world.hospital && p.owner == 0)
        world.hospital->illnesses[p.illness % kIllnessCount].cured++;
    reputation_affected(world, p.illness, kRepCured);
    p.known_illness = p.illness = 0;
    if (p.contagion) {
        p.person_flags &= ~kPfContagious;
        p.contagion = 0;
    }
    if (p.person_flags & kPfEmergency)
        p.person_flags &= ~kPfEmergency;
    record_person_as_patient(world, id);
}

} // namespace

void setup_next_treatment(World &world, const AnimData &anims, uint16_t staff_id, uint16_t id, uint32_t game_turn)
{
    trace_event("next_treatment %u staff=%u", unsigned(id), unsigned(staff_id));
    Thing &p = world.things[id];
    bool pay = true;
    p.impatience = 0;
    p.waiting_time = 0;
    reputation_affected(world, p.treatment, kRepNextTreatment);
    switch (kExpertiseData[p.treatment].outcome) {
    case 0:
        pay_for_treatment(world, id, 2);
        p.treatment = 0;
        break;
    case 1:
        pay_for_treatment(world, id, 2);
        p.treatment = kIllGeneralPractice;
        pay = shall_peep_pay(world, id);
        if (pay)
            pay_for_treatment(world, id, 3);
        break;
    case 2:
        pay_for_treatment(world, id, 2);
        p.treatment = 0;
        cure_illness(world, id);
        if (staff_id)
            world.things[staff_id].happiness = uint16_t(world.things[staff_id].happiness + 5);
        p.person_flags &= ~kPfAwaitingDecision;
        if (p.person_flags & kPfWrongGuess) {
            change_health(p, -1000);
        } else {
            change_health(p, 1000);
            p.person_flags |= kPfCured;
        }
        break;
    case 4:
        pay_for_treatment(world, id, 2);
        diagnose_patient(world, world.things[staff_id], p, id);
        pay = shall_peep_pay(world, id);
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
        pay = shall_peep_pay(world, id);
        if (pay)
            pay_for_treatment(world, id, 3);
        break;
    default:
        break;
    }
    if (!pay) {
        p.treatment = 0;
        pay_for_treatment(world, id, 2);
    }
    if (p.person_flags & kPfNeedsDiagnosisRoom) {
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
    next_treatment_now_setup(world, anims, id, true, game_turn);
}

namespace
{

const PersonComplexes &person_complexes_of(const World &world)
{
    static const PersonComplexes none;
    return world.person_complexes ? *world.person_complexes : none;
}

// modify_machine (0x38f14): from 75% of its strength a worn machine puffs
// smoke (a draw from world.seed picks where), from 100% it may break down,
// surely by 150%. Its warning sounds and the smoke's overlay frame are not
// ported, nor the breakdown (machine_disaster): only its draw is made.
void modify_machine(World &world, Thing &machine)
{
    if (machine.strength * 75 / 100 <= machine.times_used) {
        int count = 0;
        for (uint8_t puff : machine.smoke)
            count += puff == 1;
        if (machine.strength * 85 / 100 <= machine.times_used && count != 0) {
            if (machine.strength * 95 / 100 <= machine.times_used) {
                machine.smoke = {1, 1, 1};
            } else if (count == 1) {
                size_t j = 0;
                while (j < 3 && machine.smoke[j] != 1)
                    j++;
                j = (j + 1 + (old_newrand(world.seed) & 1)) % 3;
                machine.smoke[j] = 1;
            }
        } else {
            machine.smoke[old_newrand(world.seed) % 3] = 1;
        }
    }
    if (machine.times_used >= machine.strength) {
        int32_t i = machine.strength * 3 / 2 - machine.times_used + 1;
        // The game's machine_disaster follows a 0, or at once if i <= 1.
        if (i > 1)
            old_newrand(world.seed);
    }
}

} // namespace

void start_rooms_in_use(World &world, HospitalSettings &hospital, Finance &ledger, int &balance, const Date &date,
                        const PersonComplexes &person_complexes, const StaffComplexes &staff_complexes)
{
    world.pagers = {};
    world.room_of_type = {};
    world.calls = {};
    world.next_call_id = 1;
    world.repair_pager = {};
    world.hospital = &hospital;
    world.ledger = &ledger;
    world.balance = &balance;
    world.date = &date;
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
        int j = 0;
        for (; j < 9; j++) {
            if (((1 << j) & mask) == kSkillSurgeon && surgeon_count(world, room.owner) < 2)
                break;
            if ((mask & (1 << j)) && staff_with_skill(world, room.owner, j) == 0)
                break;
        }
        if (j >= 9)
            world.cure_available[i] = 1;
    }
    if (!ward && world.cure_available[kIllDiagWard] == 1)
        set_expertise_ward(world, room.owner);
}

void set_expertise_staff(World &world, uint16_t id)
{
    const Thing &staff = world.things[id];
    if (staff.owner != 0)
        return;
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
    if (count_present) {
        for (const Thing &t : world.things) {
            if (t.type == kThingStaff && t.room == room_id && (t.skills & skill_mask)) {
                if (--wanted <= 0)
                    return true;
            }
        }
    }
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
    for (size_t i = 1; i < world.calls.size(); i++) {
        World::PlayerCall &call = world.calls[i];
        if (call.type == 0) {
            call = {type, subject, world.next_call_id++, owner, timeout};
            trace_event("playercall %u type=%u subject=%u owner=%u", unsigned(call.id), unsigned(type),
                        unsigned(subject), unsigned(owner));
            return call.id;
        }
    }
    return 0;
}

void expire_playercalls(World &world)
{
    for (size_t i = 1; i < world.calls.size(); i++) {
        World::PlayerCall &call = world.calls[i];
        if (call.timer && --call.timer <= 0)
            call = {};
    }
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
            call = {};
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

uint16_t find_object(World &world, uint16_t tile, uint8_t kind, bool random, bool free_only, uint8_t owner)
{
    std::array<uint16_t, 30> found{};
    uint8_t count = 0;
    if (uint16_t room = room_at(world, tile)) {
        for (uint16_t o = world.rooms[room].first_object; o; o = world.things[o].room_obj_next) {
            const Thing &obj = world.things[o];
            if (obj.subtype != kind || (free_only && obj.in_use != 0))
                continue;
            if (!random || count >= found.size())
                return o;
            found[count++] = o;
        }
    } else if (owner == 0) {
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
            if (!random || count >= found.size())
                return o;
            found[count++] = o;
        }
    }
    return count ? found[old_newrand(world.seed) % count] : 0;
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
    uint8_t ins = p.insurance;
    if (ins & kInsuranceAccepted) {
        for (size_t i = 0; i < 3; i++) {
            if (!(ins & (1 << i)))
                continue;
            if (world.hospital && world.date && p.owner == 0) {
                int month = world.date->month + kPaymentDelay[world.ins_control[i] % kPaymentDelay.size()];
                if (month >= 12)
                    month -= 12;
                if (month >= 12)
                    month = 0;
                world.hospital->insurance_claims[i][size_t(month)] += fee;
            }
            create_numfx(world, kNumfxMoney, uint16_t(p.x.tile + (p.y.tile << 7)), kColourRed, 0x40);
            break;
        }
        return true;
    }
    if (fee > p.cash)
        return false;
    p.cash -= fee;
    if (world.ledger && world.balance && world.date && p.owner == 0)
        change_balance(*world.ledger, *world.balance, world.date->day, world.date->month, kTxTreat, fee, p.treatment);
    create_numfx(world, kNumfxMoney, uint16_t(p.x.tile + (p.y.tile << 7)), kColourBlack, 0x40);
    return true;
}

void done_treatment(World &world, const AnimData &anims, uint16_t staff, uint16_t patient, uint32_t game_turn)
{
    trace_event("done_treatment %u staff=%u", unsigned(patient), unsigned(staff));

    Thing &p = world.things[patient];
    if (p.room != 0 && world.rooms[p.room].type == kRoomDestroyed)
        return;
    affect_diagnosis(world, world.things[staff], p);
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

void affect_diagnosis(World &world, Thing &staff, Thing &patient)
{
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
            change_health(patient, -5);
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

bool treatment_machine_used(World &world, uint8_t illness, uint16_t tile)
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
        modify_machine(world, machine);
    return false;
}

uint16_t find_possible_diagnosis(World &world, uint16_t id)
{
    Thing &p = world.things[id];
    const PossibleDiag &diag = kPossibleDiagnoses[kExpertiseData[p.illness].diag_set % kPossibleDiagnoses.size()];
    uint16_t result = 0;
    p.treatment = 0;
    if (diag.count == 0)
        return 0;
    std::array<uint8_t, 9> list{};
    uint16_t count = 0;
    for (size_t i = 0; i < diag.count; i++) {
        uint8_t treatment = diag.treatments[i];
        if (world.cure_available[treatment] == 1)
            list[count++] = treatment;
    }
    if (count == 0)
        return 0;
    for (int i = 0; i < 9; i++) {
        uint16_t pick = uint16_t(old_newrand(world.seed) % count);
        uint8_t treatment = list[pick];
        if (world.cure_available[treatment] != 0) {
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

} // namespace th
