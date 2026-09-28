#include "clib.h"

double __math1err(unsigned err_info, double *arg1);

double __log87_err(double x, unsigned code);
#pragma aux __log87_err parm routine [];

// NONMATCHING: the frame pragma adds lea esp,[ebp-0xc] to the epilogue; built
// with -of (and without the pragma) it matches exactly
// FUNCTION: 0xfcf2f
double __log87_err(double x, unsigned code)
{
    unsigned err_code;

    err_code = code & 0x1f;
    if (err_code != 8 && x == 0.0)
        err_code |= 0x1080;
    else
        err_code |= 0x1040;
    return __math1err(err_code, &x);
}
