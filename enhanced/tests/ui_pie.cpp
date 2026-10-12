// The pie chart's render plan (core/ui/pie.h).

#include "common.h"

#include "core/ui/pie.h"

#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <vector>

namespace
{

// src/pie.cpp's pie_render, transcribed with sector_render reduced to
// recording its call and setting the sector's label (sec->text_x/y): the
// reference for pie_render_plan.
struct PieReference {
    std::array<th::PieSlice, 4> sectors;
    th::PieGeometry g;
    std::vector<th::PieDraw> log;
    int side_off = 0;

    static int sin_at(int a) { return th::pie_sin(a); }
    static int cos_at(int a) { return th::pie_sin((a & 0x7ff) + 0x200); }

    void sector_render(int cx, int cy, int i)
    {
        th::PieSlice &sec = sectors[size_t(i)];
        int start = sec.start, end = sec.end;
        if (end < start)
            end += 0x800;
        int mid = (start + end) >> 1;
        int text_radius = g.text_offset + sec.pull_out;
        sec.text_x = cx + (th::fixmul(cos_at(mid), g.rx * text_radius) >> 8);
        sec.text_y = cy - (th::fixmul(sin_at(mid), g.ry * text_radius) >> 8);
        log.push_back({size_t(i), side_off, cx, cy, sec.start, sec.end, sec.pull_out});
    }

    bool sector_behind(int a, int b) const
    {
        int as = cos_at(sectors[size_t(a)].start), ae = cos_at(sectors[size_t(a)].end);
        int bs = cos_at(sectors[size_t(b)].start), be = cos_at(sectors[size_t(b)].end);
        if (!(as > 0 || ae < 0))
            return false;
        if (!(ae > 0 || as < 0))
            return true;
        if (!(bs > 0 || be < 0))
            return true;
        if (!(be > 0 || bs < 0))
            return false;
        if (as < 0 && bs > 0)
            return false;
        if (as > 0 && bs < 0)
            return false;
        int asin = -sin_at(sectors[size_t(a)].start), bsin = -sin_at(sectors[size_t(b)].start);
        return asin < bsin;
    }

    int split = -1, ox = 0, oy = 0;
    bool start_half_first = false;
    th::PieSlice copy;

    // The first loop of pie_render: splitting the first sector over half the pie.
    void split_pass()
    {
        for (int i = 0; i < 4; i++) {
            int span = sectors[size_t(i)].end - sectors[size_t(i)].start;
            if (span < 0)
                span += 0x800;
            if (span <= 0x400)
                continue;
            if (!(cos_at(sectors[size_t(i)].start) < 0 && cos_at(sectors[size_t(i)].end) > 0))
                split_sector(i, span);
            break;
        }
    }

    void split_sector(int i, int span)
    {
        th::PieSlice *sec = &sectors[size_t(i)];
        int mid = sec->start + (span >> 1);
        ox = g.cx + (th::fixmul(cos_at(mid), g.rx * sec->pull_out) >> 8);
        oy = g.cy - (th::fixmul(sin_at(mid), g.ry * sec->pull_out) >> 8);
        int text_radius = g.text_offset + sec->pull_out;
        sec->text_x = g.cx + (th::fixmul(cos_at(mid), g.rx * text_radius) >> 8);
        sec->text_y = g.cy - (th::fixmul(sin_at(mid), g.ry * text_radius) >> 8);
        split = i;
        copy = *sec;
        sec->pull_out = 0;
        start_half_first = cos_at(sec->start) >= 0 || cos_at(mid) <= 0;
        if (start_half_first) {
            sec->end = mid;
            side_off = 2;
            sector_render(ox, oy, i);
            sec->end = copy.end;
            sec->start = mid;
        } else {
            sec->start = mid;
            side_off = 1;
            sector_render(ox, oy, i);
            sec->start = copy.start;
            sec->end = mid;
        }
    }

    void pie_render()
    {
        split_pass();
        bool done[4] = {};
        for (int j = 4; --j >= 0;) {
            int best = -1;
            for (int k = 0; k < 4; k++)
                if (!done[k] && (best == -1 || sector_behind(k, best)))
                    best = k;
            done[best] = true;
            if (best == split) {
                side_off = start_half_first ? 1 : 2;
                sector_render(ox, oy, best);
            } else {
                side_off = 0;
                sector_render(g.cx, g.cy, best);
            }
        }
        if (split != -1)
            sectors[size_t(split)] = copy;
    }
};

// The slices calculate_piechart makes of `share` percents (before the pull).
std::array<th::PieSlice, 4> pie_slices(const std::array<int, 4> &share, int pull)
{
    std::array<th::PieSlice, 4> s{};
    int total = share[0] + share[1] + share[2] + share[3];
    int prev = 0;
    for (size_t i = 0; i < 4; i++) {
        s[i].start = prev;
        if (total == 0)
            s[i].end = (prev + 0x200) & 0x7ff;
        else if (share[i] == 100)
            s[i].end = (prev + 0x7eb) & 0x7ff;
        else
            s[i].end = (prev + (share[i] << 11) / 100) & 0x7ff;
        prev = s[i].end;
    }
    s[3].end = 0x7ff;
    for (size_t i = 0; i < 4; i++) {
        s[i].start += 4;
        s[i].end += 4;
        s[i].pull_out = int(i) == 0 ? pull : 0;
    }
    return s;
}

} // namespace

// pie_render_plan draws the same sectors, in the same order and about the
// same centres, as src/pie.cpp's pie_render, and places the same labels, for
// slices of every size including one over half the pie (which it splits).
void test_pie_render_plan()
{
    const std::array<std::array<int, 4>, 8> shares = {{{25, 25, 25, 25},
                                                       {0, 0, 0, 0},
                                                       {70, 10, 10, 10},
                                                       {10, 70, 10, 10},
                                                       {5, 10, 80, 5},
                                                       {10, 10, 10, 70},
                                                       {100, 0, 0, 0},
                                                       {0, 0, 100, 0}}};
    int splits = 0;
    for (const auto &share : shares) {
        for (int pull : {0, 0x28, 0x50}) {
            PieReference ref;
            ref.g = {0x1fa, 0x78, 0x40, 0x1e, 0xaf};
            ref.sectors = pie_slices(share, pull);
            std::array<th::PieSlice, 4> slices = ref.sectors;
            ref.pie_render();
            std::vector<th::PieDraw> plan = th::pie_render_plan(slices, ref.g);
            CHECK(plan.size() == ref.log.size());
            for (size_t i = 0; i < plan.size() && i < ref.log.size(); i++) {
                const th::PieDraw &a = plan[i], &b = ref.log[i];
                CHECK(a.slice == b.slice && a.side_off == b.side_off);
                CHECK(a.centre_x == b.centre_x && a.centre_y == b.centre_y);
                CHECK(a.start == b.start && a.end == b.end && a.pull_out == b.pull_out);
            }
            for (size_t i = 0; i < 4; i++) {
                CHECK(slices[i].start == ref.sectors[i].start && slices[i].end == ref.sectors[i].end);
                CHECK(slices[i].pull_out == ref.sectors[i].pull_out);
                CHECK(slices[i].text_x == ref.sectors[i].text_x && slices[i].text_y == ref.sectors[i].text_y);
            }
            splits += plan.size() == 5;
        }
    }
    // One draw more than sectors where a slice is split: some cases split.
    CHECK(splits > 0);
}
