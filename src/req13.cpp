#include "hosp.h"

struct Req13Layout {
    u8 unknown0[0x240];
    s32 text_x;
    s32 text_y;
    s32 text_w;
    s32 text_h;
};

extern "C" {

extern u8 *D_0015f402;
extern Req13Layout *D_0015f40c;
extern Event D_001549d0[];
extern u8 D_00155d56;
extern u16 D_00155d57;
extern u16 D_00155d5a;
extern u8 D_00155d65;
extern u16 D_00155d66;
extern u16 D_00155d69;
extern u8 D_00155d7b;
extern u8 D_00155d7c;
extern char D_00155ced[];
extern u16 D_00188c74;
extern u8 D_00188c7c;
extern s32 D_00188c3c;
extern s32 D_00188c40;

}

void req13_no_pressed(void);

// NONMATCHING: register choice
// FUNCTION: 0x0006fbc0
void req13(Window *w)
{
    u16 i;
    WindowButton *b;

    for (b = w->buttons, i = 1; b; b = b->next) {
        if (b->flags & 6) {
            debug.unknown4 = i;
            if (i == D_0015f402[1]) {
                req13_no_pressed();
                play_sound(10, 0x42, -1, -1);
            } else if (i == D_0015f402[0]) {
                D_001549d0[player].type = D_00155d56;
                D_001549d0[player].value = D_00155d57;
                D_001549d0[player].arg = D_00155d5a;
                play_sound(10, 0x41, -1, -1);
            }
        }
        i++;
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0006fcc4
void req13_overdraw(Window *w)
{
    s32 x;
    s32 y;

    compute_help_text(0, 0, 8);
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
                              window.unknown2cf - window.unknown2c7);
    x = w->x - D_00188c3c;
    y = w->y - D_00188c40;
    set_font_from_cache(1);
    D_00188c74 = 0x20;
    LbTextSetWindow(D_0015f40c->text_x, D_0015f40c->text_y, D_0015f40c->text_w, D_0015f40c->text_h);
    LbTextDraw(0, 0, D_00155ced);
    D_00188c74 = 0;
}

// FUNCTION: 0x0006fd98
void open_yes_no_requester(u8 a, u16 b, u16 c, u8 d, u16 e, u16 f)
{
    D_00155d56 = a;
    D_00155d57 = b;
    D_00155d5a = c;
    D_00155d65 = d;
    D_00155d66 = e;
    D_00155d69 = f;
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
        if (reopen && D_00155d7b != 0x63) {
            crequester.unknown2 = D_00155d7b;
            fn_0001bc94(crequester.unknown2, 1);
            set_mouse_mode(D_00155d7c);
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0006fe88
void req13_no_pressed(void)
{
    D_001549d0[player].type = D_00155d65;
    D_001549d0[player].value = D_00155d66;
    D_001549d0[player].arg = D_00155d69;
}

// FUNCTION: 0x0006fed8
void req13_cancel_check(void)
{
    if (crequester.unknown2 == 4) {
        if (lbInkey == 1 || (D_00188c7c && !mouse_was_on_window)) {
            req13_no_pressed();
            D_00188c7c = 0;
        }
    }
}
