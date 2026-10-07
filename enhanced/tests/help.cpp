// The help system (core/ui/helpmsg.*): the hireshelp table, the group
// indices, the tooltip timing and, with the game data, the text and pages.

#include "common.h"

#include "core/data/text.h"
#include "core/render/surface.h"
#include "core/ui/helpmsg.h"
#include "core/ui/layer.h"
#include "core/ui/menubar.h"

#include <cstdint>
#include <string>
#include <vector>

namespace
{

th::HelpContext game_context(const th::LanguageText *text, th::HelpBox &box, const th::UiMouse &mouse)
{
    th::HelpContext ctx;
    ctx.text = text;
    ctx.help = &box;
    ctx.mouse = &mouse;
    return ctx;
}

bool surface_all_zero(th::Surface &s)
{
    for (int y = 0; y < s.height(); y++)
        for (int x = 0; x < s.width(); x++)
            if (s.row(y)[x] != 0)
                return false;
    return true;
}

} // namespace

// The generated table: 343 entries, a -2 at the end and 26 -1 group ends,
// and every hotspot inside the 640 x 480 screen.
void test_help_table()
{
    CHECK(th::kHiresHelpCount == 343);
    CHECK(th::kHiresHelp[th::kHiresHelpCount - 1].x0 == -2);
    int ends = 0;
    for (int i = 0; i < th::kHiresHelpCount; i++) {
        const th::HelpHotspot &h = th::kHiresHelp[i];
        if (h.x0 == -1)
            ends++;
        if (h.x0 < 0)
            continue;
        CHECK(h.x0 >= 0 && h.y0 >= 0 && h.x1 >= h.x0 && h.y1 >= h.y0);
        CHECK(h.x1 < 640 && h.y1 < 480);
    }
    CHECK(ends == 26);
}

// setup_help_indices: where each group's hotspots begin.
void test_help_group_starts()
{
    th::HelpSystem help;
    const int expected[] = {0,   1,   7,   24,  41,  57,  92,  153, 165, 168, 179, 195, 202, 216,
                            221, 236, 238, 252, 263, 269, 282, 290, 300, 307, 310, 325, 332};
    for (int g = 0; g <= 26; g++)
        CHECK(help.help_start(g) == expected[g]);
    // The first hotspot of each group is a real one.
    for (int g = 1; g <= 26; g++) {
        uint16_t s = help.help_start(g);
        CHECK(s < th::kHiresHelpCount && th::kHiresHelp[s].x0 >= 0);
    }
}

// help_hotspot_detected: no tooltip before 750 ms of rest, one at once within
// 500 ms of the previous, none after a longer gap.
void test_help_timing()
{
    th::HelpSystem help;
    th::UiMouse mouse{80, 40, false, false, false, false, false, false};
    th::HelpContext ctx = game_context(nullptr, help.box(), mouse);

    // The first hotspot of group 1, hovered from clock 10000.
    help.set_clock(10000);
    help.compute_help_text(ctx, 0, 0, 1);
    CHECK(help.box().hotspot == 1);
    CHECK(help.box().shown_time == 0);

    help.set_clock(10750); // 750 ms up: not yet
    help.compute_help_text(ctx, 0, 0, 1);
    CHECK(help.box().shown_time == 0);

    help.set_clock(10751);
    help.compute_help_text(ctx, 0, 0, 1);
    CHECK(help.box().shown_time == 10751);

    // Move to the next hotspot 49 ms later: it shows at once.
    mouse.x = 100;
    mouse.y = 60;
    help.set_clock(10800);
    help.compute_help_text(ctx, 0, 0, 1);
    CHECK(help.box().hotspot == 2);
    CHECK(help.box().shown_time == 10800);

    // A new hotspot more than 500 ms on does not show at once.
    mouse.x = 60;
    mouse.y = 60;
    help.set_clock(12000);
    help.compute_help_text(ctx, 0, 0, 1);
    CHECK(help.box().hotspot == 3);
    CHECK(help.box().shown_time == 10800);
}

// A tooltip for a hotspot with no callback is its help_text entry, wrapped;
// a callback hotspot falls back to the same text.
void test_help_tooltip_text(const char *dir)
{
    std::string error;
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    th::MenuBar bar;
    CHECK(bar.load(dir, gd.text(), &error));
    th::Font font;
    CHECK(th::load_font(dir, 0, font, &error));

    th::HelpSystem help;
    help.set_art(font, bar.sprites);
    th::UiMouse mouse{10, 10, false, false, false, false, false, false};
    th::HelpContext ctx = game_context(&gd.text(), help.box(), mouse);

    const int id = 80;
    help.set_force_help(1, uint16_t(id));
    help.compute_help_text(ctx, 0, 0, 1);
    std::string_view raw = gd.text().at(th::TextSection::Help, id);
    CHECK(!raw.empty());
    CHECK(help.box().msg == th::HelpSystem::copy_and_format(raw));
    CHECK(help.box().style == 1 && help.box().hotspot != 0);
}

