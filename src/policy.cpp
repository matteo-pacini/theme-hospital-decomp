#include "hosp.h"

struct PolicyRect {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

extern "C" {

extern u16 D_00188c74;
extern s32 D_00188c38;
extern u8 D_00188c7b, D_00188c7e, D_00188c81;
extern volatile s16 D_00188c6c, D_00188c70;
extern Event D_001549d0[];
extern Button *D_0015f43c;
extern PolicyRect *D_0015f430;
extern Point *D_0015f434;
extern char **D_001602dc;
extern char D_00120414[];
extern char D_00120417[];
extern char D_0012041a[];
extern char D_0012041d[];
extern char D_00120420[];

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

// NONMATCHING: register choice
// FUNCTION: 0x0004488c
void fn_0004488c(void)
{
    u16 i;
    s32 x;
    char buf[200];

    D_00188c74 = 0x100;
    set_font_from_cache(7);
    for (i = 0; i < 5; i++) {
        LbTextSetWindow(D_0015f430[i + 0x20].x, D_0015f430[i + 0x20].y, D_0015f430[i + 0x20].w, D_0015f430[i + 0x20].h);
        sprintf(buf, D_00120414, D_001602dc[i]);
        LbTextDraw(0, 0, buf);
        D_00188c74 = 0x20;
    }
    set_font_from_cache(8);
    D_00188c74 = 0x100;
    x = allhosps[player].f219 * 3;
    if (x < 0)
        x = 0;
    else if (x > 299)
        x = 299;
    reqspr_draw(D_0015f430[0x25].x + x, D_0015f430[0x25].y, &screen_sprites_tab_start[2]);
    LbTextSetWindow(D_0015f434[0x54].x + (D_0015f430[0x25].x + x), D_0015f430[0x25].y + D_0015f434[0x54].y,
                    screen_sprites_tab_start[2].width, screen_sprites_tab_start[2].height);
    sprintf(buf, D_00120417, D_001602dc[i++]);
    LbTextDraw(0, 0, buf);
    x = allhosps[player].f21b * 3;
    if (x < 0)
        x = 0;
    else if (x > 299)
        x = 299;
    reqspr_draw(D_0015f430[0x26].x + x, D_0015f430[0x26].y, &screen_sprites_tab_start[1]);
    LbTextSetWindow(D_0015f434[0x54].x + (D_0015f430[0x26].x + x), D_0015f430[0x26].y + D_0015f434[0x54].y,
                    screen_sprites_tab_start[1].width, screen_sprites_tab_start[1].height);
    sprintf(buf, D_0012041a, D_001602dc[i++]);
    LbTextDraw(0, 0, buf);
    x = (allhosps[player].f217 - 1000) / 0x42 * 20;
    if (x < 0)
        x = 0;
    else if (x > 299)
        x = 299;
    reqspr_draw(D_0015f430[0x27].x + x, D_0015f430[0x27].y, &screen_sprites_tab_start[3]);
    LbTextSetWindow(D_0015f434[0x54].x + (D_0015f430[0x27].x + x), D_0015f430[0x27].y + D_0015f434[0x54].y,
                    screen_sprites_tab_start[3].width, screen_sprites_tab_start[3].height);
    sprintf(buf, D_0012041d, D_001602dc[i++]);
    LbTextDraw(0, 0, buf);
    x = allhosps[player].f21d / 0x27 * 10;
    if (x < 0)
        x = 0;
    else if (x > 0xfb)
        x = 0xfb;
    reqspr_draw(D_0015f430[0x28].x + x, D_0015f430[0x28].y, &screen_sprites_tab_start[3]);
    LbTextSetWindow(D_0015f434[0x54].x + (D_0015f430[0x28].x + x), D_0015f430[0x28].y + D_0015f434[0x54].y,
                    screen_sprites_tab_start[3].width, screen_sprites_tab_start[3].height);
    sprintf(buf, D_00120420, D_001602dc[i++]);
    LbTextDraw(0, 0, buf);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    if (allhosps[player].f21f)
        reqspr_draw(D_0015f434[0x52].x, D_0015f434[0x52].y, &screen_sprites_tab_start[4]);
    else
        reqspr_draw(D_0015f434[0x53].x, D_0015f434[0x53].y, &screen_sprites_tab_start[5]);
    if (D_00188c7e) {
        if (pol.unknown0 == 0) {
            if (over_hotspot(mouse_dx, mouse_dy, 0, D_0015f430[0x25].y, D_00188c34, D_0015f430[0x25].h))
                pol.unknown0 = 2;
            else if (over_hotspot(mouse_dx, mouse_dy, 0, D_0015f430[0x26].y, D_00188c34, D_0015f430[0x26].h))
                pol.unknown0 = 1;
            else if (over_hotspot(mouse_dx, mouse_dy, 0, D_0015f430[0x27].y, D_00188c34, D_0015f430[0x27].h))
                pol.unknown0 = 3;
            else if (over_hotspot(mouse_dx, mouse_dy, 0, D_0015f430[0x28].y, D_00188c34, D_0015f430[0x28].h))
                pol.unknown0 = 4;
            else
                pol.unknown0 = 0x63;
        }
        if (pol.unknown0 == 2) {
            if (over_hotspot(mouse_dx, mouse_dy, screen_sprites_tab_start[2].width / 2 + D_0015f430[0x25].x, 0,
                             screen_sprites_tab_start[2].width / 2 + D_0015f430[0x25].w, D_00188c38)) {
                x = (mouse_dx - screen_sprites_tab_start[2].width / 2 - D_0015f430[0x25].x) / 3;
                if (x < 0)
                    x = 0;
                else if (x > 100)
                    x = 100;
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 2;
                D_001549d0[player].arg = x;
                pol.unknown0 = 2;
            }
            if (over_hotspot(mouse_dx, mouse_dy, 0, 0, D_0015f430[0x25].x, D_00188c38)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 2;
                D_001549d0[player].arg = 0;
                pol.unknown0 = 2;
            }
            if (over_hotspot(mouse_dx, mouse_dy, D_0015f430[0x25].x + D_0015f430[0x25].w + screen_sprites_tab_start[2].width / 2, 0,
                             D_00188c34 - (screen_sprites_tab_start[2].width / 2 + D_0015f430[0x25].w), D_00188c38)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 2;
                D_001549d0[player].arg = 100;
                pol.unknown0 = 2;
            }
        }
        if (pol.unknown0 == 1) {
            if (over_hotspot(mouse_dx, mouse_dy, screen_sprites_tab_start[1].width / 2 + D_0015f430[0x26].x, 0,
                             screen_sprites_tab_start[1].width / 2 + D_0015f430[0x26].w, D_00188c38)) {
                x = (mouse_dx - screen_sprites_tab_start[1].width / 2 - D_0015f430[0x26].x) / 3;
                if (x < 0)
                    x = 0;
                else if (x > 100)
                    x = 100;
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 1;
                D_001549d0[player].arg = x;
                pol.unknown0 = 1;
            }
            if (over_hotspot(mouse_dx, mouse_dy, 0, 0, D_0015f430[0x26].x, D_00188c38)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 1;
                D_001549d0[player].arg = 0;
                pol.unknown0 = 1;
            }
            if (over_hotspot(mouse_dx, mouse_dy, D_0015f430[0x26].x + D_0015f430[0x26].w + screen_sprites_tab_start[1].width / 2, 0,
                             D_00188c34 - (screen_sprites_tab_start[1].width / 2 + D_0015f430[0x26].w), D_00188c38)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 1;
                D_001549d0[player].arg = 100;
                pol.unknown0 = 1;
            }
        }
        if (pol.unknown0 == 3) {
            if (over_hotspot(mouse_dx, mouse_dy, screen_sprites_tab_start[3].width / 2 + D_0015f430[0x27].x, 0,
                             screen_sprites_tab_start[3].width / 2 + D_0015f430[0x27].w, D_00188c38)) {
                x = (mouse_dx - screen_sprites_tab_start[3].width / 2 - D_0015f430[0x27].x) / 20 * 0x42;
                if (x < 0)
                    x = 0;
                else if (x > 1000)
                    x = 1000;
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 3;
                D_001549d0[player].arg = x + 1000;
                pol.unknown0 = 3;
            }
            if (over_hotspot(mouse_dx, mouse_dy, 0, 0, D_0015f430[0x27].x, D_00188c38)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 3;
                D_001549d0[player].arg = 1000;
                pol.unknown0 = 3;
            }
            if (over_hotspot(mouse_dx, mouse_dy, D_0015f430[0x27].x + D_0015f430[0x27].w + screen_sprites_tab_start[3].width / 2, 0,
                             D_00188c34 - (screen_sprites_tab_start[3].width / 2 + D_0015f430[0x27].w), D_00188c38)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 3;
                D_001549d0[player].arg = 2000;
                pol.unknown0 = 3;
            }
        }
        if (pol.unknown0 == 4) {
            if (over_hotspot(mouse_dx, mouse_dy, screen_sprites_tab_start[3].width / 2 + D_0015f430[0x28].x, 0,
                             screen_sprites_tab_start[3].width / 2 + D_0015f430[0x28].w, D_00188c38)) {
                x = (mouse_dx - screen_sprites_tab_start[3].width / 2 - D_0015f430[0x28].x) / 10 * 0x27;
                if (x < 0)
                    x = 0;
                else if (x > 1000)
                    x = 1000;
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 4;
                D_001549d0[player].arg = x;
                pol.unknown0 = 4;
            }
            if (over_hotspot(mouse_dx, mouse_dy, 0, 0, D_0015f430[0x28].x, D_00188c38)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 4;
                D_001549d0[player].arg = 0;
                pol.unknown0 = 4;
            }
            if (over_hotspot(mouse_dx, mouse_dy, D_0015f430[0x28].x + D_0015f430[0x28].w + screen_sprites_tab_start[3].width / 2, 0,
                             D_00188c34 - (screen_sprites_tab_start[3].width / 2 + D_0015f430[0x28].w), D_00188c38)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 4;
                D_001549d0[player].arg = 1000;
                pol.unknown0 = 4;
            }
        }
    } else {
        pol.unknown0 = 0;
    }
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
}

}
