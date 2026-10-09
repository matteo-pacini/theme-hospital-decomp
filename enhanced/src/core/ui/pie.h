#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace th
{

// The status screen's pie chart (src/pie.cpp): which sectors it draws, in
// what order and about which centre, apart from the drawing itself.

// A sector (Sector): its angles in 2048ths of a turn, how far it is pulled
// out of the pie (1/256 of the radii) and where its label goes.
struct PieSlice {
    int32_t start = 0, end = 0, pull_out = 0;
    int text_x = 0, text_y = 0;
};

// The pie's centre, radii and the labels' distance (a sector's text_offset).
struct PieGeometry {
    int cx = 0, cy = 0, rx = 0, ry = 0, text_offset = 0;
};

// One call of sector_render: sector `slice` between `start` and `end`
// (`end` below `start` wraps), pulled out `pull_out` from the centre
// (centre_x, centre_y). `side_off` 1 or 2 leaves out its start or end side.
struct PieDraw {
    size_t slice = 0;
    int side_off = 0;
    int centre_x = 0, centre_y = 0;
    int32_t start = 0, end = 0, pull_out = 0;
};

// lbSinTable (src/lib/bf1/gtrig.cpp): the sine and cosine of an angle in
// 2048ths of a turn, 16.16 truncated; and fixmul, the 16.16 product.
int32_t pie_sin(int angle);
int32_t pie_cos(int angle);
int32_t fixmul(int32_t a, int32_t b);

// pie_render (0xb86a8): the sector_render calls that draw `slices` back to
// front. A sector spanning more than half the pie, unless it starts on the
// left and ends on the right, is split in two so that it overlaps correctly:
// its first half is drawn at once, its second in the drawing order, about the
// centre its pull-out moves it to. Sets each slice's label position.
std::vector<PieDraw> pie_render_plan(std::array<PieSlice, 4> &slices, const PieGeometry &geometry);

} // namespace th
