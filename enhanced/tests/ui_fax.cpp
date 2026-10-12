// The fax window and the queue window's bar names (core/ui/fax.h,
// core/ui/queue_window.h).

#include "common.h"

#include "core/data/gamedata.h"
#include "core/data/palette.h"
#include "core/data/text.h"
#include "core/render/panel.h"
#include "core/render/surface.h"
#include "core/ui/charts.h"
#include "core/ui/fax.h"
#include "core/ui/hire_staff.h"
#include "core/ui/info_windows.h"
#include "core/ui/layer.h"
#include "core/ui/popups.h"
#include "core/ui/queue_window.h"
#include "core/ui/town_map.h"
#include "core/ui/windows.h"
#include "core/world/epidemic.h"
#include "core/world/events.h"
#include "core/world/finance.h"
#include "core/world/mayor.h"
#include "core/world/rooms.h"
#include "core/world/savegame.h"
#include "core/world/snapshot.h"
#include "core/world/staff.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace
{

// A left click at `at` on the fax: pressed, then released a game frame on.
th::FaxResult fax_click(th::Fax &fax, th::Game &g, uint32_t &safeseed, th::Point at)
{
    th::UiMouse m;
    m.x = at.x;
    m.y = at.y;
    m.left_pressed = m.left_held = true;
    th::fax_frame(fax, m, {}, 0.04, g, safeseed);
    m.left_pressed = m.left_held = false;
    m.left_released = true;
    return th::fax_frame(fax, m, {}, 0.04, g, safeseed);
}

th::FaxResult fax_answer(th::Fax &fax, th::Game &g, uint32_t &safeseed, size_t button)
{
    th::Rect b = th::fax_answer_buttons(fax)[button];
    return fax_click(fax, g, safeseed, {b.x + b.w / 2, b.y + b.h / 2});
}

bool has_sound(const th::FaxResult &r, uint8_t sfx)
{
    return std::find(r.sounds.begin(), r.sounds.end(), sfx) != r.sounds.end();
}

// The fax opened for `choice`: its first frame, which draws it.
th::FaxResult fax_open(th::Fax &fax, th::Game &g, uint32_t &safeseed, th::Game::ChoiceScreen choice)
{
    th::display_choice_screen(fax, choice);
    return th::fax_frame(fax, {}, {}, 0.04, g, safeseed);
}

// The emergency offered: accept or refuse; accepting refused while an
// emergency runs.
void check_fax_emergency(th::Fax &fax, th::Game &g, uint32_t &safeseed, const th::LanguageText &text)
{
    g.disasters.offers[0] = {5, 500, 1, 2, 6, 75};
    g.disasters.popup_call = 5;
    th::FaxResult r = fax_open(fax, g, safeseed, {th::kFaxEmergency, 0, 0});
    CHECK(has_sound(r, 0xab) && fax.open && fax.buttons.size() == 2 && (fax.available & 3) == 3);
    uint16_t available = 0xffff;
    uint32_t seed = safeseed;
    std::string message = th::fax_message(fax, g, available, seed);
    CHECK(message.find(std::string(text.at(th::TextSection::Illness, 2))) != std::string::npos);
    CHECK(message.find("3000") != std::string::npos);
    r = fax_answer(fax, g, safeseed, 0);
    CHECK(r.event.type == th::EV_CHOICE_RESULT && r.event.value == th::kChoiceEmergencyAccept && has_sound(r, 0x41));
    th::fax_command(fax, r.event);
    CHECK(!fax.open);
    g.world.emergency.state = 10;
    fax_open(fax, g, safeseed, {th::kFaxEmergency, 0, 0});
    CHECK((fax.available & 1) == 0 && fax_answer(fax, g, safeseed, 0).event.type == th::EV_NONE);
    r = fax_answer(fax, g, safeseed, 1);
    CHECK(r.event.type == th::EV_CHOICE_RESULT && r.event.value == th::kChoiceEmergencyRefuse);
    g.world.emergency.state = 0;
}

// A patient with no treatment (send home, wait, or the research department,
// refused until autopsies can be done) and with no diagnosis (guessing
// refused unless the hospital knows and can cure the illness).
void check_fax_patient(th::Fax &fax, th::Game &g, uint32_t &safeseed, uint16_t patient)
{
    th::Thing &p = g.world.things[patient];
    p.type = th::kThingPatient;
    p.illness = 2;
    fax_open(fax, g, safeseed, {th::kFaxNoTreatment, patient, 0});
    CHECK(fax.buttons.size() == 3 && (fax.available & 7) == 3);
    CHECK(fax_answer(fax, g, safeseed, 2).event.type == th::EV_NONE);
    th::FaxResult r = fax_answer(fax, g, safeseed, 1);
    CHECK(r.event.type == th::EV_CHOICE_RESULT && r.event.value == th::kChoiceTreatWait && r.event.arg == patient);
    r = fax_answer(fax, g, safeseed, 0);
    CHECK(r.event.value == th::kChoiceTreatSendHome && r.event.arg == patient);
    fax_open(fax, g, safeseed, {th::kFaxNoDiagnosis, patient, 0});
    CHECK(fax.buttons.size() == 3 && (fax.available & 7) == 5);
    g.world.cure_available[2] = 1;
    g.hospital.illnesses[2].discovered = 1;
    fax_open(fax, g, safeseed, {th::kFaxNoDiagnosis, patient, 0});
    CHECK((fax.available & 7) == 7);
    r = fax_answer(fax, g, safeseed, 1);
    CHECK(r.event.value == th::kChoiceDiagGuess && r.event.arg == patient);
    CHECK(fax_answer(fax, g, safeseed, 2).event.value == th::kChoiceDiagWait);
}

// A new illness's fax closes by its one button, sending nothing; the
// emergency's result closes on Escape, once the mouse has not moved for 400
// turns, or by the close button.
void check_fax_closing(th::Fax &fax, th::Game &g, uint32_t &safeseed)
{
    fax_open(fax, g, safeseed, {th::kFaxNewIllness, 2, 0});
    CHECK(fax.buttons.size() == 1);
    th::FaxResult r = fax_answer(fax, g, safeseed, 0);
    CHECK(r.event.type == th::EV_NONE && !fax.open);
    fax_open(fax, g, safeseed, {th::kFaxEmergencyOver, 0, 400});
    CHECK(fax.buttons.size() == 1);
    r = th::fax_frame(fax, {}, {true}, 0.04, g, safeseed);
    CHECK(!fax.open && has_sound(r, 0x42));
    fax_open(fax, g, safeseed, {th::kFaxEmergencyOver, 0, 400});
    g.turn_count += 400;
    th::fax_frame(fax, {}, {}, 0.04, g, safeseed);
    CHECK(fax.open);
    g.turn_count += 1;
    th::fax_frame(fax, {}, {}, 0.04, g, safeseed);
    CHECK(!fax.open);
    fax_open(fax, g, safeseed, {th::kFaxEmergencyOver, 0, 400});
    fax_click(fax, g, safeseed, {600, 450});
    CHECK(!fax.open);
}

// The keypad: digits beep their own sounds; a cheat's number dialled and
// sent plays SFX_FAX_YES, another SFX_FAX_NO. The close button does nothing
// on a fax that must be answered; an answer carried out closes it.
void check_fax_keypad(th::Fax &fax, th::Game &g, uint32_t &safeseed, uint16_t patient)
{
    fax_open(fax, g, safeseed, {th::kFaxNoTreatment, patient, 0});
    for (int i = 0; i < 3; i++)
        CHECK(has_sound(fax_click(fax, g, safeseed, {330, 375}), 0xc7));
    CHECK(has_sound(fax_click(fax, g, safeseed, {170, 352}), 0xac));
    CHECK(has_sound(fax_click(fax, g, safeseed, {222, 352}), 0xbf));
    CHECK(has_sound(fax_click(fax, g, safeseed, {480, 355}), 0xad));
    fax_click(fax, g, safeseed, {600, 450});
    CHECK(fax.open);
    th::fax_command(fax, {th::EV_CHOICE_RESULT, th::kChoiceTreatWait, 0, patient});
    CHECK(!fax.open);
}

// A VIP's offer: accepting, refusing, and the acceptance that overwrites a
// refusal after more than five refusals in a row while the turn's random
// percentage is below 10 (process_mayor_start_buttons); the report closes by
// its one button.
void check_fax_vip(th::Fax &fax, th::Game &g, uint32_t &safeseed)
{
    g.world.mayor.postponed = 0;
    g.world.rand_pct = 10;
    fax_open(fax, g, safeseed, {th::kFaxVip, 0, 0});
    CHECK(fax.buttons.size() == 2);
    th::FaxResult r = fax_answer(fax, g, safeseed, 0);
    CHECK(r.event.value == th::kChoiceVipAccept && g.world.mayor.postponed == 0);
    fax_open(fax, g, safeseed, {th::kFaxVip, 0, 0});
    r = fax_answer(fax, g, safeseed, 1);
    CHECK(r.event.value == th::kChoiceVipRefuse && g.world.mayor.postponed == 1);
    // The sixth refusal with the turn's draw below 10 is accepted instead.
    g.world.mayor.postponed = 5;
    g.world.rand_pct = 9;
    fax_open(fax, g, safeseed, {th::kFaxVip, 0, 0});
    r = fax_answer(fax, g, safeseed, 1);
    CHECK(r.event.value == th::kChoiceVipAccept && g.world.mayor.postponed == 0);
    fax_open(fax, g, safeseed, {th::kFaxVipResult, 0, 400});
    CHECK(fax.buttons.size() == 1);
    r = fax_answer(fax, g, safeseed, 0);
    CHECK(r.event.type == th::EV_NONE && !fax.open);
}

// The epidemic's offer: declaring it answers with the fine; covering it up
// answers and raises the advisor's covered-up event
// (process_epidemic_start_buttons), which the command log carries.
void check_fax_epidemic(th::Fax &fax, th::Game &g, uint32_t &safeseed)
{
    g.advisor.event_flags = 0;
    fax_open(fax, g, safeseed, {th::kFaxEpidemic, 0, 0});
    CHECK(fax.buttons.size() == 2);
    th::FaxResult r = fax_answer(fax, g, safeseed, 0);
    CHECK(r.event.value == th::kChoiceEpidemicDeclare && g.advisor.event_flags == 0);
    fax_open(fax, g, safeseed, {th::kFaxEpidemic, 0, 0});
    r = fax_answer(fax, g, safeseed, 1);
    CHECK(r.event.value == th::kChoiceEpidemicCoverUp && g.advisor.event_flags == th::kAdvisorEventCoverUp);
    CHECK(!g.command_log.empty() && g.command_log.back().ends_with(" @advisor_event 256"));
}

} // namespace

