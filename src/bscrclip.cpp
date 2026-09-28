#include "hosp.h"

extern "C" {
extern u8 D_001567c0[][16];
extern u8 D_0015f3f8;
}

void copy_region(const s16 x, const s16 y, const s16 dx, const s16 dy, const s16 w, const s16 h);
void copy_bscreen_window_to_wscreen_mk1(const s32 x0, const s32 y0, const s32 x1, const s32 y1, const s16 dx, const s16 dy);

// FUNCTION: 0x000adad0
void copy_bscreen_window_to_wscreen_mk1(const s32 x0, const s32 y0, const s32 x1, const s32 y1, const s16 dx, const s16 dy)
{
    s16 b;
    s16 a;
    s16 sx;
    s16 sy;
    s16 w;
    s16 h;
    s16 rx;
    s16 ry;
    s16 ex;
    s16 ey;

    a = 0;
    b = 0;
    sx = x0;
    sy = y0;
    w = x1 - x0;
    h = y1 - y0;
    rx = lbDisplay.unknown18 - (dx + x0);
    ry = lbDisplay.unknown1c - (dy + y0);
    ex = dx + x1 - lbDisplay.unknown18;
    ey = dy + y1 - lbDisplay.unknown1c;
    if (rx < 0)
        a = -rx;
    else
        sx += rx;
    if (ry < 0)
        b = -ry;
    else
        sy += ry;
    if ((s32)rx > (s32)w)
        rx = w;
    if ((s32)ry > (s32)h)
        ry = h;
    if ((s32)ex > (s32)w)
        ex = w;
    if ((s32)ey > (s32)h)
        ey = h;
    if (ry > 0 && dy + y0 < lbDisplay.unknown1c) {
        if (rx > 0 && dx + x0 < lbDisplay.unknown18)
            copy_region(dx + x0, dy + y0, x0, y0, rx, ry);
        if (ex > 0)
            copy_region(a, dy + y0, sx, y0, ex, ry);
    }
    if (ey > 0) {
        if (rx > 0 && dx + x0 < lbDisplay.unknown18)
            copy_region(dx + x0, b, x0, sy, rx, ey);
        if (ex > 0)
            copy_region(a, b, sx, sy, ex, ey);
    }
}

// FUNCTION: 0x000adcc8
void copy_region(const s16 x, const s16 y, const s16 dx, const s16 dy, const s16 w, const s16 h)
{
    u32 i;
    u8 *src;
    u8 *dst;

    i = 0;
    src = bscreen + y * lbDisplay.unknown18 + x;
    dst = lbDisplay.unknown4 + dy * D_00188c34 + dx;
    for (; i < h; i++, src += lbDisplay.unknown18, dst += D_00188c34)
        LbDrawBoxCoords(dst, src, w);
}

