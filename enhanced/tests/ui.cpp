#include "common.h"

#include "core/audio/music.h"
#include "core/audio/sounds.h"
#include "core/data/balance.h"
#include "core/data/gamedata.h"
#include "core/data/level.h"
#include "core/data/palette.h"
#include "core/data/smacker.h"
#include "core/data/sprites.h"
#include "core/data/text.h"
#include "core/render/panel.h"
#include "core/render/surface.h"
#include "core/ui/charts.h"
#include "core/ui/hire_staff.h"
#include "core/ui/jukebox.h"
#include "core/ui/layer.h"
#include "core/ui/menu_tables.h"
#include "core/ui/menubar.h"
#include "core/ui/startup.h"
#include "core/ui/town_map.h"
#include "core/ui/windows.h"
#include "core/world/arrivals.h"
#include "core/world/events.h"
#include "core/world/finance.h"
#include "core/world/staff.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace
{

th::MenuEvent menu_step(th::MenuBar &bar, int x, int y, bool held, bool released, const th::MenuContext &ctx = {})
{
    th::UiMouse m;
    m.x = x;
    m.y = y;
    m.left_held = held;
    m.left_released = released;
    return th::menu_frame(bar, m, ctx);
}

size_t drawn_pixels(const th::MenuBar &bar, const th::Palette &palette)
{
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    th::draw_menu_bar(layer, bar);
    auto argb = layer.to_argb(palette);
    return size_t(std::count_if(argb.begin(), argb.end(), [](uint32_t p) { return p != 0; }));
}

} // namespace

