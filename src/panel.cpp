#include "hosp.h"

/*
 * The panel's entries of the in-game box table (resvars.boxes) and position
 * table (resvars.coords), both read through this struct.
 */
struct PanelLayout {
    u8 unknown0[0x150];
    // 0x150: Box: clip window of the balance's rolling digits.
    s32 balance_clip_x;
    // 0x154: Its y.
    s32 balance_clip_y;
    // 0x158: Its width.
    s32 balance_clip_w;
    u8 unknown15c[4];
    // 0x160: Box: clip window of the balance digits that stay still while the
    // others roll.
    s32 balance_fixed_clip_x;
    // 0x164: Its y.
    s32 balance_fixed_clip_y;
    // 0x168: Its width before the extra digits.
    s32 balance_fixed_clip_w;
    u8 unknown16c[0xd4];
    // 0x240: Position: the advisor's x.
    s32 advisor_x;
    u8 unknown244[4];
    // 0x248: Position: width of a balance digit.
    s32 balance_digit_width;
    u8 unknown24c[4];
    // 0x250: Position: balance text x.
    s32 balance_x;
    // 0x254: Balance text y.
    s32 balance_y;
    // 0x258: Position: reputation scale x at 0.
    s32 rep_scale_x0;
    // 0x25c: Reputation marker y.
    s32 rep_marker_y;
    // 0x260: Reputation scale x at 1000.
    s32 rep_scale_x1;
    u8 unknown264[0x1bc];
    // 0x420: Position: the panel watch.
    s32 watch_x;
    // 0x424: Watch y.
    s32 watch_y;
    u8 unknown428[8];
    // 0x430: Position: the vaccinate or announce button by the watch.
    s32 watch_button_x;
    // 0x434: Its y.
    s32 watch_button_y;
    // 0x438: Offset of the watch face from the watch.
    s32 watch_face_dx;
    // 0x43c: Its y offset.
    s32 watch_face_dy;
    // 0x440: Position: the date text.
    s32 date_x;
    // 0x444: Its y.
    s32 date_y;
    // 0x448: Positions: fax icon slots.
    Point slots[9];
    u8 unknown490[0x10];
    // 0x4a0: Position: start of the message bar shutter.
    s32 shutter_x;
    // 0x4a4: Its y.
    s32 shutter_y;
    // 0x4a8: Added to a slot's x for its fax icon.
    s32 popup_icon_dx;
    // 0x4ac: Added to the top of the fax icons' clip window.
    s32 popup_window_dy;
    u8 unknown4b0[0x10];
    // 0x4c0: Box: text window of the date.
    s32 date_box_x;
    // 0x4c4: Its y.
    s32 date_box_y;
    // 0x4c8: Its width.
    s32 date_box_w;
    // 0x4cc: Its height.
    s32 date_box_h;
    u8 unknown4d0[0xa0];
    // 0x570: Position: info bar text x, also the margin on each side when
    // wrapping.
    s32 info_text_x;
    u8 unknown574[8];
    // 0x57c: Info bar line height.
    s32 info_line_height;
    u8 unknown580[4];
    // 0x584: Info bar first line y.
    s32 info_text_y;
    u8 unknown588[0xc];
    // 0x594: Extra y of the paused or panel message line.
    s32 info_message_dy;
    // 0x598: Offset of the emergency animation on its fax icon.
    s32 emergency_anim_dx;
    // 0x59c: Its y offset.
    s32 emergency_anim_dy;
    u8 unknown5a0[0x40];
    // 0x5e0: Box: percentage bar on the info bar's third line (x relative to
    // the panel).
    s32 info_bar_x;
    // 0x5e4: Its y below the third line.
    s32 info_bar_y;
    // 0x5e8: Its width.
    s32 info_bar_w;
    // 0x5ec: Its height.
    s32 info_bar_h;
};

void draw_panel(u8 all);
void choose_panel_icons(void);
void draw_money_on_panel(void);
void draw_date_on_panel(void);
void draw_rep_on_panel(void);
void panel_flash(void);
void draw_ratfest_on_panel(void);
u8 display_panel_special_info(void);
void display_panel_person_info(void);
void display_panel_staff_info(void);
void display_panel_object_info(void);
void display_panel_queue_info(void);
void display_panel_mayor_info(void);
void display_panel_usage_info(void);
void maintain_panel_popup(void);

extern Icons panel_icons_over[];
extern Icons panel_icons_normal[];
extern Icons panel_icons_ratfest[];
extern PopupIconControl popup_icons[];
extern u8 panel_emergency_spr[];

extern "C" {

extern char **room_names;
extern char **object_names;
extern Event D_001549d0[];
extern u8 redraw_panel;
extern TbSprite *game_panel_data;
extern TbSprite *end_game_panel;
extern char **misc_text;
extern char **staff_names;
extern char **skill_names;
extern char **illness_text;

void strcat(char *dst, char *src);
void LbDataLoadAll(TbLoadFiles *files);
void rebuild_panel_popup_pos(void);
void draw_panel_percent_bar(s32 value, char *text);
void draw_panel_info_line(char *text, u8 *line, u8 lines);
void LbSpriteReset(TbSprite *start, TbSprite *end, void *data);
void LbTextSetClipWindow(s32 x, s32 y, s32 width, s32 height);
void LbDrawHVLine(s32 x0, s32 y0, s32 x1, s32 y1, u8 colour);
#pragma aux LbDrawHVLine "LbDrawHVLine_";
}

extern "C" {
// demo: panel_popup_coord
Coord panel_popup_coord[40] = {
    {0, 0},    {174, 29}, {174, 29}, {174, 29}, {174, 29}, {174, 29}, {174, 29}, {174, 29}, {174, 29}, {174, 29},
    {174, 29}, {174, 29}, {174, 29}, {174, 25}, {174, 20}, {174, 14}, {174, 7},  {174, 0},  {174, 0},  {174, 0},
    {174, 0},  {174, 0},  {174, 0},  {170, 0},  {165, 0},  {153, 0},  {140, 0},  {125, 0},  {113, 0},  {93, 0},
    {73, 0},   {53, 0},   {33, 0},   {1, 0},    {1, 0},    {2, 0},    {4, 0},    {4, 0},    {2, 0},    {0, 0},
};
}

// Per-frame bottom panel update: redraws it when the mouse moves on or off
// it or the balance changes, then fills the info bar for what the pointer is
// over (othervar.panel_info: 1 staff, 2 patient, 3 object, 4 queue, 5 VIP or
// inspector, 6 machine use).
// FUNCTION: 0x00031c70
void update_panel(void)
{
    u8 drawn;

    drawn = 0;
    switch (othervar.panel_icon_set) {
    case 1:
        if (statvar.panel_top <= lbDisplay.mmouse_y && !gamevar.rat_fest_level)
            redraw_panel = 1;
        break;
    case 2:
        if (statvar.panel_top > lbDisplay.mmouse_y || gamevar.rat_fest_level)
            redraw_panel = 1;
        break;
    }
    if (othervar.shown_balance != myhosp->balance)
        redraw_panel = 1;
    if (redraw_panel == 1) {
        choose_panel_icons();
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        redraw_panel = 0;
        drawn = 1;
        draw_panel(1);
        set_all_refresh_zones_in_window(0, statvar.view_height, lbDisplay.physical_screen_width,
                                        lbDisplay.physical_screen_height, 3);
    }
    if (!gamevar.rat_fest_level) {
        if (!display_panel_special_info() && othervar.panel_icon_set == 1) {
            switch (othervar.panel_info) {
            case 0:
                if (othervar.prev_panel_info != 0 || drawn)
                    clear_panel_info_bar();
                break;
            case 2:
                display_panel_person_info();
                break;
            case 1:
                display_panel_staff_info();
                break;
            case 3:
                display_panel_object_info();
                break;
            case 4:
                display_panel_queue_info();
                break;
            case 5:
                display_panel_mayor_info();
                break;
            case 6:
                display_panel_usage_info();
                break;
            }
        }
    }
}

