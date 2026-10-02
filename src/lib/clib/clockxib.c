#include "rtinit.h"

typedef unsigned long clock_t;

clock_t __clock(void);
void __clock_init(void);

// clock: processor time used, in CLOCKS_PER_SEC (100) ticks since startup.
// FUNCTION: 0xcf432
clock_t clock(void)
{
    return __clock();
}

AXI(__clock_init, 0x20)
