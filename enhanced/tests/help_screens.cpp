// The full-screen screens' help tooltips (core/ui/charts.cpp,
// town_map.cpp, startup.cpp): a hotspot hovered long enough shows its
// language Help text, drawn over the screen in its own palette.

#include "common.h"

#include "core/data/text.h"
#include "core/render/panel.h"
#include "core/render/text.h"
#include "core/ui/charts.h"
#include "core/ui/helpmsg.h"
#include "core/ui/menubar.h"
#include "core/ui/startup.h"
#include "core/ui/town_map.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace
{

// The first hotspot of help `group` containing (x, y), or -1. Hotspots
// overlap, so the position is what chooses the entry.
int hotspot_at(const th::HelpSystem &help, int group, int x, int y)
{
    for (uint16_t i = help.help_start(group); i < th::kHiresHelpCount && th::kHiresHelp[i].x0 >= 0; i++) {
        const th::HelpHotspot &h = th::kHiresHelp[i];
        if (x >= h.x0 && x <= h.x1 && y >= h.y0 && y <= h.y1)
            return i;
    }
    return -1;
}

// The plain (no callback) hotspot of `group` at (x, y) and its Help text id.
// Fails the test if it is a callback or missing.
int plain_hotspot_id(const th::HelpSystem &help, int group, int x, int y)
{
    int i = hotspot_at(help, group, x, y);
    CHECK(i >= 0);
    if (i < 0)
        return -1;
    CHECK(th::kHiresHelp[i].func == nullptr);
    return th::kHiresHelp[i].text();
}

// The palette index nearest to (r, g, b), each 0-255 (find_colour's rule).
uint8_t nearest_colour(const th::Palette &palette, int r, int g, int b)
{
    auto at = [&](int i, int k) { return int((palette[size_t(i)] >> (16 - 8 * k)) & 0xff); };
    auto dist = [&](int i) {
        int dr = r - at(i, 0), dg = g - at(i, 1), db = b - at(i, 2);
        return dr * dr + dg * dg + db * db;
    };
    int best = 0;
    for (int i = 1; i < 256; i++)
        if (dist(i) < dist(best))
            best = i;
    return uint8_t(best);
}

// The drawn tooltip box: its outline and the fill inside it are the white and
// black of `palette`, and its text is black.
void check_tooltip_pixels(const th::Surface &surface, const th::Palette &palette, const th::HelpBox &box)
{
    uint8_t black = nearest_colour(palette, 0, 0, 0);
    uint8_t white = nearest_colour(palette, 255, 255, 255);
    auto pixel = [&](int x, int y) { return surface.data()[size_t(y) * size_t(surface.width()) + size_t(x)]; };
    CHECK(box.w > 2 && box.h > 2);
    CHECK(box.x >= 0 && box.y >= 0 && box.x + box.w <= surface.width() && box.y + box.h <= surface.height());
    if (box.x + box.w > surface.width() || box.y + box.h > surface.height())
        return;
    CHECK(pixel(box.x, box.y) == black);         // the outline
    CHECK(pixel(box.x + 1, box.y + 1) == white); // the fill inside it
    bool text = false;
    for (int y = box.y + 1; y < box.y + box.h - 1 && !text; y++)
        for (int x = box.x + 1; x < box.x + box.w - 1; x++)
            if (pixel(x, y) == black) {
                text = true;
                break;
            }
    CHECK(text);
}

// A level, its game and the help art, shared by the screen tests.
struct HelpFixture {
    th::GameData gd;
    std::string error;
    std::unique_ptr<th::Game> game;
    std::optional<th::Level> level;
    th::Balance settings;
    th::MenuBar bar;
    th::Font font;
    th::HelpSystem help;
    th::UiMouse mouse;
    th::HelpContext ctx;

    bool start(const char *dir, int level_number)
    {
        CHECK(gd.load(dir, &error));
        level = gd.load_level(level_number, &error);
        CHECK(level && th::load_balance(dir, level_number, th::kDefaultBalanceId, settings, &error));
        if (!level)
            return false;
        game = std::make_unique<th::Game>();
        game->staff_market.ref_counts = th::reference_counts(gd.text());
        game->start(*level, level_number, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
        CHECK(bar.load(dir, gd.text(), &error));
        CHECK(th::load_font(dir, 0, font, &error));
        help.set_art(font, bar.sprites);
        ctx.game = game.get();
        ctx.world = &game->world;
        ctx.text = &gd.text();
        ctx.help = &help.box();
        ctx.pulldv = &bar.sprites;
        ctx.mouse = &mouse;
        return true;
    }

    // Runs `frames` frames with the mouse at (x, y), the clock advancing a
    // frame time each, calling `step`; returns the tooltip text then.
    std::string hover(int x, int y, int frames, const std::function<void()> &step)
    {
        mouse = {x, y};
        uint32_t clock = 100000;
        for (int i = 0; i < frames; i++) {
            help.set_clock(clock);
            step();
            clock += 40;
        }
        return help.drawn_text();
    }

    std::string expected(int id) const
    {
        return th::HelpSystem::copy_and_format(gd.text().at(th::TextSection::Help, id));
    }
};

} // namespace

// A plain tooltip hotspot of every chart, after the 750 ms rest, is that
// hotspot's wrapped Help text.
void test_help_charts_screens(const char *dir)
{
    HelpFixture f;
    if (!f.start(dir, 1))
        return;
    th::Charts charts;
    CHECK(th::load_charts(charts, dir, f.gd.text(), f.gd.palette(), &f.error));
    charts.hospital_names[0] = "Test Hospital";
    charts.set_help(f.help, f.ctx);
    f.ctx.charts = &charts;
    th::HospitalSettings hs;
    th::start_hospital_settings(hs, f.settings);

    // One plain hotspot per chart: the mouse point and its help group.
    struct Case {
        int chart;
        int x, y;
        int group;
    };
    const Case cases[] = {
        {th::kChartStatement, 614, 454, 0xf}, {th::kChartCasebook, 476, 54, 0xe}, {th::kChartPolicy, 620, 460, 0x19},
        {th::kChartResearch, 620, 460, 0x10}, {th::kChartGraphs, 76, 397, 0x11},  {th::kChartStaff, 82, 44, 6},
        {th::kChartStatus, 329, 81, 0x18},    {th::kChartBank, 132, 145, 0x15},
    };
    for (const Case &c : cases) {
        int id = plain_hotspot_id(f.help, c.group, c.x, c.y);
        CHECK(id >= 0);
        if (id < 0)
            continue;
        th::chart_command(charts, {th::EV_OPEN_SCREEN, uint16_t(c.chart)});
        std::string msg = f.hover(c.x, c.y, 25, [&] { th::chart_frame(charts, f.mouse, {}, 0.04, *f.game, hs); });
        CHECK(msg == f.expected(id));
        CHECK(f.help.box().style == 1);
        check_tooltip_pixels(charts.surface, charts.palette, f.help.box());
    }
}

// The town map's tooltip: group 0x13, drawn over the map.
void test_help_town_map_screen(const char *dir)
{
    HelpFixture f;
    if (!f.start(dir, 2))
        return;
    th::TownLand land;
    th::start_town_land(land, *f.level, 2);
    th::TownMap town;
    CHECK(town.load(dir, f.gd.text(), &f.error));
    town.set_help(f.help, f.ctx);

    const int x = 163, y = 60;
    int id = plain_hotspot_id(f.help, 0x13, x, y);
    CHECK(id >= 0);
    if (id < 0)
        return;
    th::PanelState panel;
    panel.shown_balance = f.game->balance;
    town.open();
    auto step = [&] {
        town.frame({f.mouse, false}, *f.game, land, panel, "Test Hospital", [](int, int) { return false; });
    };
    CHECK(f.hover(x, y, 25, step) == f.expected(id));
    CHECK(f.help.box().style == 1);
    check_tooltip_pixels(town.surface(), town.palette(), f.help.box());
}

namespace
{

// From the intro, skipped, through the title and the player's name to the
// main menu (test_startup_new_game's path).
bool reach_main_menu(th::Startup &s)
{
    th::UiMouse still;
    still.x = still.y = 0;
    auto run = [&](th::UiMouse m, std::vector<th::StartupKey> keys, int frames) {
        for (int i = 0; i < frames; i++)
            s.frame(m, keys, 0.04);
    };
    run(still, {}, 1);
    run(still, {{th::StartupKey::Space, 0}}, 1);
    run({5, 5, false, false, true, false, true, false}, {}, 1);
    run(still, {}, 60);
    run(still, {{th::StartupKey::Letter, 'a'}, {th::StartupKey::Return, 0}}, 3);
    run(still, {{th::StartupKey::Space, 0}}, 2);
    return s.screen() == th::StartupScreen::MainMenu;
}

} // namespace

// The main menu's tooltip: the hovered entry's absolute hotspot (1..5) of
// group 1, after the rest.
void test_help_main_menu_screen(const char *dir)
{
    th::LanguageText text;
    CHECK(th::load_language(dir, 0, text));
    th::Startup startup;
    std::string error;
    CHECK(startup.load(dir, text, 0, &error));
    if (!reach_main_menu(startup))
        return;

    th::MenuBar bar;
    th::Font font;
    th::Font menu_font;
    CHECK(bar.load(dir, text, &error));
    CHECK(th::load_font(dir, 0, font, &error));
    CHECK(th::load_font(dir, 0x2d, menu_font, &error));
    th::HelpSystem help;
    help.set_art(font, bar.sprites);
    help.set_main_menu_font(menu_font);
    th::UiMouse mouse;
    th::HelpContext ctx;
    ctx.text = &text;
    ctx.help = &help.box();
    ctx.pulldv = &bar.sprites;
    ctx.mouse = &mouse;
    startup.set_help(help, ctx);

    // The new game entry's hotspot (index 1), text id 80.
    const int x = 80, y = 40;
    CHECK(th::kHiresHelp[1].x0 <= x && x <= th::kHiresHelp[1].x1);
    CHECK(th::kHiresHelp[1].y0 <= y && y <= th::kHiresHelp[1].y1);
    const int id = th::kHiresHelp[1].text();

    uint32_t clock = 100000;
    for (int frame = 0; frame < 25; frame++) {
        mouse = {x, y};
        help.set_clock(clock);
        startup.frame(mouse, {}, 0.04);
        clock += 40;
    }
    std::string_view raw = text.at(th::TextSection::Help, id);
    CHECK(!raw.empty());
    CHECK(help.drawn_text() == th::HelpSystem::copy_and_format(raw));
    CHECK(help.box().style == 1);
    check_tooltip_pixels(startup.surface(), startup.palette(), help.box());
}
