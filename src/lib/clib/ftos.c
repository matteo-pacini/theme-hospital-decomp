#include "clib.h"

extern int D_00152ac8;

// FUNCTION: 0xfc117
void _SetMaxPrec(int max_prec)
{
    D_00152ac8 = max_prec;
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

// NONMATCHING: the target shares DoEFormat's epilogue (built without -zm)
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
#pragma aux AdjField frame;

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
