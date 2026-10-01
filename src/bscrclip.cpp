#include "hosp.h"

extern "C" {
extern u8 D_001567c0[][16];
}

void copy_region(const s16 src_x, const s16 src_y, const s16 dst_x, const s16 dst_y, const s16 w, const s16 h);
void copy_bscreen_window_to_wscreen_mk1(const s32 x0, const s32 y0, const s32 x1, const s32 y1, const s16 dx, const s16 dy);

// FUNCTION: 0x000adad0
void copy_bscreen_window_to_wscreen_mk1(const s32 x0, const s32 y0, const s32 x1, const s32 y1, const s16 dx, const s16 dy)
{
    s16 wrap_y;
    s16 wrap_x;
    s16 sx;
    s16 sy;
    s16 w;
    s16 h;
    s16 fit_w;
    s16 fit_h;
    s16 wrap_w;
    s16 wrap_h;

    wrap_x = 0;
    wrap_y = 0;
    sx = x0;
    sy = y0;
    w = x1 - x0;
    h = y1 - y0;
    fit_w = lbDisplay.unknown18 - (dx + x0);
    fit_h = lbDisplay.unknown1c - (dy + y0);
    wrap_w = dx + x1 - lbDisplay.unknown18;
    wrap_h = dy + y1 - lbDisplay.unknown1c;
    if (fit_w < 0)
        wrap_x = -fit_w;
    else
        sx += fit_w;
    if (fit_h < 0)
        wrap_y = -fit_h;
    else
        sy += fit_h;
    if ((s32)fit_w > (s32)w)
        fit_w = w;
    if ((s32)fit_h > (s32)h)
        fit_h = h;
    if ((s32)wrap_w > (s32)w)
        wrap_w = w;
    if ((s32)wrap_h > (s32)h)
        wrap_h = h;
    if (fit_h > 0 && dy + y0 < lbDisplay.unknown1c) {
        if (fit_w > 0 && dx + x0 < lbDisplay.unknown18)
            copy_region(dx + x0, dy + y0, x0, y0, fit_w, fit_h);
        if (wrap_w > 0)
            copy_region(wrap_x, dy + y0, sx, y0, wrap_w, fit_h);
    }
    if (wrap_h > 0) {
        if (fit_w > 0 && dx + x0 < lbDisplay.unknown18)
            copy_region(dx + x0, wrap_y, x0, sy, fit_w, wrap_h);
        if (wrap_w > 0)
            copy_region(wrap_x, wrap_y, sx, sy, wrap_w, wrap_h);
    }
}

// FUNCTION: 0x000adcc8
void copy_region(const s16 src_x, const s16 src_y, const s16 dst_x, const s16 dst_y, const s16 w, const s16 h)
{
    u32 i;
    u8 *src;
    u8 *dst;

    i = 0;
    src = bscreen + src_y * lbDisplay.unknown18 + src_x;
    dst = lbDisplay.unknown4 + dst_y * lbDisplay.graphics_screen_width + dst_x;
    for (; i < h; i++, src += lbDisplay.unknown18, dst += lbDisplay.graphics_screen_width)
        fn_000c21e6(dst, src, w);
}

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
    if (x >= lbDisplay.unknown18 - resvars.f4 && blocks[mp->unknown4].info->height + lbDisplay.unknown1c > y) {
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
    if (x >= lbDisplay.unknown18 - resvars.f4 && y >= lbDisplay.unknown1c) {
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

// FUNCTION: 0x000ae518
void draw_a_clipped_four_zone_wall_0(s32 x, s32 y, const TbSprite *const spr, s32 x0, s32 y0, s32 x1, s32 y1)
{
    s32 cx0;
    s32 cx1;
    s32 cy0;
    s32 cy1;

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
    if (x < lbDisplay.unknown18 && spr->height + lbDisplay.unknown1c > y) {
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
                LbSpriteDraw(x - cx0, y - cy0 - spr->height, (TbSprite *)spr);
            }
        }
    }
    if (x >= lbDisplay.unknown18 - resvars.f4 && spr->height + lbDisplay.unknown1c > y) {
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
                LbSpriteDraw(x - lbDisplay.unknown18 - cx0, y - cy0 - spr->height,
                             (TbSprite *)spr);
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
                LbSpriteDraw(x - cx0, y - lbDisplay.unknown1c - cy0 - spr->height,
                             (TbSprite *)spr);
            }
        }
    }
    if (x >= lbDisplay.unknown18 - resvars.f4 && y >= lbDisplay.unknown1c) {
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
                LbSpriteDraw(x - lbDisplay.unknown18 - cx0, y - lbDisplay.unknown1c - cy0 - spr->height,
                             (TbSprite *)spr);
            }
        }
    }
}

// FUNCTION: 0x000ae9ec
void draw_a_clipped_four_zone_floor_tile(s32 x, s32 y, const TbSprite *const spr, s32 x0, s32 y0, s32 x1, s32 y1)
{
    s32 cx0;
    s32 cx1;
    s32 cy0;
    s32 cy1;

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
    if (x < lbDisplay.unknown18 && resvars.f3 + lbDisplay.unknown1c > y) {
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
                LbSpriteDraw(x - cx0, y - cy0 - spr->height, (TbSprite *)spr);
            }
        }
    }
    if (x >= lbDisplay.unknown18 - resvars.f4 && resvars.f3 + lbDisplay.unknown1c > y) {
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
                LbSpriteDraw(x - lbDisplay.unknown18 - cx0, y - cy0 - spr->height,
                             (TbSprite *)spr);
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
                LbSpriteDraw(x - cx0, y - lbDisplay.unknown1c - cy0 - spr->height,
                             (TbSprite *)spr);
            }
        }
    }
    if (x >= lbDisplay.unknown18 - resvars.f4 && y >= lbDisplay.unknown1c) {
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
                LbSpriteDraw(x - lbDisplay.unknown18 - cx0, y - lbDisplay.unknown1c - cy0 - spr->height,
                             (TbSprite *)spr);
            }
        }
    }
}
