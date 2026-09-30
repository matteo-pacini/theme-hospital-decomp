#include "bflib.h"

extern u32 _lbScreenInitialised;
extern u32 screen_initialised;

extern "C" {
extern s32 D_00188c0c;

int LbScreenFindVideoModes(void);

// FUNCTION: 0x000be900
s32 LbScreenIsModeAvailable(TbScreenMode mode)
{
    if (!D_00188c0c) {
        if (LbScreenFindVideoModes() != 1)
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

}
