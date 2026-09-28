#include "clib.h"

double _Scale10V(double x, int scale);
#pragma aux _Scale10V parm routine [];
void __Rnd2Int(double *x, void *bigint);
#pragma aux __Rnd2Int "*" parm [eax] [edx];
int __CmpBigInt(int digits, void *bigint);
#pragma aux __CmpBigInt "*" parm [eax] [edx];

int _Scale(int exp, double x, int digits, void *bigint);
#pragma aux _Scale parm [eax] frame;

// NONMATCHING: the frame pragma adds lea esp,[ebp-0x10] to the epilogue;
// built with -of (and without the pragma) it matches exactly
// FUNCTION: 0xfca97
int _Scale(int exp, double x, int digits, void *bigint)
{
    if (exp != digits)
        x = _Scale10V(x, digits - exp);
    __Rnd2Int(&x, bigint);
    return __CmpBigInt(digits, bigint);
}
