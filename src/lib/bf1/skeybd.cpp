#include "bflib.h"

extern "C" {

extern u8 lbShift;
extern u8 lbExtendedKeyPress;
extern volatile u8 lbInkeyFlags;
extern u8 lbIInkey;
extern u8 lbIInkeyFlags;
extern u8 KInt_oinkey;

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

}

void __interrupt __far KInt(void);
extern void (__interrupt __far *OldInt)(void);

extern "C" {

void (__interrupt __far *_dos_getvect(unsigned num))();
void _dos_setvect(unsigned num, void (__interrupt __far *handler)());
void LbDosKeyboard(void);

// Returns 1 if the library's keyboard interrupt handler is installed, else
// 0.
// FUNCTION: 0xec9a0
int LbIsIKeyboardInstalled(void)
{
    if (OldInt)
        return 1;
    return 0;
}

// Does nothing. Returns 1.
// FUNCTION: 0xec9c4
int LbKeyboardOpen(void)
{
    return 1;
}

// Does nothing. Returns 1.
// FUNCTION: 0xec9d0
int LbKeyboardClose(void)
{
    return 1;
}

// Polls the BIOS keyboard (LbDosKeyboard) and returns the ASCII code of the
// key pressed, or 0.
// FUNCTION: 0xec9dc
int LbKeyboard(void)
{
    LbDosKeyboard();
    if (INKEY < 0x80)
        return (u8)lbInkeyToAscii[INKEY];
    return 0;
}

// Installs the keyboard interrupt handler (KInt) on IRQ 1, clearing
// lbKeyOn. Returns 1.
// FUNCTION: 0xeca0c
int LbIKeyboardOpen(void)
{
    if (!OldInt) {
        memset(lbKeyOn, 0, 0x100);
        OldInt = _dos_getvect(9);
        _dos_setvect(9, KInt);
    }
    return 1;
}

// Restores the previous keyboard interrupt handler. Returns 1.
// FUNCTION: 0xeca68
int LbIKeyboardClose(void)
{
    if (OldInt) {
        _dos_setvect(9, OldInt);
        OldInt = 0;
    }
    return 1;
}

// Polls the BIOS: sets lbShift (1 shift, 2 ctrl, 4 alt) and lbInkey to the
// scan code of a waiting key, or 0.
// FUNCTION: 0xecab4
void LbDosKeyboard(void)
{
    REGS r;

    r.h.ah = 0x12;
    int386(0x16, &r, &r);
    lbShift = 0;
    if (r.h.al & 3)
        lbShift = 1;
    if (r.h.al & 4)
        lbShift |= 2;
    if (r.h.al & 8)
        lbShift |= 4;
    lbInkey = 0;
    read_key();
}

}

// Keyboard interrupt handler: reads the scan code into lbInkey, updates
// lbKeyOn (0xe0-prefixed keys at code | 0x80, held keys tagged with the
// shift 0x10, ctrl 0x20 and alt 0x40 bits), and latches the first new
// press in lbIInkey and lbIInkeyFlags.
// FUNCTION: 0xecb20
void __interrupt __far KInt(void)
{
    s32 key;
    s32 old;

    INKEY = in_port(0x60);
    if (KInt_oinkey == 0xe0) {
        lbExtendedKeyPress = 1;
        if (INKEY == 0x2a || INKEY == 0xaa) {
            INKEY = 0x80;
            key = 0x80;
        } else {
            key = (u8)(INKEY | 0x80);
            KEYON[key] = INKEY <= 0x7f;
        }
    } else {
        lbExtendedKeyPress = 0;
        key = (u8)(INKEY & 0x7f);
        KEYON[key] = INKEY <= 0x7f;
    }
    KInt_oinkey = INKEY;
    lbInkeyFlags = 0;
    if (KEYON[0x2a] || KEYON[0x36])
        lbInkeyFlags |= 0x10;
    if (KEYON[0x1d] || KEYON[0x9d])
        lbInkeyFlags |= 0x20;
    if (KEYON[0x38] || KEYON[0xb8])
        lbInkeyFlags |= 0x40;
    if (KEYON[key])
        KEYON[key] |= lbInkeyFlags;
    key = in_port(0x61);
    old = key;
    key |= 0x80;
    out_port(0x61, key);
    out_port(0x61, old);
    if (INKEY < 0x80 && !lbIInkey) {
        lbIInkey = INKEY;
        lbIInkeyFlags = lbInkeyFlags;
    }
    out_port_b(0x20, 0x20);
}
