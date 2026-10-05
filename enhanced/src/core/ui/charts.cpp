#include "core/ui/charts.h"

#include "core/data/balance.h"
#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/panel.h"
#include "core/render/text.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/staff.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <numbers>
#include <string_view>

namespace th
{

namespace
{

constexpr double kGameFrame = 0.04; // limit_framerate: 40 timer ticks

// play_sound's sfx numbers (include/enums.h).
constexpr uint8_t kSfxWrong2 = 0x08;
constexpr uint8_t kSfxNo4 = 0x42;
constexpr uint8_t kSfxSelectX = 0x43;
constexpr uint8_t kSfxHighlight5 = 0x44;
constexpr uint8_t kSfxResearch = 0x75;
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

// Each chart's picture and sprites (hiresscreens and
// hiresscreen_sprites_data, src/files.cpp, at SCR_STATEMENT, SCR_DRUGS,
// SCR_POLICY, SCR_RESEARCH, SCR_GRAPHS, SCR_STAFF_LIST, SCR_BANK_MANAGER and
// SCR_OVERVIEW).
constexpr std::array<std::string_view, kChartCount> kPictures = {"stat01v",  "drugn01v", "pol01v",  "res01v",
                                                                 "graph01v", "staff01v", "bank01v", "rep01v"};
constexpr std::array<std::string_view, kChartCount> kSprites = {"stat02v",  "drugn02v", "pol02v",  "res02v",
                                                                "graph02v", "staff02v", "bank02v", "rep02v"};

// The font cache slots the charts draw with.
constexpr std::array<int, 16> kChartFonts = {1, 2, 3, 7, 8, 9, 0xb, 0xd, 0xe, 0xf, 0x1b, 0x1c, 0x24, 0x25, 0x26, 0x2e};

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

// The faces file's layout (HiresCoords[162-165]) and draw_face's frame.
constexpr int kFaceW = 65, kHeadH = 29, kBodyH = 24, kLegsH = 22, kFaceH = kHeadH + kBodyH + kLegsH;
constexpr int kFaceBorder = 3, kFaceParts = 18;
constexpr uint8_t kFaceBorderColour = 0x9c;

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

// The bank manager's face animations (bank_manager_face1-3,
// src/tables.cpp): sprites by frame from 1, ended by -1.
constexpr int8_t kManagerFace1[] = {0, 7, 7, 8, 8, 8, 8, 7, 7, -1};
constexpr int8_t kManagerFace2[] = {0, 9, 9, 9, 10, 10, 10, 11, 11, 11, 11, 11, 11, 10, 10, 10, 9, 9, 9, -1};
constexpr int8_t kManagerFace3[] = {0,  12, 12, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
                                    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
                                    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 13, 13, 12, 12, -1};

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

// The graphs' statistics in the order of their buttons (graph[], src/tables.cpp,
// none inverted, each its own colour), and the history row each plots
// (draw_graphs).
constexpr std::array<int, 8> kGraphRows = {kHistoryIncome,   kHistoryExpenditure, kHistoryWages,  kHistoryBalance,
                                           kHistoryVisitors, kHistoryCures,       kHistoryDeaths, kHistoryReputation};

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

// The interface's colours (setup_colors, src/utils.cpp) in a palette.
struct Colours {
    uint8_t black, dark_blue, green, orange, light_red, red, purple, blue, salmon;
};
Colours setup_colours(const Palette &p)
{
    return {find_colour(p, 0, 0, 0),          find_colour(p, 0, 0, 0x10),       find_colour(p, 0, 0x3f, 0),
            find_colour(p, 0x3f, 0x30, 0x17), find_colour(p, 0x3f, 0x17, 0x17), find_colour(p, 0x3f, 0, 0),
            find_colour(p, 0x35, 0x1f, 0x42), find_colour(p, 0, 0x26, 0x56),    find_colour(p, 0x3f, 0x1e, 0x1e)};
}

// saferand (src/rand.cpp).
uint16_t saferand(uint32_t &seed)
{
    seed = seed * 0x24a1 + 0x24df;
    seed = (seed >> 13) | (seed << 19);
    return uint16_t(seed);
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

// A string of the game's sprintf, for the formats the charts use.
template <typename... Args> std::string format(const char *fmt, Args... args)
{
    char buf[256];
    std::snprintf(buf, sizeof buf, fmt, args...);
    return buf;
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
void draw_triangle(Surface &dst, int x1, int y1, int x2, int y2, int x3, int y3, uint8_t colour)
{
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

// lbSinTable (src/lib/bf1/gtrig.cpp): sines of 2048ths of a turn, 16.16
// truncated, which it holds for a turn and a quarter so that cosines are
// read at +0x200; and fixmul, the 16.16 product.
int32_t sin_table(int i)
{
    return int32_t(std::sin(double(i) * 2 * std::numbers::pi / 2048) * 65536);
}
int32_t fixmul(int32_t a, int32_t b)
{
    return int32_t((int64_t(a) * b) >> 16);
}

// A full-screen picture: 640 x 480 palette indices and its VGA palette.
bool load_picture(const std::filesystem::path &dir, std::string_view name, Charts::Picture &out, std::string *error)
{
    auto dat = load_data_file(dir, "qdata/" + std::string(name) + ".dat", error);
    if (!dat)
        return false;
    auto pal = load_data_file(dir, "qdata/" + std::string(name) + ".pal", error);
    if (!pal)
        return false;
    if (dat->size() < size_t(kUiWidth) * kUiHeight || pal->size() < out.vga.size()) {
        if (error)
            *error = std::string(name) + ": not a full-screen picture";
        return false;
    }
    out.pixels.resize(kUiWidth, kUiHeight);
    for (int y = 0; y < kUiHeight; y++)
        std::memcpy(out.pixels.row(y), dat->data() + size_t(y) * kUiWidth, kUiWidth);
    std::memcpy(out.vga.data(), pal->data(), out.vga.size());
    palette_from_vga(out.vga, out.palette);
    return true;
}

bool load_sheet(const std::filesystem::path &dir, std::string_view name, SpriteSheet &sheet, std::string *error)
{
    auto tab = load_data_file(dir, "qdata/" + std::string(name) + ".tab", error);
    auto dat = tab ? load_data_file(dir, "qdata/" + std::string(name) + ".dat", error) : std::nullopt;
    if (!dat)
        return false;
    if (!sheet.load(std::move(*tab), std::move(*dat), SpriteFormat::Req)) {
        if (error)
            *error = std::string(name) + ": not a sprite table";
        return false;
    }
    return true;
}

bool has_room(const World &world, uint8_t type)
{
    return std::any_of(world.rooms.begin() + 1, world.rooms.end(),
                       [&](const RoomInfo &r) { return r.type == type && r.owner == 0; });
}

// The staff of the kinds the staff list's `filter` shows: the player's,
// not applicants (the game's test in get_slist_staff and its kin).
bool slist_shows(const Thing &t, int filter)
{
    if (t.type != kThingStaff || t.owner != 0 || t.state == kStaffApplicant)
        return false;
    return ((t.subtype == kStaffDoctor || t.subtype == kStaffSurgeon) && (filter & 1)) ||
           (t.subtype == kStaffNurse && (filter & 2)) || (t.subtype == kStaffReceptionist && (filter & 8)) ||
           (t.subtype == kStaffHandyman && (filter & 4));
}

// A hospital's figures the status screen's criteria measure (AllHosps).
struct HospitalFigures {
    int reputation = 500;
    int32_t balance = 0;
    int32_t loan = 0;
    int32_t value = 0;
    int visitors = 0, diagnosed = 0, cures = 0, deaths = 0;
};

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
            if (c.met) {
                sum += 100;
                continue;
            }
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
            sum += std::clamp(pct, 0, 100);
        }
        if (count && sum / count > best) {
            best = sum / count;
            best_group = group;
        }
    }
    return best_group;
}

// One game frame of the open chart: the screen's display function, as
// do_charts (src/game.cpp) runs it, with the mouse as lbDisplay holds it.
class Frame
{
  public:
    Frame(Charts &c, const UiMouse &m, const ChartKeys &k, const Game &g, const HospitalSettings &s, ChartsResult &r)
        : c_(c), keys_(k), game_(g), hs_(s), r_(r), dst_(c.surface), text_(*c.text)
    {
        mx_ = m.x;
        my_ = m.y;
        left_ = m.left_pressed;
        mleft_ = m.left_held;
        rleft_ = m.left_released;
        mright_ = m.right_held;
        rright_ = m.right_released;
        inkey_escape_ = k.escape || k.space;
    }

    void run()
    {
        switch (c_.chart) {
        case kChartStatement:
            bank_statement();
            break;
        case kChartCasebook:
            display_drugs();
            break;
        case kChartPolicy:
            display_policy();
            break;
        case kChartResearch:
            display_research();
            break;
        case kChartGraphs:
            draw_graphs(kBoxes[46].x, kBoxes[46].y, kBoxes[46].h, kBoxes[46].w);
            break;
        case kChartStaff:
            display_slist();
            break;
        case kChartBank:
            display_manager();
            break;
        case kChartStatus:
            display_peep_overview();
            break;
        default:
            r_.event = {EV_CLOSE_SCREEN};
            break;
        }
    }

