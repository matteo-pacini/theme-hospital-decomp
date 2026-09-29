#include "bflib.h"

extern "C" {
extern u8 *lbSpriteReMapPtr;

u32 LbSpriteSetScalingData(int x, int y, int width, int height, int dest_width, int dest_height);
int LbSpriteDrawUsingScalingData(int x, int y, TbSprite *sprite);

// NONMATCHING: target range also covers the next function (0xbc824)
// FUNCTION: 0x000bc7e0
int LbSpriteReset(TbSprite *start, TbSprite *end, void *data)
{
    for (; start < end; start++) {
        if (start->data >= (u32)data)
            start->data -= (u32)data;
    }
    return 1;
}

// FUNCTION: 0x000bc86c
int LbSpriteSetup(TbSprite *start, TbSprite *end, void *data)
{
    for (; start < end; start++) {
        if (start->data < (u32)data)
            start->data += (u32)data;
    }
    return 1;
}

struct TbSetupSprite {
    TbSprite **Start;
    TbSprite **End;
    u8 **Data;
};

// FUNCTION: 0x000bc8b0
int fn_000bc8b0(TbSetupSprite *t)
{
    for (; t->Start; t++)
        LbSpriteSetup(*t->Start, *t->End, *t->Data);
    return 1;
}

// NONMATCHING: eax/edx swapped in remap pointer; target range also covers the next function (0xbc980)
// FUNCTION: 0x000bc8f8
int LbSpriteDrawScaled(int x, int y, TbSprite *sprite, int dest_width, int dest_height)
{
    if (dest_width <= 0 || dest_height <= 0)
        return 1;
    if (lbDisplay.draw_flags & 0x800)
        lbSpriteReMapPtr = &(*(u8 **)&lbDisplay.unknown8[4])[(lbDisplay.fade_step & 0x3f) * 256];
    LbSpriteSetScalingData(x, y, sprite->width, sprite->height, dest_width, dest_height);
    return LbSpriteDrawUsingScalingData(0, 0, sprite);
}

}
