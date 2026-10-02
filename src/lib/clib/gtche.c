#include "clib.h"

extern int _cbyte;

unsigned char _TinyGetCharEcho(void);
#pragma aux _TinyGetCharEcho = \
    "mov ah,1" \
    "int 21h" \
    value [al] modify [eax];

// getche: read a keystroke with echo (a pushed-back _cbyte first).
// FUNCTION: 0xfc8ff
int getche(void)
{
    int c;

    c = _cbyte;
    _cbyte = 0;
    if (c == 0)
        c = _TinyGetCharEcho();
    return c;
}
