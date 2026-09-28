#include "clib.h"

union REGS;
struct SREGS {
    unsigned short es, cs, ss, ds;
    unsigned short fs, gs;
};

void segread(struct SREGS *segregs);
int int386x(int intno, union REGS *in_regs, union REGS *out_regs, struct SREGS *segregs);

// FUNCTION: 0xcf3c4
// NONMATCHING: config/functions.txt runs int386_ over the C++ runtime
// helpers that follow it (0xcf3e1..0xcf40f)
int int386(int intno, union REGS *in_regs, union REGS *out_regs)
{
    struct SREGS segregs;

    segread(&segregs);
    return int386x(intno, in_regs, out_regs, &segregs);
}