// Draws the panel's icons with the balance, date and reputation (only when
// all is 1 or a full-screen window is shown), or the rat game's panel.
// FUNCTION: 0x00031e20
void draw_panel(u8 all)
{
    s32 unused;
    s32 i;

    unused = 0;
    if (gamevar.rat_fest_level) {
        for (i = 0; ((PanelIcon *)panel)[i].sprite > 0; i++)
            reqspr_draw(((PanelIcon *)panel)[i].x, ((PanelIcon *)panel)[i].y,
                        &game_panel[((PanelIcon *)panel)[i].sprite]);
        draw_ratfest_on_panel();
    } else if (all == 1 || display_mode != DM_GAME) {
        for (i = 0; ((PanelIcon *)panel)[i].sprite > 0; i++)
            reqspr_draw(((PanelIcon *)panel)[i].x, ((PanelIcon *)panel)[i].y,
                        &game_panel[((PanelIcon *)panel)[i].sprite]);
        draw_money_on_panel();
        draw_date_on_panel();
        draw_rep_on_panel();
    }
    panel_flash();
}

// Chooses the panel icon set: raised while the mouse is over the panel
// (othervar.panel_icon_set 2), normal otherwise (1), or the rat game's.
// FUNCTION: 0x00031f5c
void choose_panel_icons(void)
{
    if (statvar.panel_top <= lbDisplay.mmouse_y && !gamevar.rat_fest_level && !gamevar.paused) {
        panel = panel_icons_over;
        othervar.panel_icon_set = 2;
    } else {
        panel = panel_icons_normal;
        othervar.panel_icon_set = 1;
    }
    if (gamevar.rat_fest_level)
        panel = panel_icons_ratfest;
    statvar.panel_top = ((PanelIcon *)panel)[1].y;
    advisor.x_u = resvars.coords_PanelLayoutp->advisor_x;
    advisor.y_u = statvar.panel_top;
}

// Lays out the panel icon sets along the bottom of screen mode `mode`.
// FUNCTION: 0x00031ffc
void setup_panel_icons(const u8 mode)
{
    u8 i;
    u16 x;
    s8 pass;

    i = 0;
    x = 0;
    pass = 2;
    if (!gamevar.rat_fest_level) {
        panel = panel_icons_over;
    } else {
        panel = panel_icons_ratfest;
        --pass;
    }
    while (--pass >= 0) {
        if (pass == 0 && !gamevar.rat_fest_level) {
            panel = panel_icons_normal;
            x = 0;
            i = 0;
        }
        while (((PanelIcon *)panel)[i].sprite > 0) {
            ((PanelIcon *)panel)[i].y =
                lbScreenModeInfo[mode].height - game_panel[((PanelIcon *)panel)[i].sprite].height;
            ((PanelIcon *)panel)[i].y2 = lbScreenModeInfo[mode].height;
            ((PanelIcon *)panel)[i].x = x;
            ((PanelIcon *)panel)[i].x2 = x + game_panel[((PanelIcon *)panel)[i].sprite].width;
            x += game_panel[((PanelIcon *)panel)[i].sprite].width;
            i++;
        }
    }
    othervar.panel_info_hold = 0;
    othervar.popup_shutter_state = 1;
}

// Returns the index of the panel icon under the mouse, or 0x63 if the mouse
// is not over the panel.
// FUNCTION: 0x000321b4
u8 which_icon_is_mouse_over(void)
{
    u8 i;
    u8 over;

    i = 0;
    over = 0x63;
    if (statvar.panel_top <= lbDisplay.mmouse_y && !gamevar.rat_fest_level) {
        while (((PanelIcon *)panel)[i].sprite > 0) {
            if (((PanelIcon *)panel)[i].x <= lbDisplay.mmouse_x && ((PanelIcon *)panel)[i].x2 >= lbDisplay.mmouse_x) {
                over = i;
                break;
            }
            i++;
        }
        redraw_panel = 1;
    }
    return over;
}

// Draws panel icon `icon` pressed while the button is held (0x63: none).
// FUNCTION: 0x00032268
void panel_ml_button(const u16 icon)
{
    if (icon != 0x63) {
        if (((PanelIcon *)panel)[icon].kind != 0x40) {
            reqspr_draw(((PanelIcon *)panel)[icon].x, ((PanelIcon *)panel)[icon].y,
                        &game_panel[((PanelIcon *)panel)[icon].pressed]);
            redraw_panel = 1;
        }
    }
}

