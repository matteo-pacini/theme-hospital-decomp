// The status chart of Frame: the win and lose criteria, the patients, the
// market share pie (src/overscr.cpp, src/pie.cpp, src/sim.cpp).

#include "core/ui/charts_internal.h"

#include "core/data/balance.h"
#include "core/ui/pie.h"
#include "core/world/finance.h"
#include "core/world/year_end.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace th
{

namespace
{

// A criterion as check_criteria_values (src/sim.cpp) leaves it.
struct Criterion {
    int criterion = 0, op = 0, group = 0;
    int32_t value = 0, base = 0, current = 0;
    bool met = false;
    int progress = 0;
};

// check_criteria_values (src/sim.cpp): win (mode 0) or lose criteria
// measured on `h`.
std::array<Criterion, 10> check_criteria_values(const std::array<Balance::WinLose, 10> &table, int mode,
                                                const HospitalFigures &h, int *groups)
{
    std::array<Criterion, 10> out{};
    int max = 0;
    for (size_t i = 0; i < 10; i++) {
        Criterion &c = out[i];
        c.criterion = table[i].criteria;
        c.op = table[i].max_min;
        c.value = table[i].value;
        c.base = table[i].bound;
        c.group = table[i].group;
        max = std::max(max, c.group);
        int32_t val = 0;
        switch (c.criterion) {
        case 0:
            c.met = true;
            break;
        case 1:
            val = h.reputation;
            break;
        case 2:
            val = mode == 0 ? h.balance - h.loan : h.balance;
            break;
        case 3:
            val = h.visitors > 10 ? h.diagnosed * 100 / h.visitors : 0;
            break;
        case 4:
            val = h.cures;
            break;
        case 5:
            val = h.visitors > 10 ? h.deaths * 100 / h.visitors : 0;
            break;
        case 6:
            val = h.value;
            break;
        }
        if (c.op == 1 ? val >= c.value : val <= c.value)
            c.met = true;
        int32_t range = c.value - c.base, pct = 0;
        if (range != 0)
            pct = (val - c.base) * 100 / range;
        c.progress = std::clamp(pct, 0, 200);
        c.current = val;
    }
    if (groups)
        *groups = max;
    return out;
}

// evaluate_criteria (src/sim.cpp), one criterion's share of its group's
// score: 100 when met, else how near it is, 0 to 100.
int criterion_score(const Criterion &c)
{
    if (c.met)
        return 100;
    int pct = 0;
    if (c.op == 1) {
        if (c.value != 0) {
            pct = std::abs(c.current * 100 / c.value);
            if (c.criterion == 2 && c.current < 0)
                pct = 0;
        }
    } else if (c.current != 0) {
        pct = std::abs(c.value * 100 / c.current);
    }
    return std::clamp(pct, 0, 100);
}

// evaluate_criteria (src/sim.cpp), its best group: the group of win
// criteria nearest to being met, 0 for none (the level not won).
int best_win_group(const std::array<Criterion, 10> &win, int groups)
{
    int best = 0, best_group = 0;
    for (int group = 1; group <= groups; group++) {
        int count = 0, sum = 0;
        for (const Criterion &c : win) {
            if (c.group != group)
                continue;
            count++;
            sum += criterion_score(c);
        }
        if (count && sum / count > best) {
            best = sum / count;
            best_group = group;
        }
    }
    return best_group;
}

} // namespace

inline namespace charts_detail
{

HospitalFigures Frame::figures(int hospital) const
{
    HospitalFigures h;
    const Balance &bal = game_.settings();
    if (hospital == 0) {
        Finance fin = game_.finance;
        calculate_value(fin, game_.world, bal, game_.reputation());
        h.reputation = game_.reputation();
        h.balance = game_.balance;
        h.loan = fin.loan;
        h.value = fin.hospital_value;
        h.cures = fin.cures;
        h.deaths = fin.deaths;
    } else {
        // Game::overview_frame writes the shown hospital's value back each frame; a copy
        // gives the same figures here.
        RivalHospital rival = game_.world.rivals[size_t(hospital)];
        h.reputation = game_.arrivals.hospitals[size_t(hospital)].hospital_reputation;
        calculate_value(rival, h.reputation, bal);
        h.balance = rival.balance;
        h.loan = rival.loan;
        h.value = rival.hospital_value;
        h.cures = rival.cures;
        h.deaths = rival.deaths;
    }
    const HospitalMarket &market = game_.arrivals.hospitals[size_t(hospital)];
    h.visitors = int(market.visitors);
    h.diagnosed = int(market.diagnosed);
    return h;
}

// allhosps[hospital].market_share: the percent of the month's patients it took.
int Frame::market_share(size_t hospital) const
{
    return hospital == 0 ? game_.progress.market_share : game_.world.rivals[hospital].market_share;
}

void Frame::display_peep_overview()
{
    switch (c_.state) {
    case 0:
        load_pic();
        c_.state = 1;
        c_.value = 0;
        draw_overview_screen();
        draw_piechart();
        break;
    case 1:
        copy_screen();
        process_overview_panel();
        r_.overview_hospital = c_.value;
        draw_overview_screen();
        draw_piechart();
        break;
    case 2:
        return_from_screen();
        break;
    }
    if (exit_quick_requester())
        c_.state = 2;
}

// display_peep_overview, the buttons: the four hospitals' and the
// exit, pressed while held and acted on when released.
void Frame::process_overview_panel()
{
    if (mleft_) {
        for (int slot = 0; slot < 4; slot++) {
            const Rect &b = kBoxes[0x37 + slot];
            if (over(mx_, my_, b.x, b.y, b.w, b.h))
                reqspr(b.x + 0x13, b.y, 9);
        }
        const Rect &b = kBoxes[0x3c];
        if (over(mx_, my_, b.x, b.y, b.w, b.h))
            reqspr(b.x, b.y, 8);
    }
    if (!rleft_)
        return;
    for (int slot = 0; slot < 4; slot++) {
        const Rect &b = kBoxes[0x37 + slot];
        if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
            c_.value = slot;
            sound(kSfxSelectX);
            reqspr(b.x + 0x13, b.y, 9);
        }
    }
    const Rect &b = kBoxes[0x3c];
    if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
        c_.state = 2;
        sound(kSfxNo4);
        reqspr(b.x, b.y, 8);
    }
    rleft_ = false;
}

