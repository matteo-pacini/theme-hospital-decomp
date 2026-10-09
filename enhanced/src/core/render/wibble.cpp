#include "core/render/wibble.h"

#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/view.h"

#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <numbers>

namespace th
{

namespace
{

// pixBuf's size, where draw_wibble_frame puts a frame's origin in it, and
// init_wibble's arguments (src/start.cpp): the wobble's period in table
// entries and its amplitude in pixels.
constexpr int kBufSize = 0xb4;
constexpr int kOriginX = 0x3a;
constexpr int kOriginY = 0x74;
constexpr int kTableSize = kBufSize + 0x20;
constexpr int kPeriod = 0x11;
constexpr int kAmplitude = 2;

// lbSinTable (src/lib/bf1/gtrig.cpp): sines of 2048ths of a turn, 16.16
// truncated.
int32_t sin_table(int i)
{
    return int32_t(std::sin(double(i) * 2 * std::numbers::pi / 2048) * 65536);
}

// init_wibble's tables: the rows' offsets and the columns' steps.
struct WibbleTables {
    std::array<int32_t, kTableSize> yoff;
    std::array<int32_t, kTableSize> xoff;
};

WibbleTables make_tables()
{
    WibbleTables t{};
    const int step = 0x800 / kPeriod;
    int angle = 0;
    for (size_t n = 0; n < kTableSize; n++, angle = (angle + step) & 0x7ff)
        t.yoff[n] = kAmplitude + (sin_table(angle) * kAmplitude >> 16);
    angle = 0;
    for (size_t m = 0; m < kTableSize; m++, angle = (angle + step) & 0x7ff)
        t.xoff[m] = sin_table(angle) * kAmplitude >> 16;
    for (size_t m = 0; m + 1 < kTableSize; m++)
        t.xoff[m] = kBufSize * (t.xoff[m + 1] - t.xoff[m]) + 1;
    return t;
}

const WibbleTables &tables()
{
    static const WibbleTables t = make_tables();
    return t;
}

// The buffer's pixel at `index` (row * kBufSize + column), kMaskColour
// outside it.
uint8_t buffer_pixel(const Surface &buffer, ptrdiff_t index)
{
    if (index < 0 || index >= ptrdiff_t(kBufSize) * kBufSize)
        return kMaskColour;
    return buffer.data()[index];
}

// What draw_wibble draws: the w x h image whose top-left is (bx, by) in the
// buffer, onto the surface at (x, y), shifted by the tables from `phase`.
struct WibbleBlit {
    int x, y, bx, by, w, h;
    size_t phase;
};

// draw_wibble (src/wibble.cpp), clipped to the clip rectangle of `dst`
// (the game's graphics window).
void draw_wibble(Surface &dst, const Surface &buffer, WibbleBlit b)
{
    const WibbleTables &t = tables();
    const Rect win = dst.clip();
    int skip_x = 0, skip_y = 0;
    if (b.x < win.x) {
        skip_x = win.x - b.x;
        b.w -= skip_x;
        b.x = win.x;
    }
    if (b.y < win.y) {
        skip_y = win.y - b.y;
        b.h -= skip_y;
        b.y = win.y;
    }
    b.w = std::min(b.w, win.x + win.w - b.x);
    b.h = std::min(b.h, win.y + win.h - b.y);
    if (b.w <= 0 || b.h <= 0)
        return;
    const ptrdiff_t first =
        ptrdiff_t(b.by + skip_y) * kBufSize + b.bx + ptrdiff_t(t.yoff[b.phase] - kAmplitude) * kBufSize;
    for (int row = 0; row < b.h; row++) {
        ptrdiff_t sample = first + ptrdiff_t(row) * kBufSize - kAmplitude + t.yoff[b.phase + size_t(skip_y + row)];
        size_t xp = b.phase;
        for (int k = 0; k < skip_x; k++)
            sample += t.xoff[xp++];
        uint8_t *out = dst.row(b.y + row) + b.x;
        for (int k = 0; k < b.w; k++) {
            uint8_t c = buffer_pixel(buffer, sample);
            if (c != kMaskColour)
                out[k] = c;
            sample += t.xoff[xp++];
        }
    }
}

// FillBoxInPixBuffer: `box` of the buffer back to kMaskColour.
void clear_buffer(Surface &buffer, Rect box)
{
    for (int y = std::max(box.y, 0); y < std::min(box.y + box.h, kBufSize); y++)
        for (int x = std::max(box.x, 0); x < std::min(box.x + box.w, kBufSize); x++)
            buffer.row(y)[x] = kMaskColour;
}

// What draw_wibble_frame keeps while it goes over a frame's elements.
struct WibbleDrawing {
    Surface &dst;
    Surface &buffer;
    Point at;
    bool flip;
    const GameData &data;
    const AnimFrame &frame;
    size_t phase;
    // The box of the elements drawn so far, relative to `at` (the game's
    // frame_bounds), 999 until one is.
    int left = 999, top = 999;
    // Elements wait in the buffer to wobble onto the screen.
    bool drawn = false;

