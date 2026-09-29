#include "clib.h"

#define SIGN     0x01
#define EXPSIGN  0x02
#define DIGITS   0x04
#define DOT      0x08

#define MAX_SIG_DIGITS 17

extern const double D_00128eb0;

void __ZBuf2F(char *buf, double *value);
/* config/functions.txt has no name for __ZBuf2F. */
#pragma aux __ZBuf2F "fn_000fce62_" parm [eax] [edx];
void __set_ERANGE(void);
double _Scale10V(double x, int scale);
#pragma aux _Scale10V parm routine [];

double strtod(const char *s, char **endptr);
/* config/functions.txt has no name for strtod. */
#pragma aux strtod "fn_000fc4e3_"

// NONMATCHING: the exponent sign is read with movzx into edi instead of into
// dl, and the exponent digit is accumulated in esi rather than edx
// FUNCTION: 0xfc4e3
double strtod(const char *s, char **endptr)
{
    unsigned char flags;
    char buf[20];
    const char *endp;
    unsigned char c;
    const char *exp_start;
    int exp;
    int dec_exp;
    unsigned char nonzero;
    int ndigits;
    double value;

    endp = s;
    for (;;) {
        c = *s;
        if (!((int)c == ' ' || ((int)c >= 9 && (int)c <= 13)))
            break;
        ++s;
    }
    flags = 0;
    if ((int)c == '+') {
        ++s;
    } else if ((int)c == '-') {
        flags = SIGN;
        ++s;
    }
    ndigits = 0;
    nonzero = '0';
    dec_exp = 0;
    for (;;) {
        c = *s++;
        if ((int)c == '.') {
            if ((int)(unsigned char)(flags & DOT) != 0)
                break;
            flags |= DOT;
            continue;
        }
        if ((int)c < '0' || (int)c > '9')
            break;
        if ((int)(unsigned char)(flags & DOT) != 0)
            ++dec_exp;
        if ((int)(nonzero |= c) != '0') {
            if (ndigits < MAX_SIG_DIGITS)
                buf[ndigits] = c;
            ++ndigits;
        }
        flags |= DIGITS;
    }
    exp = 0;
    if ((int)(unsigned char)(flags & DIGITS) != 0) {
        if ((int)c == 'e' || (int)c == 'E') {
            exp_start = s - 1;
            if ((int)*s == '+') {
                ++s;
            } else if ((int)*s == '-') {
                flags |= EXPSIGN;
                ++s;
            }
            flags &= ~DIGITS;
            for (;;) {
                c = *s;
                if ((int)c < '0' || (int)c > '9')
                    break;
                if (exp < 1000)
                    exp = c + exp * 10 - '0';
                flags |= DIGITS;
                ++s;
            }
            if ((int)(unsigned char)(flags & EXPSIGN) != 0)
                exp = -exp;
            if ((int)(unsigned char)(flags & DIGITS) == 0)
                s = exp_start;
        } else {
            --s;
        }
        endp = s;
    }
    exp -= dec_exp;
    if (ndigits > MAX_SIG_DIGITS) {
        exp += ndigits - MAX_SIG_DIGITS;
        ndigits = MAX_SIG_DIGITS;
    }
    while (ndigits > 0 && (int)buf[ndigits - 1] == '0') {
        ++exp;
        --ndigits;
    }
    if (ndigits == 0) {
        value = 0.0;
    } else {
        buf[ndigits] = '\0';
        __ZBuf2F(buf, &value);
        if (exp + ndigits - 1 > 308) {
            __set_ERANGE();
            if ((int)(unsigned char)(flags & SIGN) != 0) {
                value = -D_00128eb0;
            } else {
                value = D_00128eb0;
            }
        } else if (exp + ndigits - 1 < -308) {
            __set_ERANGE();
            value = 0.0;
        } else {
            if (exp != 0)
                value = _Scale10V(value, exp);
            if ((int)(unsigned char)(flags & SIGN) != 0)
                value = -value;
        }
    }
    if (endptr != NULL)
        *endptr = (char *)endp;
    return value;
}
