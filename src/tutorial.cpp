#include "hosp.h"

extern "C" {
extern Event player_events[];
extern Event old_packet[];
extern u8 tutorial_display_flags[];
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
        if (tutorial_display_flags[0] != 2) {
            if (old_packet[player].type != 0x62) {
                if (tutorial_display_flags[0] == 0) {
                    sprintf(advisor.text, advisor_text[29]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 0x13;
                    tutorial_display_flags[0] = 1;
                    return 1;
                }
            } else {
                tutorial_display_flags[0] = 2;
                advisor.tutorial_step = 0x28;
                advisor.tutorial_highlight = 0;
                clear_force_help();
            }
        }
        break;
    case 0x28:
        if (tutorial_display_flags[11] != 2) {
            if (player_events[player].type == EV_CANCEL_BUY_OBJECTS) {
                tutorial_display_flags[0] = 0;
                tutorial_display_flags[11] = 0;
                advisor.tutorial_step = 0x1d;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type == 0x2b) {
                tutorial_display_flags[0] = 0;
                tutorial_display_flags[11] = 0;
                advisor.tutorial_step = 0x1d;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type != EV_BUY_OBJECT || old_packet[player].value != 0xb) {
                if (tutorial_display_flags[11] == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[40]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 0xa;
                    set_force_help(5, 0x10b);
                    tutorial_display_flags[11] = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                tutorial_display_flags[11] = 2;
                advisor.tutorial_step = 0x2a;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        }
        break;
    case 0x2a:
        if (tutorial_display_flags[13] != 2) {
            if (player_events[player].type == EV_CANCEL_BUY_OBJECTS) {
                tutorial_display_flags[0] = 0;
                tutorial_display_flags[11] = 0;
                tutorial_display_flags[13] = 0;
                advisor.tutorial_step = 0x1d;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if ((old_packet[player].type == EV_SELL_OBJECT && old_packet[player].value == 0xb) &&
                       gamevar.buy_list.to_place[0xb] == 0) {
                tutorial_display_flags[13] = 0;
                tutorial_display_flags[11] = 0;
                advisor.tutorial_step = 0x28;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type != 0x2b) {
                if (tutorial_display_flags[13] == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[42]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 9;
                    set_force_help(5, 0x27);
                    tutorial_display_flags[13] = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                tutorial_display_flags[13] = 2;
                advisor.tutorial_step = 0x29;
                advisor.tutorial_highlight = 0;
                clear_force_help();
            }
        }
        break;
    case 0x29:
        if (tutorial_display_flags[12] != 2) {
            if (player_events[player].type == EV_CANCEL_BUILD) {
                tutorial_display_flags[12] = 0;
                tutorial_display_flags[13] = 0;
                tutorial_display_flags[11] = 0;
                tutorial_display_flags[0] = 0;
                advisor.tutorial_step = 0x1d;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (advisor.flags & 2) {
                sprintf(advisor.text, advisor_text[46]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                tutorial_display_flags[12] = 0;
                return 1;
            } else if (!listhead[player].rooms[ROOM_RECEPTION]) {
                if (tutorial_display_flags[12] == 0) {
                    sprintf(advisor.text, advisor_text[41]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    tutorial_display_flags[12] = 1;
                    return 1;
                }
            } else {
                tutorial_display_flags[12] = 2;
                advisor.tutorial_step = 0x1e;
                advisor.tutorial_highlight = 0;
            }
        }
        break;
    case 0x1e:
        if (tutorial_display_flags[1] != 2 && !listhead[player].staff[0]) {
            if (old_packet[player].type != 0x2e) {
                if (tutorial_display_flags[1] == 0) {
                    sprintf(advisor.text, advisor_text[30]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 0x15;
                    tutorial_display_flags[1] = 1;
                    return 1;
                }
            } else {
                tutorial_display_flags[1] = 2;
                advisor.tutorial_step = 0x21;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            tutorial_display_flags[1] = 2;
            advisor.tutorial_step = 0x20;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x21:
        if (tutorial_display_flags[4] != 2) {
            if (old_packet[player].type == 0x38 ||
                (old_packet[player].type == EV_MARK_EMPLOYING && old_packet[player].value != 3)) {
                tutorial_display_flags[1] = 0;
                tutorial_display_flags[4] = 0;
                advisor.tutorial_step = 0x1e;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type == 0x2f && old_packet[player].value != 3) {
                if (resvars.game_mode == SCREEN_MODE_13) {
                    sprintf(advisor.text, advisor_text[33]);
                    advisor.rewrap = 1;
                }
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                advisor.tutorial_highlight = 3;
                set_force_help(4, 0x19);
                return 1;
            } else if (old_packet[player].type != 0x2f || old_packet[player].value != 3) {
                if (tutorial_display_flags[4] == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[33]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 3;
                    set_force_help(4, 0x19);
                    tutorial_display_flags[4] = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                tutorial_display_flags[4] = 2;
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
        if (tutorial_display_flags[8] != 2) {
            if (old_packet[player].type == 0x38 ||
                (old_packet[player].type == EV_MARK_EMPLOYING && old_packet[player].value != 3)) {
                tutorial_display_flags[8] = 2;
                tutorial_display_flags[9] = 2;
                tutorial_display_flags[1] = 0;
                tutorial_display_flags[4] = 0;
                advisor.tutorial_step = 0x1e;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type == EV_MARK_EMPLOYING && old_packet[player].value == 3) {
                tutorial_display_flags[8] = 2;
                tutorial_display_flags[9] = 2;
                advisor.tutorial_step = 0x24;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type == 0x2f && old_packet[player].value != 3) {
                tutorial_display_flags[8] = 2;
                tutorial_display_flags[9] = 2;
                tutorial_display_flags[4] = 0;
                advisor.tutorial_step = 0x21;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type != 0x32 || old_packet[player].value != 1) {
                if (tutorial_display_flags[8] == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[37]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 7;
                    set_force_help(4, 0x1b);
                    tutorial_display_flags[8] = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                tutorial_display_flags[8] = 2;
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
        if (tutorial_display_flags[9] != 2) {
            if (old_packet[player].type == 0x38 ||
                (old_packet[player].type == EV_MARK_EMPLOYING && old_packet[player].value != 3)) {
                tutorial_display_flags[9] = 2;
                tutorial_display_flags[1] = 0;
                tutorial_display_flags[4] = 0;
                advisor.tutorial_step = 0x1e;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type == EV_MARK_EMPLOYING && old_packet[player].value == 3) {
                tutorial_display_flags[9] = 2;
                advisor.tutorial_step = 0x24;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type == 0x2f && old_packet[player].value != 3) {
                tutorial_display_flags[9] = 2;
                tutorial_display_flags[4] = 0;
                advisor.tutorial_step = 0x21;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type != 0x32 || old_packet[player].value != 0) {
                if (tutorial_display_flags[9] == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[38]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 6;
                    set_force_help(4, 0x1a);
                    tutorial_display_flags[9] = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                tutorial_display_flags[9] = 2;
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
        if (tutorial_display_flags[5] != 2) {
            if (old_packet[player].type == 0x38) {
                tutorial_display_flags[5] = 0;
                tutorial_display_flags[1] = 0;
                tutorial_display_flags[4] = 0;
                advisor.tutorial_step = 0x1e;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type == 0x2f && old_packet[player].value != 3) {
                tutorial_display_flags[4] = 0;
                advisor.tutorial_step = 0x21;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type != EV_MARK_EMPLOYING) {
                if (tutorial_display_flags[5] == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[34]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 5;
                    set_force_help(4, 0x1c);
                    tutorial_display_flags[5] = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                tutorial_display_flags[5] = 2;
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
        if (tutorial_display_flags[7] != 2) {
            if (old_packet[player].type == 0x38) {
                tutorial_display_flags[7] = 2;
                tutorial_display_flags[1] = 0;
                tutorial_display_flags[4] = 0;
                advisor.tutorial_step = 0x1e;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (!gamevar.staff_place_ok) {
                sprintf(advisor.text, advisor_text[60]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                tutorial_display_flags[7] = 0;
                return 1;
            } else if (!listhead[player].staff[0]) {
                if (tutorial_display_flags[7] == 0) {
                    sprintf(advisor.text, advisor_text[36]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    tutorial_display_flags[7] = 1;
                    advisor.tutorial_highlight = 0;
                    return 1;
                }
            } else {
                tutorial_display_flags[7] = 2;
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
        if (tutorial_display_flags[3] != 2 && !listhead[player].rooms[ROOM_GP]) {
            if (old_packet[player].type != 0x37) {
                if (tutorial_display_flags[3] == 0) {
                    sprintf(advisor.text, advisor_text[32]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 0x12;
                    set_force_help(3, 0xb);
                    tutorial_display_flags[3] = 1;
                    return 1;
                }
            } else {
                tutorial_display_flags[3] = 2;
                advisor.tutorial_step = 0x2b;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            tutorial_display_flags[3] = 2;
            advisor.tutorial_step = 0x1f;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x2b:
        if (old_packet[player].type == 0x38) {
            tutorial_display_flags[3] = 0;
            tutorial_display_flags[14] = 0;
            advisor.tutorial_step = 0x20;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (tutorial_display_flags[14] != 2) {
            if (old_packet[player].type != EV_REQ1_RESET_ROOMS || old_packet[player].value != 3) {
                if (tutorial_display_flags[14] == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[43]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 8;
                    set_force_help(2, 1);
                    tutorial_display_flags[14] = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                tutorial_display_flags[14] = 2;
                advisor.tutorial_step = 0x2c;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        }
        break;
    case 0x2c:
        if (old_packet[player].type == 0x26) {
            tutorial_display_flags[3] = 0;
            tutorial_display_flags[14] = 0;
            tutorial_display_flags[15] = 0;
            advisor.tutorial_step = 0x20;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (tutorial_display_flags[15] != 2) {
            if (old_packet[player].type == 0x35 && old_packet[player].value != 7) {
                tutorial_display_flags[3] = 0;
                tutorial_display_flags[14] = 0;
                tutorial_display_flags[15] = 0;
                advisor.tutorial_step = 0x20;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type != 0x35) {
                if (tutorial_display_flags[15] == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[44]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 0xb;
                    lib_strcpy(othervar.help_msg, room_help[7]);
                    set_force_help(2, 0xed);
                    tutorial_display_flags[14] = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                tutorial_display_flags[15] = 2;
                advisor.tutorial_step = 0x39;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        }
        break;
    case 0x39:
        if (old_packet[player].type == 0x38) {
            tutorial_display_flags[3] = 0;
            tutorial_display_flags[14] = 0;
            tutorial_display_flags[15] = 0;
            tutorial_display_flags[28] = 0;
            advisor.tutorial_step = 0x20;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (old_packet[player].type != EV_MODIFY_BLUEPRINT) {
            sprintf(advisor.text, advisor_text[47]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            tutorial_display_flags[28] = 0;
            return 1;
        } else {
            tutorial_display_flags[28] = 2;
            advisor.tutorial_step = 0x2f;
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x2f:
        if (old_packet[player].type == 0x38) {
            tutorial_display_flags[3] = 0;
            tutorial_display_flags[14] = 0;
            tutorial_display_flags[15] = 0;
            tutorial_display_flags[28] = 0;
            tutorial_display_flags[18] = 0;
            advisor.tutorial_step = 0x20;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if ((advisor.flags & 4) && (advisor.flags & 2)) {
            sprintf(advisor.text, advisor_text[61]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            tutorial_display_flags[27] = 0;
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
            tutorial_display_flags[18] = 0;
            advisor.flags &= ~2;
            advisor.tutorial_highlight = 0;
            clear_force_help();
            return 1;
        } else {
            tutorial_display_flags[18] = 2;
            advisor.tutorial_step = 0x38;
            advisor.tutorial_highlight = 0;
            advisor.flags &= ~2;
            advisor.flags &= ~4;
            clear_force_help();
        }
        break;
    case 0x38:
        if (old_packet[player].type == 0x38) {
            tutorial_display_flags[3] = 0;
            tutorial_display_flags[14] = 0;
            tutorial_display_flags[15] = 0;
            tutorial_display_flags[28] = 0;
            tutorial_display_flags[18] = 0;
            tutorial_display_flags[27] = 0;
            tutorial_display_flags[19] = 0;
            tutorial_display_flags[20] = 0;
            tutorial_display_flags[21] = 0;
            tutorial_display_flags[29] = 0;
            advisor.tutorial_step = 0x20;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if ((advisor.flags & 4) && (advisor.flags & 2)) {
            sprintf(advisor.text, advisor_text[61]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            tutorial_display_flags[27] = 0;
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
            tutorial_display_flags[27] = 0;
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
            tutorial_display_flags[27] = 0;
            advisor.flags &= ~2;
            advisor.forced_advice_id_s = 0;
            advisor.tutorial_highlight = 0;
            clear_force_help();
            return 1;
        } else if (old_packet[player].type != EV_COMMIT_BLUEPRINT) {
            if (tutorial_display_flags[27] == 0) {
                sprintf(advisor.text, advisor_text[56]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                advisor.tutorial_highlight = resvars.requester_buttons[0x52];
                set_force_help(2, 0x86);
                tutorial_display_flags[27] = 1;
                advisor.flags &= ~2;
                return 1;
            }
        } else {
            tutorial_display_flags[27] = 2;
            advisor.tutorial_step = 0x30;
            advisor.tutorial_highlight = 0;
            clear_force_help();
        }
        break;
    case 0x30:
        if (old_packet[player].type == EV_EDIT_BLUEPRINT) {
            tutorial_display_flags[3] = 0;
            tutorial_display_flags[14] = 0;
            tutorial_display_flags[15] = 0;
            tutorial_display_flags[28] = 0;
            tutorial_display_flags[18] = 0;
            tutorial_display_flags[27] = 0;
            tutorial_display_flags[19] = 0;
            advisor.tutorial_step = 0x38;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (old_packet[player].type == EV_PLACE_BP_DOOR && othervar.door_invalid[player] == 1) {
            sprintf(advisor.text, advisor_text[53]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            tutorial_display_flags[24] = 1;
            return 1;
        } else if (old_packet[player].type != EV_PLACE_BP_DOOR) {
            if (tutorial_display_flags[19] == 0) {
                sprintf(advisor.text, advisor_text[48]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                tutorial_display_flags[19] = 1;
                return 1;
            }
        } else {
            tutorial_display_flags[19] = 2;
            advisor.tutorial_step = 0x31;
            advisor.tutorial_highlight = 0;
            clear_force_help();
        }
        break;
    case 0x31:
        if (old_packet[player].type == EV_EDIT_BLUEPRINT) {
            tutorial_display_flags[3] = 0;
            tutorial_display_flags[14] = 0;
            tutorial_display_flags[15] = 0;
            tutorial_display_flags[28] = 0;
            tutorial_display_flags[18] = 0;
            tutorial_display_flags[27] = 0;
            tutorial_display_flags[19] = 0;
            tutorial_display_flags[20] = 0;
            advisor.tutorial_step = 0x38;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (old_packet[player].type == EV_PLACE_BP_DOOR && othervar.door_invalid[player] == 1) {
            sprintf(advisor.text, advisor_text[54]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            tutorial_display_flags[25] = 1;
            return 1;
        } else if (old_packet[player].type == EV_PLACE_BP_DOOR && othervar.door_invalid[player] == 0) {
            sprintf(advisor.text, advisor_text[49]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            tutorial_display_flags[20] = 1;
            return 1;
        } else if (old_packet[player].type != EV_COMMIT_DOORS) {
            if (tutorial_display_flags[20] == 0) {
                sprintf(advisor.text, advisor_text[49]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                advisor.tutorial_highlight = resvars.requester_buttons[0x52];
                set_force_help(0xd, 0x86);
                tutorial_display_flags[20] = 1;
                return 1;
            }
        } else {
            tutorial_display_flags[19] = 2;
            advisor.tutorial_step = 0x32;
            advisor.tutorial_highlight = 0;
            clear_force_help();
            advisor.flags &= ~2;
        }
        break;
    case 0x32:
        if (old_packet[player].type == EV_EDIT_BLUEPRINT) {
            tutorial_display_flags[3] = 0;
            tutorial_display_flags[14] = 0;
            tutorial_display_flags[15] = 0;
            tutorial_display_flags[28] = 0;
            tutorial_display_flags[18] = 0;
            tutorial_display_flags[27] = 0;
            tutorial_display_flags[19] = 0;
            tutorial_display_flags[20] = 0;
            tutorial_display_flags[21] = 0;
            advisor.tutorial_step = 0x38;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (advisor.flags & 2) {
            sprintf(advisor.text, advisor_text[55]);
            advisor.rewrap = 1;
            advisor.line_count = 0;
            advisor.extra_segments = 0;
            advisor.flags &= ~2;
            tutorial_display_flags[21] = 0;
            return 1;
        } else if (!(advisor.flags & 0x10)) {
            if (tutorial_display_flags[21] == 0) {
                sprintf(advisor.text, advisor_text[50]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                tutorial_display_flags[21] = 1;
                return 1;
            }
        } else {
            tutorial_display_flags[29] = 0;
            advisor.tutorial_step = 0x3a;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x3a:
        if (old_packet[player].type == EV_EDIT_BLUEPRINT) {
            tutorial_display_flags[3] = 0;
            tutorial_display_flags[14] = 0;
            tutorial_display_flags[15] = 0;
            tutorial_display_flags[28] = 0;
            tutorial_display_flags[18] = 0;
            tutorial_display_flags[27] = 0;
            tutorial_display_flags[19] = 0;
            tutorial_display_flags[20] = 0;
            tutorial_display_flags[21] = 0;
            tutorial_display_flags[29] = 0;
            advisor.tutorial_step = 0x38;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        } else if (old_packet[player].type != EV_OPEN_ROOM) {
            if (tutorial_display_flags[29] == 0) {
                sprintf(advisor.text, advisor_text[58]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                advisor.tutorial_highlight = resvars.requester_buttons[0x52];
                set_force_help(0xd, 0x86);
                tutorial_display_flags[29] = 1;
                return 1;
            }
        } else {
            tutorial_display_flags[29] = 2;
            advisor.tutorial_step = 0x1f;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x1f:
        if (tutorial_display_flags[2] != 2 && !listhead[player].staff[4]) {
            if (old_packet[player].type != 0x2e) {
                if (tutorial_display_flags[2] == 0) {
                    sprintf(advisor.text, advisor_text[31]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 0x15;
                    tutorial_display_flags[2] = 1;
                    return 1;
                }
            } else {
                tutorial_display_flags[2] = 2;
                advisor.tutorial_step = 0x23;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            tutorial_display_flags[2] = 2;
            clear_force_help();
            advisor.tutorial_highlight = 0;
            advisor.flags &= ~1;
        }
        break;
    case 0x23:
        if (tutorial_display_flags[6] != 2) {
            if (old_packet[player].type == 0x38 ||
                (old_packet[player].type == EV_MARK_EMPLOYING && old_packet[player].value != 1)) {
                tutorial_display_flags[2] = 0;
                tutorial_display_flags[6] = 0;
                advisor.tutorial_step = 0x1f;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type != 0x2f || old_packet[player].value != 1) {
                if (tutorial_display_flags[6] == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[35]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 4;
                    tutorial_display_flags[6] = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                tutorial_display_flags[6] = 2;
                advisor.tutorial_step = 0x2d;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            tutorial_display_flags[6] = 2;
            advisor.tutorial_step = 0x2d;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x2d:
        if (tutorial_display_flags[16] != 2) {
            if (old_packet[player].type == 0x38) {
                tutorial_display_flags[2] = 0;
                tutorial_display_flags[6] = 0;
                tutorial_display_flags[16] = 0;
                advisor.tutorial_step = 0x1f;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type == 0x2f && old_packet[player].value != 1) {
                tutorial_display_flags[6] = 0;
                advisor.tutorial_step = 0x23;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (old_packet[player].type != EV_MARK_EMPLOYING) {
                if (tutorial_display_flags[16] == 0) {
                    if (resvars.game_mode == SCREEN_MODE_13) {
                        sprintf(advisor.text, advisor_text[45]);
                        advisor.rewrap = 1;
                    }
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    advisor.tutorial_highlight = 5;
                    tutorial_display_flags[16] = 1;
                    if (resvars.game_mode == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(advisor.text, 0, 0x10e);
                    return 0;
                }
            } else {
                tutorial_display_flags[16] = 2;
                advisor.tutorial_step = 0x27;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            }
        } else {
            tutorial_display_flags[16] = 2;
            advisor.tutorial_step = 0x27;
            clear_force_help();
            advisor.tutorial_highlight = 0;
        }
        break;
    case 0x27:
        if (tutorial_display_flags[10] != 2) {
            if (old_packet[player].type == 0x38) {
                tutorial_display_flags[10] = 0;
                tutorial_display_flags[2] = 0;
                tutorial_display_flags[6] = 0;
                tutorial_display_flags[16] = 0;
                advisor.tutorial_step = 0x1f;
                clear_force_help();
                advisor.tutorial_highlight = 0;
            } else if (!gamevar.staff_place_ok) {
                sprintf(advisor.text, advisor_text[59]);
                advisor.rewrap = 1;
                advisor.line_count = 0;
                advisor.extra_segments = 0;
                tutorial_display_flags[10] = 0;
                return 1;
            } else if (!listhead[player].staff[4]) {
                if (tutorial_display_flags[10] == 0) {
                    sprintf(advisor.text, advisor_text[39]);
                    advisor.rewrap = 1;
                    advisor.line_count = 0;
                    advisor.extra_segments = 0;
                    tutorial_display_flags[10] = 1;
                    return 1;
                }
            } else {
                tutorial_display_flags[10] = 2;
                advisor.tutorial_highlight = 0;
                advisor.flags &= ~1;
                show_help_page(1, 0xf);
            }
        } else {
            tutorial_display_flags[10] = 2;
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
