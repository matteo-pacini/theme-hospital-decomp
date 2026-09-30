#include "clib.h"

#define EDOM   13
#define ERANGE 14

int tolower(int c);

/* Indexed by radix - 2: the largest value that can take another digit. */
static unsigned long nearly_overflowing[] = {
    0xffffffffUL / 2, 0xffffffffUL / 3, 0xffffffffUL / 4, 0xffffffffUL / 5, 0xffffffffUL / 6,
    0xffffffffUL / 7, 0xffffffffUL / 8, 0xffffffffUL / 9, 0xffffffffUL / 10, 0xffffffffUL / 11,
    0xffffffffUL / 12, 0xffffffffUL / 13, 0xffffffffUL / 14, 0xffffffffUL / 15, 0xffffffffUL / 16,
    0xffffffffUL / 17, 0xffffffffUL / 18, 0xffffffffUL / 19, 0xffffffffUL / 20, 0xffffffffUL / 21,
    0xffffffffUL / 22, 0xffffffffUL / 23, 0xffffffffUL / 24, 0xffffffffUL / 25, 0xffffffffUL / 26,
    0xffffffffUL / 27, 0xffffffffUL / 28, 0xffffffffUL / 29, 0xffffffffUL / 30, 0xffffffffUL / 31,
    0xffffffffUL / 32, 0xffffffffUL / 33, 0xffffffffUL / 34, 0xffffffffUL / 35, 0xffffffffUL / 36
};

int radix_value(char c);

// FUNCTION: 0xeeeb9
unsigned long _stol(const char *nptr, char **endptr, int base, int who)
{
    const char *p;
    const char *startp;
    int digit;
    unsigned long value;
    unsigned long prev_value;
    char sign;
    char overflow;

    if (endptr != NULL)
        *endptr = (char *)nptr;
    p = nptr;
    while ((int)(unsigned char)isspace(*p))
        ++p;
    sign = *p;
    if ((int)sign == '+' || (int)sign == '-')
        ++p;
    if (base == 0) {
        if ((int)p[0] == '0' && ((int)p[1] == 'x' || (int)p[1] == 'X')) {
            base = 16;
        } else if ((int)*p == '0') {
            base = 8;
        } else {
            base = 10;
        }
    }
    if (base < 2 || base > 36) {
        __set_errno(EDOM);
        return 0;
    }
    if (base == 16) {
        if ((int)p[0] == '0' && ((int)p[1] == 'x' || (int)p[1] == 'X'))
            p += 2;
    }
    startp = p;
    overflow = 0;
    value = 0;
    for (;;) {
        digit = radix_value(*p);
        if (digit >= base)
            break;
        if (value > nearly_overflowing[base - 2])
            overflow = 1;
        prev_value = value;
        value = value * base + digit;
        if (value < prev_value)
            overflow = 1;
        ++p;
    }
    if (p == startp)
        p = nptr;
    if (endptr != NULL)
        *endptr = (char *)p;
    if (who == 1) {
        if (value >= 0x80000000) {
            if (value == 0x80000000 && (int)sign == '-') {
            } else {
                overflow = 1;
            }
        }
    }
    if (overflow) {
        __set_errno(ERANGE);
        if (who == 0)
            return 0xffffffff;
        if ((int)sign == '-')
            return 0x80000000;
        return 0x7fffffff;
    }
    if ((int)sign == '-')
        value = -value;
    return value;
}

// FUNCTION: 0xef01c
unsigned long strtoul(const char *nptr, char **endptr, int base)
{
    return _stol(nptr, endptr, base, 0);
}

// FUNCTION: 0xef026
long strtol(const char *nptr, char **endptr, int base)
{
    return _stol(nptr, endptr, base, 1);
}

// FUNCTION: 0xef02e
int radix_value(char c)
{
    if ((int)c >= '0' && (int)c <= '9')
        return c - '0';
    c = tolower(c);
    if ((int)c >= 'a' && (int)c <= 'i')
        return c - 'a' + 10;
    if ((int)c >= 'j' && (int)c <= 'r')
        return c - 'j' + 19;
    if ((int)c >= 's' && (int)c <= 'z')
        return c - 's' + 28;
    return 37;
}
