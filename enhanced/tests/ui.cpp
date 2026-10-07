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
#include "core/ui/fax.h"
#include "core/ui/hire_staff.h"
#include "core/ui/info_windows.h"
#include "core/ui/jukebox.h"
#include "core/ui/layer.h"
#include "core/ui/menu_tables.h"
#include "core/ui/menubar.h"
#include "core/ui/popups.h"
#include "core/ui/startup.h"
#include "core/ui/town_map.h"
#include "core/ui/windows.h"
#include "core/world/arrivals.h"
#include "core/world/events.h"
#include "core/world/finance.h"
#include "core/world/mayor.h"
#include "core/world/popups.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
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

// Every language's pulldown_text builds a menu bar.
void check_menu_languages(const char *dir)
{
    std::string error;
    for (int language = 0; language < 6; language++) {
        th::LanguageText lang;
        th::MenuBar bar;
        CHECK(th::load_language(dir, language, lang, &error));
        CHECK(bar.load(dir, lang, &error));
        CHECK(bar.count == 6 && bar.subcount == 12);
    }
}

// The English menus' titles, sizes and submenus.
void check_menu_layout(const th::MenuBar &bar)
{
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
}

// Hidden until the mouse reaches the top 22 lines; then held on File,
// dragged down to Quit and released.
void check_menu_quit(th::MenuBar &bar, const th::Palette &palette)
{
    menu_step(bar, 300, 200, false, false);
    CHECK(!bar.active && drawn_pixels(bar, palette) == 0);
    menu_step(bar, 300, 21, false, false);
    CHECK(bar.active && bar.open == 0);
    size_t bar_only = drawn_pixels(bar, palette);
    CHECK(bar_only >= size_t(th::kUiWidth) * 16);

    auto ev = menu_step(bar, 20, 5, true, false);
    CHECK(bar.open == 1 && ev.sound == th::MenuSound::Highlight);
    menu_step(bar, 20, 14 + 3 * 14 + 5, true, false);
    CHECK(bar.open == 1 && bar.active);
    CHECK(drawn_pixels(bar, palette) > bar_only);
    ev = menu_step(bar, 20, 14 + 3 * 14 + 5, false, true);
    CHECK(ev.action == th::MenuAction::Quit && ev.sound == th::MenuSound::Select);
    CHECK(bar.open == 0 && bar.active); // the release frame still belongs to the bar
    menu_step(bar, 20, 200, false, false);
    CHECK(!bar.active);
}

// Options > Game speed > each speed; refused while paused, when Options >
// Sound is not; back from a submenu to its parent and released outside the
// menus.
void check_menu_options(th::MenuBar &bar)
{
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
        auto ev = menu_step(bar, submenu_x, y, false, true);
        CHECK(ev.action == th::MenuAction::Speed && ev.value == 10 * (item + 1));
    }
    th::MenuContext paused;
    paused.paused = true;
    menu_step(bar, options_x, 5, true, false, paused);
    menu_step(bar, options_x, 14 + 7 * 14 + 5, true, false, paused);
    menu_step(bar, submenu_x, 14 + 7 * 14 + 5, true, false, paused);
    auto ev = menu_step(bar, submenu_x, 14 + 7 * 14 + 5, false, true, paused);
    CHECK(ev.action == th::MenuAction::None && ev.sound == th::MenuSound::Wrong);
    menu_step(bar, options_x, 5, true, false, paused);
    ev = menu_step(bar, options_x, 14 + 5, false, true, paused);
    CHECK(ev.action == th::MenuAction::ToggleSound);

    menu_step(bar, options_x, 5, true, false);
    menu_step(bar, options_x, 14 + 3 * 14 + 5, true, false);
    CHECK(bar.open == 8);
    menu_step(bar, options_x, 14 + 5, true, false);
    CHECK(bar.open == 2);
    ev = menu_step(bar, 600, 300, false, true);
    CHECK(ev.action == th::MenuAction::None && bar.open == 0);
}

} // namespace

