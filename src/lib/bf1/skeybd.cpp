#include "bflib.h"

extern "C" {

extern u8 D_00189790;
extern u8 D_00189688;
extern volatile u8 D_0018978d;
extern u8 D_0018978e;
extern u8 D_0018978f;
extern u8 D_0018cfb8;

s16 in_port(u32 port);
#pragma aux in_port = \
    "sub eax,eax" \
    "in al,dx" \
    parm [edx] value [ax] modify exact [eax];
void out_port(u32 port, u32 value);
#pragma aux out_port = \
    "out dx,al" \
    parm [edx] [eax] modify exact [];
void out_port_b(u32 port, u8 value);
#pragma aux out_port_b = \
    "out dx,al" \
    parm [edx] [al] modify exact [];

/* Updated by the keyboard interrupt handler. */
#define INKEY (*(volatile u8 *)&lbInkey)
#define KEYON ((volatile u8 *)lbKeyOn)

void read_key(void);
#pragma aux read_key = \
    "mov ah,11h" \
    "int 16h" \
    "jz skip" \
    "mov ah,10h" \
    "int 16h" \
    "mov lbInkey,ah" \
    "skip:" \
    modify [eax];

// FUNCTION: 0xecab4
void fn_000ecab4(void)
{
    REGS r;

    r.h.ah = 0x12;
    int386(0x16, &r, &r);
    D_00189790 = 0;
    if (r.h.al & 3)
        D_00189790 = 1;
    if (r.h.al & 4)
        D_00189790 |= 2;
    if (r.h.al & 8)
        D_00189790 |= 4;
    lbInkey = 0;
    read_key();
}

}

extern "C" {

// FUNCTION: 0xecb20
void __interrupt __far fn_000ecb20(void)
{
    s32 key;
    s32 old;

    INKEY = in_port(0x60);
    if (D_0018cfb8 == 0xe0) {
        D_00189688 = 1;
        if (INKEY == 0x2a || INKEY == 0xaa) {
            INKEY = 0x80;
            key = 0x80;
        } else {
            key = (u8)(INKEY | 0x80);
            KEYON[key] = INKEY <= 0x7f;
        }
    } else {
        D_00189688 = 0;
        key = (u8)(INKEY & 0x7f);
        KEYON[key] = INKEY <= 0x7f;
    }
    D_0018cfb8 = INKEY;
    D_0018978d = 0;
    if (KEYON[0x2a] || KEYON[0x36])
        D_0018978d |= 0x10;
    if (KEYON[0x1d] || KEYON[0x9d])
        D_0018978d |= 0x20;
    if (KEYON[0x38] || KEYON[0xb8])
        D_0018978d |= 0x40;
    if (KEYON[key])
        KEYON[key] |= D_0018978d;
    key = in_port(0x61);
    old = key;
    key |= 0x80;
    out_port(0x61, key);
    out_port(0x61, old);
    if (INKEY < 0x80 && !D_0018978e) {
        D_0018978e = INKEY;
        D_0018978f = D_0018978d;
    }
    out_port_b(0x20, 0x20);
}

}