void Frame::draw_overview_screen()
{
    text(0x24, kCoords[0x6f].x, kCoords[0x6f].y,
         format("%s %4.4d", std::string(str(TextSection::Overview, 0)).c_str(), game_.date.year + 2000));
    for (int i = 0; i < 4; i++)
        text(c_.value == i ? 0x25 : 0x24, kCoords[0x68 + i].x, kCoords[0x68 + i].y, c_.hospital_names[size_t(i)]);
    draw_overview_patients();
    std::string share =
        format("%s (%d%%)", std::string(str(TextSection::Overview, 6)).c_str(), market_share(size_t(c_.value)));
    text(0x26, kCoords[0x70].x, kCoords[0x70].y, conv_string(share, 0));
    text(0x26, kCoords[0x71].x, kCoords[0x71].y, str(TextSection::Overview, 7));
    display_overview_bars();
}

// draw_overview_screen, the patients' average happiness, refreshment
// and warmth on their scales, with the warnings they call for.
void Frame::draw_overview_patients()
{
    // The patients' figures (adcounts, counted from 1 by the game's
    // statistics pass over the patients indoors).
    int patients = 1, happiness = 0, refreshment = 0, warmth = 0;
    for (const Thing &t : game_.world.things) {
        if (t.type != kThingPatient || t.owner != 0)
            continue;
        if (game_.world.map[size_t(t.y.tile) * kMapSize + t.x.tile].owner_flags & kCellOutdoor)
            continue;
        patients++;
        happiness += t.happiness;
        refreshment += t.refreshment;
        warmth += t.warmth;
    }
    if (patients <= 1)
        return;
    const Balance::Gbv &gbv = game_.settings().gbv;
    std::string warn;
    int pct = std::min(happiness / patients, 100);
    if (gbv.very_unhappy > pct)
        warn += str(TextSection::Overview, 1);
    else if (gbv.unhappy > pct)
        warn += str(TextSection::Overview, 2);
    reqspr(kCoords[0x6c].x + pct, kCoords[0x6c].y, 5);
    pct = std::min(refreshment / patients, 100);
    if (pct < 30)
        warn += str(TextSection::Overview, 3);
    reqspr(kCoords[0x6d].x + pct, kCoords[0x6d].y, 5);
    pct = std::min(warmth / patients, 100);
    if (pct > 80)
        warn += str(TextSection::Overview, 4);
    else if (pct < 20)
        warn += str(TextSection::Overview, 5);
    reqspr(kCoords[0x6e].x + pct, kCoords[0x6e].y, 5);
    reqspr(0xf5, 0x12c, 7);
    text(0x24, kBoxes[0x3b], {0, 0}, warn, kTextWrap);
}

