#include "hosp.h"

extern "C" {
extern u8 redraw_panel;
extern u8 help_close_hover;
extern char **show_help_text_0;
extern char **level_text;
extern char **net_mission_text;
extern char **beta_demo_text_short;
extern Event D_001549d0[];

int strcmp(const char *a, const char *b);
u32 lib_strlen(char *s);
#pragma aux lib_strlen "strlen_"
void LbMouseSetWindow(s32 x, s32 y, s32 width, s32 height);
}

void set_help_text(u8 mode, s16 x, s16 y, char **strs);
void copy_and_format(char *dst, char *src);
void copy_and_format_composite(char *dst, char **strs);
void help_hotspot_detected(u16 hotspot, u8 help_group, s16 x, s16 y);
void set_help_from_index(u16 hotspot, s16 x, s16 y);

// Records in statvar.help_start where each help group's hotspots begin in the
// layout's help hotspot list (groups end with x0 -1, the list with -2).
// FUNCTION: 0x00099c70
void setup_help_indices(void)
{
    u8 help_group;
    u8 done;
    u16 i;

    i = 0;
    help_group = 0;
    done = 0;
    while (!done) {
        while (resvars.help_hotspots_HelpHotspotp[i].x0 >= 0)
            i++;
        if (resvars.help_hotspots_HelpHotspotp[i].x0 == -2)
            done = 1;
        ++i;
        ++help_group;
        statvar.help_start[help_group] = i;
        if (i == 0x1b) {
        }
    }
}

// Shows the tooltip of help group help_group's hotspot under the mouse, the
// group's hotspots being relative to x,y; or the forced help hotspot if one is
// set for this group. Not in help mode.
// FUNCTION: 0x00099d00
void compute_help_text(s16 x, s16 y, u8 help_group)
{
    s16 dx;
    s16 dy;
    u16 i;

    dx = lbDisplay.mmouse_x - x;
    dy = lbDisplay.mmouse_y - y;
    if (gamevar.mouse_mode == MM_HELP)
        return;
    if (othervar.forced_help_hotspot) {
        if (help_group == othervar.forced_help_group)
            set_help_from_index(othervar.forced_help_hotspot, x, y);
        return;
    }
    for (i = statvar.help_start[help_group]; resvars.help_hotspots_HelpHotspotp[i].x0 >= 0; i++) {
        if (dx >= resvars.help_hotspots_HelpHotspotp[i].x0 && dx <= resvars.help_hotspots_HelpHotspotp[i].x1 &&
            dy >= resvars.help_hotspots_HelpHotspotp[i].y0 && dy <= resvars.help_hotspots_HelpHotspotp[i].y1) {
            help_hotspot_detected(i, help_group, x, y);
            return;
        }
    }
    if (help_group == othervar.help_group)
        othervar.help_hotspot = 0;
}

