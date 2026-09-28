#include "hosp.h"

struct QuakeState {
    u8 unknown0[2];
    u8 state;
    u8 magnitude;
};

u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 n);

extern "C" {
extern QuakeState D_0015532d;
extern s8 D_00155332;
extern u8 D_00155333;
extern s8 D_00155334;
extern s8 D_00145338[16][2];
void ReleaseLoopedSample(u32 source, s16 sample);
void fn_000a30f0(s8 dx, s8 dy);
void fn_000a3140(void);
}

// NONMATCHING: register choice
// FUNCTION: 0x000a2f00
u8 earthquake(const s8 magnitude, const u8 chance)
{
    if (D_00155334 == 1) {
        while (D_00155332 <= magnitude) {
            if (old_newrand() % chance == 0) {
                D_00155332++;
            } else {
                D_00155333 += saferand() % 16;
                D_00155333 = D_00155333 % 16;
                fn_000a30f0(D_00155332 * D_00145338[D_00155333][0], D_00155332 * D_00145338[D_00155333][1]);
                return 1;
            }
        }
        D_00155334 = -1;
    }
    if (D_00155334 == -1) {
        while (D_00155332 >= 0) {
            if (old_newrand() % chance == 0) {
                D_00155332--;
            } else {
                D_00155333 += saferand() % 16;
                D_00155333 = D_00155333 % 16;
                fn_000a30f0(D_00155332 * D_00145338[D_00155333][0], D_00155332 * D_00145338[D_00155333][1]);
                return 1;
            }
        }
    }
    ReleaseLoopedSample(0xc, 0x48);
    NRAdcounts.unknowna_ = D_0015532d.magnitude;
    D_0015532d.magnitude = 0;
    D_00155333 = 0;
    D_00155334 = 1;
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
