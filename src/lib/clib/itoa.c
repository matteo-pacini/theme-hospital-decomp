#include "clib.h"

const char D_000ec560[] = "0123456789abcdefghijklmnopqrstuvwxyz";

unsigned __udiv(unsigned value, unsigned __near *radix);
#pragma aux __udiv = \
    "xor edx,edx" \
    "div dword ptr [ebx]" \
    "mov [ebx],eax" \
    parm caller [eax] [ebx] \
    value [edx] \
    modify exact [eax edx];

/* config/functions.txt has no name for utoa. */
#pragma aux utoa "fn_000ec585_"

// FUNCTION: 0xec585
// NONMATCHING: the digit table sits in the code segment (read with a cs:
// override), which -zm cannot reproduce; the target names it
// __set_doserrno_+0xc
char *utoa(unsigned value, char *buffer, int radix)
{
    char *p = buffer;
    char *q;
    unsigned rem;
    unsigned quot;
    char buf[34];

    buf[0] = '\0';
    q = &buf[1];
    do {
        quot = radix;
        rem = __udiv(value, (unsigned __near *)&quot);
        *q = D_000ec560[rem];
        ++q;
        value = quot;
    } while (value != 0);
    while ((*p++ = *--q) != '\0')
        ;
    return buffer;
}

/* config/functions.txt gives itoa the name of the function that follows it. */
#pragma aux itoa "_no_support_loaded_"

// FUNCTION: 0xec5cf
// NONMATCHING: config/functions.txt extends this function over
// _no_support_loaded (0xec5ea) and ultoa's digit table
char *itoa(int value, char *buffer, int radix)
{
    char *p = buffer;

    if (radix == 10) {
        if (value < 0) {
            *p++ = '-';
            value = -value;
        }
    }
    utoa(value, p, radix);
    return buffer;
}
