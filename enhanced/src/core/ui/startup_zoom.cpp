// The glass boxes of Startup's screens: the zoom in and out of the panels
// and buttons and their drawing (src/glass.cpp).

#include "core/ui/startup_internal.h"

#include "core/render/blit.h"
#include "core/render/text.h"

#include <algorithm>

namespace th
{

namespace
{

// calculate_grey's bevel greys (Gray[2..7]).
constexpr std::array<uint8_t, 8> kGray = {0, 0xff, 0xf5, 0xf5, 0xf5, 0xf8, 0xf8, 0xf6};
} // namespace

void Startup::init_zoom_boxes(int mask)
{
    current_display_ = desired_display_ = 0;
    zoom_ = {};
    for (int bit : {1, 2, 0x2000}) {
        if (!(mask & bit))
            continue;
        // add_glass_frame and add_glass_box.
        std::span<const GlassBox> frames = bit == 1   ? std::span<const GlassBox>(kPrefsGlass)
                                           : bit == 2 ? std::span<const GlassBox>(kNameGlass)
                                                      : std::span<const GlassBox>(kLoadGlass);
        for (const GlassBox &g : frames) {
            auto slot = std::find_if(zoom_.begin(), zoom_.end(), [](const ZoomBox &z) { return !z.active; });
            if (slot == zoom_.end())
                break;
            ZoomBox &z = *slot;
            z.active = true;
            if (g.x == z.to[0] && g.y == z.to[1] && g.x1 - g.x + 1 == z.to[2] && g.y1 - g.y + 1 == z.to[3])
                continue;
            z.step = 0;
            z.steps = g.steps;
            z.id = bit;
            z.type = g.type;
            z.button = g.button;
            z.open_delay = z.close_delay = g.delay;
            switch (g.origin) {
            case 1:
                z.from = {g.x + (g.x1 - g.x) / 2, g.y, 0, g.y1 - g.y + 1};
                break;
            case 0:
                z.from = {g.x + (g.x1 - g.x) / 2, g.y + (g.y1 - g.y) / 2, 0, 0};
                break;
            case 2:
                z.from = {kUiWidth / 2, kUiHeight / 2, 0, 0};
                break;
            }
            z.to = {g.x, g.y, g.x1 - g.x + 1, g.y1 - g.y + 1};
        }
        desired_display_ |= bit;
    }
}

void Startup::zoom_levels(int &lo, int &hi, bool &incomplete) const
{
    lo = -128, hi = -128;
    incomplete = false;
    bool found = false;
    for (const ZoomBox &z : zoom_) {
        if (!z.active)
            continue;
        if ((z.step > 0 && z.step < z.steps - 1) || (z.step && !(desired_display_ & z.id))) {
            found = true;
            hi = std::max(hi, z.type);
        }
        if (z.step == 0 && !found) {
            hi = z.type;
            found = true;
        }
        if (z.step > 0 && z.step <= z.steps - 1)
            lo = std::max(lo, z.type);
        if (!(desired_display_ & z.id))
            incomplete = true;
    }
    if (hi == -128)
        hi = lo;
    else if (hi > lo)
        lo = hi;
}

bool Startup::open_zoom_box(ZoomBox &z, int hi, bool incomplete)
{
    if (z.id & current_display_) {
        draw_box(z);
        return false;
    }
    if (incomplete || z.type > hi)
        return false;
    bool opening = false;
    if (z.type == hi) {
        if (z.open_delay) {
            z.open_delay--;
        } else if (++z.step < z.steps - 1) {
            opening = true;
        } else {
            bool all = std::none_of(zoom_.begin(), zoom_.end(), [&](const ZoomBox &o) {
                return o.active && &o != &z && o.id == z.id && o.step != o.steps - 1;
            });
            if (all)
                current_display_ |= z.id;
            if (z.step > z.steps - 1)
                z.step--;
        }
    }
    draw_box(z);
    return opening;
}

bool Startup::close_zoom_box(ZoomBox &z, int hi)
{
    bool closing = false;
    if (z.type == hi) {
        if (!z.close_delay) {
            current_display_ &= ~z.id;
            closing = true;
            if (--z.step <= 0)
                z.active = false;
        } else {
            z.close_delay--;
        }
    }
    draw_box(z);
    return closing;
}

bool Startup::draw_zoom_boxes()
{
    int lo = 0, hi = 0;
    bool incomplete = false;
    zoom_levels(lo, hi, incomplete);
    bool opening = false, closing = false;
    for (ZoomBox &z : zoom_) {
        if (!z.active || z.type > lo)
            continue;
        if (z.id & desired_display_)
            opening |= open_zoom_box(z, hi, incomplete);
        else
            closing |= close_zoom_box(z, hi);
    }
    if (opening && !zoom_open_sound_) {
        zoom_open_sound_ = true;
        sound(0x9c); // SFX_ZOOM_OPEN
    } else if (!opening) {
        zoom_open_sound_ = false;
    }
    if (closing && !zoom_close_sound_) {
        zoom_close_sound_ = true;
        sound(0x9e); // SFX_ZOOM_CLOSE
    } else if (!closing) {
        zoom_close_sound_ = false;
    }
    return opening || closing;
}

void Startup::draw_box(const ZoomBox &z)
{
    // draw_a_box and the box drawers of src/glass.cpp.
    int total = z.steps - 1;
    int s = z.step;
    if (s == 0 || total <= 0)
        return;
    int x = int16_t((z.to[0] * s + (total - s) * z.from[0]) / total);
    int y = int16_t(((total - s) * z.from[1] + z.to[1] * s) / total);
    int w = int16_t(((total - s) * z.from[2] + z.to[2] * s) / total);
    int h = int16_t(((total - s) * z.from[3] + z.to[3] * s) / total);

    auto fill = [&](int bx, int by, int bw, int bh, uint8_t colour, bool ghost) {
        fill_box({bx, by, bw, bh}, colour, ghost);
    };
    auto outline = [&](int bx, int by, int bw, int bh, uint8_t colour) { outline_box({bx, by, bw, bh}, colour); };
    const BoxColours &c = colours_;
    // A single difficulty hides the arrows (and every button but type 2's).
    if (z.button) {
        if (!(z.type != 2 && balance_files_.size() == 1)) {
            fill(x, y, w, h, c.white, false);
            outline(x, y, w, h, 0xf8);
            draw_button_edges(x, y, w - 1, h - 1, false);
        }
        return;
    }
    switch (z.type) {
    case -1:
    case 0:
        fill(x, y, w, h, c.white, z.type == 0);
        outline(x, y, w, h, c.black);
        for (int i = 1; i <= 3; i++)
            outline(x + i, y + i, w - 2 * i, h - 2 * i, z.type == 0 ? c.blue_grey : 0xca);
        break;
    case 1:
        fill(x, y, w, h, c.white, false);
        outline(x, y, w, h, 0xf8);
        for (int i = 1; i <= 3; i++)
            outline(x + i, y + i, w - 2 * i, h - 2 * i, c.blue_grey);
        break;
    default:
        fill(x, y, w, h, c.white, false);
        outline(x, y, w, h, c.blue_grey);
        outline(x + 1, y + 1, w - 2, h - 2, c.blue_grey);
        break;
    }
}

void Startup::fill_box(Rect box, uint8_t colour, bool ghost)
{
    // LbDrawBox filled: see-through through the ghost table (the colour as
    // the row, as sprites are) or solid.
    for (int yy = std::max(box.y, 0); yy < std::min(box.y + box.h, surface_.height()); yy++) {
        uint8_t *row = surface_.row(yy);
        for (int xx = std::max(box.x, 0); xx < std::min(box.x + box.w, surface_.width()); xx++)
            row[xx] = ghost ? prefs_ghost_[size_t(colour) * 256 + row[xx]] : colour;
    }
}

void Startup::outline_box(Rect box, uint8_t colour)
{
    // LbDrawBox outlined.
    const int bx = box.x, by = box.y, bw = box.w, bh = box.h;
    if (bw < 1 || bh < 1)
        return;
    fill_box({bx, by, bw, 1}, colour, false);
    fill_box({bx, by + bh - 1, bw, 1}, colour, false);
    if (bh > 2) {
        fill_box({bx, by + 1, 1, bh - 2}, colour, false);
        fill_box({bx + bw - 1, by + 1, 1, bh - 2}, colour, false);
    }
}

void Startup::draw_glass_text(const UiMouse &mouse, int button, std::string_view text, bool highlight)
{
    // draw_glass_text and draw_glass_text_hilight: centred on the button,
    // a pixel down and right in the highlighted font while held or chosen.
    const ScreenButton &b = kPrefsButtons[size_t(button)];
    int w = b.x1 - b.x + 1, h = b.y1 - b.y + 1;
    bool held = (mouse.left_held || mouse.left_released) && over(mouse.x, mouse.y, b.x, b.y, w, h);
    bool down = highlight || held;
    const Font &font = fonts_[down ? kFontGlassHi : kFontGlass];
    int x = (w - text_width(font, text)) >> 1;
    int y = (b.y1 + b.y + 1 - font.glyph('A').height) >> 1;
    if (down) {
        x++;
        y++;
    }
    draw_text(surface_, font, x, y, text, {{b.x, 0, w}, {0, 0, surface_.width(), surface_.height()}});
}

void Startup::draw_button_edges(int x, int y, int w, int h, bool sunken)
{
    // draw_button_edges: three lines a side, light above and left when
    // raised, below and right when sunken.
    auto line = [&](int x1, int y1, int x2, int y2, uint8_t colour) {
        for (int yy = std::min(y1, y2); yy <= std::max(y1, y2); yy++) {
            for (int xx = std::min(x1, x2); xx <= std::max(x1, x2); xx++) {
                if (xx >= 0 && yy >= 0 && xx < surface_.width() && yy < surface_.height())
                    surface_.row(yy)[xx] = colour;
            }
        }
    };
    int a = sunken ? 5 : 2, b = sunken ? 2 : 5;
    for (int i = 0; i < 3; i++) {
        uint8_t top = kGray[size_t(a + i)], bottom = kGray[size_t(b + i)];
        line(x + 1 + i, y + 1 + i, x + w - 1 - i, y + 1 + i, top);
        line(x + 1 + i, y + 2 + i, x + 1 + i, y + h - 1 - i, top);
        line(x + 2 + i, y + h - 1 - i, x + w - 1 - i, y + h - 1 - i, bottom);
        line(x + w - 1 - i, y + 2 + i, x + w - 1 - i, y + h - 2 - i, bottom);
    }
}

} // namespace th
