#include "hosp.h"

extern "C" {
extern u8 D_001567c0[][16];
}

void copy_region(const s16 src_x, const s16 src_y, const s16 dst_x, const s16 dst_y, const s16 w, const s16 h);
void copy_bscreen_window_to_wscreen_mk1(const s32 x0, const s32 y0, const s32 x1, const s32 y1, const s16 dx, const s16 dy);

// Copies the screen rectangle x0,y0 to x1,y1 (exclusive) from the background
// buffer (bscreen), which is scrolled by dx,dy and wraps at its width and
// height (lbDisplay.physical_screen_width, unknown1c), to the work screen; a rectangle
// crossing the wrap is copied in up to four parts.
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
    fit_w = lbDisplay.physical_screen_width - (dx + x0);
    fit_h = lbDisplay.physical_screen_height - (dy + y0);
    wrap_w = dx + x1 - lbDisplay.physical_screen_width;
    wrap_h = dy + y1 - lbDisplay.physical_screen_height;
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
    if (fit_h > 0 && dy + y0 < lbDisplay.physical_screen_height) {
        if (fit_w > 0 && dx + x0 < lbDisplay.physical_screen_width)
            copy_region(dx + x0, dy + y0, x0, y0, fit_w, fit_h);
        if (wrap_w > 0)
            copy_region(wrap_x, dy + y0, sx, y0, wrap_w, fit_h);
    }
    if (wrap_h > 0) {
        if (fit_w > 0 && dx + x0 < lbDisplay.physical_screen_width)
            copy_region(dx + x0, wrap_y, x0, sy, fit_w, wrap_h);
        if (wrap_w > 0)
            copy_region(wrap_x, wrap_y, sx, sy, wrap_w, wrap_h);
    }
}

// Copies a w x h block from bscreen at src_x,src_y to the work screen at
// dst_x,dst_y, with no clipping or wrapping.
// FUNCTION: 0x000adcc8
void copy_region(const s16 src_x, const s16 src_y, const s16 dst_x, const s16 dst_y, const s16 w, const s16 h)
{
    u32 i;
    u8 *src;
    u8 *dst;

    i = 0;
    src = bscreen + src_y * lbDisplay.physical_screen_width + src_x;
    dst = lbDisplay.wscreen + dst_y * lbDisplay.graphics_screen_width + dst_x;
    for (; i < h; i++, src += lbDisplay.physical_screen_width, dst += lbDisplay.graphics_screen_width)
        fn_000c21e6(dst, src, w);
}

