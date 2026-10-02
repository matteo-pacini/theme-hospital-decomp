#include "hosp.h"

struct PolicyRect {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

extern "C" {

extern Event D_001549d0[];
extern char **policy_names;

void process_policy_panel(void);
void process_policy_sliders(void);
}

// Runs one frame of the policy screen (pol.state: 0 opening, 1 shown, 2-3
// closing, which queues EV_RETURN_FROM_SCREEN).
// FUNCTION: 0x00044530
void display_policy(void)
{
    switch (pol.state) {
    case 0:
        load_ingame_pic(SCR_POLICY, 1, 0x1c, 0, SCREEN_MODE_13);
        pol.state = 1;
        pol.value = 0;
        lbDisplay.draw_flags = 0;
        process_policy_sliders();
        lbDisplay.left_button = lbDisplay.mleft_button = lbDisplay.rleft_button = 0;
        break;
    case 1:
        lbDisplay.draw_flags = 0;
        copy_screen_to_wscreen(bscreen);
        process_policy_panel();
        process_policy_sliders();
        break;
    case 2:
        pol.state = 3;
        break;
    case 3:
        D_001549d0[player].type = EV_RETURN_FROM_SCREEN;
        D_001549d0[player].value = 0;
        break;
    }
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    if (exit_quick_requester()) {
        pol.state = 2;
    }
    compute_help_text(0, 0, 0x19);
}

extern "C" {

// Policy screen clicks: runs the callback of the resvars.screen_buttons button released
// over.
// FUNCTION: 0x00044664
void process_policy_panel(void)
{
    u8 i;

    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.screen_buttons[i + 10].width != 0 && resvars.screen_buttons[i + 10].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_buttons[i + 10].x,
                             resvars.screen_buttons[i + 10].y, (s16)resvars.screen_buttons[i + 10].width,
                             (s16)resvars.screen_buttons[i + 10].height)) {
                resvars.screen_buttons[i + 10].callback(i);
            }
        }
        lbDisplay.rleft_button = 0;
    }
}

// Policy screen button `action`: 0 and 1 set the staff pager option
// (allhosps.staff_leave_rooms) on and off through event EV_SET_POLICY, 2 closes the screen.
// FUNCTION: 0x0004477c
void process_policy_button(u8 action)
{
    debug.last_action = action;
    switch (action) {
    case 0:
        D_001549d0[player].type = EV_SET_POLICY;
        D_001549d0[player].value = 5;
        D_001549d0[player].arg = 1;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    case 1:
        D_001549d0[player].type = EV_SET_POLICY;
        D_001549d0[player].value = 5;
        D_001549d0[player].arg = 0;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    case 2:
        pol.state = 2;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    }
}

