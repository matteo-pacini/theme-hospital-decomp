// The startup screens (new game, load screen) and the town map window
// (core/ui/startup.h, core/ui/town_map.h).

#include "common.h"

#include "core/data/balance.h"
#include "core/data/gamedata.h"
#include "core/data/level.h"
#include "core/data/palette.h"
#include "core/data/smacker.h"
#include "core/data/text.h"
#include "core/render/panel.h"
#include "core/render/surface.h"
#include "core/ui/charts.h"
#include "core/ui/hire_staff.h"
#include "core/ui/info_windows.h"
#include "core/ui/layer.h"
#include "core/ui/popups.h"
#include "core/ui/queue_window.h"
#include "core/ui/startup.h"
#include "core/ui/town_map.h"
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
#include <functional>
#include <memory>
#include <string>
#include <vector>

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

// From the intro, skipped, through the title and the about screen to the main
// menu.
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
    d.run(still, {}, 10); // the input guard after a skip
    d.run(startup_click(5, 5));
    d.run(still, {}, 100);
    CHECK(s.screen() == th::StartupScreen::About);
    d.run(still, {{th::StartupKey::Return, 0}}, 3);
    CHECK(s.player_name() == "HORZA");
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieMainMenu1);
    d.run(still, {space}, 2);
    CHECK(s.screen() == th::StartupScreen::MainMenu && s.surface().width() == 320);
    d.run(still, {}, 10);
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
    d.run(still, {}, 10);
    d.run(startup_click(320, 240));
    CHECK(d.r.kind == th::StartupResult::Kind::StartLevel);
    CHECK(d.r.level == 1 && d.r.balance_id == "FULL" && !d.r.tutorial);
    auto sounds = s.take_sounds();
    CHECK(std::count(sounds.begin(), sounds.end(), uint8_t(0x08)) == 1); // SFX_WRONG2 for Continue
    // Opening the preferences' glass boxes plays SFX_ZOOM_OPEN.
    CHECK(std::count(sounds.begin(), sounds.end(), uint8_t(0x9c)) >= 1);

    auto page = th::level_briefing(text, 1);
    CHECK(!page.empty());
}

namespace
{

// From the intro to the main menu's opening movie, playing.
void reach_menu_movie(StartupDriver &d)
{
    const th::UiMouse still = startup_still();
    d.run(still);
    d.run(still, {{th::StartupKey::Space, 0}});
    d.run(still, {}, 10);
    d.run(startup_click(5, 5));
    d.run(still, {}, 100);
    d.run(still, {{th::StartupKey::Return, 0}}, 3);
}

} // namespace

// A click or key that skips a movie does not also act on the screen after
// it: the menu ignores input until every button is up and 0.3 s have passed.
void test_startup_input_guard(const char *dir)
{
    th::LanguageText text;
    CHECK(th::load_language(dir, 0, text));
    th::Startup s;
    if (!s.load(dir, text, 0))
        return;
    StartupDriver d{s, {}};
    const th::UiMouse still = startup_still();
    reach_menu_movie(d);
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieMainMenu1);

    // New Game is a click on the diamond's top quarter (0x55, 0x20 in
    // test_startup_new_game): refused for the frames after the skip.
    d.run(startup_click(5, 5));
    CHECK(s.screen() == th::StartupScreen::MainMenu);
    d.run(startup_click(0x55, 0x20), {{th::StartupKey::Return, 0}}, 2);
    CHECK(s.screen() == th::StartupScreen::MainMenu);
    d.run(still, {}, 10);
    d.run(startup_click(0x55, 0x20), {}, 2);
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieMainMenu2);
}

