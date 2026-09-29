#include "bflib.h"

extern u32 _lbScreenInitialised;
extern u32 screen_initialised;

extern "C" {
extern s32 D_00188c0c;

int fn_000beda8(void);
extern s32 D_001507d4;
s32 fn_000d12e0(void);
/* config/functions.txt has this function under a wrong demo name. */
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
    if (x1 > lbDisplay.graphics_screen_width)
        x1 = lbDisplay.graphics_screen_width;
    if (x2 > lbDisplay.graphics_screen_width)
        x2 = lbDisplay.graphics_screen_width;
    if (y1 > lbDisplay.graphics_screen_height)
        y1 = lbDisplay.graphics_screen_height;
    if (y2 > lbDisplay.graphics_screen_height)
        y2 = lbDisplay.graphics_screen_height;
    lbDisplay.graphics_window_x = x1;
    lbDisplay.graphics_window_y = y1;
    lbDisplay.graphics_window_width = x2 - x1;
    lbDisplay.graphics_window_height = y2 - y1;
    lbDisplay.graphics_window = x + (lbDisplay.unknown4 + lbDisplay.graphics_screen_width * y);
    return 1;
}

// FUNCTION: 0x000bea70
u32 LbScreenIsInitialised(void)
{
    return _lbScreenInitialised;
}

// FUNCTION: 0x000d12e0
s32 fn_000d12e0(void)
{
    return D_001507d4;
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
    if (!lbDisplay.old_video_mode) {
        in.x.eax = 0xf00;
        int386(0x10, &in, &out);
        lbDisplay.old_video_mode = out.h.al;
    }
    if (lbScreenModeInfo[mode].VideoMode >= 256) {
        lbDisplay.vesa_is_set_up = 1;
        LbVesaSetMode(lbScreenModeInfo[mode].VideoMode);
    } else {
        lbDisplay.vesa_is_set_up = 0;
        in.x.eax = lbScreenModeInfo[mode].VideoMode;
        int386(0x10, &in, &out);
    }
    lbDisplay.physical_screen = (u8 *)0xa0000;
    lbDisplay.unknown18 = lbScreenModeInfo[mode].width;
    lbDisplay.unknown1c = lbScreenModeInfo[mode].height;
    lbDisplay.graphics_screen_width = width;
    lbDisplay.graphics_screen_height = height;
    lbDisplay.screen_mode = mode;
    lbDisplay.draw_flags = 0;
    lbDisplay.draw_colour = 0;
    LbScreenSetGraphicsWindow(0, 0, lbScreenModeInfo[mode].width, lbScreenModeInfo[mode].height);
    LbTextSetWindow(0, 0, lbScreenModeInfo[mode].width, lbScreenModeInfo[mode].height);
    if (palette)
        LbPaletteSet(palette);
    screen_initialised = 1;
    _lbScreenInitialised = 1;
    return 1;
}

// FUNCTION: 0x000bec84
int fn_000bec84(u8 colour, ...)
{
    s32 h;
    u8 *p;

    p = lbDisplay.graphics_window;
    h = lbDisplay.graphics_window_height;
    while (--h != -1) {
        LbMemorySet(p, colour, lbDisplay.graphics_window_width);
        p += lbDisplay.graphics_screen_width;
    }
    return 1;
}

}