// Left click on panel icon `icon` (0x63: none): opens the screen, requester
// or mode it stands for (0 bank manager, 2, 3 and 5 the requesters of events
// 0x37, 0x62 and 0x2e, 4 room editing on/off, 6 staff list, 7 town map,
// 8 casebook, 9 research, 10 status, 11 charts, 12 policy). Refused while
// placing a room, door or staff member.
// FUNCTION: 0x00032308
void panel_l_button(const u16 icon)
{
    if (gamevar.mouse_mode == MM_EDIT_ROOM_WAIT) {
        play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
    } else if (icon != 0x63) {
        switch (icon) {
        case 0:
            D_001549d0[player].type = EV_OPEN_SCREEN;
            D_001549d0[player].value = 6;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            break;
        case 2:
            if (!(gamevar.mouse_mode == MM_BLUEPRINT || gamevar.mouse_mode == MM_PLACE_DOOR ||
                  gamevar.mouse_mode == MM_PLACE_STAFF || gamevar.mouse_mode == MM_EDIT_ROOM_WAIT ||
                  gamevar.mouse_mode == 0x10)) {
                set_mouse_mode(MM_NORMAL);
                D_001549d0[player].type = EV_OPEN_BUILD_ROOM;
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
            break;
        case 5:
            if (!(gamevar.mouse_mode == MM_BLUEPRINT || gamevar.mouse_mode == MM_PLACE_DOOR ||
                  gamevar.mouse_mode == MM_PLACE_STAFF || gamevar.mouse_mode == MM_EDIT_ROOM_WAIT ||
                  gamevar.mouse_mode == 0x10)) {
                set_mouse_mode(MM_NORMAL);
                D_001549d0[player].type = EV_OPEN_HIRE_STAFF;
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
            break;
        case 4:
            if (gamevar.mouse_mode == MM_NORMAL && crequester.id == REQ_NONE) {
                set_mouse_mode(MM_EDIT_ROOM);
                allhosps[player].edited_room = 0;
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            } else if (gamevar.mouse_mode == MM_EDIT_ROOM && crequester.id == REQ_NONE) {
                set_mouse_mode(MM_NORMAL);
                allhosps[player].edited_room = 0;
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
            break;
        case 3:
            if (!(gamevar.mouse_mode == MM_BLUEPRINT || gamevar.mouse_mode == MM_PLACE_DOOR ||
                  gamevar.mouse_mode == MM_PLACE_STAFF || gamevar.mouse_mode == MM_EDIT_ROOM_WAIT ||
                  gamevar.mouse_mode == 0x10)) {
                set_mouse_mode(MM_NORMAL);
                D_001549d0[player].type = EV_OPEN_FURNISH_CORRIDOR;
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
            break;
        case 6:
            D_001549d0[player].type = EV_OPEN_SCREEN;
            D_001549d0[player].value = 5;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            break;
        case 7:
            if (crequester.id != REQ_EDIT_ROOM && (crequester.id != REQ_BUY_OBJECTS || crequester.arg == 1)) {
                display_mode = DM_TOWN_MAP;
                town.state = 0;
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
            break;
        case 8:
            D_001549d0[player].type = EV_OPEN_SCREEN;
            D_001549d0[player].value = 1;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            break;
        case 9:
            if (listhead[player].rooms[ROOM_RESEARCH]) {
                D_001549d0[player].type = EV_OPEN_SCREEN;
                D_001549d0[player].value = 3;
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
            break;
        case 10:
            D_001549d0[player].type = EV_OPEN_SCREEN;
            D_001549d0[player].value = 7;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            break;
        case 11:
            D_001549d0[player].type = EV_OPEN_SCREEN;
            D_001549d0[player].value = 4;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            break;
        case 12:
            D_001549d0[player].type = EV_OPEN_SCREEN;
            D_001549d0[player].value = 2;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            break;
        case 1:
        default:
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            break;
        }
        redraw_panel = 1;
    }
}

// Right click on panel icon `icon`: 0 opens the bank statement, 5 the staff
// list.
// FUNCTION: 0x000328c0
void panel_r_button(const u16 icon)
{
    if (gamevar.mouse_mode == MM_EDIT_ROOM_WAIT) {
        play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
    } else if (icon != 0x63) {
        switch (icon) {
        case 0:
            D_001549d0[player].type = EV_OPEN_SCREEN;
            D_001549d0[player].value = 0;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            break;
        case 5:
            D_001549d0[player].type = EV_OPEN_SCREEN;
            D_001549d0[player].value = 5;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            break;
        default:
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            break;
        }
        redraw_panel = 1;
    }
}

// Draws this player's balance on the panel, rolling the changed digits from
// the shown value (othervar.shown_balance) to the new one over four frames.
// FUNCTION: 0x000329d4
void draw_money_on_panel(void)
{
    u8 digits;
    u8 height;
    u32 diff;
    s32 div;
    char buf[16];

    digits = 0;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetClipWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    set_font_from_cache(5);
    height = lbFontPtr ? lbFontPtr[18].height : 0;
    if (othervar.shown_balance != myhosp->balance) {
        diff = abs(othervar.shown_balance - myhosp->balance);
        for (div = 1000000; div >= 0; div /= 10) {
            if (diff >= div) {
                digits++;
                break;
            }
        }
        LbTextSetClipWindow(
            resvars.boxes_PanelLayoutp->balance_clip_x + resvars.coords_PanelLayoutp->balance_digit_width * digits,
            resvars.boxes_PanelLayoutp->balance_clip_y,
            resvars.boxes_PanelLayoutp->balance_clip_w - resvars.coords_PanelLayoutp->balance_digit_width * digits,
            height);
        sprintf(buf, "%7d", myhosp->balance);
        LbTextDraw(resvars.coords_PanelLayoutp->balance_x,
                   resvars.coords_PanelLayoutp->balance_y - (height - othervar.balance_roll_step * 6), buf);
        sprintf(buf, "%7d", othervar.shown_balance);
        LbTextDraw(resvars.coords_PanelLayoutp->balance_x,
                   resvars.coords_PanelLayoutp->balance_y + othervar.balance_roll_step * 6, buf);
        if (digits) {
            LbTextSetClipWindow(resvars.boxes_PanelLayoutp->balance_fixed_clip_x,
                                resvars.boxes_PanelLayoutp->balance_fixed_clip_y,
                                resvars.coords_PanelLayoutp->balance_digit_width * (digits - 3) +
                                    resvars.boxes_PanelLayoutp->balance_fixed_clip_w,
                                height);
            sprintf(buf, "%7d", myhosp->balance);
            LbTextDraw(resvars.coords_PanelLayoutp->balance_x, resvars.coords_PanelLayoutp->balance_y, buf);
        }
        othervar.balance_roll_step++;
        if (othervar.balance_roll_step == 4) {
            othervar.balance_roll_step = 0;
            othervar.shown_balance = myhosp->balance;
            redraw_panel = 1;
        }
    } else {
        LbTextSetClipWindow(resvars.boxes_PanelLayoutp->balance_clip_x, resvars.boxes_PanelLayoutp->balance_clip_y,
                            resvars.boxes_PanelLayoutp->balance_clip_w, height);
        sprintf(buf, "%7d", myhosp->balance);
        LbTextDraw(resvars.coords_PanelLayoutp->balance_x, resvars.coords_PanelLayoutp->balance_y, buf);
    }
}

// Draws the date (day and month) on the panel.
// FUNCTION: 0x00032cc4
void draw_date_on_panel(void)
{
    char buf[12];

    set_font_from_cache(6);
    lbDisplay.draw_flags = 0x100;
    LbTextSetWindow(resvars.boxes_PanelLayoutp->date_box_x, resvars.boxes_PanelLayoutp->date_box_y,
                    resvars.boxes_PanelLayoutp->date_box_w, resvars.boxes_PanelLayoutp->date_box_h);
    sprintf(buf, "%d %s", mycontrol->day, month_text[mycontrol->month]);
    LbTextDraw(resvars.coords_PanelLayoutp->date_x, resvars.coords_PanelLayoutp->date_y, buf);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
}

// Draws the reputation marker on the panel's scale (reputation 0-1000).
// FUNCTION: 0x00032da0
void draw_rep_on_panel(void)
{
    s32 offset;

    offset = (resvars.coords_PanelLayoutp->rep_scale_x1 - resvars.coords_PanelLayoutp->rep_scale_x0) *
             myhosp->reputation / 1000;
    lbDisplay.draw_flags = 0;
    reqspr_draw(resvars.coords_PanelLayoutp->rep_scale_x0 + offset, resvars.coords_PanelLayoutp->rep_marker_y,
                &game_panel[36]);
}

// Draws the panel's watch for an emergency, an epidemic (myhosp->epidemic_timer) or
// the hospital's opening countdown (myhosp->opening_countdown), with its time
// left and the button for vaccinating (epidemic) or opening the hospital at
// once (countdown).
// FUNCTION: 0x00032e20
void draw_emergency(void)
{
    u8 height;
    u8 off;

    off = 0;
    if (!gamevar.rat_fest_level) {
        lbDisplay.draw_flags = 0;
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        set_font_from_cache(6);
        height = lbFontPtr ? lbFontPtr[25].height : 0;
        if (myhosp->emergency_timer || (myhosp->opening_countdown && !(advisor.flags & 1)) || myhosp->epidemic_timer ||
            myhosp->epidemic_over || (s8)myhosp->emergency_state > 1) {
            reqspr_draw(resvars.coords_PanelLayoutp->watch_x, resvars.coords_PanelLayoutp->watch_y, &watch[13]);
            set_all_refresh_zones_in_window(resvars.coords_PanelLayoutp->watch_x, resvars.coords_PanelLayoutp->watch_y,
                                            watch[13].width + resvars.coords_PanelLayoutp->watch_x,
                                            resvars.coords_PanelLayoutp->watch_y + watch[13].height, 3);
            if (myhosp->emergency_timer) {
                if ((s16)myhosp->emergency_timer / 8 >= 6) {
                    reqspr_draw(resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx,
                                resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy,
                                &watch[1]);
                    set_all_refresh_zones_in_window(
                        resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx,
                        resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy,
                        resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx +
                            watch[1].width,
                        resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy +
                            watch[1].height,
                        3);
                }
                reqspr_draw(resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx,
                            resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy,
                            &watch[watch_face[(s16)myhosp->emergency_timer / 8]]);
                set_all_refresh_zones_in_window(
                    resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx,
                    resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy,
                    (resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx) +
                        watch[watch_face[(s16)myhosp->emergency_timer / 8]].width,
                    (resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy) +
                        watch[watch_face[(s16)myhosp->emergency_timer / 8]].height,
                    3);
            } else if (myhosp->epidemic_timer) {
                if (myhosp->emergency_timer)
                    off = watch[15].width / 4 + watch[15].width;
                if (myhosp->epidemic_illness && !myhosp->epidemic_timer)
                    lbDisplay.draw_flags = 4;
                reqspr_draw(off + resvars.coords_PanelLayoutp->watch_button_x,
                            resvars.coords_PanelLayoutp->watch_button_y, &watch[14]);
                set_all_refresh_zones_in_window(off + resvars.coords_PanelLayoutp->watch_button_x,
                                                resvars.coords_PanelLayoutp->watch_button_y,
                                                off + resvars.coords_PanelLayoutp->watch_button_x + watch[14].width,
                                                watch[14].height + resvars.coords_PanelLayoutp->watch_button_y, 3);
                lbDisplay.draw_flags = 0;
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                                 off + resvars.coords_PanelLayoutp->watch_button_x,
                                 resvars.coords_PanelLayoutp->watch_button_y, watch[14].width, watch[14].height)) {
                    reqspr_draw(off + resvars.coords_PanelLayoutp->watch_button_x,
                                resvars.coords_PanelLayoutp->watch_button_y, &watch[15]);
                    set_all_refresh_zones_in_window(
                        off + resvars.coords_PanelLayoutp->watch_button_x, resvars.coords_PanelLayoutp->watch_button_y,
                        watch[15].width + (off + resvars.coords_PanelLayoutp->watch_button_x),
                        watch[15].height + resvars.coords_PanelLayoutp->watch_button_y, 3);
                    if (!othervar.watch_hovered)
                        play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
                    othervar.watch_hovered = 1;
                } else {
                    othervar.watch_hovered = 0;
                }
                if (myhosp->epidemic_illness && !myhosp->epidemic_timer)
                    lbDisplay.draw_flags = 4;
                if ((s16)myhosp->epidemic_timer / 8 >= 6) {
                    reqspr_draw(off + resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx,
                                resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy,
                                &watch[1]);
                    set_all_refresh_zones_in_window(
                        off + resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx,
                        resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy,
                        watch[1].width +
                            (off + resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx),
                        resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy +
                            watch[1].height,
                        3);
                }
                reqspr_draw(off + resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx,
                            resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy,
                            &watch[watch_face[(s16)myhosp->epidemic_timer / 8]]);
                set_all_refresh_zones_in_window(
                    off + resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx,
                    resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy,
                    watch[watch_face[(s16)myhosp->epidemic_timer / 8]].width +
                        (off + resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx),
                    (resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy) +
                        watch[watch_face[(s16)myhosp->epidemic_timer / 8]].height,
                    3);
                lbDisplay.draw_flags = 0;
            } else if (myhosp->opening_countdown) {
                reqspr_draw(resvars.coords_PanelLayoutp->watch_button_x, resvars.coords_PanelLayoutp->watch_button_y,
                            &watch[16]);
                set_all_refresh_zones_in_window(resvars.coords_PanelLayoutp->watch_button_x,
                                                resvars.coords_PanelLayoutp->watch_button_y,
                                                watch[16].width + resvars.coords_PanelLayoutp->watch_button_x,
                                                watch[16].height + resvars.coords_PanelLayoutp->watch_button_y, 3);
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.coords_PanelLayoutp->watch_button_x,
                                 resvars.coords_PanelLayoutp->watch_button_y, watch[14].width, watch[14].height)) {
                    reqspr_draw(resvars.coords_PanelLayoutp->watch_button_x,
                                resvars.coords_PanelLayoutp->watch_button_y, &watch[17]);
                    set_all_refresh_zones_in_window(resvars.coords_PanelLayoutp->watch_button_x,
                                                    resvars.coords_PanelLayoutp->watch_button_y,
                                                    watch[17].width + resvars.coords_PanelLayoutp->watch_button_x,
                                                    resvars.coords_PanelLayoutp->watch_button_y + watch[17].height, 3);
                    if (!othervar.watch_hovered)
                        play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
                    othervar.watch_hovered = 1;
                } else {
                    othervar.watch_hovered = 0;
                }
                if (myhosp->opening_countdown / (gbv.open_countdown / 13) >= 6) {
                    reqspr_draw(resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx,
                                resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy,
                                &watch[1]);
                    set_all_refresh_zones_in_window(
                        resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx,
                        resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy,
                        resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx +
                            watch[1].width,
                        resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy +
                            watch[1].height,
                        3);
                }
                reqspr_draw(resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx,
                            resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy,
                            &watch[watch_face[myhosp->opening_countdown / (gbv.open_countdown / 13)]]);
                set_all_refresh_zones_in_window(
                    resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx,
                    resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy,
                    (resvars.coords_PanelLayoutp->watch_x + resvars.coords_PanelLayoutp->watch_face_dx) +
                        watch[watch_face[myhosp->opening_countdown / (gbv.open_countdown / 13)]].width,
                    (resvars.coords_PanelLayoutp->watch_y + resvars.coords_PanelLayoutp->watch_face_dy) +
                        watch[watch_face[myhosp->opening_countdown / (gbv.open_countdown / 13)]].height,
                    3);
            }
        }
    }
}

// Clears the panel's info bar.
// FUNCTION: 0x00033a40
void clear_panel_info_bar(void)
{
    u8 i;

    if (!gamevar.rat_fest_level) {
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
        reqspr_draw(((PanelIcon *)panel)[13].x, ((PanelIcon *)panel)[13].y,
                    &game_panel[((PanelIcon *)panel)[13].sprite]);
        set_all_refresh_zones_in_window(((PanelIcon *)panel)[13].x, ((PanelIcon *)panel)[13].y,
                                        ((PanelIcon *)panel)[13].x + game_panel[((PanelIcon *)panel)[13].sprite].width,
                                        ((PanelIcon *)panel)[13].y + game_panel[((PanelIcon *)panel)[13].sprite].height,
                                        3);
        for (i = 0; ((PanelIcon *)panel)[i].sprite > 0; i++) {
            if (((PanelIcon *)panel)[i].sprite == 0xd) {
                reqspr_draw(((PanelIcon *)panel)[i].x, ((PanelIcon *)panel)[i].y,
                            &game_panel[((PanelIcon *)panel)[i].sprite]);
                set_all_refresh_zones_in_window(
                    ((PanelIcon *)panel)[i].x, ((PanelIcon *)panel)[i].y,
                    ((PanelIcon *)panel)[i].x + game_panel[((PanelIcon *)panel)[i].sprite].width,
                    ((PanelIcon *)panel)[i].y + game_panel[((PanelIcon *)panel)[i].sprite].height, 3);
            }
        }
    }
}

// Fills the info bar for the patient under the pointer: their status, the
// room they queue for or are heading to, and the diagnosis or its progress.
// FUNCTION: 0x00033c8c
void display_panel_person_info(void)
{
    u8 height;
    u8 line;
    u8 lines;
    char *text;
    RoomInfo *room;
    Thing *person;
    char buf[100];

    person = &thing[othervar.panel_thing];
    line = 0;
    height = resvars.coords_PanelLayoutp->info_line_height;
    clear_panel_info_bar();
    LbTextSetWindow(panel->x, panel->y, lbDisplay.physical_screen_width - panel->x,
                    lbDisplay.physical_screen_height - panel->y);
    set_font_from_cache(1);
    text = 0;
    if (person->flags2 & (PF_DYING >> 16))
        text = info_bar_text[4];
    else if (person->flags2 & (PF_NO_DIAGNOSIS >> 16))
        text = info_bar_text[15];
    else if (person->flags2 & (PF_NO_TREATMENT >> 16))
        text = info_bar_text[16];
    else if (person->flags2 & (PF_TOO_EXPENSIVE >> 16))
        text = info_bar_text[19];
    if (text) {
        line = 1;
        draw_panel_info_line(text, &line, 3);
        return;
    }
    if (resvars.game_mode == SCREEN_MODE_13)
        lines = 2;
    else
        lines = 1;
    if (person->flags0 & PF_AWAITING_DECISION) {
        draw_panel_info_line(info_bar_text[5], &line, lines);
    } else if (person->flags0 & PF_WAITING_FOR_ROOM) {
        if (person->flags1 & (PF_NEEDS_DIAGNOSIS_ROOM >> 8))
            draw_panel_info_line(info_bar_text[17], &line, lines);
        else
            draw_panel_info_line(info_bar_text[18], &line, lines);
    } else {
        room = &room_info[person->queue_room];
        if (room > room_info && room < end_room_info) {
            sprintf(buf, info_bar_text[6], room_names[rooms[room->type].id]);
            draw_panel_info_line(buf, &line, lines);
        }
        room = &room_info[person->dest_room];
        if (room > room_info && room < end_room_info && person->dest_room != person->queue_room) {
            sprintf(buf, info_bar_text[7], room_names[rooms[room->type].id]);
            draw_panel_info_line(buf, &line, lines);
        }
    }
    if (person->flags1 & (PF_CURED >> 8))
        draw_panel_info_line(info_bar_text[8], &line, lines);
    else if (person->flags1 & (PF_SENT_HOME_BY_INSPECTOR >> 8))
        draw_panel_info_line(info_bar_text[24], &line, lines);
    else if (person->flags1 & (PF_FED_UP >> 8))
        draw_panel_info_line(info_bar_text[9], &line, lines);
    else if (person->flags1 & (PF_SENT_HOME >> 8))
        draw_panel_info_line(info_bar_text[10], &line, lines);
    else if (person->flags2 & (PF_REFERRED >> 16))
        draw_panel_info_line(info_bar_text[11], &line, lines);
    else if (person->flags0 & PF_CONTAGIOUS)
        draw_panel_info_line(info_bar_text[25], &line, lines);
    if (line < 3 && person->illness != 0) {
        if (person->flags0 & PF_EMERGENCY) {
            line = 2;
            sprintf(buf, info_bar_text[22], illness_text[person->illness]);
            draw_panel_info_line(buf, &line, 3);
        } else if (person->flags32 & (PF_GUESSED | PF_DIAGNOSED)) {
            u8 k;

            k = person->known_illness;
            line = 2;
            if (person->flags2 & (PF_DIAGNOSED >> 16))
                sprintf(buf, info_bar_text[12], illness_text[k]);
            else
                sprintf(buf, info_bar_text[13], illness_text[k]);
            draw_panel_info_line(buf, &line, 3);
        } else {
            draw_panel_percent_bar(person->diagnosis * 100 / gbv.diagnosis_complete, info_bar_text[14]);
        }
    }
}

// Fills the info bar for the staff member under the pointer: their kind and
// skills, where they are heading, and their tiredness with the hospital's
// rest threshold.
// FUNCTION: 0x00034118
void display_panel_staff_info(void)
{
    u8 n;
    u8 height;
    u8 i;
    u8 pct;
    u8 line;
    Thing *staff;
    RoomInfo *room;
    char buf[100];
    char names[4][50];

    staff = &thing[othervar.panel_thing];
    line = 0;
    height = resvars.coords_PanelLayoutp->info_line_height;
    if (staff->owner != 0x63) {
        clear_panel_info_bar();
        LbTextSetWindow(panel->x, panel->y, lbDisplay.physical_screen_width - panel->x,
                        lbDisplay.physical_screen_height - panel->y);
        set_font_from_cache(1);
        lib_strcpy(buf, staff_names[staff->subtype]);
        if ((staff->subtype == STAFF_DOCTOR || staff->subtype == STAFF_SURGEON) &&
            (staff->skills & (SKILL_SURGEON | SKILL_PSYCHIATRIST | SKILL_CONSULTANT | SKILL_RESEARCHER))) {
            lib_strcpy(buf, "");
            n = 0;
            if (staff->skills & SKILL_CONSULTANT)
                lib_strcpy(names[n++], skill_names[7]);
            if (staff->skills & SKILL_RESEARCHER)
                lib_strcpy(names[n++], skill_names[8]);
            if (staff->skills & SKILL_SURGEON)
                lib_strcpy(names[n++], skill_names[5]);
            if (staff->skills & SKILL_PSYCHIATRIST) {
                if (n == 3)
                    lib_strcpy(names[n++], info_bar_text[26]);
                else
                    lib_strcpy(names[n++], skill_names[6]);
            }
            lib_strcpy(buf, names[0]);
            for (i = 1; i < n; i++)
                sprintf(buf, "%s %s", buf, names[i]);
        }
        draw_panel_info_line(buf, &line, 2);
        if (resvars.game_mode == SCREEN_MODE_13) {
            room = &room_info[staff->dest_room];
            if (room > room_info) {
                sprintf(buf, info_bar_text[7], room_names[rooms[room->type].id]);
                draw_panel_info_line(buf, &line, 2);
            } else if ((staff->subtype == STAFF_DOCTOR && staff->state == SS_DOCTOR_WAITS_FOR_PATIENT) ||
                       staff->next_state == SS_RECEPT_WAITS_FOR_PATIENT) {
                draw_panel_info_line(info_bar_text[23], &line, 2);
            } else if (staff->state == SS_WANDERS) {
                draw_panel_info_line(info_bar_text[27], &line, 2);
            }
        }
        pct = allhosps[staff->owner].rest_threshold * 100 / 1000;
        pct > 100 ? pct = 100 : 0;
        draw_panel_percent_bar(staff->tiredness * 100 / 1000, info_bar_text[28]);
        LbDrawHVLine(pct * (resvars.boxes_PanelLayoutp->info_bar_w - 2) / 100 +
                         (resvars.boxes_PanelLayoutp->info_bar_x + panel->x + 1),
                     resvars.boxes_PanelLayoutp->info_bar_y + (panel->y + resvars.coords_PanelLayoutp->info_text_y +
                                                               resvars.coords_PanelLayoutp->info_line_height * 2 + 1),
                     panel->x + resvars.boxes_PanelLayoutp->info_bar_x + 1 +
                         pct * (resvars.boxes_PanelLayoutp->info_bar_w - 2) / 100,
                     panel->y + resvars.coords_PanelLayoutp->info_text_y +
                         resvars.coords_PanelLayoutp->info_line_height * 2 + resvars.boxes_PanelLayoutp->info_bar_h -
                         1 + resvars.boxes_PanelLayoutp->info_bar_y,
                     color.white);
    }
}

// Fills the info bar for the object under the pointer: its name, strength and
// uses.
// FUNCTION: 0x000345e0
void display_panel_object_info(void)
{
    Thing *obj;
    u8 line;
    char buf[100];

    obj = &thing[othervar.panel_thing];
    line = 0;
    clear_panel_info_bar();
    if (obj->type == THING_OBJECT) {
        LbTextSetWindow(panel->x, panel->y, lbDisplay.physical_screen_width - panel->x,
                        lbDisplay.physical_screen_height - panel->y);
        set_font_from_cache(1);
        draw_panel_info_line(object_names[obj->subtype], &line, 3);
        if (resvars.game_mode == SCREEN_MODE_13) {
            sprintf(buf, info_bar_text[30], obj->strength);
            draw_panel_info_line(buf, &line, 3);
            sprintf(buf, info_bar_text[31], obj->times_used);
            draw_panel_info_line(buf, &line, 3);
        } else {
            sprintf(buf, info_bar_text[31], obj->times_used);
            sprintf(buf, "%s/%d", buf, obj->strength);
            line = 2;
            draw_panel_info_line(buf, &line, 3);
        }
    }
}

// Fills the info bar for the queue of the room under the pointer (or of a
// receptionist's desk): the room and its queue lengths.
// FUNCTION: 0x0003474c
void display_panel_queue_info(void)
{
    Thing *selected;
    RoomInfo *room;
    u8 line;
    u8 height;
    char buf[100];

    selected = &thing[othervar.panel_thing];
    line = 0;
    height = resvars.coords_PanelLayoutp->info_line_height;
    if (selected->type == THING_STAFF)
        room = selected->room;
    else
        room = &room_info[selected->room_id];
    if (room > room_info) {
        clear_panel_info_bar();
        LbTextSetWindow(panel->x, panel->y, lbDisplay.physical_screen_width - panel->x,
                        lbDisplay.physical_screen_height - panel->y);
        set_font_from_cache(1);
        draw_panel_info_line(room_names[rooms[room->type].id], &line, 3);
        sprintf(buf, info_bar_text[32], room->queue_length);
        if (resvars.game_mode != SCREEN_MODE_13)
            line = 2;
        draw_panel_info_line(buf, &line, 3);
        if (resvars.game_mode == SCREEN_MODE_13) {
            sprintf(buf, info_bar_text[33], room->queue_incoming);
            draw_panel_info_line(buf, &line, 3);
        }
    }
}

// Fills the info bar for the VIP or inspector under the pointer.
// FUNCTION: 0x000348d4
void display_panel_mayor_info(void)
{
    Thing *mayor;
    u8 line;

    mayor = &thing[othervar.panel_thing];
    line = 0;
    clear_panel_info_bar();
    LbTextSetWindow(panel->x, panel->y, lbDisplay.physical_screen_width - panel->x,
                    lbDisplay.physical_screen_height - panel->y);
    set_font_from_cache(1);
    if (mayor->type == THING_MAYOR) {
        draw_panel_info_line(info_bar_text[20], &line, 3);
        draw_panel_info_line(mayor_names[allhosps[mayor->owner].mayor_name], &line, 3);
    } else if (mayor->type == THING_INSPECTOR) {
        draw_panel_info_line(info_bar_text[21], &line, 3);
    }
}

// Fills the info bar for the machine under the pointer: its name and uses.
// FUNCTION: 0x000349e8
void display_panel_usage_info(void)
{
    Thing *machine;
    u8 line;
    char buf[100];

    machine = &thing[othervar.panel_thing];
    line = 0;
    clear_panel_info_bar();
    LbTextSetWindow(panel->x, panel->y, lbDisplay.physical_screen_width - panel->x,
                    lbDisplay.physical_screen_height - panel->y);
    set_font_from_cache(1);
    draw_panel_info_line(object_names[machine->subtype], &line, 3);
    if (resvars.game_mode != SCREEN_MODE_13)
        line = 2;
    sprintf(buf, info_bar_text[31], machine->times_used);
    draw_panel_info_line(buf, &line, 3);
}

// Draws this player's pending calls (faxes) above the panel, sliding new ones
// in, records the one under the mouse in othervar.hovered_popup_icon, and animates the
// panel's message bar.
// FUNCTION: 0x00034acc
void draw_popup_icons(void)
{
    u8 redraw;
    u8 no_flash;
    u8 j;
    s32 y;
    s32 y1;
    PlayerCall *call;
    TbSprite *spr;
    s32 i;
    s32 x;
    s32 x1;

    redraw = 0;
    no_flash = 1;
    if (!gamevar.rat_fest_level) {
        maintain_panel_popup();
        othervar.hovered_popup_icon = 0;
        if (resvars.game_mode == SCREEN_MODE_13) {
            LbScreenSetGraphicsWindow(0xb1, ((PanelIcon *)panel)[0].y, 100, 100);
            LbDrawBox(0, 0, 0x1c, 3, color.black);
        }
        othervar.emergency_anim_frame++;
        if (panel_emergency_spr[othervar.emergency_anim_frame] == 0)
            othervar.emergency_anim_frame = 0;
        switch (othervar.popup_shutter_state) {
        case 1:
        case 2:
        case 3:
        case 4:
            redraw = 1;
            break;
        }
        LbScreenSetGraphicsWindow(0,
                                  ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].sprite].height +
                                      resvars.coords_PanelLayoutp->popup_window_dy,
                                  lbDisplay.graphics_screen_width, game_panel[popup_icons[1].sprite].height);
        for (i = 1; popup_icons[i].sprite != 0; i++) {
            if (listhead[player].first[i]) {
                spr = &game_panel[popup_icons[i].sprite];
                call = &playercall[listhead[player].first[i]];
                if (call->slot < 9) {
                    if (call->flash) {
                        if (call->flash > 0x22)
                            x = resvars.coords_PanelLayoutp->slots[call->slot].x +
                                resvars.coords_PanelLayoutp->popup_icon_dx;
                        else
                            x = resvars.coords_PanelLayoutp->slots[0].x + resvars.coords_PanelLayoutp->popup_icon_dx;
                    } else {
                        x = resvars.coords_PanelLayoutp->slots[call->slot].x +
                            resvars.coords_PanelLayoutp->popup_icon_dx;
                    }
                    y = 0;
                    if (call->flash) {
                        x += resvars.panel_popup_coords_Pointp[call->flash].x;
                        y += resvars.panel_popup_coords_Pointp[call->flash].y;
                        if (resvars.panel_popup_coords_Pointp[call->flash].x == 0 &&
                            resvars.panel_popup_coords_Pointp[call->flash].y == 0) {
                            call->flash = 0;
                        } else {
                            if (call->flash == 1 && othervar.popup_shutter_state != 3) {
                            } else {
                                call->flash++;
                            }
                            for (;;) {
                                if (resvars.panel_popup_coords_Pointp[call->flash].x <
                                    resvars.coords_PanelLayoutp->slots[call->slot].x) {
                                    if (resvars.panel_popup_coords_Pointp[call->flash].x == 0 &&
                                        resvars.panel_popup_coords_Pointp[call->flash].y == 0) {
                                        if (call->flash > 0x22) {
                                            call->flash = 0;
                                        } else {
                                            switch (call->slot) {
                                            case 4:
                                            case 5:
                                            case 6:
                                            case 7:
                                                call->flash = 0;
                                                break;
                                            default:
                                                call->flash = 0x22;
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                    call->flash++;
                                } else {
                                    break;
                                }
                            }
                        }
                    }
                    x1 = x + spr->width;
                    y1 = y + spr->height;
                    if (y) {
                        reqspr_draw(x, y, spr);
                        if (popup_icons[i].sprite == 0x2b)
                            reqspr_draw(x + resvars.coords_PanelLayoutp->emergency_anim_dx,
                                        y + resvars.coords_PanelLayoutp->emergency_anim_dy,
                                        &game_panel[panel_emergency_spr[othervar.emergency_anim_frame]]);
                        othervar.popup_shutter_state = 2;
                        no_flash = 0;
                    }
                    redraw = 1;
                    y += ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].sprite].height;
                    y1 += ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].sprite].height;
                    if (lbDisplay.mmouse_x >= x && lbDisplay.mmouse_x < x1 && lbDisplay.mmouse_y >= y &&
                        lbDisplay.mmouse_y < y1)
                        othervar.hovered_popup_icon = i;
                    set_all_refresh_zones_in_window(x, y, x1, y1, 3);
                }
            }
        }
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
        if (no_flash)
            othervar.popup_shutter_state = 4;
        if (redraw) {
            reqspr_draw(((PanelIcon *)panel)[0].x, ((PanelIcon *)panel)[0].y,
                        &game_panel[((PanelIcon *)panel)[0].sprite]);
            reqspr_draw(((PanelIcon *)panel)[1].x, ((PanelIcon *)panel)[1].y,
                        &game_panel[((PanelIcon *)panel)[1].sprite]);
            draw_money_on_panel();
            draw_date_on_panel();
            draw_rep_on_panel();
            set_all_refresh_zones_in_window(((PanelIcon *)panel)[0].x, ((PanelIcon *)panel)[0].y,
                                            ((PanelIcon *)panel)[0].x2, ((PanelIcon *)panel)[0].y2, 3);
        }
        LbScreenSetGraphicsWindow(0,
                                  ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].sprite].height +
                                      resvars.coords_PanelLayoutp->popup_window_dy,
                                  lbDisplay.graphics_screen_width, game_panel[popup_icons[1].sprite].height);
        for (i = 1; popup_icons[i].sprite != 0; i++) {
            if (listhead[player].first[i]) {
                spr = &game_panel[popup_icons[i].sprite];
                call = &playercall[listhead[player].first[i]];
                if (call->slot < 9) {
                    if (call->flash) {
                        if (call->flash > 0x22)
                            x = resvars.coords_PanelLayoutp->slots[call->slot].x +
                                resvars.coords_PanelLayoutp->popup_icon_dx;
                        else
                            x = resvars.coords_PanelLayoutp->slots[0].x + resvars.coords_PanelLayoutp->popup_icon_dx;
                    } else {
                        x = resvars.coords_PanelLayoutp->slots[call->slot].x +
                            resvars.coords_PanelLayoutp->popup_icon_dx;
                    }
                    y = 0;
                    if (call->flash) {
                        x += resvars.panel_popup_coords_Pointp[call->flash].x;
                        y += resvars.panel_popup_coords_Pointp[call->flash].y;
                    }
                    if (y == 0) {
                        x1 = x + spr->width;
                        y1 = y + spr->height;
                        reqspr_draw(x, y, spr);
                        if (popup_icons[i].sprite == 0x2b)
                            reqspr_draw(x + resvars.coords_PanelLayoutp->emergency_anim_dx,
                                        y + resvars.coords_PanelLayoutp->emergency_anim_dy,
                                        &game_panel[panel_emergency_spr[othervar.emergency_anim_frame]]);
                        y += ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].sprite].height;
                        y1 += ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].sprite].height;
                        set_all_refresh_zones_in_window(x, y, x1, y1, 3);
                    }
                }
            }
        }
        if (resvars.game_mode == SCREEN_MODE_13)
            LbScreenSetGraphicsWindow(0xb1, ((PanelIcon *)panel)[0].y, 100, 100);
        x = resvars.coords_PanelLayoutp->shutter_x;
        y = resvars.coords_PanelLayoutp->shutter_y;
        switch (othervar.popup_shutter_state) {
        case 2:
            if (othervar.popup_shutter_width - 2 >= 0)
                othervar.popup_shutter_width -= 2;
            else
                othervar.popup_shutter_width = 0;
            if (othervar.popup_shutter_width == 0) {
                othervar.popup_shutter_state = 3;
                othervar.popup_shutter_timer = 0;
            } else {
                for (j = 0; j < othervar.popup_shutter_width; j++) {
                    if (resvars.game_mode == SCREEN_MODE_13)
                        reqspr_draw(x, y, &game_panel[41]);
                    x += game_panel[41].width;
                }
                if (resvars.game_mode == SCREEN_MODE_13)
                    reqspr_draw(x, y, &game_panel[42]);
            }
            break;
        case 3:
            othervar.popup_shutter_width = 0;
            othervar.popup_shutter_timer++;
            if (othervar.popup_shutter_timer > 0x28)
                othervar.popup_shutter_state = 4;
            break;
        case 4:
            if (othervar.popup_shutter_timer <= 0x28) {
                othervar.popup_shutter_timer++;
            } else {
                if (othervar.popup_shutter_width + 2 < 0x18)
                    othervar.popup_shutter_width += 2;
                else
                    othervar.popup_shutter_width = 0x19;
                for (j = 0; j < othervar.popup_shutter_width; j++) {
                    if (resvars.game_mode == SCREEN_MODE_13)
                        reqspr_draw(x, y, &game_panel[41]);
                    x += game_panel[41].width;
                }
                if (resvars.game_mode == SCREEN_MODE_13)
                    reqspr_draw(x, y, &game_panel[42]);
                if (othervar.popup_shutter_width >= 0x18)
                    othervar.popup_shutter_state = 1;
            }
            break;
        case 1:
            if (resvars.game_mode == SCREEN_MODE_13)
                reqspr_draw(x, y, &game_panel[40]);
            for (j = 0; j < 0x19; j++) {
                if (resvars.game_mode == SCREEN_MODE_13)
                    reqspr_draw(x, y, &game_panel[41]);
                x += game_panel[41].width;
            }
            if (resvars.game_mode == SCREEN_MODE_13)
                reqspr_draw(x, y, &game_panel[42]);
            othervar.popup_shutter_width = 0x18;
            break;
        default:
            othervar.popup_shutter_state = 1;
            break;
        }
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
        lbDisplay.draw_flags = 0;
    }
}

