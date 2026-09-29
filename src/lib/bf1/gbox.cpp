#include "bflib.h"

extern "C" {
void LbDrawLine(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
void LbDrawBoxClip(s32 x, s32 y, s32 w, s32 h, u8 colour);


// FUNCTION: 0x000c8390
int LbDrawBox(s32 x, s32 y, s32 w, s32 h, u8 colour)
{
    if (lbDisplay.draw_flags & 0x10) {
        if (w < 1 || h < 1)
            return -1;
        LbDrawLine(x, y, x + w - 1, y, colour);
        LbDrawLine(x, y + h - 1, x + w - 1, y + h - 1, colour);
        if ((u32)h > 2) {
            LbDrawLine(x, y + 1, x, y + h - 2, colour);
            LbDrawLine(x + w - 1, y + 1, x + w - 1, y + h - 2, colour);
        }
    } else {
        LbDrawBoxClip(x, y, w, h, colour);
    }
    return 1;
}

// FUNCTION: 0x000c8474
int LbDrawBoxCoords(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour)
{
    if (x1 > x2) {
        x1 ^= x2;
        x2 ^= x1;
        x1 ^= x2;
    }
    if (y1 > y2) {
        y1 ^= y2;
        y2 ^= y1;
        y1 ^= y2;
    }
    LbDrawBox(x1, y1, x2 - x1 + 1, y2 - y1 + 1, colour);
    return 1;
}

}
