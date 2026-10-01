#include "hosp.h"

struct PolicyRect {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

extern "C" {

extern Event D_001549d0[];
extern char **D_001602dc;

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
        lbDisplay.draw_flags = 0;
        fn_0004488c();
        lbDisplay.left_button = lbDisplay.mleft_button = lbDisplay.rleft_button = 0;
        break;
    case 1:
        lbDisplay.draw_flags = 0;
        copy_screen_to_wscreen(bscreen);
        fn_00044664();
        fn_0004488c();
        break;
    case 2:
        pol.unknown2 = 3;
        break;
    case 3:
        D_001549d0[player].type = EV_RETURN_FROM_SCREEN;
        D_001549d0[player].value = 0;
        break;
    }
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
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

    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.f48[i + 10].width != 0 && resvars.f48[i + 10].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f48[i + 10].x, resvars.f48[i + 10].y,
                            (s16)resvars.f48[i + 10].width, (s16)resvars.f48[i + 10].height)) {
                resvars.f48[i + 10].callback(i);
            }
        }
        lbDisplay.rleft_button = 0;
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
        play_sound(10, SFX_SELECTX, -1, -1);
        break;
    case 1:
        D_001549d0[player].type = 0x71;
        D_001549d0[player].value = 5;
        D_001549d0[player].arg = 0;
        play_sound(10, SFX_SELECTX, -1, -1);
        break;
    case 2:
        pol.unknown2 = 2;
        play_sound(10, SFX_SELECTX, -1, -1);
        break;
    }
}

