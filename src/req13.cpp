#include "hosp.h"

struct Req13Layout {
    u8 unknown0[0x240];
    s32 text_x;
    s32 text_y;
    s32 text_w;
    s32 text_h;
};

extern "C" {

extern Event D_001549d0[];

}

void req13_no_pressed(void);

// FUNCTION: 0x0006fbc0
void req13(Window *win)
{
    u16 i;
    WindowButton *button;

    for (button = win->buttons, i = 1; button; button = button->next) {
        if (button->flags & 6) {
            debug.unknown4 = i;
            if (i == resvars.fe[1]) {
                req13_no_pressed();
                play_sound(10, SFX_NO4, -1, -1);
            } else if (i == resvars.fe[0]) {
                D_001549d0[player].type = othervar.f99e;
                D_001549d0[player].value = othervar.f99f;
                D_001549d0[player].arg = othervar.f9a2;
                play_sound(10, SFX_YESX, -1, -1);
            }
        }
        i++;
    }
}

// FUNCTION: 0x0006fcc4
void req13_overdraw(Window *win)
{
    s32 x;
    s32 y;

    compute_help_text(0, 0, 8);
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
                              window.unknown2cf - window.unknown2c7);
    x = win->x - lbDisplay.graphics_window_x;
    y = win->y - lbDisplay.graphics_window_y;
    set_font_from_cache(1);
    lbDisplay.draw_flags = 0x20;
    LbTextSetWindow(resvars.f18_Req13Layoutp->text_x, resvars.f18_Req13Layoutp->text_y, resvars.f18_Req13Layoutp->text_w, resvars.f18_Req13Layoutp->text_h);
    LbTextDraw(0, 0, othervar.f935);
    lbDisplay.draw_flags = 0;
}

// FUNCTION: 0x0006fd98
void open_yes_no_requester(u8 yes_type, u16 yes_value, u16 yes_arg, u8 no_type, u16 no_value, u16 no_arg)
{
    othervar.f99e = yes_type;
    othervar.f99f = yes_value;
    othervar.f9a2 = yes_arg;
    othervar.f9ad = no_type;
    othervar.f9ae = no_value;
    othervar.f9b1 = no_arg;
    D_001549d0[player].type = 0x77;
}

// FUNCTION: 0x0006fdfc
void close_yes_no_requester(u8 reopen)
{
    if (crequester.unknown2 == 4) {
        pmenu.unknown7 = 0;
        display_mode = 0;
        close_crequester(crequester.unknown2);
        close_irequesters();
        window.unknown2eb = 0;
        if (reopen && othervar.f9c3 != 0x63) {
            crequester.unknown2 = othervar.f9c3;
            fn_0001bc94(crequester.unknown2, 1);
            set_mouse_mode(othervar.f9c4);
        }
    }
}

// FUNCTION: 0x0006fe88
void req13_no_pressed(void)
{
    D_001549d0[player].type = othervar.f9ad;
    D_001549d0[player].value = othervar.f9ae;
    D_001549d0[player].arg = othervar.f9b1;
}

// FUNCTION: 0x0006fed8
void req13_cancel_check(void)
{
    if (crequester.unknown2 == 4) {
        if (lbInkey == KC_ESCAPE || (lbDisplay.right_button && !mouse_was_on_window)) {
            req13_no_pressed();
            lbDisplay.right_button = 0;
        }
    }
}