// display_overview_bars: up to five criteria bars, a lose criterion with
// progress in place of the win criterion of its kind in the player's
// best group.
void Frame::display_overview_bars()
{
    const Balance &bal = game_.settings();
    int groups = 0;
    auto player_win = check_criteria_values(bal.win_criteria, 0, figures(0), &groups);
    int best_group = best_win_group(player_win, groups);
    HospitalFigures shown = figures(c_.value);
    auto win = check_criteria_values(bal.win_criteria, 0, shown, nullptr);
    auto lose = check_criteria_values(bal.lose_criteria, 1, shown, nullptr);
    int col = 0;
    auto bar = [&](int progress, int criterion, int fill, int top) {
        for (int j = 0; j < 50; j++) {
            if (progress / 2 >= j) {
                reqspr(col * 30 + 0x10d, 0xed - j, fill);
                reqspr(col * 30 + 0x10d, 0xed - j - 1, top);
            }
        }
        reqspr(col * 30 + 0x106, 0xf3, criterion + 9);
        col++;
    };
    for (int crit = 0; crit <= 6 && col < 5; crit++) {
        auto l = std::find_if(lose.begin(), lose.end(),
                              [&](const Criterion &c) { return c.criterion == crit && c.progress; });
        if (l != lose.end()) {
            bar(l->progress, l->criterion, 3, 4);
            continue;
        }
        auto w = std::find_if(win.begin(), win.end(),
                              [&](const Criterion &c) { return c.criterion == crit && c.group == best_group; });
        if (w != win.end())
            bar(w->progress, w->criterion, 1, 2);
    }
}

// calculate_piechart and pie_render (src/pie.cpp): a slice for each
// hospital's market share (single player), the shown hospital's pulled out.
void Frame::draw_piechart()
{
    std::array<PieSlice, 4> slices = calculate_piechart();
    const PieGeometry geometry = {kPieX, kPieY, kPieRx, kPieRy, kSliceTextOffset};
    for (const PieDraw &draw : pie_render_plan(slices, geometry))
        sector_render(draw, kSlices[draw.slice]);
    for (size_t i = 0; i < 4; i++)
        text(0x26, slices[i].text_x, slices[i].text_y, c_.hospital_names[i]);
}

// calculate_piechart: the slices' angles, and their pull, eased towards
// the shown hospital's.
std::array<PieSlice, 4> Frame::calculate_piechart()
{
    std::array<PieSlice, 4> sectors{};
    std::array<int, 4> share{};
    int total = 0;
    for (size_t i = 0; i < 4; i++) {
        share[i] = market_share(i);
        total += share[i];
    }
    int prev = 0;
    for (size_t i = 0; i < 4; i++) {
        sectors[i].start = prev;
        if (total == 0)
            sectors[i].end = (prev + 0x200) & 0x7ff;
        else if (share[i] == 100)
            sectors[i].end = (prev + 0x7eb) & 0x7ff;
        else
            sectors[i].end = (prev + (share[i] << 11) / 100) & 0x7ff;
        prev = sectors[i].end;
    }
    sectors[3].end = 0x7ff;
    for (size_t i = 0; i < 4; i++) {
        sectors[i].start += 4;
        sectors[i].end += 4;
        if (c_.pie_pull[i] != 0 && int(i) != c_.value)
            c_.pie_pull[i] -= 2;
    }
    if (c_.pie_pull[size_t(c_.value)] < 0x50)
        c_.pie_pull[size_t(c_.value)] += 2;
    for (size_t i = 0; i < 4; i++)
        sectors[i].pull_out = c_.pie_pull[i];
    return sectors;
}