// Removes call's icon from the panel if it is this player's, and closes up
// any empty slot.
// FUNCTION: 0x00035624
void delete_panel_popup(PlayerCall *call)
{
    u8 i;

    if (call->owner == player) {
        for (i = 0; i < 9; i++) {
            if (panel_popup_pos[i] == call - playercall)
                remove_panel_popup(i);
            if (panel_popup_pos[i] == 0)
                remove_panel_popup(i);
        }
    }
}

// Removes the call icon in panel slot `slot` (0-8), moving later ones down.
// FUNCTION: 0x000356c4
void remove_panel_popup(const u8 slot)
{
    u8 i;
    PlayerCall *call;

    for (i = slot + 1; i < 9; i++) {
        call = &playercall[panel_popup_pos[i]];
        if (call->slot > 0)
            call->slot--;
        panel_popup_pos[i - 1] = panel_popup_pos[i];
    }
    panel_popup_pos[8] = 0;
}

// Gives a new call of this player a panel slot (9 if all are taken) and
// starts its slide-in with a fax sound.
// FUNCTION: 0x00035760
void add_panel_popup(PlayerCall *call)
{
    u8 i;

    rebuild_panel_popup_pos();
    if (call->owner == player) {
        for (i = 0; panel_popup_pos[i] != 0; i++)
            ;
        if (i < 9) {
            call->slot = i;
            panel_popup_pos[i] = call - playercall;
            call->flash = 1;
            othervar.popup_shutter_state = 2;
        } else {
            call->slot = 9;
        }
        play_sound(SNDSRC_GENERAL, SFX_NEWFAX, -1, -1);
    }
}

