#include "hosp.h"

extern "C" {
extern u8 D_0016041c;
extern u8 D_00189688;
extern u8 D_0018978d;
extern u8 D_0018978e;
extern u8 D_0018978f;
void fn_0007424c(u8 modifiers, u8 key);
unsigned fn_000cd1bf(unsigned port);
unsigned fn_000cd1c7(unsigned port, unsigned value);
void (__interrupt __far *_dos_getvect(unsigned num))();
void _dos_setvect(unsigned num, void (__interrupt __far *handler)());
}

extern volatile long keybuffer[];
extern volatile long keybufferin;
extern volatile long keybufferout;
extern void (__interrupt __far *myOldInt)();

// FUNCTION: 0x00074220
void fn_00074220(u8 modifiers, u8 key)
{
    fn_0007424c(modifiers, key);
}


extern "C" {
extern Event D_001549d0[];
s32 fn_00085660(s32 n);
void fn_0001bab4(u8 slot);
}

void process_nettext_keys(u8 shift, u8 key);
void make_a_rat(u8 owner);

// FUNCTION: 0x0007424c
void fn_0007424c(u8 modifiers, u8 key)
{
    if (gamevar.f12e == MM_HELP) {
        if (key == KC_SPACE || key == KC_ESCAPE)
            finish_composite_help();
        lbInkey = 0;
        return;
    }
    if (statvar.unknownfd <= 1) {
        if ((lbKeyOn[KC_W] || lbKeyOn[0x91]) && !(modifiers & MOD_CTRL))
            othervar.f8b0 = 1;
    }
    if (othervar.f4a) {
        process_nettext_keys(modifiers, key);
        lbInkey = 0;
        return;
    }
    if (lbKeyOn[KC_NUMPAD8] || lbKeyOn[KC_UP])
        D_001549d0[player].flags |= 1;
    if (lbKeyOn[KC_NUMPAD4] || lbKeyOn[KC_LEFT])
        D_001549d0[player].flags |= 4;
    if (lbKeyOn[KC_NUMPAD2] || lbKeyOn[KC_DOWN])
        D_001549d0[player].flags |= 2;
    if (lbKeyOn[KC_NUMPAD6] || lbKeyOn[KC_RIGHT])
        D_001549d0[player].flags |= 8;
    if ((D_001549d0[player].flags & 1) && (D_001549d0[player].flags & 2))
        D_001549d0[player].flags &= ~3;
    if ((D_001549d0[player].flags & 4) && (D_001549d0[player].flags & 8))
        D_001549d0[player].flags &= ~0xc;
    switch (key) {
    case KC_1:
        if (statvar.unknownfd > 1 && (modifiers & MOD_ALT)) {
            if (display_mode == 0) {
                if (statvar.unknownfd >= 2) {
                    othervar.f4c = 1 << fn_00085660(1);
                    othervar.f4a = 1;
                    net_text[player].unknown0 = 0;
                    D_001549d0[player].type = 0x4e;
                }
            }
        } else if (!(modifiers & MOD_SHIFT) && gamevar.f174 == 0) {
            D_001549d0[player].type = EV_VARY_SPEED;
            D_001549d0[player].value = 10;
            lbInkey = 0;
        } else if (modifiers & MOD_SHIFT) {
            if (othervar.fa8e) {
                D_001549d0[player].type = 0x17;
                D_001549d0[player].value = othervar.f28;
                D_001549d0[player].arg = (u16)othervar.ff;
                lbInkey = 0;
            }
        }
        break;
    case KC_2:
        if (statvar.unknownfd > 1 && (modifiers & MOD_ALT)) {
            if (display_mode == 0) {
                if (statvar.unknownfd >= 3) {
                    othervar.f4c = 1 << fn_00085660(2);
                    othervar.f4a = 1;
                    net_text[player].unknown0 = 0;
                    D_001549d0[player].type = 0x4e;
                }
            }
        } else if (gamevar.f174 == 0) {
            D_001549d0[player].type = EV_VARY_SPEED;
            D_001549d0[player].value = 0x14;
        }
        lbInkey = 0;
        break;
    case KC_3:
        if (statvar.unknownfd > 1 && (modifiers & MOD_ALT)) {
            if (display_mode == 0) {
                if (statvar.unknownfd >= 4) {
                    othervar.f4c = 1 << fn_00085660(3);
                    othervar.f4a = 1;
                    net_text[player].unknown0 = 0;
                    D_001549d0[player].type = 0x4e;
                }
            }
        } else {
            D_001549d0[player].type = EV_VARY_SPEED;
            D_001549d0[player].value = 0x1e;
        }
        lbInkey = 0;
        break;
    case KC_4:
        if (statvar.unknownfd > 1 && (modifiers & MOD_ALT)) {
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
            D_001549d0[player].type = EV_VARY_SPEED;
            D_001549d0[player].value = 0x28;
            lbInkey = 0;
        }
        break;
    case KC_5:
        D_001549d0[player].type = EV_VARY_SPEED;
        D_001549d0[player].value = 0x32;
        lbInkey = 0;
        break;
    case KC_Q:
    case KC_X:
        if (((modifiers & MOD_SHIFT) || (modifiers & MOD_ALT)) || (modifiers & MOD_CTRL)) {
            sprintf(othervar.f935, "%s", yesno_text[0]);
            if (statvar.unknownfd > 1)
                open_yes_no_requester(0x7e, 1, 0, 0x78, 1, 0);
            else
                open_yes_no_requester(0x1c, 0, 0, 0x78, 1, 0);
            lbInkey = 0;
        }
        break;
    case KC_ESCAPE:
        if ((modifiers & MOD_SHIFT) && ((modifiers & MOD_CTRL) || lbKeyOn[KC_TAB])) {
            if (statvar.unknownfd > 1)
                D_001549d0[player].type = 0x7e;
            else
                D_001549d0[player].type = 0x1c;
            D_001549d0[player].value = 0;
            D_001549d0[player].arg = 0;
        } else if (modifiers & MOD_SHIFT) {
            sprintf(othervar.f935, "%s", yesno_text[0]);
            if (statvar.unknownfd > 1)
                open_yes_no_requester(0x7e, 1, 0, 0x78, 1, 0);
            else
                open_yes_no_requester(0x1c, 0, 0, 0x78, 1, 0);
        } else {
            setup_abort_all_requesters();
        }
        lbInkey = 0;
        break;
    case KC_P:
        D_001549d0[player].type = EV_PAUSE;
        lbInkey = 0;
        break;
    case KC_T:
        display_mode = 4;
        town.unknown2 = 0;
        break;
    case KC_J:
        if (!gamevar.unknown21) {
            if (!(modifiers & MOD_SHIFT)) {
                if (resvars.f1 == SCREEN_MODE_13)
                    D_001549d0[player].type = 0x49;
                lbInkey = 0;
            }
        }
        break;
    case KC_F4:
        if (!gamevar.unknown21) {
            if (modifiers & MOD_ALT) {
                play_sound(10, SFX_SNAPSHOT, -1, -1);
                fn_0001bab4(key - KC_F1);
            } else if (modifiers & MOD_CTRL) {
                recall_mappos(key - KC_F1);
            } else if (listhead[player].rooms[ROOM_RESEARCH] == 0) {
                play_sound(10, SFX_WRONG2, -1, -1);
            } else {
                D_001549d0[player].type = EV_OPEN_SCREEN;
                D_001549d0[player].value = (u16)key - KC_F1;
            }
            lbInkey = 0;
        }
        break;
    case KC_F1:
    case KC_F2:
    case KC_F3:
    case KC_F5:
    case KC_F6:
    case KC_F7:
    case KC_F8:
        if (!gamevar.unknown21 && gamevar.f174 == 0) {
            if (modifiers & MOD_ALT) {
                play_sound(10, SFX_SNAPSHOT, -1, -1);
                fn_0001bab4(key - KC_F1);
            } else if (modifiers & MOD_CTRL) {
                recall_mappos(key - KC_F1);
            } else {
                D_001549d0[player].type = EV_OPEN_SCREEN;
                D_001549d0[player].value = (u16)key - KC_F1;
            }
            lbInkey = 0;
        }
        break;
    case KC_A:
        if (modifiers & MOD_SHIFT) {
            myplayer.f41[20] = myplayer.f41[20] == 1 ? 0 : 1;
            LbMemorySet(gamevar.unknown7b, 0, 0x70);
            lbInkey = 0;
        }
        break;
    case KC_C:
        if (modifiers & MOD_ALT) {
            debug.unknownd4 = 1;
            lbInkey = 0;
        } else if (modifiers & MOD_CTRL) {
            if (modifiers & MOD_SHIFT)
                cheat(4);
            else
                cheat(2);
        } else if (modifiers & MOD_SHIFT) {
            cheat(8);
        }
        break;
    case KC_S:
        if (modifiers & MOD_ALT) {
            D_001549d0[player].type = 0xc;
            D_001549d0[player].value = (u16)debug.unknown10a;
            lbInkey = 0;
        }
        break;
    case KC_L:
        if (modifiers & MOD_ALT) {
            D_001549d0[player].type = 0xd;
            D_001549d0[player].value = (u16)debug.unknown10a;
            lbInkey = 0;
        }
        break;
    case KC_D:
        if ((modifiers & MOD_ALT) && (modifiers & MOD_CTRL)) {
            if (othervar.fa8e) {
                gamevar.f132 = 0;
                gamevar.f134 = 0x3fff;
                lbInkey = 0;
            }
        }
        break;
    case KC_R:
        if (modifiers & MOD_SHIFT) {
            if (othervar.fa8e) {
                make_a_rat(player);
                lbInkey = 0;
            }
        }
        break;
    case KC_Y:
        if (modifiers & MOD_CTRL) {
            if (othervar.fa8e) {
                D_001549d0[player].type = 0x89;
                lbInkey = 0;
            }
        }
        break;
    case KC_M:
        if (modifiers & MOD_CTRL) {
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
void __interrupt __far myKInt(void)
{
    u32 key;
    s32 port61;

    lbInkey = fn_000cd1bf(0x60);
    if (D_0016041c == 0xe0) {
        D_00189688 = 1;
        if (lbInkey == KC_LSHIFT || lbInkey == 0xaa) {
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
            s32 next;

            keybuffer[keybufferin] = key;
            next = (keybufferin + 1) & 0xf;
            if (next != keybufferout)
                keybufferin = next;
        }
    }
    D_0016041c = lbInkey;
    D_0018978d = 0;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
        D_0018978d |= 0x10;
    if (lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL])
        D_0018978d |= 0x20;
    if (lbKeyOn[KC_LALT] || lbKeyOn[KC_RALT])
        D_0018978d |= 0x40;
    if (lbKeyOn[key])
        lbKeyOn[key] |= D_0018978d;
    key = (s16)fn_000cd1bf(0x61);
    port61 = key;
    key |= 0x80;
    fn_000cd1c7(0x61, key);
    fn_000cd1c7(0x61, port61);
    if (!(lbInkey >= 0x80 || D_0018978e)) {
        D_0018978e = lbInkey;
        D_0018978f = D_0018978d;
    }
    fn_000cd1c7(0x20, 0x20);
}