// With the game data: the menu bar built from English pulldown_text
// (setup_menus), opened, dragged through and chosen from as process_menus
// does, and drawn with the PULLDV art. Every language builds.
void test_menu_bar(const char *dir)
{
    check_menu_languages(dir);
    std::string error;
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    th::MenuBar bar;
    CHECK(bar.load(dir, gd.text(), &error));
    if (bar.count != 6)
        return;
    check_menu_layout(bar);
    check_menu_quit(bar, gd.palette());
    check_menu_options(bar);

    // File > Save > Game 3; a modal requester hides the bar.
    menu_step(bar, 20, 5, true, false);
    menu_step(bar, 20, 14 + 14 + 5, true, false);
    CHECK(bar.open == 7);
    const int save_x = bar.pulldowns[1].width * 9 + 10 + 5;
    auto ev = menu_step(bar, save_x, 28 + 2 * 14 + 5, false, true);
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

namespace
{

// The raised set with a pressed icon draws the pressed sprite.
void check_pressed_icon_drawn(const th::GameData &gd)
{
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

} // namespace

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
    check_pressed_icon_drawn(gd);
}

namespace
{

// The open yes/no requester drawn: within its window and filling most of
// it. Returns the image.
std::vector<uint32_t> draw_yes_no(const th::Interface &ui, const th::RequesterArt &art, const th::Palette &palette)
{
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    th::draw_interface(layer, ui, art);
    std::vector<uint32_t> argb = layer.to_argb(palette);
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
    return argb;
}

// Yes: held while pressed, answered on release; the requester stays until
// closed. `drawn` is the requester drawn before.
void check_yes_answer(th::Interface &ui, const th::RequesterArt &art, const th::Palette &palette,
                      const std::vector<uint32_t> &drawn)
{
    const th::Window &win = ui.window(int(th::Requester::YesNo));
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
    CHECK(held.to_argb(palette) != drawn);
    m.left_pressed = m.left_held = false;
    m.left_released = true;
    ev = ui.frame(m);
    CHECK(ev.yes_no == th::YesNoAnswer::Yes && ev.triggered_button == 12);
    CHECK(ui.requester() == th::Requester::YesNo);
    ui.close_yes_no(false);
    CHECK(!ui.modal() && ui.window_count() == 0 && ui.requester() == th::Requester::None);
}

// No, clicked, reopens the requester the question covered; Escape and a
// right click outside the requester answer no too.
void check_no_answers(th::Interface &ui)
{
    const th::Window &win = ui.window(int(th::Requester::YesNo));
    ui.open_requester(th::Requester::BuildRoom);
    ui.open_yes_no("?");
    th::UiMouse m;
    m.x = win.x + 40;
    m.y = win.y + 170;
    m.left_pressed = m.left_held = true;
    ui.frame(m);
    m.left_pressed = m.left_held = false;
    m.left_released = true;
    th::InterfaceEvents ev = ui.frame(m);
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

} // namespace

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
    const std::vector<uint32_t> argb = draw_yes_no(ui, art, gd.palette());
    check_yes_answer(ui, art, gd.palette(), argb);
    check_no_answers(ui);
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

namespace
{

// The startup screens driven frame by frame with the mouse and keys.
struct StartupDriver {
    th::Startup &s;
    th::StartupResult r;

    // Up to `frames` frames while the screens run: `keys` and any click in
    // the first only.
    void run(th::UiMouse m, std::vector<th::StartupKey> keys = {}, int frames = 1)
    {
        for (int i = 0; i < frames && r.kind == th::StartupResult::Kind::Running; i++) {
            r = s.frame(m, keys, 0.04);
            keys.clear();
            m.left_pressed = m.left_released = false;
        }
    }
};

th::UiMouse startup_click(int x, int y)
{
    th::UiMouse m;
    m.x = x;
    m.y = y;
    m.left_pressed = m.left_released = true;
    return m;
}

th::UiMouse startup_still()
{
    th::UiMouse still;
    still.x = still.y = 0;
    return still;
}

// From the intro, skipped, through the title and the player's name to the
// main menu.
void reach_main_menu(StartupDriver &d)
{
    const th::Startup &s = d.s;
    const th::UiMouse still = startup_still();
    const th::StartupKey space{th::StartupKey::Space, 0};
    d.run(still);
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieIntroSm4);
    CHECK(s.surface().width() == 320 && s.surface().height() == 200);
    CHECK(s.movie_audio_format().rate == 22050);
    d.run(still, {space});
    CHECK(s.screen() == th::StartupScreen::Title && s.surface().width() == 640);
    d.run(startup_click(5, 5));
    d.run(still, {}, 60);
    CHECK(s.screen() == th::StartupScreen::Name);
    d.run(still, {{th::StartupKey::Letter, 'a'}, {th::StartupKey::Return, 0}}, 3);
    CHECK(s.player_name() == "A");
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieMainMenu1);
    d.run(still, {space}, 2);
    CHECK(s.screen() == th::StartupScreen::MainMenu && s.surface().width() == 320);
}

} // namespace

