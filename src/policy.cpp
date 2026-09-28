#include "hosp.h"

extern "C" {

extern u16 D_00188c74;
extern s32 D_00188c38;
extern u8 D_00188c7b, D_00188c7e, D_00188c81;
extern volatile s16 D_00188c6c, D_00188c70;
extern Event D_001549d0[];
extern Button *D_0015f43c;

void fn_00044664(void);
void fn_0004488c(void);

}

// FUNCTION: 0x00044530
void display_policy(void)
{
    switch (pol.unknown2) {
    case 0:
        load_ingame_pic(5, 1, 0x1c, 0, SCREEN_MODE_13);
        pol.unknown2 = 1;
        pol.unknown0 = 0;
        D_00188c74 = 0;
        fn_0004488c();
        D_00188c7b = D_00188c7e = D_00188c81 = 0;
        break;
    case 1:
        D_00188c74 = 0;
        copy_screen_to_wscreen(bscreen);
        fn_00044664();
        fn_0004488c();
        break;
    case 2:
        pol.unknown2 = 3;
        break;
    case 3:
        D_001549d0[player].type = 0x70;
        D_001549d0[player].value = 0;
        break;
    }
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    if (exit_quick_requester()) {
        pol.unknown2 = 2;
    }
    compute_help_text(0, 0, 0x19);
}

extern "C" {

// NONMATCHING: register choice
// FUNCTION: 0x00044664
void fn_00044664(void)
{
    u8 i;

    if (D_00188c81) {
        for (i = 0; D_0015f43c[i + 10].width != 0 && D_0015f43c[i + 10].height != 0; i++) {
            if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i + 10].x, D_0015f43c[i + 10].y,
                            (s16)D_0015f43c[i + 10].width, (s16)D_0015f43c[i + 10].height)) {
                D_0015f43c[i + 10].callback(i);
            }
        }
        D_00188c81 = 0;
    }
}

// FUNCTION: 0x0004477c
void fn_0004477c(u8 action)
{
    debug.unknown8 = action;
    switch (action) {
    case 0:
        D_001549d0[player].type = 0x71;
        D_001549d0[player].value = 5;
        D_001549d0[player].arg = 1;
        play_sound(10, 0x43, -1, -1);
        break;
    case 1:
        D_001549d0[player].type = 0x71;
        D_001549d0[player].value = 5;
        D_001549d0[player].arg = 0;
        play_sound(10, 0x43, -1, -1);
        break;
    case 2:
        pol.unknown2 = 2;
        play_sound(10, 0x43, -1, -1);
        break;
    }
}

}