// Copies to the work screen each 32x32 cell of the current view zone whose
// dirty count (D_001567c0) is positive, decrementing the count.
// FUNCTION: 0x000add58
void refresh_wscreen(void)
{
    u8 x;
    u8 y;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;

    find_common_window(window.zone_x[window.bscreen_window], window.zone_y[window.bscreen_window], window.zone_x1[window.bscreen_window],
                       window.zone_y1[window.bscreen_window], window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
    if (info.clip_empty == 0) {
        x0 = info.clip_x0;
        y0 = info.clip_y0;
        x1 = info.clip_x1;
        y1 = info.clip_y1;
        for (y = y0 / 32; y <= y1 / 32; y++) {
            for (x = x0 / 32; x <= x1 / 32; x++) {
                if (D_001567c0[x][y] > 0) {
                    find_common_window(x << 5, y << 5, (x << 5) + 32, (y << 5) + 32, window.clip_x0, window.clip_y0,
                                       window.clip_x1, window.clip_y1);
                    find_common_window(info.clip_x0, info.clip_y0, info.clip_x1, info.clip_y1, window.zone_x[window.bscreen_window],
                                       window.zone_y[window.bscreen_window], window.zone_x1[window.bscreen_window], window.zone_y1[window.bscreen_window]);
                    if (info.clip_empty == 0) {
                        copy_bscreen_window_to_wscreen_mk1(info.clip_x0, info.clip_y0, info.clip_x1, info.clip_y1,
                                                           refresh.scroll_x, refresh.scroll_y);
                        D_001567c0[x][y]--;
                    }
                }
            }
        }
    }
}

// Draws the wall block of map cell cell (map unknown4) at view position x,y
// into the wrapping background buffer, clipped to x0,y0 to x1,y1 and the
// current view zone, drawing up to four copies where it crosses the wrap.
// FUNCTION: 0x000adf8c
void draw_a_clipped_four_zone_wall_1(s32 x, s32 y, const s32 cell, s32 x0, s32 y0, s32 x1, s32 y1)
{
    s32 cx0;
    s32 cx1;
    s32 cy0;
    s32 cy1;
    Map *mp;

    mp = &map[cell];
    if (window.zone_x[window.bscreen_window] > x0)
        x0 = window.zone_x[window.bscreen_window];
    if (window.zone_x1[window.bscreen_window] < x1)
        x1 = window.zone_x1[window.bscreen_window];
    if (window.zone_y[window.bscreen_window] > y0)
        y0 = window.zone_y[window.bscreen_window];
    if (window.zone_y1[window.bscreen_window] < y1)
        y1 = window.zone_y1[window.bscreen_window];
    x += refresh.scroll_x;
    y += refresh.scroll_y;
    if (x < lbDisplay.physical_screen_width && blocks[mp->wall_w].info->height + lbDisplay.physical_screen_height > y) {
        cx0 = x0 + refresh.scroll_x;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = x1 + refresh.scroll_x;
        if (cx1 > lbDisplay.physical_screen_width)
            cx1 = lbDisplay.physical_screen_width;
        if (cx0 < cx1) {
            cy0 = y0 + refresh.scroll_y;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = y1 + refresh.scroll_y;
            if (cy1 > lbDisplay.physical_screen_height)
                cy1 = lbDisplay.physical_screen_height;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - cx0, y - cy0 - blocks[mp->wall_w].info->height, blocks[mp->wall_w].info);
            }
        }
    }
    if (x >= lbDisplay.physical_screen_width - resvars.tile_width && blocks[mp->wall_w].info->height + lbDisplay.physical_screen_height > y) {
        cx0 = refresh.scroll_x + x0 - lbDisplay.physical_screen_width;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = refresh.scroll_x + x1 - lbDisplay.physical_screen_width;
        if (cx1 > lbDisplay.physical_screen_width)
            cx1 = lbDisplay.physical_screen_width;
        if (cx0 < cx1) {
            cy0 = y0 + refresh.scroll_y;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = y1 + refresh.scroll_y;
            if (cy1 > lbDisplay.physical_screen_height)
                cy1 = lbDisplay.physical_screen_height;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - lbDisplay.physical_screen_width - cx0, y - cy0 - blocks[mp->wall_w].info->height,
                             blocks[mp->wall_w].info);
            }
        }
    }
    if (x < lbDisplay.physical_screen_width && y >= lbDisplay.physical_screen_height) {
        cx0 = x0 + refresh.scroll_x;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = x1 + refresh.scroll_x;
        if (cx1 > lbDisplay.physical_screen_width)
            cx1 = lbDisplay.physical_screen_width;
        if (cx0 < cx1) {
            cy0 = refresh.scroll_y + y0 - lbDisplay.physical_screen_height;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = refresh.scroll_y + y1 - lbDisplay.physical_screen_height;
            if (cy1 > lbDisplay.physical_screen_height)
                cy1 = lbDisplay.physical_screen_height;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - cx0, y - lbDisplay.physical_screen_height - cy0 - blocks[mp->wall_w].info->height,
                             blocks[mp->wall_w].info);
            }
        }
    }
    if (x >= lbDisplay.physical_screen_width - resvars.tile_width && y >= lbDisplay.physical_screen_height) {
        cx0 = refresh.scroll_x + x0 - lbDisplay.physical_screen_width;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = refresh.scroll_x + x1 - lbDisplay.physical_screen_width;
        if (cx1 > lbDisplay.physical_screen_width)
            cx1 = lbDisplay.physical_screen_width;
        if (cx0 < cx1) {
            cy0 = refresh.scroll_y + y0 - lbDisplay.physical_screen_height;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = refresh.scroll_y + y1 - lbDisplay.physical_screen_height;
            if (cy1 > lbDisplay.physical_screen_height)
                cy1 = lbDisplay.physical_screen_height;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - lbDisplay.physical_screen_width - cx0, y - lbDisplay.physical_screen_height - cy0 - blocks[mp->wall_w].info->height,
                             blocks[mp->wall_w].info);
            }
        }
    }
}