// From the intro to level 1 on the default difficulty with clicks and keys
// as a player gives them, skipping each movie.
void test_startup_new_game(const char *dir)
{
    th::LanguageText text;
    CHECK(th::load_language(dir, 0, text));
    th::Startup s;
    if (!s.load(dir, text, 0))
        return;
    StartupDriver d{s, {}};
    reach_main_menu(d);
    const th::UiMouse still = startup_still();
    const th::StartupKey space{th::StartupKey::Space, 0};

    // New Game is the diamond's top quarter; Continue, refused without a
    // save, its left one.
    d.run(startup_click(0x2e, 0x3a), {}, 2);
    CHECK(s.screen() == th::StartupScreen::MainMenu);
    d.run(startup_click(0x55, 0x20), {}, 2);
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieMainMenu2);
    d.run(still, {space}, 80);
    CHECK(s.screen() == th::StartupScreen::Preferences);
    d.run(startup_click(350, 390), {}, 2); // accept
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieArea);
    d.run(still, {space}, 2);
    CHECK(s.screen() == th::StartupScreen::Briefing);
    d.run(startup_click(320, 240));
    CHECK(d.r.kind == th::StartupResult::Kind::StartLevel);
    CHECK(d.r.level == 1 && d.r.balance_id == "FULL" && !d.r.tutorial);
    auto sounds = s.take_sounds();
    CHECK(std::count(sounds.begin(), sounds.end(), uint8_t(0x08)) == 1); // SFX_WRONG2 for Continue

    auto page = th::level_briefing(text, 1);
    CHECK(!page.empty());
}

namespace
{

// Level 2's plots 2 and 3 for sale, with no neighbour rule; level 8's plot
// 4 borders plot 5 alone.
void check_town_land(const th::GameData &gd, const th::Level &level, th::TownLand &land)
{
    std::string error;
    th::start_town_land(land, level, 2);
    CHECK(land.for_sale[0] == 0x10 && land.for_sale[1] == 1 && land.for_sale[2] == 0 && land.for_sale[3] == 0);
    CHECK(th::land_amount(land, 1) == 0 && th::land_amount(land, 2) == 518 && th::land_amount(land, 3) == 220);
    CHECK(th::can_buy_plot(land, 2) == 2 && th::can_buy_plot(land, 0) == 1);
    auto level8 = gd.load_level(8, &error);
    auto land8 = std::make_unique<th::TownLand>();
    if (level8) {
        th::start_town_land(*land8, *level8, 8);
        CHECK(th::can_buy_plot(*land8, 4) == 0 && th::can_buy_plot(*land8, 3) == 3);
        land8->for_sale[5] = 1;
        CHECK(th::can_buy_plot(*land8, 4) == 4);
    }
}

// The town map open on a game, driven frame by frame with the mouse.
struct TownDriver {
    th::TownMap &town;
    th::Game &game;
    th::TownLand &land;
    th::PanelState panel;
    th::UiMouse m;

    th::TownMapResult step(bool escape = false)
    {
        auto go_to = [](int, int) { return true; };
        return town.frame({m, escape}, game, land, panel, "Test", go_to);
    }

