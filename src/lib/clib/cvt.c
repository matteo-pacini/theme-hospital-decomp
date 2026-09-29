#include "clib.h"


int __Nan_Inf(double value, char *buf);
#pragma aux __Nan_Inf parm routine [];
double frexp(double value, int *exp);
#pragma aux frexp parm routine [];
double modf(double value, double *iptr);
#pragma aux modf parm routine [];
int _Scale(int exp, double x, int digits, void *bigint);
#pragma aux _Scale parm [eax];
void __Bin2String(void *bigint, char *buf, int digits);
#pragma aux __Bin2String "*" parm [eax] [edx] [ebx];
char *strcpy(char *dst, const char *src);
#pragma intrinsic(strcpy)

char *__cvt(double value, int ndigits, int *dec, int *sign, int fmt, char *buf);
#pragma aux __cvt parm routine [];

// NONMATCHING: the zero test of bigint accumulates in edi instead of eax,
// and exact is stored as an immediate rather than through al
// FUNCTION: 0xfcbf5
char *__cvt(double value, int ndigits, int *dec, int *sign, int fmt, char *buf)
{
    double ipart;
    char tmp[20];
    short bigint[4];
    int n;
    int i;
    char exact;

    *sign = 0;
    *dec = 0;
    if (__Nan_Inf(value, buf))
        return buf;
    bigint[0] = 0;
    bigint[1] = 0;
    bigint[2] = 0;
    bigint[3] = 0;
    if (value != 0.0) {
        if (value < 0.0) {
            value = -value;
            *sign = -1;
        }
        frexp(value, dec);
        n = *dec;
        if (n < 0)
            n = -n;
        n = (n * 3 + 5) / 10;
        if (*dec < 0)
            n = -n;
        *dec = n;
        if (fmt == 'F')
            ndigits += n;
        if (ndigits >= 0) {
            if (ndigits > 16)
                ndigits = 16;
            exact = 0;
            if (modf(value, &ipart) == 0.0 && ndigits > n) {
                exact = 1;
                ndigits = n;
            }
            i = _Scale(*dec, value, ndigits, bigint);
            if (i != 0) {
                *dec += i;
                if (fmt == 'F' || exact)
                    ndigits += i;
                if (ndigits < 1) {
                    ndigits = 1;
                } else if (ndigits > 16) {
                    ndigits = 16;
                }
                if (_Scale(*dec, value, ndigits, bigint) > 0) {
                    ++*dec;
                    ++ndigits;
                }
            }
        }
        if ((bigint[0] | bigint[1] | bigint[2] | bigint[3]) == 0) {
            *sign = 0;
            *dec = 0;
            if (fmt == 'F')
                ndigits -= n;
        }
    }
    if (ndigits < 1) {
        ndigits = 1;
    } else if (ndigits > 17) {
        ndigits = 17;
    }
    __Bin2String(bigint, tmp, ndigits);
    strcpy(buf, tmp);
    return buf;
}
