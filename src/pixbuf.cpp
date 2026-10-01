#include "hosp.h"

extern "C" {
}

// Sets up the off-screen pixel buffer as w x h pixels, to be drawn at screen
// position (x, y), filling it with colour if clear is set.
// FUNCTION: 0x000564f0
void InitPixBuffer(s16 x, s16 y, s16 w, s16 h, u8 colour, u8 clear)
{
    pixBuf.size = w * h;
    if (clear)
        memset(pixBuf.data, colour, pixBuf.size);
    pixBuf.x = x;
    pixBuf.y = y;
    pixBuf.width = w;
    pixBuf.height = h;
}

// Fills the whole pixel buffer with colour.
// FUNCTION: 0x00056560
void FillPixBuffer(u8 colour)
{
    memset(pixBuf.data, colour, pixBuf.size);
}

// Fills the box (x, y, w, h) of the pixel buffer with colour, clipped to
// 180 x 180.
// FUNCTION: 0x00056590
void FillBoxInPixBuffer(u8 colour, s32 x, s32 y, s32 w, s32 h)
{
    x < 0 ? x = 0 : 0;
    x > 0xb4 ? x = 0xb4 : 0;
    y < 0 ? y = 0 : 0;
    y > 0xb4 ? y = 0xb4 : 0;
    w < 0 ? w = 0 : 0;
    0xb4 - x < w ? w = 0xb4 - x : 0;
    h < 0 ? h = 0 : 0;
    0xb4 - y < h ? h = 0xb4 - y : 0;
    {
        s32 end;

        for (end = y + h; y < end; y++)
            memset(pixBuf.data + pixBuf.width * y + x, colour, w);
    }
}

// Redirects drawing into the pixel buffer, saving the screen and graphics
// window to restore. Sets othervar.f1c while active.
// FUNCTION: 0x000566ec
void DrawIntoPixBuffer(void)
{
    pixBuf.saved_width = lbDisplay.graphics_screen_width;
    pixBuf.saved_height = lbDisplay.graphics_screen_height;
    pixBuf.saved_ptr = lbDisplay.unknown4;
    pixBuf.saved_x = lbDisplay.graphics_window_x;
    pixBuf.saved_y = lbDisplay.graphics_window_y;
    pixBuf.saved_w = lbDisplay.graphics_window_width;
    pixBuf.saved_h = lbDisplay.graphics_window_height;
    lbDisplay.graphics_screen_width = pixBuf.width;
    lbDisplay.graphics_screen_height = pixBuf.height;
    lbDisplay.unknown4 = pixBuf.data;
    LbScreenSetGraphicsWindow(0, 0, pixBuf.width, pixBuf.height);
    othervar.f1c = 1;
}

// Restores drawing to the screen saved by DrawIntoPixBuffer and clears
// othervar.f1c.
// FUNCTION: 0x00056790
void RestoreFromPixBuffer(void)
{
    lbDisplay.graphics_screen_width = pixBuf.saved_width;
    lbDisplay.graphics_screen_height = pixBuf.saved_height;
    lbDisplay.unknown4 = pixBuf.saved_ptr;
    LbScreenSetGraphicsWindow(pixBuf.saved_x, pixBuf.saved_y, pixBuf.saved_w, pixBuf.saved_h);
    othervar.f1c = 0;
}

