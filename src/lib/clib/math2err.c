#include "clib.h"

struct exception {
    int type;
    char *name;
    double arg1;
    double arg2;
    double retval;
};

#define DOMAIN    1
#define SING      2
#define OVERFLOW  3
#define UNDERFLOW 4
#define TLOSS     5
#define PLOSS     6

#define M_DOMAIN    0x0040
#define M_SING      0x0080
#define M_OVERFLOW  0x0100
#define M_PLOSS     0x0200
#define M_TLOSS     0x0400
#define M_UNDERFLOW 0x0800

#define V_NEG_HUGEVAL 0x1000
#define V_ZERO        0x2000
#define V_ONE         0x4000
#define V_HUGEVAL     0x8000

/* Function names, indexed by the low five bits of err_info (from 1). */
static char *MathFuncNames[] = {
    "acos",
    "asin",
    "sqrt",
    "exp",
    "cosh",
    "sinh",
    "pow",
    "acosh",
    "log2",
    "log",
    "log10",
    "atanh",
    "atan2",
    "ipow",
    "dpowi",
    "cos",
    "sin",
    "tan",
    "y0",
    "y1",
    "yn",
    "mod",
    "cotan"
};
extern const double _HugeValue;

double _matherr(struct exception *excp);
double __math2err(unsigned err_info, double *arg1, double *arg2);

// __math1err: __math2err for a one-argument function.
// FUNCTION: 0xfdbf2
double __math1err(unsigned err_info, double *arg1)
{
    return __math2err(err_info, arg1, arg1);
}

// __math2err: build a struct exception from err_info (error kind, function name, default result) and the arguments, and pass it to _matherr; returns the result.
// FUNCTION: 0xfdbfc
double __math2err(unsigned err_info, double *arg1, double *arg2)
{
    struct exception exc;
    int why;

    exc.arg1 = *arg1;
    exc.arg2 = *arg2;
    if (err_info & M_DOMAIN) {
        why = DOMAIN;
    } else if (err_info & M_SING) {
        why = SING;
    } else if (err_info & M_OVERFLOW) {
        why = OVERFLOW;
    } else if (err_info & M_UNDERFLOW) {
        why = UNDERFLOW;
    } else if (err_info & M_PLOSS) {
        why = PLOSS;
    } else if (err_info & M_TLOSS) {
        why = TLOSS;
    }
    exc.type = why;
    exc.name = MathFuncNames[(err_info & 0x1f) - 1];
    if (err_info & V_NEG_HUGEVAL) {
        exc.retval = -_HugeValue;
    } else if (err_info & V_ZERO) {
        exc.retval = 0.0;
    } else if (err_info & V_ONE) {
        exc.retval = 1.0;
    } else if (err_info & V_HUGEVAL) {
        exc.retval = _HugeValue;
    } else {
        exc.retval = *arg2;
    }
    return _matherr(&exc);
}
