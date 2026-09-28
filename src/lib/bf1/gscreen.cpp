#include "bflib.h"

extern u32 _lbScreenInitialised;
extern u32 screen_initialised;

extern "C" {
extern s32 D_00188c34;
#pragma aux D_00188c34 "W?research_names$npnpna"
extern s32 D_00188c38;
extern s32 D_00188c3c;
extern s32 D_00188c40;
extern s32 D_00188c44;
extern s32 D_00188c48;
extern s32 D_00188c0c;

int fn_000beda8(void);
extern u16 D_00188c74;
extern u16 D_00188c76;
extern s16 D_00188c78;
extern u8 D_00188c7a;
extern u8 D_00188c85;
void fn_000d12e0(void);
#pragma aux fn_000d12e0 "__wcpp_2_pure_error__"
int LbMouseSuspend(void);
int LbVesaSetMode(s16 mode);

// FUNCTION: 0x000be900
s32 fn_000be900(TbScreenMode mode)
{
    if (!D_00188c0c) {
        if (fn_000beda8() != 1)
            return 0;
        D_00188c0c = 1;
    }
    return lbScreenModeInfo[mode].Available;
}

// FUNCTION: 0x000be948
int LbScreenSetGraphicsWindow(s32 x, s32 y, s32 w, s32 h)
{
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;

    x1 = x;
    y1 = y;
    x2 = x + w;
    y2 = y + h;
    if (x2 < x1) {
        x2 ^= x1;
        x1 ^= x2;
        x2 ^= x1;
    }
    if (y2 < y1) {
        y2 ^= y1;
        y1 ^= y2;
        y2 ^= y1;
    }
    if (x1 < 0)
        x1 = 0;
    if (x2 < 0)
        x2 = 0;
    if (y1 < 0)
        y1 = 0;
    if (y2 < 0)
        y2 = 0;
    if (x1 > D_00188c34)
        x1 = D_00188c34;
    if (x2 > D_00188c34)
        x2 = D_00188c34;
    if (y1 > D_00188c38)
        y1 = D_00188c38;
    if (y2 > D_00188c38)
        y2 = D_00188c38;
    D_00188c3c = x1;
    D_00188c40 = y1;
    D_00188c44 = x2 - x1;
    D_00188c48 = y2 - y1;
    lbDisplay.graphics_window = x + (lbDisplay.unknown4 + D_00188c34 * y);
    return 1;
}

// FUNCTION: 0x000bea70
u32 LbScreenIsInitialised(void)
{
    return _lbScreenInitialised;
}

// FUNCTION: 0x000bea80
int fn_000bea80(TbScreenMode mode, s32 width, s32 height, u8 *palette, ...)
{
    REGS out;
    REGS in;

    fn_000d12e0();
    if (!fn_000be900(mode))
        return -1;
    LbMouseSuspend();
    if (!D_00188c76) {
        in.x.eax = 0xf00;
        int386(0x10, &in, &out);
        D_00188c76 = out.h.al;
    }
    if (lbScreenModeInfo[mode].VideoMode >= 256) {
        D_00188c7a = 1;
        LbVesaSetMode(lbScreenModeInfo[mode].VideoMode);
    } else {
        D_00188c7a = 0;
        in.x.eax = lbScreenModeInfo[mode].VideoMode;
        int386(0x10, &in, &out);
    }
    lbDisplay.physical_screen = (u8 *)0xa0000;
    lbDisplay.unknown18 = lbScreenModeInfo[mode].width;
    lbDisplay.unknown1c = lbScreenModeInfo[mode].height;
    D_00188c34 = width;
    D_00188c38 = height;
    D_00188c78 = mode;
    D_00188c74 = 0;
    D_00188c85 = 0;
    LbScreenSetGraphicsWindow(0, 0, lbScreenModeInfo[mode].width, lbScreenModeInfo[mode].height);
    LbTextSetWindow(0, 0, lbScreenModeInfo[mode].width, lbScreenModeInfo[mode].height);
    if (palette)
        LbPaletteSet(palette);
    screen_initialised = 1;
    _lbScreenInitialised = 1;
    return 1;
}

int fn_000bec84(u8 colour, ...)
{
    s32 h;
    u8 *p;

    p = lbDisplay.graphics_window;
    h = D_00188c48;
    while (--h != -1) {
        LbMemorySet(p, colour, D_00188c44);
        p += D_00188c34;
    }
    return 1;
}

}