// The same with the skipping button kept down: its release, however late, is
// not a click on the menu.
void test_startup_input_guard_held(const char *dir)
{
    th::LanguageText text;
    CHECK(th::load_language(dir, 0, text));
    th::Startup s;
    if (!s.load(dir, text, 0))
        return;
    StartupDriver d{s, {}};
    const th::UiMouse still = startup_still();
    reach_menu_movie(d);
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieMainMenu1);

    th::UiMouse down;
    down.x = 0x55;
    down.y = 0x20;
    down.left_held = down.left_pressed = true;
    d.run(down);
    down.left_pressed = false;
    d.run(down, {}, 30);
    CHECK(s.screen() == th::StartupScreen::MainMenu);
    th::UiMouse up;
    up.x = 0x55;
    up.y = 0x20;
    up.left_released = true;
    d.run(up);
    CHECK(s.screen() == th::StartupScreen::MainMenu);
    d.run(still);
    d.run(startup_click(0x55, 0x20), {}, 2);
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieMainMenu2);
}

namespace
{

// A real save of level 1's start, so the load screen and a load see a whole
// game and its format.
bool write_test_save(const char *dir, const std::filesystem::path &path)
{
    th::GameData gd;
    std::string error;
    th::Balance settings;
    auto level = gd.load(dir, &error) ? gd.load_level(1, &error) : std::nullopt;
    if (!level || !th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error))
        return false;
    th::Game game;
    game.staff_market.ref_counts = th::reference_counts(gd.text());
    game.start(*level, 1, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
    return th::write_save_slot(path, game, th::SaveInfo{0, std::string(th::kDefaultBalanceId), false, 0, 0}, &error);
}

} // namespace

// The main menu's Load entry and the load game screen (src/loadscr.cpp):
// with a save in the continue slot the main menu's Continue loads it, and
// the Load entry opens the screen, whose used slot loads when chosen and
// whose Back returns to the menu.
void test_startup_load_screen(const char *dir)
{
    th::LanguageText text;
    CHECK(th::load_language(dir, 0, text));
    std::filesystem::path save_dir = std::filesystem::path("build") / "ui-test-save";
    std::error_code ec;
    std::filesystem::remove_all(save_dir, ec);
    std::filesystem::path slot = th::save_slot_path(save_dir, std::string(text.misc(1)), 0);
    CHECK(write_test_save(dir, slot));
    CHECK(th::save_slot_compatible(slot) && th::save_slot_level(slot) == 1);

    th::Startup s;
    if (!s.load(dir, text, 0))
        return;
    s.set_save_dir(save_dir);
    StartupDriver d{s, {}};
    reach_main_menu(d);
    const th::UiMouse still = startup_still();
    const th::StartupKey space{th::StartupKey::Space, 0};

    // Load is the diamond's right quarter; its closing movie, then the
    // screen's panels open.
    d.run(startup_click(0x73, 0x3a), {}, 2);
    CHECK(d.r.kind == th::StartupResult::Kind::Running);
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieMainMenu2);
    d.run(still, {space}, 5);
    for (int i = 0; i < 60 && s.screen() != th::StartupScreen::LoadGame; i++)
        d.run(still, {}, 1);
    CHECK(s.screen() == th::StartupScreen::LoadGame);
    CHECK(s.surface().width() == 640 && s.surface().height() == 480);

    // The panels open over the 800 ms delay and, type by type, seven zoom
    // steps.
    d.run(still, {}, 60);
    CHECK(s.load_ready());

    // The used slot 0 loads (the first slot button, kLoadGlass[2]).
    d.run(startup_click(319, 85), {}, 3);
    CHECK(d.r.kind == th::StartupResult::Kind::LoadGame && d.r.slot == 0);

    // A load the caller refused: the load game screen stays up (loadscr's
    // state 1 failure), and its result is spent so the frame does not hand it
    // out again; the Continue failure returns to the main menu.
    s.load_failed(true);
    CHECK(s.screen() == th::StartupScreen::LoadGame);
    s.load_failed(false);
    CHECK(s.screen() != th::StartupScreen::LoadGame && s.screen() != th::StartupScreen::Done);
    d.r = th::StartupResult{};
    d.run(still);
    CHECK(d.r.kind == th::StartupResult::Kind::Running);
    std::filesystem::remove_all(save_dir, ec);
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
