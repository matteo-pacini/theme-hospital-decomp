#include "hosp.h"

struct MenuItem {
    u8 unknown0[8];
    u16 text;
    u16 arg;
    u8 unknownc[4];
};

struct ScreenBox {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

extern "C" {
extern u16 overview;
extern u8 D_00158166;
extern Event D_001549d0[];
extern char **overview_text;
extern char **win_criteria_help;

void fn_000c93bf(char *dst, char *src);
void conv_string(char *dst, s8 *src, u8 mode);
s32 nth_active_player(int nth);
}


static Sector slices[] = {
    {0x000, 0x300, 0, 0xca, 0xce, 0xc6, 0xce, 0xce, 0xaf, 0, 0, 0},
    {0x300, 0x578, 0, 0xfd, 0xdc, 0xd8, 0xdc, 0xd8, 0xaf, 0, 0, 0},
    {0x578, 0x708, 0, 0xfb, 0xeb, 0xf9, 0xeb, 0xf9, 0xaf, 0, 0, 0},
    {0x708, 0x7ff, 0, 0xfc, 0xe4, 0xe7, 0xe4, 0xe7, 0xaf, 0, 0, 0},
};
u32 n_slices = 4;
extern WinLose win_criteria[];
extern WinLose lose_criteria[];

void draw_overview_screen(void);
void calculate_piechart(void);
void draw_piechart(void);
void display_overview_bars(u8 who);
s32 evaluate_criteria(const u8 owner);
u32 check_criteria_values(const u32 mode, const u8 owner);
void pie_render(s32 cx, s32 cy, s32 rx, s32 ry, s32 h, s32 count, Sector *sectors);

// Runs one frame of the status (overview) screen (state D_00158166: 0
// opening, 1 shown, 2 closing): clicking a hospital's name shows its
// figures, the close button queues EV_RETURN_FROM_SCREEN.
// FUNCTION: 0x00056d50
void display_peep_overview(void)
{
    u8 nplayers;
    s32 slot;
    s32 i;

    nplayers = 4;
    switch (D_00158166) {
    case 0:
        load_ingame_pic(SCR_OVERVIEW, 1, 0x26, 0, SCREEN_MODE_13);
        D_00158166 = 1;
        overview = player;
        close_irequesters();
        copy_screen_to_wscreen(bscreen);
        draw_overview_screen();
        calculate_piechart();
        draw_piechart();
        break;
    case 1:
        if (statvar.num_players > 1 && D_001609c4[overview].active == 0)
            overview = player;
        copy_screen_to_wscreen(bscreen);
        if (lbDisplay.mleft_button) {
            slot = 0;
            for (i = 0; i < nplayers; i++) {
                if (statvar.num_players > 1 && D_001609c4[i].active == 0)
                    continue;
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_boxes_ScreenBoxp[0x37 + slot].x, resvars.screen_boxes_ScreenBoxp[0x37 + slot].y,
                                 resvars.screen_boxes_ScreenBoxp[0x37 + slot].w, resvars.screen_boxes_ScreenBoxp[0x37 + slot].h))
                    reqspr_draw(resvars.screen_boxes_ScreenBoxp[0x37 + slot].x + 0x13, resvars.screen_boxes_ScreenBoxp[0x37 + slot].y, &screen_sprites_tab_start[9]);
                slot++;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_boxes_ScreenBoxp[0x3c].x, resvars.screen_boxes_ScreenBoxp[0x3c].y, resvars.screen_boxes_ScreenBoxp[0x3c].w,
                             resvars.screen_boxes_ScreenBoxp[0x3c].h))
                reqspr_draw(resvars.screen_boxes_ScreenBoxp[0x3c].x, resvars.screen_boxes_ScreenBoxp[0x3c].y, &screen_sprites_tab_start[8]);
        }
        if (lbDisplay.rleft_button) {
            slot = 0;
            for (i = 0; i < nplayers; i++) {
                if (statvar.num_players > 1 && D_001609c4[i].active == 0)
                    continue;
                if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_boxes_ScreenBoxp[0x37 + slot].x, resvars.screen_boxes_ScreenBoxp[0x37 + slot].y,
                                 resvars.screen_boxes_ScreenBoxp[0x37 + slot].w, resvars.screen_boxes_ScreenBoxp[0x37 + slot].h)) {
                    overview = i;
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    reqspr_draw(resvars.screen_boxes_ScreenBoxp[0x37 + slot].x + 0x13, resvars.screen_boxes_ScreenBoxp[0x37 + slot].y, &screen_sprites_tab_start[9]);
                }
                slot++;
            }
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_boxes_ScreenBoxp[0x3c].x, resvars.screen_boxes_ScreenBoxp[0x3c].y, resvars.screen_boxes_ScreenBoxp[0x3c].w,
                             resvars.screen_boxes_ScreenBoxp[0x3c].h)) {
                D_00158166 = 2;
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
                reqspr_draw(resvars.screen_boxes_ScreenBoxp[0x3c].x, resvars.screen_boxes_ScreenBoxp[0x3c].y, &screen_sprites_tab_start[8]);
            }
            lbDisplay.rleft_button = 0;
        }
        calculate_value(overview);
        evaluate_criteria(overview);
        check_criteria_values(1, overview);
        draw_overview_screen();
        calculate_piechart();
        draw_piechart();
        break;
    case 2:
        D_001549d0[player].type = EV_RETURN_FROM_SCREEN;
        D_001549d0[player].value = 0;
        cancel_helpmsg();
        break;
    }
    lbDisplay.draw_flags = 0;
    if (exit_quick_requester())
        D_00158166 = 2;
}