// Rebuilds panel_popup_pos from the calls' slots.
// FUNCTION: 0x00035824
void rebuild_panel_popup_pos(void)
{
    u8 i;
    u16 j;
    PlayerCall *call;

    for (i = 0; i < 9; i++)
        panel_popup_pos[i] = 0;
    for (j = 1; popup_icons[j].sprite != 0; j++) {
        if (listhead[player].first[j]) {
            call = &playercall[listhead[player].first[j]];
            panel_popup_pos[call->slot] = call - playercall;
        }
    }
}

// Keeps the panel's call icons packed: drops slots whose call has gone,
// moves calls down into empty slots and separates calls sharing a slot.
// FUNCTION: 0x00035914
void maintain_panel_popup(void)
{
    u8 slot;
    u8 found;
    u16 j;
    u16 k;
    u16 i;
    PlayerCall *call;
    PlayerCall *other_call;

    rebuild_panel_popup_pos();
    for (i = 0; i < 9; i++) {
        if (panel_popup_pos[i]) {
            call = &playercall[panel_popup_pos[i]];
            found = 1;
            for (j = 1; popup_icons[j].sprite != 0; j++) {
                if (panel_popup_pos[i] == listhead[player].first[j])
                    found = 0;
            }
            if (found)
                remove_panel_popup(i);
        } else {
            remove_panel_popup(i);
        }
    }
    for (j = 1; popup_icons[j].sprite != 0; j++) {
        if (listhead[player].first[j]) {
            call = &playercall[listhead[player].first[j]];
            if (call->slot < 9 && call->slot > 0) {
                if (!panel_popup_pos[call->slot - 1]) {
                    remove_panel_popup(call->slot - 1);
                    call->slot--;
                    panel_popup_pos[call->slot] = call - playercall;
                    call->flash = 1;
                }
            }
        }
    }
    for (j = 1; popup_icons[j].sprite != 0; j++) {
        if (listhead[player].first[j]) {
            call = &playercall[listhead[player].first[j]];
            if (call->slot < 9) {
                slot = call->slot;
                for (k = 1; popup_icons[k].sprite != 0; k++) {
                    if (listhead[player].first[k]) {
                        other_call = &playercall[listhead[player].first[k]];
                        if (other_call->slot < 9) {
                            if (other_call != call) {
                                if (slot == other_call->slot)
                                    other_call->slot++;
                            }
                        }
                    }
                }
            }
        }
    }
}

