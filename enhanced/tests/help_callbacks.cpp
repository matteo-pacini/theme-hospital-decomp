// The hireshelp callbacks (core/ui/helpmsg_callbacks.cpp): each builds the
// same text the original callback writes, from a constructed state.

#include "common.h"

#include "core/data/text.h"
#include "core/ui/charts.h"
#include "core/ui/helpmsg.h"
#include "core/ui/hire_staff.h"
#include "core/ui/info_windows.h"
#include "core/world/arrivals.h"
#include "core/world/objects.h"
#include "core/world/queue.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"

#include <cstdio>
#include <memory>
#include <optional>
#include <string>

namespace
{

using th::Charts;
using th::Game;
using th::HelpBox;
using th::HelpContext;
using th::HelpHotspot;
using th::HireStaff;
using th::InfoWindows;
using th::ObjectPlacing;
using th::TextSection;
using th::Thing;

// The game loaded once for all the callback tests.
struct Fixture {
    th::GameData gd;
    std::optional<th::Level> level;
    th::Balance settings;
    std::unique_ptr<Game> game = std::make_unique<Game>();
    bool ok = false;
};

Fixture &fixture(const char *dir)
{
    static Fixture f;
    if (f.ok)
        return f;
    std::string error;
    if (!f.gd.load(dir, &error))
        return f;
    f.level = f.gd.load_level(1, &error);
    if (!f.level || !th::load_balance(dir, 1, th::kDefaultBalanceId, f.settings, &error))
        return f;
    f.game->staff_market.ref_counts = th::reference_counts(f.gd.text());
    f.game->start(*f.level, 1, f.settings, f.gd.anims(), f.gd.person_complexes(), f.gd.staff_complexes());
    f.ok = true;
    return f;
}

// A hotspot whose low word is text `text` and high word is `slot`.
HelpHotspot hotspot(int text, int slot = 0)
{
    HelpHotspot h;
    h.text_and_slot = int32_t(uint16_t(text)) | (int32_t(slot) << 16);
    return h;
}

HelpContext make_ctx(Fixture &f, HelpBox &box, HelpHotspot &hs)
{
    HelpContext ctx;
    ctx.game = f.game.get();
    ctx.world = &f.game->world;
    ctx.text = &f.gd.text();
    ctx.help = &box;
    ctx.hotspot = &hs;
    return ctx;
}

std::string help_text(Fixture &f, int id)
{
    return std::string(f.gd.text().at(TextSection::Help, id));
}

std::string fmt(std::string_view format, int a, int b)
{
    char buf[512];
    std::snprintf(buf, sizeof buf, std::string(format).c_str(), a, b);
    return buf;
}

std::string fmt_name(std::string_view format, std::string_view s)
{
    char buf[512];
    std::snprintf(buf, sizeof buf, std::string(format).c_str(), std::string(s).c_str());
    return buf;
}

} // namespace

void test_help_callbacks_panel(const char *dir)
{
    Fixture &f = fixture(dir);
    if (!f.ok)
        return;
    HelpBox box;
    auto hs = hotspot(9);
    HelpContext ctx = make_ctx(f, box, hs);

    th::panel_rep_help(ctx);
    CHECK(box.msg == help_text(f, 9) + " (" + std::to_string(f.game->reputation()) + ")");

    f.game->world.emergency.timer = 5;
    th::panel_watch_help(ctx);
    CHECK(box.msg == help_text(f, 0xee));

    f.game->world.emergency.timer = 0;
    f.game->opening_countdown = 7;
    th::panel_watch_help(ctx);
    CHECK(box.msg == help_text(f, 0xed));

    f.game->opening_countdown = 0;
    box.msg = "sentinel";
    th::panel_watch_help(ctx);
    CHECK(box.msg == "sentinel");
}

void test_help_callbacks_patient_windows(const char *dir)
{
    Fixture &f = fixture(dir);
    if (!f.ok)
        return;
    InfoWindows info;
    info.patient = 1;
    Thing &p = f.game->world.things[1];
    p.diagnosis = 1000;
    p.treatment = 5;
    p.illness = 3;
    p.known_illness = 1;
    p.person_flags = 0;
    p.queue_room = 4;

    HelpBox box;
    auto hs = hotspot(92);
    HelpContext ctx = make_ctx(f, box, hs);
    ctx.info = &info;

    th::req3_help_if_diagnosed(ctx);
    CHECK(box.msg == help_text(f, 92));

    th::req3_help_if_curable(ctx);
    CHECK(box.msg == help_text(f, 92));

    th::req3_help_if_queueing(ctx);
    CHECK(box.msg == help_text(f, 92));

    // Not diagnosed, and guessed: neither text appears.
    p.diagnosis = 0;
    p.person_flags = th::kPfGuessed;
    box.msg = "sentinel";
    th::req3_help_if_diagnosed(ctx);
    CHECK(box.msg == "sentinel");
    th::req3_help_if_curable(ctx);
    CHECK(box.msg == "sentinel");
}

