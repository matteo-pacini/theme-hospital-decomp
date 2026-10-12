// The graphs chart of Frame: the statistics' lines, keys and dates
// (src/graph.cpp).

#include "core/ui/charts_internal.h"

#include "core/world/finance.h"

#include <algorithm>

namespace th
{

namespace
{

// The graphs' statistics in the order of their buttons (graph[], src/tables.cpp,
// none inverted, each its own colour), and the history row each plots
// (draw_graphs).
constexpr std::array<int, 8> kGraphRows = {kHistoryIncome,   kHistoryExpenditure, kHistoryWages,  kHistoryBalance,
                                           kHistoryVisitors, kHistoryCures,       kHistoryDeaths, kHistoryReputation};

} // namespace

inline namespace charts_detail
{

void Frame::draw_graphs(int x, int y, int h, int step)
{
    switch (c_.state) {
    case 0:
        load_pic();
        c_.state = 1;
        break;
    case 1:
        copy_screen();
        process_graph_panel();
        if (exit_quick_requester())
            c_.state = 2;
        break;
    case 2:
        return_from_screen();
        break;
    }
    Colours col = colours();
    const std::array<uint8_t, 8> colour = {col.red,  col.light_red, col.orange, col.green,
                                           col.blue, col.dark_blue, col.purple, col.salmon};
    draw_graphs_keys(colour);
    std::vector<GraphLabel> labels;
    for (int i = 0; i < 8; i++)
        if (c_.graph_visible[size_t(i)] == 1)
            draw_graphs_line(i, {x, y, h, step}, colour[size_t(i)], labels);
    spread_graph_labels(labels);
    for (const GraphLabel &l : labels) {
        std::string s = format("%s:%d", std::string(str(TextSection::Report, l.graph + 3)).c_str(), l.data);
        text(kGraphFont, x + 4, l.y, s, kTextOneColour, colour[size_t(l.graph)]);
    }
    draw_graphs_dates(x, y, step, col.black);
}

// draw_graphs: whether the nth step back from now is drawn, the game
// being old enough for it at the period shown.
bool Frame::graph_shown(int n) const
{
    const int scale = c_.graph_period == 0 ? 1 : c_.graph_period == 1 ? 12 : 48;
    return uint32_t(n * 1500 * scale) < game_.turn_count;
}

// draw_graphs, the keys: the hidden graphs' buttons up, the names in
// their colours.
void Frame::draw_graphs_keys(const std::array<uint8_t, 8> &colour)
{
    int font_height = line_height(font(kGraphFont));
    for (int i = 0; i < 8; i++) {
        const ScreenButton &b = kButtons[i + kGraphButtons];
        if (!c_.graph_visible[size_t(i)])
            reqspr(b.sprite_x, b.sprite_y, b.sprite);
        const Rect &box = kBoxes[i + 47];
        text(kGraphFont, box, {0, (box.h - font_height) >> 1}, str(TextSection::Report, i + 3),
             kTextCentre | kTextOneColour, colour[size_t(i)]);
    }
}

// draw_graphs, one graph: its line, from now at (x, y) leftwards by
// `step` a point, over height `h`, scaled to its own range; its label
// goes into `labels`.
void Frame::draw_graphs_line(int graph, GraphArea area, uint8_t colour, std::vector<GraphLabel> &labels)
{
    const int x = area.x, y = area.y, h = area.h, step = area.step;
    int font_height = line_height(font(kGraphFont));
    // The game keeps a history per scale, all the same.
    const auto &values = game_.finance.history[size_t(kGraphRows[size_t(graph)])];
    int32_t max = -100000000, min = 100000000;
    for (int k = 11; k >= 0 && graph_shown(12 - k - 1); k--) {
        max = std::max(max, values[size_t(k)]);
        min = std::min(min, values[size_t(k)]);
    }
    if (max == min) {
        max <<= 1;
        min = 0;
    }
    int64_t range = int64_t(max) - min;
    if (range == 0)
        range = 1;
    int px = x;
    int py = uint16_t(y - int((values[11] - min) * int64_t(h) / range));
    int at = uint16_t(py - font_height / 2);
    auto pos = std::find_if(labels.begin(), labels.end(), [&](const GraphLabel &l) { return l.y > at; });
    labels.insert(pos, {graph, at, values[11]});
    for (int k = 11; k > 0 && graph_shown(12 - k); k--) {
        int nx = x - (12 - k) * step;
        int ny = uint16_t(y - int((values[size_t(k - 1)] - min) * int64_t(h) / range));
        ny = std::clamp(ny, y - h, y);
        draw_line(dst_, px, py, nx, ny, colour);
        draw_line(dst_, px - 1, py, nx - 1, ny, colour);
        px = nx;
        py = ny;
    }
}

// spread_graph_labels: nudged apart a pixel at a time, within the
// graph.
void Frame::spread_graph_labels(std::vector<GraphLabel> &labels) const
{
    int font_height = line_height(font(kGraphFont));
    for (bool moved = true; moved;) {
        moved = false;
        for (size_t i = 0; i < labels.size(); i++) {
            bool below = i > 0 && labels[i].y - labels[i - 1].y < font_height;
            bool above = i + 1 < labels.size() && labels[i + 1].y - labels[i].y < font_height;
            if (below && !above) {
                if (labels[i].y < kBoxes[46].y)
                    labels[i].y++;
                moved = true;
            }
            if (!below && above) {
                if (labels[i].y > kBoxes[46].y - kBoxes[46].h - font_height)
                    labels[i].y--;
                moved = true;
            }
        }
    }
}

// draw_graphs, the time axis: a tick and a month or year per step back
// from (x, y).
void Frame::draw_graphs_dates(int x, int y, int step, uint8_t black)
{
    const int f = kGraphFont;
    int month = game_.date.month;
    int year = int(game_.turn_count / 0x4650);
    int span_year = year - (year & 3);
    for (int i = 0; i < 12 && graph_shown(i); i++) {
        int px = x - i * step;
        std::string s;
        if (c_.graph_period == 0) {
            s = std::string(text_.month(month));
            month = month == 0 ? 11 : month - 1;
        } else if (c_.graph_period == 1) {
            s = format("%d", year--);
        } else {
            s = format("%d", span_year);
            span_year -= 4;
        }
        // LbTextWordWidth: the first word's width.
        int width = text_width(font(f), std::string_view(s).substr(0, s.find(' ')));
        text(f, px + 2 - width / 2, y + 10, s, kTextOneColour, black);
        draw_line(dst_, px, y + 2, px, y + 6, black);
    }
}

void Frame::process_graph_panel()
{
    if (rleft_ || rright_) {
        for (int i = kGraphButtons; kButtons[i].w && kButtons[i].h; i++) {
            const ScreenButton &b = kButtons[i];
            if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                process_graph_button(i - kGraphButtons);
                reqspr(b.sprite_x, b.sprite_y, b.sprite);
                break;
            }
        }
        rleft_ = rright_ = false;
    }
    if (mleft_ || mright_) {
        for (int i = kGraphButtons; kButtons[i].w && kButtons[i].h; i++) {
            const ScreenButton &b = kButtons[i];
            if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                reqspr(b.sprite_x, b.sprite_y, b.sprite);
                break;
            }
        }
    }
}

void Frame::process_graph_button(int button)
{
    if (button < 8) {
        if (rright_) {
            c_.graph_visible.fill(0);
            c_.graph_visible[size_t(button)] = 1;
        } else {
            c_.graph_visible[size_t(button)] = !c_.graph_visible[size_t(button)];
        }
        sound(kSfxSelectX);
    } else if (button == 8) {
        c_.graph_period = (c_.graph_period + 1) % 3;
        sound(kSfxSelectX);
    } else {
        c_.state = 2;
        sound(kSfxNo4);
    }
}

} // namespace charts_detail

} // namespace th
