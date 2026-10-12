// The advisor's find_advice (src/advisor.cpp): which situation's advice is
// to be given, the targets and counts it tests first, and the helpers the
// situations of advisor_situations.cpp share.

#include "core/world/advisor_internal.h"

#include "core/world/research.h"
#include "core/world/staff.h"
#include "core/world/world.h"

#include <bit>
#include <cstdlib>

namespace th
{

inline namespace advisor_detail
{

// The player has a reception (listhead.rooms[ROOM_RECEPTION], which placing
// the first reception desk sets).
bool has_reception(const World &world)
{
    return world.first_of_kind[kObjectReceptionDesk] != 0;
}
} // namespace advisor_detail

inline namespace advisor_detail
{

// is_relevant_criterion (src/sim.cpp): the last entry of `table` for
// `criterion`, 0 for none (entry 0 is never found).
int relevant_criterion(uint16_t criterion, const std::array<Balance::WinLose, 10> &table)
{
    for (int i = 9; i >= 0; i--)
        if (table[size_t(i)].criteria == criterion)
            return i;
    return 0;
}
} // namespace advisor_detail

namespace
{

// forced_advice (src/advisor.cpp). Forced message 2 (a network game's
// losers) is not ported and gives no text. The mayor's visit (8) is said in
// the situation's own words the first time, then names the visitor.
void forced_advice(Advisor &a, const AdvisorInputs &in)
{
    const LanguageText &text = *in.text;
    if (a.forced_advice_id == 8) {
        if (!a.given[7]) {
            a.text = text.at(TextSection::AdvisorSituations, 8);
            a.given[7] = 1;
        } else {
            say(a, text.at(TextSection::ForcedAdvice, 8),
                std::string(text.at(TextSection::MayorNames, in.world->mayor.name)).c_str());
        }
    } else if (a.forced_advice_id != 2) {
        a.text = text.at(TextSection::ForcedAdvice, a.forced_advice_id);
    }
    a.advice_id = 0;
    a.forced_advice_id = 0;
}

} // namespace

inline namespace advisor_detail
{

// saferand (src/advisor.cpp), drawing on `safeseed`.
uint16_t saferand(uint32_t &safeseed)
{
    safeseed = std::rotr(safeseed * 0x24a1u + 0x24dfu, 13);
    return uint16_t(safeseed);
}
} // namespace advisor_detail

inline namespace advisor_detail
{

std::string_view advisor_text(const AdvisorInputs &in, int i)
{
    return in.text->at(TextSection::Advisor, i);
}
} // namespace advisor_detail

inline namespace advisor_detail
{

std::string_view extra_advice(const AdvisorInputs &in, int i)
{
    return in.text->at(TextSection::ExtraAdvice, i);
}
} // namespace advisor_detail

namespace
{

// find_advice's low balance advice (0x48).
bool find_advice_low_balance(Advisor &a, const AdvisorInputs &in)
{
    const Balance &settings = *in.settings;
    const Finance &f = *in.finance;
    int32_t balance = in.balance;
    int crit = relevant_criterion(kCriterionBalance, settings.lose_criteria);
    if (crit && in.open) {
        int win = relevant_criterion(kCriterionBalance, settings.win_criteria);
        int32_t lose = settings.lose_criteria[size_t(crit)].value;
        if (!win || balance - f.loan >= settings.win_criteria[size_t(win)].value)
            return false;
        // The fourth step's line (0x1f) is about damaged machinery, as in
        // the game.
        for (int i = 0; i < 4; i++) {
            if (balance < lose + (i + 1) * std::abs(lose) / 10 && balance > lose) {
                say(a, extra_advice(in, 0x1c + i), balance - lose);
                return true;
            }
        }
        return false;
    }
    if (f.start_cash / 10 >= uint32_t(balance) && f.loan == 0) {
        // Unsigned, as in the game: not when overdrawn.
        say(a, advisor_text(in, 0x48));
        return true;
    }
    if (f.start_cash / 10 >= uint32_t(balance) && f.loan != 0 && f.loan < f.max_loan) {
        say(a, extra_advice(in, 0xf));
        return true;
    }
    return false;
}

// find_advice's reputation target advice (0x42).
bool find_advice_reputation(Advisor &a, const AdvisorInputs &in)
{
    const Balance &settings = *in.settings;
    int crit = relevant_criterion(kCriterionReputation, settings.win_criteria);
    if (!crit || !in.open || in.month <= 5)
        return false;
    int32_t value = settings.win_criteria[size_t(crit)].value;
    if (in.reputation > value)
        say(a, extra_advice(in, 0x3b), value);
    else if ((value - in.reputation) * 100 / value < 10)
        say(a, extra_advice(in, 0x3c), value);
    else
        say(a, advisor_text(in, 0x42), in.reputation - value);
    return true;
}

// find_advice's hospital value target advice (0x43).
bool find_advice_hospital_value(Advisor &a, const AdvisorInputs &in)
{
    const Balance &settings = *in.settings;
    const Finance &f = *in.finance;
    int crit = relevant_criterion(kCriterionValue, settings.win_criteria);
    if (!crit || !in.open || in.month <= 6)
        return false;
    int32_t value = settings.win_criteria[size_t(crit)].value;
    if (f.hospital_value > value) {
        say(a, extra_advice(in, 0x3d), value);
        return true;
    }
    if ((value - f.hospital_value) * 100 / value < 10) {
        say(a, extra_advice(in, 0x3e), value - f.hospital_value);
        return true;
    }
    if (f.hospital_value < value) {
        say(a, advisor_text(in, 0x43), value);
        return true;
    }
    return false;
}

// find_advice's loan and bank balance target advice (0x14).
bool find_advice_bank_balance(Advisor &a, const AdvisorInputs &in)
{
    const Balance &settings = *in.settings;
    const Finance &f = *in.finance;
    int32_t balance = in.balance;
    if (f.loan && balance > f.loan * 3) {
        say(a, extra_advice(in, 0x38));
        return true;
    }
    int crit = relevant_criterion(kCriterionBalance, settings.win_criteria);
    if (!crit || !in.open)
        return false;
    int32_t value = settings.win_criteria[size_t(crit)].value;
    if (balance - f.loan > value && f.low_balance) {
        say(a, extra_advice(in, 0x37), value);
        return true;
    }
    if (balance - f.loan > value - std::abs(value) / 10 && balance - f.loan < value && f.low_balance) {
        say(a, advisor_text(in, 0x14), value - (balance - f.loan));
        return true;
    }
    return false;
}

// find_advice's death count advice (0x4c).
bool find_advice_deaths(Advisor &a, const AdvisorInputs &in, uint32_t &safeseed)
{
    const Finance &f = *in.finance;
    if (f.deaths <= 4 || !(a.event_flags & kAdvisorEventDeath) || saferand(safeseed) % 4 != 0)
        return false;
    if (saferand(safeseed) % 2 == 0)
        say(a, advisor_text(in, 0x4c), int(f.deaths));
    else
        say(a, extra_advice(in, 0x25), int(f.deaths));
    a.event_flags &= ~kAdvisorEventDeath;
    return true;
}

// find_advice's cure count advice (0x51).
bool find_advice_cures(Advisor &a, const AdvisorInputs &in, uint32_t &safeseed)
{
    const Finance &f = *in.finance;
    if (f.cures <= 4 || !(a.event_flags & kAdvisorEventCure) || saferand(safeseed) % 4 != 0)
        return false;
    if (saferand(safeseed) % 2 == 0)
        say(a, advisor_text(in, 0x51), int(f.cures));
    else
        say(a, extra_advice(in, 0x24), int(f.cures));
    a.event_flags &= ~kAdvisorEventCure;
    return true;
}

bool find_advice_announcement(Advisor &a, const AdvisorInputs &in, uint16_t situation);

// find_advice's situations of the statistics pass and of what the
// hospital holds (see advisor_frame).
bool find_advice_counts(Advisor &a, int id, const AdvisorInputs &in, uint32_t &safeseed)
{
    if (!in.world || !in.finance || !in.settings)
        return false;
    uint16_t situation = a.advice[size_t(id)].id;
    if (situation == 0x17 || situation == 0x18 || situation == 0x19 || situation == 0x1a || situation == 0x1b ||
        situation == 0x79 || situation == 0x7a || situation == 0x86)
        return find_advice_announcement(a, in, situation);
    Situation s{a, in, *in.world, in.world->counts, count_staff(*in.world), safeseed};
    switch (situation) {
    case 9:
    case 0xa:
        return advise_no_visitors(s, id, situation == 0xa);
    case 0x50:
    case 0x53:
    case 0x8c:
    case 0x5d:
    case 0x68:
        return advise_room_built(s, situation);
    default:
        return advise_patients(s, situation) || advise_staff(s, situation) ||
               advise_litter_and_plants(s, id, situation) || advise_hospital(s, id, situation) ||
               advise_events(s, situation);
    }
}

// find_advice's case 0x86: the oldest computer player waiting to be said to
// have opened its hospital, named, dropped once said.
bool find_advice_rival_opened(Advisor &a, const AdvisorInputs &in)
{
    uint32_t value = in.world->announce[kAnnRivalOpened][0];
    if (!value)
        return false;
    done_announcement(*in.world, kAnnRivalOpened);
    say(a, in.text->at(TextSection::Advisor, 0x86), in.world->rivals[value % in.world->rivals.size()].name.data());
    a.rewrap = true;
    return true;
}

// find_advice's research announcements (0x17-0x1b, 0x79, 0x7a): the oldest value
// waiting of the kind, named by its object, illness or research category,
// dropped once said.
bool find_advice_announcement(Advisor &a, const AdvisorInputs &in, uint16_t situation)
{
    if (!in.world || !in.text)
        return false;
    auto once = [&](uint8_t kind, uint16_t advice, TextSection section) {
        uint32_t value = in.world->announce[kind % kAnnounceKinds][0];
        if (!value)
            return false;
        done_announcement(*in.world, kind);
        say(a, in.text->at(TextSection::Advisor, int(advice)), std::string(in.text->at(section, int(value))).c_str());
        a.rewrap = true;
        return true;
    };
    switch (situation) {
    case 0x7a:
        if (!in.world->announce[kAnnAutopsyDiscovered % kAnnounceKinds][0])
            return false;
        done_announcement(*in.world, kAnnAutopsyDiscovered);
        say(a, advisor_text(in, 0x7a));
        return true;
    case 0x17:
        return once(kAnnObjectResearched, 0x17, TextSection::ObjectNames);
    case 0x18:
        return once(kAnnNewDrug, 0x18, TextSection::Illness);
    case 0x19:
        return once(kAnnDrugImproved, 0x19, TextSection::Illness);
    case 0x1a:
        return once(kAnnObjectImproved, 0x1a, TextSection::ObjectNames);
    case 0x1b:
        return once(kAnnObjectAvailable, 0x1b, TextSection::ObjectNames);
    case 0x79:
        return once(kAnnCategoryDone, 0x79, TextSection::ResearchNames);
    case 0x86:
        return find_advice_rival_opened(a, in);
    default:
        return false;
    }
}

// find_advice's test of the situation advice `id` is about.
bool find_advice_situation(Advisor &a, int id, const AdvisorInputs &in, uint32_t &safeseed)
{
    switch (a.advice[size_t(id)].id) {
    case 0x48:
        return find_advice_low_balance(a, in);
    case 0x42:
        return find_advice_reputation(a, in);
    case 0x43:
        return find_advice_hospital_value(a, in);
    case 0x14:
        return find_advice_bank_balance(a, in);
    case 4:
        if (!a.forced_advice_id)
            return false;
        forced_advice(a, in);
        return true;
    case 5:
        if (!a.mouse_advice_id)
            return false;
        a.text = in.text->at(TextSection::MouseAdvice, a.mouse_advice_id);
        return true;
    case 0x49:
        if (count_staff(*in.world).total <= in.level + 5 || in.world->room_of_type[kRoomStaffRoom])
            return false;
        say(a, advisor_text(in, 0x49));
        return true;
    case 0x69:
        if (!has_reception(*in.world) || count_staff(*in.world).by_kind[kStaffReceptionist] != 0 || in.level != 1)
            return false;
        say(a, advisor_text(in, 0x69));
        in.world->building.last_built_room = 0;
        return true;
    case 0x4c:
        return find_advice_deaths(a, in, safeseed);
    case 0x51:
        return find_advice_cures(a, in, safeseed);
    case 0x15:
        if (!a.heli_landed)
            return false;
        if (!a.given[19])
            say(a, in.text->at(TextSection::AdvisorSituations, 16));
        else
            say(a, advisor_text(in, 0x15));
        a.given[19] = 1;
        a.heli_landed = false;
        return true;
    case 0x1c:
        if (a.given[10] || !(a.event_flags & kAdvisorEventResign))
            return false;
        say(a, advisor_text(in, 0x1c));
        a.given[10] = 1;
        a.event_flags &= ~kAdvisorEventResign;
        return true;
    default:
        return find_advice_counts(a, id, in, safeseed);
    }
}

} // namespace

inline namespace advisor_detail
{

// find_advice (src/advisor.cpp) for the situations ported (see
// advisor_frame), drawing on `safeseed` (saferand).
bool find_advice(Advisor &a, int id, const AdvisorInputs &in, uint32_t &safeseed)
{
    bool found = find_advice_situation(a, id, in, safeseed);
    if (found && a.advice[size_t(id)].id && !a.text.empty()) {
        Advice &advice = a.advice[size_t(id)];
        a.mouse_advice_id = 0;
        a.line_count = 0;
        a.extra_segments = 0;
        advice.times_given++;
        advice.time = in.game_turn;
        a.advice_id = int16_t(id);
        a.time_left = int16_t(advice.duration / 2);
    }
    return found;
}
} // namespace advisor_detail

} // namespace th
