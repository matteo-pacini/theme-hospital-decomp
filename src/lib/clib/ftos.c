#include "clib.h"

static int MaxPrec = 17;

// FUNCTION: 0xfc117
void _SetMaxPrec(int max_prec)
{
    MaxPrec = max_prec;
}

// FUNCTION: 0xfc11d
static int DoEFormat(char *p, int exp, int exp_digits, int width)
{
    char sign;
    int divisor;
    int ndigits;
    int len;

    if (exp < 0) {
        sign = '-';
        exp = -exp;
    } else {
        sign = '+';
    }
    divisor = 100;
    ndigits = 3;
    if (exp < divisor) {
        divisor = 10;
        ndigits = 2;
        if (exp < divisor) {
            ndigits = 1;
            divisor = ndigits;
        }
    }
    if (exp_digits == 0) {
        exp_digits = 2;
        if (ndigits == 3)
            exp_digits = ndigits;
    }
    len = exp_digits + 1;
    if (len <= width) {
        if (ndigits <= exp_digits) {
            *p = sign;
            for (;;) {
                ++p;
                if (exp_digits <= ndigits)
                    break;
                --exp_digits;
                *p = '0';
            }
            do {
                *p = exp / divisor + '0';
                exp %= divisor;
                divisor /= 10;
                ++p;
            } while (divisor != 0);
        } else {
            len = ndigits + 1;
        }
    }
    return len;
}

// FUNCTION: 0xfc1c0
static char *DoFFormat(char *p, char *digits, int ndigits, int frac)
{
    if (frac == 0 && ndigits <= 0) {
        *p++ = '0';
        *p++ = '.';
        return p;
    }
    while (ndigits > 0 && *digits != '\0') {
        *p++ = *digits++;
        --ndigits;
    }
    if (ndigits > 0) {
        do {
            --ndigits;
            *p = '0';
            ++p;
        } while (ndigits > 0);
    }
    *p = '.';
    ++p;
    if (frac > 0) {
        while (ndigits != 0) {
            ++ndigits;
            *p = '0';
            ++p;
            if (--frac == 0)
                break;
        }
    }
    if (frac > 0) {
        while (*digits != '\0') {
            *p++ = *digits++;
            if (--frac == 0)
                break;
        }
        while (frac != 0) {
            --frac;
            *p = '0';
            ++p;
        }
    }
    return p;
}

void *memset(void *s, int c, size_t n);

static void AdjField(char *p, int len, int width);

// FUNCTION: 0xfc223
static void AdjField(char *p, int len, int width)
{
    char *src;
    char *dst;
    char *start;
    char c1;

    if (len != width) {
        c1 = p[1];
        start = p;
        dst = (char *)(width + (int)p);
        src = (char *)(len + (int)p);
        do {
            *--dst = *--src;
            --len;
            --width;
        } while (src != start);
        if ((int)*p == '.') {
            p[--width] = '0';
        } else if (((int)*p == '+' || (int)*p == '-') && (int)c1 == '.') {
            p[width] = '0';
            --width;
            p[width] = *p;
        }
        memset(p, ' ', width);
    }
}

int __Nan_Inf(double value, char *buf);
#pragma aux __Nan_Inf parm routine [];
double _Scale10V(double x, int scale);
#pragma aux _Scale10V parm routine [];
void __cvt(double value, int ndigits, int *dec, int *sign, char fmt, char *buf);
#pragma aux __cvt parm routine [];
/* The 387 log10 entry: argument and result on the FPU stack. */
double IF_LOG10(double x);
#pragma aux IF_LOG10 "IF@LOG10" parm [8087] value [8087];
double floor(double x);
double fabs(double x);
#pragma intrinsic(fabs)

// NONMATCHING: the overflow test computes p - buf in edx and the digit count
// in eax, the other way round
// FUNCTION: 0xfc294
void _FtoS(char *buf, double *value, int digits, char plus, int width,
           int scale, int exp_digits, char exp_char, char fmt)
{
    char *p;
    char cfmt;
    int ndigits;
    char tmp[20];
    double v;
    int len;
    int exp;
    int dec;
    double x;
    int sign;

    if (__Nan_Inf(*value, tmp)) {
        p = buf;
        for (len = 0; tmp[len] != '\0'; ++p, ++len) {
            if (len < width)
                *p = tmp[len];
        }
    } else {
        cfmt = fmt;
        if ((int)fmt == 'G') {
            x = fabs(*value);
            if (x != 0.0) {
                x = IF_LOG10(x);
                exp = floor(x);
                if (exp < -4 || exp >= digits) {
                    cfmt = 'E';
                } else {
                    if (x >= 0.0)
                        exp = exp + 1;
                    digits -= exp;
                    if (exp < 0)
                        --digits;
                    scale = 0;
                    cfmt = 'F';
                }
            } else {
                scale = 0;
                cfmt = 'F';
            }
        }
        if ((int)cfmt == 'E') {
            if (-digits >= scale || scale >= digits + 2) {
            overflow:
                memset(buf, '*', width);
                return;
            }
        }
        ndigits = digits;
        if ((int)fmt == 'E') {
            if (scale > 0) {
                ndigits = digits + 1;
            } else if (scale < 0) {
                ndigits = scale + digits;
            }
        }
        v = *value;
        if (v != 0.0 && (int)cfmt != 'E' && scale != 0)
            v = _Scale10V(v, scale);
        __cvt(v, ndigits, &dec, &sign, cfmt, tmp);
        p = buf;
        if (sign) {
            *p++ = '-';
        } else if (plus) {
            *p++ = '+';
        }
        if ((int)cfmt == 'E') {
            p = DoFFormat(p, tmp, scale, ndigits - scale);
            len = p - buf;
            if (exp_char && len < width) {
                *p++ = exp_char;
                ++len;
            }
            if (v != 0.0)
                dec -= scale;
            len += DoEFormat(p, dec, exp_digits, width - len);
        } else if (width < (dec + 1 + ndigits) + (p - buf)) {
            len = width + 1;
        } else {
            p = DoFFormat(p, tmp, dec, ndigits);
            len = p - buf;
        }
    }
    if (len > width)
        goto overflow;
    AdjField(buf, len, width);
}
