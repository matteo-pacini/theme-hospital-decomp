// The jukebox and the charts windows (core/ui/jukebox.h, core/ui/charts.h).

#include "common.h"

#include "core/audio/music.h"
#include "core/audio/sounds.h"
#include "core/data/balance.h"
#include "core/data/gamedata.h"
#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/render/panel.h"
#include "core/render/surface.h"
#include "core/ui/charts.h"
#include "core/ui/hire_staff.h"
#include "core/ui/info_windows.h"
#include "core/ui/jukebox.h"
#include "core/ui/layer.h"
#include "core/ui/popups.h"
#include "core/ui/queue_window.h"
#include "core/ui/town_map.h"
#include "core/ui/windows.h"
#include "core/world/arrivals.h"
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

// The jukebox open on `music`, driven frame by frame with the mouse.
struct JukeboxDriver {
    th::Jukebox &jukebox;
    th::Interface &ui;
    const th::RequesterArt &art;
    th::MusicPlayer &music;
    th::UiMouse m;

    th::JukeboxResult frame(const th::InterfaceEvents &e)
    {
        return th::jukebox_frame(jukebox, ui, {e, m}, art, music, false);
    }

    // Presses and releases button `button` (counted from 1). Returns the
    // release frame's result.
    th::JukeboxResult click_button(int button)
    {
        const th::Window &win = ui.window(int(th::Requester::Jukebox));
        const th::WindowButton &b = win.buttons[size_t(button - 1)];
        m = {};
        m.x = win.x + b.x + b.width / 2;
        m.y = win.y + b.y + b.height / 2;
        m.left_pressed = m.left_held = true;
        frame(ui.frame(m));
        m.left_pressed = m.left_held = false;
        m.left_released = true;
        th::JukeboxResult r = frame(ui.frame(m));
        m.left_released = false;
        return r;
    }

    // Presses the button at (x, y) of the track list in one frame and lets
    // go in the next. Returns the second frame's result.
    th::JukeboxResult click_list(int x, int y)
    {
        m = {};
        m.x = x;
        m.y = y;
        m.left_pressed = m.left_held = true;
        frame({});
        m = {};
        m.x = x;
        m.y = y;
        return frame({});
    }
};

// The jukebox drawn: the list's last row, below the window, too.
void check_jukebox_drawn(const th::Interface &ui, const th::RequesterArt &art, const th::MusicPlayer &music)
{
    const th::Window &win = ui.window(int(th::Requester::Jukebox));
    const th::Sprite row = art.sheet(int(th::Requester::Jukebox)).at(0x193);
    const th::Sprite left_end = art.sheet(int(th::Requester::Jukebox)).at(0x192);
    CHECK(row.width > 0 && row.height > 0);
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    th::draw_interface(layer, ui, art);
    th::draw_jukebox(layer, ui, art, music);
    th::Palette opaque;
    opaque.fill(0xff000000);
    auto argb = layer.to_argb(opaque);
    int last_y = win.y + 79 + 7 * row.height + row.height / 2;
    CHECK(argb[size_t(last_y) * th::kUiWidth + size_t(win.x + 2 + left_end.width + 1)] != 0);
}

// The buttons: next, previous, pause, loop and stop.
void check_jukebox_buttons(JukeboxDriver &j)
{
    th::MusicPlayer &music = j.music;
    music.start_level(13);
    CHECK(music.playing() && music.track() == 5);
    // Next ends the song; the frame's play_next moves on.
    th::JukeboxResult r = j.click_button(11);
    CHECK(r.sounds.size() == 1 && r.sounds[0] == th::kSfxSelectX && music.song_over(false));
    music.play_next(false);
    CHECK(music.playing() && music.track() == 6);
    // Previous goes back at once.
    j.click_button(10);
    CHECK(music.playing() && music.track() == 5 && !music.song_over(false));
    // Pause and play again.
    j.click_button(9);
    CHECK(!music.playing());
    j.click_button(9);
    CHECK(music.playing() && music.track() == 5 && !music.song_over(false));
    // Looping, next is ignored.
    j.click_button(13);
    CHECK(music.loop());
    j.click_button(11);
    CHECK(!music.song_over(false) && music.track() == 5);
    // Stop ends the song and the loop.
    j.click_button(12);
    CHECK(!music.playing() && !music.loop());
    j.click_button(9);
    CHECK(music.playing() && music.track() == 5);
}

