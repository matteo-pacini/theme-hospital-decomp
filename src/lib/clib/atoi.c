#include "clib.h"

// FUNCTION: 0xbc00f
int atoi(const char *p)
{
    int value;
    char sign;

    while ((int)(unsigned char)isspace(*p))
        ++p;
    sign = *p;
    if ((int)sign == '+' || (int)sign == '-')
        ++p;
    value = 0;
    while ((int)(unsigned char)isdigit(*p)) {
        value = value * 10 + *p - '0';
        ++p;
    }
    if ((int)sign == '-')
        value = -value;
    return value;
}
