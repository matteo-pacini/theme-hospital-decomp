#include "clib.h"

/* The inline strcpy of the Watcom headers, two bytes per iteration. */
char *_inline_strcpy(char *dst, const char *src);
#pragma aux _inline_strcpy = \
    0x50                    /* push eax */ \
    0x8a 0x0a               /* next: mov cl,[edx] */ \
    0x88 0x08               /* mov [eax],cl */ \
    0x80 0xf9 0x00          /* cmp cl,0 */ \
    0x74 0x11               /* je done */ \
    0x8a 0x4a 0x01          /* mov cl,[edx+1] */ \
    0x83 0xc2 0x02          /* add edx,2 */ \
    0x88 0x48 0x01          /* mov [eax+1],cl */ \
    0x83 0xc0 0x02          /* add eax,2 */ \
    0x80 0xf9 0x00          /* cmp cl,0 */ \
    0x75 0xe6               /* jne next */ \
    0x58                    /* done: pop eax */ \
    parm [eax] [edx] value [eax] modify [ecx edx];

// FUNCTION: 0xbbff0
char *strcpy(char *dst, const char *src)
{
    return _inline_strcpy(dst, src);
}