    // Presses the button at (x, y) in one frame and releases it in the
    // next. Returns the second frame's result.
    th::TownMapResult click(int x, int y, bool right = false)
    {
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
    }
};

// A cell of plot 2 whose neighbours above and to the left are plot 2 too,
// 0 if none.
int plot2_cell(const th::TownLand &land)
{
    int cell = 0;
    for (int i = th::kMapSize + 1; i < th::kMapCells && !cell; i++)
        if (land.level.plots[size_t(i)] == 2 && i % th::kMapSize > 1 && i / th::kMapSize > 1 &&
            land.level.plots[size_t(i - 1)] == 2 && land.level.plots[size_t(i - th::kMapSize)] == 2 &&
            land.level.plots[size_t(i - th::kMapSize - 1)] == 2)
            cell = i;
    return cell;
}

// A left click on a cell of plot 2 on the map buys it, for 518 cells at the
// land's price, and its pixel changes.
void check_plot_bought(TownDriver &t, const th::Balance &settings)
{
    th::Game &game = t.game;
    const th::Surface &s = t.town.surface();
    const int cell = plot2_cell(t.land);
    CHECK(cell != 0);
    const int px = 230 + (cell % th::kMapSize) * 3 + 1, py = 25 + (cell / th::kMapSize) * 3 + 1;
    uint8_t for_sale_colour = s.data()[size_t(py) * th::kUiWidth + size_t(px)];

    game.balance = 1000000;
    t.panel.shown_balance = game.balance;
    t.m.x = px;
    t.m.y = py;
    t.m.left_pressed = t.m.left_held = true;
    th::TownMapResult r = t.step();
    t.m.left_pressed = t.m.left_held = false;
    CHECK(r.event.type == th::EV_BUY_LAND && r.event.value == 2);
    CHECK(r.sounds.size() == 1 && r.sounds[0] == 0x45);
    game.event = r.event;
    CHECK(th::apply_town_event(game, t.land, game.event));
    const int cost = 518 * settings.gbv.land_cost_per_tile;
    CHECK(cost > 0 && game.balance == 1000000 - cost);
    CHECK(game.finance.entry_count > 0 && game.finance.entries[game.finance.entry_count - 1].type == th::kTxBuyLand);
    CHECK((game.world.map[size_t(cell)].owner_flags & 1) && t.land.for_sale[2] == 1);
    CHECK(th::land_amount(t.land, 2) == 0 && t.land.level.plots[size_t(cell)] == 0);
    t.step();
    CHECK(s.data()[size_t(py) * th::kUiWidth + size_t(px)] != for_sale_colour);
}

// The heating, up then down, as commands; a right click on an overlay button
// shows that one alone, a left one toggles it; Escape closes the map on the
// next frame.
void check_town_buttons(TownDriver &t)
{
    int heating = t.game.finance.heating_level;
    th::TownMapResult r = t.click(171 + 9, 315 + 9);
    CHECK(r.event.type == th::EV_SET_HEATING && r.event.value == heating + 1);
    CHECK(th::apply_town_event(t.game, t.land, r.event) && t.game.finance.heating_level == heating + 1);
    r = t.click(70 + 9, 315 + 9);
    CHECK(r.event.type == th::EV_SET_HEATING && r.event.value == heating);
    t.click(140 + 27, 90 + 27, true);
    CHECK(t.town.overlays() == 2);
    t.click(140 + 27, 36 + 27);
    CHECK(t.town.overlays() == 3);
    t.step(true);
    CHECK(t.town.is_open());
    r = t.step();
    CHECK(r.closed && !t.town.is_open());
}

} // namespace

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
    check_town_land(gd, *level, *land);

    th::TownMap town;
    CHECK(town.load(dir, gd.text(), &error));
    if (!error.empty())
        std::fprintf(stderr, "%s\n", error.c_str());
    CHECK(!town.is_open());
    TownDriver t{town, *game, *land, {}, {}};
    t.panel.shown_balance = game->balance;
    t.m.x = 320;
    t.m.y = 470;
    town.open();
    th::TownMapResult r = t.step();
    CHECK(town.is_open() && !r.closed && r.event.type == th::EV_NONE && town.overlays() == 0x1f);
    const th::Surface &s = town.surface();
    CHECK(s.width() == th::kUiWidth && s.height() == th::kUiHeight);
    CHECK(std::any_of(town.palette().begin(), town.palette().end(), [](uint32_t c) { return (c & 0xffffff) != 0; }));
    check_plot_bought(t, settings);
    check_town_buttons(t);
}

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

namespace
{

// Presses and releases the left button on button `button` (counted from 1)
// of window `window`; the frame whose callback ran, or the release's.
th::InterfaceEvents click_button(th::Interface &ui, int window, int button)
{
    const th::Window &win = ui.window(window);
    const th::WindowButton &b = win.buttons[size_t(button - 1)];
    th::UiMouse m;
    m.x = win.x + b.x + b.width / 2;
    m.y = win.y + b.y + b.height / 2;
    m.left_pressed = m.left_held = true;
    th::InterfaceEvents ev = ui.frame(m);
    if (ev.triggered_button >= 0)
        return ev;
    m.left_pressed = m.left_held = false;
    m.left_released = true;
    return ui.frame(m);
}

// The mouse at (x, y) of window `window`.
th::UiMouse window_mouse(const th::Interface &ui, int window, int x, int y)
{
    th::UiMouse m;
    m.x = ui.window(window).x + x;
    m.y = ui.window(window).y + y;
    return m;
}

constexpr int kPatient = int(th::Requester::Patient), kStaff = int(th::Requester::Staff),
              kHandyman = int(th::Requester::Handyman);

// The info windows on level 1 with a doctor, a handyman and two patients.
struct InfoFixture {
    QueueFixture f;
    th::RequesterArt art;
    th::HireStaffArt hire_art;
    th::Interface ui;
    th::InfoWindows info;
    th::UiLayer layer;
    th::ScrollArea scroll = {};
    uint16_t doctor = 0, handyman = 0, patient = 0, patient2 = 0;