// Draws the policy sliders at this player's settings (allhosps.guess_cure_threshold and
// f21b 0-100, f217 1000-2000, f21d 0-1000 rest threshold) and the pager
// option; dragging a slider sends its new value as event EV_SET_POLICY (value =
// slider 1-4).
// FUNCTION: 0x0004488c
void process_policy_sliders(void)
{
    u16 i;
    s32 x;
    char buf[200];

    lbDisplay.draw_flags = 0x100;
    set_font_from_cache(7);
    for (i = 0; i < 5; i++) {
        LbTextSetWindow(resvars.screen_boxes_PolicyRectp[i + 0x20].x, resvars.screen_boxes_PolicyRectp[i + 0x20].y,
                        resvars.screen_boxes_PolicyRectp[i + 0x20].w, resvars.screen_boxes_PolicyRectp[i + 0x20].h);
        sprintf(buf, "%s", policy_names[i]);
        LbTextDraw(0, 0, buf);
        lbDisplay.draw_flags = 0x20;
    }
    set_font_from_cache(8);
    lbDisplay.draw_flags = 0x100;
    x = allhosps[player].guess_cure_threshold * 3;
    if (x < 0)
        x = 0;
    else if (x > 299)
        x = 299;
    reqspr_draw(resvars.screen_boxes_PolicyRectp[0x25].x + x, resvars.screen_boxes_PolicyRectp[0x25].y,
                &screen_sprites_tab_start[2]);
    LbTextSetWindow((resvars.screen_boxes_PolicyRectp[0x25].x + x) + resvars.screen_coords[0x54].x,
                    resvars.screen_boxes_PolicyRectp[0x25].y + resvars.screen_coords[0x54].y,
                    screen_sprites_tab_start[2].width, screen_sprites_tab_start[2].height);
    sprintf(buf, "%s", policy_names[i++]);
    LbTextDraw(0, 0, buf);
    x = allhosps[player].send_home_threshold * 3;
    if (x < 0)
        x = 0;
    else if (x > 299)
        x = 299;
    reqspr_draw(resvars.screen_boxes_PolicyRectp[0x26].x + x, resvars.screen_boxes_PolicyRectp[0x26].y,
                &screen_sprites_tab_start[1]);
    LbTextSetWindow((resvars.screen_boxes_PolicyRectp[0x26].x + x) + resvars.screen_coords[0x54].x,
                    resvars.screen_boxes_PolicyRectp[0x26].y + resvars.screen_coords[0x54].y,
                    screen_sprites_tab_start[1].width, screen_sprites_tab_start[1].height);
    sprintf(buf, "%s", policy_names[i++]);
    LbTextDraw(0, 0, buf);
    x = (allhosps[player].diag_threshold - 1000) / 0x42 * 20;
    if (x < 0)
        x = 0;
    else if (x > 299)
        x = 299;
    reqspr_draw(resvars.screen_boxes_PolicyRectp[0x27].x + x, resvars.screen_boxes_PolicyRectp[0x27].y,
                &screen_sprites_tab_start[3]);
    LbTextSetWindow((resvars.screen_boxes_PolicyRectp[0x27].x + x) + resvars.screen_coords[0x54].x,
                    resvars.screen_boxes_PolicyRectp[0x27].y + resvars.screen_coords[0x54].y,
                    screen_sprites_tab_start[3].width, screen_sprites_tab_start[3].height);
    sprintf(buf, "%s", policy_names[i++]);
    LbTextDraw(0, 0, buf);
    x = allhosps[player].rest_threshold / 0x27 * 10;
    if (x < 0)
        x = 0;
    else if (x > 0xfb)
        x = 0xfb;
    reqspr_draw(resvars.screen_boxes_PolicyRectp[0x28].x + x, resvars.screen_boxes_PolicyRectp[0x28].y,
                &screen_sprites_tab_start[3]);
    LbTextSetWindow((resvars.screen_boxes_PolicyRectp[0x28].x + x) + resvars.screen_coords[0x54].x,
                    resvars.screen_boxes_PolicyRectp[0x28].y + resvars.screen_coords[0x54].y,
                    screen_sprites_tab_start[3].width, screen_sprites_tab_start[3].height);
    sprintf(buf, "%s", policy_names[i++]);
    LbTextDraw(0, 0, buf);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    if (allhosps[player].staff_leave_rooms)
        reqspr_draw(resvars.screen_coords[0x52].x, resvars.screen_coords[0x52].y, &screen_sprites_tab_start[4]);
    else
        reqspr_draw(resvars.screen_coords[0x53].x, resvars.screen_coords[0x53].y, &screen_sprites_tab_start[5]);
    if (lbDisplay.mleft_button) {
        if (pol.value == 0) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, resvars.screen_boxes_PolicyRectp[0x25].y,
                             lbDisplay.graphics_screen_width, resvars.screen_boxes_PolicyRectp[0x25].h))
                pol.value = 2;
            else if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, resvars.screen_boxes_PolicyRectp[0x26].y,
                                  lbDisplay.graphics_screen_width, resvars.screen_boxes_PolicyRectp[0x26].h))
                pol.value = 1;
            else if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, resvars.screen_boxes_PolicyRectp[0x27].y,
                                  lbDisplay.graphics_screen_width, resvars.screen_boxes_PolicyRectp[0x27].h))
                pol.value = 3;
            else if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, resvars.screen_boxes_PolicyRectp[0x28].y,
                                  lbDisplay.graphics_screen_width, resvars.screen_boxes_PolicyRectp[0x28].h))
                pol.value = 4;
            else
                pol.value = 0x63;
        }
        if (pol.value == 2) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                             screen_sprites_tab_start[2].width / 2 + resvars.screen_boxes_PolicyRectp[0x25].x, 0,
                             screen_sprites_tab_start[2].width / 2 + resvars.screen_boxes_PolicyRectp[0x25].w,
                             lbDisplay.graphics_screen_height)) {
                x = (lbDisplay.mmouse_x - screen_sprites_tab_start[2].width / 2 -
                     resvars.screen_boxes_PolicyRectp[0x25].x) /
                    3;
                if (x < 0)
                    x = 0;
                else if (x > 100)
                    x = 100;
                D_001549d0[player].type = EV_SET_POLICY;
                D_001549d0[player].value = 2;
                D_001549d0[player].arg = x;
                pol.value = 2;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, 0, resvars.screen_boxes_PolicyRectp[0x25].x,
                             lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = EV_SET_POLICY;
                D_001549d0[player].value = 2;
                D_001549d0[player].arg = 0;
                pol.value = 2;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                             resvars.screen_boxes_PolicyRectp[0x25].x + resvars.screen_boxes_PolicyRectp[0x25].w +
                                 screen_sprites_tab_start[2].width / 2,
                             0,
                             lbDisplay.graphics_screen_width -
                                 (screen_sprites_tab_start[2].width / 2 + resvars.screen_boxes_PolicyRectp[0x25].w),
                             lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = EV_SET_POLICY;
                D_001549d0[player].value = 2;
                D_001549d0[player].arg = 100;
                pol.value = 2;
            }
        }
        if (pol.value == 1) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                             screen_sprites_tab_start[1].width / 2 + resvars.screen_boxes_PolicyRectp[0x26].x, 0,
                             screen_sprites_tab_start[1].width / 2 + resvars.screen_boxes_PolicyRectp[0x26].w,
                             lbDisplay.graphics_screen_height)) {
                x = (lbDisplay.mmouse_x - screen_sprites_tab_start[1].width / 2 -
                     resvars.screen_boxes_PolicyRectp[0x26].x) /
                    3;
                if (x < 0)
                    x = 0;
                else if (x > 100)
                    x = 100;
                D_001549d0[player].type = EV_SET_POLICY;
                D_001549d0[player].value = 1;
                D_001549d0[player].arg = x;
                pol.value = 1;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, 0, resvars.screen_boxes_PolicyRectp[0x26].x,
                             lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = EV_SET_POLICY;
                D_001549d0[player].value = 1;
                D_001549d0[player].arg = 0;
                pol.value = 1;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                             resvars.screen_boxes_PolicyRectp[0x26].x + resvars.screen_boxes_PolicyRectp[0x26].w +
                                 screen_sprites_tab_start[1].width / 2,
                             0,
                             lbDisplay.graphics_screen_width -
                                 (screen_sprites_tab_start[1].width / 2 + resvars.screen_boxes_PolicyRectp[0x26].w),
                             lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = EV_SET_POLICY;
                D_001549d0[player].value = 1;
                D_001549d0[player].arg = 100;
                pol.value = 1;
            }
        }
        if (pol.value == 3) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                             screen_sprites_tab_start[3].width / 2 + resvars.screen_boxes_PolicyRectp[0x27].x, 0,
                             screen_sprites_tab_start[3].width / 2 + resvars.screen_boxes_PolicyRectp[0x27].w,
                             lbDisplay.graphics_screen_height)) {
                x = (lbDisplay.mmouse_x - screen_sprites_tab_start[3].width / 2 -
                     resvars.screen_boxes_PolicyRectp[0x27].x) /
                    20 * 0x42;
                if (x < 0)
                    x = 0;
                else if (x > 1000)
                    x = 1000;
                D_001549d0[player].type = EV_SET_POLICY;
                D_001549d0[player].value = 3;
                D_001549d0[player].arg = x + 1000;
                pol.value = 3;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, 0, resvars.screen_boxes_PolicyRectp[0x27].x,
                             lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = EV_SET_POLICY;
                D_001549d0[player].value = 3;
                D_001549d0[player].arg = 1000;
                pol.value = 3;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                             resvars.screen_boxes_PolicyRectp[0x27].x + resvars.screen_boxes_PolicyRectp[0x27].w +
                                 screen_sprites_tab_start[3].width / 2,
                             0,
                             lbDisplay.graphics_screen_width -
                                 (screen_sprites_tab_start[3].width / 2 + resvars.screen_boxes_PolicyRectp[0x27].w),
                             lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = EV_SET_POLICY;
                D_001549d0[player].value = 3;
                D_001549d0[player].arg = 2000;
                pol.value = 3;
            }
        }
        if (pol.value == 4) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                             screen_sprites_tab_start[3].width / 2 + resvars.screen_boxes_PolicyRectp[0x28].x, 0,
                             screen_sprites_tab_start[3].width / 2 + resvars.screen_boxes_PolicyRectp[0x28].w,
                             lbDisplay.graphics_screen_height)) {
                x = (lbDisplay.mmouse_x - screen_sprites_tab_start[3].width / 2 -
                     resvars.screen_boxes_PolicyRectp[0x28].x) /
                    10 * 0x27;
                if (x < 0)
                    x = 0;
                else if (x > 1000)
                    x = 1000;
                D_001549d0[player].type = EV_SET_POLICY;
                D_001549d0[player].value = 4;
                D_001549d0[player].arg = x;
                pol.value = 4;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, 0, 0, resvars.screen_boxes_PolicyRectp[0x28].x,
                             lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = EV_SET_POLICY;
                D_001549d0[player].value = 4;
                D_001549d0[player].arg = 0;
                pol.value = 4;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                             resvars.screen_boxes_PolicyRectp[0x28].x + resvars.screen_boxes_PolicyRectp[0x28].w +
                                 screen_sprites_tab_start[3].width / 2,
                             0,
                             lbDisplay.graphics_screen_width -
                                 (screen_sprites_tab_start[3].width / 2 + resvars.screen_boxes_PolicyRectp[0x28].w),
                             lbDisplay.graphics_screen_height)) {
                D_001549d0[player].type = EV_SET_POLICY;
                D_001549d0[player].value = 4;
                D_001549d0[player].arg = 1000;
                pol.value = 4;
            }
        }
    } else {
        pol.value = 0;
    }
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
}
}
