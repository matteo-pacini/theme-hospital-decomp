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

// Runs one turn of the current earthquake: its strength (gamevar.quake_amplitude) builds
// up to magnitude, then dies down, each step taken with a 1 in chance per
// call; otherwise the main view is shaken by the strength. When it has died
// out it stops the quake sound, records gamevar.quake_strength in NRAdcounts and resets
// for the next quake. Always returns 1.
// FUNCTION: 0x000a2f00
u8 earthquake(const s8 magnitude, const u8 chance)
{
    if (gamevar.quake_ramp == 1) {
        while (gamevar.quake_amplitude <= magnitude) {
            if (old_newrand() % chance == 0) {
                gamevar.quake_amplitude++;
            } else {
                gamevar.quake_shake_index += saferand() % 16;
                gamevar.quake_shake_index = gamevar.quake_shake_index % 16;
                shake_screen(gamevar.quake_amplitude * earthquake_table[gamevar.quake_shake_index][0], gamevar.quake_amplitude * earthquake_table[gamevar.quake_shake_index][1]);
                return 1;
            }
        }
        gamevar.quake_ramp = -1;
    }
    if (gamevar.quake_ramp == -1) {
        while (gamevar.quake_amplitude >= 0) {
            if (old_newrand() % chance == 0) {
                gamevar.quake_amplitude--;
            } else {
                gamevar.quake_shake_index += saferand() % 16;
                gamevar.quake_shake_index = gamevar.quake_shake_index % 16;
                shake_screen(gamevar.quake_amplitude * earthquake_table[gamevar.quake_shake_index][0], gamevar.quake_amplitude * earthquake_table[gamevar.quake_shake_index][1]);
                return 1;
            }
        }
    }
    ReleaseLoopedSample(0xc, 0x48);
    NRAdcounts.last_quake_size = gamevar.quake_strength;
    gamevar.quake_strength = 0;
    gamevar.quake_shake_index = 0;
    gamevar.quake_ramp = 1;
    return 1;
}

// Moves the main view by dx,dy unless that would pass the scroll limit.
// FUNCTION: 0x000a30f0
void shake_screen(s8 dx, s8 dy)
{
    if (!check_if_point_beyond_scroll_limit(window.scroll_x[0] + dx, window.scroll_y[0] + dy, 0)) {
        window.scroll_x[0] += dx;
        window.scroll_y[0] += dy;
    }
}