// Flashes the panel icon the advisor points at (advisor.tutorial_highlight 0x10-0x1b)
// every ten frames.
// FUNCTION: 0x00035c68
void panel_flash(void)
{
    set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    if (advisor.tutorial_highlight >= 0x10 && advisor.tutorial_highlight <= 0x1b) {
        othervar.flash_counter++;
        if (othervar.flash_counter % 10 == 0) {
            reqspr_draw(((PanelIcon *)panel)[advisor.tutorial_highlight - 0x10].x,
                        ((PanelIcon *)panel)[advisor.tutorial_highlight - 0x10].y,
                        &game_panel[((PanelIcon *)panel)[advisor.tutorial_highlight - 0x10].pressed]);
            othervar.flash_counter = 0;
            redraw_panel = 1;
        }
    }
}

// Draws the rat game's panel: the score, the chosen weapon and a
// four-segment bar from myhosp->litter_count.
// FUNCTION: 0x00035d5c
void draw_ratfest_on_panel(void)
{
    u8 level;
    u8 s3;
    u8 s0;
    u8 s1;
    u8 s2;
    char buf[100];

    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x29);
    sprintf(buf, "%5d", myhosp->rats_killed_u);
    LbTextDraw(0x5d, 0x1be, buf);
    switch (gamevar.rat_fest_powerup) {
    case 0:
        reqspr_draw(((PanelIcon *)panel)[1].x, ((PanelIcon *)panel)[1].y + 0xb,
                    &game_panel[((PanelIcon *)panel)[1].pressed]);
        break;
    case 1:
        reqspr_draw(((PanelIcon *)panel)[2].x, ((PanelIcon *)panel)[2].y + 0xb,
                    &game_panel[((PanelIcon *)panel)[2].pressed]);
        break;
    case 2:
        reqspr_draw(((PanelIcon *)panel)[3].x, ((PanelIcon *)panel)[3].y + 0xb,
                    &game_panel[((PanelIcon *)panel)[3].pressed]);
        break;
    case 3:
        reqspr_draw(((PanelIcon *)panel)[4].x, ((PanelIcon *)panel)[4].y + 0xb,
                    &game_panel[((PanelIcon *)panel)[4].pressed]);
        break;
    }
    if (myhosp->litter_count < 100)
        level = 10 - myhosp->litter_count / 8;
    else
        level = 10;
    s0 = s1 = s2 = s3 = 0;
    switch (level) {
    case 0:
        if (actual_game_turn & 3) {
            s0 = 0x1a;
            s1 = 0x1a;
            s2 = 0x1a;
            s3 = 0x1a;
        } else {
            s0 = 0x1b;
            s1 = 0x1b;
            s2 = 0x1b;
            s3 = 0x1b;
        }
        break;
    case 1:
        s0 = 0x1a;
        s1 = 0x1a;
        s2 = 0x1a;
        s3 = 0x1a;
        break;
    case 2:
        s0 = 0x1a;
        s1 = 0x1a;
        s2 = 0x1a;
        s3 = 0x19;
        break;
    case 3:
        s0 = 0x1a;
        s1 = 0x1a;
        s2 = 0x1a;
        s3 = 0x18;
        break;
    case 4:
        s0 = 0x1a;
        s1 = 0x1a;
        s2 = 0x1a;
        break;
    case 5:
        s0 = 0x1a;
        s1 = 0x1a;
        s2 = 0x19;
        break;
    case 6:
        s0 = 0x1a;
        s1 = 0x1a;
        s2 = 0x18;
        break;
    case 7:
        s0 = 0x1a;
        s1 = 0x1a;
        break;
    case 8:
        s0 = 0x1a;
        s1 = 0x19;
        break;
    case 9:
        s0 = 0x1a;
        s1 = 0x18;
        break;
    case 10:
        s0 = 0x1a;
        break;
    case 11:
        s0 = 0x19;
        break;
    case 12:
        s0 = 0x18;
        break;
    }
    reqspr_draw(((PanelIcon *)panel)[5].x + 0x2c, ((PanelIcon *)panel)[5].y + 0xc, &game_panel[s0]);
    reqspr_draw(((PanelIcon *)panel)[5].x + 0x2c + game_panel[26].width, ((PanelIcon *)panel)[5].y + 0xc,
                &game_panel[s1]);
    reqspr_draw(((PanelIcon *)panel)[5].x + 0x2c + game_panel[26].width * 2, ((PanelIcon *)panel)[5].y + 0xc,
                &game_panel[s2]);
    reqspr_draw(((PanelIcon *)panel)[5].x + 0x2c + game_panel[26].width * 3, ((PanelIcon *)panel)[5].y + 0xc,
                &game_panel[s3]);
}

