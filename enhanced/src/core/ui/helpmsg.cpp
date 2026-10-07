#include "core/ui/helpmsg.h"

#include "core/data/text.h"
#include "core/render/blit.h"
#include "core/ui/layer.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace th
{
namespace
{

// help_hotspot_detected's timing: a tooltip appears once the mouse has rested
// on its hotspot this long, and shows at once when another tooltip was shown
// within the shorter time before.
constexpr uint32_t kHelpRestTicks = 0x2ee;
constexpr uint32_t kHelpReShowTicks = 0x1f4;
// The tooltip's border and the framed page's frame margin.
constexpr int kHelpBorder = 3;
constexpr int kPageMargin = 0xc;

// LbDrawBoxClip: fills r, clipped to dst.
void fill_box(Surface &dst, Rect r, uint8_t colour)
{
    int x0 = std::max(r.x, 0), y0 = std::max(r.y, 0);
    int x1 = std::min(r.x + r.w, dst.width()), y1 = std::min(r.y + r.h, dst.height());
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++)
            dst.row(y)[x] = colour;
}

// LbDrawBox with draw flag 0x10: the outline of r in colour.
void outline_box(Surface &dst, Rect r, uint8_t colour)
{
    if (r.w < 1 || r.h < 1)
        return;
    fill_box(dst, {r.x, r.y, r.w, 1}, colour);
    fill_box(dst, {r.x, r.y + r.h - 1, r.w, 1}, colour);
    if (r.h > 2) {
        fill_box(dst, {r.x, r.y + 1, 1, r.h - 2}, colour);
        fill_box(dst, {r.x + r.w - 1, r.y + 1, 1, r.h - 2}, colour);
    }
}

// LbPaletteFindColour: the nearest colour by squared distance, ties broken by
// the first (the darkest). The port's Palette is 8-bit (palette_from_vga
// scales the 6-bit values up), so both sides are compared in 6 bits, as the
// original's does on its 6-bit palette.
uint8_t find_colour(const Palette &palette, int r, int g, int b)
{
    auto at = [&](int i, int k) { return int(((palette[size_t(i)] >> (16 - 8 * k)) & 0xff) >> 2); };
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

// Blends colour into r through `ghost` (draw flag 4), as draw_b_line does;
// solid without a ghost table.
void fill_box_ghost(Surface &dst, Rect r, uint8_t colour, const uint8_t *ghost)
{
    int x0 = std::max(r.x, 0), y0 = std::max(r.y, 0);
    int x1 = std::min(r.x + r.w, dst.width()), y1 = std::min(r.y + r.h, dst.height());
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++)
            dst.row(y)[x] = ghost ? ghost[(colour << 8) | dst.row(y)[x]] : colour;
}

// The section of `strs` from `start` up to its terminating "." string.
std::vector<std::string_view> page_section(const LanguageText &text, uint16_t start)
{
    std::vector<std::string_view> strs;
    for (size_t i = start; i < text.size(TextSection::ShowHelp0); i++) {
        std::string_view s = text.at(TextSection::ShowHelp0, int(i));
        strs.push_back(s);
        if (s == ".")
            break;
    }
    return strs;
}

// copy_and_format_composite's running width limit, from its longest paragraph.
int composite_limit(const std::vector<std::string_view> &strs, int screen_width, const Font &font)
{
    int most = 0, count = 0;
    for (size_t si = 0; si < strs.size() && !(strs[si].size() == 1 && strs[si][0] == '.'); si++) {
        std::string_view sp = strs[si];
        for (size_t p = 0; p < sp.size(); p++) {
            if (sp.substr(p) == "//") {
                if (most < count)
                    most = count;
                count = 0;
                p++;
            } else {
                count++;
            }
        }
    }
    if (most < count)
        most = count;
    int limit = 0xc;
    for (int k = 1; limit * k < most; limit += 4, k++)
        ;
    limit = limit * font.glyph(77).width;
    return std::min(limit, screen_width * 3 / 4);
}

// Breaks `out` at its last space before `cur` exceeds `limit`.
void wrap_composite_line(std::string &out, int &cur, int limit, const Font &font)
{
    if (cur <= limit)
        return;
    for (size_t q = out.size(); q-- > 0;) {
        if (out[q] == '\n')
            break;
        if (out[q] == ' ') {
            out[q] = '\n';
            cur = text_width(font, std::string_view(out).substr(q + 1));
            break;
        }
    }
}

} // namespace

