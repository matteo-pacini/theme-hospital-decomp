#include "core/world/staff_complex.h"

#include "core/data/gamedata.h"
#include "core/world/complex_tables.h"
#include "core/world/nav.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"

#include <algorithm>

namespace th
{

const std::array<std::array<uint16_t, 0x8c>, 5> kStaffAnimStarts = {{
    {{603,  1620, 1628, 1624, 9999, 825,  1632, 1632, 1636, 9999, 9999, 795,  797,  799,  813,  833,  9999, 1826, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 1939, 1943, 1947, 2390, 1108, 2382, 999}}, // 0
    {{16,   28,   36,   32,   44,   20,   335,  335,  2375, 9999, 239,  241,  248,  232,  288,  328,  355,
      359,  363,  399,  401,  403,  405,  407,  515,  683,  685,  687,  689,  655,  649,  9999, 9999, 9999,
      657,  659,  661,  681,  663,  665,  9999, 9999, 9999, 9999, 653,  691,  9999, 1049, 2439, 1089, 1093,
      1097, 1104, 1108, 1850, 1256, 1260, 1258, 1272, 1207, 1116, 1118, 1283, 1102, 1137, 1295, 1299, 1301,
      1303, 1305, 1307, 1316, 1311, 1318, 1297, 1342, 1281, 1285, 1287, 615,  617,  619,  622,  624,  999}}, // 1
    {{929,  12,   9999, 9999, 9999, 933,  1643, 1643, 1759, 9999, 936,  938,  986,  990,  177,  9999, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 1785, 1789,
      1793, 2386, 1108, 1850, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 1962, 1964, 1966, 1968, 1970, 999}}, // 2
    {{1834, 9999, 1051, 9999, 1051, 1838, 1842, 1842, 9999, 9999, 1051, 1051, 1051, 1051, 1051, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
      9999, 1852, 1856, 1860, 1951, 1108, 1850, 9999, 9999, 9999, 9999, 9999, 999}}, // 3
    {{1144, 9999, 9999, 9999, 9999, 1148, 335,  335,  1020, 9999, 1165, 1167, 1163, 1153, 1159, 1171, 1169, 1181, 1183,
      1391, 1392, 1395, 1396, 2445, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 999}}, // 4
}};

namespace
{

// The staff's steps, indexed by the bytes of StaffComplexes::group. Copied
// from the game's staff_complex (src/cpxtable.cpp).
constexpr std::array<ComplexStep, 237> kStaffComplex = {{
    {0, 0, 0, 0, 0, 0},     // 0
    {0, 0, 0, 0, 0, 0},     // 1
    {0, 0, 0, 0, 0, 0},     // 2
    {0, 24, 0, 0, 0, 1},    // 3
    {0, 50, 0, 0, 0, 1},    // 4
    {0, 0, 0, 6, 0, 0},     // 5
    {0, 6, 0, 0, 0, 1},     // 6
    {0, 8, 0, 0, 0, 1},     // 7
    {0, 38, 0, 0, 0, 1},    // 8
    {0, 40, 0, 0, 0, 1},    // 9
    {0, 18, 0, 0, 0, 1},    // 10
    {0, 0, 0, 6, 0, 0},     // 11
    {0, 0, 0, 38, 0, 0},    // 12
    {0, 56, 0, 18, 0, 0},   // 13
    {0, 46, 0, 18, 0, 0},   // 14
    {0, 20, 0, 14, 0, 0},   // 15
    {0, 22, 0, 0, 0, 1},    // 16
    {0, 0, 0, 0, 0, 0},     // 17
    {0, 0, 0, 0, 0, 1},     // 18
    {0, 0, 0, 38, 0, 0},    // 19
    {0, 28, 0, 0, 0, 0},    // 20
    {0, 30, 0, 0, 0, 1},    // 21
    {0, 36, 0, 0, 0, 0},    // 22
    {0, 20, 1, 14, 0, 0},   // 23
    {0, 48, 0, 0, 0, 1},    // 24
    {0, 44, 0, 18, 0, 0},   // 25
    {0, 0, 0, 0, 0, 1},     // 26
    {0, 0, 0, 0, 0, 1},     // 27
    {0, 54, 42, 14, 0, 0},  // 28
    {0, 52, 8, 0, 0, 0},    // 29
    {0, 54, 44, 14, 0, 0},  // 30
    {0, 52, 10, 0, 0, 0},   // 31
    {0, 52, 12, 0, 0, 0},   // 32
    {0, 54, 47, 14, 0, 0},  // 33
    {0, 54, 48, 14, 0, 0},  // 34
    {0, 54, 76, 14, 0, 0},  // 35
    {0, 54, 77, 14, 0, 0},  // 36
    {0, 54, 78, 14, 0, 0},  // 37
    {0, 54, 79, 14, 0, 0},  // 38
    {0, 54, 80, 14, 0, 0},  // 39
    {0, 52, 18, 0, 0, 0},   // 40
    {0, 52, 20, 0, 0, 0},   // 41
    {0, 52, 22, 0, 0, 0},   // 42
    {0, 54, 88, 14, 0, 0},  // 43
    {0, 54, 89, 14, 0, 0},  // 44
    {0, 54, 90, 14, 0, 0},  // 45
    {0, 54, 91, 14, 0, 0},  // 46
    {0, 54, 92, 14, 0, 0},  // 47
    {0, 54, 93, 14, 0, 0},  // 48
    {0, 10, 1386, 0, 0, 1}, // 49
    {0, 10, 1387, 0, 0, 1}, // 50
    {0, 10, 1388, 0, 0, 1}, // 51
    {0, 58, 1, 0, 0, 1},    // 52
    {0, 58, 4, 0, 0, 1},    // 53
    {0, 10, 1404, 0, 0, 1}, // 54
    {0, 10, 1293, 0, 0, 1}, // 55
    {0, 52, 24, 0, 0, 1},   // 56
    {0, 0, 0, 0, 0, 0},     // 57
    {0, 0, 0, 0, 0, 0},     // 58
    {0, 4, 60, 14, 0, 0},   // 59
    {0, 4, 61, 14, 0, 0},   // 60
    {0, 4, 62, 14, -4, 0},  // 61
    {0, 4, 63, 14, 0, 0},   // 62
    {0, 4, 64, 14, 0, 0},   // 63
    {42, 2, 0, 18, 0, 0},   // 64
    {61, 2, 0, 18, 0, 0},   // 65
    {56, 2, 0, 18, 0, 0},   // 66
    {0, 4, 55, 14, 0, 0},   // 67
    {0, 4, 56, 14, 0, 0},   // 68
    {0, 4, 57, 14, 0, 0},   // 69
    {0, 4, 58, 14, 0, 0},   // 70
    {0, 4, 59, 14, 0, 0},   // 71
    {0, 4, 17, 14, 0, 0},   // 72
    {0, 4, 18, 14, 0, 0},   // 73
    {33, 2, 0, 18, 0, 0},   // 74
    {34, 2, 0, 18, 0, 0},   // 75
    {0, 4, 16, 14, 0, 0},   // 76
    {0, 4, 15, 14, 0, 0},   // 77
    {30, 2, 0, 18, 0, 0},   // 78
    {31, 2, 0, 18, 0, 0},   // 79
    {0, 4, 10, 14, 0, 0},   // 80
    {0, 4, 11, 14, 0, 0},   // 81
    {0, 4, 12, 14, 10, 0},  // 82
    {0, 4, 13, 14, 0, 0},   // 83
    {0, 4, 14, 14, 0, 0},   // 84
    {0, 4, 14, 14, 0, 0},   // 85
    {0, 4, 54, 14, 0, 0},   // 86
    {57, 2, 0, 18, 0, 0},   // 87
    {0, 4, 53, 14, 0, 0},   // 88
    {10, 2, 0, 18, 0, 0},   // 89
    {19, 2, 0, 18, 0, 0},   // 90
    {0, 4, 49, 14, 0, 0},   // 91
    {0, 4, 50, 14, 0, 0},   // 92
    {0, 4, 51, 14, 0, 0},   // 93
    {0, 4, 52, 14, 0, 0},   // 94
    {0, 4, 47, 14, 0, 0},   // 95
    {0, 4, 48, 14, 0, 0},   // 96
    {40, 2, 0, 18, 0, 0},   // 97
    {41, 2, 0, 18, 0, 0},   // 98
    {55, 2, 0, 18, 0, 0},   // 99
    {0, 4, 10, 14, 0, 0},   // 100
    {0, 4, 11, 14, 0, 0},   // 101
    {1, 2, 0, 18, 0, 0},    // 102
    {2, 2, 0, 18, 0, 0},    // 103
    {3, 2, 0, 18, 0, 0},    // 104
    {4, 2, 0, 18, 0, 0},    // 105
    {6, 2, 0, 18, 0, 0},    // 106
    {7, 2, 0, 18, 0, 0},    // 107
    {0, 2, 0, 18, 0, 0},    // 108
    {9, 2, 0, 18, 0, 0},    // 109
    {10, 2, 0, 18, 0, 0},   // 110
    {11, 2, 0, 18, 0, 0},   // 111
    {12, 2, 0, 18, 0, 0},   // 112
    {13, 2, 0, 18, 0, 0},   // 113
    {14, 2, 0, 18, 0, 0},   // 114
    {15, 2, 0, 18, 0, 0},   // 115
    {0, 4, 0, 14, 0, 0},    // 116
    {0, 4, 1, 14, 0, 0},    // 117
    {0, 4, 2, 14, 0, 0},    // 118
    {0, 4, 3, 14, 0, 0},    // 119
    {0, 4, 4, 14, 0, 0},    // 120
    {0, 4, 5, 14, 0, 0},    // 121
    {0, 4, 6, 14, 0, 0},    // 122
    {0, 4, 7, 14, 0, 0},    // 123
    {0, 4, 8, 14, 0, 0},    // 124
    {0, 4, 9, 14, 0, 0},    // 125
    {0, 4, 10, 14, 0, 0},   // 126
    {0, 4, 11, 14, 0, 0},   // 127
    {0, 4, 12, 14, 0, 0},   // 128
    {0, 4, 13, 14, 0, 0},   // 129
    {0, 4, 14, 14, 0, 0},   // 130
    {0, 4, 15, 14, 0, 0},   // 131
    {0, 4, 16, 14, 0, 0},   // 132
    {0, 4, 17, 14, 0, 0},   // 133
    {0, 4, 18, 14, 0, 0},   // 134
    {0, 4, 19, 14, 0, 0},   // 135
    {0, 4, 20, 14, 0, 0},   // 136
    {0, 4, 21, 14, 0, 0},   // 137
    {0, 4, 22, 14, 0, 0},   // 138
    {0, 4, 23, 14, 0, 0},   // 139
    {0, 4, 24, 14, 0, 0},   // 140
    {0, 4, 25, 14, 0, 0},   // 141
    {0, 4, 26, 14, 0, 0},   // 142
    {0, 4, 27, 14, 0, 0},   // 143
    {0, 4, 28, 14, 0, 0},   // 144
    {0, 4, 29, 14, 0, 0},   // 145
    {5, 2, 0, 18, 0, 0},    // 146
    {47, 2, 0, 18, 0, 0},   // 147
    {0, 4, 30, 14, 0, 0},   // 148
    {0, 54, 128, 14, 0, 0}, // 149
    {0, 54, 129, 14, 0, 0}, // 150
    {0, 54, 130, 14, 0, 0}, // 151
    {0, 4, 34, 14, 0, 0},   // 152
    {0, 4, 35, 14, 0, 0},   // 153
    {0, 4, 36, 14, 0, 0},   // 154
    {0, 4, 37, 14, 0, 0},   // 155
    {0, 4, 38, 14, 0, 0},   // 156
    {0, 4, 39, 14, 0, 0},   // 157
    {0, 54, 131, 14, 0, 0}, // 158
    {0, 54, 132, 14, 0, 0}, // 159
    {0, 54, 133, 14, 0, 0}, // 160
    {0, 54, 134, 14, 0, 0}, // 161
    {0, 4, 44, 14, 0, 0},   // 162
    {0, 4, 45, 14, 0, 0},   // 163
    {0, 54, 127, 14, 0, 0}, // 164
    {26, 2, 0, 18, 0, 0},   // 165
    {39, 2, 0, 18, 0, 0},   // 166
    {0, 0, 0, 0, 0, 0},     // 167
    {0, 4, 11, 14, 0, 0},   // 168
    {0, 4, 12, 14, 0, 0},   // 169
    {0, 4, 13, 14, 0, 0},   // 170
    {0, 4, 14, 14, 0, 0},   // 171
    {20, 2, 0, 18, 0, 0},   // 172
    {0, 4, 15, 14, 0, 0},   // 173
    {0, 4, 12, 14, 0, 0},   // 174
    {0, 4, 13, 14, 0, 0},   // 175
    {0, 0, 0, 0, 0, 0},     // 176
    {0, 0, 0, 14, 0, 0},    // 177
    {0, 0, 0, 14, 0, 0},    // 178
    {0, 60, 0, 14, 0, 0},   // 179
    {0, 62, 0, 14, 0, 0},   // 180
    {0, 0, 0, 14, 0, 0},    // 181
    {0, 0, 0, 14, 0, 0},    // 182
    {0, 0, 0, 14, 0, 0},    // 183
    {0, 0, 0, 14, 0, 0},    // 184
    {35, 2, 0, 18, 0, 0},   // 185
    {37, 2, 0, 18, 0, 0},   // 186
    {0, 4, 65, 14, 0, 0},   // 187
    {0, 4, 66, 14, 0, 0},   // 188
    {0, 4, 67, 14, -3, 0},  // 189
    {0, 4, 68, 14, 0, 0},   // 190
    {0, 4, 69, 14, 0, 0},   // 191
    {0, 4, 70, 14, 0, 0},   // 192
    {0, 4, 71, 14, 0, 0},   // 193
    {0, 4, 72, 14, 0, 0},   // 194
    {0, 4, 73, 14, 0, 0},   // 195
    {0, 4, 74, 14, 0, 0},   // 196
    {0, 4, 75, 14, 0, 0},   // 197
    {60, 2, 0, 18, 0, 0},   // 198
    {36, 2, 0, 18, 0, 0},   // 199
    {0, 4, 76, 14, 0, 0},   // 200
    {0, 4, 77, 14, 10, 0},  // 201
    {0, 4, 78, 14, 0, 0},   // 202
    {24, 2, 0, 18, 0, 0},   // 203
    {0, 54, 103, 14, 0, 0}, // 204
    {0, 54, 104, 14, 0, 0}, // 205
    {0, 54, 105, 14, 0, 0}, // 206
    {0, 54, 106, 14, 0, 0}, // 207
    {0, 54, 107, 14, 0, 0}, // 208
    {0, 54, 108, 14, 0, 0}, // 209
    {0, 4, 23, 14, 1, 0},   // 210
    {0, 4, 79, 14, -3, 0},  // 211
    {0, 4, 80, 14, 0, 0},   // 212
    {0, 4, 81, 14, 0, 0},   // 213
    {0, 4, 82, 14, 0, 0},   // 214
    {0, 4, 83, 14, 0, 0},   // 215
    {0, 4, 17, 14, 0, 0},   // 216
    {0, 54, 109, 14, 0, 0}, // 217
    {0, 54, 110, 14, 0, 0}, // 218
    {0, 54, 111, 14, 0, 0}, // 219
    {0, 54, 112, 14, 0, 0}, // 220
    {0, 54, 113, 14, 0, 0}, // 221
    {0, 54, 114, 14, 0, 0}, // 222
    {0, 54, 115, 14, 0, 0}, // 223
    {0, 54, 116, 14, 0, 0}, // 224
    {0, 54, 117, 14, 0, 0}, // 225
    {0, 54, 118, 14, 0, 0}, // 226
    {20, 2, 0, 18, 0, 0},   // 227
    {22, 2, 0, 18, 0, 0},   // 228
    {0, 54, 122, 14, 0, 0}, // 229
    {0, 54, 123, 14, 0, 0}, // 230
    {0, 54, 124, 14, 0, 0}, // 231
    {0, 54, 125, 14, 0, 0}, // 232
    {0, 54, 126, 14, 0, 0}, // 233
    {0, 52, 14, 0, 0, 0},   // 234
    {0, 52, 16, 0, 0, 0},   // 235
    {0, 99, 0, 0, 0, 0},    // 236
}};

// One step of an interaction (the game's DoorAnim, include/hosp.h): the
// animations (a kStaffAnimStarts or kPatientAnimStarts column) the thing and
// its partner play, 0 for none, and the times the step plays. An anim of -1
// ends a sequence, -99 the table.
struct InteractionStep {
    int16_t anim;
    int16_t partner_anim;
    uint8_t count;
};

// Copied from the game's interaction (src/tables.cpp).
constexpr std::array<InteractionStep, 93> kInteractions = {{
    {-1, 0, 0},   // 0
    {16, 17, 1},  // 1
    {-1, 0, 0},   // 2
    {17, 0, 1},   // 3
    {0, 16, 1},   // 4
    {-1, 0, 0},   // 5
    {18, 16, 1},  // 6
    {0, 17, 1},   // 7
    {16, 0, 1},   // 8
    {-1, 0, 0},   // 9
    {15, 12, 1},  // 10
    {0, 12, 3},   // 11
    {-1, 0, 0},   // 12
    {0, 12, 3},   // 13
    {-1, 0, 0},   // 14
    {4, 9, 5},    // 15
    {4, 0, 5},    // 16
    {11, 0, 5},   // 17
    {-1, 0, 0},   // 18
    {12, 9, 4},   // 19
    {11, 0, 4},   // 20
    {12, 0, 4},   // 21
    {14, 0, 4},   // 22
    {11, 0, 4},   // 23
    {-1, 0, 0},   // 24
    {22, 35, 1},  // 25
    {23, 35, 1},  // 26
    {25, 35, 1},  // 27
    {26, 38, 10}, // 28
    {27, 38, 1},  // 29
    {28, 39, 1},  // 30
    {0, 36, 5},   // 31
    {0, 40, 1},   // 32
    {-1, 0, 0},   // 33
    {21, 0, 1},   // 34
    {0, 22, 1},   // 35
    {0, 23, 1},   // 36
    {23, 0, 1},   // 37
    {0, 24, 1},   // 38
    {0, 25, 1},   // 39
    {23, 0, 1},   // 40
    {0, 26, 1},   // 41
    {0, 23, 1},   // 42
    {21, 0, 1},   // 43
    {0, 27, 1},   // 44
    {0, 18, 3},   // 45
    {-1, 0, 0},   // 46
    {23, 0, 1},   // 47
    {0, 19, 1},   // 48
    {0, 20, 2},   // 49
    {23, 0, 1},   // 50
    {0, 21, 1},   // 51
    {0, 18, 2},   // 52
    {23, 0, 1},   // 53
    {0, 19, 1},   // 54
    {0, 20, 2},   // 55
    {23, 0, 1},   // 56
    {0, 21, 1},   // 57
    {0, 18, 2},   // 58
    {-1, 0, 0},   // 59
    {23, 0, 1},   // 60
    {0, 19, 1},   // 61
    {0, 20, 4},   // 62
    {0, 21, 1},   // 63
    {0, 18, 3},   // 64
    {21, 0, 1},   // 65
    {0, 22, 1},   // 66
    {0, 23, 1},   // 67
    {23, 0, 1},   // 68
    {0, 24, 1},   // 69
    {0, 25, 4},   // 70
    {0, 26, 1},   // 71
    {0, 23, 3},   // 72
    {21, 0, 1},   // 73
    {0, 27, 1},   // 74
    {0, 18, 1},   // 75
    {-1, 0, 0},   // 76
    {25, 0, 1},   // 77
    {26, 65, 10}, // 78
    {27, 0, 1},   // 79
    {-1, 0, 0},   // 80
    {25, 0, 1},   // 81
    {26, 69, 3},  // 82
    {27, 0, 1},   // 83
    {-1, 0, 0},   // 84
    {25, 0, 1},   // 85
    {26, 72, 1},  // 86
    {0, 73, 15},  // 87
    {27, 0, 1},   // 88
    {-1, 0, 0},   // 89
    {58, 87, 25}, // 90
    {-1, 0, 0},   // 91
    {-99, 0, 0},  // 92
}};

// Where each interaction sequence starts in kInteractions, as
// setup_interactions (0x54d60) records them in statvar.interaction_start.
constexpr std::array<uint8_t, 17> kInteractionStarts = [] {
    std::array<uint8_t, 17> starts{};
    size_t seq = 0;
    for (size_t i = 0; kInteractions[i].anim != -99; i++)
        if (kInteractions[i].anim == -1)
            starts[++seq] = uint8_t(i + 1);
    return starts;
}();

// The interaction sequences a step of action 36 picks from at random, -1
// ending a list. Copied from the game's group_ia (src/cpxtable.cpp).
constexpr std::array<std::array<int8_t, 5>, 1> kGroupInteractions = {{{9, 10, 11, -1, -1}}};

// Object states (the game's OS_*, include/enums.h).
constexpr uint8_t kOsStatic = 0x02;
constexpr uint8_t kOsInUse = 0x04;
constexpr uint8_t kOsDoubleDoorSwings = 0x10;
constexpr uint8_t kOsRemoved = 0x12;

// Object kind of a single door (OBJ_DOOR), and the kStaffAnimStarts columns
// of going through a single or double door.
constexpr uint8_t kObjDoor = 3;
constexpr size_t kAnimSingleDoor = 6;
constexpr size_t kAnimDoubleDoor = 8;
// The starts of the other half of a double door swinging open (by the
// facing of who opens it) and of a door swinging shut.
constexpr uint16_t kAnimSwingOpen = 0x3f8;
constexpr uint16_t kAnimSwingClose = 0x400;
// MapCell.flags a door in use sets on its cell.
constexpr uint8_t kCellDoorInUse = 0x10;
// RoomInfo.state of a room being furnished or edited (RS_EDITED), and the
// room type of an operating theatre (ROOM_THEATRE).
constexpr int8_t kRoomEdited = 4;
constexpr uint8_t kRoomTheatre = 10;

// Patient state (the game's PS_*) set by the staff's complexes.
constexpr uint8_t kPsTwoPeepsInteract = 0x22;

// MapCell.flags: something on the cell covers more than one cell.
constexpr uint8_t kCellBig = 0x20;

// The animation columns of kStaffAnimStarts with a name in the game.
constexpr size_t kAnimWalk = 0;
constexpr size_t kAnimStand = 5;

// The game's around (src/tables.cpp): the cell offsets of the four
// neighbours.
constexpr std::array<int16_t, 4> kAround = {-128, 1, 128, -1};

struct Run {
    World &world;
    const AnimData &anims;
    const PersonComplexes &person;
    const StaffComplexes &staff;
    uint32_t game_turn;
};

uint16_t cell_of(const Thing &t)
{
    return uint16_t(t.x.tile + (t.y.tile << 7));
}

// The use cell of an object a member of staff uses: handymen use 2, others
// 0 (and leave to 5 or 3).
size_t use_side(const Thing &staff)
{
    return staff.subtype == kStaffHandyman ? 2 : 0;
}

void play_staff_anim(const AnimData &anims, Thing &s, size_t column)
{
    play_frame(s, anims, kStaffAnimStarts[s.subtype][column]);
}

// The game's PLAY_FRAME after person_head_for: faces the step's direction,
// if any, and starts walking.
void head_off(const Run &r, uint16_t id)
{
    Thing &s = r.world.things[id];
    int dir = person_head_for(r.world, id);
    if (dir >= 0 && dir < 4)
        s.facing = uint8_t(dir);
    play_staff_anim(r.anims, s, kAnimWalk);
}

// can_i_go_near_point (0x1ac9c): of the four cells beside `target`, the
// nearest to the person that a path reaches (can_i_get_to); 0 if none.
uint16_t can_i_go_near_point(const World &world, const Thing &person, uint16_t target)
{
    uint16_t best = 0xffff;
    uint16_t result = 0;
    uint16_t pos = cell_of(person);
    for (int16_t offset : kAround) {
        uint16_t cand = uint16_t(target + offset);
        auto dist = uint16_t(std::abs(pos % 128 - cand % 128) + std::abs(pos / 128 - cand / 128));
        if (dist >= best)
            continue;
        OrthonavPath path;
        orthonav_init(world, path, person.x.tile, person.y.tile, cand % 128, cand / 128);
        if (path.count != 0) {
            best = dist;
            result = cand;
        }
    }
    return result;
}

void fail(const Run &r, uint16_t id)
{
    Thing &s = r.world.things[id];
    if (s.resume_state) {
        s.state = s.resume_state;
        s.complex_pc = s.complex_queued = s.complex_queued2 = 0;
        s.complex_action = s.complex_resume = 0;
        play_staff_anim(r.anims, s, kAnimStand);
        if (s.subtype == kStaffHandyman)
            thingpage_cancel_my_response(r.world.repair_pager, id);
    } else {
        s.state = kStaffComplexRetry;
    }
}

uint8_t process(const Run &r, uint16_t id);

// check_room_mutex_on_entry (0x41e44): claims the room the member of staff
// enters by its door (at exit_tile) for them: whether they hold it already
// or take a free place (two in an operating theatre); true for the
// corridor.
bool check_room_mutex_on_entry(World &world, uint16_t id)
{
    uint16_t room_id = room_at(world, world.things[id].exit_tile);
    if (!room_id)
        return true;
    RoomInfo &room = world.rooms[room_id];
    if (id == room.staff[0] || id == room.staff[1])
        return true;
    if (room.type == kRoomTheatre) {
        for (uint16_t &holder : room.staff) {
            if (!holder) {
                holder = id;
                return true;
            }
        }
        return false;
    }
    if (room.staff[0])
        return false;
    room.staff[0] = id;
    return true;
}

// clear_my_mutex (0x2bb58): the member of staff no longer holds their room.
void clear_my_mutex(World &world, uint16_t id)
{
    const Thing &s = world.things[id];
    if (!s.room)
        return;
    RoomInfo &room = world.rooms[s.room];
    if (room.staff[0] == id)
        room.staff[0] = 0;
    else if (room.staff[1] == id)
        room.staff[1] = 0;
}

// The starting frame `start` (an AnimData::starts index), without
// check_frame, as the game sets a door's.
void set_start(Thing &t, const AnimData &anims, size_t start)
{
    if (start >= anims.starts.size())
        return;
    t.sprite = uint16_t(anims.starts[start].frame);
    t.flip = uint8_t(anims.starts[start].flip);
}

// rest_staff (0x2be14): tiredness down by `amount` (less when very tired,
// at least gbv.recovery_minimum), happiness up by 1.
void rest_staff(World &world, Thing &s, int16_t amount)
{
    static const Balance statics;
    const Balance::Gbv &gbv = world.settings ? world.settings->gbv : statics.gbv;
    if (s.tiredness > gbv.recovery_factor / 2) {
        amount = int16_t(amount * gbv.recovery_factor / s.tiredness);
        if (amount < int16_t(gbv.recovery_minimum))
            amount = int16_t(gbv.recovery_minimum);
    }
    s.tiredness = int16_t(s.tiredness - amount);
    s.happiness = uint16_t(std::clamp(int(int16_t(s.happiness + 1)), 0, 100));
}

// complex_success (0x423c8) for a member of staff: a rest on a sofa, at
// snooker or a video game (kinds 18, 20, 22) and a use of their object
// (24, Thing.times_used). Research (8, 10, 14, 16) and an autopsy (12) are
// not ported.
void staff_success(World &world, Thing &s, uint16_t kind)
{
    static const Balance statics;
    const Balance::Gbv &gbv = world.settings ? world.settings->gbv : statics.gbv;
    switch (kind) {
    case 18:
        rest_staff(world, s, int16_t(gbv.rest_sofa));
        break;
    case 20:
        rest_staff(world, s, int16_t(gbv.rest_snooker));
        break;
    case 22:
        rest_staff(world, s, int16_t(gbv.rest_game));
        break;
    case 24:
        world.things[s.object].times_used++;
        break;
    default:
        break;
    }
    s.happiness = uint16_t(std::clamp(int(int16_t(s.happiness)), 0, 100));
}

// process_complex_either (0x3e6c0).
uint8_t process_either(const Run &r, uint16_t id)
{
    switch (r.world.things[id].type) {
    case kThingStaff:
        return process(r, id);
    case kThingPatient:
        return process_person_complex(r.world, r.anims, r.person, id, r.game_turn);
    default:
        return 0;
    }
}

// setup_next_interaction (0x549bc): starts the next step of `id`'s
// interaction (or the current one again while it repeats), playing its
// animations on `id` and its slave; at the sequence's end clears both and
// goes on with `id`'s complex.
void setup_next_interaction(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &w = world.things[id];
    uint8_t step;
    if (w.interact_repeat > 0) {
        step = w.seq_index;
        w.interact_repeat--;
    } else {
        step = ++w.seq_index;
        w.interact_repeat = int8_t(kInteractions[step].count - 1);
    }
    int16_t anim = kInteractions[step].anim;
    if (anim == -1) {
        w.seq_index = 0;
        w.interact_anim = 0;
        if (w.slave) {
            Thing &partner = world.things[w.slave];
            partner.seq_index = 0;
            partner.interact_anim = 0;
        }
        process_either(r, id);
        return;
    }
    if (anim) {
        w.interact_anim = 1;
        if (w.type == kThingStaff)
            play_frame(w, r.anims, kStaffAnimStarts[w.subtype][size_t(anim)]);
        else
            play_frame(w, r.anims, kPatientAnimStarts[w.subtype][size_t(anim)]);
    } else {
        w.interact_anim = 0;
    }
    if (w.slave) {
        Thing &partner = world.things[w.slave];
        anim = kInteractions[step].partner_anim;
        if (anim) {
            partner.interact_anim = 1;
            if (partner.type == kThingPatient)
                play_frame(partner, r.anims, kPatientAnimStarts[partner.subtype][size_t(anim)]);
            else
                play_frame(partner, r.anims, kStaffAnimStarts[partner.subtype][size_t(anim)]);
        } else {
            partner.interact_anim = 0;
        }
    }
}

void start_interaction(const Run &r, uint16_t id, uint8_t seq)
{
    Thing &w = r.world.things[id];
    switch (w.type) {
    case kThingPatient:
        w.state = kPsTwoPeepsInteract;
        break;
    case kThingStaff:
        w.state = kStaffTwoPeepsInteract;
        break;
    default:
        return;
    }
    w.seq_index = uint8_t(kInteractionStarts[seq] - 1);
    w.interact_repeat = 0;
    setup_next_interaction(r, id);
}

// Advances an interacting thing's animation, noting in interact_anim when
// it has played through.
void interaction_frame(const Run &r, Thing &t)
{
    if (t.interact_anim != 0 && r.game_turn % t.anim_speed == 0) {
        t.sprite = r.anims.frames[t.sprite].next;
        if (r.anims.frames[t.sprite].flags & 1)
            t.interact_anim = 2;
    }
}

// merge_slave (0x4042c).
void merge_slave(World &world, uint16_t id)
{
    Thing &e = world.things[id];
    if (!e.slave)
        return;
    Thing &slave = world.things[e.slave];
    if (slave.state != kPsSlaveReady)
        return;
    slave.state = kPsMerged;
    if (e.object) {
        const Thing &obj = world.things[e.object];
        for (size_t i = 10; i < 13; i++)
            e.layers[i] = obj.layers[i];
    }
    for (size_t i = 0; i < 5; i++)
        e.layers[i] = slave.layers[i];
    slave.object = e.object;
}

uint8_t process(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &e = world.things[id];
    bool run_things = false;
    if (e.complex_pc == 0)
        return 2;
    uint8_t op = r.staff.at(e.complex_pc);
    if (op == 0) {
        e.complex_pc = 0;
        return 2;
    }
    uint16_t tile = cell_of(e);
    const ComplexStep &entry = kStaffComplex[op];
    uint16_t obj_id = 0;
    if (entry.object) {
        obj_id = find_nearest_free_object(world, id, entry.object);
        if (obj_id == 0) {
            fail(r, id);
            return 0;
        }
    }
    if (entry.state)
        e.state = entry.state;
    size_t side = use_side(e);
    switch (entry.action) {
    case 56: {
        // staff_go_near_object (0x42748).
        const Thing &obj = world.things[e.object];
        uint16_t pos = can_i_go_near_point(world, e, cell_of(obj));
        if (!pos) {
            fail(r, id);
            return 0;
        }
        set_destination_coords(world, id, pos);
        head_off(r, id);
        e.countdown = 0;
        break;
    }
    case 2:
        e.object = obj_id;
        [[fallthrough]];
    case 44:
        set_destination_coords(world, id, world.things[e.object].use_pos[side]);
        head_off(r, id);
        e.countdown = 0;
        break;
    case 4:
        play_staff_anim(r.anims, e, entry.anim);
        break;
    case 54:
        if (e.slave)
            play_frame(e, r.anims, kPatientAnimStarts[world.things[e.slave].subtype][entry.anim]);
        break;
    case 6: {
        Thing &obj = world.things[e.object];
        if (obj.in_use == 1 || obj.state == kOsRemoved || obj.type != kThingObject || obj.use_pos[side] != tile) {
            fail(r, id);
            return 3;
        }
        obj.in_use = 1;
        obj.user = id;
        e.facing = obj.facing;
        if (obj.use_style == 1 || obj.use_style == 2) {
            e.exit_tile = obj.use_pos[side + 3];
            move_node(world, id, obj.use_x, obj.use_y);
            obj.state = kOsInUse;
            e.depth = obj.depth;
            if (kObjectUse[obj.subtype].overlay_anim) {
                obj.saved_depth = obj.depth;
                obj.depth = 8;
            }
            world.map[cell_of(e)].flags |= kCellBig;
        }
        for (size_t i = 10; i < 13; i++)
            e.layers[i] = obj.layers[i];
        break;
    }
    case 8: {
        Thing &obj = world.things[e.object];
        obj.user = 0;
        e.object = 0;
        obj.in_use = 0;
        if (obj.use_style == 2 || obj.use_style == 1) {
            adjust_big_cell_flag(world, tile);
            uint16_t leave = obj.use_pos[side + 3];
            move_node(world, id, {0x80, uint8_t(leave % kMapSize)}, {0x80, uint8_t(leave / kMapSize)});
            e.depth = 0;
            if (kObjectUse[obj.subtype].overlay_anim) {
                obj.depth = obj.saved_depth;
                obj.saved_depth = 0;
            }
            obj.state = kOsStatic;
            if (obj.use_style == 2 && e.slave) {
                move_node(world, e.slave, {0x80, uint8_t(obj.use_pos[4] % kMapSize)},
                          {0x80, uint8_t(obj.use_pos[4] / kMapSize)});
                world.things[e.slave].object = 0;
            }
        }
        play_staff_anim(r.anims, e, kAnimStand);
        e.layers[10] = e.layers[11] = e.layers[12] = 0;
        break;
    }
    case 10:
        if (e.object)
            play_frame(world.things[e.object], r.anims, entry.anim);
        break;
    case 16: {
        const GroupAnim &group = kGroupAnims[entry.anim];
        auto choice = int8_t(old_newrand(world.seed, "process_staff_complex") % uint32_t(group.count));
        play_staff_anim(r.anims, e, size_t(group.anims[size_t(choice)]));
        break;
    }
    case 14:
        if (uint16_t door = find_door(world, id))
            e.facing = kOppositeAngle[world.things[door].facing & 3];
        break;
    case 20: {
        uint16_t door_id = find_door(world, id);
        if (!door_id) {
            fail(r, id);
            return 4;
        }
        Thing &door = world.things[door_id];
        bool double_door = door.subtype != kObjDoor;
        if (door.in_use == 1) {
            e.state = kStaffComplexRetry;
            return 3;
        }
        e.facing = e.room ? door.facing : kOppositeAngle[door.facing & 3];
        e.exit_tile = tile == door.use_pos[0] ? door.use_pos[3] : door.use_pos[0];
        if (entry.anim == 1 && !check_room_mutex_on_entry(world, id)) {
            fail(r, id);
            return 4;
        }
        const RoomInfo &room = world.rooms[door.room_id];
        if (e.room == 0 && (room.being_edited || room.state == kRoomEdited ||
                            (door.owner == 0 && door.room_id == world.building.build.room))) {
            fail(r, id);
            return 4;
        }
        play_staff_anim(r.anims, e, double_door ? kAnimDoubleDoor : kAnimSingleDoor);
        door.in_use = 1;
        e.object = door_id;
        door.user = id;
        move_node(world, id, door.x, door.y);
        door.state = kOsInUse;
        MapCell &cell = world.map[cell_of(door)];
        door.door_cell_flag = cell.flags & kCellDoorInUse ? 1 : 0;
        cell.flags |= kCellDoorInUse;
        if (double_door) {
            Thing &other = world.things[door.door_partner];
            other.state = kOsDoubleDoorSwings;
            set_start(other, r.anims, kAnimSwingOpen + e.facing);
            other.anim_speed = e.anim_speed;
        }
        break;
    }
    case 22: {
        clear_my_mutex(world, id);
        Thing &door = world.things[e.object];
        door.user = 0;
        e.object = 0;
        MapPos x{0x80, uint8_t(e.exit_tile % kMapSize)};
        MapPos y{0x80, uint8_t(e.exit_tile / kMapSize)};
        e.dest_x = x.tile;
        e.dest_y = y.tile;
        move_node(world, id, x, y);
        move_person_in_room_list(world, id, e.exit_tile);
        if (e.room) {
            world.rooms[e.room].staff_free = 0;
            pager_arrival(world, id);
        }
        MapCell &cell = world.map[cell_of(door)];
        if (!door.door_cell_flag)
            cell.flags &= uint8_t(~kCellDoorInUse);
        if (door.subtype == kObjDoor) {
            door.in_use = 0;
            door.state = kOsStatic;
        } else {
            door.state = kOsDoubleDoorSwings;
            set_start(door, r.anims, kAnimSwingClose + e.facing);
        }
        break;
    }
    case 24:
        e.complex_pc++;
        e.complex_resume = uint16_t(r.staff.at(e.complex_pc) * 10);
        break;
    case 26:
        e.complex_pc = uint16_t(e.complex_queued - 1);
        e.complex_queued = 0;
        break;
    case 28:
        e.complex_pc++;
        start_interaction(r, id, uint8_t(r.staff.at(e.complex_pc) + 1));
        break;
    case 30:
        merge_slave(world, id);
        break;
    case 36: {
        const std::array<int8_t, 5> &list = kGroupInteractions[entry.anim];
        uint8_t n = 0;
        while (n < 5 && list[n] != -1)
            n++;
        start_interaction(r, id, uint8_t(list[old_newrand(world.seed, "process_staff_complex") % n]));
        break;
    }
    case 38:
    case 40: {
        if (!e.slave) {
            fail(r, id);
            return 0;
        }
        e.complex_pc++;
        Thing &patient = world.things[e.slave];
        patient.next_state = kPsSlaveReady;
        set_complex(world, e.slave, uint16_t(r.staff.at(e.complex_pc) * 10), 0);
        if (entry.action == 38)
            process_either(r, e.slave);
        break;
    }
    case 46:
        head_off(r, id);
        break;
    case 48:
        if (e.slave && e.object) {
            uint16_t leave = world.things[e.object].use_pos[4];
            move_node(world, e.slave, {0x80, uint8_t(leave % kMapSize)}, {0x80, uint8_t(leave / kMapSize)});
            world.things[e.slave].object = 0;
        }
        break;
    case 18:
        if (e.object) {
            const Thing &obj = world.things[e.object];
            int face = kObjectUse[obj.subtype].face_dirn;
            // Kinds whose face_dirn is -2 or -99 make the game read before
            // face_dirns; here they leave the facing.
            if (face >= 0 && size_t(face) < kFaceDirns.size()) {
                int8_t d = kFaceDirns[size_t(face)][obj.facing];
                if (d != -1) {
                    e.facing = uint8_t(d);
                    play_staff_anim(r.anims, e, kAnimStand);
                }
            }
        }
        break;
    case 50:
        run_things = true;
        break;
    case 58:
        e.subtype = uint8_t(entry.anim);
        break;
    case 52:
        staff_success(world, e, entry.anim);
        break;
    default:
        // 0 and 12 do nothing; 60 and 62 are not ported, see
        // process_staff_complex.
        break;
    }
    if (entry.repeat) {
        if (e.complex_repeat) {
            e.complex_repeat--;
            if (e.complex_repeat == 0)
                e.complex_pc++;
        } else if (entry.repeat < 0) {
            e.complex_repeat = uint8_t(old_newrand(world.seed, "process_staff_complex") % uint32_t(-entry.repeat) + 1);
        } else {
            e.complex_repeat = uint8_t(entry.repeat);
        }
    } else {
        e.complex_pc++;
    }
    if (r.staff.at(e.complex_pc) == 0) {
        e.complex_pc = 0;
        uint16_t action = e.complex_queued;
        if (action) {
            uint16_t arg = uint16_t(e.queued_arg_x + (e.queued_arg_y << 7));
            e.complex_queued = e.complex_queued2;
            e.complex_queued2 = 0;
            e.queued_arg_x = e.queued2_arg_x;
            e.queued2_arg_x = 0;
            e.queued_arg_y = e.queued2_arg_y;
            e.queued2_arg_y = 0;
            if (set_complex(world, id, action, arg))
                process(r, id);
        } else if (e.next_state) {
            e.state = e.next_state;
            e.next_state = 0;
        }
    } else if (entry.chain) {
        process(r, id);
    }
    // The game runs process_things for the member of staff alone.
    if (run_things)
        staff_turn(world, r.anims, r.person, r.staff, id, r.game_turn);
    return 1;
}

} // namespace

bool load_staff_complexes(const std::filesystem::path &dir, StaffComplexes &out, std::string *error)
{
    auto data = load_data_file(dir, "data/s_comp.lex", error);
    if (!data)
        return false;
    for (uint8_t op : *data) {
        if (op >= kStaffComplex.size()) {
            if (error)
                *error = "data/s_comp.lex: unknown step " + std::to_string(op);
            return false;
        }
    }
    out.group = std::move(*data);
    return true;
}

uint8_t process_staff_complex(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                              const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn)
{
    return process({world, anims, person_complexes, staff_complexes, game_turn}, id);
}

void fail_staff_complex(World &world, const AnimData &anims, uint16_t id)
{
    static const PersonComplexes no_person;
    static const StaffComplexes no_staff;
    fail({world, anims, no_person, no_staff, 0}, id);
}

void start_new_interaction(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                           const StaffComplexes &staff_complexes, uint16_t id, uint8_t seq, uint32_t game_turn)
{
    start_interaction({world, anims, person_complexes, staff_complexes, game_turn}, id, seq);
}

bool staff_complex_turn(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                        const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn)
{
    const Run r{world, anims, person_complexes, staff_complexes, game_turn};
    Thing &s = world.things[id];
    switch (s.state) {
    case kStaffPlayComplexAnim:
        // play_complex_staff_anim.
        if (game_turn % s.anim_speed == 0) {
            uint16_t next = anims.frames[s.sprite].next;
            if (anims.frames[next].flags & 1)
                process(r, id);
            else
                s.sprite = next;
        }
        return true;
    case kStaffTwoPeepsInteract: {
        // two_peeps_interact.
        interaction_frame(r, s);
        Thing &partner = world.things[s.slave];
        if (s.slave)
            interaction_frame(r, partner);
        if (s.interact_anim != 1 && partner.interact_anim != 1)
            setup_next_interaction(r, id);
        return true;
    }
    case kStaffWaitForComplexSlave:
        // staff_wait_for_complex_slave.
        if (s.slave && world.things[s.slave].state == kPsSlaveReady)
            process(r, id);
        return true;
    case kStaffComplexRetry:
        // staff_complex_retry.
        if (game_turn % s.anim_speed == 0) {
            s.sprite = anims.frames[s.sprite].next;
            if (anims.frames[s.sprite].flags & 1)
                process(r, id);
        }
        return true;
    default:
        return false;
    }
}

} // namespace th