// While the game is paused, or a panel message (othervar.panel_message, a misc_text
// index) is timed, shows it on the info bar. Returns 1 if it did.
// FUNCTION: 0x000361a4
u8 display_panel_special_info(void)
{
    u8 i;
    u8 line;
    u8 y;
    s16 x;
    Thing *selected;

    selected = &thing[othervar.panel_thing];
    line = 0;
    y = resvars.coords_PanelLayoutp->info_message_dy;
    i = 0;
    x = 0;
    lbDisplay.draw_flags = 0;
    if (gamevar.paused || othervar.panel_message) {
        if (!othervar.panel_info_hold) {
            if (statvar.panel_top <= lbDisplay.mmouse_y && !gamevar.rat_fest_level && !gamevar.paused) {
                othervar.panel_message = 0;
                return 0;
            }
        } else {
            othervar.panel_info_hold--;
        }
        panel = panel_icons_normal;
        while (((PanelIcon *)panel)[i].sprite > 0) {
            ((PanelIcon *)panel)[i].y =
                lbDisplay.physical_screen_height - game_panel[((PanelIcon *)panel)[i].sprite].height;
            ((PanelIcon *)panel)[i].y2 = lbDisplay.physical_screen_height;
            ((PanelIcon *)panel)[i].x = x;
            ((PanelIcon *)panel)[i].x2 = x + game_panel[((PanelIcon *)panel)[i].sprite].width;
            x += game_panel[((PanelIcon *)panel)[i].sprite].width;
            i++;
        }
        statvar.panel_top = ((PanelIcon *)panel)[1].y;
        clear_panel_info_bar();
        LbTextSetWindow(panel->x, panel->y, lbDisplay.physical_screen_width - panel->x,
                        lbDisplay.physical_screen_height - panel->y);
        set_font_from_cache(0x2c);
        if (gamevar.paused) {
            LbTextDraw(resvars.coords_PanelLayoutp->info_text_x, y + resvars.coords_PanelLayoutp->info_text_y,
                       misc_text[21]);
        } else if (othervar.panel_message) {
            LbTextDraw(resvars.coords_PanelLayoutp->info_text_x, y + resvars.coords_PanelLayoutp->info_text_y,
                       misc_text[othervar.panel_message]);
            othervar.panel_message_time--;
            if (othervar.panel_message_time == 0)
                othervar.panel_message = 0;
        }
        return 1;
    }
    return 0;
}