    // Whether the level started and all four could be put on the map.
    bool start(const char *dir)
    {
        std::string error;
        CHECK(f.start(dir) && art.load(dir, &error) && hire_art.load(dir, &error));
        if (!f.level)
            return false;
        start_rooms(f);
        th::Game &g = *f.game;
        const int start = f.level->camera_start[0];
        scroll = {f.level->scroll_min, f.level->scroll_max};
        doctor = hire_near(f, th::kStaffDoctor, start);
        handyman = hire_near(f, th::kStaffHandyman, start + 2 * th::kMapSize);
        patient = g.add_walker(start + 4 * th::kMapSize);
        patient2 = g.add_walker(start + 5 * th::kMapSize);
        CHECK(doctor && handyman && patient && patient2);
        layer.resize(th::kUiWidth, th::kUiHeight);
        return doctor && handyman && patient && patient2;
    }

    th::World &world() { return f.game->world; }

    th::InfoWindowResult frame(const th::InterfaceEvents &ev, const th::UiMouse &m = {})
    {
        return th::info_window_frame(info, ui, ev, m, {world(), f.gd.text(), scroll, f.game->balance});
    }

    void draw_windows() { th::draw_info_windows(layer, info, ui, {art, hire_art, f.gd.text()}, world(), 0xc8); }