uint8_t requester_help_group(Requester requester)
{
    switch (requester) {
    case Requester::YesNo:
        return 8; // req13
    case Requester::BuildRoom:
        return 2; // req10
    case Requester::HireStaff:
        return 4; // req2
    case Requester::BuyObjects:
        return 5; // req11
    case Requester::Patient:
        return 9; // req3
    case Requester::Staff:
        return 10; // req4, any member of staff but a handyman
    case Requester::Handyman:
        return 0xc; // req4, a handyman
    case Requester::EditRoom:
        return 0xd; // req14
    case Requester::Queue:
        return 7; // req7
    case Requester::Jukebox:
        return 0x14; // req6
    default:
        return 0;
    }
}

HelpSystem::HelpSystem()
{
    setup_help_indices();
}

void HelpSystem::setup_help_indices()
{
    uint16_t i = 0;
    uint8_t group = 0;
    bool done = false;
    while (!done && i < kHiresHelpCount) {
        while (i < kHiresHelpCount && kHiresHelp[i].x0 >= 0)
            i++;
        if (i >= kHiresHelpCount || kHiresHelp[i].x0 == -2)
            done = true;
        i++;
        group++;
        if (group < help_start_.size())
            help_start_[group] = i;
    }
}

void HelpSystem::setup_page_indices(const LanguageText &text)
{
    page0_start_.fill(0);
    size_t i = 0;
    size_t count = 0;
    while (i < text.size(TextSection::ShowHelp0) && text.at(TextSection::ShowHelp0, int(i)) != ".." &&
           count < page0_start_.size()) {
        page0_start_[count] = uint16_t(i);
        while (i < text.size(TextSection::ShowHelp0) && text.at(TextSection::ShowHelp0, int(i)) != ".")
            i++;
        i++;
        count++;
    }
}

void HelpSystem::set_art(const Font &font, const SpriteSheet &pulldv)
{
    font_ = &font;
    pulldv_ = &pulldv;
}

std::string_view HelpSystem::help_text(const HelpContext &ctx, uint16_t id)
{
    if (!ctx.text)
        return {};
    return ctx.text->at(TextSection::Help, int(id));
}

void HelpSystem::compute_help_text(HelpContext &ctx, int x, int y, uint8_t group)
{
    // No tooltip while a framed help page is open (the game's MM_HELP).
    if (box_.style == 2 && !box_.msg.empty())
        return;
    if (box_.forced_hotspot) {
        if (group == box_.forced_group)
            set_help_from_index(ctx, box_.forced_hotspot, x, y);
        return;
    }
    if (!ctx.mouse)
        return;
    int dx = ctx.mouse->x - x;
    int dy = ctx.mouse->y - y;
    for (uint16_t i = help_start(group); i < kHiresHelpCount && kHiresHelp[i].x0 >= 0; i++) {
        const HelpHotspot &h = kHiresHelp[i];
        if (dx >= h.x0 && dx <= h.x1 && dy >= h.y0 && dy <= h.y1) {
            help_hotspot_detected(ctx, i, group, x, y);
            return;
        }
    }
    if (group == box_.group)
        box_.hotspot = 0;
}

void HelpSystem::help_hotspot_detected(HelpContext &ctx, uint16_t hotspot, uint8_t group, int x, int y)
{
    if (hotspot == box_.hotspot) {
        if (box_.hover_start + kHelpRestTicks < box_.clock_ms)
            box_.shown_time = box_.clock_ms;
    } else {
        box_.group = group;
        box_.hotspot = hotspot;
        box_.hover_start = box_.clock_ms;
    }
    if (box_.shown_time + kHelpReShowTicks > box_.clock_ms) {
        box_.shown_time = box_.clock_ms;
        box_.group = group;
        set_help_from_index(ctx, hotspot, x, y);
    }
}

void HelpSystem::set_help_from_index(HelpContext &ctx, uint16_t hotspot, int x, int y)
{
    if (hotspot >= kHiresHelpCount)
        return;
    const HelpHotspot &h = kHiresHelp[hotspot];
    int cx = x + (h.x0 + h.x1) / 2;
    int cy = y + h.y0;
    if (h.func) {
        box_.msg.clear();
        ctx.hotspot = &h;
        h.func(ctx);
        // set_help_text(1, ...) wraps the callback's text too.
        std::string raw = box_.msg;
        set_tooltip(cx, cy, raw);
    } else {
        set_tooltip(cx, cy, help_text(ctx, h.text()));
    }
    box_.hotspot = hotspot;
}

