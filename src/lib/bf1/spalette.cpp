#include "bflib.h"

extern "C" {
u8 inp_(u16 port);
#pragma aux inp_ = 0x29 0xc0 0xec parm [dx] value [al] modify [eax];
void outp_(u16 port, u32 value);
#pragma aux outp_ = 0xee parm [dx] [al] modify exact [];

// FUNCTION: 0x000c2210
int LbPaletteGet(u8 *palette, ...)
{
    s32 i;

    outp_(0x3c7, 0);
    for (i = 0; i < 0x300;) {
        palette[i++] = inp_(0x3c9);
        palette[i++] = inp_(0x3c9);
        palette[i++] = inp_(0x3c9);
    }
    return 1;
}

// NONMATCHING: target range also covers the next two functions (0xc22d8, 0xc2350)
// FUNCTION: 0x000c2274
int LbPaletteSet(u8 *palette, ...)
{
    u8 *p;
    u32 i;

    p = palette;
    outp_(0x3c6, 0xff);
    for (i = 0; (u16)i < 256; ++i) {
        outp_(0x3c8, i);
        outp_(0x3c9, *p++);
        outp_(0x3c9, *p++);
        outp_(0x3c9, *p++);
    }
    return 1;
}

}