// Prepares a help box with text strs: mode 1 a tooltip (strs[0] wrapped) above
// x,y, mode 2 a framed help page (the "."-ended list of strs) centred on screen
// when x,y is -1,-1, with the mouse kept inside it. Stores the text, size and
// position in othervar (help_msg, help_w, help_h, help_x, help_y), kept on screen (the
// bottom limit uses the screen width).
// FUNCTION: 0x00099e4c
void set_help_text(u8 mode, s16 x, s16 y, char **strs)
{
    s32 border;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 mx;
    s32 my;

    othervar.help_style = mode;
    set_font_from_cache(display_mode == DM_MAIN_MENU ? 0x2d : 0);
    lbDisplay.draw_flags = 0;
    switch (mode) {
    case 1:
        copy_and_format(othervar.help_msg, strs[0]);
        border = 3;
        othervar.help_w = LbTextStringWidth(othervar.help_msg) + border * 2;
        othervar.help_h = LbTextStringHeight(othervar.help_msg) + border * 2;
        break;
    case 2:
        copy_and_format_composite(othervar.help_msg, strs);
        border = pulldv[12].height + 0xc;
        othervar.help_w = LbTextStringWidth(othervar.help_msg) + border * 2;
        othervar.help_h = LbTextStringHeight(othervar.help_msg) + border * 2 + pulldv[19].height;
        break;
    }
    if (x == -1 && y == -1) {
        othervar.help_x = (lbDisplay.physical_screen_width - othervar.help_w) / 2;
        othervar.help_y = (lbDisplay.physical_screen_height - othervar.help_h) / 2;
        LbMouseSetWindow(othervar.help_x, othervar.help_y, othervar.help_w, othervar.help_h);
        x0 = othervar.help_x;
        y0 = othervar.help_y;
        x1 = x0 + othervar.help_w;
        y1 = y0 + othervar.help_h;
        if (!((lbDisplay.mmouse_x >= x0 && lbDisplay.mmouse_x < x1) && lbDisplay.mmouse_y >= y0 &&
              lbDisplay.mmouse_y < y1)) {
            mx = lbDisplay.mmouse_x;
            my = lbDisplay.mmouse_y;
            mx < x0 ? mx = x0 : 0;
            mx > x1 ? mx = x1 : 0;
            my < y0 ? my = y0 : 0;
            my > y1 ? my = y1 : 0;
            LbMouseSetPosition(mx, my);
        }
    } else {
        othervar.help_x = x - border;
        othervar.help_y = y - border;
        othervar.help_y -= othervar.help_h;
        othervar.help_y += 6;
    }
    othervar.help_x < 0 ? othervar.help_x = 0 : 0;
    othervar.help_y < 0 ? othervar.help_y = 0 : 0;
    othervar.help_x > lbDisplay.physical_screen_width - othervar.help_w
        ? othervar.help_x = lbDisplay.physical_screen_width - othervar.help_w
        : 0;
    othervar.help_y > lbDisplay.physical_screen_width - othervar.help_h
        ? othervar.help_y = lbDisplay.physical_screen_width - othervar.help_h
        : 0;
}

