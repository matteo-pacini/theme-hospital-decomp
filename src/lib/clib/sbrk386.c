#include "clib.h"

#define ENOMEM 5

extern unsigned _STACKTOP;
extern unsigned char _Extender;
extern unsigned char _ExtenderSubtype;
extern unsigned _curbrk;

int *__get_errno_ptr(void);

unsigned short GetDS(void);
#pragma aux GetDS = \
    "mov ax,ds" \
    value [ax];

long TinyGetParentSeg(unsigned sel);
#pragma aux TinyGetParentSeg = \
    "mov ah,0edh" \
    "int 21h" \
    "shl eax,1fh" \
    "and edi,0ffffh" \
    "or edi,eax" \
    parm [ebx] \
    value [edi] \
    modify [eax ebx ecx edx esi];

long TinySetBlock(unsigned short seg, unsigned size);
#pragma aux TinySetBlock = \
    "mov ah,4ah" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    parm [es] [ebx] \
    value [eax] \
    modify [ebx ecx edx esi edi];

// FUNCTION: 0xf47b5
// NONMATCHING: the target shares one TinySetBlock call between both
// branches and places the extender branch after the error exit
void *__brk(unsigned brk_value)
{
    unsigned old_brk_value;
    unsigned seg_size;
    long parent;

    if (brk_value < _STACKTOP)
        goto error;
    if (_Extender == 0) {
        seg_size = (brk_value + 0x0f) >> 4;
        if (seg_size == 0)
            seg_size = 0x0fffffff;
        parent = TinyGetParentSeg(GetDS());
        if (parent < 0) {
            if (TinySetBlock(parent, seg_size) < 0)
                goto error;
        }
        if (TinySetBlock(GetDS(), seg_size) < 0) {
error:
            *__get_errno_ptr() = ENOMEM;
            return (void *)-1;
        }
    } else {
        seg_size = (brk_value + 0xfff) >> 12;
        if (seg_size == 0)
            seg_size = 0xfffff;
        if ((int)_Extender == 1 && (int)_ExtenderSubtype == 1)
            seg_size <<= 8;
        if (TinySetBlock(GetDS(), seg_size) < 0)
            goto error;
    }
    old_brk_value = _curbrk;
    _curbrk = brk_value;
    return (void *)old_brk_value;
}