// Copies the box (x, y, w, h) of the pixel buffer (clipped to 180 x 180) to
// the screen at pixBuf's position plus (x, y), clipped to the graphics
// window. flags: 2 opaque (otherwise colour 0xfe is transparent), 1 blended
// through the ghost table, 4 with the buffer and screen swapped in the
// blend.
// FUNCTION: 0x000567e8
void DrawPixBuffer(u8 flags, s32 x, s32 y, s32 w, s32 h)
{
    x < 0 ? x = 0 : 0;
    x > 0xb4 ? x = 0xb4 : 0;
    y < 0 ? y = 0 : 0;
    y > 0xb4 ? y = 0xb4 : 0;
    w < 0 ? w = 0 : 0;
    0xb4 - x < w ? w = 0xb4 - x : 0;
    h < 0 ? h = 0 : 0;
    0xb4 - y < h ? h = 0xb4 - y : 0;
    {
        u8 *next_dst;
        u8 *next_src;
        s32 width;
        u8 *dst;
        u8 *src;
        s32 sy;
        s32 sx;
        s32 rows;
        s32 cols;

        cols = w;
        rows = h;
        sx = x + pixBuf.x;
        sy = y + pixBuf.y;
        src = x + (pixBuf.width * y + pixBuf.data);
        dst = sx + ((lbDisplay.graphics_window_y + sy) * lbDisplay.graphics_screen_width + lbDisplay.unknown4 + lbDisplay.graphics_window_x);
        if (sx < 0) {
            cols = sx + w;
            if (cols < 0)
                return;
            src += -sx;
            dst += -sx;
        }
        if (sx + w > lbDisplay.graphics_window_width) {
            if (sx >= lbDisplay.graphics_window_width)
                return;
            cols -= sx + w - lbDisplay.graphics_window_width;
        }
        width = cols;
        if (sy < 0) {
            rows = sy + h;
            if (rows < 0)
                return;
            src += -(pixBuf.width * sy);
            dst += -(sy * lbDisplay.graphics_screen_width);
        }
        if (sy + h > lbDisplay.graphics_window_height) {
            if (sy >= lbDisplay.graphics_window_height)
                return;
            rows -= sy + h - lbDisplay.graphics_window_height;
        }
        next_src = src + pixBuf.width;
        next_dst = dst + lbDisplay.graphics_screen_width;
        if (flags & 1) {
            if (flags & 4) {
                if (flags & 2) {
                    for (; rows > 0; rows--) {
                        for (cols = width; cols > 0; cols--, src++, dst++)
                            *dst = *(lbDisplay.ghost + (*src << 8) + *dst);
                        dst = next_dst;
                        next_dst += lbDisplay.graphics_screen_width;
                        src = next_src;
                        next_src += pixBuf.width;
                    }
                } else {
                    for (; rows > 0; rows--) {
                        for (cols = width; cols > 0; cols--, src++, dst++) {
                            if (*src != 0xfe)
                                *dst = *(lbDisplay.ghost + (*src << 8) + *dst);
                        }
                        dst = next_dst;
                        next_dst += lbDisplay.graphics_screen_width;
                        src = next_src;
                        next_src += pixBuf.width;
                    }
                }
            } else {
                if (flags & 2) {
                    for (; rows > 0; rows--) {
                        for (cols = width; cols > 0; cols--, src++, dst++)
                            *dst = *(lbDisplay.ghost + (*dst << 8) + *src);
                        dst = next_dst;
                        next_dst += lbDisplay.graphics_screen_width;
                        src = next_src;
                        next_src += pixBuf.width;
                    }
                } else {
                    for (; rows > 0; rows--) {
                        for (cols = width; cols > 0; cols--, src++, dst++) {
                            if (*src != 0xfe)
                                *dst = *(lbDisplay.ghost + (*dst << 8) + *src);
                        }
                        dst = next_dst;
                        next_dst += lbDisplay.graphics_screen_width;
                        src = next_src;
                        next_src += pixBuf.width;
                    }
                }
            }
        } else {
            if (flags & 2) {
                for (; rows > 0; rows--) {
                    fn_000c21e6(dst, src, cols);
                    dst = next_dst;
                    next_dst += lbDisplay.graphics_screen_width;
                    src = next_src;
                    next_src += pixBuf.width;
                }
            } else {
                for (; rows > 0; rows--) {
                    for (cols = width; cols > 0; cols--, src++, dst++) {
                        if (*src != 0xfe)
                            *dst = *src;
                    }
                    dst = next_dst;
                    next_dst += lbDisplay.graphics_screen_width;
                    src = next_src;
                    next_src += pixBuf.width;
                }
            }
        }
    }
}
