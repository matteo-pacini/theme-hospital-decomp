#include "bflib.h"

extern "C" {

// FUNCTION: 0x000bf778
int LbDate(void)
{
    return 1;
}

// FUNCTION: 0x000c21c0
u32 LbTimerClock(void)
{
    return clock() * 1000 / 100;
}

}
