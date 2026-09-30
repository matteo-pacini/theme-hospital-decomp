#include "clib.h"

#define ENOMEM 5
#define EINVAL 9

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

/* 0x74 0x10 is jz over the five instructions after it: aux code has no labels. */
void *DPMIAllocMem(unsigned short hi, unsigned short lo);
#pragma aux DPMIAllocMem = \
    "mov ah,5" \
    "mov al,1" \
    "int 31h" \
    "sbb eax,eax" \
    "inc eax" \
    0x74 0x10 \
    "mov ax,bx" \
    "shl eax,16" \
    "mov ax,cx" \
    "mov [eax],di" \
    "mov [eax+2],si" \
    parm [bx] [cx] \
    value [eax] \
    modify [ebx ecx esi edi];

void *TinyMemAlloc(unsigned size);
#pragma aux TinyMemAlloc = \
    "mov eax,80004800h" \
    "int 21h" \
    "sbb ebx,ebx" \
    "not ebx" \
    "and eax,ebx" \
    parm [ebx] \
    value [eax] \
    modify [ebx];

unsigned GetDSLimit(void);
#pragma aux GetDSLimit = \
    "xor eax,eax" \
    "mov ax,ds" \
    "lsl eax,eax" \
    "inc eax" \
    value [eax];

void *__brk(unsigned brk_value);

// FUNCTION: 0xf46ed
void *sbrk(int increment)
{
    void *p;

    if (((int)_Extender == 1 && _ExtenderSubtype == 0) || (int)_Extender == 9) {
        if (increment > 0) {
            increment = (increment + 0xfff) & ~0xfff;
            if ((int)_Extender == 1) {
                p = DPMIAllocMem(increment >> 16, increment);
            } else {
                p = TinyMemAlloc(increment);
            }
            if (p != NULL)
                return p;
            *__get_errno_ptr() = ENOMEM;
        } else {
            *__get_errno_ptr() = EINVAL;
        }
        return (void *)-1;
    }
    if ((int)_Extender >= 2 && (int)_Extender <= 8)
        _curbrk = GetDSLimit();
    return __brk(_curbrk + increment);
}

// FUNCTION: 0xf47b5
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
        if (TinySetBlock(GetDS(), seg_size) < 0) {
            *__get_errno_ptr() = ENOMEM;
            return (void *)-1;
        }
    }
    old_brk_value = _curbrk;
    _curbrk = brk_value;
    return (void *)old_brk_value;
}
