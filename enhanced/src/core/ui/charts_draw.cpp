// The helpers the charts draw with: screen tests, palette colours, the
// code page 437 case conversion and the box, line and triangle primitives
// (src/utils.cpp, src/lib/bf1/gpalette.cpp, src/asm/tri_f.s).

#include "core/ui/charts_internal.h"

#include <algorithm>
#include <cmath>

namespace th
{

inline namespace charts_detail
{

bool over(int mx, int my, int x, int y, int w, int h)
{
    return mx >= x && mx < x + w && my >= y && my < y + h;
}

// LbPaletteFindColour (src/lib/bf1/gpalette.cpp) for 6-bit r, g, b: least
// squared distance, then least sum of differences, then the darkest, then
// the lowest index.
uint8_t find_colour(const Palette &palette, int r, int g, int b)
{
    auto channel = [&](int i, int shift) { return int((palette[size_t(i)] >> shift) & 0xff) >> 2; };
    auto sq = [](int v) { return v * v; };
    auto dist = [&](int i) { return sq(r - channel(i, 16)) + sq(g - channel(i, 8)) + sq(b - channel(i, 0)); };
    auto diff = [&](int i) {
        return std::abs(r - channel(i, 16)) + std::abs(g - channel(i, 8)) + std::abs(b - channel(i, 0));
    };
    auto dark = [&](int i) { return 2 * sq(channel(i, 16)) + 2 * sq(channel(i, 8)) + sq(channel(i, 0)); };
    int best = 999999;
    for (int i = 0; i < 256; i++)
        best = std::min(best, dist(i));
    int best_diff = 999999, best_dark = 999999, found = 0;
    for (int i = 0; i < 256; i++)
        if (dist(i) == best)
            best_diff = std::min(best_diff, diff(i));
    for (int i = 0; i < 256; i++) {
        if (dist(i) == best && diff(i) == best_diff && dark(i) < best_dark) {
            best_dark = dark(i);
            found = i;
        }
    }
    return uint8_t(found);
}

Colours setup_colours(const Palette &p)
{
    return {find_colour(p, 0, 0, 0),          find_colour(p, 0, 0, 0x10),       find_colour(p, 0, 0x3f, 0),
            find_colour(p, 0x3f, 0x30, 0x17), find_colour(p, 0x3f, 0x17, 0x17), find_colour(p, 0x3f, 0, 0),
            find_colour(p, 0x35, 0x1f, 0x42), find_colour(p, 0, 0x26, 0x56),    find_colour(p, 0x3f, 0x1e, 0x1e)};
}

// conv_string (src/utils.cpp): mode 0 upper-cases, code page 437 accented
// letters too (to_upper_char); mode 1 lower-cases ASCII.
std::string conv_string(std::string_view s, int mode)
{
    static constexpr std::pair<uint8_t, uint8_t> kUpper[] = {
        {0x81, 0x9a}, {0x82, 0x90}, {0x83, 0x41}, {0x84, 0x8e}, {0x85, 0x41}, {0x86, 0x8f}, {0x87, 0x80}, {0x88, 0x45},
        {0x89, 0x45}, {0x8a, 0x45}, {0x8b, 0x49}, {0x8c, 0x49}, {0x8d, 0x49}, {0x93, 0x4f}, {0x94, 0x99}, {0x95, 0x4f},
        {0x96, 0x55}, {0x97, 0x55}, {0x98, 0x59}, {0xa0, 0x41}, {0xa1, 0x49}, {0xa2, 0x4f}, {0xa3, 0x55}, {0xa4, 0xa5}};
    std::string out(s);
    for (char &ch : out) {
        uint8_t c = uint8_t(ch);
        if (mode == 1) {
            if (c >= 'A' && c <= 'Z')
                ch = char(c + 32);
        } else if (c < 0x80) {
            if (c >= 'a' && c <= 'z')
                ch = char(c - 32);
        } else {
            for (auto [from, to] : kUpper)
                if (c == from)
                    ch = char(to);
        }
    }
    return out;
}

// LbDrawBox, filled.
void draw_box(Surface &dst, int x, int y, int w, int h, uint8_t colour)
{
    int x0 = std::max(x, 0), y0 = std::max(y, 0);
    int x1 = std::min(x + w, dst.width()), y1 = std::min(y + h, dst.height());
    for (int yy = y0; yy < y1; yy++)
        for (int xx = x0; xx < x1; xx++)
            dst.row(yy)[xx] = colour;
}

void plot(Surface &dst, int x, int y, uint8_t colour)
{
    if (x >= 0 && y >= 0 && x < dst.width() && y < dst.height())
        dst.row(y)[x] = colour;
}

// LbDrawLine: both ends included.
void draw_line(Surface &dst, int x1, int y1, int x2, int y2, uint8_t colour)
{
    int dx = std::abs(x2 - x1), dy = -std::abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        plot(dst, x1, y1, colour);
        if (x1 == x2 && y1 == y2)
            break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// LbDrawTriangleFilled (src/asm/tri_f.s): the pixels whose centres are
// inside or on the triangle, drawn only if its corners run clockwise on
// screen; the pie relies on that to leave out its slices' far sides.
void draw_triangle(Surface &dst, Point p1, Point p2, Point p3, uint8_t colour)
{
    const int x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y, x3 = p3.x, y3 = p3.y;
    if ((x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1) <= 0)
        return;
    int top = std::max(std::min({y1, y2, y3}), 0), bottom = std::min(std::max({y1, y2, y3}), dst.height() - 1);
    const int xs[3] = {x1, x2, x3}, ys[3] = {y1, y2, y3};
    for (int y = top; y <= bottom; y++) {
        double lo = 1e9, hi = -1e9;
        for (int e = 0; e < 3; e++) {
            int ax = xs[e], ay = ys[e], bx = xs[(e + 1) % 3], by = ys[(e + 1) % 3];
            if ((y < ay && y < by) || (y > ay && y > by))
                continue;
            if (ay == by) {
                lo = std::min({lo, double(ax), double(bx)});
                hi = std::max({hi, double(ax), double(bx)});
            } else {
                double x = ax + double(y - ay) * (bx - ax) / (by - ay);
                lo = std::min(lo, x);
                hi = std::max(hi, x);
            }
        }
        for (int x = std::max(int(std::ceil(lo)), 0); x <= std::min(int(std::floor(hi)), dst.width() - 1); x++)
            dst.row(y)[x] = colour;
    }
}

} // namespace charts_detail

} // namespace th
