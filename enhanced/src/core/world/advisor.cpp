#include "core/world/advisor.h"

#include "core/data/gamedata.h"
#include "core/render/panel.h"
#include "core/world/objects.h"
#include "core/world/staff.h"
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
constexpr uint16_t kCriterionValue = 6;

// The game's OBJ_RECEPTION_DESK.
constexpr uint8_t kObjectReceptionDesk = 11;

// The player's hired staff, as the game counts them as they are hired
// (allhosps.staff_count, staff_by_kind with surgeons as doctors, and
// listhead.staff_by_skill).
struct StaffCounts {
    int total = 0;
    std::array<int, 5> by_kind{};
    int receptionists = 0; // staff_by_skill[0]
};

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
    }
    return c;
}

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
        // No staff room can be built yet.
        if (count_staff(*in.world).total <= in.level + 5)
            return false;
        say(a, advisor_text(in, 0x49));
        return true;
    case 0x69:
        if (!has_reception(*in.world) || count_staff(*in.world).by_kind[kStaffReceptionist] != 0 || in.level != 1)
            return false;
        say(a, advisor_text(in, 0x69));
        return true;
    case 0x4c:
        return find_advice_deaths(a, in, safeseed);
    case 0x51:
        return find_advice_cures(a, in, safeseed);
    case 0x1c:
        if (a.given[10] || !(a.event_flags & kAdvisorEventResign))
            return false;
        say(a, advisor_text(in, 0x1c));
        a.given[10] = 1;
        a.event_flags &= ~kAdvisorEventResign;
        return true;
    default:
        return false;
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

// tutorial step 0x20: open the room building. No GP's office can be built
// yet. Steps 0x2b on (building it) and 0x1f on (hiring a doctor) are not
// ported.
int tutorial_open_build_room(Tutorial &t)
{
    if (t.shown[3] == 2)
        return -1;
    if (t.last.type == EV_OPEN_BUILD_ROOM) {
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

// tutorial (src/tutorial.cpp) to step 0x20, in high resolution, where the
// hints about controls are help text (set_force_help, not ported) and the
// advisor is hidden. Returns 1 when a message was just given, 0 when he is
// to be hidden, -1 otherwise.
int tutorial(Advisor &a, const AdvisorInputs &in)
{
    const Event &last = in.last;
    Tutorial t{a,
               in,
               *in.world,
               a.tutorial_shown,
               last,
               in.pending,
               last.type == EV_CANCEL_BUILD || (last.type == EV_MARK_EMPLOYING && last.value != kStaffReceptionist)};
    switch (a.tutorial_step) {
    case 0x1d:
        return tutorial_open_furnish_corridor(t);
    case 0x28:
        return tutorial_buy_reception_desk(t);
    case 0x2a:
        return tutorial_confirm_purchase(t);
    case 0x29:
        return tutorial_place_reception_desk(t);
    case 0x1e:
        return tutorial_open_hire_staff(t);
    case 0x21:
        return tutorial_select_receptionists(t);
    case 0x25:
        return tutorial_next_applicant(t);
    case 0x26:
        return tutorial_previous_applicant(t);
    case 0x22:
        return tutorial_hire_applicant(t);
    case 0x24:
        return tutorial_place_receptionist(t);
    case 0x20:
        return tutorial_open_build_room(t);
    default:
        return -1;
    }
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

void start_advisor(Advisor &a, bool tutorial)
{
    // reset_advisor
    a.advice = kAdviceTable;
    a.advice_id = 0;
    a.time_left = 0;
    a.forced_advice_id = 0;
    a.mouse_advice_id = 0;
    a.text.clear();
    a.has_patients = false;
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