    uint8_t pixel(int window, int x, int y)
    {
        return layer.pixels().row(ui.window(window).y + y)[ui.window(window).x + x];
    }
};

// Hovering gives the person and their pointer, nothing off them; a click
// opens the window, closing another, and draws it undiagnosed: the casebook
// and guess cure buttons covered, the graph drawn.
void check_patient_window_opens(InfoFixture &t)
{
    th::World &w = t.world();
    th::MapHover h = th::map_hover(w, t.patient);
    CHECK(h.hit && h.thing == t.patient && h.pointer == th::kPointerPatient);
    h = th::map_hover(w, t.doctor);
    CHECK(h.hit && h.thing == t.doctor && h.pointer == th::kPointerStaff);
    CHECK(!th::map_hover(w, 0).hit && th::map_hover(w, 0).pointer == th::kPointerDefault);

    t.ui.open_window(kStaff);
    th::Event e = th::map_left_click(w, th::map_hover(w, t.patient), t.ui);
    CHECK(e.type == th::EV_OPEN_THING_WINDOW && e.value == t.patient && t.ui.window_count() == 0);
    th::info_window_event(t.info, t.ui, w, e);
    CHECK(t.ui.is_open(kPatient) && t.ui.window_count() == 1 && t.info.patient == t.patient && t.info.show_graph);
    CHECK(t.ui.requester() == th::Requester::None);

    t.layer.clear();
    th::draw_interface(t.layer, t.ui, t.art);
    t.draw_windows();
    CHECK(t.pixel(kPatient, 167 + 10, 164 + 10) == 0x46);
    CHECK(t.pixel(kPatient, 167 + 10, 115 + 10) == 0x46);
    int graph = 0;
    for (int y = 28; y <= 101; y++)
        for (int x = 99; x <= 198; x++)
            graph += t.pixel(kPatient, x, y) == 0xc8;
    CHECK(graph > 50);
}

// Send home; guess cure only once diagnosed, with a treatment, its illness
// not the one known.
void check_patient_buttons(InfoFixture &t)
{
    th::InterfaceEvents ev = click_button(t.ui, kPatient, 9);
    CHECK(ev.triggered_window == kPatient && ev.triggered_button == 9);
    th::InfoWindowResult r = t.frame(ev);
    CHECK(r.event.type == th::EV_PERSON_GO_HOME && r.event.value == t.patient);
    CHECK(r.sounds.size() == 1 && r.sounds[0] == th::kSfxSack);
    r = t.frame(click_button(t.ui, kPatient, 11));
    CHECK(r.event.type == th::EV_NONE);
    th::Thing &p = t.world().things[t.patient];
    p.diagnosis = 1000;
    p.known_illness = 1;
    p.illness = p.treatment = 5;
    r = t.frame(click_button(t.ui, kPatient, 11));
    CHECK(r.event.type == th::EV_FORCE_CURE && r.event.value == t.patient);
    r = t.frame(click_button(t.ui, kPatient, 8));
    CHECK(r.event.type == th::EV_NONE && r.casebook == 5);
}

// The treatments in place of the graph, then the next patient; then the
// window closed.
void check_patient_treatments_and_next(InfoFixture &t)
{
    th::World &w = t.world();
    th::UiMouse m = window_mouse(t.ui, kPatient, 78 + 10, 25 + 10);
    m.left_pressed = m.left_held = true;
    t.frame({}, m);
    CHECK(!t.info.show_graph && t.info.toggle_delay == 5);
    t.frame({}, m);
    CHECK(!t.info.show_graph);
    t.layer.clear();
    t.draw_windows();
    CHECK(t.pixel(kPatient, 78 + 2, 25 + 2) == 0xa9);
    m = window_mouse(t.ui, kPatient, 44 + 10, 211 + 10);
    m.left_pressed = m.left_held = true;
    th::InfoWindowResult r = t.frame({}, m);
    CHECK(r.centre_on == t.patient);
    m.left_pressed = m.left_held = false;
    m.right_pressed = m.right_held = true;
    r = t.frame({}, m);
    CHECK(t.info.patient == th::find_next_thing(w, t.patient, th::kThingPatient, -1, -1, t.scroll));
    CHECK(t.info.patient != t.patient && r.sounds.size() == 1);
    r = t.frame({}, m);
    CHECK(r.sounds.empty());

    r = t.frame(click_button(t.ui, kPatient, 6));
    CHECK(r.event.type == th::EV_CLOSE_QUERY && r.event.value == 1);
    th::info_window_event(t.info, t.ui, w, r.event);
    CHECK(t.ui.window_count() == 0);
}

// The staff window drawn: the face's border; the qualifications covered for
// all but doctors.
void check_staff_window_drawn(InfoFixture &t)
{
    th::World &w = t.world();
    t.layer.clear();
    th::draw_interface(t.layer, t.ui, t.art);
    t.draw_windows();
    CHECK(t.pixel(kStaff, 47, 134 - 1) == 0x9c);
    auto covered = [&] {
        int n = 0;
        for (int y = 141; y < 141 + 45; y++)
            for (int x = 41; x < 41 + 176; x++)
                n += t.pixel(kStaff, x, y) == 0x7a;
        return n;
    };
    CHECK(covered() < 176 * 45);
    w.things[t.doctor].subtype = th::kStaffNurse;
    t.draw_windows();
    CHECK(covered() == 176 * 45);
    w.things[t.doctor].subtype = th::kStaffDoctor;
}

// The staff window: sacking asks first; no opens the window again; the
// staff list and picking up from its buttons.
void check_staff_window(InfoFixture &t)
{
    th::World &w = t.world();
    th::info_window_event(t.info, t.ui, w, th::map_left_click(w, th::map_hover(w, t.doctor), t.ui));
    CHECK(t.ui.is_open(kStaff) && t.info.staff == t.doctor);
    check_staff_window_drawn(t);
    th::InfoWindowResult r = t.frame(click_button(t.ui, kStaff, 16));
    CHECK(r.event.type == th::EV_OPEN_YES_NO && r.question == 5 && t.ui.window_count() == 0);
    CHECK(r.yes.type == th::EV_SACK_STAFF && r.yes.value == t.doctor && r.yes.arg == 0);
    CHECK(r.no.type == th::EV_CANCEL_SACK && r.no.value == t.doctor);
    th::info_window_event(t.info, t.ui, w, r.no);
    CHECK(t.ui.is_open(kStaff) && t.info.staff == t.doctor);
    th::UiMouse m = window_mouse(t.ui, kStaff, 47 + 10, 53 + 10);
    m.left_pressed = m.left_held = true;
    r = t.frame({}, m);
    CHECK(r.event.type == th::EV_OPEN_SCREEN && r.event.value == 5 && r.slist_staff == t.doctor);
    r = t.frame(click_button(t.ui, kStaff, 17));
    CHECK(r.event.type == th::EV_PICK_UP_STAFF && r.event.value == t.doctor);
    th::info_window_event(t.info, t.ui, w, r.event);
    CHECK(t.ui.window_count() == 0);
}

// The handyman's window: his priorities' buttons act while held.
void check_handyman_window(InfoFixture &t)
{
    th::info_window_event(t.info, t.ui, t.world(), {th::EV_OPEN_THING_WINDOW, t.handyman});
    CHECK(t.ui.is_open(kHandyman) && !t.ui.is_open(kStaff) && t.info.staff == t.handyman);
    for (int button : {29, 30, 31}) {
        th::InterfaceEvents ev = click_button(t.ui, kHandyman, button);
        CHECK(ev.triggered_button == button);
        th::InfoWindowResult r = t.frame(ev);
        CHECK(r.event.type == th::EV_ADJUST_HANDYMAN_VALUES && r.event.value == t.handyman && r.event.arg == button);
        t.frame({});
    }
    t.layer.clear();
    t.draw_windows();
    CHECK(t.pixel(kHandyman, 55, 202) != 0);
    th::InfoWindowResult r = t.frame(click_button(t.ui, kHandyman, 26));
    CHECK(r.event.type == th::EV_CLOSE_QUERY && r.event.value == 2);
}

// The window closes once its person is gone; another player's people open
// nothing.
void check_window_follows_person(InfoFixture &t)
{
    th::World &w = t.world();
    th::info_window_event(t.info, t.ui, w, {th::EV_OPEN_THING_WINDOW, t.patient2});
    CHECK(t.ui.is_open(kPatient));
    t.frame({});
    CHECK(t.ui.is_open(kPatient));
    th::Rect box;
    th::Point camera;
    CHECK(th::info_view(t.info, t.ui, w, box, camera) && box.w == 77 && box.h == 77);
    w.things[t.patient2].type = th::kThingFree;
    t.frame({});
    CHECK(t.ui.window_count() == 0 && !th::info_view(t.info, t.ui, w, box, camera));
    w.things[t.patient2].type = th::kThingPatient;

    w.things[t.patient2].owner = 1;
    CHECK(th::map_left_click(w, th::map_hover(w, t.patient2), t.ui).type == th::EV_NONE);
    th::info_window_event(t.info, t.ui, w, {th::EV_OPEN_THING_WINDOW, t.patient2});
    CHECK(t.ui.window_count() == 0);
}

} // namespace

