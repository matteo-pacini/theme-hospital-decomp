#include "hosp.h"

extern "C" {
extern u8 D_0016041c;
extern u8 D_00189688;
extern u8 D_0018978d;
extern u8 D_0018978e;
extern u8 D_0018978f;
void fn_0007424c(u8 shift, u8 key);
u8 fn_000cd1bf(u16 port);
void fn_000cd1c7(u16 port, u8 value);
void (__interrupt __far *_dos_getvect(unsigned num))();
void _dos_setvect(unsigned num, void (__interrupt __far *handler)());
}

extern volatile long keybuffer[];
extern volatile long keybufferin;
extern volatile long keybufferout;
extern void (__interrupt __far *myOldInt)();

// FUNCTION: 0x00074220
void fn_00074220(u8 a, u8 b)
{
    fn_0007424c(a, b);
}

// FUNCTION: 0x00074cd8
extern "C" void fn_00074cd8(void)
{
    keybufferin = keybufferout = 0;
}

// FUNCTION: 0x00074d04
long keybuffer_get(void)
{
    long key;

    if (keybufferin == keybufferout)
        return 0;
    key = keybuffer[keybufferout];
    keybufferout++;
    keybufferout &= 0xf;
    return key;
}

void __interrupt __far myKeyboardInt(void);

// NONMATCHING: handler relocation (the target names it SWIKeyboardClose+0x40)
// FUNCTION: 0x00074d60
s32 SWIKeyboardOpen(void)
{
    memset(lbKeyOn, 0, 0x100);
    myOldInt = _dos_getvect(9);
    _dos_setvect(9, myKeyboardInt);
    return 1;
}

// NONMATCHING: the target symbol also covers myKeyboardInt, which follows it
// FUNCTION: 0x00074dc8
s32 SWIKeyboardClose(void)
{
    _dos_setvect(9, myOldInt);
    return 1;
}

void __interrupt __far myKeyboardInt(void)
{
    u32 key;
    s32 next;
    s32 ctrl;

    lbInkey = fn_000cd1bf(0x60);
    if (D_0016041c == 0xe0) {
        D_00189688 = 1;
        if (lbInkey == 0x2a || lbInkey == 0xaa) {
            lbInkey = 0x80;
            key = 0x80;
        } else {
            key = (u16)(lbInkey | 0x80);
            lbKeyOn[key] = lbInkey <= 0x7f ? (u8)1 : (u8)0;
        }
    } else {
        D_00189688 = 0;
        key = (u16)(lbInkey & 0x7f);
        lbKeyOn[key] = lbInkey <= 0x7f ? (u8)1 : (u8)0;
        if (lbKeyOn[key]) {
            keybuffer[keybufferin] = key;
            next = (keybufferin + 1) & 0xf;
            if (next != keybufferout)
                keybufferin = next;
        }
    }
    D_0016041c = lbInkey;
    D_0018978d = 0;
    if (lbKeyOn[0x2a] || lbKeyOn[0x36])
        D_0018978d |= 0x10;
    if (lbKeyOn[0x1d] || lbKeyOn[0x9d])
        D_0018978d |= 0x20;
    if (lbKeyOn[0x38] || lbKeyOn[0xb8])
        D_0018978d |= 0x40;
    if (lbKeyOn[key])
        lbKeyOn[key] |= D_0018978d;
    key = (s16)fn_000cd1bf(0x61);
    ctrl = key;
    key |= 0x80;
    fn_000cd1c7(0x61, key);
    fn_000cd1c7(0x61, ctrl);
    if (!(lbInkey >= 0x80 || D_0018978e)) {
        D_0018978e = lbInkey;
        D_0018978f = D_0018978d;
    }
    fn_000cd1c7(0x20, 0x20);
}