// With the game data: the menu bar built from English pulldown_text
// (setup_menus), opened, dragged through and chosen from as process_menus
// does, and drawn with the PULLDV art. Every language builds.
void test_menu_bar(const char *dir)
{
    std::string error;
    for (int language = 0; language < 6; language++) {
        th::LanguageText lang;
        th::MenuBar bar;
        CHECK(th::load_language(dir, language, lang, &error));
        CHECK(bar.load(dir, lang, &error));
        CHECK(bar.count == 6 && bar.subcount == 12);
    }
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    th::MenuBar bar;
    CHECK(bar.load(dir, gd.text(), &error));
    if (bar.count != 6)
        return;
    const char *titles[] = {"  FILE  ", "  OPTIONS  ", "  DISPLAY  ", "  CHARTS  ", "  DEBUG  "};
    for (int i = 1; i < 6; i++) {
        CHECK(bar.text[size_t(bar.pulldowns[size_t(i)].text)] == titles[i - 1]);
        CHECK(bar.pulldowns[size_t(i)].active == (i != 5));
    }
    CHECK(bar.line_height == 14 && bar.char_width == 9);
    CHECK(bar.pulldowns[1].count == 4 && bar.pulldowns[2].count == 9 && bar.pulldowns[4].count == 9);
    // Load, Save under File; the volumes and speed under Options.
    const int parents[] = {1, 1, 2, 2, 2, 2};
    for (int s = 6; s < 12; s++)
        CHECK(bar.pulldowns[size_t(s)].parent == parents[s - 6]);
    CHECK(bar.pulldowns[2].submenu[7] == 11 && bar.pulldowns[11].count == 5 && bar.pulldowns[11].y == 14 + 7 * 14);

    // Hidden until the mouse reaches the top 22 lines.
    menu_step(bar, 300, 200, false, false);
    CHECK(!bar.active && drawn_pixels(bar, gd.palette()) == 0);
    menu_step(bar, 300, 21, false, false);
    CHECK(bar.active && bar.open == 0);
    size_t bar_only = drawn_pixels(bar, gd.palette());
    CHECK(bar_only >= size_t(th::kUiWidth) * 16);

    // Hold on File, drag down to Quit and release.
    auto ev = menu_step(bar, 20, 5, true, false);
    CHECK(bar.open == 1 && ev.sound == th::MenuSound::Highlight);
    menu_step(bar, 20, 14 + 3 * 14 + 5, true, false);
    CHECK(bar.open == 1 && bar.active);
    CHECK(drawn_pixels(bar, gd.palette()) > bar_only);
    ev = menu_step(bar, 20, 14 + 3 * 14 + 5, false, true);
    CHECK(ev.action == th::MenuAction::Quit && ev.sound == th::MenuSound::Select);
    CHECK(bar.open == 0 && bar.active); // the release frame still belongs to the bar
    menu_step(bar, 20, 200, false, false);
    CHECK(!bar.active);

    // Options > Game speed > each speed.
    const int options_x = 8 * 9 + 5;
    const int submenu_x = 8 * 9 + bar.pulldowns[2].width * 9 + 10 + 5;
    for (int item = 0; item < 5; item++) {
        menu_step(bar, options_x, 5, true, false);
        CHECK(bar.open == 2);
        menu_step(bar, options_x, 14 + 7 * 14 + 5, true, false);
        CHECK(bar.open == 11);
        int y = 14 + 7 * 14 + item * 14 + 5;
        menu_step(bar, submenu_x, y, true, false);
        CHECK(bar.open == 11);
        ev = menu_step(bar, submenu_x, y, false, true);
        CHECK(ev.action == th::MenuAction::Speed && ev.value == 10 * (item + 1));
    }
    // Refused while paused; Options > Sound is not.
    th::MenuContext paused;
    paused.paused = true;
    menu_step(bar, options_x, 5, true, false, paused);
    menu_step(bar, options_x, 14 + 7 * 14 + 5, true, false, paused);
    menu_step(bar, submenu_x, 14 + 7 * 14 + 5, true, false, paused);
    ev = menu_step(bar, submenu_x, 14 + 7 * 14 + 5, false, true, paused);
    CHECK(ev.action == th::MenuAction::None && ev.sound == th::MenuSound::Wrong);
    menu_step(bar, options_x, 5, true, false, paused);
    ev = menu_step(bar, options_x, 14 + 5, false, true, paused);
    CHECK(ev.action == th::MenuAction::ToggleSound);

    // Back from a submenu to its parent; released outside the menus.
    menu_step(bar, options_x, 5, true, false);
    menu_step(bar, options_x, 14 + 3 * 14 + 5, true, false);
    CHECK(bar.open == 8);
    menu_step(bar, options_x, 14 + 5, true, false);
    CHECK(bar.open == 2);
    ev = menu_step(bar, 600, 300, false, true);
    CHECK(ev.action == th::MenuAction::None && bar.open == 0);

    // File > Save > Game 3; a modal requester hides the bar.
    menu_step(bar, 20, 5, true, false);
    menu_step(bar, 20, 14 + 14 + 5, true, false);
    CHECK(bar.open == 7);
    const int save_x = bar.pulldowns[1].width * 9 + 10 + 5;
    ev = menu_step(bar, save_x, 28 + 2 * 14 + 5, false, true);
    CHECK(ev.action == th::MenuAction::Save && ev.value == 3);
    th::MenuContext modal;
    modal.disabled = true;
    menu_step(bar, 20, 5, true, false, modal);
    CHECK(!bar.active && bar.open == 0);
}

// The requester sheets share one numbering: each holds its own requester's
// sprites among 1 x 1 placeholders.
void test_requester_art(const char *dir)
{
    th::RequesterArt art;
    std::string error;
    CHECK(art.load(dir, &error));
    const th::SpriteSheet &yes_no = art.sheet(int(th::Requester::YesNo));
    CHECK(yes_no.size() == 415);
    CHECK(yes_no.at(357).width == 183 && yes_no.at(357).height == 22);
    CHECK(yes_no.at(362).width == 93 && yes_no.at(362).height == 53);
    CHECK(art.sheet(int(th::Requester::Jukebox)).at(389).width == 30);
    CHECK(art.sheet(7).size() == 0);
    CHECK(!art.font().empty());
}