// Every plain hotspot's text id exists in the language's Help section. A
// callback hotspot may carry a requester slot or another section's index
// rather than a Help entry (req10_help_room reads RoomHelp, req11 reads
// ObjectHelp, and some call it a slot), so it is not indexed as Help text.
void test_help_text_ids(const char *dir)
{
    std::string error;
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    size_t count = gd.text().size(th::TextSection::Help);
    CHECK(count > 0);
    for (int i = 0; i < th::kHiresHelpCount; i++) {
        const th::HelpHotspot &h = th::kHiresHelp[i];
        if (h.x0 < 0 || h.func)
            continue;
        CHECK(h.text() < count);
    }
}

// The box is clamped inside the screen.
void test_help_box_clamped(const char *dir)
{
    std::string error;
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    th::MenuBar bar;
    CHECK(bar.load(dir, gd.text(), &error));
    th::Font font;
    CHECK(th::load_font(dir, 0, font, &error));

    th::HelpSystem help;
    help.set_art(font, bar.sprites);
    help.set_tooltip(639, 470, gd.text().at(th::TextSection::Help, 80));
    const th::HelpBox &b = help.box();
    CHECK(b.x >= 0 && b.y >= 0);
    CHECK(b.x + b.w <= 640);
    CHECK(b.y + b.h <= 640); // the original clamps y with the width
}

// A room's help page opens, draws and closes.
void test_help_page(const char *dir)
{
    std::string error;
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    th::MenuBar bar;
    CHECK(bar.load(dir, gd.text(), &error));
    th::Font font;
    CHECK(th::load_font(dir, 0, font, &error));

    th::HelpSystem help;
    help.set_art(font, bar.sprites);
    help.setup_page_indices(gd.text());
    help.show_help_page(gd.text(), 1);
    CHECK(help.box().style == 2);
    CHECK(!help.box().msg.empty());

    th::Surface dst;
    dst.resize(640, 480);
    dst.fill(0);
    help.draw(dst, gd.palette());
    bool changed = false;
    for (int y = 0; y < dst.height() && !changed; y++)
        for (int x = 0; x < dst.width(); x++)
            if (dst.row(y)[x] != 0) {
                changed = true;
                break;
            }
    CHECK(changed);

    CHECK(help.finish_composite_help());
    CHECK(help.box().msg.empty());
    CHECK(!help.finish_composite_help());
}

// draw clears the tooltip after drawing it (the original's help_msg[0] = 0
// outside MM_HELP), so it does not stay once the mouse leaves.
void test_help_clears_after_draw(const char *dir)
{
    std::string error;
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    th::MenuBar bar;
    CHECK(bar.load(dir, gd.text(), &error));
    th::Font font;
    CHECK(th::load_font(dir, 0, font, &error));

    th::HelpSystem help;
    help.set_art(font, bar.sprites);
    th::UiMouse mouse{80, 40, false, false, false, false, false, false};
    th::HelpContext ctx = game_context(&gd.text(), help.box(), mouse);

    // Rest on the first hotspot of group 1 until its tooltip shows.
    help.set_clock(10000);
    help.compute_help_text(ctx, 0, 0, 1);
    help.set_clock(10751);
    help.compute_help_text(ctx, 0, 0, 1);
    CHECK(!help.box().msg.empty());

    th::Surface dst;
    dst.resize(640, 480);
    dst.fill(0);
    help.draw(dst, gd.palette(), true, false);
    CHECK(help.box().msg.empty()); // cleared after drawing

    // Off the hotspot: nothing is prepared, and drawing leaves the surface
    // empty.
    mouse.x = 10;
    mouse.y = 400;
    help.set_clock(20000);
    help.compute_help_text(ctx, 0, 0, 1);
    CHECK(help.box().hotspot == 0 && help.box().msg.empty());
    th::Surface dst2;
    dst2.resize(640, 480);
    dst2.fill(0);
    help.draw(dst2, gd.palette(), true, false);
    CHECK(surface_all_zero(dst2));
}