// Draws the prepared help box and its text, with a close button for help
// pages; the text is cleared afterwards unless in help mode. Only when the
// mouse mode allows help or in the tutorial.
// FUNCTION: 0x0009a1f8
void draw_help_text(void)
{
    s32 i;
    s32 j;
    s32 step;
    s32 right;
    s32 bottom;
    s32 border;

    if (!(othervar.help_msg[0] && (mouse_mode_help_allowed[gamevar.mouse_mode] || (advisor.flags & 1))))
        return;
    set_all_refresh_zones_in_window(othervar.help_x, othervar.help_y, othervar.help_x + othervar.help_w + 0xc,
                                    othervar.help_y + othervar.help_h + 0xc, 3);
    if (othervar.help_group == 3)
        redraw_panel = 1;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_width);
    lbDisplay.draw_flags = 0;
    LbDrawBox(othervar.help_x, othervar.help_y, othervar.help_w, othervar.help_h,
              othervar.screen_pic_loaded ? color.white : 0xab);
    switch (othervar.help_style) {
    case 1:
        lbDisplay.draw_flags = 0x10;
        LbDrawBox(othervar.help_x, othervar.help_y, othervar.help_w, othervar.help_h, color.black);
        border = 3;
        break;
    case 2:
        step = pulldv[12].width;
        j = othervar.help_y;
        bottom = othervar.help_y + othervar.help_h - pulldv[16].height;
        for (i = othervar.help_x; othervar.help_x + othervar.help_w - step > i; i += step) {
            reqspr_draw(i, j, &pulldv[12]);
            reqspr_draw(i, bottom, &pulldv[16]);
        }
        step = pulldv[18].height;
        i = othervar.help_x;
        right = othervar.help_x + othervar.help_w - pulldv[14].width;
        for (j = othervar.help_y; othervar.help_y + othervar.help_h - step > j; j += step) {
            reqspr_draw(i, j, &pulldv[18]);
            reqspr_draw(right, j, &pulldv[14]);
        }
        reqspr_draw(othervar.help_x, othervar.help_y, &pulldv[11]);
        reqspr_draw(othervar.help_x + othervar.help_w - pulldv[13].width, othervar.help_y, &pulldv[13]);
        reqspr_draw(othervar.help_x, othervar.help_y + othervar.help_h - pulldv[17].height, &pulldv[17]);
        reqspr_draw(othervar.help_x + othervar.help_w - pulldv[15].width,
                    othervar.help_y + othervar.help_h - pulldv[15].height, &pulldv[15]);
        border = pulldv[12].height + 0xc;
        othervar.help_close_x = othervar.help_x + othervar.help_w - 0xc - pulldv[19].width;
        othervar.help_close_y = othervar.help_y + othervar.help_h - 0xc - pulldv[19].height;
        if ((othervar.help_close_x <= lbDisplay.mmouse_x &&
             othervar.help_close_x + pulldv[19].width > lbDisplay.mmouse_x) &&
            othervar.help_close_y <= lbDisplay.mmouse_y &&
            othervar.help_close_y + pulldv[19].height > lbDisplay.mmouse_y) {
            reqspr_draw(othervar.help_close_x, othervar.help_close_y, &pulldv[20]);
            if (!help_close_hover)
                play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
            help_close_hover = 1;
        } else {
            reqspr_draw(othervar.help_close_x, othervar.help_close_y, &pulldv[19]);
            help_close_hover = 0;
        }
        lbDisplay.draw_flags = 4;
        LbDrawBox(othervar.help_x + othervar.help_w, othervar.help_y + 0xc, 0xc, othervar.help_h, color.black);
        LbDrawBox(othervar.help_x + 0xc, othervar.help_y + othervar.help_h, othervar.help_w - 0xc, 0xc, color.black);
        lbDisplay.draw_flags = 0;
        break;
    }
    lbDisplay.draw_flags = 0x40;
    set_font_from_cache(display_mode == DM_MAIN_MENU ? 0x2d : 0);
    lbDisplay.draw_colour = color.black;
    LbTextDraw(othervar.help_x + border, othervar.help_y + border, othervar.help_msg);
    lbDisplay.draw_flags = 0;
    if (gamevar.mouse_mode != MM_HELP)
        othervar.help_msg[0] = 0;
}

// Copies src to dst, breaking lines at spaces once a line reaches a width
// (in characters) that grows with the text's length.
// FUNCTION: 0x0009a7c8
void copy_and_format(char *dst, char *src)
{
    s32 len;
    s32 k;
    s32 width;
    s32 col;

    len = lib_strlen(src);
    col = 0;
    width = 0xc;
    for (k = 1; width * k < len; width += 4, k++)
        ;
    while (*src) {
        if (*src == ' ' && col >= width) {
            *dst++ = '\n';
            src++;
            col = 0;
        } else {
            *dst++ = *src++;
            col++;
        }
    }
    *dst = 0;
}