// As draw_a_clipped_four_zone_wall_1, for the sprite spr.
// FUNCTION: 0x000ae518
void draw_a_clipped_four_zone_wall_0(s32 x, s32 y, const TbSprite *const spr, s32 x0, s32 y0, s32 x1, s32 y1)
{
    s32 cx0;
    s32 cx1;
    s32 cy0;
    s32 cy1;

    if (window.zone_x[window.bscreen_window] > x0)
        x0 = window.zone_x[window.bscreen_window];
    if (window.zone_x1[window.bscreen_window] < x1)
        x1 = window.zone_x1[window.bscreen_window];
    if (window.zone_y[window.bscreen_window] > y0)
        y0 = window.zone_y[window.bscreen_window];
    if (window.zone_y1[window.bscreen_window] < y1)
        y1 = window.zone_y1[window.bscreen_window];
    x += refresh.scroll_x;
    y += refresh.scroll_y;
    if (x < lbDisplay.physical_screen_width && spr->height + lbDisplay.physical_screen_height > y) {
        cx0 = x0 + refresh.scroll_x;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = x1 + refresh.scroll_x;
        if (cx1 > lbDisplay.physical_screen_width)
            cx1 = lbDisplay.physical_screen_width;
        if (cx0 < cx1) {
            cy0 = y0 + refresh.scroll_y;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = y1 + refresh.scroll_y;
            if (cy1 > lbDisplay.physical_screen_height)
                cy1 = lbDisplay.physical_screen_height;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - cx0, y - cy0 - spr->height, (TbSprite *)spr);
            }
        }
    }
    if (x >= lbDisplay.physical_screen_width - resvars.tile_width && spr->height + lbDisplay.physical_screen_height > y) {
        cx0 = refresh.scroll_x + x0 - lbDisplay.physical_screen_width;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = refresh.scroll_x + x1 - lbDisplay.physical_screen_width;
        if (cx1 > lbDisplay.physical_screen_width)
            cx1 = lbDisplay.physical_screen_width;
        if (cx0 < cx1) {
            cy0 = y0 + refresh.scroll_y;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = y1 + refresh.scroll_y;
            if (cy1 > lbDisplay.physical_screen_height)
                cy1 = lbDisplay.physical_screen_height;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - lbDisplay.physical_screen_width - cx0, y - cy0 - spr->height,
                             (TbSprite *)spr);
            }
        }
    }
    if (x < lbDisplay.physical_screen_width && y >= lbDisplay.physical_screen_height) {
        cx0 = x0 + refresh.scroll_x;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = x1 + refresh.scroll_x;
        if (cx1 > lbDisplay.physical_screen_width)
            cx1 = lbDisplay.physical_screen_width;
        if (cx0 < cx1) {
            cy0 = refresh.scroll_y + y0 - lbDisplay.physical_screen_height;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = refresh.scroll_y + y1 - lbDisplay.physical_screen_height;
            if (cy1 > lbDisplay.physical_screen_height)
                cy1 = lbDisplay.physical_screen_height;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - cx0, y - lbDisplay.physical_screen_height - cy0 - spr->height,
                             (TbSprite *)spr);
            }
        }
    }
    if (x >= lbDisplay.physical_screen_width - resvars.tile_width && y >= lbDisplay.physical_screen_height) {
        cx0 = refresh.scroll_x + x0 - lbDisplay.physical_screen_width;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = refresh.scroll_x + x1 - lbDisplay.physical_screen_width;
        if (cx1 > lbDisplay.physical_screen_width)
            cx1 = lbDisplay.physical_screen_width;
        if (cx0 < cx1) {
            cy0 = refresh.scroll_y + y0 - lbDisplay.physical_screen_height;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = refresh.scroll_y + y1 - lbDisplay.physical_screen_height;
            if (cy1 > lbDisplay.physical_screen_height)
                cy1 = lbDisplay.physical_screen_height;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - lbDisplay.physical_screen_width - cx0, y - lbDisplay.physical_screen_height - cy0 - spr->height,
                             (TbSprite *)spr);
            }
        }
    }
}