  private:
    // load_ingame_pic: the chart's picture shown in its palette.
    void load_pic()
    {
        const Charts::Picture &p = c_.pictures[size_t(c_.chart)];
        c_.previous_palette = c_.palette;
        c_.palette = p.palette;
        copy_screen();
    }
    // copy_screen_to_wscreen: the picture redrawn.
    void copy_screen()
    {
        const Surface &src = c_.pictures[size_t(c_.chart)].pixels;
        for (int y = 0; y < kUiHeight; y++)
            std::memcpy(dst_.row(y), src.data() + size_t(y) * kUiWidth, kUiWidth);
    }
    Sprite sprite(int index) const { return c_.pictures[size_t(c_.chart)].sprites.at(size_t(index)); }
    void reqspr(int x, int y, int index) { draw_sprite(dst_, x, y, sprite(index)); }
    const Font &font(int slot) const { return c_.fonts[size_t(slot)]; }
    static int line_height(const Font &f) { return f.glyph(65).height; }
    // LbTextSetWindow then LbTextDraw: `s` at (x, y) in `window`, clipped
    // to it.
    void text(int slot, Rect window, int x, int y, std::string_view s, unsigned flags = 0, uint8_t colour = 0)
    {
        draw_text(dst_, font(slot), {window.x, window.y, window.w}, x, y, s, flags, window, colour);
    }
    void text(int slot, int x, int y, std::string_view s, unsigned flags = 0, uint8_t colour = 0)
    {
        text(slot, {0, 0, kUiWidth, kUiHeight}, x, y, s, flags, colour);
    }
    std::string_view str(TextSection section, int index) const { return text_.at(section, index); }
    void sound(uint8_t sfx) { r_.sounds.push_back(sfx); }
    Colours colours() const { return setup_colours(c_.palette); }
    const Thing &thing(int id) const { return game_.world.things[size_t(id)]; }

    // exit_quick_requester (src/finance.cpp): Escape or Space leaves.
    bool exit_quick_requester()
    {
        if (!inkey_escape_)
            return false;
        inkey_escape_ = false;
        sound(kSfxNo4);
        return true;
    }
    // process_keys for F1-F8 within a chart.
    void chart_key(int chart)
    {
        if (!game_.paused)
            r_.event = {EV_OPEN_SCREEN, uint16_t(chart)};
    }
    void return_from_screen() { r_.event = {EV_RETURN_FROM_SCREEN, 0}; }

    // ---- Bank statement (src/finance.cpp) ----

    void bank_statement()
    {
        switch (c_.state) {
        case 0:
            load_pic();
            c_.state = 1;
            break;
        case 1:
            copy_screen();
            process_stat_panel();
            break;
        case 2:
            return_from_screen();
            break;
        }
        const int f = 0x1c;
        for (int i = 0; i < 6; i++)
            text(f, kBoxes[i + 4], kCoords[71].x, kCoords[71].y * 2, str(TextSection::Stat, i), kTextCentre);
        const Finance &fin = game_.finance;
        int line_h = line_height(font(f));
        // banktext (src/finance.cpp): the name, the lines from y 40 to 405,
        // the closing balance.
        constexpr int kName = 32, kNameY = 8, kLinesY = 40, kEndY = 405;
        const std::string &name = c_.hospital_names[0];
        text(f, kName, kNameY, name.size() > 9 ? std::string_view(name).substr(9) : std::string_view{});
        int row = 0;
        int count = fin.entry_count;
        int first = line_h ? (kEndY - kLinesY) / line_h : 0;
        first = count - first > 0 ? count - first : 0;
        int32_t balance = fin.statement_balance;
        for (int k = 0; k < first; k++)
            balance += fin.entries[size_t(k)].amount;
        for (; first < count && row < kEndY - line_h; first++) {
            if (row + kCoords[72].y > kCoords[72].y + line_h * 0x1b)
                break;
            const BankEntry &e = fin.entries[size_t(first)];
            text(f, kCoords[72].x, row + kCoords[72].y,
                 format("%d %s", e.day, std::string(text_.month(e.month)).c_str()));
            std::string what(str(TextSection::Finance, e.type));
            if (e.type == kTxInsurance)
                what += " " + std::string(str(TextSection::InsuranceNames, e.extra));
            else if (e.type == kTxTreat || e.type == kTxFinalTreat || e.type == kTxCureTreatment ||
                     e.type == kTxAdvance)
                what += " " + std::string(str(TextSection::Illness, e.extra));
            text(f, kCoords[73].x, row + kCoords[73].y, what);
            if (e.amount < 0)
                text(f, kCoords[74].x, row + kCoords[74].y, format("$ %7d", -e.amount));
            else if (e.amount != 0)
                text(f, kCoords[75].x, row + kCoords[75].y, format("$ %7d", e.amount));
            balance += e.amount;
            text(f, kCoords[76].x, row + kCoords[76].y, format("$ %7d", balance));
            row += line_h;
        }
        text(f, kCoords[77].x, kCoords[77].y, format("$ %7d", balance));
        if (exit_quick_requester())
            c_.state = 2;
    }

    void process_stat_panel()
    {
        if (rleft_) {
            for (int i = kStatButtons; kButtons[i].w && kButtons[i].h; i++) {
                const ScreenButton &b = kButtons[i];
                if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                    process_stat_button(i - kStatButtons);
                    reqspr(b.sprite_x, b.sprite_y, b.sprite);
                }
            }
        }
        if (mleft_) {
            for (int i = kStatButtons; kButtons[i].w && kButtons[i].h; i++) {
                const ScreenButton &b = kButtons[i];
                if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                    reqspr(b.sprite_x, b.sprite_y, b.sprite);
                    break;
                }
            }
        }
        rleft_ = false;
    }

    void process_stat_button(int button)
    {
        if (button == 0) {
            c_.state = 2;
            sound(kSfxNo4);
        } else if (button == 1) {
            sound(kSfxSelectX);
            chart_key(kChartBank);
        }
    }

    // ---- Bank manager (src/manager.cpp) ----

    void display_manager()
    {
        switch (c_.state) {
        case 0:
            load_pic();
            // Looked up before the chart's palette is faded in.
            c_.line_colour = find_colour(c_.previous_palette, 0x15, 0x15, 0x18);
            c_.line_shadow_colour = find_colour(c_.previous_palette, 0x2c, 0x2c, 0x2c);
            c_.state = 1;
            c_.value = 0;
            display_manager_text();
            break;
        case 1:
            copy_screen();
            display_manager_text();
            display_manager_face();
            process_bank_panel();
            break;
        case 2:
            return_from_screen();
            break;
        }
        if (rleft_) {
            const Rect &b = kBoxes[31];
            if (over(mx_, my_, b.x, b.y, b.w, b.h))
                chart_key(kChartStatement);
            if (over(mx_, my_, 0x15b, 0x198, 0x87, 0x45))
                r_.town_map = true;
        }
        left_ = mleft_ = rleft_ = rright_ = false;
        if (exit_quick_requester())
            c_.state = 2;
    }

    void display_manager_text()
    {
        const int f = 0x1b;
        const Point at = kCoords[71];
        auto boxed = [&](int box, std::string_view s) { text(f, kBoxes[box], at.x, at.y, s, kTextCentre); };
        for (int i = 0; i < 5; i++)
            boxed(11 + i, str(TextSection::Bank, i));
        boxed(19, str(TextSection::Bank, 5));
        boxed(20, str(TextSection::Bank, 6));
        const Finance &fin = game_.finance;
        const Balance &bal = game_.settings();
        const Balance::Town &town = bal.towns[size_t(game_.level_number()) % bal.towns.size()];
        boxed(21, format("$ %d", fin.hospital_value));
        boxed(22, format("$ %d", game_.balance));
        boxed(23, format("$ %d", fin.loan));
        boxed(24, format("$ %d", int32_t(town.interest_rate) * fin.loan / 10000 / 12));
        for (int i = 0; i < 3; i++) {
            int32_t sum = 0;
            for (int32_t v : hs_.insurance_claims[size_t(i)])
                sum += v;
            boxed(25 + i, format("$ %d", sum));
        }
        boxed(28, format("%d.%d%%", town.inflation_rate / 100, town.inflation_rate % 100));
        boxed(29, format("%d.%d%%", town.interest_rate / 100, town.interest_rate % 100));
        if (c_.value) {
            reqspr(kCoords[78].x, kCoords[78].y, 1);
            c_.insurance_button_sprites = {0, 0, 2};
            size_t company = size_t(c_.value - 1);
            boxed(16, str(TextSection::InsuranceNames, hs_.insurers[company]));
            const auto &claims = hs_.insurance_claims[company];
            int32_t max = 1;
            for (int32_t v : claims)
                max = std::max(max, v);
            uint8_t black = colours().black;
            if (max > 1)
                text(3, kCoords[169].x, kCoords[169].y, format("%d", max), kTextOneColour, black);
            const Rect &g = kBoxes[30];
            int month = game_.date.month;
            int x = g.x, y = g.y - claims[size_t(month)] * g.h / max;
            int i = month, end = 12;
            for (int pass = 0; pass < 2; pass++) {
                for (; i < end; i++) {
                    int px = x, py = y;
                    x += g.w / 12;
                    y = g.y - claims[size_t(i)] * g.h / max;
                    draw_line(dst_, px + 1, py + 1, x + 1, y + 1, c_.line_shadow_colour);
                    draw_line(dst_, px, py, x, y, c_.line_colour);
                }
                end = month;
                i = 0;
            }
            text(3, kCoords[170].x, kCoords[170].y, text_.month(month), kTextOneColour, black);
        } else {
            c_.insurance_button_sprites = {3, 3, 3};
            for (int i = 0; i < 3; i++)
                boxed(16 + i, str(TextSection::InsuranceNames, hs_.insurers[size_t(i)]));
        }
    }

