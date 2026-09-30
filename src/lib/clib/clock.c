#include "clib.h"

typedef unsigned long clock_t;

struct BYTEREGS {
    unsigned char al, ah;
    unsigned short _1;
    unsigned char bl, bh;
    unsigned short _2;
    unsigned char cl, ch;
    unsigned short _3;
    unsigned char dl, dh;
    unsigned short _4;
};

struct DWORDREGS {
    unsigned int eax, ebx, ecx, edx, esi, edi, cflag;
};

union REGS {
    struct DWORDREGS x;
    struct BYTEREGS h;
};

int intdos(union REGS *in_regs, union REGS *out_regs);

extern clock_t D_0018cfac;

// FUNCTION: 0xec34a
clock_t clock_value(void)
{
    union REGS regs;

    regs.h.ah = 0x2c;
    intdos(&regs, &regs);
    return ((regs.h.ch * 60UL + regs.h.cl) * 60 + regs.h.dh) * 100 + regs.h.dl;
}

// FUNCTION: 0x000ec3a5
void __clock_init(void)
{
    D_0018cfac = clock_value();
}

// FUNCTION: 0x000ec3b0
clock_t __clock(void)
{
    clock_t ticks;

    ticks = clock_value();
    if (ticks < D_0018cfac)
        ticks += 24UL * 60 * 60 * 100;
    return ticks - D_0018cfac;
}