// The patient, staff and handyman windows (src/req3.cpp, src/req4.cpp):
// what hovering and clicking a person on the map gives, the windows the
// commands open and close, their buttons' commands from clicks where the
// art puts them, their own clicks (the treatments, the next person, the
// staff list), closing once their person is gone, and their drawing.
void test_info_windows(const char *dir)
{
    auto t = std::make_unique<InfoFixture>();
    if (!t->start(dir))
        return;
    check_patient_window_opens(*t);
    check_patient_buttons(*t);
    check_patient_treatments_and_next(*t);
    check_staff_window(*t);
    check_handyman_window(*t);
    check_window_follows_person(*t);
}

namespace
{

// Game frames of the fax icons, the mouse at `m`, until `done` or `limit`
// frames; the frames run.
int popup_frames(th::PopupIcons &icons, th::World &w, const th::GameData &gd, const th::UiMouse &m, int limit,
                 const std::function<bool()> &done)
{
    for (int i = 0; i < limit; i++) {
        if (done())
            return i;
        th::popup_icons_frame(icons, w, m, gd, 0.04);
    }
    return limit;
}

bool popups_settled(const th::World &w)
{
    for (size_t kind = 1; kind < th::kPopupIcons.size(); kind++)
        if (uint16_t head = w.first_call[0][kind]; head && w.calls[head].slide_step)
            return false;
    return true;
}

// The kinds of the icons level with their slots, left to right.
std::vector<int> level_kinds(const th::PopupIcons &icons)
{
    std::vector<std::pair<int, int>> by_x;
    for (uint8_t i = 0; i < icons.level_count; i++)
        by_x.emplace_back(icons.level[i].x, icons.level[i].kind);
    std::sort(by_x.begin(), by_x.end());
    std::vector<int> kinds;
    for (auto [x, kind] : by_x)
        kinds.push_back(kind);
    return kinds;
}

// The icons settled, a game frame on: those level with their slots.
void popups_settle(th::PopupIcons &icons, th::World &w, const th::GameData &gd)
{
    th::UiMouse away;
    popup_frames(icons, w, gd, away, 200, [&] { return popups_settled(w); });
    th::popup_icons_frame(icons, w, away, gd, 0.04);
}

// Three calls of three kinds and a second of the first kind: the three
// rise and slide into their slots, the fourth has none.
void check_popups_slide(th::PopupIcons &icons, th::World &w, const th::GameData &gd)
{
    uint16_t first = th::make_a_playercall(w, 4, 10, 0, 0);
    th::make_a_playercall(w, 7, 22, 0xf, 0);
    th::make_a_playercall(w, 8, 11, 0, 0);
    uint16_t second = th::make_a_playercall(w, 4, 12, 0, 0);
    CHECK(first == 1 && second == 4 && w.popups.new_faxes == 4);
    CHECK(w.first_call[0][4] == 1 && w.first_call[0][7] == 2 && w.first_call[0][8] == 3);
    CHECK(w.calls[1].slot == 0 && w.calls[2].slot == 1 && w.calls[3].slot == 2 && w.calls[1].slide_step == 1);
    CHECK(w.popups.shutter_state == 2);
    th::UiMouse away;
    th::popup_icons_frame(icons, w, away, gd, 0.04);
    CHECK(w.popups.pos[3] == 0 && icons.rising_count == 3 && icons.level_count == 0);
    CHECK(icons.rising[0].y == 29 && icons.rising[0].x == th::kPopupSlotX[0] + 174 && w.calls[1].slide_step == 1);
    int frames = popup_frames(icons, w, gd, away, 200, [&] { return popups_settled(w); });
    CHECK(frames > 13 && frames < 100);
    th::popup_icons_frame(icons, w, away, gd, 0.04);
    CHECK(icons.rising_count == 0 && level_kinds(icons) == std::vector<int>({4, 7, 8}));
    for (uint8_t i = 0; i < icons.level_count; i++)
        CHECK(icons.level[i].x == th::kPopupSlotX[w.calls[w.first_call[0][icons.level[i].kind]].slot]);
}

// The mouse over the second icon: it answers the new illness's call; just
// above the icons, nothing.
void check_popups_hover(th::PopupIcons &icons, th::World &w, const th::GameData &gd)
{
    th::UiMouse over;
    over.x = th::kPopupSlotX[1] + 2;
    over.y = icons.level[0].hover_y + 2;
    th::popup_icons_frame(icons, w, over, gd, 0.0);
    CHECK(icons.hovered == 7);
    th::Event click = th::popup_icon_click(icons);
    CHECK(click.type == th::EV_ANSWER_PLAYERCALL && click.value == 7 && click.arg == 0);
    over.y = icons.level[0].hover_y - 1;
    th::popup_icons_frame(icons, w, over, gd, 0.0);
    CHECK(icons.hovered == 0 && th::popup_icon_click(icons).type == th::EV_NONE);
}

// Drawn in their window above the panel's left end, which is redrawn.
void check_popups_drawn(const th::PopupIcons &icons, const th::GameData &gd)
{
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    th::draw_popup_icons(layer, icons, gd, {}, nullptr);
    std::vector<uint32_t> argb = layer.to_argb(gd.palette());
    int top = th::kUiHeight, right = 0;
    for (int y = 0; y < th::panel_top(gd); y++)
        for (int x = 0; x < th::kUiWidth; x++)
            if (argb[size_t(y * th::kUiWidth + x)]) {
                top = std::min(top, y);
                right = std::max(right, x);
            }
    CHECK(top == icons.level[0].hover_y + th::kPopupWindowDy);
    CHECK(right == th::kPopupSlotX[1] + gd.panel().at(th::kPopupIcons[4]).width - 1);
    CHECK(argb[size_t((th::kUiHeight - 1) * th::kUiWidth + 10)] != 0);
}

} // namespace

