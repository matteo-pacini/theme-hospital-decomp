#include "clib.h"

extern unsigned _amblksiz;
extern char _Extender;

void __GETDS(void);
#pragma aux __GETDS "*";
extern char __GETDSEnd[];
#pragma aux __GETDSEnd "___GETDSEnd_"

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

// __CommonInit: startup init; sets _amblksiz to 32K and, unless running under Phar Lap (_Extender 2-8), locks the __GETDS code region when a DPMI host is present.
// FUNCTION: 0xec934
void __CommonInit(void)
{
    int not_pharlap;

    _amblksiz = 32 * 1024;
    if ((int)_Extender < 2 || (int)_Extender > 8)
        not_pharlap = 1;
    else
        not_pharlap = 0;
    if (not_pharlap) {
        if (__DPMI_hosted() == 1)
            DPMILockLinearRegion((long)__GETDS, __GETDSEnd - (char *)__GETDS);
    }
}