// NONMATCHING: register choice
// FUNCTION: 0x000add58
void refresh_wscreen(void)
{
    u8 x;
    u8 y;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;

    find_common_window(window.zone_x[window.f2ea], window.zone_y[window.f2ea], window.zone_w[window.f2ea],
                       window.zone_h[window.f2ea], window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
    if (info.unknown78 == 0) {
        x0 = info.unknown68;
        y0 = info.unknown6c;
        x1 = info.unknown70;
        y1 = info.unknown74;
        for (y = y0 / 32; y <= y1 / 32; y++) {
            for (x = x0 / 32; x <= x1 / 32; x++) {
                if (D_001567c0[x][y] > 0) {
                    find_common_window(x << 5, y << 5, (x << 5) + 32, (y << 5) + 32, window.unknown2c3, window.unknown2c7,
                                       window.unknown2cb, window.unknown2cf);
                    find_common_window(info.unknown68, info.unknown6c, info.unknown70, info.unknown74, window.zone_x[window.f2ea],
                                       window.zone_y[window.f2ea], window.zone_w[window.f2ea], window.zone_h[window.f2ea]);
                    if (info.unknown78 == 0) {
                        copy_bscreen_window_to_wscreen_mk1(info.unknown68, info.unknown6c, info.unknown70, info.unknown74,
                                                           refresh.f0, refresh.f2);
                        D_001567c0[x][y]--;
                    }
                }
            }
        }
    }
}

// FUNCTION: 0x000adf8c
void draw_a_clipped_four_zone_wall_1(s32 x, s32 y, const s32 cell, s32 x0, s32 y0, s32 x1, s32 y1)
{
    s32 cx0;
    s32 cx1;
    s32 cy0;
    s32 cy1;
    Map *mp;

    mp = &map[cell];
    if (window.zone_x[window.f2ea] > x0)
        x0 = window.zone_x[window.f2ea];
    if (window.zone_w[window.f2ea] < x1)
        x1 = window.zone_w[window.f2ea];
    if (window.zone_y[window.f2ea] > y0)
        y0 = window.zone_y[window.f2ea];
    if (window.zone_h[window.f2ea] < y1)
        y1 = window.zone_h[window.f2ea];
    x += refresh.f0;
    y += refresh.f2;
    if (x < lbDisplay.unknown18 && blocks[mp->unknown4].info->height + lbDisplay.unknown1c > y) {
        cx0 = x0 + refresh.f0;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = x1 + refresh.f0;
        if (cx1 > lbDisplay.unknown18)
            cx1 = lbDisplay.unknown18;
        if (cx0 < cx1) {
            cy0 = y0 + refresh.f2;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = y1 + refresh.f2;
            if (cy1 > lbDisplay.unknown1c)
                cy1 = lbDisplay.unknown1c;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - cx0, y - cy0 - blocks[mp->unknown4].info->height, blocks[mp->unknown4].info);
            }
        }
    }
    if (x >= lbDisplay.unknown18 - D_0015f3f8 && blocks[mp->unknown4].info->height + lbDisplay.unknown1c > y) {
        cx0 = refresh.f0 + x0 - lbDisplay.unknown18;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = refresh.f0 + x1 - lbDisplay.unknown18;
        if (cx1 > lbDisplay.unknown18)
            cx1 = lbDisplay.unknown18;
        if (cx0 < cx1) {
            cy0 = y0 + refresh.f2;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = y1 + refresh.f2;
            if (cy1 > lbDisplay.unknown1c)
                cy1 = lbDisplay.unknown1c;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - lbDisplay.unknown18 - cx0, y - cy0 - blocks[mp->unknown4].info->height,
                             blocks[mp->unknown4].info);
            }
        }
    }
    if (x < lbDisplay.unknown18 && y >= lbDisplay.unknown1c) {
        cx0 = x0 + refresh.f0;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = x1 + refresh.f0;
        if (cx1 > lbDisplay.unknown18)
            cx1 = lbDisplay.unknown18;
        if (cx0 < cx1) {
            cy0 = refresh.f2 + y0 - lbDisplay.unknown1c;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = refresh.f2 + y1 - lbDisplay.unknown1c;
            if (cy1 > lbDisplay.unknown1c)
                cy1 = lbDisplay.unknown1c;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - cx0, y - lbDisplay.unknown1c - cy0 - blocks[mp->unknown4].info->height,
                             blocks[mp->unknown4].info);
            }
        }
    }
    if (x >= lbDisplay.unknown18 - D_0015f3f8 && y >= lbDisplay.unknown1c) {
        cx0 = refresh.f0 + x0 - lbDisplay.unknown18;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = refresh.f0 + x1 - lbDisplay.unknown18;
        if (cx1 > lbDisplay.unknown18)
            cx1 = lbDisplay.unknown18;
        if (cx0 < cx1) {
            cy0 = refresh.f2 + y0 - lbDisplay.unknown1c;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = refresh.f2 + y1 - lbDisplay.unknown1c;
            if (cy1 > lbDisplay.unknown1c)
                cy1 = lbDisplay.unknown1c;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - lbDisplay.unknown18 - cx0, y - lbDisplay.unknown1c - cy0 - blocks[mp->unknown4].info->height,
                             blocks[mp->unknown4].info);
            }
        }
    }
}