// Draws the status screen for hospital `overview`: the year, the players'
// names, the patients' happiness, thirst and warmth gauges with a warning,
// its share (allhosps.market_share percent) and the criteria bars.
// FUNCTION: 0x000571ec
void draw_overview_screen(void)
{
    s32 height;
    u8 nplayers;
    s32 i;
    s32 slot;
    s32 pct;


    nplayers = 4;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    set_font_from_cache(0x24);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    {
        char buf[500];

        sprintf(buf, "%s %4.4d", overview_text[0], mycontrol->year + 2000);
        LbTextDraw(resvars.screen_coords[0x6f].x, resvars.screen_coords[0x6f].y, buf);
        slot = 0;
        for (i = 0; i < nplayers; i++) {
            if (statvar.num_players > 1 && D_001609c4[i].active == 0)
                continue;
            if (overview == i)
                set_font_from_cache(0x25);
            else
                set_font_from_cache(0x24);
            sprintf(buf, "%s", &allhosps[i]);
            LbTextDraw(resvars.screen_coords[0x68 + slot].x, resvars.screen_coords[0x68 + slot].y, buf);
            slot++;
        }
        buf[0] = 0;
        if (adcounts.patients > 1) {
            pct = adcounts.happiness_sum / adcounts.patients;
            if (pct > 100)
                pct = 100;
            if (gbv.very_unhappy > pct)
                fn_000c93bf(buf, overview_text[1]);
            else if (gbv.unhappy > pct)
                fn_000c93bf(buf, overview_text[2]);
            reqspr_draw(resvars.screen_coords[0x6c].x + pct, resvars.screen_coords[0x6c].y, &screen_sprites_tab_start[5]);
            pct = adcounts.refreshment_sum / adcounts.patients;
            if (pct > 100)
                pct = 100;
            if (pct < 30)
                fn_000c93bf(buf, overview_text[3]);
            reqspr_draw(resvars.screen_coords[0x6d].x + pct, resvars.screen_coords[0x6d].y, &screen_sprites_tab_start[5]);
            pct = adcounts.warmth_sum / adcounts.patients;
            if (pct > 100)
                pct = 100;
            if (pct > 80)
                fn_000c93bf(buf, overview_text[4]);
            else if (pct < 20)
                fn_000c93bf(buf, overview_text[5]);
            reqspr_draw(resvars.screen_coords[0x6e].x + pct, resvars.screen_coords[0x6e].y, &screen_sprites_tab_start[5]);
            reqspr_draw(0xf5, 0x12c, &screen_sprites_tab_start[7]);
            set_font_from_cache(0x24);
            lbDisplay.draw_flags = 0x20;
            LbTextSetWindow(resvars.screen_boxes_ScreenBoxp[0x3b].x, resvars.screen_boxes_ScreenBoxp[0x3b].y, resvars.screen_boxes_ScreenBoxp[0x3b].w, resvars.screen_boxes_ScreenBoxp[0x3b].h);
            LbTextDraw(0, 0, buf);
        }
        set_font_from_cache(0x26);
        lbDisplay.draw_flags = 0;
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        sprintf(buf, "%s (%d%%)", overview_text[6], allhosps[overview].market_share);
        conv_string(buf, (s8 *)buf, 0);
        LbTextDraw(resvars.screen_coords[0x70].x, resvars.screen_coords[0x70].y, buf);
        sprintf(buf, "%s", overview_text[7]);
        LbTextDraw(resvars.screen_coords[0x71].x, resvars.screen_coords[0x71].y, buf);
        set_font_from_cache(0x24);
        display_overview_bars(overview);
        compute_help_text(0, 0, 0x18);
    }
}

