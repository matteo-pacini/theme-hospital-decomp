#include "core/ui/helpmsg.h"

#include "core/data/balance.h"
#include "core/data/text.h"
#include "core/ui/charts.h"
#include "core/ui/hire_staff.h"
#include "core/ui/info_windows.h"
#include "core/world/arrivals.h"
#include "core/world/build.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/objects.h"
#include "core/world/queue.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/world.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

namespace th
{
namespace
{

// The game's static gbv.diagnosis_complete (src/array.cpp).
constexpr int kDiagnosisComplete = 1000;
// The object kind and room type the casebook's cure texts test (the game's
// OBJ_PHARMACY_CABINET and ROOM_THEATRE, include/enums.h).
constexpr uint8_t kObjPharmacyCabinet = 39;
constexpr uint8_t kRoomTheatre = 10;

// Sets the tooltip to string `id` of `section`.
void set_section(HelpContext &ctx, TextSection section, int id)
{
    if (ctx.help && ctx.text)
        ctx.help->msg = std::string(ctx.text->at(section, id));
}

// Sets the tooltip to the hotspot's plain help_text entry, the fallback the
// original leaves for state that is not ported.
void set_plain(HelpContext &ctx)
{
    if (ctx.help && ctx.hotspot && ctx.text)
        ctx.help->msg = std::string(ctx.text->at(TextSection::Help, ctx.hotspot->text()));
}

// snprintf of `format` with its integer arguments, as the original's sprintf.
std::string format_ints(std::string_view format, int a, int b)
{
    char buf[512];
    std::snprintf(buf, sizeof buf, std::string(format).c_str(), a, b);
    return buf;
}

// snprintf of `format` with one string argument.
std::string format_name(std::string_view format, std::string_view name)
{
    char buf[512];
    std::snprintf(buf, sizeof buf, std::string(format).c_str(), std::string(name).c_str());
    return buf;
}

// `id` if it is a thing of `ctx.world`, as the game's `candidate > thing`.
const Thing *shown_thing(const HelpContext &ctx, uint16_t id)
{
    if (!ctx.world || id == 0 || id >= ctx.world->things.size())
        return nullptr;
    return &ctx.world->things[id];
}

// get_slist_staff's kind test for the staff list: a hired member of staff of
// player 0 whose kind the filter shows.
bool slist_shows(const Thing &t, int filter)
{
    if (t.type != kThingStaff || t.owner != 0 || t.state == kStaffApplicant)
        return false;
    return ((t.subtype == kStaffDoctor || t.subtype == kStaffSurgeon) && (filter & 1)) ||
           (t.subtype == kStaffNurse && (filter & 2)) || (t.subtype == kStaffReceptionist && (filter & 8)) ||
           (t.subtype == kStaffHandyman && (filter & 4));
}

// othervar.slist_rows: the listed staff on the current page.
int slist_rows(const HelpContext &ctx)
{
    if (!ctx.charts || !ctx.world)
        return 0;
    int listed = 0, rows = 0;
    for (const Thing &t : ctx.world->things) {
        if (!slist_shows(t, ctx.charts->slist_filter))
            continue;
        if (listed >= ctx.charts->slist_page * 10 && listed < (ctx.charts->slist_page + 1) * 10)
            rows++;
        listed++;
    }
    return rows;
}

// Status screen: a hospital's figures the criteria measure (AllHosps).
struct Figures {
    int reputation = 500;
    int32_t balance = 0;
    int32_t loan = 0;
    int32_t value = 0;
    int visitors = 0, diagnosed = 0, cures = 0, deaths = 0;
};

// AllHosps `hospital`'s figures, as the status screen values them.
Figures shown_figures(const HelpContext &ctx, int hospital)
{
    Figures f;
    if (!ctx.game)
        return f;
    if (hospital != 0) {
        const Balance &bal = ctx.game->settings();
        f.balance = int32_t(bal.towns[size_t(ctx.game->level_number()) % bal.towns.size()].start_cash);
        return f;
    }
    f.reputation = ctx.game->reputation();
    f.balance = ctx.game->balance;
    f.loan = ctx.game->finance.loan;
    f.value = ctx.game->finance.hospital_value;
    f.cures = ctx.game->finance.cures;
    f.deaths = ctx.game->finance.deaths;
    f.visitors = int(ctx.game->arrivals.hospitals[0].visitors);
    return f;
}

// A criterion as check_criteria_values (src/sim.cpp) leaves it.
struct Criterion {
    int criterion = 0, op = 0, group = 0;
    int32_t value = 0, base = 0, current = 0;
    bool met = false;
    int progress = 0;
};

// check_criteria_values: win (mode 0) or lose criteria measured on `h`.
std::array<Criterion, 10> check_criteria(const std::array<Balance::WinLose, 10> &table, int mode, const Figures &h,
                                         int *groups)
{
    std::array<Criterion, 10> out{};
    int max = 0;
    for (size_t i = 0; i < 10; i++) {
        Criterion &c = out[i];
        c.criterion = table[i].criteria;
        c.op = table[i].max_min;
        c.value = table[i].value;
        c.base = table[i].bound;
        c.group = table[i].group;
        max = std::max(max, c.group);
        int32_t val = 0;
        switch (c.criterion) {
        case 0:
            c.met = true;
            break;
        case 1:
            val = h.reputation;
            break;
        case 2:
            val = mode == 0 ? h.balance - h.loan : h.balance;
            break;
        case 3:
            val = h.visitors > 10 ? h.diagnosed * 100 / h.visitors : 0;
            break;
        case 4:
            val = h.cures;
            break;
        case 5:
            val = h.visitors > 10 ? h.deaths * 100 / h.visitors : 0;
            break;
        case 6:
            val = h.value;
            break;
        }
        if (c.op == 1 ? val >= c.value : val <= c.value)
            c.met = true;
        int32_t range = c.value - c.base, pct = 0;
        if (range != 0)
            pct = (val - c.base) * 100 / range;
        c.progress = std::clamp(pct, 0, 200);
        c.current = val;
    }
    if (groups)
        *groups = max;
    return out;
}

// evaluate_criteria, one criterion's share of its group's score.
int criterion_score(const Criterion &c)
{
    if (c.met)
        return 100;
    int pct = 0;
    if (c.op == 1) {
        if (c.value != 0) {
            pct = std::abs(c.current * 100 / c.value);
            if (c.criterion == 2 && c.current < 0)
                pct = 0;
        }
    } else if (c.current != 0) {
        pct = std::abs(c.value * 100 / c.current);
    }
    return std::clamp(pct, 0, 100);
}

// evaluate_criteria, the player's best win criteria group, 0 for none.
int best_win_group(const std::array<Criterion, 10> &win, int groups)
{
    int best = 0, best_group = 0;
    for (int group = 1; group <= groups; group++) {
        int count = 0, sum = 0;
        for (const Criterion &c : win) {
            if (c.group != group)
                continue;
            count++;
            sum += criterion_score(c);
        }
        if (count && sum / count > best) {
            best = sum / count;
            best_group = group;
        }
    }
    return best_group;
}

// win_criteria_help index for criterion `which`.
int win_criteria_text(int which)
{
    switch (which) {
    case 2:
        return 1;
    case 3:
        return 6;
    case 4:
        return 3;
    case 5:
        return 4;
    case 6:
        return 5;
    default:
        return 0;
    }
}

// The figure the hovered criteria bar shows beside its target.
int win_criteria_current(int which, const Figures &h)
{
    switch (which) {
    case 1:
        return h.reputation;
    case 2:
        return h.balance - h.loan;
    case 3:
        return h.visitors > 10 ? h.diagnosed * 100 / h.visitors : 0;
    case 4:
        return h.cures;
    case 5:
        return h.visitors > 10 ? h.deaths * 100 / h.visitors : 0;
    default:
        return h.value;
    }
}

// GPIO-style mask test: `skills` has all bits of `mask`.
bool has_skills(uint16_t skills, uint16_t mask)
{
    return (skills & mask) == mask;
}

} // namespace

void help_default_text(HelpContext &ctx)
{
    set_plain(ctx);
}

// ---- Patient window (req3, src/req3.cpp) ----

void req3_help_if_diagnosed(HelpContext &ctx)
{
    const Thing *patient = ctx.info ? shown_thing(ctx, ctx.info->patient) : nullptr;
    if (patient && patient->diagnosis >= kDiagnosisComplete)
        set_plain(ctx);
}

void req3_help_if_curable(HelpContext &ctx)
{
    const Thing *patient = ctx.info ? shown_thing(ctx, ctx.info->patient) : nullptr;
    if (patient && patient->treatment != 0 && !(patient->person_flags & kPfGuessed) &&
        !(patient->person_flags & kPfFedUp) && patient->illness != patient->known_illness &&
        patient->diagnosis >= kDiagnosisComplete)
        set_plain(ctx);
}

void req3_help_if_queueing(HelpContext &ctx)
{
    const Thing *patient = ctx.info ? shown_thing(ctx, ctx.info->patient) : nullptr;
    if (patient && patient->queue_room)
        set_plain(ctx);
}

// ---- Bank manager (manager.cpp) ----

void manager_help_pointed_ins(HelpContext &ctx)
{
    if (!ctx.help || !ctx.world || !ctx.text || !ctx.hotspot)
        return;
    size_t idx = ctx.hotspot->req_slot();
    if (idx >= ctx.world->ins_control.size())
        return;
    ctx.help->msg = format_name(ctx.text->at(TextSection::Help, ctx.hotspot->text()),
                                ctx.text->at(TextSection::InsuranceNames, ctx.world->ins_control[idx]));
}

void manager_help_displayed_ins(HelpContext &ctx)
{
    if (!ctx.help || !ctx.world || !ctx.text || !ctx.charts || !ctx.hotspot)
        return;
    int value = ctx.charts->value;
    if (value < 1 || size_t(value) > ctx.world->ins_control.size())
        return;
    ctx.help->msg = format_name(ctx.text->at(TextSection::Help, ctx.hotspot->text()),
                                ctx.text->at(TextSection::InsuranceNames, ctx.world->ins_control[size_t(value - 1)]));
}

// ---- Status screen (overscr.cpp) ----

void overscr_help_add_name(HelpContext &ctx)
{
    if (!ctx.help || !ctx.charts || !ctx.text || !ctx.hotspot)
        return;
    size_t owner = ctx.hotspot->req_slot();
    if (owner >= ctx.charts->hospital_names.size())
        return;
    ctx.help->msg =
        format_name(ctx.text->at(TextSection::Help, ctx.hotspot->text()), ctx.charts->hospital_names[owner]);
}

// The first lose criterion `crit` with progress shown, 0 if none.
const Criterion *lose_bar(const std::array<Criterion, 10> &lose, int crit)
{
    for (const Criterion &c : lose)
        if (c.criterion == crit && c.progress)
            return &c;
    return nullptr;
}

// The idx-th criteria bar shown: its criterion `which` and target value, as
// the original scans the lose criteria (with progress) then the player's best
// win group's. False if the screen holds fewer bars.
bool find_criteria_bar(const std::array<Criterion, 10> &lose, const std::array<Criterion, 10> &win, int best, int idx,
                       int &which, int32_t &target)
{
    int count = 0;
    for (int crit = 0; crit <= 6; crit++) {
        const Criterion *l = lose_bar(lose, crit);
        if (l) {
            if (count++ == idx) {
                which = crit;
                target = l->value;
                return true;
            }
            continue;
        }
        for (const Criterion &c : win) {
            if (c.criterion != crit || c.group != best)
                continue;
            if (count++ == idx) {
                which = crit;
                target = c.value;
                return true;
            }
        }
    }
    return false;
}

void overscr_help_win_criteria(HelpContext &ctx)
{
    if (!ctx.help || !ctx.charts || !ctx.text || !ctx.game || !ctx.hotspot)
        return;
    const Balance &bal = ctx.game->settings();
    int groups = 0;
    auto player_win = check_criteria(bal.win_criteria, 0, shown_figures(ctx, 0), &groups);
    int best = best_win_group(player_win, groups);
    Figures shown = shown_figures(ctx, ctx.charts->value);
    auto win = check_criteria(bal.win_criteria, 0, shown, nullptr);
    auto lose = check_criteria(bal.lose_criteria, 1, shown, nullptr);

    int which = 0;
    int32_t target = 0;
    if (!find_criteria_bar(lose, win, best, ctx.hotspot->req_slot(), which, target))
        return;
    ctx.help->msg = format_ints(ctx.text->at(TextSection::WinCriteriaHelp, win_criteria_text(which)), int(target),
                                win_criteria_current(which, shown));
}

// ---- Panel (panel.cpp) ----

void panel_rep_help(HelpContext &ctx)
{
    if (!ctx.help || !ctx.text || !ctx.game)
        return;
    ctx.help->msg =
        std::string(ctx.text->at(TextSection::Help, 9)) + " (" + std::to_string(ctx.game->reputation()) + ")";
}

void panel_watch_help(HelpContext &ctx)
{
    if (!ctx.help || !ctx.game || !ctx.world || !ctx.text)
        return;
    if (ctx.world->emergency.timer != 0)
        set_section(ctx, TextSection::Help, 0xee);
    else if (ctx.game->opening_countdown != 0)
        set_section(ctx, TextSection::Help, 0xed);
}

// ---- Build room requester (req10.cpp) ----

void req10_help_room(HelpContext &ctx)
{
    if (!ctx.placing || !ctx.hotspot)
        return;
    size_t slot = ctx.hotspot->req_slot();
    if (slot >= ctx.placing->offered.size())
        return;
    if (uint8_t idx = ctx.placing->offered[slot])
        set_section(ctx, TextSection::RoomHelp, idx);
}

// ---- Buy objects requester (req11.cpp) ----

void req11_help_object(HelpContext &ctx)
{
    if (!ctx.placing || !ctx.hotspot)
        return;
    size_t slot = ctx.hotspot->req_slot();
    if (slot >= ctx.placing->offered.size())
        return;
    if (uint8_t idx = ctx.placing->offered[slot])
        set_section(ctx, TextSection::ObjectHelp, idx);
}

void req11_help_plus(HelpContext &ctx)
{
    if (!ctx.placing || !ctx.hotspot)
        return;
    size_t slot = ctx.hotspot->req_slot();
    if (slot < ctx.placing->offered.size() && ctx.placing->offered[slot])
        set_section(ctx, TextSection::Help, 0x29);
}

void req11_help_minus(HelpContext &ctx)
{
    if (!ctx.placing || !ctx.hotspot)
        return;
    size_t slot = ctx.hotspot->req_slot();
    if (slot < ctx.placing->offered.size() && ctx.placing->offered[slot])
        set_section(ctx, TextSection::Help, 0x2a);
}

// ---- Demand requester (req12.cpp; the requester is not ported) ----

void req12_help_if_doctor(HelpContext &ctx)
{
    set_plain(ctx);
}
void req12_help_if_surgeon(HelpContext &ctx)
{
    set_plain(ctx);
}
void req12_help_if_shrink(HelpContext &ctx)
{
    set_plain(ctx);
}
void req12_help_if_research(HelpContext &ctx)
{
    set_plain(ctx);
}

// ---- Hire staff requester (req2.cpp) ----

void req2_help_if_doctor(HelpContext &ctx)
{
    const Thing *c = ctx.hire ? shown_thing(ctx, ctx.hire->shown) : nullptr;
    if (c && (c->skills & kSkillDoctor))
        set_plain(ctx);
}

void req2_help_if_surgeon(HelpContext &ctx)
{
    const Thing *c = ctx.hire ? shown_thing(ctx, ctx.hire->shown) : nullptr;
    if (c && has_skills(c->skills, kSkillDoctor | kSkillSurgeon))
        set_plain(ctx);
}

void req2_help_if_psycho(HelpContext &ctx)
{
    const Thing *c = ctx.hire ? shown_thing(ctx, ctx.hire->shown) : nullptr;
    if (c && has_skills(c->skills, kSkillDoctor | kSkillPsychiatrist))
        set_plain(ctx);
}

void req2_help_if_researcher(HelpContext &ctx)
{
    const Thing *c = ctx.hire ? shown_thing(ctx, ctx.hire->shown) : nullptr;
    if (c && has_skills(c->skills, kSkillDoctor | kSkillResearcher))
        set_plain(ctx);
}

void req2_help_if_selected(HelpContext &ctx)
{
    if (ctx.hire && ctx.hire->shown)
        set_plain(ctx);
}

// ---- Staff and handyman windows (req4.cpp) ----

void req4_help_if_doctor(HelpContext &ctx)
{
    const Thing *s = ctx.info ? shown_thing(ctx, ctx.info->staff) : nullptr;
    if (s && (s->skills & kSkillDoctor))
        set_plain(ctx);
}

void req4_help_if_surgeon(HelpContext &ctx)
{
    const Thing *s = ctx.info ? shown_thing(ctx, ctx.info->staff) : nullptr;
    if (s && (s->skills & kSkillSurgeon))
        set_plain(ctx);
}

void req4_help_if_shrink(HelpContext &ctx)
{
    const Thing *s = ctx.info ? shown_thing(ctx, ctx.info->staff) : nullptr;
    if (s && (s->skills & kSkillPsychiatrist))
        set_plain(ctx);
}

void req4_help_if_research(HelpContext &ctx)
{
    const Thing *s = ctx.info ? shown_thing(ctx, ctx.info->staff) : nullptr;
    if (s && (s->skills & kSkillResearcher))
        set_plain(ctx);
}

// ---- Staff list (slist.cpp) ----

void slist_salary_help_if_present(HelpContext &ctx)
{
    if (ctx.hotspot && int(ctx.hotspot->req_slot()) < slist_rows(ctx))
        set_section(ctx, TextSection::Help, 50);
}

void slist_happiness_help_if_present(HelpContext &ctx)
{
    if (ctx.hotspot && int(ctx.hotspot->req_slot()) < slist_rows(ctx))
        set_section(ctx, TextSection::Help, 51);
}

void slist_tiredness_help_if_present(HelpContext &ctx)
{
    if (ctx.hotspot && int(ctx.hotspot->req_slot()) < slist_rows(ctx))
        set_section(ctx, TextSection::Help, 52);
}

void slist_ability_help_if_present(HelpContext &ctx)
{
    if (ctx.hotspot && int(ctx.hotspot->req_slot()) < slist_rows(ctx))
        set_section(ctx, TextSection::Help, 53);
}

void slist_help_if_doctor(HelpContext &ctx)
{
    if (ctx.charts && (ctx.charts->slist_filter & 1))
        set_plain(ctx);
}

// The surgeon, psychiatrist and researcher entries: the qualification's text
// when the shown doctor has it. The original's else writes their training
// progress only while it is nonzero; those fields are not ported (they read
// 0), so it writes nothing, as here.
void slist_help_if_surgeon(HelpContext &ctx)
{
    const Thing *staff = ctx.charts ? shown_thing(ctx, ctx.charts->value) : nullptr;
    if (ctx.charts && (ctx.charts->slist_filter & 1) && staff && (staff->skills & kSkillSurgeon))
        set_plain(ctx);
}

void slist_help_if_shrink(HelpContext &ctx)
{
    const Thing *staff = ctx.charts ? shown_thing(ctx, ctx.charts->value) : nullptr;
    if (ctx.charts && (ctx.charts->slist_filter & 1) && staff && (staff->skills & kSkillPsychiatrist))
        set_plain(ctx);
}

void slist_help_if_researcher(HelpContext &ctx)
{
    const Thing *staff = ctx.charts ? shown_thing(ctx, ctx.charts->value) : nullptr;
    if (ctx.charts && (ctx.charts->slist_filter & 1) && staff && (staff->skills & kSkillResearcher))
        set_plain(ctx);
}

// ---- Casebook (drugs.cpp) ----

void drugs_help_concentrate(HelpContext &ctx)
{
    // othervar.concentrate_help_on is not ported (nothing sets it and the
    // casebook clears it each frame): the original would rarely write.
    set_plain(ctx);
}

void drugs_help_cure_type(HelpContext &ctx)
{
    if (!ctx.help || !ctx.charts || !ctx.text || !ctx.world || !ctx.game)
        return;
    int sel = ctx.charts->selected_illness;
    if (sel >= 0x24 || sel == 1)
        return;
    const ExpertiseInfo &def = kExpertiseData[size_t(sel)];
    if (def.kind == kObjPharmacyCabinet) {
        if (ctx.world->cure_available[size_t(sel)] == 1) {
            int effect = ctx.game->hospital.illnesses[size_t(sel)].drug_effectiveness;
            ctx.help->msg = format_ints(ctx.text->at(TextSection::Help, 148), effect, 0);
        } else {
            set_section(ctx, TextSection::Help, 147);
        }
    } else if (def.staff_mask == 0x40) {
        set_section(ctx, TextSection::Help, 149);
    } else if (def.category == kRoomTheatre) {
        set_section(ctx, TextSection::Help, 150);
    } else {
        set_section(ctx, TextSection::Help, 151);
    }
}

namespace
{

// The room type a ward is (ROOM_WARD, src/tables.cpp).
constexpr uint8_t kRoomWard = 9;

// what_is_required's room part (src/disaster.cpp): help_text 153/154 when
// hospital 0 has no room of the type treating `exp`, else 0.
int required_room_text(const World &world, const ExpertiseInfo &exp)
{
    if (world.room_of_type[exp.category] != 0)
        return 0;
    if (!room_available(world, exp.category))
        return 153;
    return exp.category ? 154 : 0;
}

// what_is_required's staff part: the two surgeons, the second surgeon, or the
// first skill the treatment needs that the hospital lacks.
int required_staff_text(const World &world, uint16_t mask, int &missing_skill)
{
    if (mask & kSkillSurgeon) {
        int surgeons = staff_with_skill(world, 0, 5);
        if (surgeons == 0)
            return 155;
        return surgeons < 2 ? 156 : 0;
    }
    for (int i = 0; i < 9; i++)
        if ((mask & (1 << i)) && staff_with_skill(world, 0, i) == 0) {
            missing_skill = i;
            return i ? 157 : 0;
        }
    return 0;
}

// what_is_required's ward and nurse part.
int required_ward_text(const World &world, uint8_t needs_ward)
{
    if (!needs_ward || world.cure_available[kIllDiagWard] != 0)
        return 0;
    if (world.room_of_type[kRoomWard] == 0)
        return 158;
    if (staff_with_skill(world, 0, 2) == 0)
        return 159;
    return 0;
}

} // namespace

void drugs_help_handled(HelpContext &ctx)
{
    if (!ctx.help || !ctx.world || !ctx.text || !ctx.charts)
        return;
    size_t sel = size_t(ctx.charts->selected_illness);
    if (sel >= kExpertiseData.size())
        return;
    if (ctx.world->cure_available[sel] == 1) {
        set_section(ctx, TextSection::Help, 152);
        return;
    }
    const ExpertiseInfo &exp = kExpertiseData[sel];
    int missing_skill = 0;
    int text = required_room_text(*ctx.world, exp);
    if (!text)
        text = required_staff_text(*ctx.world, exp.staff_mask, missing_skill);
    if (!text)
        text = required_ward_text(*ctx.world, exp.needs_ward);
    if (text == 157)
        ctx.help->msg =
            format_name(ctx.text->at(TextSection::Help, 157), ctx.text->at(TextSection::SkillNames, missing_skill));
    else
        set_section(ctx, TextSection::Help, text ? text : 160);
}

} // namespace th
