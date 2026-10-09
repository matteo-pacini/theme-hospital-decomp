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

// reputation_affected's change for the events that move the hospital's
// reputation points by a percentage of its event count (0 for the others).
int32_t reputation_affected_percent(RepEvent event, int32_t amount)
{
    switch (event) {
    case kRepDiagnosisFailed:
        return -5;
    case kRepMachineExploded:
    case kRepEpidemicCoverUpFound:
        return -10;
    case kRepEmergencySuccess:
    case kRepVipFair:
        return 2;
    case kRepEmergencyFailed:
        return -(amount * 4);
    case kRepEpidemicDeclared:
        return -2;
    case kRepVipGood:
        return 4;
    case kRepVipBad:
        return -4;
    case kRepAutopsy:
        return -20;
    case kRepTrophy:
    case kRepAward:
        return amount;
    default:
        return 0;
    }
}

} // namespace

RepChange reputation_change(const HospitalMarket &hosp, uint8_t treatment, RepEvent event, int32_t amount)
{
    RepChange change;
    switch (event) {
    case kRepCured:
    case kRepComputerDiagnosed:
    case kRepIllnessFound:
        change.delta = 1;
        break;
    case kRepDied:
        change.delta = -1;
        break;
    case kRepCase:
        change.count = 1;
        break;
    case kRepResearched:
        change.delta = (treatment ? hosp.rep_events[treatment] : hosp.hospital_rep_events) * 5 / 100;
        break;
    default:
        change.delta = hosp.hospital_rep_events * reputation_affected_percent(event, amount) / 100;
        break;
    }
    return change;
}

void reputation_affected(World &world, uint8_t treatment, RepEvent event, int32_t amount)
{
    trace_event("rep %u event=%d amount=%d", unsigned(treatment), int(event), int(amount));
    if (!world.market)
        return;
    HospitalMarket &hosp = *world.market;
    const auto [delta, count] = reputation_change(hosp, treatment, event, amount);
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

void evaluate_reputation(HospitalMarket &hosp, uint8_t treatment)
{
    if (treatment)
        hosp.reputation[treatment] = evaluated(hosp.rep_points[treatment], hosp.rep_events[treatment]);
    else
        hosp.hospital_reputation = evaluated(hosp.hospital_rep_points, hosp.hospital_rep_events);
}

void evaluate_reputation(World &world, uint8_t treatment)
{
    if (world.market)
        evaluate_reputation(*world.market, treatment);
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