void HelpSystem::set_force_help(uint8_t group, uint16_t id)
{
    box_.forced_group = group;
    box_.forced_hotspot = 0;
    for (uint16_t i = 0; i < kHiresHelpCount && kHiresHelp[i].x0 != -2; i++)
        if (kHiresHelp[i].x0 >= 0 && kHiresHelp[i].text() == id) {
            box_.forced_hotspot = i;
            return;
        }
}

void HelpSystem::clear_force_help()
{
    box_.forced_hotspot = 0;
}

void HelpSystem::cancel()
{
    box_.hotspot = 0;
    box_.shown_time = 0;
    box_.msg.clear();
}

void HelpSystem::clamp_to_screen(int screen_w)
{
    if (box_.x < 0)
        box_.x = 0;
    if (box_.y < 0)
        box_.y = 0;
    // The original clamps both limits with the screen's width, not its height.
    if (box_.x > screen_w - box_.w)
        box_.x = screen_w - box_.w;
    if (box_.y > screen_w - box_.h)
        box_.y = screen_w - box_.h;
}

void HelpSystem::place_tooltip(int cx, int cy)
{
    box_.style = 1;
    if (!font_)
        return;
    box_.w = text_width(active_font(), box_.msg) + kHelpBorder * 2;
    box_.h = text_height(active_font(), box_.msg) + kHelpBorder * 2;
    box_.x = cx - kHelpBorder;
    box_.y = cy - kHelpBorder - box_.h + 6;
    clamp_to_screen(640);
}

void HelpSystem::set_tooltip(int x, int y, std::string_view text)
{
    box_.msg = copy_and_format(text);
    place_tooltip(x, y);
}

void HelpSystem::set_page(int x, int y, const std::vector<std::string_view> &strs)
{
    if (!font_)
        return;
    box_.style = 2;
    box_.msg = copy_and_format_composite(strs, 640, active_font());
    int border = (pulldv_ ? pulldv_->at(12).height : 0) + kPageMargin;
    box_.w = text_width(active_font(), box_.msg) + border * 2;
    box_.h = text_height(active_font(), box_.msg) + border * 2 + (pulldv_ ? pulldv_->at(19).height : 0);
    if (x == -1 && y == -1) {
        box_.x = (640 - box_.w) / 2;
        box_.y = (480 - box_.h) / 2;
    } else {
        box_.x = x - border;
        box_.y = y - border - box_.h + 6;
    }
    clamp_to_screen(640);
    box_.close_hover = false;
}

void HelpSystem::show_help_page(const LanguageText &text, int sub)
{
    if (sub < 0 || sub >= int(page0_start_.size()))
        return;
    set_page(-1, -1, page_section(text, page0_start_[sub]));
    box_.page = 0;
    box_.sub = uint8_t(sub);
}

bool HelpSystem::finish_composite_help()
{
    bool was_page = box_.style == 2 && !box_.msg.empty();
    box_.msg.clear();
    box_.close_hover = false;
    box_.page = 0;
    box_.sub = 0;
    return was_page;
}

std::string HelpSystem::copy_and_format(std::string_view src)
{
    int len = int(src.size());
    int col = 0;
    int width = 0xc;
    for (int k = 1; width * k < len; width += 4, k++)
        ;
    std::string out;
    for (char ch : src) {
        if (ch == ' ' && col >= width) {
            out += '\n';
            col = 0;
        } else {
            out += ch;
            col++;
        }
    }
    return out;
}

std::string HelpSystem::copy_and_format_composite(const std::vector<std::string_view> &strs, int screen_width,
                                                  const Font &font)
{
    int limit = composite_limit(strs, screen_width, font);
    std::string out;
    int cur = 0;
    for (size_t si = 0; si < strs.size() && strs[si] != "."; si++) {
        std::string_view sp = strs[si];
        for (size_t p = 0; p < sp.size(); p++) {
            wrap_composite_line(out, cur, limit, font);
            if (sp.substr(p) == "//") {
                out += "\n\n";
                p++;
                cur = 0;
            } else {
                out += sp[p];
                cur += font.glyph(uint8_t(sp[p])).width;
            }
        }
        if (!sp.empty()) {
            char last = sp.back();
            if (last == '.' || last == '!' || last == '?' || last == ',') {
                out += ' ';
                cur += font.space_width();
            }
        }
    }
    return out;
}

