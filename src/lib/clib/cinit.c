#include "clib.h"

extern unsigned _amblksiz;
extern char _Extender;

void __GETDS(void);
#pragma aux __GETDS "*";
extern char C_000bcce3[];

int __DPMI_hosted(void);

short DPMILockLinearRegion(long linear, long size);
#pragma aux DPMILockLinearRegion = \
    "mov di,dx" \
    "shr edx,16" \
    "mov si,dx" \
    "mov bx,ax" \
    "shr eax,16" \
    "mov cx,ax" \
    "mov ax,0600h" \
    "int 31h" \
    "sbb ax,ax" \
    parm caller [eax] [edx] \
    value [ax] \
    modify [ebx ecx esi edi];

// FUNCTION: 0xec934
void __CommonInit(void)
{
    int not_zero_based;

    _amblksiz = 32 * 1024;
    if ((int)_Extender < 2 || (int)_Extender > 8)
        not_zero_based = 1;
    else
        not_zero_based = 0;
    if (not_zero_based) {
        if (__DPMI_hosted() == 1)
            DPMILockLinearRegion((long)__GETDS, C_000bcce3 - (char *)__GETDS);
    }
}