// The panel's fax icons: a call of each kind takes the next slot and rises
// from behind the panel, held until the message bar's shutter has closed,
// then slides left into its slot; a second call of a kind shows no icon of
// its own. The icon under the mouse answers its kind's first call. As
// calls are answered or expire the icons close up, the next call of a kind
// taking over its icon.
void test_panel_popups(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::World &w = f.game->world;
    th::PopupIcons icons;
    check_popups_slide(icons, w, f.gd);
    check_popups_hover(icons, w, f.gd);
    // The first call answered: the second of its kind takes its icon, at the
    // end, the others closing up.
    CHECK(th::get_playercall(w, 4, 1, 0) == 10 && w.first_call[0][4] == 4);
    popups_settle(icons, w, f.gd);
    CHECK(level_kinds(icons) == std::vector<int>({7, 8, 4}));
    // The new illness's call expires after 15 days.
    for (int day = 0; day < 15; day++)
        th::expire_playercalls(w);
    CHECK(w.first_call[0][7] == 0);
    popups_settle(icons, w, f.gd);
    CHECK(level_kinds(icons) == std::vector<int>({8, 4}));
    check_popups_drawn(icons, f.gd);
}

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

} // namespace

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
    check_fax_emergency(fax, g, safeseed, f.gd.text());
    check_fax_vip(fax, g, safeseed);
    check_fax_patient(fax, g, safeseed, patient);
    check_fax_closing(fax, g, safeseed);
    check_fax_keypad(fax, g, safeseed, patient);
}