// Joins the "."-ended list of strings strs into dst as one text, "//" giving
// a paragraph break, wrapping lines to a width that grows with the longest
// paragraph (at most three quarters of the screen).
// FUNCTION: 0x0009a86c
void copy_and_format_composite(char *dst, char **strs)
{
    s32 most;
    s32 count;
    s32 k;
    s32 limit;
    s32 cur;
    char **pp;
    char *p;
    char *q;
    char *dst_start;
    char last;

    most = 0;
    count = 0;
    for (pp = strs; **pp != '.'; pp++) {
        for (p = *pp; *p; p++) {
            if (strcmp(p, "//") == 0) {
                most < count ? most = count : 0;
                count = 0;
                p++;
            } else {
                count++;
            }
        }
    }
    most < count ? most = count : 0;
    limit = 0xc;
    for (k = 1; limit * k < most; limit += 4, k++)
        ;
    set_font_from_cache(display_mode == DM_MAIN_MENU ? 0x2d : 0);
    limit = limit * (lbFontPtr ? lbFontPtr[46].width : 0);
    limit > lbDisplay.physical_screen_width * 3 / 4 ? limit = lbDisplay.physical_screen_width * 3 / 4 : 0;
    dst_start = dst;
    cur = 0;
    for (; strcmp(*strs, ".") != 0; strs++) {
        for (p = *strs; *p; p++) {
            if (cur > limit) {
                q = dst;
                while (--q > dst_start) {
                    if (*q == '\n')
                        break;
                    if (*q == ' ') {
                        *q = '\n';
                        *dst = 0;
                        q++;
                        cur = LbTextStringWidth(q);
                        break;
                    }
                }
            }
            if (strcmp(p, "//") == 0) {
                *dst++ = '\n';
                *dst++ = '\n';
                p++;
                cur = 0;
            } else {
                *dst++ = *p;
                cur += lbFontPtr ? lbFontPtr[*p - 31].width : 0;
            }
        }
        last = p[-1];
        if (last == '.' || last == '!' || last == '?' || last == ',') {
            *dst++ = ' ';
            cur += lbFontPtr ? lbFontPtr[1].width : 0;
        }
    }
    *dst = 0;
}

// Clears the tooltip and its hover state.
// FUNCTION: 0x0009ab54
void cancel_helpmsg(void)
{
    othervar.help_hotspot = 0;
    othervar.help_shown_time = 0;
    othervar.help_msg[0] = 0;
}

// Handles the mouse resting on help hotspot hotspot of group help_group: its
// tooltip appears after 750 timer ticks, or at once within 500 ticks of the
// previous tooltip. In network games the player slots of group 0x18 beyond
// the players present are ignored.
// FUNCTION: 0x0009ab84
void help_hotspot_detected(u16 hotspot, u8 help_group, s16 x, s16 y)
{
    s32 slot;

    if (statvar.num_players > 1) {
        slot = hotspot - statvar.help_start[help_group];
        if (help_group == 0x18) {
            if (slot < 4 && network_num_players() <= slot)
                return;
        }
    }
    if (hotspot == othervar.help_hotspot) {
        if (othervar.help_hover_start + 0x2ee < othervar.clock_ms)
            othervar.help_shown_time = othervar.clock_ms;
    } else {
        othervar.help_group = help_group;
        othervar.help_hotspot = hotspot;
        othervar.help_hover_start = othervar.clock_ms;
    }
    if (othervar.help_shown_time + 0x1f4 > othervar.clock_ms) {
        othervar.help_shown_time = othervar.clock_ms;
        othervar.help_group = help_group;
        set_help_from_index(hotspot, x, y);
    }
}

// Shows the tooltip of help hotspot hotspot above its top centre (offset by
// x,y): its help_text entry, or the text its callback writes.
// FUNCTION: 0x0009ac70
void set_help_from_index(u16 hotspot, s16 x, s16 y)
{
    s16 cy;
    s16 cx;
    char *text;

    cx = x + (resvars.help_hotspots_HelpHotspotp[hotspot].x0 + resvars.help_hotspots_HelpHotspotp[hotspot].x1) / 2;
    cy = y + resvars.help_hotspots_HelpHotspotp[hotspot].y0;
    if (resvars.help_hotspots_HelpHotspotp[hotspot].func) {
        text = othervar.help_msg;
        resvars.help_hotspots_HelpHotspotp[hotspot].func();
        set_help_text(1, cx, cy, &text);
    } else {
        set_help_text(1, cx, cy, &help_text[resvars.help_hotspots_HelpHotspotp[hotspot].text]);
    }
}

// Forces the tooltip of the help hotspot with text id id to show while help
// group help_group is displayed.
// FUNCTION: 0x0009ad68
void set_force_help(u8 help_group, u16 id)
{
    u16 i;
    u8 found;

    i = 0;
    found = 0;
    othervar.forced_help_group = help_group;
    for (i = 0; resvars.help_hotspots_HelpHotspotp[i].x0 != -2; i++) {
        if (id == resvars.help_hotspots_HelpHotspotp[i].text) {
            othervar.forced_help_hotspot = i;
            break;
        }
    }
}

