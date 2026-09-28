#include "bflib.h"

extern "C" {
extern u8 D_00178c0c[];

u8 LbPaletteFindColour(u8 *palette, u8 r, u8 g, u8 b);
#pragma aux LbPaletteFindColour "W?draw_bdem_unavailable_message$n()v"

// FUNCTION: 0x000be7a0
int LbGhostTableGenerate(u8 *palette, u8 intensity, char *name)
{
    s32 i;
    s32 n;
    u8 *out;
    u8 *p2;
    u8 *p1;
    s32 r1;
    s32 g1;
    s32 b1;
    s32 r;
    s32 g;
    s32 b;

    if (LbFileLoadAt(name, D_00178c0c) == -1) {
        out = D_00178c0c;
        p1 = palette;
        for (i = 0; i < 256; i++) {
            r1 = *p1++;
            g1 = *p1++;
            b1 = *p1++;
            p2 = palette;
            for (n = 256; n != 0; n--) {
                r = r1 + (*p2++ - r1) * intensity / 100;
                g = g1 + (*p2++ - g1) * intensity / 100;
                b = b1 + (*p2++ - b1) * intensity / 100;
                *out++ = LbPaletteFindColour(palette, r, g, b);
            }
        }
        if (LbFileSaveAt(name, D_00178c0c, 0x10000) == -1)
            return -1;
    }
    lbDisplay.ghost = D_00178c0c;
    return 1;
}

}