void test_panel_buttons(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    CHECK(th::panel_top(gd) == 432);
    const int xs[] = {20, 40, 41, 100, 220, 260, 300, 340, 380, 420, 460, 500, 540, 580, 620};
    const int icons[] = {0, 0, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    for (size_t i = 0; i < std::size(xs); i++)
        CHECK(th::panel_icon_at(gd, true, xs[i], 460) == icons[i]);
    CHECK(th::panel_icon_at(gd, true, 220, 431) == th::kNoPanelIcon);
    CHECK(th::panel_icon_at(gd, false, 220, 470) == th::kIconBuildRoom);

    th::PanelState state;
    th::UiMouse m;
    m.x = 220;
    m.y = 300;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::None && !state.raised);
    m.y = 460;
    m.left_pressed = m.left_held = true;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::None);
    CHECK(state.raised && state.pressed == th::kIconBuildRoom);
    m.left_pressed = false;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::None && state.pressed == th::kIconBuildRoom);
    m.left_held = false;
    m.left_released = true;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::BuildRoom);
    CHECK(state.pressed == th::kNoPanelIcon);
    m.left_released = false;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::None);

    m.x = 100;
    m.left_pressed = m.left_released = true;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::Refused);
    m.left_pressed = m.left_released = false;
    m.right_pressed = true;
    m.x = 20;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::BankStatement);
    m.x = 340;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::StaffManagement);
    m.x = 620;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::Refused);
    CHECK(th::panel_frame(state, m, gd, true) == th::PanelAction::None && !state.raised);
    CHECK(th::panel_frame(state, m, gd, false, false) == th::PanelAction::None && state.raised);

    // The raised set with a pressed icon draws the pressed sprite.
    th::UiLayer a, b;
    a.resize(th::kUiWidth, th::kUiHeight);
    b.resize(th::kUiWidth, th::kUiHeight);
    a.clear();
    b.clear();
    th::PanelState raised;
    raised.raised = true;
    th::draw_panel_ui(a, raised, nullptr, gd);
    raised.pressed = th::kIconTownMap;
    th::draw_panel_ui(b, raised, nullptr, gd);
    std::vector<uint32_t> pa = a.to_argb(gd.palette()), pb = b.to_argb(gd.palette());
    CHECK(pa != pb);
    CHECK(std::equal(pa.begin(), pa.begin() + 432 * th::kUiWidth, pb.begin()));
}