// Stops forcing a tooltip.
// FUNCTION: 0x0009ade8
void clear_force_help(void)
{
    othervar.forced_help_hotspot = 0;
}

// Opens help page sub of kind page in help mode: 0 a room's help, 1 a level's
// briefing, 2 the network game's criterion (filled with its target and time
// left), 3 a network result page.
// FUNCTION: 0x0009ae08
void show_help_page(u32 page, u32 sub)
{
    char *saved;

    set_mouse_mode(MM_HELP);
    switch (page) {
    case 0:
        set_help_text(2, -1, -1, &show_help_text_0[statvar.help_page0_start[sub]]);
        break;
    case 1:
        set_help_text(2, -1, -1, &level_text[statvar.help_page1_start[sub]]);
        break;
    case 2: {
        char buf[500];

        switch (current_net_criteria.criterion) {
        case 0:
            set_mouse_mode(MM_NORMAL);
            return;
        case 1:
        case 9:
        case 2:
        case 8:
        case 3:
            sprintf(buf, net_mission_text[statvar.net_help_start[sub]],
                    current_net_criteria.start - othervar.game_month);
            saved = net_mission_text[statvar.net_help_start[sub]];
            net_mission_text[statvar.net_help_start[sub]] = buf;
            break;
        case 4:
        case 6:
        case 5:
        case 7:
            if (current_net_criteria.criterion == 7 && myplayer.language == 2)
                sprintf(buf, net_mission_text[statvar.net_help_start[sub]],
                        current_net_criteria.start - othervar.game_month, current_net_criteria.value);
            else
                sprintf(buf, net_mission_text[statvar.net_help_start[sub]], current_net_criteria.value,
                        current_net_criteria.start - othervar.game_month);
            saved = net_mission_text[statvar.net_help_start[sub]];
            net_mission_text[statvar.net_help_start[sub]] = buf;
            break;
        }
        set_help_text(2, -1, -1, &net_mission_text[statvar.net_help_start[sub]]);
        net_mission_text[statvar.net_help_start[sub]] = saved;
        break;
    }
    case 3:
        set_help_text(2, -1, -1, &beta_demo_text_short[statvar.net_result_base[sub]]);
        break;
    }
    othervar.help_page = page;
    othervar.help_sub = sub;
}

// Fills idx with the index in strs of the start of each "."-ended section, up
// to n sections or the ".." that ends the list.
// FUNCTION: 0x0009b144
void setup_indices_for_composite_help(char **strs, u16 *idx, u32 n)
{
    u32 count;
    u32 i;

    count = 0;
    i = 0;
    if (strs) {
    }
    for (; strcmp(*strs, "..") != 0 && count < n; count++) {
        *idx++ = i;
        for (; strcmp(*strs, ".") != 0; i++)
            strs++;
        strs++;
        i++;
    }
    if (count == n && strcmp(*strs, "..") != 0) {
    }
}

// Closes a help page and leaves help mode; after some briefing pages opens
// the next, and after network result page 2 queues event EV_LEAVE_NET_GAME.
// FUNCTION: 0x0009b1f0
void finish_composite_help(void)
{
    LbMouseSetWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
    help_close_hover = 0;
    redraw_panel = 1;
    othervar.help_msg[0] = 0;
    if (gamevar.mouse_mode == MM_HELP) {
        set_mouse_mode(MM_NORMAL);
        play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
        switch (othervar.help_page) {
        case 1:
            switch (othervar.help_sub) {
            case 0xf:
            case 0x10:
                show_help_page(1, othervar.help_sub + 1);
                break;
            }
            break;
        case 3:
            if (othervar.help_sub == 2) {
                D_001549d0[player].type = EV_LEAVE_NET_GAME;
                D_001549d0[player].value = 1;
            }
            break;
        }
    }
}
