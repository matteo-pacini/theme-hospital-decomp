#include "clib.h"

extern unsigned _STACKLOW;

unsigned _SP(void);
#pragma aux _SP = \
    "mov eax,esp" \
    value [eax];

// FUNCTION: 0xec92b
unsigned stackavail(void)
{
    return _SP() - _STACKLOW;
}
