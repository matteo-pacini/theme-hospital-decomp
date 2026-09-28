#include "hosp.h"

extern "C" {
extern s32 D_00188c38;
extern s32 D_00188c3c;
extern s32 D_00188c40;
extern s32 D_00188c44;
extern s32 D_00188c48;
extern u8 D_001553d4;
}

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

// FUNCTION: 0x00056560
void FillPixBuffer(u8 colour)
{
    memset(pixBuf.data, colour, pixBuf.size);
}

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

// FUNCTION: 0x000566ec
void DrawIntoPixBuffer(void)
{
    pixBuf.saved_width = D_00188c34;
    pixBuf.saved_height = D_00188c38;
    pixBuf.saved_ptr = lbDisplay.unknown4;
    pixBuf.saved_x = D_00188c3c;
    pixBuf.saved_y = D_00188c40;
    pixBuf.saved_w = D_00188c44;
    pixBuf.saved_h = D_00188c48;
    D_00188c34 = pixBuf.width;
    D_00188c38 = pixBuf.height;
    lbDisplay.unknown4 = pixBuf.data;
    LbScreenSetGraphicsWindow(0, 0, pixBuf.width, pixBuf.height);
    D_001553d4 = 1;
}

// FUNCTION: 0x00056790
void RestoreFromPixBuffer(void)
{
    D_00188c34 = pixBuf.saved_width;
    D_00188c38 = pixBuf.saved_height;
    lbDisplay.unknown4 = pixBuf.saved_ptr;
    LbScreenSetGraphicsWindow(pixBuf.saved_x, pixBuf.saved_y, pixBuf.saved_w, pixBuf.saved_h);
    D_001553d4 = 0;
}

// NONMATCHING: register choice in the ghost lookup; pixBuf.x is reached through draw_cells+0x63e
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
        dst = sx + ((D_00188c40 + sy) * D_00188c34 + lbDisplay.unknown4 + D_00188c3c);
        if (sx < 0) {
            cols = sx + w;
            if (cols < 0)
                return;
            src += -sx;
            dst += -sx;
        }
        if (sx + w > D_00188c44) {
            if (sx >= D_00188c44)
                return;
            cols -= sx + w - D_00188c44;
        }
        width = cols;
        if (sy < 0) {
            rows = sy + h;
            if (rows < 0)
                return;
            src += -(pixBuf.width * sy);
            dst += -(sy * D_00188c34);
        }
        if (sy + h > D_00188c48) {
            if (sy >= D_00188c48)
                return;
            rows -= sy + h - D_00188c48;
        }
        next_src = src + pixBuf.width;
        next_dst = dst + D_00188c34;
        if (flags & 1) {
            if (flags & 4) {
                if (flags & 2) {
                    for (; rows > 0; rows--) {
                        for (cols = width; cols > 0; cols--, src++, dst++)
                            *dst = *(lbDisplay.ghost + (*src << 8) + *dst);
                        dst = next_dst;
                        next_dst += D_00188c34;
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
                        next_dst += D_00188c34;
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
                        next_dst += D_00188c34;
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
                        next_dst += D_00188c34;
                        src = next_src;
                        next_src += pixBuf.width;
                    }
                }
            }
        } else {
            if (flags & 2) {
                for (; rows > 0; rows--) {
                    LbDrawBoxCoords(dst, src, cols);
                    dst = next_dst;
                    next_dst += D_00188c34;
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
                    next_dst += D_00188c34;
                    src = next_src;
                    next_src += pixBuf.width;
                }
            }
        }
    }
}