// The yes/no requester drawn from its sheet, answered by clicks, Escape and
// a right click outside it.
void test_yes_no_requester(const char *dir)
{
    th::GameData gd;
    th::RequesterArt art;
    std::string error;
    CHECK(gd.load(dir, &error) && art.load(dir, &error));
    th::Interface ui;
    ui.open_requester(th::Requester::BuildRoom);
    ui.open_yes_no(gd.text().at(th::TextSection::YesNo, 0));
    CHECK(ui.modal() && ui.requester() == th::Requester::YesNo && ui.window_count() == 1);

    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    th::draw_interface(layer, ui, art);
    std::vector<uint32_t> argb = layer.to_argb(gd.palette());
    const th::Window &win = ui.window(int(th::Requester::YesNo));
    int inside = 0, outside = 0;
    for (int y = 0; y < th::kUiHeight; y++) {
        for (int x = 0; x < th::kUiWidth; x++) {
            bool in = x >= win.x && x < win.x + win.width && y >= win.y && y < win.y + win.height;
            if (argb[size_t(y * th::kUiWidth + x)])
                (in ? inside : outside)++;
        }
    }
    CHECK(inside > 180 * 190 && outside == 0);

    // Yes: held while pressed, answered on release; the requester stays.
    th::UiMouse m;
    m.x = win.x + 92 + 46;
    m.y = win.y + 148 + 26;
    th::InterfaceEvents ev = ui.frame(m);
    CHECK(ev.mouse_on_window && ev.yes_no == th::YesNoAnswer::None);
    m.left_pressed = m.left_held = true;
    ev = ui.frame(m);
    CHECK(ev.yes_no == th::YesNoAnswer::None && (win.buttons[11].flags & th::kButtonHeld));
    th::UiLayer held;
    held.resize(th::kUiWidth, th::kUiHeight);
    held.clear();
    th::draw_interface(held, ui, art);
    CHECK(held.to_argb(gd.palette()) != argb);
    m.left_pressed = m.left_held = false;
    m.left_released = true;
    ev = ui.frame(m);
    CHECK(ev.yes_no == th::YesNoAnswer::Yes && ev.triggered_button == 12);
    CHECK(ui.requester() == th::Requester::YesNo);
    ui.close_yes_no(false);
    CHECK(!ui.modal() && ui.window_count() == 0 && ui.requester() == th::Requester::None);

    // No reopens the requester the question covered.
    ui.open_requester(th::Requester::BuildRoom);
    ui.open_yes_no("?");
    m = {};
    m.x = win.x + 40;
    m.y = win.y + 170;
    m.left_pressed = m.left_held = true;
    ui.frame(m);
    m.left_pressed = m.left_held = false;
    m.left_released = true;
    ev = ui.frame(m);
    CHECK(ev.yes_no == th::YesNoAnswer::No && !ui.modal());
    CHECK(ui.requester() == th::Requester::BuildRoom && ui.window_count() == 1);
    ui.close_requester(th::Requester::BuildRoom);

    ui.open_yes_no("?");
    m = {};
    m.x = 10;
    m.y = 10;
    CHECK(ui.frame(m, true).yes_no == th::YesNoAnswer::No && !ui.modal());
    ui.open_yes_no("?");
    m.right_pressed = true;
    m.x = win.x + 10;
    m.y = win.y + 10;
    CHECK(ui.frame(m).yes_no == th::YesNoAnswer::None && ui.modal());
    m.x = 10;
    CHECK(ui.frame(m).yes_no == th::YesNoAnswer::No && ui.window_count() == 0);
}

// The main menu's picture, palette and hover sprites, and the other screens'.
void test_main_menu_art(const char *dir)
{
    auto pic = th::load_data_file(dir, "qdata/main01m.dat");
    auto pal = th::load_data_file(dir, "qdata/main01m.pal");
    CHECK(pic && pic->size() == 320 * 200 && pal && pal->size() == 768);
    th::LanguageText text;
    CHECK(th::load_language(dir, 0, text));
    th::Startup startup;
    std::string error;
    CHECK(startup.load(dir, text, 0, &error));
}

// From the intro to level 1 on the default difficulty with clicks and keys
// as a player gives them, skipping each movie.
void test_startup_new_game(const char *dir)
{
    th::LanguageText text;
    CHECK(th::load_language(dir, 0, text));
    th::Startup s;
    if (!s.load(dir, text, 0))
        return;
    th::StartupResult r;
    auto run = [&](th::UiMouse m, std::vector<th::StartupKey> keys = {}, int frames = 1) {
        for (int i = 0; i < frames && r.kind == th::StartupResult::Kind::Running; i++) {
            r = s.frame(m, keys, 0.04);
            keys.clear();
            m.left_pressed = m.left_released = false;
        }
    };
    auto click = [](int x, int y) {
        th::UiMouse m;
        m.x = x;
        m.y = y;
        m.left_pressed = m.left_released = true;
        return m;
    };
    th::UiMouse still;
    still.x = still.y = 0;
    const th::StartupKey space{th::StartupKey::Space, 0};

    run(still);
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieIntroSm4);
    CHECK(s.surface().width() == 320 && s.surface().height() == 200);
    CHECK(s.movie_audio_format().rate == 22050);
    run(still, {space});
    CHECK(s.screen() == th::StartupScreen::Title && s.surface().width() == 640);
    run(click(5, 5));
    run(still, {}, 60);
    CHECK(s.screen() == th::StartupScreen::Name);
    run(still, {{th::StartupKey::Letter, 'a'}, {th::StartupKey::Return, 0}}, 3);
    CHECK(s.player_name() == "A");
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieMainMenu1);
    run(still, {space}, 2);
    CHECK(s.screen() == th::StartupScreen::MainMenu && s.surface().width() == 320);

    // New Game is the diamond's top quarter; Continue, refused without a
    // save, its left one.
    run(click(0x2e, 0x3a), {}, 2);
    CHECK(s.screen() == th::StartupScreen::MainMenu);
    run(click(0x55, 0x20), {}, 2);
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieMainMenu2);
    run(still, {space}, 80);
    CHECK(s.screen() == th::StartupScreen::Preferences);
    run(click(350, 390), {}, 2); // accept
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieArea);
    run(still, {space}, 2);
    CHECK(s.screen() == th::StartupScreen::Briefing);
    run(click(320, 240));
    CHECK(r.kind == th::StartupResult::Kind::StartLevel);
    CHECK(r.level == 1 && r.balance_id == "FULL" && !r.tutorial);
    auto sounds = s.take_sounds();
    CHECK(std::count(sounds.begin(), sounds.end(), uint8_t(0x08)) == 1); // SFX_WRONG2 for Continue

    auto page = th::level_briefing(text, 1);
    CHECK(!page.empty());
}

