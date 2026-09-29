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


extern "C" {
extern Event D_001549d0[];
extern char D_001205f0[];
extern char D_001205f3[];
s32 fn_00085660(s32 n);
void fn_0001bab4(u8 slot);
}

void process_nettext_keys(u8 shift, u8 key);
void make_a_rat(u8 owner);

// FUNCTION: 0x0007424c
void fn_0007424c(u8 shift, u8 key)
{
    if (gamevar.f12e == 0x25) {
        if (key == 0x39 || key == 1)
            finish_composite_help();
        lbInkey = 0;
        return;
    }
    if (statvar.unknownfd <= 1) {
        if ((lbKeyOn[0x11] || lbKeyOn[0x91]) && !(shift & 2))
            othervar.f8b0 = 1;
    }
    if (othervar.f4a) {
        process_nettext_keys(shift, key);
        lbInkey = 0;
        return;
    }
    if (lbKeyOn[0x48] || lbKeyOn[0xc8])
        D_001549d0[player].flags |= 1;
    if (lbKeyOn[0x4b] || lbKeyOn[0xcb])
        D_001549d0[player].flags |= 4;
    if (lbKeyOn[0x50] || lbKeyOn[0xd0])
        D_001549d0[player].flags |= 2;
    if (lbKeyOn[0x4d] || lbKeyOn[0xcd])
        D_001549d0[player].flags |= 8;
    if ((D_001549d0[player].flags & 1) && (D_001549d0[player].flags & 2))
        D_001549d0[player].flags &= ~3;
    if ((D_001549d0[player].flags & 4) && (D_001549d0[player].flags & 8))
        D_001549d0[player].flags &= ~0xc;
    switch (key) {
    case 2:
        if (statvar.unknownfd > 1 && (shift & 4)) {
            if (display_mode == 0) {
                if (statvar.unknownfd >= 2) {
                    othervar.f4c = 1 << fn_00085660(1);
                    othervar.f4a = 1;
                    net_text[player].unknown0 = 0;
                    D_001549d0[player].type = 0x4e;
                }
            }
        } else if (!(shift & 1) && gamevar.f174 == 0) {
            D_001549d0[player].type = 0xe;
            D_001549d0[player].value = 10;
            lbInkey = 0;
        } else if (shift & 1) {
            if (othervar.fa8e) {
                D_001549d0[player].type = 0x17;
                D_001549d0[player].value = othervar.f28;
                D_001549d0[player].arg = (u16)othervar.ff;
                lbInkey = 0;
            }
        }
        break;
    case 3:
        if (statvar.unknownfd > 1 && (shift & 4)) {
            if (display_mode == 0) {
                if (statvar.unknownfd >= 3) {
                    othervar.f4c = 1 << fn_00085660(2);
                    othervar.f4a = 1;
                    net_text[player].unknown0 = 0;
                    D_001549d0[player].type = 0x4e;
                }
            }
        } else if (gamevar.f174 == 0) {
            D_001549d0[player].type = 0xe;
            D_001549d0[player].value = 0x14;
        }
        lbInkey = 0;
        break;
    case 4:
        if (statvar.unknownfd > 1 && (shift & 4)) {
            if (display_mode == 0) {
                if (statvar.unknownfd >= 4) {
                    othervar.f4c = 1 << fn_00085660(3);
                    othervar.f4a = 1;
                    net_text[player].unknown0 = 0;
                    D_001549d0[player].type = 0x4e;
                }
            }
        } else {
            D_001549d0[player].type = 0xe;
            D_001549d0[player].value = 0x1e;
        }
        lbInkey = 0;
        break;
    case 5:
        if (statvar.unknownfd > 1 && (shift & 4)) {
            if (display_mode == 0) {
                if (statvar.unknownfd <= 1) {
                    othervar.f4c = 0xff;
                } else {
                    othervar.f4c = 1 << player;
                    othervar.f4c = ~othervar.f4c;
                }
                othervar.f4a = 1;
                net_text[player].unknown0 = 0;
                D_001549d0[player].type = 0x4e;
            }
            break;
        } else {
            D_001549d0[player].type = 0xe;
            D_001549d0[player].value = 0x28;
            lbInkey = 0;
        }
        break;
    case 6:
        D_001549d0[player].type = 0xe;
        D_001549d0[player].value = 0x32;
        lbInkey = 0;
        break;
    case 0x10:
    case 0x2d:
        if (((shift & 1) || (shift & 4)) || (shift & 2)) {
            sprintf(othervar.f935, D_001205f0, yesno_text[0]);
            if (statvar.unknownfd > 1)
                open_yes_no_requester(0x7e, 1, 0, 0x78, 1, 0);
            else
                open_yes_no_requester(0x1c, 0, 0, 0x78, 1, 0);
            lbInkey = 0;
        }
        break;
    case 1:
        if ((shift & 1) && ((shift & 2) || lbKeyOn[0xf])) {
            if (statvar.unknownfd > 1)
                D_001549d0[player].type = 0x7e;
            else
                D_001549d0[player].type = 0x1c;
            D_001549d0[player].value = 0;
            D_001549d0[player].arg = 0;
        } else if (shift & 1) {
            sprintf(othervar.f935, D_001205f3, yesno_text[0]);
            if (statvar.unknownfd > 1)
                open_yes_no_requester(0x7e, 1, 0, 0x78, 1, 0);
            else
                open_yes_no_requester(0x1c, 0, 0, 0x78, 1, 0);
        } else {
            setup_abort_all_requesters();
        }
        lbInkey = 0;
        break;
    case 0x19:
        D_001549d0[player].type = 0x22;
        lbInkey = 0;
        break;
    case 0x14:
        display_mode = 4;
        town.unknown2 = 0;
        break;
    case 0x24:
        if (!gamevar.unknown21) {
            if (!(shift & 1)) {
                if (resvars.f1 == SCREEN_MODE_13)
                    D_001549d0[player].type = 0x49;
                lbInkey = 0;
            }
        }
        break;
    case 0x3e:
        if (!gamevar.unknown21) {
            if (shift & 4) {
                play_sound(10, 0xda, -1, -1);
                fn_0001bab4(key - 0x3b);
            } else if (shift & 2) {
                recall_mappos(key - 0x3b);
            } else if (listhead[player].rooms[0x1c] == 0) {
                play_sound(10, 8, -1, -1);
            } else {
                D_001549d0[player].type = 0x33;
                D_001549d0[player].value = (u16)key - 0x3b;
            }
            lbInkey = 0;
        }
        break;
    case 0x3b:
    case 0x3c:
    case 0x3d:
    case 0x3f:
    case 0x40:
    case 0x41:
    case 0x42:
        if (!gamevar.unknown21 && gamevar.f174 == 0) {
            if (shift & 4) {
                play_sound(10, 0xda, -1, -1);
                fn_0001bab4(key - 0x3b);
            } else if (shift & 2) {
                recall_mappos(key - 0x3b);
            } else {
                D_001549d0[player].type = 0x33;
                D_001549d0[player].value = (u16)key - 0x3b;
            }
            lbInkey = 0;
        }
        break;
    case 0x1e:
        if (shift & 1) {
            myplayer.f41[20] = myplayer.f41[20] == 1 ? 0 : 1;
            LbMemorySet(gamevar.unknown7b, 0, 0x70);
            lbInkey = 0;
        }
        break;
    case 0x2e:
        if (shift & 4) {
            debug.unknownd4 = 1;
            lbInkey = 0;
        } else if (shift & 2) {
            if (shift & 1)
                cheat(4);
            else
                cheat(2);
        } else if (shift & 1) {
            cheat(8);
        }
        break;
    case 0x1f:
        if (shift & 4) {
            D_001549d0[player].type = 0xc;
            D_001549d0[player].value = (u16)debug.unknown10a;
            lbInkey = 0;
        }
        break;
    case 0x26:
        if (shift & 4) {
            D_001549d0[player].type = 0xd;
            D_001549d0[player].value = (u16)debug.unknown10a;
            lbInkey = 0;
        }
        break;
    case 0x20:
        if ((shift & 4) && (shift & 2)) {
            if (othervar.fa8e) {
                gamevar.f132 = 0;
                gamevar.f134 = 0x3fff;
                lbInkey = 0;
            }
        }
        break;
    case 0x13:
        if (shift & 1) {
            if (othervar.fa8e) {
                make_a_rat(player);
                lbInkey = 0;
            }
        }
        break;
    case 0x15:
        if (shift & 2) {
            if (othervar.fa8e) {
                D_001549d0[player].type = 0x89;
                lbInkey = 0;
            }
        }
        break;
    case 0x32:
        if (shift & 2) {
            if (othervar.fa8e) {
                D_001549d0[player].type = 0x65;
                lbInkey = 0;
            }
        }
        break;
    }
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

void __interrupt __far myKInt(void);

// FUNCTION: 0x00074d60
s32 SWIKeyboardOpen(void)
{
    memset(lbKeyOn, 0, 0x100);
    myOldInt = _dos_getvect(9);
    _dos_setvect(9, myKInt);
    return 1;
}

// FUNCTION: 0x00074dc8
s32 SWIKeyboardClose(void)
{
    _dos_setvect(9, myOldInt);
    return 1;
}

// FUNCTION: 0x00074e08
// NONMATCHING: stack slots of the locals differ; one extra zero-extension
void __interrupt __far myKInt(void)
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
