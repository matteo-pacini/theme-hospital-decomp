#include "bflib.h"

extern "C" {
extern u8 D_00178c0c[];

u8 LbPaletteFindColour(u8 *palette, u8 r, u8 g, u8 b);

// FUNCTION: 0x000be7a0
int LbGhostTableGenerate(u8 *palette, u8 intensity, char *name)
{
    s32 i;
    s32 n;
    u8 *out;
    u8 *blend_rgb;
    u8 *base_rgb;
    s32 base_r;
    s32 base_g;
    s32 base_b;
    s32 r;
    s32 g;
    s32 b;

    if (LbFileLoadAt(name, D_00178c0c) == -1) {
        out = D_00178c0c;
        base_rgb = palette;
        for (i = 0; i < 256; i++) {
            base_r = *base_rgb++;
            base_g = *base_rgb++;
            base_b = *base_rgb++;
            blend_rgb = palette;
            for (n = 256; n != 0; n--) {
                r = base_r + (*blend_rgb++ - base_r) * intensity / 100;
                g = base_g + (*blend_rgb++ - base_g) * intensity / 100;
                b = base_b + (*blend_rgb++ - base_b) * intensity / 100;
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
