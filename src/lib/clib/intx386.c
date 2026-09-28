#include "clib.h"

struct DWORDREGS {
    unsigned int eax, ebx, ecx, edx, esi, edi;
    unsigned int cflag;
};
union REGS {
    struct DWORDREGS x;
};
struct SREGS;

void __int386x(int intno, union REGS *in_regs, union REGS *out_regs, struct SREGS *segregs);
#pragma aux __int386x parm [esi] [edi] [edx] [ebx] modify [eax ebx ecx edx esi edi];

// FUNCTION: 0xd0b43
int int386x(int intno, union REGS *in_regs, union REGS *out_regs, struct SREGS *segregs)
{
    __int386x(intno, in_regs, out_regs, segregs);
    return out_regs->x.eax;
}