    // draw_wibble of what the buffer holds, over the frame's box.
    void wobble()
    {
        const int w = std::min(frame.width + 8, 0x7a - left + 3);
        const int h = std::min(frame.height + 8, 0x40 - top + 3);
        draw_wibble(dst, buffer,
                    {at.x + left - 4, at.y + top - 4, kOriginX + left - 4, kOriginY + top - 4, w, h, phase});
        drawn = false;
        clear_buffer(buffer, {left + kOriginX, top + kOriginY, frame.width, frame.height});
    }

    void grow(int x, int y)
    {
        left = std::min(left, x);
        top = std::min(top, y);
    }

    // An element tagged with a layer: its sprite into the buffer.
    void into_buffer(const AnimElement &e, const Sprite &s)
    {
        const int ex = e.x - kElementBiasX, ey = e.y - kElementBiasY;
        unsigned flags = e.flags & 0xf;
        if (flip) {
            draw_sprite(buffer, kOriginX - ex - s.width, ey + kOriginY, s, flags ^ kDrawFlipX, data.ghost().data());
            grow(-(ex + s.width), ey);
        } else {
            draw_sprite(buffer, ex + kOriginX, ey + kOriginY, s, flags, data.ghost().data());
            grow(ex, ey);
        }
        drawn = true;
    }

    // An element with no layer: its sprite onto the screen.
    void onto_screen(const AnimElement &e, const Sprite &s)
    {
        const int ex = e.x - kElementBiasX, ey = e.y - kElementBiasY;
        unsigned flags = e.flags & 0xf;
        if (flip) {
            draw_sprite(dst, at.x - ex - s.width, at.y + ey, s, flags ^ kDrawFlipX, data.ghost().data());
            grow(-(ex + s.width), ey);
        } else {
            draw_sprite(dst, at.x + ex, at.y + ey, s, flags, data.ghost().data());
            grow(ex, ey);
        }
    }
};

// Whether the element tagged with a layer is the one `layers` picks.
bool layer_picked(const AnimElement &e, const uint8_t *layers)
{
    if (!(e.flags & 0xffb0))
        return true;
    int layer = (e.flags >> 4) & 0x1f;
    return layer < kLayerCount && layers && layers[layer] == (e.flags >> 9);
}

} // namespace

Surface make_wibble_buffer()
{
    Surface buffer;
    buffer.resize(kBufSize, kBufSize);
    buffer.fill(kMaskColour);
    return buffer;
}

Rect draw_wibble_frame(Surface &dst, Surface &buffer, const WibbleFrame &f, const GameData &data,
                       const FrameOptions &options)
{
    const AnimData &anims = data.anims();
    if (f.frame >= anims.frames.size())
        return {};
    WibbleDrawing d{dst, buffer, f.at, f.flip, data, anims.frames[f.frame], f.turn % 17};
    buffer.reset_clip();
    for (size_t i = d.frame.list; i < anims.lists.size() && anims.lists[i] != kListEnd; i++) {
        if (anims.lists[i] >= anims.elements.size())
            continue;
        const AnimElement &e = anims.elements[anims.lists[i]];
        const bool layered = (e.flags & 0xfff0) != 0;
        if (layered && !layer_picked(e, options.layers))
            continue;
        if (!layered && d.drawn)
            d.wobble();
        if (e.sprite == 0)
            continue;
        const Sprite s = data.sprites().at(e.sprite / 6);
        if (layered)
            d.into_buffer(e, s);
        else
            d.onto_screen(e, s);
    }
    if (d.drawn)
        d.wobble();
    clear_buffer(buffer, {d.left + kOriginX, d.top + kOriginY, d.frame.width, d.frame.height});
    if (d.left == 999)
        return {};
    return {f.at.x + d.left - 4, f.at.y + d.top - 4, d.frame.width + 8, d.frame.height + 8};
}

} // namespace th
