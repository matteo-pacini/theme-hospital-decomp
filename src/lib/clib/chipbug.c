#include "clib.h"
#include "rtinit.h"

extern unsigned char D_00150815;
extern unsigned char _chipbug;

/* Sets bit 0 of _chipbug on a Pentium with the FDIV bug (387 or later only). */
// FUNCTION: 0xfdcd7
void verify_pentium_fdiv_bug(void)
{
    volatile double num;
    volatile double den;

    num = 4195835.0;
    den = 3145727.0;
    if ((int)D_00150815 >= 3) {
        if (num - (num / den) * den > 1.0e-8)
            _chipbug |= 1;
    }
}

AXI(verify_pentium_fdiv_bug, 3)
