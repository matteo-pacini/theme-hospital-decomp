#include "clib.h"

double _Scale10V(double x, int scale);
#pragma aux _Scale10V parm routine [];
void __Rnd2Int(double *x, void *bigint);
#pragma aux __Rnd2Int "*" parm [eax] [edx];
int __CmpBigInt(int digits, void *bigint);
#pragma aux __CmpBigInt "*" parm [eax] [edx];

int _Scale(int exp, double x, int digits, void *bigint);
#pragma aux _Scale parm [eax];

// _Scale: scale x by 10^(digits - exp), round it to an integer in bigint, and return __CmpBigInt(digits, bigint).
// FUNCTION: 0xfca97
int _Scale(int exp, double x, int digits, void *bigint)
{
    if (exp != digits)
        x = _Scale10V(x, digits - exp);
    __Rnd2Int(&x, bigint);
    return __CmpBigInt(digits, bigint);
}

typedef struct {
    short exp;
    double value;
} BIGPOW10;

extern const BIGPOW10 _BigPow10Table[];

/*
 * _Scale10V is matched from generated assembly (scale10v.s). Watcom 10.6
 * wraps each -fpd FDIV check in pushfd/popfd, which the compiler that built
 * the original did not, and the check cannot be written out as inline code
 * (see docs/MATCHING.md). Its source, as decompiled (INF is +infinity; the
 * four constants it compares with and substitutes are the module's CONST2,
 * defined in scale10v.s):
 */
#if 0
double _Scale10V(double x, int scale)
{
    int n;
    double factor;
    const BIGPOW10 *table;

    if (x == INF) {
        x = DBL_MAX;
    } else if (x == -INF) {
        x = -DBL_MAX;
    }
    table = _BigPow10Table;
    if (scale < 0) {
        n = -scale;
        if (n > 308) {
            x /= table->value;
            n -= 216;
        }
    } else {
        n = scale;
        if (n > 308) {
            x *= table->value;
            n -= 216;
        }
    }
    factor = 1.0;
    for (;;) {
        if (n >= table->exp) {
            factor *= table->value;
            n -= table->exp;
        }
        if (n == 0)
            break;
        if ((int)table->exp != 1)
            ++table;
    }
    if (scale < 0) {
        x /= factor;
    } else {
        x *= factor;
    }
    return x;
}
#endif
