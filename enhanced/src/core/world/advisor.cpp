#include "core/world/advisor.h"

#include "core/data/gamedata.h"
#include "core/render/panel.h"
#include "core/world/arrivals.h"
#include "core/world/build.h"
#include "core/world/finance.h"
#include "core/world/objects.h"
#include "core/world/queue.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <bit>
#include <cstdio>
#include <cstdlib>

namespace th
{

namespace
{

// The game's advice table (advice[], src/advisor.cpp): situation, duration
// and repeat weight.
constexpr std::array<Advice, kAdviceCount> kAdviceTable = {{
    {0, 0, 0},          {0x6a, 0xfa, 0},     {0x1c, 0xfa, 0x1e}, {0x87, 0xfa, 0x1e}, {0x8d, 0xfa, 0x1e},
    {0x8a, 0xfa, 0x1e}, {0x8b, 0xfa, 0x1e},  {0x8c, 0xfa, 0x1e}, {0x15, 0xfa, 0},    {0x4d, 0xfa, 0},
    {0x48, 0xfa, 0x14}, {0x5e, 0xfa, 0x14},  {0x78, 0xfa, 0x14}, {0x3, 0xfa, 0x14},  {0x6f, 0xfa, 0x1e},
    {0x72, 0xfa, 0x1e}, {0x75, 0xfa, 0x14},  {0x4, 0x82, 0},     {0x5, 0x82, 0},     {0xc, 0xfa, 0},
    {0x58, 0xfa, 0x14}, {0x7b, 0xfa, 0},     {0x6, 0xfa, 0x14},  {0x63, 0xfa, 0x14}, {0x7, 0xfa, 0x14},
    {0x8, 0xfa, 0x14},  {0x9, 0xfa, 0x14},   {0xa, 0xfa, 0x14},  {0x14, 0xfa, 0x14}, {0x76, 0xfa, 0x14},
    {0x77, 0xfa, 0x14}, {0xb, 0xfa, 0x14},   {0xc, 0xfa, 0x14},  {0xd, 0xfa, 0x14},  {0xe, 0xfa, 0x14},
    {0xf, 0xfa, 0x14},  {0x10, 0xfa, 0x14},  {0x11, 0xfa, 0x14}, {0x12, 0xfa, 0x14}, {0x13, 0xfa, 0x14},
    {0x86, 0xfa, 0},    {0x7a, 0xfa, 0},     {0x79, 0x15e, 0},   {0x44, 0xfa, 0},    {0x89, 0x15e, 0},
    {0x16, 0x15e, 0},   {0x17, 0xfa, 0},     {0x18, 0xfa, 0},    {0x19, 0xfa, 0},    {0x1a, 0xfa, 0},
    {0x1b, 0xfa, 0},    {0x45, 0xfa, 0x14},  {0x46, 0xfa, 0x14}, {0x88, 0xfa, 0x14}, {0x41, 0xfa, 0x14},
    {0x42, 0xfa, 0x14}, {0x43, 0xfa, 0x14},  {0x64, 0xfa, 0x14}, {0x65, 0xfa, 0x14}, {0x4b, 0xfa, 0x14},
    {0x5c, 0xfa, 0x14}, {0x5a, 0xfa, 0xf},   {0x5b, 0xfa, 0x14}, {0x4a, 0xfa, 0x14}, {0x49, 0xfa, 0x14},
    {0x66, 0x96, 0x14}, {0x54, 0x15e, 0x14}, {0x67, 0x96, 0x14}, {0x55, 0x96, 0x14}, {0x56, 0x96, 0x14},
    {0x50, 0x64, 0x14}, {0x53, 0x15e, 0x14}, {0x4f, 0x96, 0x14}, {0x57, 0x64, 0x14}, {0x4e, 0x15e, 0x14},
    {0x5d, 0x64, 0x14}, {0x47, 0x4b, 0x14},  {0x52, 0x4b, 0x14}, {0x4c, 0x64, 0x14}, {0x51, 0x64, 0x14},
    {0x69, 0x64, 0x1e}, {0x68, 0x64, 0x1e},  {0x6e, 0x4b, 0x14}, {0x6b, 0xfa, 0x1e}, {0, 0, 0},
}};

// advisor_specials (src/tables.cpp): the up states' occasional animations.
constexpr std::array<uint8_t, 2> kAdvisorSpecials = {9, 10};

// The speech bubble's layout in high resolution (HiresCoords 0x93-0x9e,
// src/coords.cpp, through AdvisorLayout in src/advtext.cpp): the wrap width
// is middle pieces * 17 + 42 + 11, less the text's margins of 8 and 4.
constexpr int kMiddleTextWidth = 17;
constexpr int kRightTextWidth = 42;
constexpr int kLeftTextWidth = 11;
constexpr int kTextLeftMargin = 8;
constexpr int kTextRightMargin = 4;

// The rest of the layout (HiresCoords 0x48, 0x9e, 0x93, 0x96-0x98): the
// advisor's x, the bubble's right end, the y of its other pieces and the
// message's y for three, two and one lines.
constexpr int kAdvisorX = 560;
constexpr int kRightPieceX = 515, kRightPieceY = 375;
constexpr int kBubbleY = 390;
constexpr int kTextY3 = 395, kTextY2 = 400, kTextY1 = 403;

// The panel sprites of the bubble: its left end, a middle piece, its right
// end (game_panel[37] to [39]).
constexpr size_t kBubbleLeft = 37, kBubbleMiddle = 38, kBubbleRightEnd = 39;

// statvar.view_height in high resolution: the message is clipped to the
// view above the panel.
constexpr int kViewHeight = 432;

// Right clicks on the bubble (HiresCoords 0x93's y, 0x92) and on him
// (0x90 to 0x91), process_mouse (src/mouse.cpp).
constexpr int kBubbleTop = 390;
constexpr int kBubbleRight = 531, kBubbleBottom = 428;
constexpr int kAdvisorLeft = 531, kAdvisorTop = 336, kAdvisorRight = 619, kAdvisorBottom = 428;

// Win and lose criteria (WinLose.criteria).
constexpr uint16_t kCriterionReputation = 1;
constexpr uint16_t kCriterionBalance = 2;
constexpr uint16_t kCriterionCures = 4;
constexpr uint16_t kCriterionDeaths = 5;
constexpr uint16_t kCriterionValue = 6;

// The game's OBJ_RECEPTION_DESK.
constexpr uint8_t kObjectReceptionDesk = 11;

// The level briefing's page that ends the tutorial (show_help_page(1, 0xf)).
constexpr int kTutorialEndPage = 0xf;

// The player has a reception (listhead.rooms[ROOM_RECEPTION], which placing
// the first reception desk sets).
bool has_reception(const World &world)
{
    return world.first_of_kind[kObjectReceptionDesk] != 0;
}

// is_relevant_criterion (src/sim.cpp): the last entry of `table` for
// `criterion`, 0 for none (entry 0 is never found).
int relevant_criterion(uint16_t criterion, const std::array<Balance::WinLose, 10> &table)
{
    for (int i = 9; i >= 0; i--)
        if (table[size_t(i)].criteria == criterion)
            return i;
    return 0;
}

// sprintf(advisor.text, format, ...) with a string of the language file.
template <typename... Args> void say(Advisor &a, std::string_view format, Args... args)
{
    if constexpr (sizeof...(args) == 0) {
        a.text = format;
    } else {
        char buf[270];
        std::snprintf(buf, sizeof buf, std::string(format).c_str(), args...);
        a.text = buf;
    }
}

void enter_state(Advisor &a, const AnimData &anims, uint8_t state)
{
    a.anim_state = state;
    uint16_t start = kAdvisorAnims[state];
    a.anim_frame = start < anims.starts.size() ? uint16_t(anims.starts[start].frame) : 0;
}

// mouse_advice and forced_advice (src/advisor.cpp). Forced messages 2 (a
// network game's losers) and 8 (the mayor's visit) are not ported and give
// no text.
void forced_advice(Advisor &a, const LanguageText &text)
{
    if (a.forced_advice_id != 2 && a.forced_advice_id != 8)
        a.text = text.at(TextSection::ForcedAdvice, a.forced_advice_id);
    a.advice_id = 0;
    a.forced_advice_id = 0;
}

// saferand (src/advisor.cpp), drawing on `safeseed`.
uint16_t saferand(uint32_t &safeseed)
{
    safeseed = std::rotr(safeseed * 0x24a1u + 0x24dfu, 13);
    return uint16_t(safeseed);
}

std::string_view advisor_text(const AdvisorInputs &in, int i)
{
    return in.text->at(TextSection::Advisor, i);
}

std::string_view extra_advice(const AdvisorInputs &in, int i)
{
    return in.text->at(TextSection::ExtraAdvice, i);
}

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

// What find_advice's situations of the statistics pass read: hospital 0's
// figures (World::counts) and staff, and where the advice goes.
struct Situation {
    Advisor &a;
    const AdvisorInputs &in;
    World &world;
    const AdCounts &c;
    StaffCounts staff;
    uint32_t &safeseed;

