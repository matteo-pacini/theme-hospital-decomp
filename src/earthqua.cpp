#include "hosp.h"

u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 with_margin);

extern "C" {
// demo: earthquake_table
s8 earthquake_table[16][2] = {
    { 4, 4 }, { 1, 2 }, { 3, 1 }, { 2, 1 }, { 0, -2 }, { -1, -2 }, { 4, -4 }, { 1, -2 },
    { -4, 4 }, { -3, -1 }, { -2, 0 }, { 0, 2 }, { -2, -1 }, { -4, -4 }, { 2, 0 }, { -1, 2 },
};
void ReleaseLoopedSample(u32 source, s16 sample);
void shake_screen(s8 dx, s8 dy);
}

// FUNCTION: 0x000a2f00
u8 earthquake(const s8 magnitude, const u8 chance)
{
    if (gamevar.ff4 == 1) {
        while (gamevar.ff2 <= magnitude) {
            if (old_newrand() % chance == 0) {
                gamevar.ff2++;
            } else {
                gamevar.ff3 += saferand() % 16;
                gamevar.ff3 = gamevar.ff3 % 16;
                shake_screen(gamevar.ff2 * earthquake_table[gamevar.ff3][0], gamevar.ff2 * earthquake_table[gamevar.ff3][1]);
                return 1;
            }
        }
        gamevar.ff4 = -1;
    }
    if (gamevar.ff4 == -1) {
        while (gamevar.ff2 >= 0) {
            if (old_newrand() % chance == 0) {
                gamevar.ff2--;
            } else {
                gamevar.ff3 += saferand() % 16;
                gamevar.ff3 = gamevar.ff3 % 16;
                shake_screen(gamevar.ff2 * earthquake_table[gamevar.ff3][0], gamevar.ff2 * earthquake_table[gamevar.ff3][1]);
                return 1;
            }
        }
    }
    ReleaseLoopedSample(0xc, 0x48);
    NRAdcounts.unknowna_ = gamevar.ff0;
    gamevar.ff0 = 0;
    gamevar.ff3 = 0;
    gamevar.ff4 = 1;
    return 1;
}

// FUNCTION: 0x000a30f0
void shake_screen(s8 dx, s8 dy)
{
    if (!check_if_point_beyond_scroll_limit(window.f0[0] + dx, window.fc[0] + dy, 0)) {
        window.f0[0] += dx;
        window.fc[0] += dy;
    }
}
