#include "hosp.h"

extern "C" {
u32 ror13(u32 x);
#pragma aux ror13 = "ror eax,13" parm [eax] value [eax];
}

u16 staffrand(void);

// FUNCTION: 0x000b0100
u32 old_newrand(void)
{
    seed = seed * 0x24a1 + 0x24df;
    seed = ror13(seed);
    return seed;
}

// FUNCTION: 0x000b0144
u32 old_normal_rand(u32 low, u32 high)
{
    s32 range;
    u32 sum;

    range = (high - low) / 2 + 1;
    if (range <= 0)
        range = 1;
    sum = old_newrand() % range;
    sum += old_newrand() % range;
    return sum + low;
}

// FUNCTION: 0x000b01a0
u16 saferand(void)
{
    safeseed = safeseed * 0x24a1 + 0x24df;
    safeseed = ror13(safeseed);
    return safeseed;
}

// FUNCTION: 0x000b01e4
u16 staffrand(void)
{
    gamevar.f0 = gamevar.f0 * 0x24a1 + 0x24df;
    gamevar.f0 = ror13(gamevar.f0);
    return gamevar.f0;
}

// FUNCTION: 0x000b0228
u32 normal_staffrand(u32 low, u32 high)
{
    s32 range;
    u32 sum;

    range = (high - low) / 2 + 1;
    if (range <= 0)
        range = 1;
    sum = staffrand() % range;
    sum += staffrand() % range;
    return sum + low;
}

// FUNCTION: 0x000b0294
u32 safe_normal_rand(u32 low, u32 high)
{
    s32 range;
    u32 sum;

    range = (high - low) / 2 + 1;
    if (range <= 0)
        range = 1;
    sum = saferand() % range;
    sum += saferand() % range;
    return sum + low;
}
