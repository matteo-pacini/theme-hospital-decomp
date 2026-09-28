#include "clib.h"

extern int _cbyte;

unsigned char _TinyGetCharNoEcho(void);
#pragma aux _TinyGetCharNoEcho = \
    "mov ah,8" \
    "int 21h" \
    value [al] modify [eax];

// FUNCTION: 0xecd44
int getch(void)
{
    int c;

    c = _cbyte;
    _cbyte = 0;
    if (c == 0)
        c = _TinyGetCharNoEcho();
    return c;
}
