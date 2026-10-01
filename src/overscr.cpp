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
s32 nth_active_player(int player);
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
void pie_render(s32 x, s32 y, s32 r, s32 h, s32 d, s32 n, Sector *s);

// FUNCTION: 0x00056d50
void display_peep_overview(void)
{
    u8 nplayers;
    s32 slot;
    s32 i;

    nplayers = 4;
    switch (D_00158166) {
    case 0:
        load_ingame_pic(0x14, 1, 0x26, 0, SCREEN_MODE_13);
        D_00158166 = 1;
        overview = player;
        close_irequesters();
        copy_screen_to_wscreen(bscreen);
        draw_overview_screen();
        calculate_piechart();
        draw_piechart();
        break;
    case 1:
        if (statvar.unknownfd > 1 && D_001609c4[overview].active == 0)
            overview = player;
        copy_screen_to_wscreen(bscreen);
        if (lbDisplay.mleft_button) {
            slot = 0;
            for (i = 0; i < nplayers; i++) {
                if (statvar.unknownfd > 1 && D_001609c4[i].active == 0)
                    continue;
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f3c_ScreenBoxp[0x37 + slot].x, resvars.f3c_ScreenBoxp[0x37 + slot].y,
                                 resvars.f3c_ScreenBoxp[0x37 + slot].w, resvars.f3c_ScreenBoxp[0x37 + slot].h))
                    reqspr_draw(resvars.f3c_ScreenBoxp[0x37 + slot].x + 0x13, resvars.f3c_ScreenBoxp[0x37 + slot].y, &screen_sprites_tab_start[9]);
                slot++;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f3c_ScreenBoxp[0x3c].x, resvars.f3c_ScreenBoxp[0x3c].y, resvars.f3c_ScreenBoxp[0x3c].w,
                             resvars.f3c_ScreenBoxp[0x3c].h))
                reqspr_draw(resvars.f3c_ScreenBoxp[0x3c].x, resvars.f3c_ScreenBoxp[0x3c].y, &screen_sprites_tab_start[8]);
        }
        if (lbDisplay.rleft_button) {
            slot = 0;
            for (i = 0; i < nplayers; i++) {
                if (statvar.unknownfd > 1 && D_001609c4[i].active == 0)
                    continue;
                if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f3c_ScreenBoxp[0x37 + slot].x, resvars.f3c_ScreenBoxp[0x37 + slot].y,
                                 resvars.f3c_ScreenBoxp[0x37 + slot].w, resvars.f3c_ScreenBoxp[0x37 + slot].h)) {
                    overview = i;
                    play_sound(10, SFX_SELECTX, -1, -1);
                    reqspr_draw(resvars.f3c_ScreenBoxp[0x37 + slot].x + 0x13, resvars.f3c_ScreenBoxp[0x37 + slot].y, &screen_sprites_tab_start[9]);
                }
                slot++;
            }
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f3c_ScreenBoxp[0x3c].x, resvars.f3c_ScreenBoxp[0x3c].y, resvars.f3c_ScreenBoxp[0x3c].w,
                             resvars.f3c_ScreenBoxp[0x3c].h)) {
                D_00158166 = 2;
                play_sound(10, SFX_NO4, -1, -1);
                reqspr_draw(resvars.f3c_ScreenBoxp[0x3c].x, resvars.f3c_ScreenBoxp[0x3c].y, &screen_sprites_tab_start[8]);
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

        sprintf(buf, "%s %4.4d", overview_text[0], mycontrol->f648 + 2000);
        LbTextDraw(resvars.f40[0x6f].x, resvars.f40[0x6f].y, buf);
        slot = 0;
        for (i = 0; i < nplayers; i++) {
            if (statvar.unknownfd > 1 && D_001609c4[i].active == 0)
                continue;
            if (overview == i)
                set_font_from_cache(0x25);
            else
                set_font_from_cache(0x24);
            sprintf(buf, "%s", &allhosps[i]);
            LbTextDraw(resvars.f40[0x68 + slot].x, resvars.f40[0x68 + slot].y, buf);
            slot++;
        }
        buf[0] = 0;
        if (adcounts.f0 > 1) {
            pct = adcounts.fa / adcounts.f0;
            if (pct > 100)
                pct = 100;
            if (gbv.f110 > pct)
                fn_000c93bf(buf, overview_text[1]);
            else if (gbv.f10e > pct)
                fn_000c93bf(buf, overview_text[2]);
            reqspr_draw(resvars.f40[0x6c].x + pct, resvars.f40[0x6c].y, &screen_sprites_tab_start[5]);
            pct = adcounts.f10 / adcounts.f0;
            if (pct > 100)
                pct = 100;
            if (pct < 30)
                fn_000c93bf(buf, overview_text[3]);
            reqspr_draw(resvars.f40[0x6d].x + pct, resvars.f40[0x6d].y, &screen_sprites_tab_start[5]);
            pct = adcounts.f14 / adcounts.f0;
            if (pct > 100)
                pct = 100;
            if (pct > 80)
                fn_000c93bf(buf, overview_text[4]);
            else if (pct < 20)
                fn_000c93bf(buf, overview_text[5]);
            reqspr_draw(resvars.f40[0x6e].x + pct, resvars.f40[0x6e].y, &screen_sprites_tab_start[5]);
            reqspr_draw(0xf5, 0x12c, &screen_sprites_tab_start[7]);
            set_font_from_cache(0x24);
            lbDisplay.draw_flags = 0x20;
            LbTextSetWindow(resvars.f3c_ScreenBoxp[0x3b].x, resvars.f3c_ScreenBoxp[0x3b].y, resvars.f3c_ScreenBoxp[0x3b].w, resvars.f3c_ScreenBoxp[0x3b].h);
            LbTextDraw(0, 0, buf);
        }
        set_font_from_cache(0x26);
        lbDisplay.draw_flags = 0;
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        sprintf(buf, "%s (%d%%)", overview_text[6], allhosps[overview].f1cb);
        conv_string(buf, (s8 *)buf, 0);
        LbTextDraw(resvars.f40[0x70].x, resvars.f40[0x70].y, buf);
        sprintf(buf, "%s", overview_text[7]);
        LbTextDraw(resvars.f40[0x71].x, resvars.f40[0x71].y, buf);
        set_font_from_cache(0x24);
        display_overview_bars(overview);
        compute_help_text(0, 0, 0x18);
    }
}

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
    if (statvar.unknownfd > 1) {
        for (i = 0; i < 4; i++) {
            if (D_001609c4[i].active != 0)
                total += allhosps[i].f1cb;
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
                slices[i].end = (slices[i].start + (allhosps[owner].f1cb << 11) / 100) & 0x7ff;
                prev = slices[i].end;
            }
            slices[3].end = 0x7ff;
        }
    } else {
        for (i = 0; i < 4; i++)
            total += allhosps[i].f1cb;
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
                if (allhosps[i].f1cb == 100)
                    slices[i].end = (slices[i].start + 0x7eb) & 0x7ff;
                else
                    slices[i].end = (slices[i].start + (allhosps[i].f1cb << 11) / 100) & 0x7ff;
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
    if (statvar.unknownfd > 1) {
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
            if (lose_criteria[i].criterion == crit && lose_criteria[i].f14) {
                for (j = 0; j < 50; j++) {
                    if (lose_criteria[i].f14 / 2 >= j) {
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
                if (win_criteria[i].criterion == crit && win_criteria[i].fc == othervar.fa4a) {
                    for (j = 0; j < 50; j++) {
                        if (win_criteria[i].f14 / 2 >= j) {
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

// FUNCTION: 0x00057f18
void overscr_help_add_name(void)
{
    s32 text;
    s32 owner;

    text = resvars.f2c[othervar.f8ac].text;
    owner = resvars.f2c[othervar.f8ac].arg;
    sprintf(othervar.f96, help_text[text], &allhosps[owner]);
}

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


    idx = resvars.f2c[othervar.f8ac].arg;
    count = 0;
    found = 0;
    hit = 0;
    owner = overview;
    for (crit = 0; crit <= 6; crit++) {
        found = 0;
        for (i = 0; i < 10; i++) {
            if (lose_criteria[i].criterion == crit && lose_criteria[i].f14) {
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
                if (win_criteria[i].criterion == crit && win_criteria[i].fc == othervar.fa4a) {
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
                sprintf(othervar.f96, win_criteria_help[0], value, allhosps[owner].unknownc7e);
                break;
            case 2:
                sprintf(othervar.f96, win_criteria_help[1], value, allhosps[owner].f35 - allhosps[owner].f3d);
                break;
            case 3:
                if (allhosps[owner].f1ed > 10)
                    sprintf(othervar.f96, win_criteria_help[6], value, allhosps[owner].f1ef * 100 / allhosps[owner].f1ed);
                else
                    sprintf(othervar.f96, win_criteria_help[6], value, 0);
                break;
            case 4:
                sprintf(othervar.f96, win_criteria_help[3], value, allhosps[owner].f1e9);
                break;
            case 5:
                if (allhosps[owner].f1ed > 10)
                    sprintf(othervar.f96, win_criteria_help[4], value, allhosps[owner].f1eb * 100 / allhosps[owner].f1ed);
                else
                    sprintf(othervar.f96, win_criteria_help[4], value, 0);
                break;
            case 6:
                sprintf(othervar.f96, win_criteria_help[5], value, allhosps[owner].f1bb);
                break;
            }
            break;
        }
    }
}
