#include "clib.h"

// Alphabet: digit characters for radix conversion; a table in the code segment, not a function.
// FUNCTION: 0xec560
static const char __based(__segname("_CODE")) Alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";

unsigned __udiv(unsigned value, unsigned __near *radix);
#pragma aux __udiv = \
    "xor edx,edx" \
    "div dword ptr [ebx]" \
    "mov [ebx],eax" \
    parm caller [eax] [ebx] \
    value [edx] \
    modify exact [eax edx];


// utoa: write unsigned value in radix (2-36) to buffer; returns buffer.
// FUNCTION: 0xec585
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
        *q = Alphabet[rem];
        ++q;
        value = quot;
    } while (value != 0);
    while ((*p++ = *--q) != '\0')
        ;
    return buffer;
}


// itoa: write value in radix (2-36) to buffer, signed only for radix 10; returns buffer.
// FUNCTION: 0xec5cf
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
