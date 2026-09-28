#include "hosp.h"

extern "C" {
extern u8 D_0015f3f7;
extern u8 D_0015f3f8;
extern s32 D_00188c38;
extern u8 D_001553c7;
void fn_000c9be0(int x0, int y0, int x1, int y1, u8 color);
}

void draw_cell_no_editor(s32 x, s32 y, u8 colour);
u32 compute_plant_frame(Thing *w);

// NONMATCHING: register choice
// FUNCTION: 0x000b10b0
void draw_cell_no(s32 cell, u8 colour)
{
    s32 b;
    s32 a;
    s32 y;
    s32 x;
    s32 col;
    s32 row;

    row = cell / 128;
    col = cell % 128;
    x = col * (D_0015f3f8 / 2) - D_0015f3f8 / 2 * row - window.f0[window.unknown2eb] + window.f90[window.unknown2eb]
        + D_0015f3f8 / 2;
    y = col * (D_0015f3f7 / 2) + D_0015f3f7 / 2 * row - window.fc[window.unknown2eb] + window.f9c[window.unknown2eb]
        - D_0015f3f7;
    find_common_window(0, 0, D_00188c34, lbDisplay.unknown1c - 0x5c, window.zone_x[window.unknown2eb],
                       window.zone_y[window.unknown2eb], window.zone_w[window.unknown2eb], window.zone_h[window.unknown2eb]);
    find_common_window(0, 0, D_00188c34, D_00188c38, window.zone_x[window.unknown2eb], window.zone_y[window.unknown2eb],
                       window.zone_w[window.unknown2eb], window.zone_h[window.unknown2eb]);
    find_common_window(info.unknown68, info.unknown6c, info.unknown70, info.unknown74, window.unknown2c3, window.unknown2c7,
                       window.unknown2cb, window.unknown2cf);
    if (info.unknown78 == 0) {
        LbScreenSetGraphicsWindow(info.unknown68, info.unknown6c, info.unknown70 - info.unknown68,
                                  info.unknown74 - info.unknown6c);
        x -= info.unknown68;
        y -= info.unknown6c;
        a = x - D_0015f3f8 / 2;
        b = y + D_0015f3f7;
        draw_cell_no_editor(a, b, colour);
    }
}

// FUNCTION: 0x000b1318
void draw_cell_no_editor(s32 x, s32 y, u8 colour)
{
    if (-(D_0015f3f8 / 2) <= x && D_0015f3f8 / 2 + D_00188c34 > x && -D_0015f3f7 <= y && lbDisplay.unknown1c - 0x5c > y) {
        fn_000c9be0(x, y, (D_0015f3f8 - 1) / 2 + x - 1, y + (D_0015f3f7 - 1) / 2, colour);
        fn_000c9be0((D_0015f3f8 - 1) / 2 + x, y + (D_0015f3f7 - 1) / 2, x, y + (D_0015f3f7 - 1), colour);
        fn_000c9be0(x, D_0015f3f7 - 1 + y, x - (D_0015f3f8 - 1) / 2, y + (D_0015f3f7 - 1) / 2, colour);
        fn_000c9be0(x - (D_0015f3f8 - 1) / 2, y + (D_0015f3f7 - 1) / 2, x, y, colour);
        set_all_refresh_zones_in_window(x - (D_0015f3f8 - 1) / 2, y, (D_0015f3f8 - 1) / 2 + x - 1, D_0015f3f7 - 1 + y, 3);
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x000b1c68
u32 get_frame_for_placing_bomb(void)
{
    u8 i;
    BuildState *bs;
    Thing *w;

    bs = &allhosps[D_001553c7].build;
    for (i = 0; i < 20; i++) {
        w = &thing[bs->objects[i]];
        if (w > thing) {
            if (w->subtype == 0x11 && w->owner == player)
                return start[w->f64 + 0x9bf].frame;
        }
    }
    return start[0x9bf].frame;
}

// FUNCTION: 0x000b1d44
u32 get_frame_for_placing_plant(void)
{
    u8 i;
    BuildState *bs;
    Thing *w;

    bs = &allhosps[D_001553c7].build;
    for (i = 0; i < 20; i++) {
        w = &thing[bs->objects[i]];
        if (w > thing) {
            if (w->subtype == 0x2d && w->owner == player)
                return compute_plant_frame(w);
        }
    }
    return start[0x3cf].frame;
}