// The list: a click on track 2's title plays it, one on track 3's light
// takes it out of the playlist, so next skips it.
void check_jukebox_list(JukeboxDriver &j)
{
    const th::Window &win = j.ui.window(int(th::Requester::Jukebox));
    const th::Sprite row = j.art.sheet(int(th::Requester::Jukebox)).at(0x193);
    const th::Sprite left_end = j.art.sheet(int(th::Requester::Jukebox)).at(0x192);
    th::MusicPlayer &music = j.music;
    int tiles_end = 2;
    while (tiles_end < row.width * 7)
        tiles_end += row.width;
    auto row_y = [&](int track) { return win.y + 79 + track * row.height + row.height / 2; };
    j.click_list(win.x + 2 + left_end.width + 1, row_y(2));
    CHECK(music.playing() && music.track() == 2);
    th::JukeboxResult r = j.click_list(win.x + tiles_end + left_end.width + 2, row_y(3));
    CHECK(!music.track_enabled(3) && music.track() == 2 && r.sounds.size() == 1);
    j.click_button(11);
    music.play_next(false);
    CHECK(music.track() == 4);
}

} // namespace

// The jukebox: the tracks of MIDI.TXT, its buttons and its track list.
void test_jukebox(const char *dir)
{
    th::MusicPlayer music;
    th::RequesterArt art;
    std::string error;
    CHECK(music.load(dir, &error) && art.load(dir, &error));
    if (!error.empty())
        std::fprintf(stderr, "%s\n", error.c_str());
    th::Interface ui;
    th::Jukebox jukebox;
    th::jukebox_event(jukebox, ui, {th::EV_OPEN_JUKEBOX, 0, 0, 0});
    CHECK(ui.requester() == th::Requester::Jukebox);
    CHECK(music.count() == 8 && music.title(0) == "Atlantis");
    for (int i = 0; i < music.count(); i++)
        CHECK(music.track_enabled(i));
    check_jukebox_drawn(ui, art, music);

    JukeboxDriver j{jukebox, ui, art, music, {}};
    check_jukebox_buttons(j);
    check_jukebox_list(j);
    th::JukeboxResult r = j.click_button(16);
    CHECK(r.close && r.sounds.size() == 1 && r.sounds[0] == th::kSfxNo4);
}

namespace
{

// A game frame of the charts with the command it gives carried out, as the
// app does: the interface's part, then the game's.
th::ChartsResult chart_step(th::Charts &c, th::Game &game, th::HospitalSettings &hs, th::UiMouse m = {},
                            th::ChartKeys k = {})
{
    th::ChartsResult r = th::chart_frame(c, m, k, 0.04, game, hs);
    if (r.event.type) {
        th::chart_command(c, r.event);
        th::apply_chart_event(game, hs, r.event);
    }
    return r;
}

// Clicks (x, y) on the chart: the button down in one frame, up in the next.
// Returns the second frame's result.
th::ChartsResult chart_click(th::Charts &c, th::Game &game, th::HospitalSettings &hs, int x, int y)
{
    th::UiMouse m;
    m.x = x;
    m.y = y;
    m.left_pressed = m.left_held = true;
    chart_step(c, game, hs, m);
    m.left_pressed = m.left_held = false;
    m.left_released = true;
    return chart_step(c, game, hs, m);
}

// Pixels of `r` the chart drew over its picture.
int chart_drawn(const th::Charts &c, th::Rect r)
{
    const th::Surface &pic = c.pictures[size_t(c.chart)].pixels;
    int n = 0;
    for (int y = r.y; y < r.y + r.h; y++)
        for (int x = r.x; x < r.x + r.w; x++)
            n += c.surface.data()[y * 640 + x] != pic.data()[y * 640 + x];
    return n;
}

// The charts on level 1's game, with the hospital's settings they change.
struct ChartsFixture {
    th::Game &game;
    th::Charts &c;
    th::HospitalSettings &hs;