// Sizes the pie chart's slices from each hospital's allhosps.market_share percentage
// (equal slices if all are 0), and grows the selected hospital's slice out
// of the pie while shrinking the others back.
// FUNCTION: 0x00057684
void calculate_piechart(void)
{
    u8 i;
    u8 owner;
    u16 prev;
    u16 total;
    s32 nplayers;

    i = 0;
    owner = 0;
    prev = 0;
    total = 0;
    nplayers = network_num_players();
    if (statvar.num_players > 1) {
        for (i = 0; i < 4; i++) {
            if (D_001609c4[i].active != 0)
                total += allhosps[i].market_share;
        }
        if (total == 0) {
            for (i = 0; i < nplayers; i++) {
                slices[i].start = prev;
                slices[i].end = (slices[i].start + (100 / nplayers << 11) / 100) & 0x7ff;
                prev = slices[i].end;
            }
            slices[3].end = 0x7ff;
        } else {
            for (i = 0; i < nplayers; i++) {
                owner = nth_active_player(i + 1);
                slices[i].start = prev;
                slices[i].end = (slices[i].start + (allhosps[owner].market_share << 11) / 100) & 0x7ff;
                prev = slices[i].end;
            }
            slices[3].end = 0x7ff;
        }
    } else {
        for (i = 0; i < 4; i++)
            total += allhosps[i].market_share;
        if (total == 0) {
            for (i = 0; i < 4u; i++) {
                slices[i].start = 0;
                slices[i].end = 0;
            }
            for (i = 0; i < 4; i++) {
                slices[i].start = prev;
                slices[i].end = (slices[i].start + 0x200) & 0x7ff;
                prev = slices[i].end;
            }
            slices[3].end = 0x7ff;
        } else {
            for (i = 0; i < 4u; i++) {
                slices[i].start = 0;
                slices[i].end = 0;
            }
            for (i = 0; i < 4; i++) {
                slices[i].start = prev;
                if (allhosps[i].market_share == 100)
                    slices[i].end = (slices[i].start + 0x7eb) & 0x7ff;
                else
                    slices[i].end = (slices[i].start + (allhosps[i].market_share << 11) / 100) & 0x7ff;
                prev = slices[i].end;
            }
            slices[3].end = 0x7ff;
        }
    }
    for (i = 0; i < 4u; i++) {
        slices[i].start += 4;
        slices[i].end += 4;
        if (slices[i].radius != 0 && i != overview)
            slices[i].radius -= 2;
    }
    if (slices[overview].radius < 0x50)
        slices[overview].radius += 2;
}

// Draws the pie chart and each slice's hospital name.
// FUNCTION: 0x00057b0c
void draw_piechart(void)
{
    char buf[0x50];
    s32 i;
    s32 nplayers;
    s32 x;
    s32 y;
    s32 rx;
    s32 ry;

    nplayers = network_num_players();
    x = 0x1fa;
    y = 0x78;
    rx = 0x40;
    ry = 0x1e;
    set_font_from_cache(0x26);
    if (statvar.num_players > 1) {
        pie_render(x, y, rx, ry, 0x14, nplayers, slices);
        for (i = 0; i < nplayers; i++) {
            s32 tx;
            s32 ty;

            tx = slices[i].text_x;
            ty = slices[i].text_y;
            sprintf(buf, "%s", &allhosps[nth_active_player(i + 1)]);
            LbTextDraw(tx, ty, buf);
        }
    } else {
        pie_render(x, y, rx, ry, 0x14, 4, slices);
        for (i = 0; i < 4u; i++) {
            s32 tx;
            s32 ty;

            tx = slices[i].text_x;
            ty = slices[i].text_y;
            sprintf(buf, "%s", &allhosps[i]);
            LbTextDraw(tx, ty, buf);
        }
    }
}

