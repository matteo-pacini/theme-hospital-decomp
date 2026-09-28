#include "bflib.h"

extern u32 _lbScreenInitialised;

extern "C" {
extern s32 D_00188c34;
#pragma aux D_00188c34 "W?research_names$npnpna"
extern s32 D_00188c38;
extern s32 D_00188c3c;
extern s32 D_00188c40;
extern s32 D_00188c44;
extern s32 D_00188c48;

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

}