// FUNCTION: 0x0004488c
void fn_0004488c(void)
{
    u16 i;
    s32 x;
    char buf[200];

    lbDisplay.draw_flags = 0x100;
    set_font_from_cache(7);
    for (i = 0; i < 5; i++) {
        LbTextSetWindow(resvars.f3c_PolicyRectp[i + 0x20].x, resvars.f3c_PolicyRectp[i + 0x20].y, resvars.f3c_PolicyRectp[i + 0x20].w, resvars.f3c_PolicyRectp[i + 0x20].h);
        sprintf(buf, "%s", D_001602dc[i]);
        LbTextDraw(0, 0, buf);
        lbDisplay.draw_flags = 0x20;
    }
    set_font_from_cache(8);
    lbDisplay.draw_flags = 0x100;
    x = allhosps[player].f219 * 3;
    if (x < 0)
        x = 0;
    else if (x > 299)
        x = 299;
    reqspr_draw(resvars.f3c_PolicyRectp[0x25].x + x, resvars.f3c_PolicyRectp[0x25].y, &screen_sprites_tab_start[2]);
    LbTextSetWindow((resvars.f3c_PolicyRectp[0x25].x + x) + resvars.f40[0x54].x, resvars.f3c_PolicyRectp[0x25].y + resvars.f40[0x54].y,
                    screen_sprites_tab_start[2].width, screen_sprites_tab_start[2].height);
    sprintf(buf, "%s", D_001602dc[i++]);
    LbTextDraw(0, 0, buf);
    x = allhosps[player].f21b * 3;
    if (x < 0)
        x = 0;
    else if (x > 299)
        x = 299;
    reqspr_draw(resvars.f3c_PolicyRectp[0x26].x + x, resvars.f3c_PolicyRectp[0x26].y, &screen_sprites_tab_start[1]);
    LbTextSetWindow((resvars.f3c_PolicyRectp[0x26].x + x) + resvars.f40[0x54].x, resvars.f3c_PolicyRectp[0x26].y + resvars.f40[0x54].y,
                    screen_sprites_tab_start[1].width, screen_sprites_tab_start[1].height);
    sprintf(buf, "%s", D_001602dc[i++]);
    LbTextDraw(0, 0, buf);
    x = (allhosps[player].f217 - 1000) / 0x42 * 20;
    if (x < 0)
        x = 0;
    else if (x > 299)
        x = 299;
    reqspr_draw(resvars.f3c_PolicyRectp[0x27].x + x, resvars.f3c_PolicyRectp[0x27].y, &screen_sprites_tab_start[3]);
    LbTextSetWindow((resvars.f3c_PolicyRectp[0x27].x + x) + resvars.f40[0x54].x, resvars.f3c_PolicyRectp[0x27].y + resvars.f40[0x54].y,
                    screen_sprites_tab_start[3].width, screen_sprites_tab_start[3].height);
    sprintf(buf, "%s", D_001602dc[i++]);
    LbTextDraw(0, 0, buf);
    x = allhosps[player].f21d / 0x27 * 10;
    if (x < 0)
        x = 0;
    else if (x > 0xfb)
        x = 0xfb;
    reqspr_draw(resvars.f3c_PolicyRectp[0x28].x + x, resvars.f3c_PolicyRectp[0x28].y, &screen_sprites_tab_start[3]);
    LbTextSetWindow((resvars.f3c_PolicyRectp[0x28].x + x) + resvars.f40[0x54].x, resvars.f3c_PolicyRectp[0x28].y + resvars.f40[0x54].y,
                    screen_sprites_tab_start[3].width, screen_sprites_tab_start[3].height);
    sprintf(buf, "%s", D_001602dc[i++]);
    LbTextDraw(0, 0, buf);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    if (allhosps[player].f21f)
        reqspr_draw(resvars.f40[0x52].x, resvars.f40[0x52].y, &screen_sprites_tab_start[4]);
    else
        reqspr_draw(resvars.f40[0x53].x, resvars.f40[0x53].y, &screen_sprites_tab_start[5]);
    if (lbDisplay.mleft_button) {
        if (pol.unknown0 == 0) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, resvars.f3c_PolicyRectp[0x25].y, lbDisplay.graphics_screen_width, resvars.f3c_PolicyRectp[0x25].h))
                pol.unknown0 = 2;
            else if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, resvars.f3c_PolicyRectp[0x26].y, lbDisplay.graphics_screen_width, resvars.f3c_PolicyRectp[0x26].h))
                pol.unknown0 = 1;
            else if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, resvars.f3c_PolicyRectp[0x27].y, lbDisplay.graphics_screen_width, resvars.f3c_PolicyRectp[0x27].h))
                pol.unknown0 = 3;
            else if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, resvars.f3c_PolicyRectp[0x28].y, lbDisplay.graphics_screen_width, resvars.f3c_PolicyRectp[0x28].h))
                pol.unknown0 = 4;
            else
                pol.unknown0 = 0x63;
        }
        if (pol.unknown0 == 2) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, screen_sprites_tab_start[2].width / 2 + resvars.f3c_PolicyRectp[0x25].x, 0,
                             screen_sprites_tab_start[2].width / 2 + resvars.f3c_PolicyRectp[0x25].w, lbDisplay.graphics_screen_height)) {
                x = (lbDisplay.mmouse_x - screen_sprites_tab_start[2].width / 2 - resvars.f3c_PolicyRectp[0x25].x) / 3;
                if (x < 0)
                    x = 0;
                else if (x > 100)
                    x = 100;
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 2;
                D_001549d0[player].arg = x;
                pol.unknown0 = 2;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, 0, resvars.f3c_PolicyRectp[0x25].x, lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 2;
                D_001549d0[player].arg = 0;
                pol.unknown0 = 2;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f3c_PolicyRectp[0x25].x + resvars.f3c_PolicyRectp[0x25].w + screen_sprites_tab_start[2].width / 2, 0,
                             lbDisplay.graphics_screen_width - (screen_sprites_tab_start[2].width / 2 + resvars.f3c_PolicyRectp[0x25].w), lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 2;
                D_001549d0[player].arg = 100;
                pol.unknown0 = 2;
            }
        }
        if (pol.unknown0 == 1) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, screen_sprites_tab_start[1].width / 2 + resvars.f3c_PolicyRectp[0x26].x, 0,
                             screen_sprites_tab_start[1].width / 2 + resvars.f3c_PolicyRectp[0x26].w, lbDisplay.graphics_screen_height)) {
                x = (lbDisplay.mmouse_x - screen_sprites_tab_start[1].width / 2 - resvars.f3c_PolicyRectp[0x26].x) / 3;
                if (x < 0)
                    x = 0;
                else if (x > 100)
                    x = 100;
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 1;
                D_001549d0[player].arg = x;
                pol.unknown0 = 1;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, 0, resvars.f3c_PolicyRectp[0x26].x, lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 1;
                D_001549d0[player].arg = 0;
                pol.unknown0 = 1;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f3c_PolicyRectp[0x26].x + resvars.f3c_PolicyRectp[0x26].w + screen_sprites_tab_start[1].width / 2, 0,
                             lbDisplay.graphics_screen_width - (screen_sprites_tab_start[1].width / 2 + resvars.f3c_PolicyRectp[0x26].w), lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 1;
                D_001549d0[player].arg = 100;
                pol.unknown0 = 1;
            }
        }
        if (pol.unknown0 == 3) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, screen_sprites_tab_start[3].width / 2 + resvars.f3c_PolicyRectp[0x27].x, 0,
                             screen_sprites_tab_start[3].width / 2 + resvars.f3c_PolicyRectp[0x27].w, lbDisplay.graphics_screen_height)) {
                x = (lbDisplay.mmouse_x - screen_sprites_tab_start[3].width / 2 - resvars.f3c_PolicyRectp[0x27].x) / 20 * 0x42;
                if (x < 0)
                    x = 0;
                else if (x > 1000)
                    x = 1000;
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 3;
                D_001549d0[player].arg = x + 1000;
                pol.unknown0 = 3;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, 0, resvars.f3c_PolicyRectp[0x27].x, lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 3;
                D_001549d0[player].arg = 1000;
                pol.unknown0 = 3;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f3c_PolicyRectp[0x27].x + resvars.f3c_PolicyRectp[0x27].w + screen_sprites_tab_start[3].width / 2, 0,
                             lbDisplay.graphics_screen_width - (screen_sprites_tab_start[3].width / 2 + resvars.f3c_PolicyRectp[0x27].w), lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 3;
                D_001549d0[player].arg = 2000;
                pol.unknown0 = 3;
            }
        }
        if (pol.unknown0 == 4) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, screen_sprites_tab_start[3].width / 2 + resvars.f3c_PolicyRectp[0x28].x, 0,
                             screen_sprites_tab_start[3].width / 2 + resvars.f3c_PolicyRectp[0x28].w, lbDisplay.graphics_screen_height)) {
                x = (lbDisplay.mmouse_x - screen_sprites_tab_start[3].width / 2 - resvars.f3c_PolicyRectp[0x28].x) / 10 * 0x27;
                if (x < 0)
                    x = 0;
                else if (x > 1000)
                    x = 1000;
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 4;
                D_001549d0[player].arg = x;
                pol.unknown0 = 4;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, 0, resvars.f3c_PolicyRectp[0x28].x, lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 4;
                D_001549d0[player].arg = 0;
                pol.unknown0 = 4;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f3c_PolicyRectp[0x28].x + resvars.f3c_PolicyRectp[0x28].w + screen_sprites_tab_start[3].width / 2, 0,
                             lbDisplay.graphics_screen_width - (screen_sprites_tab_start[3].width / 2 + resvars.f3c_PolicyRectp[0x28].w), lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = 0x71;
                D_001549d0[player].value = 4;
                D_001549d0[player].arg = 1000;
                pol.unknown0 = 4;
            }
        }
    } else {
        pol.unknown0 = 0;
    }
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
}

}