    bool has_patients() const { return c.has_patients; }
    int handymen() const { return staff.by_kind[kStaffHandyman]; }
    std::string_view text(int i) const { return advisor_text(in, i); }
    std::string_view extra(int i) const { return extra_advice(in, i); }
    std::string_view situation(int i) const { return in.text->at(TextSection::AdvisorSituations, i); }
    // play_ambient of an announcement, for the caller to play.
    void ambient(uint8_t index) const
    {
        if (in.ambient)
            *in.ambient = index;
    }
    // A message given once per game (myplayer.advisor_flags[flag]) or else
    // `otherwise`.
    void once(int flag, int situation_text, std::string_view otherwise)
    {
        if (!a.given[size_t(flag)]) {
            say(a, situation(situation_text));
            a.given[size_t(flag)] = 1;
        } else {
            say(a, otherwise);
        }
    }
};

// The game's OBJ_* kinds and ROOM_* types the advice tests.
constexpr uint8_t kObjectBench = 4;
constexpr uint8_t kObjectDrinksMachine = 7;
constexpr uint8_t kObjectToilet = 51;
constexpr uint8_t kRoomGpOffice = 7;
constexpr uint8_t kRoomTrainingRoom = 22;
constexpr uint8_t kRoomStaffRoom = 25;
constexpr uint8_t kRoomResearchRoom = 28;
constexpr uint8_t kRoomFirstBuilt = 7;

// play_ambient's announcements of the advisor (kAnnounceLitter,
// kAnnounceTooCold and kAnnounceTooHot of core/audio/sounds.h).
constexpr uint8_t kAmbientLitter = 0xd7;
constexpr uint8_t kAmbientTooCold = 0xd8;
constexpr uint8_t kAmbientTooHot = 0xd9;

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

// NRAdcounts.open_rooms: the rooms the player has opened (open_room) and
// not deleted.
int count_open_rooms(const World &world)
{
    int n = 0;
    for (const RoomInfo &r : world.rooms)
        n += r.type >= kRoomFirstBuilt && r.owner == 0 && r.paid_for;
    return n;
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

// find_advice's cures short of the level's target (0x88), after a cure
// (kAdvisorEventCure, forgotten once tested). Its advice for the target
// passed, which also needs the level's lose progress, is not ported.
bool advise_cures_target(Situation &s)
{
    const Balance &settings = *s.in.settings;
    int crit = relevant_criterion(kCriterionCures, settings.win_criteria);
    const Finance &f = *s.in.finance;
    if (!crit || !s.in.open || !(s.a.event_flags & kAdvisorEventCure) || f.cures <= s.in.level * 3)
        return false;
    int32_t value = settings.win_criteria[size_t(crit)].value;
    bool found = f.cures < value;
    if (found)
        say(s.a, s.text(0x88), value - f.cures);
    s.a.event_flags &= ~kAdvisorEventCure;
    return found;
}

// find_advice's rat holes (0x8a, 0x8b): the hole advice (0x8b) once, and the
// unraised event's (0x8a; see kAdvisorEvent0x100), which the game may
// overwrite with the hole advice.
bool advise_ratholes(Situation &s)
{
    if (!s.a.given[6] && (s.a.event_flags & kAdvisorEvent0x100)) {
        say(s.a, s.situation(7));
        s.a.given[6] = 1;
        s.a.event_flags &= ~kAdvisorEvent0x100;
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
        int open = count_open_rooms(s.world);
        if (open <= 2 || open >= 7)
            return false;
        say(s.a, s.text(0x6b + saferand(s.safeseed) % 3));
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

bool find_advice_announcement(Advisor &a, const AdvisorInputs &in, uint16_t situation);

// find_advice's situations of the statistics pass and of what the
// hospital holds (see advisor_frame).
bool find_advice_counts(Advisor &a, int id, const AdvisorInputs &in, uint32_t &safeseed)
{
    if (!in.world || !in.finance || !in.settings)
        return false;
    uint16_t situation = a.advice[size_t(id)].id;
    if (situation == 0x17 || situation == 0x18 || situation == 0x19 || situation == 0x1a || situation == 0x1b ||
        situation == 0x79 || situation == 0x7a)
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
               advise_litter_and_plants(s, id, situation) || advise_hospital(s, id, situation);
    }
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
        forced_advice(a, *in.text);
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

// What tutorial's steps work on: the advisor, the turn's inputs and the
// events the steps test.
struct Tutorial {
    Advisor &a;
    const AdvisorInputs &in;
    const World &world;
    std::array<uint8_t, 0x26> &shown;
    const Event &last;
    const Event &pending;
    // The player cancelled or hired someone other than a receptionist.
    bool hiring_other;

    void message(int i)
    {
        a.text = in.text->at(TextSection::Advisor, i);
        a.rewrap = true;
        a.line_count = 0;
        a.extra_segments = 0;
    }

    // A hint given as help text: no message, him hidden.
    void hint(uint16_t highlight, int flag)
    {
        a.line_count = 0;
        a.extra_segments = 0;
        a.tutorial_highlight = highlight;
        shown[size_t(flag)] = 1;
        a.text.clear();
    }

    void go(uint8_t step)
    {
        a.tutorial_step = step;
        a.tutorial_highlight = 0;
    }

    // Messages to give again once the player is back at the step they belong to.
    void forget(std::initializer_list<int> flags)
    {
        for (int flag : flags)
            shown[size_t(flag)] = 0;
    }
};

// Each of tutorial's steps returns what tutorial returns.

// tutorial step 0x1d: open the corridor furnishing.
int tutorial_open_furnish_corridor(Tutorial &t)
{
    if (t.shown[0] == 2)
        return -1;
    if (t.last.type == EV_OPEN_FURNISH_CORRIDOR) {
        t.shown[0] = 2;
        t.go(0x28);
    } else if (t.shown[0] == 0) {
        t.message(29);
        t.a.tutorial_highlight = 0x13;
        t.shown[0] = 1;
        return 1;
    }
    return -1;
}

// tutorial step 0x28: buy a reception desk.
int tutorial_buy_reception_desk(Tutorial &t)
{
    if (t.shown[11] == 2)
        return -1;
    if (t.pending.type == EV_CANCEL_BUY_OBJECTS || t.last.type == EV_PLACE_NEXT_OBJECT) {
        t.shown[0] = 0;
        t.shown[11] = 0;
        t.go(0x1d);
    } else if (t.last.type != EV_BUY_OBJECT || t.last.value != kObjectReceptionDesk) {
        if (t.shown[11] == 0) {
            t.hint(0xa, 11);
            return 0;
        }
    } else {
        t.shown[11] = 2;
        t.go(0x2a);
    }
    return -1;
}

// tutorial step 0x2a: confirm the purchase, to place the desk.
int tutorial_confirm_purchase(Tutorial &t)
{
    if (t.shown[13] == 2)
        return -1;
    if (t.pending.type == EV_CANCEL_BUY_OBJECTS) {
        t.shown[0] = 0;
        t.shown[11] = 0;
        t.shown[13] = 0;
        t.go(0x1d);
    } else if (t.last.type == EV_SELL_OBJECT && t.last.value == kObjectReceptionDesk &&
               t.world.placing.buy_count[kObjectReceptionDesk] == 0) {
        t.shown[13] = 0;
        t.shown[11] = 0;
        t.go(0x28);
    } else if (t.last.type != EV_PLACE_NEXT_OBJECT) {
        if (t.shown[13] == 0) {
            t.hint(9, 13);
            return 0;
        }
    } else {
        t.shown[13] = 2;
        t.go(0x29);
    }
    return -1;
}

// tutorial step 0x29: place the reception desk.
int tutorial_place_reception_desk(Tutorial &t)
{
    if (t.shown[12] == 2)
        return -1;
    if (t.pending.type == EV_CANCEL_BUILD) {
        t.shown[12] = 0;
        t.shown[13] = 0;
        t.shown[11] = 0;
        t.shown[0] = 0;
        t.go(0x1d);
    } else if (t.a.flags & kAdvisorBadPlacement) {
        t.message(46);
        t.shown[12] = 0;
        return 1;
    } else if (!has_reception(t.world)) {
        if (t.shown[12] == 0) {
            t.message(41);
            t.shown[12] = 1;
            return 1;
        }
    } else {
        t.shown[12] = 2;
        t.go(0x1e);
    }
    return -1;
}

// tutorial step 0x1e: open the staff hiring.
int tutorial_open_hire_staff(Tutorial &t)
{
    if (t.shown[1] == 2 || count_staff(t.world).receptionists != 0) {
        t.shown[1] = 2;
        t.go(0x20);
        return -1;
    }
    if (t.last.type == EV_OPEN_HIRE_STAFF) {
        t.shown[1] = 2;
        t.go(0x21);
    } else if (t.shown[1] == 0) {
        t.message(30);
        t.a.tutorial_highlight = 0x15;
        t.shown[1] = 1;
        return 1;
    }
    return -1;
}

// tutorial step 0x21: choose receptionists.
int tutorial_select_receptionists(Tutorial &t)
{
    if (t.shown[4] == 2) {
        t.go(0x25);
        return -1;
    }
    if (t.hiring_other) {
        t.shown[1] = 0;
        t.shown[4] = 0;
        t.go(0x1e);
    } else if (t.last.type == EV_HIRE_SELECT_KIND && t.last.value != kStaffReceptionist) {
        // The message stays as it was.
        t.a.line_count = 0;
        t.a.extra_segments = 0;
        t.a.tutorial_highlight = 3;
        return 1;
    } else if (t.last.type != EV_HIRE_SELECT_KIND || t.last.value != kStaffReceptionist) {
        if (t.shown[4] == 0) {
            t.hint(3, 4);
            return 0;
        }
    } else {
        t.shown[4] = 2;
        t.go(0x25);
    }
    return -1;
}

// tutorial step 0x25: look at the next applicant.
int tutorial_next_applicant(Tutorial &t)
{
    if (t.shown[8] == 2) {
        t.go(0x26);
        return -1;
    }
    if (t.hiring_other) {
        t.shown[8] = 2;
        t.shown[9] = 2;
        t.shown[1] = 0;
        t.shown[4] = 0;
        t.go(0x1e);
    } else if (t.last.type == EV_MARK_EMPLOYING && t.last.value == kStaffReceptionist) {
        t.shown[8] = 2;
        t.shown[9] = 2;
        t.go(0x24);
    } else if (t.last.type == EV_HIRE_SELECT_KIND && t.last.value != kStaffReceptionist) {
        t.shown[8] = 2;
        t.shown[9] = 2;
        t.shown[4] = 0;
        t.go(0x21);
    } else if (t.last.type != EV_HIRE_NEXT_APPLICANT || t.last.value != 1) {
        if (t.shown[8] == 0) {
            t.hint(7, 8);
            return 0;
        }
    } else {
        t.shown[8] = 2;
        t.go(0x26);
    }
    return -1;
}

// tutorial step 0x26: look at the previous applicant.
int tutorial_previous_applicant(Tutorial &t)
{
    if (t.shown[9] == 2) {
        t.go(0x22);
        return -1;
    }
    if (t.hiring_other) {
        t.shown[9] = 2;
        t.shown[1] = 0;
        t.shown[4] = 0;
        t.go(0x1e);
    } else if (t.last.type == EV_MARK_EMPLOYING && t.last.value == kStaffReceptionist) {
        t.shown[9] = 2;
        t.go(0x24);
    } else if (t.last.type == EV_HIRE_SELECT_KIND && t.last.value != kStaffReceptionist) {
        t.shown[9] = 2;
        t.shown[4] = 0;
        t.go(0x21);
    } else if (t.last.type != EV_HIRE_NEXT_APPLICANT || t.last.value != 0) {
        if (t.shown[9] == 0) {
            t.hint(6, 9);
            return 0;
        }
    } else {
        t.shown[9] = 2;
        t.go(0x22);
    }
    return -1;
}

// tutorial step 0x22: hire the applicant.
int tutorial_hire_applicant(Tutorial &t)
{
    if (t.shown[5] == 2) {
        t.go(0x24);
        return -1;
    }
    if (t.last.type == EV_CANCEL_BUILD) {
        t.shown[5] = 0;
        t.shown[1] = 0;
        t.shown[4] = 0;
        t.go(0x1e);
    } else if (t.last.type == EV_HIRE_SELECT_KIND && t.last.value != kStaffReceptionist) {
        t.shown[4] = 0;
        t.go(0x21);
    } else if (t.last.type != EV_MARK_EMPLOYING) {
        if (t.shown[5] == 0) {
            t.hint(5, 5);
            return 0;
        }
    } else {
        t.shown[5] = 2;
        t.go(0x24);
    }
    return -1;
}

// tutorial step 0x24: place the receptionist.
int tutorial_place_receptionist(Tutorial &t)
{
    if (t.shown[7] == 2) {
        t.go(0x20);
        return -1;
    }
    if (t.last.type == EV_CANCEL_BUILD) {
        t.shown[7] = 2;
        t.shown[1] = 0;
        t.shown[4] = 0;
        t.go(0x1e);
    } else if (!t.in.staff_place_ok) {
        t.message(60);
        t.shown[7] = 0;
        return 1;
    } else if (count_staff(t.world).receptionists == 0) {
        if (t.shown[7] == 0) {
            t.message(36);
            t.shown[7] = 1;
            t.a.tutorial_highlight = 0;
            return 1;
        }
    } else {
        t.shown[7] = 2;
        t.go(0x20);
    }
    return -1;
}

// tutorial step 0x20: open the room building, once no GP's office exists.
int tutorial_open_build_room(Tutorial &t)
{
    if (t.shown[3] == 2 || t.world.room_of_type[kRoomGp]) {
        t.shown[3] = 2;
        t.go(0x1f);
    } else if (t.last.type == EV_OPEN_BUILD_ROOM) {
        t.shown[3] = 2;
        t.go(0x2b);
    } else if (t.shown[3] == 0) {
        t.message(32);
        t.a.tutorial_highlight = 0x12;
        t.shown[3] = 1;
        return 1;
    }
    return -1;
}

// tutorial step 0x2b: choose the department of the GP's office.
int tutorial_choose_department(Tutorial &t)
{
    if (t.last.type == EV_CANCEL_BUILD) {
        t.forget({3, 14});
        t.go(0x20);
    } else if (t.shown[14] != 2) {
        if (t.last.type != EV_REQ1_RESET_ROOMS || t.last.value != 3) {
            if (t.shown[14] == 0) {
                t.hint(8, 14);
                return 0;
            }
        } else {
            t.shown[14] = 2;
            t.go(0x2c);
        }
    }
    return -1;
}

// tutorial step 0x2c: choose the GP's office. The hint marks flag 14 shown,
// not 15, as the game does, so it is given every turn.
int tutorial_choose_gp_office(Tutorial &t)
{
    if (t.last.type == EV_CLOSE_REQUESTERS) {
        t.forget({3, 14, 15});
        t.go(0x20);
    } else if (t.shown[15] != 2) {
        if (t.last.type == EV_INIT_BLUEPRINT && t.last.value != kRoomGp) {
            t.forget({3, 14, 15});
            t.go(0x20);
        } else if (t.last.type != EV_INIT_BLUEPRINT) {
            if (t.shown[15] == 0) {
                t.hint(0xb, 14);
                return 0;
            }
        } else {
            t.shown[15] = 2;
            t.go(0x39);
        }
    }
    return -1;
}

// tutorial step 0x39: drag the blueprint out.
int tutorial_drag_blueprint(Tutorial &t)
{
    if (t.last.type == EV_CANCEL_BUILD) {
        t.forget({3, 14, 15, 28});
        t.go(0x20);
    } else if (t.last.type != EV_MODIFY_BLUEPRINT) {
        t.message(47);
        t.shown[28] = 0;
        return 1;
    } else {
        t.shown[28] = 2;
        t.a.tutorial_step = 0x2f;
        t.a.tutorial_highlight = 0;
    }
    return -1;
}

// tutorial step 0x2f: make the blueprint valid, in size and placement.
int tutorial_valid_blueprint(Tutorial &t)
{
    uint8_t &flags = t.a.flags;
    bool bad = flags & kAdvisorBadPlacement, small = flags & kAdvisorBlueprintSmall;
    if (t.last.type == EV_CANCEL_BUILD) {
        t.forget({3, 14, 15, 28, 18});
        t.go(0x20);
    } else if (small && bad) {
        t.message(61);
        t.shown[27] = 0;
        flags &= uint8_t(~(kAdvisorBlueprintSmall | kAdvisorBadPlacement));
        t.a.tutorial_highlight = 0;
        return 1;
    } else if (!small && !bad) {
        t.message(52);
        t.a.forced_advice_id = 0;
        t.a.tutorial_highlight = 0;
        return 1;
    } else if (t.a.forced_advice_id == 4 || bad) {
        t.message(51);
        t.shown[18] = 0;
        flags &= uint8_t(~kAdvisorBadPlacement);
        t.a.tutorial_highlight = 0;
        return 1;
    } else {
        t.shown[18] = 2;
        t.go(0x38);
        flags &= uint8_t(~(kAdvisorBlueprintSmall | kAdvisorBadPlacement));
    }
    return -1;
}

// tutorial step 0x38: accept the blueprint.
int tutorial_accept_blueprint(Tutorial &t)
{
    uint8_t &flags = t.a.flags;
    bool bad = flags & kAdvisorBadPlacement, small = flags & kAdvisorBlueprintSmall;
    if (t.last.type == EV_CANCEL_BUILD) {
        t.forget({3, 14, 15, 28, 18, 27, 19, 20, 21, 29});
        t.go(0x20);
    } else if (small && bad) {
        t.message(61);
        t.shown[27] = 0;
        flags &= uint8_t(~(kAdvisorBlueprintSmall | kAdvisorBadPlacement));
        t.a.tutorial_highlight = 0;
        return 1;
    } else if (small) {
        t.message(52);
        t.shown[27] = 0;
        flags &= uint8_t(~(kAdvisorBlueprintSmall | kAdvisorBadPlacement));
        t.a.tutorial_highlight = 0;
        return 1;
    } else if (t.a.forced_advice_id == 4 || bad) {
        t.message(51);
        t.shown[27] = 0;
        flags &= uint8_t(~kAdvisorBadPlacement);
        t.a.forced_advice_id = 0;
        t.a.tutorial_highlight = 0;
        return 1;
    } else if (t.last.type != EV_COMMIT_BLUEPRINT) {
        if (t.shown[27] == 0) {
            t.message(56);
            t.a.tutorial_highlight = kEditRoomConfirm;
            t.shown[27] = 1;
            flags &= uint8_t(~kAdvisorBadPlacement);
            return 1;
        }
    } else {
        t.shown[27] = 2;
        t.go(0x30);
    }
    return -1;
}

// tutorial step 0x30: place the door.
int tutorial_place_door(Tutorial &t)
{
    if (t.last.type == EV_EDIT_BLUEPRINT) {
        t.forget({3, 14, 15, 28, 18, 27, 19});
        t.go(0x38);
    } else if (t.last.type == EV_PLACE_BP_DOOR && t.world.building.door_invalid) {
        t.message(53);
        t.shown[24] = 1;
        return 1;
    } else if (t.last.type != EV_PLACE_BP_DOOR) {
        if (t.shown[19] == 0) {
            t.message(48);
            t.shown[19] = 1;
            return 1;
        }
    } else {
        t.shown[19] = 2;
        t.go(0x31);
    }
    return -1;
}

// tutorial step 0x31: place the windows.
int tutorial_place_windows(Tutorial &t)
{
    bool placed = t.last.type == EV_PLACE_BP_DOOR;
    if (t.last.type == EV_EDIT_BLUEPRINT) {
        t.forget({3, 14, 15, 28, 18, 27, 19, 20});
        t.go(0x38);
    } else if (placed && t.world.building.door_invalid) {
        t.message(54);
        t.shown[25] = 1;
        return 1;
    } else if (placed) {
        t.message(49);
        t.shown[20] = 1;
        return 1;
    } else if (t.last.type != EV_COMMIT_DOORS) {
        if (t.shown[20] == 0) {
            t.message(49);
            t.a.tutorial_highlight = kEditRoomConfirm;
            t.shown[20] = 1;
            return 1;
        }
    } else {
        t.shown[19] = 2;
        t.go(0x32);
        t.a.flags &= uint8_t(~kAdvisorBadPlacement);
    }
    return -1;
}

// tutorial step 0x32: place the objects the room needs.
int tutorial_furnish_room(Tutorial &t)
{
    if (t.last.type == EV_EDIT_BLUEPRINT) {
        t.forget({3, 14, 15, 28, 18, 27, 19, 20, 21});
        t.go(0x38);
    } else if (t.a.flags & kAdvisorBadPlacement) {
        t.message(55);
        t.a.flags &= uint8_t(~kAdvisorBadPlacement);
        t.shown[21] = 0;
        return 1;
    } else if (!(t.a.flags & kAdvisorRoomFurnished)) {
        if (t.shown[21] == 0) {
            t.message(50);
            t.shown[21] = 1;
            return 1;
        }
    } else {
        t.shown[29] = 0;
        t.go(0x3a);
    }
    return -1;
}

// tutorial step 0x3a: open the room.
int tutorial_open_room(Tutorial &t)
{
    if (t.last.type == EV_EDIT_BLUEPRINT) {
        t.forget({3, 14, 15, 28, 18, 27, 19, 20, 21, 29});
        t.go(0x38);
    } else if (t.last.type != EV_OPEN_ROOM) {
        if (t.shown[29] == 0) {
            t.message(58);
            t.a.tutorial_highlight = kEditRoomConfirm;
            t.shown[29] = 1;
            return 1;
        }
    } else {
        t.shown[29] = 2;
        t.go(0x1f);
    }
    return -1;
}

// tutorial step 0x1f: open the staff hiring, for a doctor. The tutorial ends
// once the hospital has one.
int tutorial_open_hire_doctor(Tutorial &t)
{
    if (t.shown[2] == 2 || count_staff(t.world).doctors != 0) {
        t.shown[2] = 2;
        t.a.tutorial_highlight = 0;
        t.a.flags &= uint8_t(~kAdvisorTutorial);
    } else if (t.last.type == EV_OPEN_HIRE_STAFF) {
        t.shown[2] = 2;
        t.go(0x23);
    } else if (t.shown[2] == 0) {
        t.message(31);
        t.a.tutorial_highlight = 0x15;
        t.shown[2] = 1;
        return 1;
    }
    return -1;
}

// tutorial step 0x23: choose doctors.
int tutorial_select_doctors(Tutorial &t)
{
    bool other = t.last.type == EV_CANCEL_BUILD || (t.last.type == EV_MARK_EMPLOYING && t.last.value != kStaffDoctor);
    if (t.shown[6] == 2) {
        t.go(0x2d);
    } else if (other) {
        t.forget({2, 6});
        t.go(0x1f);
    } else if (t.last.type != EV_HIRE_SELECT_KIND || t.last.value != kStaffDoctor) {
        if (t.shown[6] == 0) {
            t.hint(4, 6);
            return 0;
        }
    } else {
        t.shown[6] = 2;
        t.go(0x2d);
    }
    return -1;
}

// tutorial step 0x2d: hire the doctor.
int tutorial_hire_doctor(Tutorial &t)
{
    if (t.shown[16] == 2) {
        t.go(0x27);
    } else if (t.last.type == EV_CANCEL_BUILD) {
        t.forget({2, 6, 16});
        t.go(0x1f);
    } else if (t.last.type == EV_HIRE_SELECT_KIND && t.last.value != kStaffDoctor) {
        t.shown[6] = 0;
        t.go(0x23);
    } else if (t.last.type != EV_MARK_EMPLOYING) {
        if (t.shown[16] == 0) {
            t.hint(5, 16);
            return 0;
        }
    } else {
        t.shown[16] = 2;
        t.go(0x27);
    }
    return -1;
}

// tutorial step 0x27: place the doctor. That ends the tutorial, with the
// level's last help page.
int tutorial_place_doctor(Tutorial &t)
{
    if (t.shown[10] == 2) {
        t.a.tutorial_highlight = 0;
        t.a.flags &= uint8_t(~kAdvisorTutorial);
    } else if (t.last.type == EV_CANCEL_BUILD) {
        t.forget({10, 2, 6, 16});
        t.go(0x1f);
    } else if (!t.in.staff_place_ok) {
        t.message(59);
        t.shown[10] = 0;
        return 1;
    } else if (count_staff(t.world).doctors == 0) {
        if (t.shown[10] == 0) {
            t.message(39);
            t.shown[10] = 1;
            return 1;
        }
    } else {
        t.shown[10] = 2;
        t.a.tutorial_highlight = 0;
        t.a.flags &= uint8_t(~kAdvisorTutorial);
        if (t.in.help_page)
            *t.in.help_page = kTutorialEndPage;
    }
    return -1;
}

// The tutorial's steps (advisor.tutorial_step) and what runs them.
struct TutorialStep {
    uint8_t step;
    int (*run)(Tutorial &);
};

constexpr std::array<TutorialStep, 24> kTutorialSteps = {{
    {0x1d, tutorial_open_furnish_corridor}, {0x28, tutorial_buy_reception_desk}, {0x2a, tutorial_confirm_purchase},
    {0x29, tutorial_place_reception_desk},  {0x1e, tutorial_open_hire_staff},    {0x21, tutorial_select_receptionists},
    {0x25, tutorial_next_applicant},        {0x26, tutorial_previous_applicant}, {0x22, tutorial_hire_applicant},
    {0x24, tutorial_place_receptionist},    {0x20, tutorial_open_build_room},    {0x2b, tutorial_choose_department},
    {0x2c, tutorial_choose_gp_office},      {0x39, tutorial_drag_blueprint},     {0x2f, tutorial_valid_blueprint},
    {0x38, tutorial_accept_blueprint},      {0x30, tutorial_place_door},         {0x31, tutorial_place_windows},
    {0x32, tutorial_furnish_room},          {0x3a, tutorial_open_room},          {0x1f, tutorial_open_hire_doctor},
    {0x23, tutorial_select_doctors},        {0x2d, tutorial_hire_doctor},        {0x27, tutorial_place_doctor},
}};

// tutorial (src/tutorial.cpp), in high resolution, where the hints about
// controls are help text (set_force_help, not ported) and the advisor is
// hidden. Returns 1 when a message was just given, 0 when he is to be
// hidden, -1 otherwise.
int tutorial(Advisor &a, const AdvisorInputs &in)
{
    trace_event("tutorial %u", unsigned(a.tutorial_step));
    // process_turn leaves a deleted room or a cancelled room edit as EV_CANCEL_BUILD in
    // old_packet (src/io.cpp:1478-1489).
    Event last = in.last;
    if (last.type == EV_DELETE_ROOM || last.type == EV_CANCEL_EDIT_ROOM)
        last = {EV_CANCEL_BUILD, last.type == EV_DELETE_ROOM ? last.value : uint16_t(0), last.flags, last.arg};
    Tutorial t{a,
               in,
               *in.world,
               a.tutorial_shown,
               last,
               in.pending,
               last.type == EV_CANCEL_BUILD || (last.type == EV_MARK_EMPLOYING && last.value != kStaffReceptionist)};
    for (const TutorialStep &step : kTutorialSteps)
        if (step.step == a.tutorial_step)
            return step.run(t);
    return -1;
}

// advisor_character (src/advtext.cpp) without its drawing.
void advisor_character(Advisor &a, const AdvisorInputs &in, const AnimData &anims, uint32_t &safeseed)
{
    auto saferand = [&] {
        safeseed = std::rotr(safeseed * 0x24a1u + 0x24dfu, 13);
        return uint16_t(safeseed);
    };
    // get_advisor
    if (a.advice_id && a.anim_state == 0)
        enter_state(a, anims, 1);
    uint32_t speed = 0;
    bool advance_at_end = false;
    switch (a.anim_state) {
    case 1:
    case 4:
    case 5:
    case 6:
    case 8:
        speed = 1;
        advance_at_end = true;
        a.state_advanced = false;
        break;
    case 9:
    case 10:
        speed = 4;
        advance_at_end = true;
        break;
    case 2:
    case 3:
    case 7:
        if (a.anim_state == 2)
            a.state_advanced = false;
        if (a.anim_state != 7 && saferand() % 50 == 0)
            enter_state(a, anims, kAdvisorSpecials[saferand() % 2]);
        speed = 2;
        break;
    default:
        break;
    }
    a.drawn = speed != 0;
    if (!speed)
        return;
    a.drawn_frame = a.anim_frame;
    if (in.game_turn % speed == 0 && !in.paused && a.anim_frame < anims.frames.size()) {
        a.anim_frame = anims.frames[a.anim_frame].next;
        if (advance_at_end && a.anim_frame < anims.frames.size() && (anims.frames[a.anim_frame].flags & 1))
            enter_state(a, anims, kNextAdvisorState[a.anim_state]);
    }
}

// format_advisor_text (src/advtext.cpp): wraps the message into a bubble
// `segments` middle pieces wide, if it fits in three lines.
void format_advisor_text(Advisor &a, const Font &font, int segments)
{
    int max_width = segments * kMiddleTextWidth + kRightTextWidth + kLeftTextWidth - kTextLeftMargin - kTextRightMargin;
    // The game indexes the font by character - 31, past its start for
    // control characters; they count nothing here.
    auto width_of = [&](uint8_t c) { return c == ' ' ? font.space_width() : c > ' ' ? font.glyph(c).width : 0; };
    const std::string &text = a.text;
    size_t end = text.size();
    std::string wrapped;
    size_t p = 0, brk = 0, line_start = 0;
    int width = 0;
    int lines = 0;
    do {
        do {
            width += width_of(uint8_t(text[p]));
            char c = text[p];
            if (c == ' ' || c == ',' || c == '.' || c == ';' || c == ':')
                brk = p;
            p++;
        } while (width <= max_width && p != end);
        if (p == end)
            brk = p;
        width = 0;
        lines++;
        wrapped.append(text, line_start, brk - line_start);
        wrapped += '\n';
        if (p != end) {
            p = brk + 1;
            brk = p;
            line_start = p;
        }
    } while (lines < 3 && p != end);
    if (p == end) {
        a.text = wrapped;
        a.line_count = uint8_t(lines);
        a.extra_segments = uint8_t(segments - kAdvisorMinSegments);
    }
}

// advisor_frame's ageing of the advice given: whether it is old enough to
// give way to other advice, and ending it when its time is up.
bool advisor_frame_advice_stale(Advisor &a, const AdvisorInputs &in)
{
    if (!a.advice_id)
        return false;
    Advice &shown = a.advice[size_t(a.advice_id)];
    if (in.requester_open) {
        shown.time = in.game_turn - shown.duration / 4;
        return false;
    }
    bool stale = false;
    // 16 bits, as the game keeps it.
    uint16_t elapsed = uint16_t(in.game_turn - shown.time);
    if (elapsed > 100 || a.dismiss)
        stale = true;
    if (elapsed >= shown.duration || a.dismiss)
        a.advice_id = 0;
    return stale;
}

// advisor_frame's search for the first advice due and fitting the
// situation.
void advisor_frame_find_advice(Advisor &a, const AdvisorInputs &in, uint32_t &safeseed)
{
    for (int i = 1; a.advice[size_t(i)].id; i++) {
        const Advice &advice = a.advice[size_t(i)];
        if (in.game_turn > advice.time &&
            uint32_t(advice.times_given * advice.repeat_weight << 10) / (in.game_turn - advice.time) < 10 &&
            find_advice(a, i, in, safeseed)) {
            a.rewrap = true;
            return;
        }
    }
}

// advisor_frame's bubble, or his going down, while he is up.
void advisor_frame_bubble(Advisor &a, const AnimData &anims)
{
    if (a.advice_id && !a.dismiss) {
        a.bubble = true;
    } else if (--a.time_left <= 0 || a.dismiss) {
        a.dismiss = false;
        a.time_left = -1;
        enter_state(a, anims, kNextAdvisorState[a.anim_state]);
        a.state_advanced = false;
    } else if (!a.state_advanced) {
        enter_state(a, anims, kNextAdvisorState[a.anim_state]);
        a.state_advanced = true;
        a.dismiss = false;
    }
}

} // namespace

StaffCounts count_staff(const World &world)
{
    StaffCounts c;
    for (const Thing &t : world.things) {
        if (t.type != kThingStaff || t.owner != 0 || t.state == kStaffApplicant)
            continue;
        c.total++;
        c.by_kind[t.subtype == kStaffSurgeon ? kStaffDoctor : t.subtype]++;
        if (t.skills & kSkillReceptionist)
            c.receptionists++;
        if (t.skills & kSkillDoctor)
            c.doctors++;
    }
    return c;
}

void start_advisor(Advisor &a, bool tutorial)
{
    // reset_advisor
    a.advice = kAdviceTable;
    a.advice_id = 0;
    a.time_left = 0;
    a.forced_advice_id = 0;
    a.mouse_advice_id = 0;
    a.text.clear();
    a.event_flags = 0;
    // setup_new_level and the area screen.
    a.tutorial_shown = {};
    a.tutorial_step = 0x1d;
    a.tutorial_highlight = 0;
    a.flags = tutorial ? kAdvisorTutorial : 0;
    a.anim_state = 0;
    a.anim_frame = 0;
    a.state_advanced = false;
    a.line_count = 0;
    a.extra_segments = 0;
    a.rewrap = false;
    a.dismiss = false;
    a.drawn = false;
    a.bubble = false;
}

void advisor_frame(Advisor &a, const AdvisorInputs &in, const AnimData &anims, uint32_t &safeseed)
{
    a.drawn = false;
    a.bubble = false;
    // What drawing the blueprint and setting the edit room requester leave in
    // the flags, before the frame's advisor.
    a.flags = blueprint_draw_flags(*in.world, a.flags);
    a.flags = in.world->placing.furnished ? uint8_t(a.flags | kAdvisorRoomFurnished)
                                          : uint8_t(a.flags & ~kAdvisorRoomFurnished);
    if (!a.enabled)
        return;
    if (in.object_place_ok) {
        if (*in.object_place_ok)
            a.flags &= ~kAdvisorBadPlacement;
        else
            a.flags |= kAdvisorBadPlacement;
    }
    if (a.flags & kAdvisorTutorial) {
        if (tutorial(a, in) == 0 || a.text.empty())
            return;
        a.advice_id = 1;
        advisor_character(a, in, anims, safeseed);
        a.bubble = kAdvisorUpStates[a.anim_state];
        return;
    }

    if (advisor_frame_advice_stale(a, in) || a.advice_id == 0)
        advisor_frame_find_advice(a, in, safeseed);
    advisor_character(a, in, anims, safeseed);
    if (!kAdvisorUpStates[a.anim_state])
        return;
    advisor_frame_bubble(a, anims);
}

void monthly_update_of_advisor_counts(Advisor &a)
{
    a.advice[14].times_given = 0;
    a.advice[15].times_given = 0;
}

void wrap_advisor_text(Advisor &a, const Font &font)
{
    a.no_pieces = kAdvisorMinSegments;
    do {
        if (!a.text.empty())
            format_advisor_text(a, font, a.no_pieces);
        a.no_pieces++;
    } while (a.no_pieces <= kAdvisorMaxSegments && !a.line_count);
    a.rewrap = false;
}

bool advisor_hit(const Advisor &a, int x, int y)
{
    if (!kAdvisorUpStates[a.anim_state])
        return false;
    // The bubble's left edge is compared unsigned, as in the game.
    return (y >= kBubbleTop && uint16_t(a.box_left) <= x && x <= kBubbleRight && y <= kBubbleBottom) ||
           (x >= kAdvisorLeft && y >= kAdvisorTop && x <= kAdvisorRight && y <= kAdvisorBottom);
}

void draw_advisor(UiLayer &layer, Advisor &a, const GameData &data, const Font &font)
{
    if (a.drawn) {
        // draw_a_frame
        const AnimData &anims = data.anims();
        int y = panel_top(data);
        if (a.drawn_frame < anims.frames.size()) {
            for (size_t i = anims.frames[a.drawn_frame].list; i < anims.lists.size() && anims.lists[i] != kListEnd;
                 i++) {
                uint16_t id = anims.lists[i];
                if (id >= anims.elements.size() || anims.elements[id].sprite == 0)
                    continue;
                const AnimElement &e = anims.elements[id];
                layer.draw_sprite(kAdvisorX + e.x - kElementBiasX, y + e.y - kElementBiasY,
                                  data.sprites().at(e.sprite / 6), e.flags & 0xf, data.ghost().data());
            }
        }
    }
    if (!a.bubble || a.text.empty())
        return;
    // advisor_text_box
    if (a.rewrap)
        wrap_advisor_text(a, font);
    // The game leaves y unset for a message not wrapped (no lines).
    int y = a.line_count == 3 ? kTextY3 : a.line_count == 2 ? kTextY2 : kTextY1;
    if (a.no_pieces > kAdvisorMaxSegments)
        return;
    const SpriteSheet &panel = data.panel();
    Sprite left = panel.at(kBubbleLeft);
    Sprite middle = panel.at(kBubbleMiddle);
    Sprite right = panel.at(kBubbleRightEnd);
    int pieces = a.extra_segments + kAdvisorMinSegments;
    layer.draw_sprite(kRightPieceX, kRightPieceY, right);
    layer.draw_sprite(kRightPieceX - left.width - pieces * middle.width, kBubbleY, left);
    for (int i = 0; i < pieces; i++)
        layer.draw_sprite(kRightPieceX - (pieces - i) * middle.width, kBubbleY, middle);
    // As the game has it: one more middle piece and the right end's width.
    a.box_left = int16_t(kRightPieceX - middle.width * (pieces + 1) - right.width);
    layer.draw_text(font, kRightPieceX - pieces * middle.width + kTextLeftMargin - left.width, y, a.text,
                    {{0, 0, kUiWidth}, {0, 0, kUiWidth, kViewHeight}});
}

} // namespace th
