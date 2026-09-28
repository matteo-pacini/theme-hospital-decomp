#include "clib.h"

const char D_000ec5fc[] = "0123456789abcdefghijklmnopqrstuvwxyz";

unsigned long __uldiv(unsigned long value, unsigned long __near *radix);
#pragma aux __uldiv = \
    "xor edx,edx" \
    "div dword ptr [ebx]" \
    "mov [ebx],edx" \
    parm caller [eax] [ebx] \
    value [eax] \
    modify exact [eax edx];

// FUNCTION: 0xec621
// NONMATCHING: the digit table sits in the code segment (read with a cs:
// override), which -zm cannot reproduce; the target names it
// _no_support_loaded_+0x2d
char *ultoa(unsigned long value, char *buffer, int radix)
{
    char *p = buffer;
    char *q;
    unsigned long rem;
    char buf[34];

    buf[0] = '\0';
    q = &buf[1];
    do {
        rem = radix;
        value = __uldiv(value, (unsigned long __near *)&rem);
        *q = D_000ec5fc[rem];
        ++q;
    } while (value != 0);
    while ((*p++ = *--q) != '\0')
        ;
    return buffer;
}

// FUNCTION: 0xec66b
char *ltoa(long value, char *buffer, int radix)
{
    char *p = buffer;

    if (radix == 10) {
        if (value < 0) {
            *p++ = '-';
            value = -value;
        }
    }
    ultoa(value, p, radix);
    return buffer;
}
