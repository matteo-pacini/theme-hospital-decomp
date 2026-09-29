#include "clib.h"

double _Scale10V(double x, int scale);
#pragma aux _Scale10V parm routine [];
void __Rnd2Int(double *x, void *bigint);
#pragma aux __Rnd2Int "*" parm [eax] [edx];
int __CmpBigInt(int digits, void *bigint);
#pragma aux __CmpBigInt "*" parm [eax] [edx];

int _Scale(int exp, double x, int digits, void *bigint);
#pragma aux _Scale parm [eax];

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
extern const double D_00128e90;
extern const double D_00128e98;
extern const double D_00128ea0;
extern const double D_00128ea8;

// NONMATCHING: Watcom 10.6 wraps each -fpd FDIV check in pushfd/popfd, which
// the compiler that built the original did not
// FUNCTION: 0xfcad8
double _Scale10V(double x, int scale)
{
    int n;
    double factor;
    const BIGPOW10 *table;

    if (x == D_00128e90) {
        x = D_00128ea0;
    } else if (x == D_00128e98) {
        x = D_00128ea8;
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