// The town map: level 2 has plots 2 and 3 for sale, with no neighbour rule.
void test_town_map(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto level = gd.load_level(2, &error);
    th::Balance settings;
    CHECK(level && th::load_balance(dir, 2, th::kDefaultBalanceId, settings, &error));
    if (!level)
        return;
    auto game = std::make_unique<th::Game>();
    game->staff_market.ref_counts = th::reference_counts(gd.text());
    game->start(*level, 2, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());

    auto land = std::make_unique<th::TownLand>();
    th::start_town_land(*land, *level, 2);
    CHECK(land->for_sale[0] == 0x10 && land->for_sale[1] == 1 && land->for_sale[2] == 0 && land->for_sale[3] == 0);
    CHECK(th::land_amount(*land, 1) == 0 && th::land_amount(*land, 2) == 518 && th::land_amount(*land, 3) == 220);
    CHECK(th::can_buy_plot(*land, 2) == 2 && th::can_buy_plot(*land, 0) == 1);
    // Level 8's plot 4 borders plot 5 alone.
    auto level8 = gd.load_level(8, &error);
    auto land8 = std::make_unique<th::TownLand>();
    if (level8) {
        th::start_town_land(*land8, *level8, 8);
        CHECK(th::can_buy_plot(*land8, 4) == 0 && th::can_buy_plot(*land8, 3) == 3);
        land8->for_sale[5] = 1;
        CHECK(th::can_buy_plot(*land8, 4) == 4);
    }

    th::TownMap town;
    CHECK(town.load(dir, gd.text(), &error));
    if (!error.empty())
        std::fprintf(stderr, "%s\n", error.c_str());
    CHECK(!town.is_open());
    th::PanelState panel;
    panel.shown_balance = game->balance;
    auto go_to = [](int, int) { return true; };
    th::UiMouse m;
    m.x = 320;
    m.y = 470;
    auto step = [&](bool escape = false) { return town.frame(m, escape, *game, *land, panel, "Test", go_to); };
    town.open();
    th::TownMapResult r = step();
    CHECK(town.is_open() && !r.closed && r.event.type == th::EV_NONE && town.overlays() == 0x1f);
    const th::Surface &s = town.surface();
    CHECK(s.width() == th::kUiWidth && s.height() == th::kUiHeight);
    CHECK(std::any_of(town.palette().begin(), town.palette().end(), [](uint32_t c) { return (c & 0xffffff) != 0; }));

    // A cell of plot 2 and its pixel on the map.
    int cell = 0;
    for (int i = th::kMapSize + 1; i < th::kMapCells && !cell; i++)
        if (land->level.plots[size_t(i)] == 2 && i % th::kMapSize > 1 && i / th::kMapSize > 1 &&
            land->level.plots[size_t(i - 1)] == 2 && land->level.plots[size_t(i - th::kMapSize)] == 2 &&
            land->level.plots[size_t(i - th::kMapSize - 1)] == 2)
            cell = i;
    CHECK(cell != 0);
    const int px = 230 + (cell % th::kMapSize) * 3 + 1, py = 25 + (cell / th::kMapSize) * 3 + 1;
    uint8_t for_sale_colour = s.data()[size_t(py) * th::kUiWidth + size_t(px)];

    // A left click on it buys it, for 518 cells at the land's price.
    game->balance = 1000000;
    panel.shown_balance = game->balance;
    m.x = px;
    m.y = py;
    m.left_pressed = m.left_held = true;
    r = step();
    m.left_pressed = m.left_held = false;
    CHECK(r.event.type == th::EV_BUY_LAND && r.event.value == 2);
    CHECK(r.sounds.size() == 1 && r.sounds[0] == 0x45);
    game->event = r.event;
    CHECK(th::apply_town_event(*game, *land, game->event));
    const int cost = 518 * settings.gbv.land_cost_per_tile;
    CHECK(cost > 0 && game->balance == 1000000 - cost);
    CHECK(game->finance.entry_count > 0 && game->finance.entries[game->finance.entry_count - 1].type == th::kTxBuyLand);
    CHECK((game->world.map[size_t(cell)].owner_flags & 1) && land->for_sale[2] == 1);
    CHECK(th::land_amount(*land, 2) == 0 && land->level.plots[size_t(cell)] == 0);
    step();
    CHECK(s.data()[size_t(py) * th::kUiWidth + size_t(px)] != for_sale_colour);

    // The heating: up, then down, as commands.
    int heating = game->finance.heating_level;
    auto click = [&](int x, int y, bool right = false) {
        m.x = x;
        m.y = y;
        (right ? m.right_pressed : m.left_pressed) = true;
        (right ? m.right_held : m.left_held) = true;
        step();
        m = {x, y};
        (right ? m.right_released : m.left_released) = true;
        th::TownMapResult out = step();
        m = {x, y};
        return out;
    };
    r = click(171 + 9, 315 + 9);
    CHECK(r.event.type == th::EV_SET_HEATING && r.event.value == heating + 1);
    CHECK(th::apply_town_event(*game, *land, r.event) && game->finance.heating_level == heating + 1);
    r = click(70 + 9, 315 + 9);
    CHECK(r.event.type == th::EV_SET_HEATING && r.event.value == heating);
    // A right click on an overlay button shows that one alone; a left one
    // toggles it.
    click(140 + 27, 90 + 27, true);
    CHECK(town.overlays() == 2);
    click(140 + 27, 36 + 27);
    CHECK(town.overlays() == 3);
    // Escape closes the map on the next frame.
    step(true);
    CHECK(town.is_open());
    r = step();
    CHECK(r.closed && !town.is_open());
}

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
    // The list's last row, below the window, is drawn.
    int last_y = win.y + 79 + 7 * row.height + row.height / 2;
    CHECK(argb[size_t(last_y) * th::kUiWidth + size_t(win.x + 2 + left_end.width + 1)] != 0);

    th::UiMouse m;
    auto frame = [&](const th::InterfaceEvents &e) { return th::jukebox_frame(jukebox, ui, e, art, music, m, false); };
    auto click_button = [&](int button) {
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
    };

    music.start_level(13);
    CHECK(music.playing() && music.track() == 5);
    // Next ends the song; the frame's play_next moves on.
    th::JukeboxResult r = click_button(11);
    CHECK(r.sounds.size() == 1 && r.sounds[0] == th::kSfxSelectX && music.song_over(false));
    music.play_next(false);
    CHECK(music.playing() && music.track() == 6);
    // Previous goes back at once.
    click_button(10);
    CHECK(music.playing() && music.track() == 5 && !music.song_over(false));
    // Pause and play again.
    click_button(9);
    CHECK(!music.playing());
    click_button(9);
    CHECK(music.playing() && music.track() == 5 && !music.song_over(false));
    // Looping, next is ignored.
    click_button(13);
    CHECK(music.loop());
    click_button(11);
    CHECK(!music.song_over(false) && music.track() == 5);
    // Stop ends the song and the loop.
    click_button(12);
    CHECK(!music.playing() && !music.loop());
    click_button(9);
    CHECK(music.playing() && music.track() == 5);

    // The list: a click on track 2's title plays it, one on track 3's light
    // takes it out of the playlist, so next skips it.
    auto click_list = [&](int x, int y) {
        m = {};
        m.x = x;
        m.y = y;
        m.left_pressed = m.left_held = true;
        frame({});
        m = {};
        m.x = x;
        m.y = y;
        return frame({});
    };
    int tiles_end = 2;
    while (tiles_end < row.width * 7)
        tiles_end += row.width;
    auto row_y = [&](int track) { return win.y + 79 + track * row.height + row.height / 2; };
    click_list(win.x + 2 + left_end.width + 1, row_y(2));
    CHECK(music.playing() && music.track() == 2);
    r = click_list(win.x + tiles_end + left_end.width + 2, row_y(3));
    CHECK(!music.track_enabled(3) && music.track() == 2 && r.sounds.size() == 1);
    click_button(11);
    music.play_next(false);
    CHECK(music.track() == 4);

    r = click_button(16);
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

    // Each chart: its picture in its own palette, with its figures drawn over.
    for (int n = 0; n < th::kChartCount; n++) {
        th::chart_command(c, {th::EV_OPEN_SCREEN, uint16_t(n)});
        CHECK(c.open && c.chart == n);
        chart_step(c, game, hs);
        CHECK(c.palette == c.pictures[size_t(n)].palette);
        CHECK(c.pictures[size_t(n)].sprites.size() > 1);
        int lit = 0;
        for (int i = 0; i < 640 * 480; i++)
            lit += c.surface.data()[i] != 0;
        CHECK(lit > 640 * 480 / 2);
        CHECK(chart_drawn(c, {0, 0, 640, 480}) > 100);
    }

    // The bank statement: no lines, then a purchase listed.
    const th::Rect lines = {40, 70, 560, 14};
    th::chart_command(c, {th::EV_OPEN_SCREEN, th::kChartStatement});
    chart_step(c, game, hs);
    CHECK(game.finance.entry_count == 0 && chart_drawn(c, lines) == 0);
    th::change_balance(game.finance, game.balance, game.date.day, game.date.month, th::kTxBuyObject, -500);
    chart_step(c, game, hs);
    CHECK(chart_drawn(c, lines) > 0);

    // A click on the statement opens the bank manager, valuing the hospital.
    chart_click(c, game, hs, 300, 200);
    CHECK(c.open && c.chart == th::kChartBank && game.finance.max_loan >= 5000);
    chart_step(c, game, hs);
    // Borrowing and repaying $5000.
    int32_t before = game.balance;
    th::ChartsResult r = chart_click(c, game, hs, 200, 275);
    CHECK(r.event.type == th::EV_TAKE_LOAN);
    CHECK(game.balance == before + 5000 && game.finance.loan == 5000);
    const th::BankEntry &loan = game.finance.entries[game.finance.entry_count - 1u];
    CHECK(loan.type == th::kTxBankLoan && loan.amount == 5000);
    r = chart_click(c, game, hs, 60, 275);
    CHECK(r.event.type == th::EV_REPAY_LOAN && game.balance == before && game.finance.loan == 0);
    r = chart_click(c, game, hs, 60, 275);
    CHECK(r.event.type == th::EV_NONE && std::count(r.sounds.begin(), r.sounds.end(), 0x08) == 1);
    // The close button: back to the game on the next frame.
    chart_click(c, game, hs, 615, 455);
    CHECK(c.open);
    r = chart_step(c, game, hs);
    CHECK(r.event.type == th::EV_RETURN_FROM_SCREEN && !c.open);

    // Policy: dragging the send-home slider, then Escape.
    th::chart_command(c, {th::EV_OPEN_SCREEN, th::kChartPolicy});
    chart_step(c, game, hs);
    th::UiMouse m;
    m.x = 250;
    m.y = 140;
    m.left_pressed = m.left_held = true;
    chart_step(c, game, hs, m);
    CHECK(hs.send_home_threshold != 10 && hs.send_home_threshold <= hs.guess_cure_threshold);
    th::ChartKeys escape;
    escape.escape = true;
    chart_step(c, game, hs, {}, escape);
    chart_step(c, game, hs);
    chart_step(c, game, hs);
    CHECK(!c.open);

    // Casebook prices: general practice's share of the most patients will
    // pay, in hospital 0's market, kept within 50-200%.
    th::HospitalMarket &market = game.arrivals.hospitals[0];
    int32_t gp_max = th::max_price_for_treatment(market, f.settings, 1);
    th::apply_chart_event(game, hs, {th::EV_PRICE_UP, 30, 0, 1});
    CHECK(market.price_scale[1] == 13000 && market.price[1] == 13000 * gp_max / 10000);
    th::apply_chart_event(game, hs, {th::EV_PRICE_DOWN, 200, 0, 1});
    CHECK(market.price_scale[1] == 5000 && market.price[1] == 5000 * gp_max / 10000);
    th::apply_chart_event(game, hs, {th::EV_PRICE_DOWN, 1, 0, 1});
    CHECK(market.price_scale[1] == 5000);

    // The staff list shows a hired staff member; a click selects them and
    // the raise button raises their wage by a tenth.
    uint16_t id = th::alter_applicant(game.world, true, 0, th::kStaffDoctor);
    int cell = free_staff_cell(game.world, *f.level);
    CHECK(id != 0 && cell >= 0);
    if (!id || cell < 0)
        return;
    CHECK(th::place_staff_on_map(game.world, f.gd.anims(), f.settings, cell, id, false, game.turn_count));
    th::chart_command(c, {th::EV_OPEN_SCREEN, th::kChartStaff});
    chart_step(c, game, hs);
    CHECK(chart_drawn(c, {49, 82, 581, 27}) > 0);
    chart_click(c, game, hs, 150, 95);
    CHECK(c.value == id);
    int32_t salary = game.world.things[id].salary;
    r = chart_click(c, game, hs, 370, 435);
    CHECK(r.event.type == th::EV_INCREASE_STAFF_SALARIES && game.world.things[id].salary == salary + salary / 10);
    // The sack button asks first: the chart closes for the yes/no requester,
    // whose yes sacks them (EV_SACK_STAFF) and no cancels (EV_CANCEL_SACK).
    r = chart_click(c, game, hs, 460, 400);
    CHECK(r.event.type == th::EV_OPEN_YES_NO && !c.open && c.value == 0 && r.question == 5);
    CHECK(r.yes.type == th::EV_SACK_STAFF && r.yes.value == id && r.yes.arg == 0);
    CHECK(r.no.type == th::EV_CANCEL_SACK && r.no.value == id);

    // Switching charts with a key; research needs a research room.
    bool refused = false;
    th::Event e = th::open_chart_command(th::kChartGraphs, game, &refused);
    CHECK(e.type == th::EV_OPEN_SCREEN && !refused);
    th::chart_command(c, e);
    CHECK(c.open && c.chart == th::kChartGraphs && c.state == 0);
    e = th::open_chart_command(th::kChartResearch, game, &refused);
    CHECK(e.type == th::EV_NONE && refused);
    CHECK(th::chart_for_panel_action(th::PanelAction::BankStatement) == th::kChartStatement);
    CHECK(th::chart_for_panel_action(th::PanelAction::HireStaff) == -1);
}