// Draws up to five criteria bars (lose criteria in place of the win
// criterion of the same kind) and their icons. The parameter is unused.
// FUNCTION: 0x00057c70
void display_overview_bars(u8 who)
{
    u8 i;
    u8 j;
    u8 unused;
    u8 found;
    u8 col;
    u8 crit;

    unused = 0;
    col = 0;
    for (crit = 0; crit <= 6; crit++) {
        found = 0;
        for (i = 0; i < 10; i++) {
            if (lose_criteria[i].criterion == crit && lose_criteria[i].progress) {
                for (j = 0; j < 50; j++) {
                    if (lose_criteria[i].progress / 2 >= j) {
                        reqspr_draw(col * 30 + 0x10d, 0xed - j, &screen_sprites_tab_start[3]);
                        reqspr_draw(col * 30 + 0x10d, 0xed - j - 1, &screen_sprites_tab_start[4]);
                    }
                }
                reqspr_draw(col * 30 + 0x106, 0xf3, &screen_sprites_tab_start[lose_criteria[i].criterion + 9]);
                col++;
                found = 1;
                break;
            }
        }
        if (!found) {
            for (i = 0; i < 10; i++) {
                if (win_criteria[i].criterion == crit && win_criteria[i].group == othervar.best_win_group) {
                    for (j = 0; j < 50; j++) {
                        if (win_criteria[i].progress / 2 >= j) {
                            reqspr_draw(col * 30 + 0x10d, 0xed - j, &screen_sprites_tab_start[1]);
                            reqspr_draw(col * 30 + 0x10d, 0xed - j - 1, &screen_sprites_tab_start[2]);
                        }
                    }
                    reqspr_draw(col * 30 + 0x106, 0xf3, &screen_sprites_tab_start[win_criteria[i].criterion + 9]);
                    col++;
                    break;
                }
            }
        }
        if (col >= 5)
            break;
    }
}

// Status screen help: formats the hovered help entry with the hospital name
// of its argument into othervar.help_msg.
// FUNCTION: 0x00057f18
void overscr_help_add_name(void)
{
    s32 text;
    s32 owner;

    text = resvars.help_hotspots[othervar.help_hotspot].text;
    owner = resvars.help_hotspots[othervar.help_hotspot].arg;
    sprintf(othervar.help_msg, help_text[text], &allhosps[owner]);
}

// Status screen help for a criteria bar: formats the hovered bar's target
// and the shown hospital's current value into othervar.help_msg.
// FUNCTION: 0x00057f98
void overscr_help_win_criteria(void)
{
    u8 crit;
    u8 hit;
    u8 owner;
    u8 which;
    u8 found;
    s32 value;
    s32 idx;
    s32 i;
    s32 count;


    idx = resvars.help_hotspots[othervar.help_hotspot].arg;
    count = 0;
    found = 0;
    hit = 0;
    owner = overview;
    for (crit = 0; crit <= 6; crit++) {
        found = 0;
        for (i = 0; i < 10; i++) {
            if (lose_criteria[i].criterion == crit && lose_criteria[i].progress) {
                found = 1;
                if (count++ == idx) {
                    hit = 1;
                    which = crit;
                    value = lose_criteria[i].value;
                }
                break;
            }
        }
        if (!found) {
            for (i = 0; i < 10; i++) {
                if (win_criteria[i].criterion == crit && win_criteria[i].group == othervar.best_win_group) {
                    if (count++ == idx) {
                        hit = 1;
                        which = crit;
                        value = win_criteria[i].value;
                        break;
                    }
                }
            }
        }
        if (hit) {
            switch (which) {
            case 1:
                sprintf(othervar.help_msg, win_criteria_help[0], value, allhosps[owner].reputation);
                break;
            case 2:
                sprintf(othervar.help_msg, win_criteria_help[1], value, allhosps[owner].balance - allhosps[owner].loan);
                break;
            case 3:
                if (allhosps[owner].visitors > 10)
                    sprintf(othervar.help_msg, win_criteria_help[6], value, allhosps[owner].diagnosed * 100 / allhosps[owner].visitors);
                else
                    sprintf(othervar.help_msg, win_criteria_help[6], value, 0);
                break;
            case 4:
                sprintf(othervar.help_msg, win_criteria_help[3], value, allhosps[owner].cures);
                break;
            case 5:
                if (allhosps[owner].visitors > 10)
                    sprintf(othervar.help_msg, win_criteria_help[4], value, allhosps[owner].deaths * 100 / allhosps[owner].visitors);
                else
                    sprintf(othervar.help_msg, win_criteria_help[4], value, 0);
                break;
            case 6:
                sprintf(othervar.help_msg, win_criteria_help[5], value, allhosps[owner].hospital_value);
                break;
            }
            break;
        }
    }
}