void test_help_callbacks_staff_windows(const char *dir)
{
    Fixture &f = fixture(dir);
    if (!f.ok)
        return;
    HireStaff hire;
    hire.shown = 1;
    InfoWindows info;
    info.staff = 1;
    Thing &t = f.game->world.things[1];
    t.skills = th::kSkillDoctor | th::kSkillSurgeon | th::kSkillPsychiatrist | th::kSkillResearcher;

    HelpBox box;
    auto hs = hotspot(30);
    HelpContext ctx = make_ctx(f, box, hs);
    ctx.hire = &hire;
    ctx.info = &info;

    th::req2_help_if_doctor(ctx);
    CHECK(box.msg == help_text(f, 30));
    th::req2_help_if_surgeon(ctx);
    CHECK(box.msg == help_text(f, 30));
    th::req2_help_if_psycho(ctx);
    CHECK(box.msg == help_text(f, 30));
    th::req2_help_if_researcher(ctx);
    CHECK(box.msg == help_text(f, 30));
    th::req2_help_if_selected(ctx);
    CHECK(box.msg == help_text(f, 30));

    th::req4_help_if_doctor(ctx);
    CHECK(box.msg == help_text(f, 30));
    th::req4_help_if_surgeon(ctx);
    CHECK(box.msg == help_text(f, 30));
    th::req4_help_if_shrink(ctx);
    CHECK(box.msg == help_text(f, 30));
    th::req4_help_if_research(ctx);
    CHECK(box.msg == help_text(f, 30));

    // A nurse has none of the doctor qualifications: the tooltips stay empty.
    t.skills = th::kSkillNurse;
    box.msg = "sentinel";
    th::req2_help_if_doctor(ctx);
    CHECK(box.msg == "sentinel");
    th::req4_help_if_surgeon(ctx);
    CHECK(box.msg == "sentinel");
}

void test_help_callbacks_requesters(const char *dir)
{
    Fixture &f = fixture(dir);
    if (!f.ok)
        return;
    HelpBox box;
    auto hs = hotspot(0, 0);
    HelpContext ctx = make_ctx(f, box, hs);
    ObjectPlacing placing;
    placing.offered[0] = 5;
    ctx.placing = &placing;

    th::req10_help_room(ctx);
    CHECK(box.msg == std::string(f.gd.text().at(TextSection::RoomHelp, 5)));

    th::req11_help_object(ctx);
    CHECK(box.msg == std::string(f.gd.text().at(TextSection::ObjectHelp, 5)));

    th::req11_help_plus(ctx);
    CHECK(box.msg == help_text(f, 0x29));
    th::req11_help_minus(ctx);
    CHECK(box.msg == help_text(f, 0x2a));

    // The demand requester is not ported: the plain fallback stands.
    box.msg.clear();
    th::req12_help_if_doctor(ctx);
    CHECK(box.msg == help_text(f, 0));
}

void test_help_callbacks_slist(const char *dir)
{
    Fixture &f = fixture(dir);
    if (!f.ok)
        return;
    Charts charts;
    charts.slist_filter = 1;
    charts.slist_page = 0;
    charts.value = 1;
    Thing &t = f.game->world.things[1];
    t.type = th::kThingStaff;
    t.owner = 0;
    t.state = 0;
    t.subtype = th::kStaffDoctor;
    t.skills = th::kSkillDoctor | th::kSkillSurgeon | th::kSkillPsychiatrist | th::kSkillResearcher;

    HelpBox box;
    auto hs = hotspot(0, 0);
    HelpContext ctx = make_ctx(f, box, hs);
    ctx.charts = &charts;

    th::slist_salary_help_if_present(ctx);
    CHECK(box.msg == help_text(f, 50));
    th::slist_happiness_help_if_present(ctx);
    CHECK(box.msg == help_text(f, 51));
    th::slist_tiredness_help_if_present(ctx);
    CHECK(box.msg == help_text(f, 52));
    th::slist_ability_help_if_present(ctx);
    CHECK(box.msg == help_text(f, 53));

    hs = hotspot(60, 0);
    ctx.hotspot = &hs;
    th::slist_help_if_doctor(ctx);
    CHECK(box.msg == help_text(f, 60));

    hs = hotspot(63, 0);
    ctx.hotspot = &hs;
    th::slist_help_if_surgeon(ctx);
    CHECK(box.msg == help_text(f, 63));
    hs = hotspot(64, 0);
    ctx.hotspot = &hs;
    th::slist_help_if_shrink(ctx);
    CHECK(box.msg == help_text(f, 64));
    hs = hotspot(65, 0);
    ctx.hotspot = &hs;
    th::slist_help_if_researcher(ctx);
    CHECK(box.msg == help_text(f, 65));

    // A filled row past the listed count shows nothing.
    hs = hotspot(0, 9);
    ctx.hotspot = &hs;
    box.msg = "sentinel";
    th::slist_salary_help_if_present(ctx);
    CHECK(box.msg == "sentinel");
}

