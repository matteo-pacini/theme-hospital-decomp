#include "hosp.h"

extern "C" {
extern Event D_001549d0[];
extern Event D_00154a0c[];
extern u8 tutorial_display_flags;
extern u8 D_00161201;
extern u8 D_00161202;
extern u8 D_00161203;
extern u8 D_00161204;
extern u8 D_00161205;
extern u8 D_00161206;
extern u8 D_00161207;
extern u8 D_00161208;
extern u8 D_00161209;
extern u8 D_0016120a;
extern u8 D_0016120b;
extern u8 D_0016120c;
extern u8 D_0016120d;
extern u8 D_0016120e;
extern u8 D_0016120f;
extern u8 D_00161210;
extern u8 D_00161211;
extern u8 D_00161212;
extern u8 D_00161213;
extern u8 D_00161214;
extern u8 D_00161215;
extern u8 D_00161216;
extern u8 D_00161217;
extern u8 D_00161218;
extern u8 D_00161219;
extern u8 D_0016121a;
extern u8 D_0016121b;
extern u8 D_0016121c;
extern u8 D_0016121d;
}

void set_force_help(u8 help_group, u16 id);
void clear_force_help(void);

// Runs the tutorial's current step (advisor.tutorial_step): shows that step's advisor
// text and points at the control to use, and moves to the next step when the
// player's event for it arrives (or back when they cancel). Returns 1 when a
// message was just shown, 0 when a hint is forced in 320x200 (where the
// message is dropped), -1 otherwise.
// FUNCTION: 0x000a4830
s8 tutorial(void)
{
    switch (advisor.tutorial_step) {
    case 0x1d:
        if (tutorial_display_flags != 2) {
            if (D_00154a0c[player].type != 0x62) {
                if (tutorial_display_flags == 0) {
                    sprintf(advisor.text, advisor_text[29]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 0x13;
                    tutorial_display_flags = 1;
                    return 1;
                }
            } else {
                tutorial_display_flags = 2;
                advisor.tutorial_step = 0x28;
                advisor.tutorial_highlight = 0;
                clear_force_help();
            }
        }
        break;
    case 0x28:
        if (D_0016120b != 2) {
            if (D_001549d0[player].type == EV_CANCEL_BUY_OBJECTS) {
                tutorial_display_flags = 0;
                D_0016120b = 0;
                advisor.tutorial_step = 0x1d;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type == 0x2b) {
                tutorial_display_flags = 0;
                D_0016120b = 0;
                advisor.tutorial_step = 0x1d;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type != EV_BUY_OBJECT || D_00154a0c[player].value != 0xb) {
                if (D_0016120b == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[40]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 0xa;
                    set_force_help(5, 0x10b);
                    D_0016120b = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                D_0016120b = 2;
                advisor.tutorial_step = 0x2a;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        }
        break;
    case 0x2a:
        if (D_0016120d != 2) {
            if (D_001549d0[player].type == EV_CANCEL_BUY_OBJECTS) {
                tutorial_display_flags = 0;
                D_0016120b = 0;
                D_0016120d = 0;
                advisor.tutorial_step = 0x1d;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if ((D_00154a0c[player].type == EV_SELL_OBJECT && D_00154a0c[player].value == 0xb) && gamevar.buy_list.to_place[0xb] == 0) {
                D_0016120d = 0;
                D_0016120b = 0;
                advisor.tutorial_step = 0x28;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type != 0x2b) {
                if (D_0016120d == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[42]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 9;
                    set_force_help(5, 0x27);
                    D_0016120d = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                D_0016120d = 2;
                advisor.tutorial_step = 0x29;
                advisor.tutorial_highlight = 0;
                clear_force_help();
            }
        }
        break;
    case 0x29:
        if (D_0016120c != 2) {
            if (D_001549d0[player].type == EV_CANCEL_BUILD) {
                D_0016120c = 0;
                D_0016120d = 0;
                D_0016120b = 0;
                tutorial_display_flags = 0;
                advisor.tutorial_step = 0x1d;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (advisor.flags & 2) {
                sprintf(advisor.text, advisor_text[46]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                D_0016120c = 0;
                return 1;
            } else if (!listhead[player].rooms[ROOM_RECEPTION]) {
                if (D_0016120c == 0) {
                    sprintf(advisor.text, advisor_text[41]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    D_0016120c = 1;
                    return 1;
                }
            } else {
                D_0016120c = 2;
                advisor.tutorial_step = 0x1e;
                advisor.tutorial_highlight = 0;
            }
        }
        break;
    case 0x1e:
        if (D_00161201 != 2 && !listhead[player].staff[0]) {
            if (D_00154a0c[player].type != 0x2e) {
                if (D_00161201 == 0) {
                    sprintf(advisor.text, advisor_text[30]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 0x15;
                    D_00161201 = 1;
                    return 1;
                }
            } else {
                D_00161201 = 2;
                advisor.tutorial_step = 0x21;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            D_00161201 = 2;
            advisor.tutorial_step = 0x20;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x21:
        if (D_00161204 != 2) {
            if (D_00154a0c[player].type == 0x38 || (D_00154a0c[player].type == EV_MARK_EMPLOYING && D_00154a0c[player].value != 3)) {
                D_00161201 = 0;
                D_00161204 = 0;
                advisor.tutorial_step = 0x1e;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type == 0x2f && D_00154a0c[player].value != 3) {
                if (resvars.game_mode == SCREEN_MODE_13) {
                    sprintf(advisor.text, advisor_text[33]);
                    advisor.rewrap = 1;
                }
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                advisor.tutorial_highlight = 3;
                set_force_help(4, 0x19);
                return 1;
            } else if (D_00154a0c[player].type != 0x2f || D_00154a0c[player].value != 3) {
                if (D_00161204 == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[33]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 3;
                    set_force_help(4, 0x19);
                    D_00161204 = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                D_00161204 = 2;
                advisor.tutorial_step = 0x25;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            advisor.tutorial_step = 0x25;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x25:
        if (D_00161208 != 2) {
            if (D_00154a0c[player].type == 0x38 || (D_00154a0c[player].type == EV_MARK_EMPLOYING && D_00154a0c[player].value != 3)) {
                D_00161208 = 2;
                D_00161209 = 2;
                D_00161201 = 0;
                D_00161204 = 0;
                advisor.tutorial_step = 0x1e;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type == EV_MARK_EMPLOYING && D_00154a0c[player].value == 3) {
                D_00161208 = 2;
                D_00161209 = 2;
                advisor.tutorial_step = 0x24;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type == 0x2f && D_00154a0c[player].value != 3) {
                D_00161208 = 2;
                D_00161209 = 2;
                D_00161204 = 0;
                advisor.tutorial_step = 0x21;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type != 0x32 || D_00154a0c[player].value != 1) {
                if (D_00161208 == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[37]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 7;
                    set_force_help(4, 0x1b);
                    D_00161208 = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                D_00161208 = 2;
                advisor.tutorial_step = 0x26;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            advisor.tutorial_step = 0x26;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x26:
        if (D_00161209 != 2) {
            if (D_00154a0c[player].type == 0x38 || (D_00154a0c[player].type == EV_MARK_EMPLOYING && D_00154a0c[player].value != 3)) {
                D_00161209 = 2;
                D_00161201 = 0;
                D_00161204 = 0;
                advisor.tutorial_step = 0x1e;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type == EV_MARK_EMPLOYING && D_00154a0c[player].value == 3) {
                D_00161209 = 2;
                advisor.tutorial_step = 0x24;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type == 0x2f && D_00154a0c[player].value != 3) {
                D_00161209 = 2;
                D_00161204 = 0;
                advisor.tutorial_step = 0x21;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type != 0x32 || D_00154a0c[player].value != 0) {
                if (D_00161209 == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[38]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 6;
                    set_force_help(4, 0x1a);
                    D_00161209 = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                D_00161209 = 2;
                advisor.tutorial_step = 0x22;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            advisor.tutorial_step = 0x22;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x22:
        if (D_00161205 != 2) {
            if (D_00154a0c[player].type == 0x38) {
                D_00161205 = 0;
                D_00161201 = 0;
                D_00161204 = 0;
                advisor.tutorial_step = 0x1e;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type == 0x2f && D_00154a0c[player].value != 3) {
                D_00161204 = 0;
                advisor.tutorial_step = 0x21;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type != EV_MARK_EMPLOYING) {
                if (D_00161205 == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[34]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 5;
                    set_force_help(4, 0x1c);
                    D_00161205 = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                D_00161205 = 2;
                advisor.tutorial_step = 0x24;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            advisor.tutorial_step = 0x24;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x24:
        if (D_00161207 != 2) {
            if (D_00154a0c[player].type == 0x38) {
                D_00161207 = 2;
                D_00161201 = 0;
                D_00161204 = 0;
                advisor.tutorial_step = 0x1e;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (!gamevar.staff_place_ok) {
                sprintf(advisor.text, advisor_text[60]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                D_00161207 = 0;
                return 1;
            } else if (!listhead[player].staff[0]) {
                if (D_00161207 == 0) {
                    sprintf(advisor.text, advisor_text[36]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    D_00161207 = 1;
                    advisor.tutorial_highlight = 0;
                    return 1;
                }
            } else {
                D_00161207 = 2;
                advisor.tutorial_step = 0x20;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            advisor.tutorial_step = 0x20;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x20:
        if (D_00161203 != 2 && !listhead[player].rooms[ROOM_GP]) {
            if (D_00154a0c[player].type != 0x37) {
                if (D_00161203 == 0) {
                    sprintf(advisor.text, advisor_text[32]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 0x12;
                    set_force_help(3, 0xb);
                    D_00161203 = 1;
                    return 1;
                }
            } else {
                D_00161203 = 2;
                advisor.tutorial_step = 0x2b;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            D_00161203 = 2;
            advisor.tutorial_step = 0x1f;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x2b:
        if (D_00154a0c[player].type == 0x38) {
            D_00161203 = 0;
            D_0016120e = 0;
            advisor.tutorial_step = 0x20;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (D_0016120e != 2) {
            if (D_00154a0c[player].type != EV_REQ1_RESET_ROOMS || D_00154a0c[player].value != 3) {
                if (D_0016120e == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[43]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 8;
                    set_force_help(2, 1);
                    D_0016120e = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                D_0016120e = 2;
                advisor.tutorial_step = 0x2c;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        }
        break;
    case 0x2c:
        if (D_00154a0c[player].type == 0x26) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            advisor.tutorial_step = 0x20;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (D_0016120f != 2) {
            if (D_00154a0c[player].type == 0x35 && D_00154a0c[player].value != 7) {
                D_00161203 = 0;
                D_0016120e = 0;
                D_0016120f = 0;
                advisor.tutorial_step = 0x20;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type != 0x35) {
                if (D_0016120f == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[44]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 0xb;
                    fn_000bbff0(othervar.help_msg, room_help[7]);
                    set_force_help(2, 0xed);
                    D_0016120e = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                D_0016120f = 2;
                advisor.tutorial_step = 0x39;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        }
        break;
    case 0x39:
        if (D_00154a0c[player].type == 0x38) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            advisor.tutorial_step = 0x20;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (D_00154a0c[player].type != EV_MODIFY_BLUEPRINT) {
            sprintf(advisor.text, advisor_text[47]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            D_0016121c = 0;
            return 1;
        } else {
            D_0016121c = 2;
            advisor.tutorial_step = 0x2f;
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x2f:
        if (D_00154a0c[player].type == 0x38) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            D_00161212 = 0;
            advisor.tutorial_step = 0x20;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if ((advisor.flags & 4) && (advisor.flags & 2)) {
            sprintf(advisor.text, advisor_text[61]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            D_0016121b = 0;
            advisor.flags &= ~4;
            advisor.flags &= ~2;
            advisor.tutorial_highlight = 0;
            clear_force_help();
            return 1;
        } else if (!(advisor.flags & 4) && !(advisor.flags & 2)) {
            sprintf(advisor.text, advisor_text[52]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            advisor.flags &= ~4;
            advisor.forced_advice_id_s = 0;
            advisor.tutorial_highlight = 0;
            clear_force_help();
            return 1;
        } else if (advisor.forced_advice_id_s == 4 || (advisor.flags & 2)) {
            sprintf(advisor.text, advisor_text[51]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            D_00161212 = 0;
            advisor.flags &= ~2;
            advisor.tutorial_highlight = 0;
            clear_force_help();
            return 1;
        } else {
            D_00161212 = 2;
            advisor.tutorial_step = 0x38;
            advisor.tutorial_highlight = 0;
            advisor.flags &= ~2;
            advisor.flags &= ~4;
            clear_force_help();
        }
        break;
    case 0x38:
        if (D_00154a0c[player].type == 0x38) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            D_00161212 = 0;
            D_0016121b = 0;
            D_00161213 = 0;
            D_00161214 = 0;
            D_00161215 = 0;
            D_0016121d = 0;
            advisor.tutorial_step = 0x20;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if ((advisor.flags & 4) && (advisor.flags & 2)) {
            sprintf(advisor.text, advisor_text[61]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            D_0016121b = 0;
            advisor.flags &= ~4;
            advisor.flags &= ~2;
            advisor.tutorial_highlight = 0;
            clear_force_help();
            return 1;
        } else if ((advisor.flags & 4) && !(advisor.flags & 2)) {
            sprintf(advisor.text, advisor_text[52]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            D_0016121b = 0;
            advisor.flags &= ~4;
            advisor.flags &= ~2;
            advisor.tutorial_highlight = 0;
            clear_force_help();
            return 1;
        } else if (advisor.forced_advice_id_s == 4 || (advisor.flags & 2)) {
            sprintf(advisor.text, advisor_text[51]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            D_0016121b = 0;
            advisor.flags &= ~2;
            advisor.forced_advice_id_s = 0;
            advisor.tutorial_highlight = 0;
            clear_force_help();
            return 1;
        } else if (D_00154a0c[player].type != EV_COMMIT_BLUEPRINT) {
            if (D_0016121b == 0) {
                sprintf(advisor.text, advisor_text[56]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                advisor.tutorial_highlight = resvars.requester_buttons[0x52];
                set_force_help(2, 0x86);
                D_0016121b = 1;
                advisor.flags &= ~2;
                return 1;
            }
        } else {
            D_0016121b = 2;
            advisor.tutorial_step = 0x30;
            advisor.tutorial_highlight = 0;
            clear_force_help();
        }
        break;
    case 0x30:
        if (D_00154a0c[player].type == EV_EDIT_BLUEPRINT) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            D_00161212 = 0;
            D_0016121b = 0;
            D_00161213 = 0;
            advisor.tutorial_step = 0x38;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (D_00154a0c[player].type == EV_PLACE_BP_DOOR && othervar.door_invalid[player] == 1) {
            sprintf(advisor.text, advisor_text[53]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            D_00161218 = 1;
            return 1;
        } else if (D_00154a0c[player].type != EV_PLACE_BP_DOOR) {
            if (D_00161213 == 0) {
                sprintf(advisor.text, advisor_text[48]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                D_00161213 = 1;
                return 1;
            }
        } else {
            D_00161213 = 2;
            advisor.tutorial_step = 0x31;
            advisor.tutorial_highlight = 0;
            clear_force_help();
        }
        break;
    case 0x31:
        if (D_00154a0c[player].type == EV_EDIT_BLUEPRINT) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            D_00161212 = 0;
            D_0016121b = 0;
            D_00161213 = 0;
            D_00161214 = 0;
            advisor.tutorial_step = 0x38;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (D_00154a0c[player].type == EV_PLACE_BP_DOOR && othervar.door_invalid[player] == 1) {
            sprintf(advisor.text, advisor_text[54]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            D_00161219 = 1;
            return 1;
        } else if (D_00154a0c[player].type == EV_PLACE_BP_DOOR && othervar.door_invalid[player] == 0) {
            sprintf(advisor.text, advisor_text[49]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            D_00161214 = 1;
            return 1;
        } else if (D_00154a0c[player].type != EV_COMMIT_DOORS) {
            if (D_00161214 == 0) {
                sprintf(advisor.text, advisor_text[49]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                advisor.tutorial_highlight = resvars.requester_buttons[0x52];
                set_force_help(0xd, 0x86);
                D_00161214 = 1;
                return 1;
            }
        } else {
            D_00161213 = 2;
            advisor.tutorial_step = 0x32;
            advisor.tutorial_highlight = 0;
            clear_force_help();
            advisor.flags &= ~2;
        }
        break;
    case 0x32:
        if (D_00154a0c[player].type == EV_EDIT_BLUEPRINT) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            D_00161212 = 0;
            D_0016121b = 0;
            D_00161213 = 0;
            D_00161214 = 0;
            D_00161215 = 0;
            advisor.tutorial_step = 0x38;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (advisor.flags & 2) {
            sprintf(advisor.text, advisor_text[55]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            advisor.flags &= ~2;
            D_00161215 = 0;
            return 1;
        } else if (!(advisor.flags & 0x10)) {
            if (D_00161215 == 0) {
                sprintf(advisor.text, advisor_text[50]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                D_00161215 = 1;
                return 1;
            }
        } else {
            D_0016121d = 0;
            advisor.tutorial_step = 0x3a;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x3a:
        if (D_00154a0c[player].type == EV_EDIT_BLUEPRINT) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            D_00161212 = 0;
            D_0016121b = 0;
            D_00161213 = 0;
            D_00161214 = 0;
            D_00161215 = 0;
            D_0016121d = 0;
            advisor.tutorial_step = 0x38;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (D_00154a0c[player].type != EV_OPEN_ROOM) {
            if (D_0016121d == 0) {
                sprintf(advisor.text, advisor_text[58]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                advisor.tutorial_highlight = resvars.requester_buttons[0x52];
                set_force_help(0xd, 0x86);
                D_0016121d = 1;
                return 1;
            }
        } else {
            D_0016121d = 2;
            advisor.tutorial_step = 0x1f;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x1f:
        if (D_00161202 != 2 && !listhead[player].staff[4]) {
            if (D_00154a0c[player].type != 0x2e) {
                if (D_00161202 == 0) {
                    sprintf(advisor.text, advisor_text[31]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 0x15;
                    D_00161202 = 1;
                    return 1;
                }
            } else {
                D_00161202 = 2;
                advisor.tutorial_step = 0x23;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            D_00161202 = 2;
            clear_force_help();
            advisor.tutorial_highlight = 0;
            advisor.flags &= ~1;
        }
        break;
    case 0x23:
        if (D_00161206 != 2) {
            if (D_00154a0c[player].type == 0x38 || (D_00154a0c[player].type == EV_MARK_EMPLOYING && D_00154a0c[player].value != 1)) {
                D_00161202 = 0;
                D_00161206 = 0;
                advisor.tutorial_step = 0x1f;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type != 0x2f || D_00154a0c[player].value != 1) {
                if (D_00161206 == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[35]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 4;
                    D_00161206 = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                D_00161206 = 2;
                advisor.tutorial_step = 0x2d;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            D_00161206 = 2;
            advisor.tutorial_step = 0x2d;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x2d:
        if (D_00161210 != 2) {
            if (D_00154a0c[player].type == 0x38) {
                D_00161202 = 0;
                D_00161206 = 0;
                D_00161210 = 0;
                advisor.tutorial_step = 0x1f;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type == 0x2f && D_00154a0c[player].value != 1) {
                D_00161206 = 0;
                advisor.tutorial_step = 0x23;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (D_00154a0c[player].type != EV_MARK_EMPLOYING) {
                if (D_00161210 == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[45]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 5;
                    D_00161210 = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                D_00161210 = 2;
                advisor.tutorial_step = 0x27;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            D_00161210 = 2;
            advisor.tutorial_step = 0x27;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x27:
        if (D_0016120a != 2) {
            if (D_00154a0c[player].type == 0x38) {
                D_0016120a = 0;
                D_00161202 = 0;
                D_00161206 = 0;
                D_00161210 = 0;
                advisor.tutorial_step = 0x1f;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (!gamevar.staff_place_ok) {
                sprintf(advisor.text, advisor_text[59]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                D_0016120a = 0;
                return 1;
            } else if (!listhead[player].staff[4]) {
                if (D_0016120a == 0) {
                    sprintf(advisor.text, advisor_text[39]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    D_0016120a = 1;
                    return 1;
                }
            } else {
                D_0016120a = 2;
                advisor.tutorial_highlight = 0;
                advisor.flags &= ~1;
                show_help_page(1, 0xf);
            }
        } else {
            D_0016120a = 2;
            advisor.tutorial_highlight = 0;
            advisor.flags &= ~1;
        }
        break;
    case 0x2e:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    default:
        break;
    }
    return -1;
}