// As draw_a_clipped_four_zone_wall_0, for a floor tile sprite spr (its
// vertical reach taken as resvars.tile_height).
// FUNCTION: 0x000ae9ec
void draw_a_clipped_four_zone_floor_tile(s32 x, s32 y, const TbSprite *const spr, s32 x0, s32 y0, s32 x1, s32 y1)
{
    s32 cx0;
    s32 cx1;
    s32 cy0;
    s32 cy1;

    if (window.zone_x[window.bscreen_window] > x0)
        x0 = window.zone_x[window.bscreen_window];
    if (window.zone_x1[window.bscreen_window] < x1)
        x1 = window.zone_x1[window.bscreen_window];
    if (window.zone_y[window.bscreen_window] > y0)
        y0 = window.zone_y[window.bscreen_window];
    if (window.zone_y1[window.bscreen_window] < y1)
        y1 = window.zone_y1[window.bscreen_window];
    x += refresh.scroll_x;
    y += refresh.scroll_y;
    if (x < lbDisplay.physical_screen_width && resvars.tile_height + lbDisplay.physical_screen_height > y) {
        cx0 = x0 + refresh.scroll_x;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = x1 + refresh.scroll_x;
        if (cx1 > lbDisplay.physical_screen_width)
            cx1 = lbDisplay.physical_screen_width;
        if (cx0 < cx1) {
            cy0 = y0 + refresh.scroll_y;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = y1 + refresh.scroll_y;
            if (cy1 > lbDisplay.physical_screen_height)
                cy1 = lbDisplay.physical_screen_height;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - cx0, y - cy0 - spr->height, (TbSprite *)spr);
            }
        }
    }
    if (x >= lbDisplay.physical_screen_width - resvars.tile_width && resvars.tile_height + lbDisplay.physical_screen_height > y) {
        cx0 = refresh.scroll_x + x0 - lbDisplay.physical_screen_width;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = refresh.scroll_x + x1 - lbDisplay.physical_screen_width;
        if (cx1 > lbDisplay.physical_screen_width)
            cx1 = lbDisplay.physical_screen_width;
        if (cx0 < cx1) {
            cy0 = y0 + refresh.scroll_y;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = y1 + refresh.scroll_y;
            if (cy1 > lbDisplay.physical_screen_height)
                cy1 = lbDisplay.physical_screen_height;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - lbDisplay.physical_screen_width - cx0, y - cy0 - spr->height,
                             (TbSprite *)spr);
            }
        }
    }
    if (x < lbDisplay.physical_screen_width && y >= lbDisplay.physical_screen_height) {
        cx0 = x0 + refresh.scroll_x;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = x1 + refresh.scroll_x;
        if (cx1 > lbDisplay.physical_screen_width)
            cx1 = lbDisplay.physical_screen_width;
        if (cx0 < cx1) {
            cy0 = refresh.scroll_y + y0 - lbDisplay.physical_screen_height;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = refresh.scroll_y + y1 - lbDisplay.physical_screen_height;
            if (cy1 > lbDisplay.physical_screen_height)
                cy1 = lbDisplay.physical_screen_height;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - cx0, y - lbDisplay.physical_screen_height - cy0 - spr->height,
                             (TbSprite *)spr);
            }
        }
    }
    if (x >= lbDisplay.physical_screen_width - resvars.tile_width && y >= lbDisplay.physical_screen_height) {
        cx0 = refresh.scroll_x + x0 - lbDisplay.physical_screen_width;
        if (cx0 < 0)
            cx0 = 0;
        cx1 = refresh.scroll_x + x1 - lbDisplay.physical_screen_width;
        if (cx1 > lbDisplay.physical_screen_width)
            cx1 = lbDisplay.physical_screen_width;
        if (cx0 < cx1) {
            cy0 = refresh.scroll_y + y0 - lbDisplay.physical_screen_height;
            if (cy0 < 0)
                cy0 = 0;
            cy1 = refresh.scroll_y + y1 - lbDisplay.physical_screen_height;
            if (cy1 > lbDisplay.physical_screen_height)
                cy1 = lbDisplay.physical_screen_height;
            if (cy0 < cy1) {
                LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
                LbSpriteDraw(x - lbDisplay.physical_screen_width - cx0, y - lbDisplay.physical_screen_height - cy0 - spr->height,
                             (TbSprite *)spr);
            }
        }
    }
}
