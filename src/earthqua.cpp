#include "hosp.h"

u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 n);

extern "C" {
extern s8 D_00145338[16][2];
void ReleaseLoopedSample(u32 source, s16 sample);
void fn_000a30f0(s8 dx, s8 dy);
void fn_000a3140(void);
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
                fn_000a30f0(gamevar.ff2 * D_00145338[gamevar.ff3][0], gamevar.ff2 * D_00145338[gamevar.ff3][1]);
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
                fn_000a30f0(gamevar.ff2 * D_00145338[gamevar.ff3][0], gamevar.ff2 * D_00145338[gamevar.ff3][1]);
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
void fn_000a30f0(s8 dx, s8 dy)
{
    if (!check_if_point_beyond_scroll_limit(window.f0[0] + dx, window.fc[0] + dy, 0)) {
        window.f0[0] += dx;
        window.fc[0] += dy;
    }
}

// FUNCTION: 0x000a3140
void fn_000a3140(void)
{
}