// The full-screen white is the palette's real white, not a dark grey (the
// palette is already 8-bit).
void test_help_white(const char *dir)
{
    std::string error;
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    th::MenuBar bar;
    CHECK(bar.load(dir, gd.text(), &error));
    th::Font font;
    CHECK(th::load_font(dir, 0, font, &error));

    th::HelpSystem help;
    help.set_art(font, bar.sprites);
    help.set_tooltip(320, 240, gd.text().at(th::TextSection::Help, 80));
    help.box().screen_pic_loaded = true;

    int expected = -1;
    for (int i = 0; i < 256; i++) {
        auto ch = [&](int k) { return int((gd.palette()[size_t(i)] >> (16 - 8 * k)) & 0xff) >> 2; };
        if (ch(0) == 0x3f && ch(1) == 0x3f && ch(2) == 0x3f) {
            expected = i;
            break;
        }
    }
    CHECK(expected >= 0);
    CHECK(expected >= 0 && int((gd.palette()[size_t(expected)] >> 16) & 0xff) > 200);

    th::Surface dst;
    dst.resize(640, 480);
    dst.fill(0);
    help.draw(dst, gd.palette(), true, false);
    const th::HelpBox &b = help.box();
    CHECK(b.w > 4 && b.h > 4 && b.x + b.w <= 640);
    CHECK(dst.row(b.y + 2)[b.x + b.w - 2] == uint8_t(expected));
}

// set_help_from_index wraps a callback's text through copy_and_format, as the
// original's set_help_text(1, ...) does.
void test_help_callback_wrapped(const char *dir)
{
    std::string error;
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    th::MenuBar bar;
    CHECK(bar.load(dir, gd.text(), &error));
    th::Font font;
    CHECK(th::load_font(dir, 0, font, &error));

    th::HelpSystem help;
    help.set_art(font, bar.sprites);
    th::UiMouse mouse{10, 10, false, false, false, false, false, false};
    th::HelpContext ctx = game_context(&gd.text(), help.box(), mouse);

    // req12's callbacks set their hotspot's plain entry unconditionally (the
    // demand requester is not ported).
    int idx = -1;
    for (int i = 0; i < th::kHiresHelpCount; i++)
        if (th::kHiresHelp[i].x0 >= 0 && th::kHiresHelp[i].func == &th::req12_help_if_doctor) {
            idx = i;
            break;
        }
    CHECK(idx >= 0);
    if (idx < 0)
        return;
    help.set_help_from_index(ctx, uint16_t(idx), 0, 0);
    std::string_view raw = gd.text().at(th::TextSection::Help, th::kHiresHelp[idx].text());
    CHECK(!raw.empty());
    CHECK(help.box().msg == th::HelpSystem::copy_and_format(raw));
}

// The requester groups: a handyman's window is 0xc, any other member of
// staff's is 10 (req4).
void test_help_requester_groups()
{
    CHECK(th::requester_help_group(th::Requester::YesNo) == 8);
    CHECK(th::requester_help_group(th::Requester::BuildRoom) == 2);
    CHECK(th::requester_help_group(th::Requester::HireStaff) == 4);
    CHECK(th::requester_help_group(th::Requester::BuyObjects) == 5);
    CHECK(th::requester_help_group(th::Requester::Patient) == 9);
    CHECK(th::requester_help_group(th::Requester::Staff) == 10);
    CHECK(th::requester_help_group(th::Requester::Handyman) == 0xc);
    CHECK(th::requester_help_group(th::Requester::EditRoom) == 0xd);
    CHECK(th::requester_help_group(th::Requester::Jukebox) == 0x14);
    CHECK(th::requester_help_group(th::Requester::None) == 0);
}

// The panel's group 3 and the front requester's group are both computed each
// frame (engine.cpp), the requester first; the panel's miss does not clear
// the requester's hotspot.
void test_help_panel_and_requester()
{
    th::HelpSystem help;
    th::UiMouse mouse;
    th::HelpContext ctx = game_context(nullptr, help.box(), mouse);

    uint16_t rs = help.help_start(2);
    mouse.x = (th::kHiresHelp[rs].x0 + th::kHiresHelp[rs].x1) / 2;
    mouse.y = (th::kHiresHelp[rs].y0 + th::kHiresHelp[rs].y1) / 2;
    help.set_clock(10000);
    help.compute_help_text(ctx, 0, 0, 2);
    help.set_clock(10751);
    help.compute_help_text(ctx, 0, 0, 2);
    CHECK(help.box().hotspot == rs && help.box().group == 2);

    // The panel is not under the mouse: its group 3 call leaves the
    // requester's hotspot alone.
    help.compute_help_text(ctx, 0, 0, 3);
    CHECK(help.box().hotspot == rs && help.box().group == 2);

    // Over a panel hotspot the requester's miss clears, then the panel sets.
    uint16_t ps = help.help_start(3);
    mouse.x = (th::kHiresHelp[ps].x0 + th::kHiresHelp[ps].x1) / 2;
    mouse.y = (th::kHiresHelp[ps].y0 + th::kHiresHelp[ps].y1) / 2;
    help.compute_help_text(ctx, 0, 0, 2);
    help.set_clock(20751);
    help.compute_help_text(ctx, 0, 0, 3);
    CHECK(help.box().hotspot == ps && help.box().group == 3);
}

