#include "clib.h"

double modf(double value, double *iptr);
#pragma aux modf parm routine [];
double floor(double x);
#pragma aux floor parm routine [] frame;

// floor: largest integral value not greater than x.
// FUNCTION: 0xfca3d
double floor(double x)
{
    double ipart;

    if (modf(x, &ipart) < 0.0)
        ipart += -1.0;
    return ipart;
}
