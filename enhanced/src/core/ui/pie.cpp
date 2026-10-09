#include "core/ui/pie.h"

#include <cmath>
#include <numbers>

namespace th
{

namespace
{

// sector_behind (0xb852c): whether sector a lies further back than sector b,
// so that it is drawn first.
bool sector_behind(const PieSlice &a, const PieSlice &b)
{
    const int as = pie_cos(a.start), ae = pie_cos(a.end);
    const int bs = pie_cos(b.start), be = pie_cos(b.end);
    if (!(as > 0 || ae < 0))
        return false;
    if (!(ae > 0 || as < 0))
        return true;
    if (!(bs > 0 || be < 0))
        return true;
    if (!(be > 0 || bs < 0))
        return false;
    if ((as < 0 && bs > 0) || (as > 0 && bs < 0))
        return false;
    return -pie_sin(a.start) < -pie_sin(b.start);
}

// sector_render's label position for a sector between `start` and `end`
// drawn about (cx, cy).
void place_label(PieSlice &slice, int cx, int cy, const PieGeometry &g, int mid)
{
    const int text_radius = g.text_offset + slice.pull_out;
    slice.text_x = cx + (fixmul(pie_cos(mid), g.rx * text_radius) >> 8);
    slice.text_y = cy - (fixmul(pie_sin(mid), g.ry * text_radius) >> 8);
}

// The middle of a sector between `start` and `end`, as sector_render takes it.
int sector_mid(int32_t start, int32_t end)
{
    if (end < start)
        end += 0x800;
    return (start + end) >> 1;
}

// The sector pie_render splits, with what the drawing order needs of it.
struct Split {
    int index = -1;
    int centre_x = 0, centre_y = 0;
    bool start_half_first = false;
    PieSlice whole;
};

// pie_render's first pass: the first sector spanning more than half the
// pie, unless it starts on the left and ends on the right, gets its label
// from its whole, is pulled in (pull_out 0) and has the half it starts in
// drawn at once; the other half stays in `slices` for the drawing order.
Split split_wide_sector(std::array<PieSlice, 4> &slices, const PieGeometry &g, std::vector<PieDraw> &draws)
{
    Split split;
    for (size_t i = 0; i < slices.size(); i++) {
        PieSlice &sec = slices[i];
        int span = sec.end - sec.start;
        if (span < 0)
            span += 0x800;
        if (span <= 0x400)
            continue;
        if (pie_cos(sec.start) < 0 && pie_cos(sec.end) > 0)
            break;
        const int mid = sec.start + (span >> 1);
        split.centre_x = g.cx + (fixmul(pie_cos(mid), g.rx * sec.pull_out) >> 8);
        split.centre_y = g.cy - (fixmul(pie_sin(mid), g.ry * sec.pull_out) >> 8);
        place_label(sec, g.cx, g.cy, g, mid);
        split.index = int(i);
        split.whole = sec;
        sec.pull_out = 0;
        split.start_half_first = pie_cos(sec.start) >= 0 || pie_cos(mid) <= 0;
        PieDraw first{i, split.start_half_first ? 2 : 1, split.centre_x, split.centre_y, sec.start, sec.end, 0};
        if (split.start_half_first) {
            first.end = mid;
            sec.start = mid;
        } else {
            first.start = mid;
            sec.end = mid;
        }
        draws.push_back(first);
        break;
    }
    return split;
}

} // namespace

int32_t pie_sin(int angle)
{
    return int32_t(std::sin(double(angle & 0x7ff) * 2 * std::numbers::pi / 2048) * 65536);
}

int32_t pie_cos(int angle)
{
    return pie_sin((angle & 0x7ff) + 0x200);
}

int32_t fixmul(int32_t a, int32_t b)
{
    return int32_t((int64_t(a) * b) >> 16);
}

std::vector<PieDraw> pie_render_plan(std::array<PieSlice, 4> &slices, const PieGeometry &g)
{
    std::vector<PieDraw> draws;
    const Split split = split_wide_sector(slices, g, draws);
    std::array<bool, 4> done{};
    for (size_t j = 0; j < slices.size(); j++) {
        int best = -1;
        for (size_t k = 0; k < slices.size(); k++)
            if (!done[k] && (best == -1 || sector_behind(slices[k], slices[size_t(best)])))
                best = int(k);
        done[size_t(best)] = true;
        PieSlice &sec = slices[size_t(best)];
        PieDraw draw{size_t(best), 0, g.cx, g.cy, sec.start, sec.end, sec.pull_out};
        if (best == split.index) {
            draw.side_off = split.start_half_first ? 1 : 2;
            draw.centre_x = split.centre_x;
            draw.centre_y = split.centre_y;
        } else {
            place_label(sec, g.cx, g.cy, g, sector_mid(sec.start, sec.end));
        }
        draws.push_back(draw);
    }
    if (split.index >= 0)
        slices[size_t(split.index)] = split.whole;
    return draws;
}

} // namespace th
