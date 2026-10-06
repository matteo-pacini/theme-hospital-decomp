#include "core/world/reputation.h"

#include "core/world/arrivals.h"
#include "core/world/reception.h"
#include "core/world/trace.h"

namespace th
{

namespace
{

int32_t evaluated(int32_t points, int32_t events)
{
    int32_t rep = events > 0 ? points * 500 / events : 500;
    if (rep < 0)
        return 0;
    return rep > 1000 ? 1000 : rep;
}

} // namespace

void reputation_affected(World &world, uint8_t treatment, RepEvent event, int32_t amount)
{
    trace_event("rep %u event=%d amount=%d", unsigned(treatment), int(event), int(amount));
    if (!world.market)
        return;
    HospitalMarket &hosp = *world.market;
    int32_t delta = 0;
    int32_t count = 0;
    switch (event) {
    case kRepCured:
    case kRepComputerDiagnosed:
    case kRepIllnessFound:
        delta = 1;
        break;
    case kRepDied:
        delta = -1;
        break;
    case kRepCase:
        count = 1;
        break;
    case kRepDiagnosisFailed:
        delta = -(hosp.hospital_rep_events * 5) / 100;
        break;
    case kRepResearched:
        delta = (treatment ? hosp.rep_events[treatment] : hosp.hospital_rep_events) * 5 / 100;
        break;
    case kRepMachineExploded:
    case kRepEpidemicCoverUpFound:
        delta = -(hosp.hospital_rep_events * 10) / 100;
        break;
    case kRepEmergencySuccess:
    case kRepVipFair:
        delta = hosp.hospital_rep_events * 2 / 100;
        break;
    case kRepEmergencyFailed:
        delta = -(hosp.hospital_rep_events * (amount * 4)) / 100;
        break;
    case kRepEpidemicDeclared:
        delta = -(hosp.hospital_rep_events * 2) / 100;
        break;
    case kRepVipGood:
        delta = hosp.hospital_rep_events * 4 / 100;
        break;
    case kRepVipBad:
        delta = -(hosp.hospital_rep_events * 4) / 100;
        break;
    case kRepAutopsy:
        delta = -(hosp.hospital_rep_events * 20) / 100;
        break;
    case kRepTrophy:
    case kRepAward:
        delta = amount * hosp.hospital_rep_events / 100;
        break;
    default:
        break;
    }
    if (treatment) {
        hosp.rep_points[treatment] += delta;
        hosp.rep_events[treatment] += count;
        evaluate_reputation(world, treatment);
    }
    hosp.hospital_rep_points += delta;
    hosp.hospital_rep_events += count;
    evaluate_reputation(world, 0);
    if (treatment)
        update_single_cure_cost(world, treatment);
}

void evaluate_reputation(World &world, uint8_t treatment)
{
    if (!world.market)
        return;
    HospitalMarket &hosp = *world.market;
    if (treatment)
        hosp.reputation[treatment] = evaluated(hosp.rep_points[treatment], hosp.rep_events[treatment]);
    else
        hosp.hospital_reputation = evaluated(hosp.hospital_rep_points, hosp.hospital_rep_events);
}

void record_person_as_patient(World &world, uint16_t id)
{
    trace_event("record %u", unsigned(id));

    Thing &p = world.things[id];
    if (!(p.person_flags & kPfRecorded)) {
        reputation_affected(world, p.illness, kRepCase);
        p.person_flags |= kPfRecorded;
    }
}

} // namespace th
