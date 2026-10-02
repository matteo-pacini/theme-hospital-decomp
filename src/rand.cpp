#include "hosp.h"

extern "C" {
u32 ror13(u32 x);
#pragma aux ror13 = "ror eax,13" parm [eax] value [eax];
}

u16 staffrand(void);

// Advances the simulation's random generator (seed) and returns its new
// value.
// FUNCTION: 0x000b0100
u32 old_newrand(void)
{
    seed = seed * 0x24a1 + 0x24df;
    seed = ror13(seed);
    return seed;
}

// Returns a random number from low to high (high - 1 when high - low is
// odd), triangularly distributed (the sum of two draws), from old_newrand.
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

// Advances the random generator kept apart from the simulation's seed
// (safeseed), used mostly for sounds and other presentation, and returns its
// low 16 bits.
// FUNCTION: 0x000b01a0
u16 saferand(void)
{
    safeseed = safeseed * 0x24a1 + 0x24df;
    safeseed = ror13(safeseed);
    return safeseed;
}

// Advances the random generator used for new staff (gamevar.staff_seed) and returns
// its low 16 bits.
// FUNCTION: 0x000b01e4
u16 staffrand(void)
{
    gamevar.staff_seed = gamevar.staff_seed * 0x24a1 + 0x24df;
    gamevar.staff_seed = ror13(gamevar.staff_seed);
    return gamevar.staff_seed;
}

// As old_normal_rand, from staffrand.
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

// As old_normal_rand, from saferand.
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