    th::ChartsResult step(th::UiMouse m = {}, th::ChartKeys k = {}) { return chart_step(c, game, hs, m, k); }
    th::ChartsResult click(int x, int y) { return chart_click(c, game, hs, x, y); }
};

// Each chart: its picture in its own palette, with its figures drawn over.
void check_chart_pictures(ChartsFixture &t)
{
    th::Charts &c = t.c;
    for (int n = 0; n < th::kChartCount; n++) {
        th::chart_command(c, {th::EV_OPEN_SCREEN, uint16_t(n)});
        CHECK(c.open && c.chart == n);
        t.step();
        CHECK(c.palette == c.pictures[size_t(n)].palette);
        CHECK(c.pictures[size_t(n)].sprites.size() > 1);
        int lit = 0;
        for (int i = 0; i < 640 * 480; i++)
            lit += c.surface.data()[i] != 0;
        CHECK(lit > 640 * 480 / 2);
        CHECK(chart_drawn(c, {0, 0, 640, 480}) > 100);
    }
}

// The bank statement, no lines then a purchase listed; a click on it opens
// the bank manager, valuing the hospital, for borrowing and repaying $5000;
// the close button goes back to the game on the next frame.
void check_statement_and_bank(ChartsFixture &t)
{
    th::Charts &c = t.c;
    th::Game &game = t.game;
    const th::Rect lines = {40, 70, 560, 14};
    th::chart_command(c, {th::EV_OPEN_SCREEN, th::kChartStatement});
    t.step();
    CHECK(game.finance.entry_count == 0 && chart_drawn(c, lines) == 0);
    th::change_balance(game.finance, game.balance, game.date.day, game.date.month, {th::kTxBuyObject, -500});
    t.step();
    CHECK(chart_drawn(c, lines) > 0);

    t.click(300, 200);
    CHECK(c.open && c.chart == th::kChartBank && game.finance.max_loan >= 5000);
    t.step();
    int32_t before = game.balance;
    th::ChartsResult r = t.click(200, 275);
    CHECK(r.event.type == th::EV_TAKE_LOAN);
    CHECK(game.balance == before + 5000 && game.finance.loan == 5000);
    const th::BankEntry &loan = game.finance.entries[game.finance.entry_count - 1u];
    CHECK(loan.type == th::kTxBankLoan && loan.amount == 5000);
    r = t.click(60, 275);
    CHECK(r.event.type == th::EV_REPAY_LOAN && game.balance == before && game.finance.loan == 0);
    r = t.click(60, 275);
    CHECK(r.event.type == th::EV_NONE && std::count(r.sounds.begin(), r.sounds.end(), 0x08) == 1);
    t.click(615, 455);
    CHECK(c.open);
    r = t.step();
    CHECK(r.event.type == th::EV_RETURN_FROM_SCREEN && !c.open);
}

// Policy: dragging the send-home slider, then Escape.
void check_policy(ChartsFixture &t)
{
    th::chart_command(t.c, {th::EV_OPEN_SCREEN, th::kChartPolicy});
    t.step();
    th::UiMouse m;
    m.x = 250;
    m.y = 140;
    m.left_pressed = m.left_held = true;
    t.step(m);
    CHECK(t.hs.send_home_threshold != 10 && t.hs.send_home_threshold <= t.hs.guess_cure_threshold);
    th::ChartKeys escape;
    escape.escape = true;
    t.step({}, escape);
    t.step();
    t.step();
    CHECK(!t.c.open);
}

// Casebook prices: general practice's share of the most patients will pay,
// in hospital 0's market, kept within 50-200%.
void check_casebook_prices(ChartsFixture &t, const th::Balance &settings)
{
    th::HospitalMarket &market = t.game.arrivals.hospitals[0];
    int32_t gp_max = th::max_price_for_treatment(market, settings, 1);
    th::apply_chart_event(t.game, t.hs, {th::EV_PRICE_UP, 30, 0, 1});
    CHECK(market.price_scale[1] == 13000 && market.price[1] == 13000 * gp_max / 10000);
    th::apply_chart_event(t.game, t.hs, {th::EV_PRICE_DOWN, 200, 0, 1});
    CHECK(market.price_scale[1] == 5000 && market.price[1] == 5000 * gp_max / 10000);
    th::apply_chart_event(t.game, t.hs, {th::EV_PRICE_DOWN, 1, 0, 1});
    CHECK(market.price_scale[1] == 5000);
}

// The staff list shows staff member `id`; a click selects them and the
// raise button raises their wage by a tenth. The sack button asks first:
// the chart closes for the yes/no requester, whose yes sacks them
// (EV_SACK_STAFF) and no cancels (EV_CANCEL_SACK).
void check_staff_list(ChartsFixture &t, uint16_t id)
{
    th::Charts &c = t.c;
    th::chart_command(c, {th::EV_OPEN_SCREEN, th::kChartStaff});
    t.step();
    CHECK(chart_drawn(c, {49, 82, 581, 27}) > 0);
    t.click(150, 95);
    CHECK(c.value == id);
    int32_t salary = t.game.world.things[id].salary;
    th::ChartsResult r = t.click(370, 435);
    CHECK(r.event.type == th::EV_INCREASE_STAFF_SALARIES && t.game.world.things[id].salary == salary + salary / 10);
    r = t.click(460, 400);
    CHECK(r.event.type == th::EV_OPEN_YES_NO && !c.open && c.value == 0 && r.question == 5);
    CHECK(r.yes.type == th::EV_SACK_STAFF && r.yes.value == id && r.yes.arg == 0);
    CHECK(r.no.type == th::EV_CANCEL_SACK && r.no.value == id);
}

// Switching charts with a key; research needs a research room.
void check_chart_switching(ChartsFixture &t)
{
    bool refused = false;
    th::Event e = th::open_chart_command(th::kChartGraphs, t.game, &refused);
    CHECK(e.type == th::EV_OPEN_SCREEN && !refused);
    th::chart_command(t.c, e);
    CHECK(t.c.open && t.c.chart == th::kChartGraphs && t.c.state == 0);
    e = th::open_chart_command(th::kChartResearch, t.game, &refused);
    CHECK(e.type == th::EV_NONE && refused);
    CHECK(th::chart_for_panel_action(th::PanelAction::BankStatement) == th::kChartStatement);
    CHECK(th::chart_for_panel_action(th::PanelAction::HireStaff) == -1);
}

} // namespace

void test_charts(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::Game &game = *f.game;
    th::Charts c;
    std::string error;
    CHECK(th::load_charts(c, dir, f.gd.text(), f.gd.palette(), &error));
    c.hospital_names[0] = "Dr Theme Hospital";
    th::HospitalSettings hs;
    th::start_hospital_settings(hs, f.settings);
    CHECK(hs.illnesses[1].discovered == 1);
    CHECK(game.arrivals.hospitals[0].price[1] == f.settings.expertise[1].start_price);
    CHECK(hs.rest_threshold == f.settings.gbv.tired);
    ChartsFixture t{game, c, hs};
    check_chart_pictures(t);
    check_statement_and_bank(t);
    check_policy(t);
    check_casebook_prices(t, f.settings);

    uint16_t id = th::alter_applicant(game.world, true, 0, th::kStaffDoctor);
    int cell = free_staff_cell(game.world, *f.level);
    CHECK(id != 0 && cell >= 0);
    if (!id || cell < 0)
        return;
    CHECK(th::place_staff_on_map(game.world, {f.gd.anims(), f.settings, game.turn_count}, cell, id, false));
    check_staff_list(t, id);
    check_chart_switching(t);
}
