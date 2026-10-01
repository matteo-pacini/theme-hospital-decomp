#include "hosp.h"

extern "C" {
extern u8 oinkey;
extern u8 D_00189688;
extern u8 lbInkeyFlags;
extern u8 lbIInkey;
extern u8 lbIInkeyFlags;
void process_game_keys(u8 modifiers, u8 key);
unsigned fn_000cd1bf(unsigned port);
unsigned fn_000cd1c7(unsigned port, unsigned value);
void (__interrupt __far *_dos_getvect(unsigned num))();
void _dos_setvect(unsigned num, void (__interrupt __far *handler)());
}

extern volatile long keybuffer[];
extern volatile long keybufferin;
extern volatile long keybufferout;
extern void (__interrupt __far *myOldInt)();

// Handles key key with modifiers (MOD_* bits) in the game view; see
// process_game_keys.
// FUNCTION: 0x00074220
void process_keys(u8 modifiers, u8 key)
{
    process_game_keys(modifiers, key);
}


extern "C" {
extern Event D_001549d0[];
s32 nth_other_active_player(s32 nth);
void record_mappos(u8 key);
}

void process_nettext_keys(u8 shift, u8 key);
void make_a_rat(u8 owner);

// Handles key key with modifiers (MOD_* bits) in the game view, mostly by
// queueing turn events: the arrow keys scroll, 1-5 set the game speed (Alt+1-4
// start a network chat message), P pauses, F1-F8 open the full-screen windows
// (Alt and Ctrl store and recall map positions), Escape closes requesters,
// Shift+Escape or a modifier with Q or X asks to quit (Shift+Ctrl+Escape quits
// at once), Shift+A toggles the advisor, plus debug and cheat keys. In help
// mode Space or Escape close the help page; while chatting, keys go to the
// message.
// FUNCTION: 0x0007424c
void process_game_keys(u8 modifiers, u8 key)
{
    if (gamevar.mouse_mode == MM_HELP) {
        if (key == KC_SPACE || key == KC_ESCAPE)
            finish_composite_help();
        lbInkey = 0;
        return;
    }
    if (statvar.num_players <= 1) {
        if ((lbKeyOn[KC_W] || lbKeyOn[0x91]) && !(modifiers & MOD_CTRL))
            othervar.turbo_key = 1;
    }
    if (othervar.net_text_entering) {
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
        if (statvar.num_players > 1 && (modifiers & MOD_ALT)) {
            if (display_mode == DM_GAME) {
                if (statvar.num_players >= 2) {
                    othervar.net_text_to = 1 << nth_other_active_player(1);
                    othervar.net_text_entering = 1;
                    net_text[player].text = 0;
                    D_001549d0[player].type = EV_NET_TEXT_START;
                }
            }
        } else if (!(modifiers & MOD_SHIFT) && gamevar.rat_fest_level == 0) {
            D_001549d0[player].type = EV_VARY_SPEED;
            D_001549d0[player].value = 10;
            lbInkey = 0;
        } else if (modifiers & MOD_SHIFT) {
            if (othervar.cheats_enabled) {
                D_001549d0[player].type = 0x17;
                D_001549d0[player].value = othervar.mouse_cell;
                D_001549d0[player].arg = (u16)othervar.cur_player;
                lbInkey = 0;
            }
        }
        break;
    case KC_2:
        if (statvar.num_players > 1 && (modifiers & MOD_ALT)) {
            if (display_mode == DM_GAME) {
                if (statvar.num_players >= 3) {
                    othervar.net_text_to = 1 << nth_other_active_player(2);
                    othervar.net_text_entering = 1;
                    net_text[player].text = 0;
                    D_001549d0[player].type = EV_NET_TEXT_START;
                }
            }
        } else if (gamevar.rat_fest_level == 0) {
            D_001549d0[player].type = EV_VARY_SPEED;
            D_001549d0[player].value = 0x14;
        }
        lbInkey = 0;
        break;
    case KC_3:
        if (statvar.num_players > 1 && (modifiers & MOD_ALT)) {
            if (display_mode == DM_GAME) {
                if (statvar.num_players >= 4) {
                    othervar.net_text_to = 1 << nth_other_active_player(3);
                    othervar.net_text_entering = 1;
                    net_text[player].text = 0;
                    D_001549d0[player].type = EV_NET_TEXT_START;
                }
            }
        } else {
            D_001549d0[player].type = EV_VARY_SPEED;
            D_001549d0[player].value = 0x1e;
        }
        lbInkey = 0;
        break;
    case KC_4:
        if (statvar.num_players > 1 && (modifiers & MOD_ALT)) {
            if (display_mode == DM_GAME) {
                if (statvar.num_players <= 1) {
                    othervar.net_text_to = 0xff;
                } else {
                    othervar.net_text_to = 1 << player;
                    othervar.net_text_to = ~othervar.net_text_to;
                }
                othervar.net_text_entering = 1;
                net_text[player].text = 0;
                D_001549d0[player].type = EV_NET_TEXT_START;
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
            sprintf(othervar.yesno_msg, "%s", yesno_text[0]);
            if (statvar.num_players > 1)
                open_yes_no_requester(EV_LEAVE_NET_GAME, 1, 0, EV_CLOSE_YES_NO, 1, 0);
            else
                open_yes_no_requester(EV_QUIT_GAME, 0, 0, EV_CLOSE_YES_NO, 1, 0);
            lbInkey = 0;
        }
        break;
    case KC_ESCAPE:
        if ((modifiers & MOD_SHIFT) && ((modifiers & MOD_CTRL) || lbKeyOn[KC_TAB])) {
            if (statvar.num_players > 1)
                D_001549d0[player].type = EV_LEAVE_NET_GAME;
            else
                D_001549d0[player].type = EV_QUIT_GAME;
            D_001549d0[player].value = 0;
            D_001549d0[player].arg = 0;
        } else if (modifiers & MOD_SHIFT) {
            sprintf(othervar.yesno_msg, "%s", yesno_text[0]);
            if (statvar.num_players > 1)
                open_yes_no_requester(EV_LEAVE_NET_GAME, 1, 0, EV_CLOSE_YES_NO, 1, 0);
            else
                open_yes_no_requester(EV_QUIT_GAME, 0, 0, EV_CLOSE_YES_NO, 1, 0);
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
        display_mode = DM_TOWN_MAP;
        town.state = 0;
        break;
    case KC_J:
        if (!gamevar.paused) {
            if (!(modifiers & MOD_SHIFT)) {
                if (resvars.game_mode == SCREEN_MODE_13)
                    D_001549d0[player].type = EV_OPEN_JUKEBOX;
                lbInkey = 0;
            }
        }
        break;
    case KC_F4:
        if (!gamevar.paused) {
            if (modifiers & MOD_ALT) {
                play_sound(SNDSRC_GENERAL, SFX_SNAPSHOT, -1, -1);
                record_mappos(key - KC_F1);
            } else if (modifiers & MOD_CTRL) {
                recall_mappos(key - KC_F1);
            } else if (listhead[player].rooms[ROOM_RESEARCH] == 0) {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
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
        if (!gamevar.paused && gamevar.rat_fest_level == 0) {
            if (modifiers & MOD_ALT) {
                play_sound(SNDSRC_GENERAL, SFX_SNAPSHOT, -1, -1);
                record_mappos(key - KC_F1);
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
            myplayer.advisor_flags[20] = myplayer.advisor_flags[20] == 1 ? 0 : 1;
            LbMemorySet(gamevar.announce, 0, 0x70);
            lbInkey = 0;
        }
        break;
    case KC_C:
        if (modifiers & MOD_ALT) {
            debug.screenshot_request = 1;
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
            D_001549d0[player].type = EV_SAVE_GAME;
            D_001549d0[player].value = (u16)debug.save_slot;
            lbInkey = 0;
        }
        break;
    case KC_L:
        if (modifiers & MOD_ALT) {
            D_001549d0[player].type = EV_LOAD_GAME;
            D_001549d0[player].value = (u16)debug.save_slot;
            lbInkey = 0;
        }
        break;
    case KC_D:
        if ((modifiers & MOD_ALT) && (modifiers & MOD_CTRL)) {
            if (othervar.cheats_enabled) {
                gamevar.scroll_min_tile = 0;
                gamevar.scroll_max_tile = 0x3fff;
                lbInkey = 0;
            }
        }
        break;
    case KC_R:
        if (modifiers & MOD_SHIFT) {
            if (othervar.cheats_enabled) {
                make_a_rat(player);
                lbInkey = 0;
            }
        }
        break;
    case KC_Y:
        if (modifiers & MOD_CTRL) {
            if (othervar.cheats_enabled) {
                D_001549d0[player].type = EV_CHEAT_END_YEAR;
                lbInkey = 0;
            }
        }
        break;
    case KC_M:
        if (modifiers & MOD_CTRL) {
            if (othervar.cheats_enabled) {
                D_001549d0[player].type = EV_CHEAT_END_MONTH;
                lbInkey = 0;
            }
        }
        break;
    }
}
// Empties the keyboard buffer.
// FUNCTION: 0x00074cd8
extern "C" void keybuffer_clear(void)
{
    keybufferin = keybufferout = 0;
}

// Returns the next scan code from the keyboard buffer, or 0 if it is empty.
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

// Installs the game's keyboard interrupt handler (myKInt), clearing the key
// states. Returns 1.
// FUNCTION: 0x00074d60
s32 SWIKeyboardOpen(void)
{
    memset(lbKeyOn, 0, 0x100);
    myOldInt = _dos_getvect(9);
    _dos_setvect(9, myKInt);
    return 1;
}

// Restores the keyboard interrupt handler replaced by SWIKeyboardOpen.
// Returns 1.
// FUNCTION: 0x00074dc8
s32 SWIKeyboardClose(void)
{
    _dos_setvect(9, myOldInt);
    return 1;
}

// Keyboard interrupt handler: reads the scan code, updates lbKeyOn (0xe0
// prefixed keys at code + 0x80, with the modifier flags), queues ordinary key
// presses in the 16-entry key buffer, records the key in lbInkey and lbIInkey,
// and acknowledges the keyboard and interrupt controller.
// FUNCTION: 0x00074e08
void __interrupt __far myKInt(void)
{
    u32 key;
    s32 port61;

    lbInkey = fn_000cd1bf(0x60);
    if (oinkey == 0xe0) {
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
    oinkey = lbInkey;
    lbInkeyFlags = 0;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
        lbInkeyFlags |= 0x10;
    if (lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL])
        lbInkeyFlags |= 0x20;
    if (lbKeyOn[KC_LALT] || lbKeyOn[KC_RALT])
        lbInkeyFlags |= 0x40;
    if (lbKeyOn[key])
        lbKeyOn[key] |= lbInkeyFlags;
    key = (s16)fn_000cd1bf(0x61);
    port61 = key;
    key |= 0x80;
    fn_000cd1c7(0x61, key);
    fn_000cd1c7(0x61, port61);
    if (!(lbInkey >= 0x80 || lbIInkey)) {
        lbIInkey = lbInkey;
        lbIInkeyFlags = lbInkeyFlags;
    }
    fn_000cd1c7(0x20, 0x20);
}
