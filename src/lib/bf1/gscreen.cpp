#include "bflib.h"

extern u32 _lbScreenInitialised;
extern u32 screen_initialised;

extern "C" {
extern s32 D_00188c0c;

TbScreenModeInfo lbScreenModeInfo[] = {
    { 0, 0, 0, 0, 0x0, "MODE_INVALID     " },
    { 320, 200, 8, 0, 0x13, "MODE_320_200_8\t  " },
    { 320, 200, 16, 0, 0x0, "MODE_320_200_16  " },
    { 320, 200, 24, 0, 0x0, "MODE_320_200_24  " },
    { 320, 240, 8, 0, 0x0, "MODE_320_240_8\t  " },
    { 320, 240, 16, 0, 0x0, "MODE_320_240_16  " },
    { 320, 240, 24, 0, 0x0, "MODE_320_240_24  " },
    { 512, 384, 8, 0, 0x0, "MODE_512_384_8\t  " },
    { 512, 384, 16, 0, 0x0, "MODE_512_384_16  " },
    { 512, 384, 24, 0, 0x0, "MODE_512_384_24  " },
    { 640, 400, 8, 0, 0x100, "MODE_640_400_8\t  " },
    { 640, 400, 16, 0, 0x0, "MODE_640_400_16  " },
    { 640, 400, 24, 0, 0x0, "MODE_640_400_24  " },
    { 640, 480, 8, 0, 0x101, "MODE_640_480_8\t  " },
    { 640, 480, 16, 0, 0x0, "MODE_640_480_16  " },
    { 640, 480, 24, 0, 0x0, "MODE_640_480_24  " },
    { 800, 600, 8, 0, 0x103, "MODE_800_600_8\t  " },
    { 800, 600, 16, 0, 0x0, "MODE_800_600_16  " },
    { 800, 600, 24, 0, 0x0, "MODE_800_600_24  " },
    { 1024, 768, 8, 0, 0x105, "MODE_1024_768_8  " },
    { 1024, 768, 16, 0, 0x0, "MODE_1024_768_16 " },
    { 1024, 768, 24, 0, 0x0, "MODE_1024_768_24 " },
    { 1200, 1024, 8, 0, 0x107, "MODE_1280_1024_8 " },
    { 1200, 1024, 16, 0, 0x0, "MODE_1280_1024_16" },
    { 1200, 1024, 24, 0, 0x0, "MODE_1280_1024_24" },
    { 1600, 1200, 8, 0, 0x0, "MODE_1600_1200_8 " },
    { 1600, 1200, 16, 0, 0x0, "MODE_1600_1200_16" },
    { 1600, 1200, 24, 0, 0x0, "MODE_1600_1200_24" },
    { 0, 0, 0, 0, 0x0, "MODE_INVALID     " },
};

int LbScreenFindVideoModes(void);

// Returns whether screen mode `mode` is available, probing the video modes
// the first time.
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

// Sets the graphics window to the box at (x, y), w x h (either corner
// order), clipped to the screen; graphics_window points at its unclipped
// corner. Returns 1.
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
    lbDisplay.graphics_window = x + (lbDisplay.wscreen + lbDisplay.graphics_screen_width * y);
    return 1;
}

// Returns nonzero if a screen mode has been set up.
// FUNCTION: 0x000bea70
u32 LbScreenIsInitialised(void)
{
    return _lbScreenInitialised;
}

}