// Help text for the panel's watch: emergency, epidemic or the other timed
// event, into othervar.help_msg.
// FUNCTION: 0x0003646c
void panel_watch_help(void)
{
    u8 text;

    if (myhosp->emergency_timer)
        text = 0xee;
    else if (myhosp->epidemic_timer)
        text = 0xef;
    else if (myhosp->opening_countdown)
        text = 0xed;
    else
        return;
    lib_strcpy(othervar.help_msg, help_text[text]);
}

// Help text for the reputation scale, with the reputation value, into
// othervar.help_msg.
// FUNCTION: 0x000364e0
void panel_rep_help(void)
{
    char buf[12];

    lib_strcpy(othervar.help_msg, help_text[9]);
    sprintf(buf, " (%d)", myhosp->reputation);
    strcat(othervar.help_msg, buf);
}

// Loads the rat game's panel sprites (not in 320x200) and lays them out.
// FUNCTION: 0x00036534
void setup_rat_fest_panel(void)
{
    LbSpriteReset(game_panel, end_game_panel, game_panel_data);
    if (resvars.screen_mode != SCREEN_MODE_1) {
        LbDataLoadAll(hiresratfestpanelfiles);
        LbSpriteSetup(game_panel, end_game_panel, game_panel_data);
        setup_panel_icons(resvars.screen_mode);
    }
}

// Draws text on the info bar's third line with a percentage bar of value
// (capped at 100).
// FUNCTION: 0x00036594
void draw_panel_percent_bar(s32 value, char *text)
{
    s32 x;
    s32 y;

    x = resvars.boxes_PanelLayoutp->info_bar_x + panel->x;
    y = resvars.coords_PanelLayoutp->info_text_y + panel->y + resvars.coords_PanelLayoutp->info_line_height * 2 +
        resvars.boxes_PanelLayoutp->info_bar_y;
    value > 100 ? (value = 100) : 0;
    LbTextDraw(resvars.coords_PanelLayoutp->info_text_x,
               resvars.coords_PanelLayoutp->info_line_height * 2 + resvars.coords_PanelLayoutp->info_text_y, text);
    LbDrawBox(x, y, resvars.boxes_PanelLayoutp->info_bar_w, resvars.boxes_PanelLayoutp->info_bar_h, color.white);
    LbDrawBox(x + 1, y + 1, (resvars.boxes_PanelLayoutp->info_bar_w - 2) * value / 100,
              resvars.boxes_PanelLayoutp->info_bar_h - 2, color.red);
    LbDrawBox(x + 1 + (resvars.boxes_PanelLayoutp->info_bar_w - 2) * value / 100, y + 1,
              resvars.boxes_PanelLayoutp->info_bar_w - 2 - (resvars.boxes_PanelLayoutp->info_bar_w - 2) * value / 100,
              resvars.boxes_PanelLayoutp->info_bar_h - 2, color.black);
}

// Draws text on info bar line *line, wrapping it onto a second line if too
// wide, and advances *line. Draws nothing if it would pass line `lines`.
// FUNCTION: 0x00036730
void draw_panel_info_line(char *text, u8 *line, u8 lines)
{
    s32 height;
    s32 width;
    s32 total;
    char *cursor;
    char *last;

    height = resvars.coords_PanelLayoutp->info_line_height;
    width = lbDisplay.physical_screen_width - panel->x - resvars.coords_PanelLayoutp->info_text_x * 2;
    if (LbTextStringWidth(text) < width) {
        if (*line + 1 <= lines) {
            LbTextDraw(resvars.coords_PanelLayoutp->info_text_x,
                       resvars.coords_PanelLayoutp->info_text_y + *line * height, text);
            (*line)++;
        }
    } else if (*line + 2 <= lines) {
        total = 0;
        cursor = text;
        while (total < width && *cursor) {
            last = cursor;
            while (*cursor && *cursor != ' ') {
                total += lbFontPtr ? lbFontPtr[*cursor - 0x1f].width : 0;
                cursor++;
            }
            if (*cursor) {
                total += lbFontPtr ? lbFontPtr[*cursor - 0x1f].width : 0;
                cursor++;
            }
        }
        last[-1] = 0;
        LbTextDraw(resvars.coords_PanelLayoutp->info_text_x, resvars.coords_PanelLayoutp->info_text_y + *line * height,
                   text);
        last[-1] = ' ';
        (*line)++;
        LbTextDraw(resvars.coords_PanelLayoutp->info_text_x, resvars.coords_PanelLayoutp->info_text_y + *line * height,
                   last);
        (*line)++;
    }
}
