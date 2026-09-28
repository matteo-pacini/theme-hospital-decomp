#include "clib.h"

double __math2err(unsigned err_info, double *arg1, double *arg2);

// FUNCTION: 0xfdbf2
double __math1err(unsigned err_info, double *arg1)
{
    return __math2err(err_info, arg1, arg1);
}