// draw_help_text's mouse-mode gate: no tooltip while placing or building,
// unless the advisor's tutorial flag allows it.
void test_help_draw_gate(const char *dir)
{
    std::string error;
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    th::MenuBar bar;
    CHECK(bar.load(dir, gd.text(), &error));
    th::Font font;
    CHECK(th::load_font(dir, 0, font, &error));

    th::HelpSystem help;
    help.set_art(font, bar.sprites);
    th::UiMouse mouse{10, 10, false, false, false, false, false, false};
    th::HelpContext ctx = game_context(&gd.text(), help.box(), mouse);
    help.set_force_help(1, 80);
    help.compute_help_text(ctx, 0, 0, 1);
    CHECK(!help.box().msg.empty());

    th::Surface dst;
    dst.resize(640, 480);
    dst.fill(0);
    help.draw(dst, gd.palette(), false, false); // not allowed
    CHECK(surface_all_zero(dst));
    CHECK(!help.box().msg.empty());

    help.draw(dst, gd.palette(), false, true); // tutorial overrides
    CHECK(!surface_all_zero(dst));
    CHECK(help.box().msg.empty());
}

// In the main menu the box is measured with font cache 0x2d.
void test_help_main_menu_font(const char *dir)
{
    std::string error;
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    th::MenuBar bar;
    CHECK(bar.load(dir, gd.text(), &error));
    th::Font font, menu_font;
    CHECK(th::load_font(dir, 0, font, &error));
    CHECK(th::load_font(dir, 0x2d, menu_font, &error));

    std::string_view text = gd.text().at(th::TextSection::Help, 80);
    th::HelpSystem help;
    help.set_art(font, bar.sprites);
    help.set_tooltip(320, 240, text);
    int w0 = help.box().w;

    help.set_main_menu_font(menu_font);
    help.set_main_menu(true);
    help.set_tooltip(320, 240, text);
    int w1 = help.box().w;
    if (th::text_width(font, text) != th::text_width(menu_font, text))
        CHECK(w0 != w1);

    help.set_main_menu(false);
    help.set_tooltip(320, 240, text);
    CHECK(help.box().w == w0);
}

// The framed page's shadow boxes are blended through the ghost table (draw
// flag 4), not drawn solid.
void test_help_page_shadow(const char *dir)
{
    std::string error;
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    th::MenuBar bar;
    CHECK(bar.load(dir, gd.text(), &error));
    th::Font font;
    CHECK(th::load_font(dir, 0, font, &error));

    th::HelpSystem help;
    help.set_art(font, bar.sprites);
    std::vector<uint8_t> ghost(0x10000, 7);
    help.set_ghost(ghost.data());
    help.setup_page_indices(gd.text());
    help.show_help_page(gd.text(), 1);

    th::Surface dst;
    dst.resize(640, 480);
    dst.fill(0);
    help.draw(dst, gd.palette());
    const th::HelpBox &b = help.box();
    int x = b.x + b.w + 2, y = b.y + b.h / 2;
    CHECK(x < 640 && y < 480);
    CHECK(dst.row(y)[x] == 7);

    // The page's shadow is the two strips to its right and below.
    std::array<th::Rect, 2> rects = help.page_shadow_rects();
    CHECK(rects[0].x == b.x + b.w && rects[0].y == b.y + 0xc && rects[0].w == 0xc && rects[0].h == b.h);
    CHECK(rects[1].x == b.x + 0xc && rects[1].y == b.y + b.h && rects[1].w == b.w - 0xc && rects[1].h == 0xc);

    // The interface layer is transparent, so its shadow has no backdrop to
    // blend into; it is drawn as black at the ghost table's opacity.
    th::UiLayer layer;
    layer.resize(640, 480);
    layer.clear();
    layer.shadow(rects[0], th::HelpSystem::kPageShadowAlpha);
    std::vector<uint32_t> argb = layer.to_argb(gd.palette());
    uint32_t px = argb[size_t(rects[0].y + 1) * 640 + size_t(rects[0].x + 1)];
    CHECK((px >> 24) == th::HelpSystem::kPageShadowAlpha);
}