bool HelpSystem::update_close_hover(int mouse_x, int mouse_y)
{
    if (box_.style != 2 || !pulldv_)
        return false;
    const Sprite &close = pulldv_->at(19);
    bool over = mouse_x >= box_.close_x && mouse_x < box_.close_x + close.width && mouse_y >= box_.close_y &&
                mouse_y < box_.close_y + close.height;
    bool was = box_.close_hover;
    box_.close_hover = over;
    return over && !was;
}

std::array<Rect, 2> HelpSystem::page_shadow_rects() const
{
    if (box_.style != 2)
        return {};
    return {Rect{box_.x + box_.w, box_.y + kPageMargin, kPageMargin, box_.h},
            Rect{box_.x + kPageMargin, box_.y + box_.h, box_.w - kPageMargin, kPageMargin}};
}

void HelpSystem::draw_page_frame(Surface &dst, uint8_t black)
{
    const SpriteSheet &s = *pulldv_;
    int step = s.at(12).width;
    int j = box_.y;
    int bottom = box_.y + box_.h - s.at(16).height;
    for (int i = box_.x; box_.x + box_.w - step > i; i += step) {
        draw_sprite(dst, i, j, s.at(12));
        draw_sprite(dst, i, bottom, s.at(16));
    }
    step = s.at(18).height;
    int i = box_.x;
    int right = box_.x + box_.w - s.at(14).width;
    for (j = box_.y; box_.y + box_.h - step > j; j += step) {
        draw_sprite(dst, i, j, s.at(18));
        draw_sprite(dst, right, j, s.at(14));
    }
    draw_sprite(dst, box_.x, box_.y, s.at(11));
    draw_sprite(dst, box_.x + box_.w - s.at(13).width, box_.y, s.at(13));
    draw_sprite(dst, box_.x, box_.y + box_.h - s.at(17).height, s.at(17));
    draw_sprite(dst, box_.x + box_.w - s.at(15).width, box_.y + box_.h - s.at(15).height, s.at(15));
    box_.close_x = box_.x + box_.w - kPageMargin - s.at(19).width;
    box_.close_y = box_.y + box_.h - kPageMargin - s.at(19).height;
    draw_sprite(dst, box_.close_x, box_.close_y, box_.close_hover ? s.at(20) : s.at(19));
    fill_box_ghost(dst, {box_.x + box_.w, box_.y + kPageMargin, kPageMargin, box_.h}, black, ghost_);
    fill_box_ghost(dst, {box_.x + kPageMargin, box_.y + box_.h, box_.w - kPageMargin, kPageMargin}, black, ghost_);
}

void HelpSystem::draw(Surface &dst, const Palette &palette, bool allowed, bool tutorial)
{
    // draw_help_text: nothing without text, or outside the mouse modes that
    // allow help (the advisor's tutorial overrides).
    if (box_.msg.empty() || !font_ || !(allowed || tutorial))
        return;
    uint8_t black = find_colour(palette, 0, 0, 0);
    uint8_t fill = box_.screen_pic_loaded ? find_colour(palette, 0x3f, 0x3f, 0x3f) : 0xab;
    Rect r{box_.x, box_.y, box_.w, box_.h};
    fill_box(dst, r, fill);
    int border = kHelpBorder;
    if (box_.style == 1) {
        outline_box(dst, r, black);
    } else if (box_.style == 2 && pulldv_) {
        draw_page_frame(dst, black);
        border = pulldv_->at(12).height + kPageMargin;
    }
    TextSettings settings;
    settings.window = {0, 0, dst.width()};
    settings.clip = {0, 0, dst.width(), dst.height()};
    settings.flags = kTextOneColour;
    settings.colour = black;
    drawn_ = box_.msg;
    draw_text(dst, active_font(), box_.x + border, box_.y + border, box_.msg, settings);
    // The original clears the tooltip after drawing it, unless a help page is
    // up (MM_HELP); the page is closed by finish_composite_help.
    if (box_.style != 2)
        box_.msg.clear();
}

} // namespace th