// sector_render (src/pie.cpp): one sector of the pie, as `draw` plans it.
void Frame::sector_render(const PieDraw &draw, const SliceColours &c)
{
    const int rx = kPieRx, ry = kPieRy, h = kPieH;
    const int start = draw.start;
    const int end = draw.end < start ? draw.end + 0x800 : draw.end;
    const int mid = (start + end) >> 1;
    Point o = {draw.centre_x + (fixmul(pie_cos(mid), rx * draw.pull_out) >> 8),
               draw.centre_y - (fixmul(pie_sin(mid), ry * draw.pull_out) >> 8)};
    Point s = {o.x + fixmul(pie_cos(start), rx), o.y - fixmul(pie_sin(start), ry)};
    Point e = {o.x + fixmul(pie_cos(end), rx), o.y - fixmul(pie_sin(end), ry)};
    const int side_off = draw.side_off;
    if (side_off != 1) {
        draw_triangle(dst_, o, s, {s.x, s.y + h}, c.side1);
        draw_triangle(dst_, {s.x, s.y + h}, {o.x, o.y + h}, o, c.side1);
    }
    if (side_off != 2) {
        draw_triangle(dst_, o, {o.x, o.y + h}, e, c.side2);
        draw_triangle(dst_, e, {o.x, o.y + h}, {e.x, e.y + h}, c.side2);
    }
    sector_render_top(o, start, end, c);
    if (side_off != 1)
        sector_render_edges(o, s, pie_cos(start) >= 0, c.edge);
    if (side_off != 2)
        sector_render_edges(o, e, pie_cos(end) <= 0, c.edge);
}

// sector_render, the top and rim: in steps of about 50 from `start` to
// `end` about `o`.
void Frame::sector_render_top(Point o, int start, int end, const SliceColours &c)
{
    const int rx = kPieRx, ry = kPieRy, h = kPieH;
    int segments = std::max((end - start) / 50, 1);
    int ang = start, step = (end - start) / segments;
    int x2 = o.x + fixmul(pie_cos(ang), rx), y2 = o.y - fixmul(pie_sin(ang), ry);
    while (--segments >= 0) {
        int x1 = x2, y1 = y2;
        ang += step;
        if (segments == 0)
            ang = end;
        x2 = o.x + fixmul(pie_cos(ang), rx);
        y2 = o.y - fixmul(pie_sin(ang), ry);
        draw_triangle(dst_, {x1, y1 + h}, {x1, y1}, {x2, y2}, c.rim);
        draw_triangle(dst_, {x1, y1 + h}, {x2, y2}, {x2, y2 + h}, c.rim);
        draw_triangle(dst_, o, {x2, y2}, {x1, y1}, c.top);
        draw_line(dst_, x1 - 1, y1, x2 - 1, y2, c.edge);
        if (!(pie_sin(ang) > 0 || pie_sin(ang - step) > 0))
            draw_line(dst_, x1 - 1, y1 + h, x2 - 1, y2 + h, c.edge);
    }
}

// sector_render, a straight side's outline from `o` to `p`, and its
// lower and upright edges too when it `faces` the viewer.
void Frame::sector_render_edges(Point o, Point p, bool faces, uint8_t edge)
{
    const int h = kPieH;
    draw_line(dst_, o.x - 1, o.y, p.x - 1, p.y, edge);
    if (!faces)
        return;
    draw_line(dst_, o.x - 1, o.y + h, p.x - 1, p.y + h, edge);
    draw_line(dst_, o.x - 1, o.y, o.x - 1, o.y + h, edge);
    draw_line(dst_, p.x - 1, p.y, p.x - 1, p.y + h, edge);
}

} // namespace charts_detail

} // namespace th
