#include "clib.h"

extern const double D_00128898;

double modf(double value, double *iptr);
#pragma aux modf parm routine [];
double floor(double x);
#pragma aux floor parm routine [] frame;

// FUNCTION: 0xfca3d
double floor(double x)
{
    double ipart;

    if (modf(x, &ipart) < 0.0)
        ipart += D_00128898;
    return ipart;
}
