// find_advice's situations of the statistics pass (src/advisor.cpp's
// advisor_* tests): staff, patients, machines, litter and plants, the
// hospital's rooms and the events left for the advisor.

#include "core/world/advisor_internal.h"

#include "core/world/queue.h"
#include "core/world/research.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <cstdlib>

namespace th
{

namespace
{

// The illnesses (ILL_*) whose treatment the advice after a room is built
// reads what is required for.
constexpr uint8_t kIllKingComplex = 4;
constexpr uint8_t kIllInvisibility = 5;
constexpr uint8_t kIllKidneyBeans = 19;

// The announcement kinds (ANN_*) of the staff promoted, and the value of
// ANN_PROMOTED for each skill a training room teaches.
constexpr uint8_t kAnnPromoted = 4;
constexpr uint8_t kAnnJuniorToDoctor = 0x0a;
constexpr uint8_t kAnnDoctorToConsultant = 0x0d;
constexpr uint32_t kPromotedSurgeon = 5;
constexpr uint32_t kPromotedPsychiatrist = 6;
constexpr uint32_t kPromotedResearcher = 8;

// play_ambient's announcements of the advisor (kAnnounceLitter,
// kAnnounceTooCold and kAnnounceTooHot of core/audio/sounds.h).
constexpr uint8_t kAmbientLitter = 0xd7;
constexpr uint8_t kAmbientTooCold = 0xd8;
constexpr uint8_t kAmbientTooHot = 0xd9;

// The text index of a variant, base + saferand % count. The seed it draws
// from is traced first, so a replay shows where the presentation generator
// of the original and of the port part.
uint16_t advice_variant(Situation &s, uint16_t base, uint16_t count)
{
    trace_event("advice_variant %u seed=%08x", unsigned(base), unsigned(s.safeseed));
    return uint16_t(base + saferand(s.safeseed) % count);
}

// Hospital 0's objects of `kind` (listhead.objects' lists, which the game
// counts as NRAdcounts.benches as they are placed; it never uncounts a
// bench sold).
int count_objects(const World &world, uint8_t kind)
{
    int n = 0;
    for (const Thing &t : world.things)
        n += t.type == kThingObject && t.subtype == kind && t.owner == 0;
    return n;
}

// The game's `plant_health_sum * factor / plants`, with the product taken in
// 32 bits as the original's multiply does, so the average wraps the same way
// past 1309 of sum (24 healthy plants).
int32_t plant_health_scaled(const World &world, uint32_t factor)
{
    return int32_t(uint32_t(world.plant_health_sum) * factor) / world.plants;
}

// advisor_old_hardware (0x60f90): the machines' average wear.
bool advisor_old_hardware(Situation &s)
{
    int32_t wear = s.c.machine_wear_sum, m = s.c.machines;
    if (wear * 20 * m > m * 0x1300) {
        say(s.a, s.handymen() > 0 ? s.text(3) : s.extra(0x21));
        return true;
    }
    if (wear * 5 * m > m * 0x400) {
        say(s.a, s.extra(0x1f));
        return true;
    }
    if (wear * 5 * m > m * 0x300) {
        say(s.a, s.handymen() > 0 ? s.extra(0x20) : s.extra(0x22));
        return true;
    }
    if (wear * 2 * m <= m * 0x100)
        return false;
    if (!s.a.given[12]) {
        say(s.a, s.situation(10));
        s.a.given[12] = 1;
        return true;
    }
    if (s.handymen() == 0)
        return false;
    say(s.a, s.extra(0x23));
    return true;
}

// find_advice's too few doctors (7, `kind` kStaffDoctor) or nurses (0x64):
// patients waiting long with none of them idle.
bool advise_short_staffed(Situation &s, uint8_t kind, int message)
{
    const AdCounts &c = s.c;
    if (c.patients <= 3 || !s.has_patients())
        return false;
    // wait_sums[0] and [1] are the nurses' and doctors' figures.
    size_t k = kind == kStaffDoctor ? 1 : 0;
    if (c.wait_counts[k] <= 0 || c.idle_staff[kind] != 0)
        return false;
    if ((c.wait_sums[k] << 8) / c.wait_counts[k] <= (s.in.settings->gbv.leave_max << 8) / 2)
        return false;
    say(s.a, s.text(message));
    return true;
}

// find_advice's too many doctors (8: at least 6, a third idle) or nurses
// (0x65: at least 4, a quarter idle).
bool advise_overstaffed(Situation &s, uint8_t kind, int least, int part, int message)
{
    int n = s.staff.by_kind[kind];
    if (n < least || s.c.idle_staff[kind] <= n / part || !s.has_patients())
        return false;
    say(s.a, s.text(message));
    return true;
}

} // namespace

inline namespace advisor_detail
{

// find_advice's no visitors (9) and no cures (0xa) after the fourth month.
bool advise_no_visitors(Situation &s, int id, bool cures)
{
    const Finance &f = *s.in.finance;
    int32_t visitors = f.history[kHistoryVisitors][11];
    if (s.in.month <= 4 || !s.in.open || s.a.advice[size_t(id)].times_given >= 1)
        return false;
    if (!cures && visitors != 0)
        return false;
    if (cures && (f.history[kHistoryCures][11] != 0 || visitors <= 0 || s.in.visitors <= s.world.patients_left_count))
        return false;
    say(s.a, s.text(cures ? 10 : 9));
    return true;
}
} // namespace advisor_detail

namespace
{

// find_advice's long waits (0xb).
bool advise_waiting(Situation &s)
{
    const AdCounts &c = s.c;
    int leave_max = s.in.settings->gbv.leave_max;
    if (c.patients <= 5 || !s.has_patients() || (c.waiting_sum << 8) / c.patients <= (leave_max << 9) / 4)
        return false;
    int32_t value = ((c.impatience_sum - c.waiting_sum) << 8) / leave_max;
    say(s.a, value * 10 < 0x100 ? s.text(11) : s.extra(0x32));
    return true;
}

// advisor_peeps_unhappy (0x612b8): patients gone home fed up (counted
// since last said), else their happiness.
bool advisor_peeps_unhappy(Situation &s)
{
    const Balance::Gbv &gbv = s.in.settings->gbv;
    int32_t happiness = (s.c.happiness_sum << 10) / s.c.patients;
    if (s.world.patients_left_count) {
        say(s.a, s.world.patients_left_count > 1 ? s.extra(7) : s.extra(0x31));
        s.world.patients_left_count = 0;
        return true;
    }
    if ((gbv.very_unhappy << 10) > happiness) {
        say(s.a, s.extra(6));
        return true;
    }
    if ((gbv.unhappy << 10) > happiness) {
        say(s.a, s.text(14));
        return true;
    }
    return false;
}

// advisor_thirsty_peeps (0x61188): the patients' refreshment. Its "very
// thirsty" test (below 10) comes after one below 30 and is never reached.
bool advisor_thirsty_peeps(Situation &s)
{
    if (s.c.refreshment_sum / s.c.patients >= 0x1e)
        return false;
    if (!s.a.given[18]) {
        say(s.a, s.situation(15));
        s.a.given[18] = 1;
    } else {
        say(s.a, s.world.first_of_kind[kObjectDrinksMachine] == 0 ? s.text(15) : s.extra(4));
    }
    return true;
}

// advisor_peeps_warmth (0x613a0): the patients' warmth, with the
// announcer's too hot or too cold.
bool advisor_peeps_warmth(Situation &s)
{
    int warmth = s.c.warmth_sum / s.c.patients;
    bool cold = warmth < 0x14;
    if (warmth * 2 > 0xf0)
        say(s.a, s.extra(0x34));
    else if (warmth > 0x50)
        s.once(17, 14, s.text(0x11));
    else if (warmth < 10)
        say(s.a, s.extra(0x35));
    else if (!cold)
        return false;
    else if (!s.a.given[15] && count_radiators(s.world, 0) == 0)
        s.once(15, 12, {});
    else if (s.a.given[15] && !s.a.given[16] && count_radiators(s.world, 0) == 0)
        s.once(16, 13, {});
    else
        say(s.a, s.extra(13));
    s.ambient(warmth < 10 || cold ? kAmbientTooCold : kAmbientTooHot);
    return true;
}

// advisor_staff_warmth (0x61b34): the staff's warmth.
bool advisor_staff_warmth(Situation &s)
{
    int warmth = s.c.staff_warmth_sum / s.staff.total;
    if (warmth > 0x50) {
        say(s.a, s.text(0x12));
        s.ambient(kAmbientTooHot);
        return true;
    }
    if (warmth < 0x14) {
        say(s.a, s.extra(0x36));
        s.ambient(kAmbientTooCold);
        return true;
    }
    return false;
}

// advisor_toilets (0x60c78): the patients' bladders, and whether there is
// a toilet.
bool advisor_toilets(Situation &s)
{
    const Balance::Gbv &gbv = s.in.settings->gbv;
    int ratio = s.c.bladder_sum / s.c.patients;
    bool none = s.world.first_of_kind[kObjectToilet] == 0;
    if (ratio * 4 > gbv.bowel_full * 3 && none)
        say(s.a, s.text(0x13));
    else if (gbv.bowel_full < ratio)
        say(s.a, none ? s.extra(10) : s.extra(11));
    else if (gbv.bowel_overflows < ratio)
        say(s.a, none ? s.extra(0x26) : s.extra(12));
    else
        return false;
    return true;
}

// advisor_litter_problem (0x61558): the litter per area of the hospital.
bool advisor_litter_problem(Situation &s)
{
    int32_t litter = (int32_t(s.world.litter_count) << 14) / s.in.finance->area;
    if (litter > 0x600) {
        s.ambient(kAmbientLitter);
        say(s.a, s.handymen() > 0 ? s.extra(0x12) : s.extra(0x11));
        return true;
    }
    if (litter > 0x300) {
        say(s.a, s.extra(0x10));
        return true;
    }
    if (litter <= 0x100 || s.handymen() == 1)
        return false;
    say(s.a, s.handymen() > 1 ? s.text(0x46) : s.extra(0x13));
    return true;
}

// advisor_plants (0x60e18): the plants per area, by the patients'
// impatience and happiness.
bool advisor_plants(Situation &s)
{
    const Balance::Gbv &gbv = s.in.settings->gbv;
    const AdCounts &c = s.c;
    int32_t per_area = (s.world.plants << 14) / s.in.finance->area;
    int32_t impatience = (c.impatience_sum << 6) / (c.patients * gbv.leave_max);
    int32_t happiness = (c.happiness_sum << 10) / c.patients;
    int few = per_area < 0x100 ? 0 : per_area < 0x200 ? 1 : -1;
    if (impatience > 0x30 || (gbv.very_unhappy << 10) > happiness) {
        if (few < 0)
            return false;
        say(s.a, s.extra((impatience > 0x30 ? 0x14 : 0x16) + few));
        return true;
    }
    if (per_area < 0x400 || per_area > 0x800)
        return false;
    say(s.a, s.text(0x4a));
    return true;
}

// advisor_dead_plants (0x617e4): the plants' average condition
// (World::plant_health_sum per plant, scaled), by how many handymen the
// hospital has; a hospital with exactly one is only told the middle bands.
bool advisor_dead_plants(Situation &s)
{
    World &world = s.world;
    if (world.plants == 0)
        return false;
    int32_t health = plant_health_scaled(world, 0x190000);
    if (health < 0xce4000) {
        if (s.handymen() == 0) {
            say(s.a, s.extra(0x27));
            return true;
        }
        if (s.handymen() > 1) {
            say(s.a, s.extra(0x28));
            return true;
        }
    } else if (health < 0x2260000) {
        say(s.a, s.extra(0x29));
        return true;
    } else if (health < 0x3c28000) {
        say(s.a, s.text(0x5a));
        return true;
    } else if (health > 0x51a4000) {
        say(s.a, s.extra(0x2b));
        return true;
    } else if (plant_health_scaled(world, 0x4000) > 0x490c000) {
        say(s.a, s.extra(0x2a));
        return true;
    }
    return false;
}

// advisor_benches (0x616b4): the bench shortage per patient, and the
// benches.
bool advisor_benches(Situation &s)
{
    int32_t standing = (s.world.bench_shortage << 10) / s.c.patients;
    int benches = count_objects(s.world, kObjectBench);
    if (standing > 0x1000)
        say(s.a, s.extra(0x1b));
    else if (standing > 0x400)
        say(s.a, s.extra(0x18));
    else if (benches > s.in.level + 8 && standing < -0x1400)
        say(s.a, s.extra(0x1a));
    else if (benches > 8 && standing < -0x400)
        say(s.a, s.extra(0x19));
    else if (benches > 4)
        say(s.a, s.text(0x4b));
    else
        return false;
    return true;
}

} // namespace

inline namespace advisor_detail
{

// find_advice's rooms just opened: a training room with no consultant
// (0x50), a research room with no researcher (0x53) or at all (0x8c), a
// room with machines and no handyman (0x5d), a GP's office with no doctor
// on level 1 (0x68). Each forgets the room once said.
bool advise_room_built(Situation &s, uint16_t situation)
{
    uint8_t &built = s.world.building.last_built_room;
    const std::array<uint16_t, 31> &rooms = s.world.room_of_type;
    bool found = false;
    if (situation == 0x50 && rooms[kRoomTrainingRoom] && s.c.consultants == 0 && built == kRoomTrainingRoom &&
        s.has_patients()) {
        say(s.a, s.text(0x50));
        found = true;
    } else if (situation == 0x53 && rooms[kRoomResearchRoom] && s.c.researchers == 0 && built == kRoomResearchRoom &&
               s.has_patients()) {
        s.once(1, 2, s.text(0x53));
        found = true;
    } else if (situation == 0x8c && !s.a.given[0] && built == kRoomResearchRoom) {
        s.once(0, 1, {});
        found = true;
    } else if (situation == 0x5d && s.c.machines > 0 && s.handymen() == 0 && built > 0 && s.has_patients()) {
        say(s.a, s.text(0x5d));
        found = true;
    } else if (situation == 0x68 && rooms[kRoomGpOffice] && s.staff.by_kind[kStaffDoctor] == 0 &&
               built == kRoomGpOffice && s.in.level == 1) {
        say(s.a, s.text(0x68));
        found = true;
    }
    if (found)
        built = 0;
    return found;
}
} // namespace advisor_detail

inline namespace advisor_detail
{

// find_advice's patients' figures: unhappy (0xe), thirsty (0xf), cold or
// hot (0x11), needing the toilet (0x13), waiting long (0xb), each once
// enough patients were counted.
bool advise_patients(Situation &s, uint16_t situation)
{
    if (!s.has_patients())
        return false;
    switch (situation) {
    case 0xb:
        return advise_waiting(s);
    case 0xe:
        return s.c.patients > 5 && advisor_peeps_unhappy(s);
    case 0xf:
        return s.c.patients > 5 && advisor_thirsty_peeps(s);
    case 0x11:
        return s.c.patients > 10 && advisor_peeps_warmth(s);
    case 0x13:
        return s.c.patients > 10 && advisor_toilets(s);
    case 0x4a:
        return s.c.patients > 8 && s.in.finance->area > 0 && advisor_plants(s);
    case 0x4b:
        return s.c.patients > 5 && advisor_benches(s);
    default:
        return false;
    }
}
} // namespace advisor_detail

inline namespace advisor_detail
{

// find_advice's staff and machines: the most worn machine (0x78) and the
// machines' wear (3), too few or many doctors and nurses (7, 8, 0x64,
// 0x65), the staff's warmth (0x12), a doctor cracked up (0x5e).
bool advise_staff(Situation &s, uint16_t situation)
{
    switch (situation) {
    case 0x5e:
        if (!s.c.crackups || !s.has_patients() || s.c.crackups != 1)
            return false;
        say(s.a, s.text(0x5e));
        s.world.counts.crackups = 0;
        return true;
    case 0x78:
        if (!s.c.worst_machine || !s.has_patients())
            return false;
        say(s.a, s.text(0x78), std::string(s.in.text->at(TextSection::ObjectNames, s.c.worst_machine)).c_str());
        return true;
    case 3:
        return s.c.machines > 0 && s.has_patients() && advisor_old_hardware(s);
    case 7:
        return advise_short_staffed(s, kStaffDoctor, 7);
    case 0x64:
        return advise_short_staffed(s, kStaffNurse, 0x64);
    case 8:
        return advise_overstaffed(s, kStaffDoctor, 6, 3, 8);
    case 0x65:
        return advise_overstaffed(s, kStaffNurse, 4, 4, 0x65);
    case 0x12:
        return s.staff.total > 10 && s.has_patients() && advisor_staff_warmth(s);
    default:
        return false;
    }
}
} // namespace advisor_detail

inline namespace advisor_detail
{

// find_advice's litter and plants: litter and no handyman (0x45, once),
// the litter (0x46), plants and no handyman (0x5c), the plants' condition
// (0x5a) and too many plants (0x5b).
bool advise_litter_and_plants(Situation &s, int id, uint16_t situation)
{
    int32_t area = s.in.finance->area;
    int plants = s.world.plants;
    switch (situation) {
    case 0x45:
        if (s.a.advice[size_t(id)].times_given >= 1 || s.world.litter_count == 0 || s.handymen() != 0)
            return false;
        say(s.a, s.text(0x45));
        return true;
    case 0x46:
        return area > 0 && s.has_patients() && advisor_litter_problem(s);
    case 0x5c:
        if (plants <= 0 || s.handymen() != 0)
            return false;
        say(s.a, s.text(0x5c));
        return true;
    case 0x5a:
        if (plants <= 2 || !s.has_patients())
            return false;
        return advisor_dead_plants(s);
    case 0x5b:
        if (!area || !s.has_patients() || (plants << 14) / area <= 0x800)
            return false;
        say(s.a, s.text(0x5b));
        return true;
    default:
        return false;
    }
}
} // namespace advisor_detail

namespace
{

// find_advice's deaths near the level's limit (0x41), after a death
// (kAdvisorEventDeath, forgotten once tested).
bool advise_deaths_limit(Situation &s)
{
    const Balance &settings = *s.in.settings;
    int crit = relevant_criterion(kCriterionDeaths, settings.lose_criteria);
    if (!crit || !s.in.open || !s.in.visitors || !(s.a.event_flags & kAdvisorEventDeath))
        return false;
    int32_t value = settings.lose_criteria[size_t(crit)].value;
    int32_t deaths = s.in.finance->deaths;
    auto visitors = int32_t(s.in.visitors);
    bool found = true;
    if (value - deaths * 100 / visitors < 10)
        say(s.a, s.extra(0x3a));
    else if (value - deaths * 100 / visitors < 0x19)
        say(s.a, s.text(0x41), visitors * value / 100 - deaths);
    else
        found = false;
    s.a.event_flags &= ~kAdvisorEventDeath;
    return found;
}

// find_advice's cures against the level's target (0x88), after a cure
// (kAdvisorEventCure, forgotten once tested): those still to come, or, with
// the target passed and the level not yet lost, a reminder of its value.
bool advise_cures_target(Situation &s)
{
    const Balance &settings = *s.in.settings;
    int crit = relevant_criterion(kCriterionCures, settings.win_criteria);
    const Finance &f = *s.in.finance;
    if (!crit || !s.in.open || !(s.a.event_flags & kAdvisorEventCure) || f.cures <= s.in.level * 3)
        return false;
    int32_t value = settings.win_criteria[size_t(crit)].value;
    bool found = true;
    if (f.cures > value && s.in.lose_progress > 0x19 && s.has_patients())
        say(s.a, s.extra(0x39), value);
    else if (f.cures < value)
        say(s.a, s.text(0x88), value - f.cures);
    else
        found = false;
    s.a.event_flags &= ~kAdvisorEventCure;
    return found;
}

// find_advice's 0x8a and 0x8b: the advice for an epidemic covered up (0x8a,
// kAdvisorEventCoverUp), which the game may overwrite with the rat-hole advice
// (0x8b) as it falls through into its case, once.
bool advise_ratholes(Situation &s)
{
    if (!s.a.given[6] && (s.a.event_flags & kAdvisorEventCoverUp)) {
        say(s.a, s.situation(7));
        s.a.given[6] = 1;
        s.a.event_flags &= ~kAdvisorEventCoverUp;
        if (!(!s.a.given[4] && s.world.rats.hole_count > 0))
            return true;
    }
    if (!s.a.given[4] && s.world.rats.hole_count > 0) {
        say(s.a, s.situation(5));
        s.a.given[4] = 1;
        return true;
    }
    return false;
}

} // namespace

inline namespace advisor_detail
{

// find_advice's first death (0x47) and cure (0x52), each once; the rooms
// open (0x6b, between 3 and 6); a treatment the patients will not pay for
// (0x75, advisor.event_flags 2); deaths near the level's limit (0x41); the
// first call awaiting a decision (0x87); a person stuck (0xc); the prices
// high or low (0x76, 0x77).
bool advise_hospital(Situation &s, int id, uint16_t situation)
{
    const Finance &f = *s.in.finance;
    bool first = s.a.advice[size_t(id)].times_given < 1;
    switch (situation) {
    case 0x47:
    case 0x52:
        if (!first || (situation == 0x47 ? f.deaths : f.cures) != 1)
            return false;
        say(s.a, s.text(situation));
        return true;
    case 0x6b: {
        int open = s.world.counts.open_rooms;
        if (open <= 2 || open >= 7)
            return false;
        say(s.a, s.text(advice_variant(s, 0x6b, 3)));
        return true;
    }
    case 0x75:
        if (!s.world.too_expensive || s.world.unpaid_treatment == 0)
            return false;
        say(s.a, s.text(0x75), std::string(s.in.text->at(TextSection::Illness, s.world.unpaid_treatment)).c_str());
        s.world.too_expensive = false;
        return true;
    case 0x41:
        return advise_deaths_limit(s);
    case 0x88:
        return advise_cures_target(s);
    case 0x76:
    case 0x77: {
        // The prices high (over 150%) or low (under 60%).
        int level = s.world.market ? s.world.market->price_level : 100;
        if (situation == 0x76 ? level <= 0x96 : (level == 0 || level >= 0x3c))
            return false;
        say(s.a, s.text(situation));
        return true;
    }
    case 0xc:
        if (!s.world.announce_stuck)
            return false;
        say(s.a, s.text(12));
        s.world.announce_stuck = false;
        return true;
    case 0x87:
        // Once per game, after a call about a patient or a new illness.
        if (s.a.given[8] || !(s.a.event_flags & kAdvisorEventCall))
            return false;
        say(s.a, s.text(0x87));
        s.a.given[8] = 1;
        s.a.event_flags &= ~kAdvisorEventCall;
        return true;
    case 0x8a:
    case 0x8b:
        return advise_ratholes(s);
    default:
        return false;
    }
}
} // namespace advisor_detail

namespace
{

// find_advice's warnings of how close the level is to being won (0x6f) or
// lost (0x72), once the level's year is well on: three messages, each for
// the progress left (from 100) being under 50, 25 and 5 percent.
bool advise_progress(Situation &s, uint16_t situation)
{
    const bool lose = situation == 0x72;
    if (s.in.level <= 1 || !s.has_patients() || s.in.month <= (lose ? 6 : s.in.level + 3))
        return false;
    const int32_t left = 100 - (lose ? s.in.lose_progress : s.in.win_progress);
    const int first = lose ? 0x74 : 0x71;
    if (left < 0x32 && left > 0x19)
        say(s.a, s.text(first));
    else if (left < 0x19 && left > 5)
        say(s.a, s.text(first - 1));
    else if (left < 5 && left > 0)
        say(s.a, s.text(first - 2));
    else
        return false;
    return true;
}

// find_advice's staff put down where they are not needed (0x7b), said once
// and forgotten: a doctor in a room that wants none, or a nurse; the room is
// named, or the receptionist's welcome is given.
bool advise_new_staff(Situation &s)
{
    World &w = s.world;
    if (!w.new_staff.pending)
        return false;
    const uint8_t kind = w.new_staff.kind, room = w.new_staff.room;
    w.new_staff = {};
    const auto room_name = [&] {
        return std::string(s.in.text->at(TextSection::RealRoomNames, kRoomTypeInfo[room % kRoomTypes].name));
    };
    if (kind == kStaffDoctor) {
        if (room == kRoomPsychiatry)
            say(s.a, s.text(0x7d));
        else if (room == kRoomResearchRoom)
            say(s.a, s.text(0x7b));
        else if (room == kRoomTheatre)
            say(s.a, s.text(0x7c));
        else if (room != kRoomToilets)
            say(s.a, s.text(advice_variant(s, 0x7e, 2)), room_name().c_str());
        else
            return false;
        return true;
    }
    if (kind == kStaffNurse && room != kRoomToilets) {
        say(s.a, s.text(advice_variant(s, 0x80, 2)), room_name().c_str());
        return true;
    }
    if (kind != kStaffReceptionist)
        return false;
    say(s.a, s.text(0x82));
    return true;
}

// find_advice's earthquake: the tremors begun (0x6a, once) and the strength
// of the one that ended (0x4d, from 8, forgotten once said).
bool advise_quake(Situation &s, uint16_t situation)
{
    if (situation == 0x6a) {
        if (s.in.quake_phase != 2 || s.a.given[13])
            return false;
        say(s.a, s.text(0x6a));
        s.a.given[13] = 1;
        return true;
    }
    if (s.c.last_quake_size < 8)
        return false;
    say(s.a, s.text(0x4d), s.c.last_quake_size / 2);
    s.world.counts.last_quake_size = 0;
    return true;
}

// find_advice's staff promoted: a skill gained in a training room (0x44,
// named by the announcement's value), a junior made a doctor (0x16) or a
// doctor a consultant (0x89); the announcement is dropped once said.
bool advise_promotion(Situation &s, uint16_t situation)
{
    const uint8_t kind = situation == 0x44   ? kAnnPromoted
                         : situation == 0x16 ? kAnnJuniorToDoctor
                                             : kAnnDoctorToConsultant;
    const uint32_t value = s.world.announce[kind % kAnnounceKinds][0];
    if (!value)
        return false;
    if (situation != 0x44) {
        // The original passes no argument: a "%s" in the text (French 0x16)
        // takes a stray stack value, shown here as the rank reached.
        const std::string_view text = s.text(situation);
        if (text.find("%s") != std::string_view::npos) {
            const int rank = situation == 0x16 ? 4 : 7;
            say(s.a, text, std::string(s.in.text->at(TextSection::SkillNames, rank)).c_str());
        } else {
            say(s.a, text);
        }
    } else if (value == kPromotedSurgeon || value == kPromotedPsychiatrist || value == kPromotedResearcher) {
        const int help = value == kPromotedSurgeon ? 0x3f : value == kPromotedPsychiatrist ? 0x40 : 0x41;
        say(s.a, s.text(0x44), std::string(s.in.text->at(TextSection::Help, help)).c_str());
    }
    done_announcement(s.world, kind);
    return true;
}

// find_advice's follow-ups to a theatre built (0x66, 0x54: two surgeons to
// hire, 0x66 and a ward too; 0x67, 0x55: the second; 0x56: only the ward).
// What is left to say, the surgery's own words the first time.
bool advise_theatre_built(Situation &s, uint16_t situation)
{
    uint8_t &built = s.world.building.last_built_room;
    const Required r = what_is_required(s.world, kIllKidneyBeans);
    const bool surgeons = (r.staff_mask & kSkillSurgeon) && r.missing_room != kRoomTheatre;
    if (built != kRoomTheatre)
        return false;
    switch (situation) {
    case 0x66:
    case 0x54:
        if (!surgeons || r.surgeons_needed != 2 || (situation == 0x66 && !r.needs_ward))
            return false;
        s.once(2, 3, s.text(situation));
        built = 1;
        return true;
    case 0x67:
    case 0x55:
        if (!surgeons || r.surgeons_needed != 1 || (situation == 0x67 && !r.needs_ward))
            return false;
        say(s.a, s.text(situation));
        return true;
    default:
        if (r.staff_mask || r.missing_room == kRoomTheatre || !r.needs_ward)
            return false;
        say(s.a, s.text(0x56));
        built = 0;
        return true;
    }
}

// find_advice's follow-ups to a room built: a ward with no nurse for it
// (0x57), a pharmacy still lacking a nurse for invisibility (0x4f) and a
// psychiatrist's room for the king complex still lacking a psychiatrist
// (0x4e, the situation's own words the first time). Each forgets the room.
bool advise_room_built_needs(Situation &s, uint16_t situation)
{
    uint8_t &built = s.world.building.last_built_room;
    bool found = false;
    if (situation == 0x57) {
        const Required r = what_is_required(s.world, kIllKidneyBeans);
        found = !r.needs_ward && r.needs_nurse && built == kRoomWard;
        if (found)
            say(s.a, s.text(0x57));
    } else if (situation == 0x4f) {
        const Required r = what_is_required(s.world, kIllInvisibility);
        found = (r.staff_mask & kSkillNurse) && r.missing_room != kRoomPharmacy && built == kRoomPharmacy;
        if (found)
            say(s.a, s.text(0x4f));
    } else {
        const Required r = what_is_required(s.world, kIllKingComplex);
        found = (r.staff_mask & kSkillPsychiatrist) && r.missing_room != kRoomPsychiatry && built == kRoomPsychiatry;
        if (found)
            s.once(3, 4, s.text(0x4e));
    }
    if (found)
        built = 0;
    return found;
}

// find_advice's epidemic (kAdvisorEventEpidemic): the first one's own words
// (0x8d, once per game, forgetting the event), and while it lasts the
// patients it has spread to once there are four (0xd; the game's further
// message for eight is never reached).
bool advise_epidemic(Situation &s, uint16_t situation)
{
    if (!(s.a.event_flags & kAdvisorEventEpidemic))
        return false;
    if (situation == 0x8d) {
        if (s.a.given[14])
            return false;
        say(s.a, s.situation(11));
        s.a.given[14] = 1;
        s.a.event_flags &= ~kAdvisorEventEpidemic;
        return true;
    }
    if (s.world.epidemic.spread_count < 4)
        return false;
    say(s.a, s.extra(1));
    return true;
}

} // namespace

inline namespace advisor_detail
{

// find_advice's situations of events the simulation leaves for him: see
// the functions above.
bool advise_events(Situation &s, uint16_t situation)
{
    switch (situation) {
    case 0x6f:
    case 0x72:
        return advise_progress(s, situation);
    case 0x7b:
        return advise_new_staff(s);
    case 0x6a:
    case 0x4d:
        return advise_quake(s, situation);
    case 0x44:
    case 0x16:
    case 0x89:
        return advise_promotion(s, situation);
    case 0x66:
    case 0x54:
    case 0x67:
    case 0x55:
    case 0x56:
        return advise_theatre_built(s, situation);
    case 0x57:
    case 0x4f:
    case 0x4e:
        return advise_room_built_needs(s, situation);
    case 0x8d:
    case 0xd:
        return advise_epidemic(s, situation);
    default:
        return false;
    }
}
} // namespace advisor_detail

} // namespace th
