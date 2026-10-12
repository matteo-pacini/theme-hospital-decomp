#pragma once

// What the charts' files share: the tables of coordinates and buttons, the
// sounds, the helpers that draw, and Frame, the open chart's frame, whose
// members charts*.cpp define by chart. Private to src/core/ui/charts*.cpp.

#include "core/ui/charts.h"

#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/text.h"
#include "core/ui/pie.h"
#include "core/world/game.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>

namespace th
{

// Names the charts' files share. An inline namespace, so that none of them
// clashes with a same-named helper of another file.
inline namespace charts_detail
{

// play_sound's sfx numbers (include/enums.h).
constexpr uint8_t kSfxWrong2 = 0x08;
constexpr uint8_t kSfxNo4 = 0x42;
constexpr uint8_t kSfxSelectX = 0x43;
constexpr uint8_t kSfxHighlight5 = 0x44;
constexpr uint8_t kSfxPageTurn2 = 0xa8;
constexpr uint8_t kSfxBonusAll2 = 0xb4;
constexpr uint8_t kSfxBonus2 = 0xb8;

// Room types (ROOM_*), object kinds (OBJ_*) and illnesses (ILL_*) of
// include/enums.h.
constexpr uint8_t kRoomTheatre = 10;
constexpr uint8_t kRoomResearch = 28;
constexpr uint8_t kObjPharmacyCabinet = 39;
constexpr uint8_t kObjAutoAutopsy = 55;
constexpr int kFirstDiagnosis = 0x24;
constexpr int kIllGeneralPractice = 1;
constexpr int kIllAutopsy = 44;

// ScreenBoxes (src/scoords.cpp), entries 0 to 117: rectangles, read by the
// screens as their x, y, w, h (for some, h is a sprite).
constexpr Rect kBoxes[] = {
    {249, 70, 113, 385},  {343, 105, 3, 3},    {50, 431, 89, 0},    {50, 431, 37, 0},    {46, 30, 59, 27},
    {127, 32, 225, 27},   {376, 34, 59, 25},   {453, 34, 57, 25},   {530, 34, 65, 23},   {378, 413, 134, 25},
    {531, 413, 79, 25},   {61, 106, 143, 23},  {61, 171, 143, 23},  {61, 236, 143, 23},  {61, 301, 143, 23},
    {431, 98, 143, 23},   {431, 129, 143, 23}, {431, 189, 143, 23}, {431, 249, 143, 23}, {431, 309, 110, 23},
    {431, 338, 110, 23},  {61, 135, 143, 23},  {61, 200, 143, 23},  {80, 265, 105, 23},  {80, 330, 105, 23},
    {431, 158, 111, 23},  {431, 218, 111, 23}, {431, 278, 111, 23}, {548, 309, 42, 23},  {548, 338, 42, 23},
    {466, 260, 120, 100}, {246, 64, 152, 304}, {161, 75, 317, 30},  {161, 94, 300, 30},  {161, 184, 300, 30},
    {161, 259, 300, 30},  {161, 374, 300, 30}, {129, 119, 300, 15}, {129, 135, 300, 15}, {124, 210, 300, 15},
    {149, 285, 251, 15},  {370, 59, 110, 33},  {370, 100, 110, 33}, {370, 141, 110, 33}, {370, 183, 110, 33},
    {370, 224, 110, 33},  {346, 355, 25, 276}, {500, 39, 85, 32},   {500, 91, 85, 32},   {500, 143, 85, 32},
    {500, 195, 85, 32},   {500, 247, 85, 32},  {500, 299, 85, 32},  {500, 352, 85, 32},  {500, 405, 85, 32},
    {246, 71, 166, 20},   {246, 96, 166, 20},  {246, 121, 166, 20}, {246, 146, 166, 20}, {290, 287, 282, 53},
    {606, 447, 26, 26},   {323, 26, 95, 19},   {428, 26, 95, 19},   {532, 26, 95, 19},   {236, 397, 0, 12},
    {259, 397, 0, 12},    {288, 397, 0, 12},   {188, 408, 0, 20},   {228, 408, 0, 21},   {268, 408, 0, 22},
    {178, 387, 40, 13},   {351, 92, 40, 16},   {351, 119, 40, 16},  {351, 146, 40, 16},  {351, 173, 40, 16},
    {351, 200, 40, 16},   {351, 227, 40, 16},  {351, 254, 40, 16},  {351, 281, 40, 16},  {351, 308, 40, 16},
    {351, 335, 40, 16},   {456, 92, 40, 15},   {456, 119, 40, 15},  {456, 146, 40, 15},  {456, 173, 40, 15},
    {456, 200, 40, 15},   {456, 227, 40, 15},  {456, 254, 40, 15},  {456, 281, 40, 15},  {456, 308, 40, 15},
    {456, 335, 40, 15},   {560, 92, 40, 14},   {560, 119, 40, 14},  {560, 146, 40, 14},  {560, 173, 40, 14},
    {560, 200, 40, 14},   {560, 227, 40, 14},  {560, 254, 40, 14},  {560, 281, 40, 14},  {560, 308, 40, 14},
    {560, 335, 40, 14},   {351, 59, 40, 16},   {456, 59, 40, 15},   {560, 59, 40, 14},   {49, 82, 581, 27},
    {49, 109, 581, 27},   {49, 136, 581, 27},  {49, 163, 581, 27},  {49, 190, 581, 27},  {49, 217, 581, 27},
    {49, 244, 581, 27},   {49, 271, 581, 27},  {49, 298, 581, 27},  {49, 325, 581, 27},  {21, 167, 11, 84},
    {8, 39, 25, 0},       {225, 367, 85, 37},  {148, 403, 162, 56}};

// ScreenCoords (src/scoords.cpp).
constexpr Point kCoords[] = {
    {407, 71},  {407, 88},  {407, 105}, {407, 122}, {407, 139}, {407, 156}, {407, 173}, {407, 190}, {407, 230},
    {407, 268}, {407, 285}, {407, 302}, {407, 319}, {407, 336}, {407, 353}, {407, 370}, {407, 387}, {0, -1},
    {0, 44},    {0, 88},    {0, 132},   {0, 176},   {0, 221},   {0, 22},    {0, 67},    {0, 111},   {0, 155},
    {0, 199},   {0, 244},   {244, 398}, {246, 353}, {307, 352}, {336, 352}, {317, 360}, {140, 37},  {140, 89},
    {140, 141}, {140, 193}, {140, 246}, {90, 55},   {90, 108},  {90, 157},  {90, 212},  {90, 268},  {100, 357},
    {101, 319}, {228, 435}, {228, 451}, {404, 435}, {404, 451}, {299, 435}, {299, 451}, {470, 435}, {470, 451},
    {13, 0},    {49, 431},  {32, 100},  {32, 163},  {32, 226},  {32, 287},  {0, 0},     {196, 100}, {196, 163},
    {196, 226}, {196, 287}, {0, 0},     {191, 393}, {191, 393}, {327, 394}, {513, 393}, {14, 367},  {0, 4},
    {46, 74},   {127, 74},  {376, 74},  {453, 74},  {530, 74},  {530, 421}, {417, 150}, {298, 171}, {295, 164},
    {303, 199}, {348, 379}, {395, 379}, {2, 2},     {480, 362}, {2, 310},   {172, 69},  {172, 110}, {172, 151},
    {172, 193}, {172, 234}, {172, 95},  {172, 136}, {172, 177}, {172, 218}, {172, 259}, {407, 69},  {407, 110},
    {407, 151}, {407, 193}, {407, 234}, {240, 288}, {408, 288}, {270, 73},  {270, 98},  {270, 123}, {270, 148},
    {456, 193}, {456, 223}, {456, 254}, {347, 44},  {451, 70},  {263, 172}, {56, 90},   {56, 117},  {56, 144},
    {56, 171},  {56, 199},  {56, 226},  {56, 253},  {56, 280},  {56, 307},  {56, 334},  {86, 90},   {86, 117},
    {86, 144},  {86, 171},  {86, 199},  {86, 226},  {86, 253},  {86, 280},  {86, 307},  {86, 334},  {240, 90},
    {240, 117}, {240, 144}, {240, 171}, {240, 199}, {240, 226}, {240, 253}, {240, 280}, {240, 307}, {240, 334},
    {77, 54},   {143, 54},  {209, 54},  {275, 54},  {41, 0},    {380, 433}, {380, 387}, {137, 98},  {327, 204},
    {175, 310}, {28, 87},   {28, 202},  {28, 317},  {537, 87},  {537, 202}, {537, 317}, {206, 87},  {206, 200},
    {206, 304}, {206, 161}, {206, 233}, {206, 321}, {301, 343}, {0, 0},     {16, 10},   {430, 154}, {460, 268},
    {230, 420}, {28, 198},  {245, 198}, {447, 198}, {28, 437},  {430, 437}, {230, 410}, {0, 2},     {18, 23},
    {18, 22},   {18, 23},   {16, 23},   {18, 23},   {19, 23},   {18, 23},   {18, 23},   {18, 23},   {18, 23},
    {18, 23},   {595, 435}, {211, 156}, {211, 181}, {211, 206}, {211, 231}, {211, 256}, {211, 281}, {211, 306},
    {211, 331}, {211, 356}, {211, 381}, {216, 131}, {258, 131}, {362, 131}, {0, 0},     {316, 103}};

// screenbuttons (src/scoords.cpp), entries 0 to 58: the screens' buttons
// (hotspot x, y, width, height) and the sprite they are drawn pressed with
// and where. Each screen's list ends with an empty entry: the bank statement
// from 0, the bank manager from 3, policy from 10, the casebook from 0x12,
// research from 0x19, graphs from 0x25, the staff list from 0x30.
struct ScreenButton {
    int x, y, w, h;
    int sprite_x, sprite_y, sprite;
};
constexpr ScreenButton kButtons[] = {
    {601, 442, 26, 26, 601, 442, 1},
    {0, 0, 600, 440, 0, 0, 0},
    {},
    {50, 265, 20, 20, 50, 265, 5},
    {191, 265, 20, 20, 192, 265, 6},
    {547, 157, 40, 20, 547, 157, 3},
    {547, 217, 40, 20, 547, 217, 3},
    {547, 277, 40, 20, 547, 277, 3},
    {607, 448, 26, 26, 607, 448, 4},
    {},
    {348, 373, 48, 17, 0, 0, 0},
    {395, 373, 48, 17, 0, 0, 0},
    {607, 447, 26, 26, 0, 0, 0},
    {},
    {},
    {},
    {},
    {},
    {355, 134, 22, 22, 354, 133, 5},
    {238, 134, 22, 22, 237, 133, 4},
    {445, 33, 62, 42, 440, 29, 1},
    {445, 403, 62, 42, 437, 394, 2},
    {607, 448, 26, 26, 607, 448, 3},
    {234, 398, 140, 16, 0, 0, 0},
    {},
    {608, 449, 24, 24, 608, 449, 4},
    {373, 63, 28, 28, 373, 63, 1},
    {373, 104, 28, 28, 373, 104, 1},
    {373, 145, 28, 28, 373, 145, 1},
    {373, 186, 28, 28, 373, 186, 1},
    {373, 227, 28, 28, 373, 227, 1},
    {450, 63, 28, 28, 450, 63, 2},
    {450, 104, 28, 28, 450, 104, 2},
    {450, 145, 28, 28, 450, 145, 2},
    {450, 186, 28, 28, 450, 186, 2},
    {450, 227, 28, 28, 450, 227, 2},
    {},
    {591, 35, 42, 42, 590, 34, 1},
    {591, 87, 42, 42, 590, 86, 1},
    {591, 139, 42, 42, 590, 138, 1},
    {591, 191, 42, 42, 590, 190, 1},
    {591, 244, 42, 42, 590, 243, 1},
    {591, 296, 42, 42, 590, 295, 1},
    {591, 348, 42, 42, 590, 347, 1},
    {591, 401, 42, 42, 590, 400, 1},
    {371, 384, 65, 26, 371, 384, 2},
    {63, 384, 26, 26, 63, 384, 3},
    {},
    {53, 15, 57, 59, 53, 15, 1},
    {119, 15, 57, 59, 119, 15, 1},
    {251, 15, 57, 59, 251, 15, 1},
    {185, 15, 57, 59, 185, 15, 1},
    {439, 372, 45, 87, 438, 372, 9},
    {319, 418, 113, 40, 319, 418, 8},
    {319, 372, 113, 40, 319, 372, 7},
    {603, 443, 22, 23, 603, 443, 10},
    {12, 86, 33, 73, 12, 86, 2},
    {12, 274, 33, 73, 12, 274, 3},
    {},
};
constexpr int kStatButtons = 0, kBankButtons = 3, kPolicyButtons = 10, kDrugButtons = 0x12, kResearchButtons = 0x19,
              kGraphButtons = 0x25, kSlistButtons = 0x30;

// The staff list's view of the selected staff member and their face
// (screenlistpos, src/scoords.cpp, as SlistArea).
constexpr Rect kSlistView = {496, 372, 581 - 496, 456 - 372};
constexpr Point kSlistFace = {65, 455};

// expertise[] (src/array.cpp) columns the casebook reads: the staff skills
// needed, the object that cures, what research gives (1 a drug, 2 a
// treatment machine, 3 a diagnosis machine, 0 nothing), the outcome and the
// room.
struct ExpertiseRow {
    uint16_t staff_mask;
    uint8_t kind, research_type, outcome, category;
};
constexpr std::array<ExpertiseRow, kIllnessCount> kExpertise = {{
    {0, 0, 0, 0, 0},    {16, 6, 0, 4, 7},   {16, 9, 2, 2, 17},  {16, 46, 2, 2, 23}, {64, 18, 0, 2, 8},
    {4, 39, 1, 2, 11},  {16, 54, 2, 2, 30}, {16, 26, 2, 2, 20}, {16, 23, 2, 2, 18}, {4, 24, 2, 2, 21},
    {16, 25, 2, 2, 19}, {4, 39, 1, 2, 11},  {16, 47, 2, 2, 24}, {4, 39, 1, 2, 11},  {32, 30, 2, 2, 10},
    {4, 39, 1, 2, 11},  {4, 39, 1, 2, 11},  {4, 39, 1, 2, 11},  {32, 30, 2, 2, 10}, {32, 30, 2, 2, 10},
    {32, 30, 2, 2, 10}, {32, 30, 2, 2, 10}, {64, 18, 0, 2, 8},  {64, 18, 0, 2, 8},  {4, 39, 1, 2, 11},
    {4, 39, 1, 2, 11},  {64, 18, 0, 2, 8},  {64, 18, 0, 2, 8},  {4, 39, 1, 2, 11},  {4, 39, 1, 2, 11},
    {32, 30, 2, 2, 10}, {64, 18, 0, 2, 8},  {4, 39, 1, 2, 11},  {4, 39, 1, 2, 11},  {32, 30, 2, 2, 10},
    {32, 30, 2, 2, 10}, {16, 14, 3, 1, 13}, {16, 42, 3, 1, 15}, {16, 13, 3, 1, 12}, {16, 27, 3, 1, 16},
    {16, 22, 3, 1, 14}, {16, 20, 0, 1, 27}, {4, 8, 0, 5, 9},    {64, 18, 0, 1, 8},  {256, 55, 0, 2, 28},
    {0, 41, 2, 0, 0},   {0, 40, 2, 0, 0},
}};

// The status screen's pie slices (slices, src/overscr.cpp): their colours
// (top, start side, end side, rim, edges) and the labels' distance.
struct SliceColours {
    uint8_t top, side1, side2, rim, edge;
};
constexpr std::array<SliceColours, 4> kSlices = {{
    {0xca, 0xce, 0xc6, 0xce, 0xce},
    {0xfd, 0xdc, 0xd8, 0xdc, 0xd8},
    {0xfb, 0xeb, 0xf9, 0xeb, 0xf9},
    {0xfc, 0xe4, 0xe7, 0xe4, 0xe7},
}};
constexpr int kSliceTextOffset = 0xaf;

// The interface's colours (setup_colors, src/utils.cpp) in a palette.
struct Colours {
    uint8_t black, dark_blue, green, orange, light_red, red, purple, blue, salmon;
};

Colours setup_colours(const Palette &p);

// A hospital's figures the status screen's criteria measure (AllHosps).
struct HospitalFigures {
    int reputation = 500;
    int32_t balance = 0;
    int32_t loan = 0;
    int32_t value = 0;
    int visitors = 0, diagnosed = 0, cures = 0, deaths = 0;
};

// A string of the game's sprintf, for the formats the charts use.
template <typename... Args> std::string format(const char *fmt, Args... args)
{
    char buf[256];
    std::snprintf(buf, sizeof buf, fmt, args...);
    return buf;
}

bool over(int mx, int my, int x, int y, int w, int h);
uint8_t find_colour(const Palette &palette, int r, int g, int b);
std::string conv_string(std::string_view s, int mode);
void draw_box(Surface &dst, int x, int y, int w, int h, uint8_t colour);
void plot(Surface &dst, int x, int y, uint8_t colour);
void draw_line(Surface &dst, int x1, int y1, int x2, int y2, uint8_t colour);
void draw_triangle(Surface &dst, Point p1, Point p2, Point p3, uint8_t colour);
bool has_room(const World &world, uint8_t type);

// One game frame of the open chart: the screen's display function, as
// do_charts (src/game.cpp) runs it, with the mouse as lbDisplay holds it.
class Frame
{
  public:
    Frame(Charts &c, const UiMouse &m, const ChartKeys &k, const Game &g, const HospitalSettings &s, ChartsResult &r);

    void run();

  private:
    void draw_help();
    static int chart_help_group(int chart);

    void load_pic();
    void copy_screen();
    Sprite sprite(int index) const { return c_.pictures[size_t(c_.chart)].sprites.at(size_t(index)); }
    void reqspr(int x, int y, int index) { draw_sprite(dst_, x, y, sprite(index)); }
    const Font &font(int slot) const { return c_.fonts[size_t(slot)]; }
    static int line_height(const Font &f) { return f.glyph(65).height; }
    void text(int slot, Rect window, Point at, std::string_view s, unsigned flags = 0, uint8_t colour = 0);
    void text(int slot, int x, int y, std::string_view s, unsigned flags = 0, uint8_t colour = 0);
    std::string_view str(TextSection section, int index) const { return text_.at(section, index); }
    void sound(uint8_t sfx) { r_.sounds.push_back(sfx); }
    Colours colours() const { return setup_colours(c_.palette); }
    const Thing &thing(int id) const { return game_.world.things[size_t(id)]; }

    bool exit_quick_requester();
    void chart_key(int chart);
    void return_from_screen() { r_.event = {EV_RETURN_FROM_SCREEN, 0}; }

    // ---- Bank statement (src/finance.cpp) ----

    void bank_statement();

    void bank_text(int f);

    void bank_text_line(int f, int row, const BankEntry &e, int32_t balance);

    void process_stat_panel();

    void process_stat_button(int button);

    // ---- Bank manager (src/manager.cpp) ----

    void display_manager();

    void display_manager_text();

    void display_manager_claims(const std::array<int32_t, 12> &claims);

    void display_manager_face();

    int bank_button_sprite(int i) const;

    void process_bank_panel();

    void process_bank_button(int button);

    // ---- Policy (src/policy.cpp) ----

    void display_policy();

    void process_policy_panel();

    void process_policy_sliders();

    void draw_policy_sliders();

    void drag_policy_slider();

    // ---- Research (src/rschscrn.cpp) ----

    void display_research();

    void draw_research_bar(int i);

    void draw_research_text();

    void reset_repeat();
    // The price and research buttons act at once, then every frame once
    // held for 250 ms, by a step that grows while held.
    bool repeat_due() const { return c_.clock_ms - c_.repeat_time > 0xfa || c_.repeat_step == 0x100; }
    int next_repeat_step();

    void process_res_panel();

    void process_res_button(int button);

    // ---- Graphs (src/graph.cpp) ----

    void draw_graphs(int x, int y, int h, int step);

    static constexpr int kGraphFont = 0xb;
    // The graphs' origin (now, at the bottom right), height and step.
    struct GraphArea {
        int x, y, h, step;
    };
    // A graph's value at its right end, kept in screen order.
    struct GraphLabel {
        int graph, y;
        int32_t data;
    };

    bool graph_shown(int n) const;

    void draw_graphs_keys(const std::array<uint8_t, 8> &colour);

    void draw_graphs_line(int graph, GraphArea area, uint8_t colour, std::vector<GraphLabel> &labels);

    void spread_graph_labels(std::vector<GraphLabel> &labels) const;

    void draw_graphs_dates(int x, int y, int step, uint8_t black);

    void process_graph_panel();

    void process_graph_button(int button);

    // ---- Casebook (src/drugs.cpp) ----

    bool casebook_lists(int i) const;

    int get_next_illness(int forward, int from) const;

    void display_drugs();

    void display_drugs_figures(int illness);

    std::string display_drugs_figure(int i, int illness);

    std::array<int, 18> display_drugs_entries() const;

    void display_drugs_list();

    void drug_effect(std::string_view s) { text(0x2e, {0x134, kCoords[0x21].y, 0x1c, 0x32}, {0, 0}, s, kTextCentre); }
    void cure_sprite(int index) { reqspr(kCoords[0x1f].x, kCoords[0x1f].y, index); }

    void display_drugs_cure(int illness);

    void display_drugs_missing_cure(const ExpertiseRow &def);

    int button_under_mouse(int first) const;

    void process_drug_panel();

    void price(bool up);

    void select_next(int forward);

    void process_drug_keys();

    void process_drug_button(int action);

    // ---- Staff list (src/slist.cpp) ----

    void display_slist();

    void open_slist_on(uint16_t id);

    void draw_slist();

    void slist_filter(int filter);

    void slist_keys();

    void slist_key_down();

    void slist_key_up();

    int get_slist_staff(int dir);

    bool staff_on_page(int page) const;

    void draw_gauge(int value, const Rect &box, int max, int min, int range, bool up);

    void draw_slist_staff(int selected);

    void draw_slist_staff_line(int row, int listed, int id, bool selected, uint8_t frame);

    void draw_face(const Thing &s, int x, int y);

    void draw_slist_training_bar(int x, int y, int value, int max);

    void draw_slist_window(int id);

    void draw_slist_overall();

    void process_slist_panel();

    void process_slist_button(int button);

    void draw_slist_nums();

    void draw_slist_button_overlays(const Thing &s);

    // ---- Status (src/overscr.cpp) ----

    HospitalFigures figures(int hospital) const;

    int market_share(size_t hospital) const;

    void display_peep_overview();

    void process_overview_panel();

    void draw_overview_screen();

    void draw_overview_patients();

    void display_overview_bars();

    static constexpr int kPieX = 0x1fa, kPieY = 0x78, kPieRx = 0x40, kPieRy = 0x1e, kPieH = 0x14;

    void draw_piechart();

    std::array<PieSlice, 4> calculate_piechart();

    void sector_render(const PieDraw &draw, const SliceColours &c);

    void sector_render_top(Point o, int start, int end, const SliceColours &c);

    void sector_render_edges(Point o, Point p, bool faces, uint8_t edge);

    Charts &c_;
    const ChartKeys &keys_;
    const Game &game_;
    const HospitalSettings &hs_;
    ChartsResult &r_;
    Surface &dst_;
    const LanguageText &text_;
    int mx_ = 0, my_ = 0;
    bool left_ = false, mleft_ = false, rleft_ = false, mright_ = false, rright_ = false;
    bool inkey_escape_ = false;
};

} // namespace charts_detail

} // namespace th