// The palette index nearest to the 6-bit colour (r, g, b) in `palette`,
// as LbPaletteFindColour finds it (the first of equals, which suffices here).
uint8_t find_colour(const th::Palette &palette, int r, int g, int b)
{
    auto at = [&](int i, int shift) { return int((palette[size_t(i)] >> shift) & 0xff) >> 2; };
    auto dist = [&](int i) {
        int dr = r - at(i, 16), dg = g - at(i, 8), db = b - at(i, 0);
        return dr * dr + dg * dg + db * db;
    };
    int best = 0;
    for (int i = 1; i < 256; i++)
        if (dist(i) < dist(best))
            best = i;
    return uint8_t(best);
}

// The colours the game finds in the hospital's palette and in the fax
// picture's (setup_colors: red 0x3f,0,0 and dark blue 0,0,0x10) are the ones
// colour_red and colour_dark_blue name.
void check_fax_colours(const th::Fax &fax, const th::Palette &hospital)
{
    th::World world;
    CHECK(find_colour(hospital, 0x3f, 0, 0) == th::colour_red(world));
    CHECK(find_colour(hospital, 0, 0, 0x10) == th::colour_dark_blue(world));
    CHECK(find_colour(hospital, 0, 0, 0) == th::kColourBlack);
    world.fax_palette = true;
    CHECK(find_colour(fax.palette, 0x3f, 0, 0) == th::colour_red(world));
    CHECK(find_colour(fax.palette, 0, 0, 0x10) == th::colour_dark_blue(world));
    CHECK(find_colour(fax.palette, 0, 0, 0) == th::kColourBlack);
}

