#include "clib.h"

extern unsigned char D_00150815;
extern unsigned char _chipbug;
extern const double D_0012890c;

/* Sets bit 0 of _chipbug on a Pentium with the FDIV bug (387 or later only). */
// FUNCTION: 0xfdcd7
void fn_000fdcd7(void)
{
    volatile double num;
    volatile double den;

    num = 4195835.0;
    den = 3145727.0;
    if ((int)D_00150815 >= 3) {
        if (num - (num / den) * den > D_0012890c)
            _chipbug |= 1;
    }
}