void test_help_callbacks_manager(const char *dir)
{
    Fixture &f = fixture(dir);
    if (!f.ok)
        return;
    f.game->world.ins_control[0] = 3;
    f.game->world.ins_control[1] = 5;
    Charts charts;
    HelpBox box;
    auto hs = hotspot(218, 0);
    HelpContext ctx = make_ctx(f, box, hs);
    ctx.charts = &charts;

    th::manager_help_pointed_ins(ctx);
    CHECK(box.msg == fmt_name(f.gd.text().at(TextSection::Help, 218), f.gd.text().at(TextSection::InsuranceNames, 3)));

    hs = hotspot(220, 0);
    ctx.hotspot = &hs;
    charts.value = 2;
    th::manager_help_displayed_ins(ctx);
    CHECK(box.msg == fmt_name(f.gd.text().at(TextSection::Help, 220), f.gd.text().at(TextSection::InsuranceNames, 5)));
}

void test_help_callbacks_status(const char *dir)
{
    Fixture &f = fixture(dir);
    if (!f.ok)
        return;
    Charts charts;
    charts.hospital_names[0] = "St Swithin";
    charts.value = 0;
    HelpBox box;
    auto hs = hotspot(222, 0);
    HelpContext ctx = make_ctx(f, box, hs);
    ctx.charts = &charts;

    th::overscr_help_add_name(ctx);
    CHECK(box.msg == fmt_name(f.gd.text().at(TextSection::Help, 222), "St Swithin"));

    // One win criterion, reputation 500, target 1000: half progress in group
    // 1, the best group, so the bar's tooltip is the first criterion's.
    std::string error;
    th::Balance bal;
    for (auto &w : bal.win_criteria)
        w = {};
    for (auto &l : bal.lose_criteria)
        l = {};
    bal.win_criteria[0] = {1, 1, 1000, 0, 1};
    Game game;
    game.staff_market.ref_counts = th::reference_counts(f.gd.text());
    game.start(*f.level, 1, bal, f.gd.anims(), f.gd.person_complexes(), f.gd.staff_complexes());
    HelpBox box2;
    auto hs2 = hotspot(268, 0);
    HelpContext ctx2;
    ctx2.game = &game;
    ctx2.world = &game.world;
    ctx2.text = &f.gd.text();
    ctx2.help = &box2;
    ctx2.hotspot = &hs2;
    ctx2.charts = &charts;
    th::overscr_help_win_criteria(ctx2);
    CHECK(box2.msg == fmt(f.gd.text().at(TextSection::WinCriteriaHelp, 0), 1000, game.reputation()));
}

void test_help_callbacks_drugs(const char *dir)
{
    Fixture &f = fixture(dir);
    if (!f.ok)
        return;
    Charts charts;
    charts.selected_illness = 5; // a drug illness (expertise kind 39)
    f.game->world.cure_available[5] = 1;
    f.game->hospital.illnesses[5].drug_effectiveness = 77;

    HelpBox box;
    auto hs = hotspot(0, 0);
    HelpContext ctx = make_ctx(f, box, hs);
    ctx.charts = &charts;

    th::drugs_help_cure_type(ctx);
    CHECK(box.msg == fmt(f.gd.text().at(TextSection::Help, 148), 77, 0));

    th::drugs_help_handled(ctx);
    CHECK(box.msg == help_text(f, 152));

    // A surgery with the theatre built but no surgeon: the two-surgeon text.
    // (The staff-list test left a surgeon in the shared fixture.)
    f.game->world.things[1] = Thing{};
    charts.selected_illness = 14;
    f.game->world.cure_available[14] = 0;
    f.game->world.room_of_type[10] = 1;
    box.msg.clear();
    th::drugs_help_handled(ctx);
    CHECK(box.msg == help_text(f, 155));

    th::drugs_help_concentrate(ctx);
    CHECK(box.msg == help_text(f, 0));
}
