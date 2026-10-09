#pragma once

#include "core/world/world.h"

#include <cstdint>

namespace th
{

// The hospital's reputation, as the game's src/sim.cpp: per illness and as a
// whole, the running totals and the 0-1000 reputation evaluated from them
// (HospitalMarket, the game's allhosps[]), for hospital 0 (World::market).

// reputation_affected's events (the game's REP_*, include/enums.h), named
// after their callers.
enum RepEvent : uint8_t {
    kRepCured = 0,
    kRepGoForTreatment = 2,
    kRepDied = 3,
    kRepDiagnosed = 4,
    kRepDiagnosisFailed = 5,
    kRepComputerDiagnosed = 6,
    kRepNoTreatmentFound = 7,
    kRepResearched = 9,
    kRepNextTreatment = 10,
    kRepMachineExploded = 11,
    kRepEmergencySuccess = 12,
    kRepEmergencyFailed = 13,
    kRepEpidemicDeclared = 14,
    kRepEpidemicCoverUpFound = 15,
    kRepVipGood = 16,
    kRepVipFair = 17,
    kRepVipBad = 18,
    kRepAutopsy = 19,
    kRepIllnessFound = 21,
    kRepCase = 22,
    kRepTrophy = 23,
    kRepAward = 24,
};

// Thing.person_flags: the patient is counted in the statistics
// (PF_RECORDED).
constexpr uint32_t kPfRecorded = 0x400000;

// reputation_affected (0x58370) for hospital 0: adds `event`'s points (a
// fixed amount, or a percentage of the hospital's events, `amount` scaling
// kRepEmergencyFailed, kRepTrophy and kRepAward) and kRepCase's one event to
// the hospital's totals and, for `treatment` (an illness; 0 for the hospital
// alone), to the treatment's, re-evaluates the reputations changed and then
// moves the treatment's price (update_single_cure_cost). Nothing happens
// without World::market. The rivals' monthly banks (comp_buckets) are not
// ported.
void reputation_affected(World &world, uint8_t treatment, RepEvent event, int32_t amount = 0);

// What reputation_affected adds for an event: points, and events counted.
struct RepChange {
    int32_t delta = 0;
    int32_t count = 0;
};

// reputation_affected's switch (src/sim.cpp:58-123) on hospital `hosp`'s
// totals: the points `event` adds, and the one event kRepCase counts.
RepChange reputation_change(const HospitalMarket &hosp, uint8_t treatment, RepEvent event, int32_t amount);

// evaluate_reputation (0x58730) for hospital `hosp`: as the hospital-0
// version below, on any hospital's totals.
void evaluate_reputation(HospitalMarket &hosp, uint8_t treatment);

// evaluate_reputation (0x58730) for hospital 0: the reputation for
// `treatment`, or the hospital's for 0, from its totals: points * 500 /
// events, 0-1000; 500 with no events.
void evaluate_reputation(World &world, uint8_t treatment);

// record_person_as_patient (0x3e668): counts patient `id` once
// (kPfRecorded) as a case of its illness (kRepCase).
void record_person_as_patient(World &world, uint16_t id);

} // namespace th