    void display_manager_face()
    {
        static constexpr const int8_t *kFaces[3] = {kManagerFace1, kManagerFace2, kManagerFace3};
        static constexpr int kOdds[3] = {45, 300, 300};
        for (int i = 0; i < 3; i++)
            if (saferand(c_.safeseed) % kOdds[i] == 1 && c_.face_frame[size_t(i)] == 0)
                c_.face_frame[size_t(i)] = 1;
        for (int i = 0; i < 3; i++) {
            int &frame = c_.face_frame[size_t(i)];
            if (!frame)
                continue;
            reqspr(kCoords[79 + i].x, kCoords[79 + i].y, kFaces[i][frame]);
            frame++;
            if (kFaces[i][frame] == -1) {
                frame = 0;
                if (i == 0 && saferand(c_.safeseed) % 5 == 1)
                    frame = 1;
            }
        }
    }

    int bank_button_sprite(int i) const
    {
        // display_manager_text sets the sprites of screenbuttons 5 to 7.
        return i >= 5 && i <= 7 ? c_.insurance_button_sprites[size_t(i - 5)] : kButtons[i].sprite;
    }

    void process_bank_panel()
    {
        if (rleft_) {
            for (int i = kBankButtons; kButtons[i].w && kButtons[i].h; i++) {
                const ScreenButton &b = kButtons[i];
                if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                    process_bank_button(i - kBankButtons);
                    reqspr(b.sprite_x, b.sprite_y, bank_button_sprite(i));
                }
            }
        }
        if (mleft_) {
            for (int i = kBankButtons; kButtons[i].w && kButtons[i].h; i++) {
                const ScreenButton &b = kButtons[i];
                if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                    reqspr(b.sprite_x, b.sprite_y, bank_button_sprite(i));
                    break;
                }
            }
        }
    }

    void process_bank_button(int button)
    {
        const Finance &fin = game_.finance;
        switch (button) {
        case 0:
            if (game_.balance >= 5000 && fin.loan > 0) {
                r_.event = {EV_REPAY_LOAN};
                sound(kSfxSelectX);
            } else {
                sound(kSfxWrong2);
            }
            break;
        case 1:
            if (fin.loan <= fin.max_loan - 5000) {
                r_.event = {EV_TAKE_LOAN};
                sound(kSfxSelectX);
            } else {
                sound(kSfxWrong2);
            }
            break;
        case 2:
        case 3:
            if (!c_.value) {
                c_.value = button - 1;
                sound(kSfxSelectX);
            }
            break;
        case 4:
            c_.value = c_.value ? 0 : 3;
            sound(kSfxSelectX);
            break;
        case 5:
            c_.state = 2;
            sound(kSfxNo4);
            break;
        }
    }

    // ---- Policy (src/policy.cpp) ----

    void display_policy()
    {
        switch (c_.state) {
        case 0:
            load_pic();
            c_.state = 1;
            c_.value = 0;
            process_policy_sliders();
            left_ = mleft_ = rleft_ = false;
            break;
        case 1:
            copy_screen();
            process_policy_panel();
            process_policy_sliders();
            break;
        case 2:
            c_.state = 3;
            break;
        case 3:
            return_from_screen();
            break;
        }
        if (exit_quick_requester())
            c_.state = 2;
    }

    void process_policy_panel()
    {
        if (!rleft_)
            return;
        for (int i = kPolicyButtons; kButtons[i].w && kButtons[i].h; i++) {
            const ScreenButton &b = kButtons[i];
            if (!over(mx_, my_, b.x, b.y, b.w, b.h))
                continue;
            int action = i - kPolicyButtons;
            if (action == 0 || action == 1)
                r_.event = {EV_SET_POLICY, 5, 0, uint16_t(action == 0 ? 1 : 0)};
            else
                c_.state = 2;
            sound(kSfxSelectX);
        }
        rleft_ = false;
    }

    void process_policy_sliders()
    {
        for (int i = 0; i < 5; i++)
            text(7, kBoxes[0x20 + i], 0, 0, str(TextSection::PolicyNames, i), i == 0 ? kTextCentre : kTextWrap);
        struct Slider {
            int box, sprite, x;
        };
        const Slider sliders[4] = {
            {0x25, 2, std::clamp(hs_.guess_cure_threshold * 3, 0, 299)},
            {0x26, 1, std::clamp(hs_.send_home_threshold * 3, 0, 299)},
            {0x27, 3, std::clamp((hs_.diag_threshold - 1000) / 0x42 * 20, 0, 299)},
            {0x28, 3, std::clamp(hs_.rest_threshold / 0x27 * 10, 0, 0xfb)},
        };
        for (int i = 0; i < 4; i++) {
            const Slider &s = sliders[i];
            const Rect &b = kBoxes[s.box];
            Sprite spr = sprite(s.sprite);
            reqspr(b.x + s.x, b.y, s.sprite);
            Rect window = {b.x + s.x + kCoords[0x54].x, b.y + kCoords[0x54].y, spr.width, spr.height};
            text(8, window, 0, 0, str(TextSection::PolicyNames, 5 + i), kTextCentre);
        }
        if (hs_.staff_leave_rooms)
            reqspr(kCoords[0x52].x, kCoords[0x52].y, 4);
        else
            reqspr(kCoords[0x53].x, kCoords[0x53].y, 5);
        if (!mleft_) {
            c_.value = 0;
            return;
        }
        // Which slider is dragged (pol.value): 2 guess a cure, 1 send home,
        // 3 diagnosis, 4 rest; 0x63 none.
        if (c_.value == 0) {
            c_.value = 0x63;
            const int order[4][2] = {{0x25, 2}, {0x26, 1}, {0x27, 3}, {0x28, 4}};
            for (const auto &o : order) {
                if (over(mx_, my_, 0, kBoxes[o[0]].y, kUiWidth, kBoxes[o[0]].h)) {
                    c_.value = o[1];
                    break;
                }
            }
        }
        struct Drag {
            int value, box, sprite, divisor, step, max, offset, top;
        };
        const Drag drags[4] = {{2, 0x25, 2, 3, 1, 100, 0, 100},
                               {1, 0x26, 1, 3, 1, 100, 0, 100},
                               {3, 0x27, 3, 20, 0x42, 1000, 1000, 2000},
                               {4, 0x28, 3, 10, 0x27, 1000, 0, 1000}};
        for (const Drag &d : drags) {
            if (c_.value != d.value)
                continue;
            const Rect &b = kBoxes[d.box];
            int half = sprite(d.sprite).width / 2;
            uint16_t which = uint16_t(d.value);
            if (over(mx_, my_, half + b.x, 0, half + b.w, kUiHeight)) {
                int x = std::clamp((mx_ - half - b.x) / d.divisor * d.step, 0, d.max);
                r_.event = {EV_SET_POLICY, which, 0, uint16_t(x + d.offset)};
            }
            if (over(mx_, my_, 0, 0, b.x, kUiHeight))
                r_.event = {EV_SET_POLICY, which, 0, uint16_t(d.offset)};
            if (over(mx_, my_, b.x + b.w + half, 0, kUiWidth - (half + b.w), kUiHeight))
                r_.event = {EV_SET_POLICY, which, 0, uint16_t(d.top)};
        }
    }

    // ---- Research (src/rschscrn.cpp) ----

    void display_research()
    {
        int frame_no = int(game_.turn_count & 0xf) / 2 + 1;
        switch (c_.state) {
        case 0:
            load_pic();
            c_.state = 1;
            c_.value = 0;
            sound(kSfxResearch);
            break;
        case 1:
            copy_screen();
            break;
        case 2:
            return_from_screen();
            break;
        }
        reqspr(kCoords[85].x, kCoords[85].y, frame_no + 12);
        reqspr(kCoords[86].x, kCoords[86].y, frame_no + 4);
        process_res_panel();
        draw_research_text();
        left_ = rleft_ = rright_ = false;
        if (exit_quick_requester())
            c_.state = 2;
    }

    // The research categories' progress bars need research costs, which
    // nothing works out yet: none is drawn, as for a category with no cost.
    void draw_research_text()
    {
        const int f = 9;
        for (int i = 0; i < 5; i++) {
            text(f, kCoords[i + 0x57].x, kCoords[i + 0x57].y, str(TextSection::ResearchNames, i));
            if (hs_.research_alloc[size_t(i)] >= 0) {
                text(f, kCoords[i + 0x61].x, kCoords[i + 0x61].y, format("%d", hs_.research_alloc[size_t(i)]));
            } else {
                const Rect &b = kBoxes[i + 0x29];
                draw_box(dst_, b.x, b.y, b.w, b.h, 0xb);
                text(f, kCoords[i + 0x61].x, kCoords[i + 0x61].y, text_.misc(0x14));
            }
        }
        uint8_t total = 0;
        for (int8_t a : hs_.research_alloc)
            if (a > 0)
                total = uint8_t(total + a);
        text(f, kCoords[0x66].x, kCoords[0x66].y, str(TextSection::ResearchNames, 6));
        text(f, kCoords[0x67].x, kCoords[0x67].y, format("%d", total));
    }

    void reset_repeat()
    {
        c_.repeat_step = 0x100;
        c_.repeat_time = c_.clock_ms;
    }
    // The price and research buttons act at once, then every frame once
    // held for 250 ms, by a step that grows while held.
    bool repeat_due() const { return c_.clock_ms - c_.repeat_time > 0xfa || c_.repeat_step == 0x100; }
    int next_repeat_step()
    {
        c_.repeat_step = c_.repeat_step < 0xa00 ? c_.repeat_step + 0x10 : 0xa00;
        return c_.repeat_step >> 8;
    }

    void process_res_panel()
    {
        if (rleft_) {
            for (int i = kResearchButtons; kButtons[i].w && kButtons[i].h; i++) {
                const ScreenButton &b = kButtons[i];
                if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                    if (i == kResearchButtons)
                        process_res_button(0);
                    reqspr(b.sprite_x, b.sprite_y, b.sprite);
                }
            }
            reset_repeat();
            c_.value = 0;
            rleft_ = false;
        }
        if (mleft_) {
            for (int i = kResearchButtons; kButtons[i].w && kButtons[i].h; i++) {
                const ScreenButton &b = kButtons[i];
                if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                    if (i != kResearchButtons)
                        process_res_button(i - kResearchButtons);
                    reqspr(b.sprite_x, b.sprite_y, b.sprite);
                    break;
                }
            }
        } else {
            reset_repeat();
        }
    }

    void process_res_button(int button)
    {
        if (button == 0) {
            c_.state = 2;
            sound(kSfxNo4);
            return;
        }
        bool down = button <= 5;
        size_t cat = size_t(down ? button - 1 : button - 6);
        int alloc = hs_.research_alloc[cat];
        if (alloc < 0)
            return;
        int others = 0;
        for (size_t i = 0; i < 5; i++)
            if (i != cat && hs_.research_alloc[i] >= 0)
                others += hs_.research_alloc[i];
        bool can = down ? alloc > 0 : alloc + others < 100;
        if (!can) {
            if (!c_.value) {
                sound(kSfxWrong2);
                c_.value = 1;
            }
            return;
        }
        if (!repeat_due())
            return;
        int step = next_repeat_step();
        int arg = down ? std::max(alloc - step, 0) : std::min(alloc + step, 100 - others);
        r_.event = {EV_ALTER_RESEARCH, uint16_t(cat), 0, uint16_t(arg)};
        sound(kSfxSelectX);
    }

    // ---- Graphs (src/graph.cpp) ----

    void draw_graphs(int x, int y, int h, int step)
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
        const int f = 0xb;
        int font_height = line_height(font(f));
        Colours col = colours();
        const uint8_t colour[8] = {col.red,  col.light_red, col.orange, col.green,
                                   col.blue, col.dark_blue, col.purple, col.salmon};
        for (int i = 0; i < 8; i++) {
            const ScreenButton &b = kButtons[i + kGraphButtons];
            if (!c_.graph_visible[size_t(i)])
                reqspr(b.sprite_x, b.sprite_y, b.sprite);
            const Rect &box = kBoxes[i + 47];
            text(f, box, 0, (box.h - font_height) >> 1, str(TextSection::Report, i + 3), kTextCentre | kTextOneColour,
                 colour[i]);
        }
        const int scale = c_.graph_period == 0 ? 1 : c_.graph_period == 1 ? 12 : 48;
        const uint32_t turn = game_.turn_count;
        auto shown = [&](int n) { return uint32_t(n * 1500 * scale) < turn; };
        struct Label {
            int graph, y;
            int32_t data;
        };
        std::vector<Label> labels;
        for (int i = 0; i < 8; i++) {
            if (c_.graph_visible[size_t(i)] != 1)
                continue;
            // The game keeps a history per scale, all the same.
            const auto &values = game_.finance.history[size_t(kGraphRows[size_t(i)])];
            int32_t max = -100000000, min = 100000000;
            for (int k = 11; k >= 0 && shown(12 - k - 1); k--) {
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
            auto pos = std::find_if(labels.begin(), labels.end(), [&](const Label &l) { return l.y > at; });
            labels.insert(pos, {i, at, values[11]});
            for (int k = 11; k > 0 && shown(12 - k); k--) {
                int nx = x - (12 - k) * step;
                int ny = uint16_t(y - int((values[size_t(k - 1)] - min) * int64_t(h) / range));
                ny = std::clamp(ny, y - h, y);
                draw_line(dst_, px, py, nx, ny, colour[i]);
                draw_line(dst_, px - 1, py, nx - 1, ny, colour[i]);
                px = nx;
                py = ny;
            }
        }
        // spread_graph_labels: nudged apart a pixel at a time, within the
        // graph.
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
        for (const Label &l : labels) {
            std::string s = format("%s:%d", std::string(str(TextSection::Report, l.graph + 3)).c_str(), l.data);
            text(f, x + 4, l.y, s, kTextOneColour, colour[l.graph]);
        }
        int month = game_.date.month;
        int year = int(turn / 0x4650);
        int span_year = year - (year & 3);
        for (int i = 0; i < 12 && shown(i); i++) {
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
            text(f, px + 2 - width / 2, y + 10, s, kTextOneColour, col.black);
            draw_line(dst_, px, y + 2, px, y + 6, col.black);
        }
    }

    void process_graph_panel()
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

    void process_graph_button(int button)
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

    // ---- Casebook (src/drugs.cpp) ----

    bool casebook_lists(int i) const
    {
        const IllnessFigures &f = hs_.illnesses[size_t(i)];
        const Balance &bal = game_.settings();
        return (i < kFirstDiagnosis && f.discovered) || (i >= kFirstDiagnosis && f.cure_available == 1) ||
               (i == kIllAutopsy && bal.objects[kObjAutoAutopsy].start_avail != 0 &&
                bal.objects[kObjAutoAutopsy].available_for_level != 0 && has_room(game_.world, kRoomResearch));
    }

    // get_next_illness: the illness before (forward 0) or after `from`,
    // 0xfa if none.
    int get_next_illness(int forward, int from) const
    {
        if (forward == 0) {
            for (int i = from - 1; i > 0; i--)
                if (casebook_lists(i))
                    return i;
        } else {
            for (int i = from + 1; i < 0x2e; i++)
                if (casebook_lists(i))
                    return i;
        }
        return 0xfa;
    }

    void display_drugs()
    {
        switch (c_.state) {
        case 0:
            load_pic();
            c_.state = 2;
            c_.value = 0;
            c_.repeat_step = 0x100;
            c_.drug_pct_display_time = 0;
            break;
        case 1:
            return_from_screen();
            break;
        case 2:
            copy_screen();
            break;
        }
        int line_h = line_height(font(0xe));
        const Rect window = kBoxes[0];
        int illness = c_.selected_illness;
        const IllnessFigures &fig = hs_.illnesses[size_t(illness)];
        const ExpertiseRow &def = kExpertise[size_t(illness)];
        for (int i = 0; i < 12; i++) {
            std::string s;
            switch (i) {
            case 6:
                s = format("%d", illness >= kFirstDiagnosis || illness == kIllGeneralPractice ? game_.reputation
                                                                                              : int(fig.reputation));
                break;
            case 7:
                if (c_.drug_pct_display_time) {
                    c_.drug_pct_display_time--;
                    s = format("%d%%", fig.price_scale / 100);
                } else {
                    s = format("$%d", fig.price);
                }
                break;
            case 8:
                s = format("$%d", fig.income);
                break;
            case 9:
                s = format("%d", fig.cured);
                break;
            case 10:
                s = format("%d", fig.deaths);
                break;
            case 11:
                s = format("%d", fig.turned_away);
                break;
            default:
                s = conv_string(str(TextSection::Cabinet, i), 1);
                break;
            }
            text(0xe, window, kCoords[i + 0x11].x, kCoords[i + 0x11].y, s, kTextCentre);
        }
        // The listed illnesses: the selected one in the middle, up to seven
        // after it and seven before.
        std::array<int, 18> list{};
        list[8] = c_.selected_illness;
        int count = 0;
        for (int i = c_.selected_illness + 1; i < 0x2e; i++) {
            if (casebook_lists(i)) {
                if (count >= 7)
                    break;
                list[size_t(count + 9)] = i;
                count++;
            }
        }
        count = 7;
        for (int i = c_.selected_illness - 1; i > 0; i--) {
            const IllnessFigures &f = hs_.illnesses[size_t(i)];
            if (((i < kFirstDiagnosis && f.discovered) || (i >= kFirstDiagnosis && f.cure_available == 1)) &&
                i < 0x2c) {
                if (count == 0)
                    break;
                list[size_t(count)] = i;
                count--;
            }
        }
        for (int i = 0; i < 0x11; i++) {
            if (!list[size_t(i)])
                continue;
            std::string s = conv_string(str(TextSection::Illness, list[size_t(i)]), 0);
            int f = 0xe;
            if (i == 8) {
                f = 0xf;
            } else if (left_ && over(mx_, my_, kCoords[i].x, kCoords[i].y, kUiWidth - kCoords[i].y, line_h)) {
                c_.selected_illness = uint8_t(list[size_t(i)]);
                sound(kSfxPageTurn2);
                left_ = false;
            }
            text(f, kCoords[i].x, kCoords[i].y, s);
        }
        const Font &small = font(0xe);
        text(0xe, {kCoords[0x1e].x, kCoords[0x1e].y, 0x36, 0x1a}, 0, (0x1a - line_height(small)) / 2,
             str(TextSection::Cabinet, 7), kTextCentre);
        const Balance &bal = game_.settings();
        auto drug_effect = [&](std::string_view s) {
            text(0x2e, {0x134, kCoords[0x21].y, 0x1c, 0x32}, 0, 0, s, kTextCentre);
        };
        auto cure_sprite = [&](int index) { reqspr(kCoords[0x1f].x, kCoords[0x1f].y, index); };
        if (!(illness >= kFirstDiagnosis || illness == kIllGeneralPractice)) {
            bool plain = def.outcome != 1 && def.outcome != 5 && def.outcome != 4;
            if (fig.cure_available == 1) {
                if (def.kind == kObjPharmacyCabinet) {
                    cure_sprite(7);
                    drug_effect(format("%d", fig.drug_effectiveness));
                } else if (def.category == kRoomTheatre) {
                    cure_sprite(8);
                } else if (plain) {
                    cure_sprite(6);
                }
                reqspr(kCoords[0x20].x, kCoords[0x20].y, 11);
            } else {
                reqspr(kCoords[0x20].x, kCoords[0x20].y, 12);
                if (plain) {
                    // The equipment is buyable (equipment[].available, at
                    // the start objects[].StartAvail) for the level.
                    const Balance::Object &obj = bal.objects[def.kind];
                    if (def.kind == kObjPharmacyCabinet) {
                        cure_sprite(7);
                        drug_effect("?");
                    } else if (obj.start_avail == 1 && obj.available_for_level == 1) {
                        cure_sprite(6);
                        if (def.staff_mask == 0x40)
                            cure_sprite(10);
                        if (def.category == kRoomTheatre)
                            cure_sprite(8);
                    } else {
                        cure_sprite(9);
                    }
                }
            }
        } else if (illness >= kFirstDiagnosis) {
            reqspr(kCoords[0x20].x, kCoords[0x20].y, fig.cure_available == 1 ? 11 : 12);
        }
        if (def.research_type != 0) {
            int f = illness == hs_.research_illness ? 0xf : 0xe;
            text(f, kCoords[0x1d].x, kCoords[0x1d].y, conv_string(str(TextSection::Cabinet, 6), 1));
        }
        process_drug_panel();
        process_drug_keys();
        if (exit_quick_requester())
            c_.state = 1;
    }

    void process_drug_panel()
    {
        if (left_) {
            c_.value = 0;
            for (int i = kDrugButtons; kButtons[i].w && kButtons[i].h; i++) {
                const ScreenButton &b = kButtons[i];
                if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                    process_drug_button(i - kDrugButtons);
                    if (b.sprite)
                        reqspr(b.sprite_x, b.sprite_y, b.sprite);
                    break;
                }
            }
            reset_repeat();
        }
        if (mleft_) {
            for (int i = kDrugButtons; kButtons[i].w && kButtons[i].h; i++) {
                const ScreenButton &b = kButtons[i];
                if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                    if (i - kDrugButtons <= 1)
                        process_drug_button(i - kDrugButtons);
                    if (b.sprite)
                        reqspr(b.sprite_x, b.sprite_y, b.sprite);
                    break;
                }
            }
        }
        left_ = false;
    }

    // price_up and price_down: the price moved through the command, refused
    // once at 200% and 50%.
    void price(bool up)
    {
        if (!repeat_due())
            return;
        uint16_t value = uint16_t(c_.repeat_step >> 8);
        next_repeat_step();
        r_.event = {uint8_t(up ? EV_PRICE_UP : EV_PRICE_DOWN), value, 0, c_.selected_illness};
        int scale = hs_.illnesses[c_.selected_illness].price_scale;
        if (up ? scale < 20000 : scale > 5000) {
            sound(kSfxSelectX);
        } else if (!c_.value) {
            sound(kSfxWrong2);
            c_.value = 1;
        }
    }

    void select_next(int forward)
    {
        int illness = get_next_illness(forward, c_.selected_illness);
        if (illness != 0xfa) {
            c_.selected_illness = uint8_t(illness);
            sound(kSfxPageTurn2);
        } else {
            sound(kSfxWrong2);
        }
    }

    void process_drug_keys()
    {
        if (keys_.up)
            select_next(0);
        if (keys_.down)
            select_next(1);
        if (keys_.right)
            price(true);
        else if (keys_.left)
            price(false);
        else if (!mleft_)
            reset_repeat();
    }

    void process_drug_button(int action)
    {
        switch (action) {
        case 0:
        case 1:
            price(action == 0);
            c_.drug_pct_display_time = 0x32;
            break;
        case 2:
        case 3:
            select_next(action - 2);
            c_.drug_pct_display_time = 0x32;
            break;
        case 4:
            c_.state = 1;
            sound(kSfxNo4);
            break;
        case 5:
            r_.event = {EV_CONCENTRATE_RESEARCH, c_.selected_illness};
            sound(kSfxSelectX);
            break;
        }
    }

    // ---- Staff list (src/slist.cpp) ----

    void display_slist()
    {
        switch (c_.state) {
        case 0:
            load_pic();
            c_.state = 1;
            c_.value = 0;
            c_.slist_filter = 1;
            c_.slist_page = 0;
            draw_slist();
            break;
        case 1:
            copy_screen();
            draw_slist();
            process_slist_panel();
            break;
        case 2:
            r_.event = {EV_RETURN_TO_THING_WINDOW, 0, 0, uint16_t(c_.value)};
            break;
        case 3:
            return_from_screen();
            break;
        }
        slist_keys();
        if (!staff_on_page(c_.slist_page) && c_.slist_page > 0)
            c_.slist_page--;
        rleft_ = left_ = false;
        if (exit_quick_requester())
            c_.state = 3;
    }

    void draw_slist()
    {
        draw_slist_staff(c_.value);
        draw_slist_window(c_.value);
        draw_slist_overall();
        draw_slist_nums();
    }

    void slist_filter(int filter)
    {
        c_.slist_filter = filter;
        c_.value = 0;
        c_.slist_cursor = 0;
        sound(kSfxSelectX);
    }

    void slist_keys()
    {
        if (keys_.right) {
            if (c_.slist_filter != 8)
                slist_filter(c_.slist_filter << 1);
        } else if (keys_.left) {
            if (c_.slist_filter != 1)
                slist_filter(c_.slist_filter >> 1);
        }
        if (keys_.down) {
            int prev = c_.value;
            c_.value = get_slist_staff(0);
            if (prev != c_.value) {
                sound(kSfxSelectX);
                if (c_.slist_cursor / 11 > 0 && c_.slist_cursor % 11 == 0) {
                    if (c_.slist_page < 150 && staff_on_page(c_.slist_page + 1))
                        c_.slist_page++;
                    sound(kSfxSelectX);
                }
            }
        } else if (keys_.up) {
            int prev = c_.value;
            c_.value = get_slist_staff(1);
            if (prev != c_.value) {
                sound(kSfxSelectX);
                if (c_.slist_cursor / 10 > 0 && c_.slist_cursor % 10 == 0) {
                    sound(kSfxSelectX);
                    if (c_.slist_page > 0) {
                        c_.slist_page--;
                        for (int i = 0; i < 10; i++)
                            c_.value = get_slist_staff(1);
                    }
                }
            }
        }
    }

    int get_slist_staff(int dir)
    {
        const auto &things = game_.world.things;
        if (dir == 0) {
            for (size_t i = size_t(c_.value) + 1; i < things.size(); i++) {
                if (slist_shows(things[i], c_.slist_filter)) {
                    c_.slist_cursor++;
                    return int(i);
                }
            }
        } else {
            for (int i = c_.value - 1; i >= 1; i--) {
                if (slist_shows(things[size_t(i)], c_.slist_filter)) {
                    c_.slist_cursor--;
                    return i;
                }
            }
        }
        return c_.value;
    }

    bool staff_on_page(int page) const
    {
        int listed = 0;
        const auto &things = game_.world.things;
        for (size_t i = 1; i < things.size(); i++) {
            if (slist_shows(things[i], c_.slist_filter)) {
                if (listed >= page * 10 && listed < (page + 1) * 10)
                    return true;
                listed++;
            }
        }
        return false;
    }

    // draw_gauge (src/req3.cpp): `value` clamped to min-max as a share of
    // `range`, filled from the left if `up`, else the rest.
    void draw_gauge(int value, const Rect &box, int max, int min, int range, bool up)
    {
        value = std::clamp(value, min, max);
        int filled = up ? box.w * value / range : box.w - box.w * value / range;
        for (int i = 0; i < filled; i++)
            reqspr(box.x + i, box.y, box.h);
    }

    void draw_slist_staff(int selected)
    {
        int height = line_height(font(1));
        for (int row = 0; row < 3; row++) {
            const Rect &b = kBoxes[row + 61];
            text(1, b, 0, (b.h - height) / 2, str(TextSection::Slist, row + 2), kTextCentre);
        }
        int row = 0, listed = 0;
        const auto &things = game_.world.things;
        Colours col = colours();
        for (size_t id = 1; id < things.size() && row < 10; id++) {
            const Thing &s = things[id];
            if (!slist_shows(s, c_.slist_filter))
                continue;
            if (listed >= c_.slist_page * 10 && listed < (c_.slist_page + 1) * 10) {
                const Rect &line = kBoxes[row + 104];
                int f = 1;
                if (over(mx_, my_, line.x, line.y, line.w, line.h)) {
                    if (c_.slist_cursor != row + c_.slist_page * 10 + 1)
                        sound(kSfxHighlight5);
                    f = 2;
                    c_.slist_cursor = c_.slist_page * 10 + row + 1;
                }
                if (selected == int(id)) {
                    f = 2;
                    int fx = line.x, fy = line.y - 1;
                    draw_line(dst_, fx, fy, fx + line.w, fy, col.red);
                    draw_line(dst_, fx, fy, fx, fy + line.h, col.red);
                    draw_line(dst_, fx + line.w, fy, fx + line.w, fy + line.h, col.red);
                    draw_line(dst_, fx, fy + line.h, fx + line.w, fy + line.h, col.red);
                    draw_slist_button_overlays(s);
                }
                text(f, kCoords[row + 114].x, kCoords[row + 114].y, format("%d", listed + 1));
                std::string name =
                    format("%c. %s%s", s.initial, std::string(str(TextSection::StaffNames1, s.surname_a)).c_str(),
                           std::string(str(TextSection::StaffNames2, s.surname_b)).c_str());
                text(f, kCoords[row + 124].x, kCoords[row + 124].y, name);
                text(f, kCoords[row + 134].x, kCoords[row + 134].y, format("$%d", s.salary));
                draw_gauge(s.happiness, kBoxes[row + 71], 100, 0, 100, true);
                draw_gauge(s.tiredness, kBoxes[row + 81], 1000, 0, 1000, false);
                draw_gauge(s.ability, kBoxes[row + 91], 1000, 30, 970, true);
                if (mleft_ && over(mx_, my_, line.x, line.y, line.w, line.h)) {
                    if (int(id) != c_.value)
                        sound(kSfxSelectX);
                    c_.value = int(id);
                }
                row++;
            }
            listed++;
        }
        if (row < 10) {
            const Rect &b = kBoxes[row + 104];
            draw_box(dst_, b.x, b.y, b.w, (10 - row) * b.h, 0x77);
        }
        int pages = 0;
        while (staff_on_page(pages))
            pages++;
        if (pages > 1) {
            const Rect &b = kBoxes[114];
            uint8_t pos = uint8_t(b.h / (pages - 1) * c_.slist_page);
            reqspr(b.x, pos + b.y, b.w);
        }
    }

    // draw_face (src/req2.cpp): the face in its frame, its bottom left at
    // (x, y).
    void draw_face(const Thing &s, int x, int y)
    {
        const size_t heads = size_t(kFaceW) * kHeadH * kFaceParts;
        const size_t bodies = size_t(kFaceW) * kBodyH * kFaceParts;
        const struct {
            size_t offset;
            int rows;
        } parts[3] = {{size_t(s.face_head) * kFaceW * kHeadH, kHeadH},
                      {heads + size_t(s.face_body) * kFaceW * kBodyH, kBodyH},
                      {heads + bodies + size_t(s.face_legs) * kFaceW * kLegsH, kLegsH}};
        int row = y - kFaceH - kFaceBorder;
        for (const auto &part : parts) {
            for (int r = 0; r < part.rows; r++, row++) {
                size_t src = part.offset + size_t(r) * kFaceW;
                if (src + kFaceW > c_.faces.size())
                    continue;
                for (int col = 0; col < kFaceW; col++)
                    plot(dst_, x + kFaceBorder + col, row, c_.faces[src + size_t(col)]);
            }
        }
        int top = y - kFaceH - kFaceBorder * 2, w = kFaceBorder * 2 + kFaceW, h = kFaceH + kFaceBorder * 2;
        draw_box(dst_, x, top, w, kFaceBorder, kFaceBorderColour);
        draw_box(dst_, x, top, kFaceBorder, h, kFaceBorderColour);
        draw_box(dst_, x, y - kFaceBorder, w, kFaceBorder, kFaceBorderColour);
        draw_box(dst_, x + kFaceW + kFaceBorder, top, kFaceBorder, h, kFaceBorderColour);
    }

    // The game also draws the staff member on the map in the view of
    // kSlistView, which is not ported; nor are the training bars (no
    // training is tracked).
    void draw_slist_window(int id)
    {
        if (!id)
            return;
        const Thing &s = thing(id);
        draw_face(s, kSlistFace.x, kSlistFace.y);
        if (s.subtype == kStaffDoctor || s.subtype == kStaffSurgeon) {
            bool shows[6] = {(s.skills & kSkillJunior) && !(s.skills & kSkillConsultant),
                             (s.skills & kSkillDoctor) && !(s.skills & kSkillConsultant) && !(s.skills & kSkillJunior),
                             (s.skills & kSkillConsultant) != 0,
                             (s.skills & kSkillSurgeon) != 0,
                             (s.skills & kSkillPsychiatrist) != 0,
                             (s.skills & kSkillResearcher) != 0};
            for (int i = 0; i < 6; i++)
                if (shows[i])
                    reqspr(kBoxes[i + 64].x, kBoxes[i + 64].y, kBoxes[i + 64].h);
        }
        int delay = std::clamp(int(s.work_delay), 0, 10);
        int count = delay * 4200 / 1000;
        for (int j = 0; j < count; j++)
            reqspr(kBoxes[70].x + j, kBoxes[70].y, kBoxes[70].h);
    }

    void draw_slist_overall()
    {
        int happiness = 0, tiredness = 0, ability = 0, count = 0;
        for (const Thing &s : game_.world.things) {
            if (slist_shows(s, c_.slist_filter)) {
                happiness += s.happiness;
                tiredness += s.tiredness;
                ability += s.ability;
                count++;
            }
        }
        if (!(c_.slist_filter & 1)) {
            // draw_slist_coverup: the doctors' parts.
            for (int b : {116, 117})
                draw_box(dst_, kBoxes[b].x, kBoxes[b].y, kBoxes[b].w, kBoxes[b].h, 0x76);
        }
        if (count) {
            draw_gauge(happiness / count, kBoxes[101], 100, 0, 100, true);
            draw_gauge(tiredness / count, kBoxes[102], 1000, 0, 1000, false);
            draw_gauge(ability / count, kBoxes[103], 1000, 30, 970, true);
        }
    }

    void process_slist_panel()
    {
        if (rleft_) {
            for (int i = kSlistButtons; kButtons[i].w && kButtons[i].h; i++) {
                const ScreenButton &b = kButtons[i];
                if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                    process_slist_button(i - kSlistButtons);
                    reqspr(b.sprite_x, b.sprite_y, b.sprite);
                }
            }
            if (over(mx_, my_, kSlistView.x, kSlistView.y, kSlistView.w, kSlistView.h) && c_.value) {
                c_.state = 2;
                sound(kSfxSelectX);
            }
            if (c_.value)
                draw_slist_button_overlays(thing(c_.value));
        }
        if (mleft_) {
            for (int i = kSlistButtons; kButtons[i].w && kButtons[i].h; i++) {
                const ScreenButton &b = kButtons[i];
                if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                    reqspr(b.sprite_x, b.sprite_y, b.sprite);
                    break;
                }
            }
            if (c_.value)
                draw_slist_button_overlays(thing(c_.value));
        }
    }

    void process_slist_button(int button)
    {
        static constexpr int kFilters[4] = {1, 2, 8, 4};
        switch (button) {
        case 0:
        case 1:
        case 2:
        case 3:
            if (c_.slist_filter != kFilters[button])
                sound(kSfxSelectX);
            c_.slist_filter = kFilters[button];
            c_.value = 0;
            c_.slist_cursor = 0;
            break;
        case 4:
            // Sacking asks first through the yes/no requester, which the
            // charts do not show yet: refused.
            if (c_.value)
                sound(kSfxWrong2);
            break;
        case 5:
            if (c_.value) {
                r_.event = {EV_INCREASE_STAFF_SALARIES, uint16_t(thing(c_.value).salary / 10), 0, uint16_t(c_.value)};
                sound(kSfxBonusAll2);
            }
            break;
        case 6:
            if (c_.value) {
                r_.event = {EV_PAY_STAFF_BONUS, uint16_t(thing(c_.value).salary / 10), 0, uint16_t(c_.value)};
                sound(kSfxBonus2);
            }
            break;
        case 7:
            c_.state = 3;
            sound(kSfxNo4);
            break;
        case 8:
        case 9: {
            uint8_t sfx = kSfxWrong2;
            int page = button == 8 ? c_.slist_page - 1 : c_.slist_page + 1;
            if (page >= 0 && page <= 150 && staff_on_page(page)) {
                c_.slist_page = page;
                for (int i = 0; i < 10; i++)
                    c_.value = get_slist_staff(button == 8 ? 1 : 0);
                sfx = kSfxSelectX;
            }
            sound(sfx);
            break;
        }
        }
    }

    void draw_slist_nums()
    {
        int tab = c_.slist_filter & 1 ? 0 : c_.slist_filter & 2 ? 1 : c_.slist_filter & 8 ? 2 : 3;
        const ScreenButton &b = kButtons[kSlistButtons + tab];
        reqspr(b.sprite_x, b.sprite_y, b.sprite);
        int counts[4] = {};
        for (const Thing &s : game_.world.things) {
            if (s.type != kThingStaff || s.owner != 0 || s.state == kStaffApplicant)
                continue;
            if (s.subtype == kStaffDoctor || s.subtype == kStaffSurgeon)
                counts[0]++;
            else if (s.subtype == kStaffNurse)
                counts[1]++;
            else if (s.subtype == kStaffHandyman)
                counts[2]++;
            else if (s.subtype == kStaffReceptionist)
                counts[3]++;
        }
        for (int i = 0; i < 4; i++)
            text(0xd, kCoords[i + 144].x, kCoords[i + 144].y, format("%d", counts[i]));
    }

    void draw_slist_button_overlays(const Thing &s)
    {
        text(1, kCoords[149].x, kCoords[149].y, format("$%d", s.salary / 10 + s.salary));
        text(1, kCoords[150].x, kCoords[150].y, format("$%d", s.salary / 10));
    }

    // ---- Status (src/overscr.cpp) ----

    HospitalFigures figures(int hospital) const
    {
        HospitalFigures h;
        const Balance &bal = game_.settings();
        if (hospital == 0) {
            Finance fin = game_.finance;
            calculate_value(fin, game_.world, bal, game_.reputation);
            h.reputation = game_.reputation;
            h.balance = game_.balance;
            h.loan = fin.loan;
            h.value = fin.hospital_value;
        } else {
            // The rivals are not simulated: as init_player leaves them.
            h.balance = int32_t(bal.towns[size_t(game_.level_number()) % bal.towns.size()].start_cash);
        }
        return h;
    }

    void display_peep_overview()
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
            if (rleft_) {
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

    void draw_overview_screen()
    {
        text(0x24, kCoords[0x6f].x, kCoords[0x6f].y,
             format("%s %4.4d", std::string(str(TextSection::Overview, 0)).c_str(), game_.date.year + 2000));
        for (int i = 0; i < 4; i++)
            text(c_.value == i ? 0x25 : 0x24, kCoords[0x68 + i].x, kCoords[0x68 + i].y, c_.hospital_names[size_t(i)]);
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
        if (patients > 1) {
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
            text(0x24, kBoxes[0x3b], 0, 0, warn, kTextWrap);
        }
        // Single player: each hospital's market share is 25%.
        std::string share = format("%s (%d%%)", std::string(str(TextSection::Overview, 6)).c_str(), 25);
        text(0x26, kCoords[0x70].x, kCoords[0x70].y, conv_string(share, 0));
        text(0x26, kCoords[0x71].x, kCoords[0x71].y, str(TextSection::Overview, 7));
        display_overview_bars();
    }

    // display_overview_bars: up to five criteria bars, a lose criterion with
    // progress in place of the win criterion of its kind in the player's
    // best group.
    void display_overview_bars()
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

    // calculate_piechart and pie_render (src/pie.cpp): four equal slices
    // (single player), the shown hospital's pulled out.
    void draw_piechart()
    {
        struct Sector {
            int32_t start, end, pull_out;
            SliceColours colours;
            int text_x, text_y;
        };
        std::array<Sector, 4> sectors{};
        int prev = 0;
        for (size_t i = 0; i < 4; i++) {
            sectors[i].start = prev;
            sectors[i].end = (prev + (25 << 11) / 100) & 0x7ff;
            prev = sectors[i].end;
            sectors[i].colours = kSlices[i];
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

        const int cx = 0x1fa, cy = 0x78, rx = 0x40, ry = 0x1e, h = 0x14;
        auto sn = [](int a) { return sin_table(a & 0x7ff); };
        auto cs = [](int a) { return sin_table((a & 0x7ff) + 0x200); };
        auto sector_render = [&](int ccx, int ccy, Sector &sec) {
            int start = sec.start, end = sec.end;
            if (end < start)
                end += 0x800;
            int mid = (start + end) >> 1;
            int ox = ccx + (fixmul(cs(mid), rx * sec.pull_out) >> 8);
            int oy = ccy - (fixmul(sn(mid), ry * sec.pull_out) >> 8);
            int sx = ox + fixmul(cs(start), rx), sy = oy - fixmul(sn(start), ry);
            int ex = ox + fixmul(cs(end), rx), ey = oy - fixmul(sn(end), ry);
            int text_radius = kSliceTextOffset + sec.pull_out;
            sec.text_x = ccx + (fixmul(cs(mid), rx * text_radius) >> 8);
            sec.text_y = ccy - (fixmul(sn(mid), ry * text_radius) >> 8);
            const SliceColours &c = sec.colours;
            draw_triangle(dst_, ox, oy, sx, sy, sx, sy + h, c.side1);
            draw_triangle(dst_, sx, sy + h, ox, oy + h, ox, oy, c.side1);
            draw_triangle(dst_, ox, oy, ox, oy + h, ex, ey, c.side2);
            draw_triangle(dst_, ex, ey, ox, oy + h, ex, ey + h, c.side2);
            int segments = std::max((end - start) / 50, 1);
            int ang = start, step = (end - start) / segments;
            int x2 = ox + fixmul(cs(ang), rx), y2 = oy - fixmul(sn(ang), ry);
            while (--segments >= 0) {
                int x1 = x2, y1 = y2;
                ang += step;
                if (segments == 0)
                    ang = end;
                x2 = ox + fixmul(cs(ang), rx);
                y2 = oy - fixmul(sn(ang), ry);
                draw_triangle(dst_, x1, y1 + h, x1, y1, x2, y2, c.rim);
                draw_triangle(dst_, x1, y1 + h, x2, y2, x2, y2 + h, c.rim);
                draw_triangle(dst_, ox, oy, x2, y2, x1, y1, c.top);
                draw_line(dst_, x1 - 1, y1, x2 - 1, y2, c.edge);
                if (!(sn(ang) > 0 || sn(ang - step) > 0))
                    draw_line(dst_, x1 - 1, y1 + h, x2 - 1, y2 + h, c.edge);
            }
            draw_line(dst_, ox - 1, oy, sx - 1, sy, c.edge);
            if (cs(start) >= 0) {
                draw_line(dst_, ox - 1, oy + h, sx - 1, sy + h, c.edge);
                draw_line(dst_, ox - 1, oy, ox - 1, oy + h, c.edge);
                draw_line(dst_, sx - 1, sy, sx - 1, sy + h, c.edge);
            }
            draw_line(dst_, ox - 1, oy, ex - 1, ey, c.edge);
            if (cs(end) <= 0) {
                draw_line(dst_, ox - 1, oy + h, ex - 1, ey + h, c.edge);
                draw_line(dst_, ox - 1, oy, ox - 1, oy + h, c.edge);
                draw_line(dst_, ex - 1, ey, ex - 1, ey + h, c.edge);
            }
        };
        auto behind = [&](int a, int b) {
            int as = cs(sectors[size_t(a)].start), ae = cs(sectors[size_t(a)].end);
            int bs = cs(sectors[size_t(b)].start), be = cs(sectors[size_t(b)].end);
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
            return -sn(sectors[size_t(a)].start) < -sn(sectors[size_t(b)].start);
        };
        // No slice spans more than half the pie with equal shares, so none
        // is split as pie_render splits such a slice.
        std::array<bool, 4> done{};
        for (int j = 0; j < 4; j++) {
            int best = -1;
            for (int k = 0; k < 4; k++)
                if (!done[size_t(k)] && (best == -1 || behind(k, best)))
                    best = k;
            done[size_t(best)] = true;
            sector_render(cx, cy, sectors[size_t(best)]);
        }
        for (size_t i = 0; i < 4; i++)
            text(0x26, sectors[i].text_x, sectors[i].text_y, c_.hospital_names[i]);
    }

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

} // namespace

void start_hospital_settings(HospitalSettings &settings, const Balance &balance, uint32_t clock)
{
    settings = {};
    settings.rest_threshold = balance.gbv.tired;
    for (int i = 0; i < kIllnessCount; i++) {
        IllnessFigures &f = settings.illnesses[size_t(i)];
        f.price = balance.expertise[size_t(i)].start_price;
        f.discovered = balance.expertise[size_t(i)].known;
        if (kExpertise[size_t(i)].kind == kObjPharmacyCabinet)
            f.drug_effectiveness = balance.gbv.start_rating;
    }
    settings.illnesses[0].cure_available = 1;
    // setup_insurances: three of the eight companies, each drawn from those
    // left.
    std::vector<uint8_t> pool = {1, 2, 3, 4, 5, 6, 7, 8};
    for (size_t i = 0; i < 3; i++) {
        size_t pick = clock % pool.size();
        settings.insurers[i] = pool[pick];
        pool.erase(pool.begin() + std::ptrdiff_t(pick));
    }
}

int32_t max_price_for_treatment(const HospitalSettings &settings, const Balance &balance, int illness, int reputation)
{
    int32_t base = balance.expertise[size_t(illness)].start_price;
    int32_t rep = illness >= kFirstDiagnosis || illness == kIllGeneralPractice
                      ? reputation
                      : settings.illnesses[size_t(illness)].reputation;
    int32_t price = base * rep / 500;
    return base > price ? base : price;
}

bool apply_chart_event(Game &game, HospitalSettings &settings, const Event &event)
{
    Finance &fin = game.finance;
    const Balance &bal = game.settings();
    auto pay = [&](uint8_t type, int32_t amount, uint16_t arg = 0) {
        change_balance(fin, game.balance, game.date.day, game.date.month, type, amount, arg);
    };
    auto happier = [](Thing &t) {
        t.happiness = uint16_t(std::clamp(int16_t(t.happiness + 50), int16_t(0), int16_t(100)));
    };
    switch (event.type) {
    case EV_OPEN_SCREEN:
        // update_cure_costs: a price follows its share of the most patients
        // will pay once it is 1% off.
        for (int i = 0; i < kIllnessCount; i++) {
            IllnessFigures &f = settings.illnesses[size_t(i)];
            int32_t target = max_price_for_treatment(settings, bal, i, game.reputation) * f.price_scale / 10000;
            if (target / 100 <= std::abs(target - f.price))
                f.price = target;
        }
        if (event.value == kChartBank || event.value == kChartStatus)
            calculate_value(fin, game.world, bal, game.reputation);
        return true;
    case EV_TAKE_LOAN:
        pay(kTxBankLoan, 5000);
        fin.loan += 5000;
        return true;
    case EV_REPAY_LOAN:
        if (game.balance >= 5000 && game.balance > 0) {
            pay(kTxLoanRepayment, -5000);
            fin.loan -= 5000;
        }
        return true;
    case EV_SET_POLICY:
        switch (event.value) {
        case 1:
            settings.send_home_threshold = std::min<int>(event.arg, settings.guess_cure_threshold);
            break;
        case 2:
            settings.guess_cure_threshold = std::max<int>(event.arg, settings.send_home_threshold);
            break;
        case 3:
            settings.diag_threshold = event.arg;
            break;
        case 4:
            settings.rest_threshold = event.arg;
            break;
        case 5:
            settings.staff_leave_rooms = event.arg != 0;
            break;
        }
        return true;
    case EV_ALTER_RESEARCH: {
        // alter_research (src/rschscrn.cpp): kept if the total would pass
        // 100. The game reads the old value unsigned, so a closed category
        // is opened too; the total is a byte.
        if (event.value >= 5)
            return true;
        int8_t &alloc = settings.research_alloc[event.value];
        int8_t old = alloc;
        alloc = int8_t(uint8_t(event.arg));
        uint8_t total = 0;
        for (int8_t a : settings.research_alloc)
            if (a > 0)
                total = uint8_t(total + a);
        if (total > 100)
            alloc = old;
        return true;
    }
    case EV_CONCENTRATE_RESEARCH:
        settings.research_illness = event.value == settings.research_illness ? 0 : uint8_t(event.value);
        return true;
    case EV_PRICE_UP:
    case EV_PRICE_DOWN: {
        if (event.arg >= kIllnessCount)
            return true;
        IllnessFigures &f = settings.illnesses[event.arg];
        bool up = event.type == EV_PRICE_UP;
        if (up ? f.price_scale < 20000 : f.price_scale > 5000) {
            f.price_scale = std::clamp(f.price_scale + (up ? 1 : -1) * event.value * 100, 5000, 20000);
            f.price = f.price_scale * max_price_for_treatment(settings, bal, event.arg, game.reputation) / 10000;
        }
        return true;
    }
    case EV_INCREASE_STAFF_SALARIES:
        // increase_staff_salaries (src/staff.cpp), by an amount: for one,
        // up to 90000; for all the player's staff the game caps no one.
        for (size_t i = 1; i < game.world.things.size(); i++) {
            Thing &t = game.world.things[i];
            bool chosen = event.arg ? i == event.arg : t.type == kThingStaff && t.owner == 0;
            if (!chosen)
                continue;
            t.salary += event.value;
            if (event.arg && t.salary > 90000)
                t.salary = 90000;
            happier(t);
        }
        return true;
    case EV_PAY_STAFF_BONUS: {
        // pay_staff_bonus (src/staff.cpp), by an amount.
        if (event.arg) {
            if (game.balance >= event.value && game.balance > 0 && event.arg < game.world.things.size()) {
                happier(game.world.things[event.arg]);
                pay(kTxPersonalBonus, -int32_t(event.value), event.arg);
            }
            return true;
        }
        int32_t total = 0;
        for (const Thing &t : game.world.things)
            if (t.type == kThingStaff && t.owner == 0)
                total += event.value;
        if (game.balance >= total && game.balance > 0) {
            for (Thing &t : game.world.things)
                if (t.type == kThingStaff && t.owner == 0)
                    happier(t);
            pay(kTxGeneralBonus, -total);
        }
        return true;
    }
    default:
        return false;
    }
}

bool load_charts(Charts &charts, const std::filesystem::path &dir, const LanguageText &text,
                 const Palette &game_palette, std::string *error)
{
    for (size_t i = 0; i < kChartCount; i++) {
        if (!load_picture(dir, kPictures[i], charts.pictures[i], error) ||
            !load_sheet(dir, kSprites[i], charts.pictures[i].sprites, error))
            return false;
    }
    for (int slot : kChartFonts)
        if (!load_font(dir, slot, charts.fonts[size_t(slot)], error))
            return false;
    auto faces = load_data_file(dir, "qdata/face01v.dat", error);
    if (!faces)
        return false;
    charts.faces = std::move(*faces);
    charts.text = &text;
    charts.game_palette = game_palette;
    charts.palette = game_palette;
    charts.surface.resize(kUiWidth, kUiHeight);
    return true;
}

void chart_command(Charts &charts, const Event &event)
{
    switch (event.type) {
    case EV_OPEN_SCREEN:
        if (event.value >= kChartCount)
            return;
        if (!charts.open)
            charts.palette = charts.game_palette;
        charts.open = true;
        charts.chart = event.value;
        charts.state = 0;
        break;
    case EV_RETURN_FROM_SCREEN:
    case EV_CLOSE_SCREEN:
    case EV_RETURN_TO_THING_WINDOW:
        charts.open = false;
        break;
    default:
        break;
    }
}

ChartsResult chart_frame(Charts &charts, const UiMouse &mouse, const ChartKeys &keys, double seconds, const Game &game,
                         const HospitalSettings &settings)
{
    ChartsResult result;
    if (!charts.open || !charts.text)
        return result;
    UiMouse &m = charts.pending_mouse;
    bool lp = m.left_pressed, rp = m.right_pressed, lr = m.left_released, rr = m.right_released;
    m = mouse;
    m.left_pressed |= lp;
    m.right_pressed |= rp;
    m.left_released |= lr;
    m.right_released |= rr;
    ChartKeys &k = charts.pending_keys;
    k.escape |= keys.escape;
    k.space |= keys.space;
    k.up = keys.up;
    k.down = keys.down;
    k.left = keys.left;
    k.right = keys.right;
    charts.frame_time += seconds;
    // The first frame of a chart runs at once: it shows the picture.
    if (charts.frame_time < kGameFrame && charts.state != 0)
        return result;
    charts.frame_time = std::max(charts.frame_time - kGameFrame, 0.0);
    charts.frame_time = std::min(charts.frame_time, kGameFrame);
    charts.clock_ms += 40;
    Frame(charts, m, k, game, settings, result).run();
    m.left_pressed = m.right_pressed = m.left_released = m.right_released = false;
    k = {};
    return result;
}

Event open_chart_command(int chart, const Game &game, bool *refused)
{
    if (refused)
        *refused = false;
    if (chart == kChartResearch && !has_room(game.world, kRoomResearch)) {
        if (refused)
            *refused = true;
        return {};
    }
    return {EV_OPEN_SCREEN, uint16_t(chart)};
}

int chart_for_panel_action(PanelAction action)
{
    switch (action) {
    case PanelAction::BankManager:
        return kChartBank;
    case PanelAction::BankStatement:
        return kChartStatement;
    case PanelAction::StaffManagement:
        return kChartStaff;
    case PanelAction::Casebook:
        return kChartCasebook;
    case PanelAction::Research:
        return kChartResearch;
    case PanelAction::Status:
        return kChartStatus;
    case PanelAction::Charts:
        return kChartGraphs;
    case PanelAction::Policy:
        return kChartPolicy;
    default:
        return -1;
    }
}

} // namespace th
