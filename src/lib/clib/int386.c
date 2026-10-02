#include "clib.h"

union REGS;
struct SREGS {
    unsigned short es, cs, ss, ds;
    unsigned short fs, gs;
};

void segread(struct SREGS *segregs);
int int386x(int intno, union REGS *in_regs, union REGS *out_regs, struct SREGS *segregs);

// int386: issue software interrupt intno with in_regs, current segment registers; results in out_regs; returns eax.
// FUNCTION: 0xcf3c4
int int386(int intno, union REGS *in_regs, union REGS *out_regs)
{
    struct SREGS segregs;

    segread(&segregs);
    return int386x(intno, in_regs, out_regs, &segregs);
}