// The fax for each kind of call: its message and buttons, each button's
// answer as the game sends it, the buttons the game refuses, closing it,
// and the keypad.
void test_fax(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::Game &g = *f.game;
    th::Fax fax;
    std::string error;
    CHECK(th::load_fax(fax, dir, f.gd.text(), &error));
    uint32_t safeseed = th::kLevelStartSeed;
    uint16_t patient = 0;
    for (uint16_t i = 1; i < g.world.things.size() && !patient; i++)
        if (g.world.things[i].type == th::kThingFree)
            patient = i;
    check_fax_colours(fax, f.gd.palette());
    check_fax_emergency(fax, g, safeseed, f.gd.text());
    check_fax_vip(fax, g, safeseed);
    check_fax_epidemic(fax, g, safeseed);
    check_fax_patient(fax, g, safeseed, patient);
    check_fax_closing(fax, g, safeseed);
    check_fax_keypad(fax, g, safeseed, patient);
}

// The queue window's right-click bar names the three computer players'
// hospitals in its refer entries.
void test_queue_bar_names(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::RequesterArt art;
    std::string error;
    CHECK(art.load(dir, &error));
    th::World &world = f.game->world;
    world.rooms[1].queue_head = 5;
    world.rooms[1].queue_length = 1;
    world.things[5].queue_next = 0;
    th::Interface ui;
    ui.open_requester(th::Requester::Queue);
    th::QueueWindow q;
    q.room = 1;
    q.pos = 1;
    q.drag = 2;

    const auto draw = [&] {
        th::UiLayer layer;
        layer.resize(th::kUiWidth, th::kUiHeight);
        layer.clear();
        th::draw_queue_window(layer, q, ui, *f.game, art, f.gd);
        return layer.to_argb(f.gd.palette());
    };
    for (size_t i = 1; i < 4; i++)
        std::snprintf(world.rivals[i].name.data(), world.rivals[i].name.size(), "RIVAL %zu", i);
    const std::vector<uint32_t> named = draw();
    for (size_t i = 1; i < 4; i++)
        world.rivals[i].name = {};
    CHECK(named != draw());
}
