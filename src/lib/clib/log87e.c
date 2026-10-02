#include "clib.h"

double __math1err(unsigned err_info, double *arg1);

double __log87_err(double x, unsigned code);
#pragma aux __log87_err parm routine [];

// __log87_err: report a log-family error for x through __math1err (singularity for x == 0 except acosh, else domain error; result -HUGE_VAL).
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
