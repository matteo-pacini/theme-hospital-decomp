#include "hosp.h"

struct Req13Layout {
    u8 unknown0[0x240];
    s32 text_x;
    s32 text_y;
    s32 text_w;
    s32 text_h;
};

extern "C" {

extern Event player_events[];
}

void req13_no_pressed(void);

// Yes/no requester clicks: yes sends the event set by open_yes_no_requester,
// no its no event.
// FUNCTION: 0x0006fbc0
void req13(Window *win)
{
    u16 i;
    WindowButton *button;

    for (button = win->buttons, i = 1; button; button = button->next) {
        if (button->flags & 6) {
            debug.last_button = i;
            if (i == resvars.requester_buttons[1]) {
                req13_no_pressed();
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
            } else if (i == resvars.requester_buttons[0]) {
                player_events[player].type = othervar.yes_type;
                player_events[player].value = othervar.yes_value;
                player_events[player].arg = othervar.yes_arg;
                play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
            }
        }
        i++;
    }
}

// Draws the yes/no requester's question (othervar.yesno_msg).
// FUNCTION: 0x0006fcc4
void req13_overdraw(Window *win)
{
    s32 x;
    s32 y;

    compute_help_text(0, 0, 8);
    LbScreenSetGraphicsWindow(window.clip_x0, window.clip_y0, window.clip_x1 - window.clip_x0,
                              window.clip_y1 - window.clip_y0);
    x = win->x - lbDisplay.graphics_window_x;
    y = win->y - lbDisplay.graphics_window_y;
    set_font_from_cache(1);
    lbDisplay.draw_flags = 0x20;
    LbTextSetWindow(resvars.boxes_Req13Layoutp->text_x, resvars.boxes_Req13Layoutp->text_y,
                    resvars.boxes_Req13Layoutp->text_w, resvars.boxes_Req13Layoutp->text_h);
    LbTextDraw(0, 0, othervar.yesno_msg);
    lbDisplay.draw_flags = 0;
}

// Opens the yes/no requester (event EV_OPEN_YES_NO) for the question in othervar.yesno_msg,
// which sends event yes_type (with yes_value, yes_arg) on yes and no_type
// (no_value, no_arg) on no.
// FUNCTION: 0x0006fd98
void open_yes_no_requester(u8 yes_type, u16 yes_value, u16 yes_arg, u8 no_type, u16 no_value, u16 no_arg)
{
    othervar.yes_type = yes_type;
    othervar.yes_value = yes_value;
    othervar.yes_arg = yes_arg;
    othervar.no_type = no_type;
    othervar.no_value = no_value;
    othervar.no_arg = no_arg;
    player_events[player].type = EV_OPEN_YES_NO;
}

// Closes the yes/no requester if it is open, and with reopen set reopens the
// requester and mouse mode it covered (othervar.yesno_saved_requester, yesno_saved_mouse_mode).
// FUNCTION: 0x0006fdfc
void close_yes_no_requester(u8 reopen)
{
    if (crequester.id == REQ_YES_NO) {
        pmenu.disabled = 0;
        display_mode = DM_GAME;
        close_crequester(crequester.id);
        close_irequesters();
        window.current_window = 0;
        if (reopen && othervar.yesno_saved_requester != REQ_NONE) {
            crequester.id = othervar.yesno_saved_requester;
            open_requester(crequester.id, 1);
            set_mouse_mode(othervar.yesno_saved_mouse_mode);
        }
    }
}

// Sends the yes/no requester's no event.
// FUNCTION: 0x0006fe88
void req13_no_pressed(void)
{
    player_events[player].type = othervar.no_type;
    player_events[player].value = othervar.no_value;
    player_events[player].arg = othervar.no_arg;
}

// Answers no to the open yes/no requester on Escape or a right click outside
// any window.
// FUNCTION: 0x0006fed8
void req13_cancel_check(void)
{
    if (crequester.id == REQ_YES_NO) {
        if (lbInkey == KC_ESCAPE || (lbDisplay.right_button && !mouse_was_on_window)) {
            req13_no_pressed();
            lbDisplay.right_button = 0;
        }
    }
}
