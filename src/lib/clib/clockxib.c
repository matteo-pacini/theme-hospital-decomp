#include "rtinit.h"

typedef unsigned long clock_t;

clock_t __clock(void);
void __clock_init(void);

// FUNCTION: 0xcf432
clock_t clock(void)
{
    return __clock();
}

AXI(__clock_init, 0x20)
