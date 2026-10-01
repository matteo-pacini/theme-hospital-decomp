#include "bflib.h"

extern "C" {
void LbDrawHVLine(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
void LbDrawBoxClip(s32 x, s32 y, s32 w, s32 h, u8 colour);


// Draws a w x h box at (x, y) in colour: filled and clipped, or only its
// outline with draw_flags 0x10. Returns 1, or -1 for an empty outline.
// FUNCTION: 0x000c8390
int LbDrawBox(s32 x, s32 y, s32 w, s32 h, u8 colour)
{
    if (lbDisplay.draw_flags & 0x10) {
        if (w < 1 || h < 1)
            return -1;
        LbDrawHVLine(x, y, x + w - 1, y, colour);
        LbDrawHVLine(x, y + h - 1, x + w - 1, y + h - 1, colour);
        if ((u32)h > 2) {
            LbDrawHVLine(x, y + 1, x, y + h - 2, colour);
            LbDrawHVLine(x + w - 1, y + 1, x + w - 1, y + h - 2, colour);
        }
    } else {
        LbDrawBoxClip(x, y, w, h, colour);
    }
    return 1;
}

// Draws the box with corners (x1, y1) and (x2, y2), inclusive, in either
// order. Returns 1.
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
