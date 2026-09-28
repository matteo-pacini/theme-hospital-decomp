#include "bflib.h"

extern u32 screen_initialised;
extern u32 _lbScreenInitialised;

extern "C" {
extern s32 D_00188c34;
#pragma aux D_00188c34 "W?research_names$npnpna"
extern s32 D_00188c38;
extern u16 D_00188c76;
extern u8 D_00188c7a;

int LbMouseSuspend(void);
void LbVesaSetPage(s16 page);

void rep_movsd(void *dst, void *src, u32 count);
#pragma aux rep_movsd = 0xf3 0xa5 parm [edi] [esi] [ecx] modify [ecx esi edi];

void wait_vbi(void);
#pragma aux wait_vbi = 0xba 0xda 0x03 0x00 0x00 0x29 0xc0 0xec 0xa8 0x08 0x74 0xf4 modify [eax edx];

// FUNCTION: 0x000becd0
int LbScreenClear(u8 colour, ...)
{
    LbMemorySet(lbDisplay.unknown4, colour, D_00188c34 * D_00188c38);
    return 1;
}

// FUNCTION: 0x000becfc
int LbScreenReset(void)
{
    REGS out;
    REGS in;

    if (!screen_initialised)
        return -1;
    LbMouseSuspend();
    in.x.eax = D_00188c76;
    int386(0x10, &in, &out);
    screen_initialised = 0;
    _lbScreenInitialised = 0;
    return 1;
}

// NONMATCHING: target range also covers the next two functions (0xbed68, 0xbed88)
// FUNCTION: 0x000bed5c
int LbIsActive(void)
{
    return 1;
}

// NONMATCHING: target range also covers the following functions up to 0xbf75e
// FUNCTION: 0x000bee60
int LbScreenSwap(void)
{
    s32 remaining;
    s32 chunk;
    u8 *src;
    s16 page;

    LbMousePlace();
    if (!D_00188c7a) {
        rep_movsd(lbDisplay.physical_screen, lbDisplay.unknown4, (D_00188c34 * D_00188c38) >> 2);
    } else {
        remaining = D_00188c34 * D_00188c38;
        src = lbDisplay.unknown4;
        for (page = 0; remaining != 0; src += 0x10000) {
            chunk = remaining < 0x10000 ? remaining : 0x10000;
            remaining -= chunk;
            LbVesaSetPage(page++);
            rep_movsd(lbDisplay.physical_screen, src, chunk >> 2);
        }
    }
    LbMouseRemove();
    return 1;
}

// FUNCTION: 0x000bf760
int LbScreenWaitVbi(void)
{
    wait_vbi();
    return 1;
}

}
