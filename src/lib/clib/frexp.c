#include "clib.h"

double frexp(double value, int *exp);
#pragma aux frexp parm routine [];

// frexp: split value into a mantissa in [0.5, 1) and a power of two stored in *exp.
// FUNCTION: 0xfd868
double frexp(double value, int *exp)
{
    int n;
    union {
        double d;
        unsigned short w[4];
    } u;

    n = 0;
    u.d = value;
    if (value != 0.0) {
        n = ((u.w[3] & 0x7ff0) >> 4) - 0x3fe;
        u.w[3] &= 0x800f;
        u.w[3] |= 0x3fe0;
    }
    *exp = n;
    return u.d;
}
