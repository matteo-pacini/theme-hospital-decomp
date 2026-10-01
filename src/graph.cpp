#include "hosp.h"

extern Graphs graph[];

extern "C" {
}


struct GraphLabel {
    u8 kind;
    u16 value;
    s32 data;
};

struct GraphRect {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

extern "C" {
extern GraphLabel D_0015eba0[];
extern s16 D_0015eb9c;
extern u8 D_0015eb94[];
extern Event D_001549d0[];
extern char **D_001602b0;
void fn_00078d70(s16 kind, s16 value, s32 data);
void fn_00078e38(u16 gap);
void fn_000c9be0(s32 x0, s32 y0, s32 x1, s32 y1, u8 colour);
s32 LbTextWordWidth(char *text);
}

void process_graph_panel(void);

extern "C" {
}

// FUNCTION: 0x00078d70
void fn_00078d70(s16 kind, s16 value, s32 data)
{
    u8 i;

    i = 0;
    while (D_0015eba0[i].value <= value && i < D_0015eb9c)
        i++;
    memmove(&D_0015eba0[i + 1], &D_0015eba0[i], (D_0015eb9c - i) * 7);
    D_0015eba0[i].kind = kind;
    D_0015eba0[i].value = value;
    D_0015eba0[i].data = data;
    D_0015eb9c++;
}

// FUNCTION: 0x00078e38
void fn_00078e38(u16 gap)
{
    u8 moved;
    s16 i;
    u8 below;
    u8 above;

    moved = 1;
    while (moved) {
        moved = 0;
        for (i = 0; i < D_0015eb9c; i++) {
            below = i > 0 && D_0015eba0[i].value - D_0015eba0[i - 1].value < gap ? (u8)1 : (u8)0;
            above = i < D_0015eb9c - 1 && D_0015eba0[i + 1].value - D_0015eba0[i].value < gap ? (u8)1 : (u8)0;
            if (below && !above) {
                if (D_0015eba0[i].value < resvars.f3c_GraphRectp[46].y)
                    D_0015eba0[i].value++;
                moved = 1;
            }
            if (!below && above) {
                if (D_0015eba0[i].value > resvars.f3c_GraphRectp[46].y - resvars.f3c_GraphRectp[46].h - gap)
                    D_0015eba0[i].value--;
                moved = 1;
            }
        }
    }
}

// FUNCTION: 0x00078fc4
void draw_graphs(u16 x, u16 y, u16 h, u16 step)
{
    s8 scale;
    s8 k;
    s8 month;
    s8 j;
    s8 i;
    u16 px;
    u16 nx;
    u16 font_height;
    u16 h2;
    s16 year;
    u16 py;
    s16 quarter;
    s32 values[3][12];
    char buf[128];

    compute_help_text(0, 0, 0x11);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    switch (graphs.unknown2) {
    case 0:
        load_ingame_pic(4, 1, 0x1f, 0, SCREEN_MODE_13);
        graphs.unknown2 = 1;
        lbDisplay.draw_flags = 0;
        break;
    case 1:
        copy_screen_to_wscreen(bscreen);
        process_graph_panel();
        if (exit_quick_requester())
            graphs.unknown2 = 2;
        break;
    case 2:
        D_001549d0[player].type = 0x70;
        D_001549d0[player].value = 0;
        break;
    }
    set_font_from_cache(0xb);
    font_height = lbFontPtr ? lbFontPtr[34].height : 0;
    h2 = lbFontPtr ? lbFontPtr[34].height : 0;
    D_0015eb94[0] = color.unknownD;
    D_0015eb94[1] = color.unknownC;
    D_0015eb94[2] = color.unknownB;
    D_0015eb94[3] = color.unknownA;
    D_0015eb94[4] = color.unknown12;
    D_0015eb94[5] = color.unknown9;
    D_0015eb94[6] = color.unknownF;
    D_0015eb94[7] = color.unknown14;
    for (i = 0; i < 8; i++) {
        if (!graph[i].visible)
            reqspr_draw(resvars.f48[i + 0x25].unknownC, resvars.f48[i + 0x25].unknownE,
                        &screen_sprites_tab_start[resvars.f48[i + 0x25].sprite]);
        LbTextSetWindow(resvars.f3c_GraphRectp[i + 47].x, resvars.f3c_GraphRectp[i + 47].y, resvars.f3c_GraphRectp[i + 47].w, resvars.f3c_GraphRectp[i + 47].h);
        lbDisplay.draw_flags |= 0x140;
        lbDisplay.draw_colour = D_0015eb94[graph[i].colour];
        lbDisplay.draw_flags |= 0x40;
        LbTextDraw(0, (resvars.f3c_GraphRectp[i + 47].h - font_height) >> 1, D_001602b0[i + 3]);
    }
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    switch (othervar.f2d) {
    case 0:
        scale = 1;
        break;
    case 1:
        scale = 0xc;
        break;
    case 2:
        scale = 0x30;
        break;
    }
    D_0015eb9c = 0;
    for (i = 0; i < 8; i++) {
        if (graph[i].visible == 1) {
            s32 max;
            s32 min;
            u32 range;
            u16 ny;

            for (k = 0; k < 3; k++) {
                for (j = 0; j < 12; j++) {
                    switch (i) {
                    case 0:
                        values[k][j] = mycontrol->history[1][k][j];
                        break;
                    case 1:
                        values[k][j] = mycontrol->history[2][k][j];
                        break;
                    case 2:
                        values[k][j] = mycontrol->history[0][k][j];
                        break;
                    case 3:
                        values[k][j] = mycontrol->history[3][k][j];
                        break;
                    case 4:
                        values[k][j] = mycontrol->history[4][k][j];
                        break;
                    case 5:
                        values[k][j] = mycontrol->history[6][k][j];
                        break;
                    case 6:
                        values[k][j] = mycontrol->history[5][k][j];
                        break;
                    case 7:
                        values[k][j] = mycontrol->history[7][k][j];
                        break;
                    }
                }
            }
            px = x;
            k = 11;
            max = -100000000;
            min = 100000000;
            for (; k >= 0 && (12 - k - 1) * 1500 * scale < actual_game_turn; k--) {
                if (values[othervar.f2d][k] > max)
                    max = values[othervar.f2d][k];
                if (values[othervar.f2d][k] < min)
                    min = values[othervar.f2d][k];
            }
            if (max == min) {
                max <<= 1;
                min = 0;
            }
            range = max - min;
            if (range == 0)
                range = 1;
            if (graph[i].unknown0 == 0)
                py = y - (values[othervar.f2d][11] - min) * h / range;
            else if (graph[i].unknown0 == 1)
                py = y + (values[othervar.f2d][11] - min) * h / range - h;
            fn_00078d70(i, py - font_height / 2, values[othervar.f2d][11]);
            for (k = 11; k > 0 && (12 - k) * 1500 * scale < actual_game_turn; k--) {
                nx = x - (12 - k) * step;
                if (graph[i].unknown0 == 0)
                    ny = y - (values[othervar.f2d][k - 1] - min) * h / range;
                else if (graph[i].unknown0 == 1)
                    ny = y + (values[othervar.f2d][k - 1] - min) * h / range - h;
                if (ny < y - h)
                    ny = y - h;
                else if (ny > y)
                    ny = y;
                fn_000c9be0(px, py, nx, ny, D_0015eb94[graph[i].colour]);
                fn_000c9be0(px - 1, py, nx - 1, ny, D_0015eb94[graph[i].colour]);
                px = nx;
                py = ny;
            }
        }
    }
    fn_00078e38(font_height);
    for (k = 0; k < D_0015eb9c; k++) {
        i = D_0015eba0[k].kind;
        sprintf(buf, "%s:%d", D_001602b0[graph[i].unknown3], D_0015eba0[k].data);
        lbDisplay.draw_colour = D_0015eb94[graph[i].colour];
        lbDisplay.draw_flags |= 0x40;
        LbTextDraw(x + 4, D_0015eba0[k].value, buf);
    }
    month = mycontrol->unknown650;
    year = actual_game_turn / 0x4650;
    quarter = actual_game_turn / 0x4650 - (actual_game_turn / 0x4650 & 3);
    lbDisplay.draw_flags = 0;
    for (i = 0; i < 12; i++) {
        if (i * 1500 * scale >= actual_game_turn)
            break;
        px = x - i * step;
        switch (othervar.f2d) {
        case 0:
            sprintf(buf, "%s", month_text[month]);
            lbDisplay.draw_colour = color.unknown8;
            lbDisplay.draw_flags |= 0x40;
            LbTextDraw(px + 2 - LbTextWordWidth(buf) / 2, y + 10, buf);
            month--;
            if (month < 0)
                month = 0xb;
            fn_000c9be0(px, y + 2, px, y + 6, color.unknown8);
            break;
        case 1:
            sprintf(buf, "%d", year);
            lbDisplay.draw_colour = color.unknown8;
            lbDisplay.draw_flags |= 0x40;
            LbTextDraw(px + 2 - LbTextWordWidth(buf) / 2, y + 10, buf);
            year--;
            fn_000c9be0(px, y + 2, px, y + 6, color.unknown8);
            break;
        case 2:
            sprintf(buf, "%d", quarter);
            lbDisplay.draw_colour = color.unknown8;
            lbDisplay.draw_flags |= 0x40;
            LbTextDraw(px + 2 - LbTextWordWidth(buf) / 2, y + 10, buf);
            quarter += -4;
            fn_000c9be0(px, y + 2, px, y + 6, color.unknown8);
            break;
        }
    }
    lbDisplay.draw_flags = 0;
}
// FUNCTION: 0x00079d30
void process_graph_panel(void)
{
    u8 i;

    if (lbDisplay.rleft_button || lbDisplay.rright_button) {
        for (i = 0; resvars.f48[i + 0x25].width != 0 && resvars.f48[i + 0x25].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f48[i + 0x25].x, resvars.f48[i + 0x25].y,
                             resvars.f48[i + 0x25].width, resvars.f48[i + 0x25].height)) {
                resvars.f48[i + 0x25].callback(i);
                reqspr_draw(resvars.f48[i + 0x25].unknownC, resvars.f48[i + 0x25].unknownE,
                            &screen_sprites_tab_start[resvars.f48[i + 0x25].sprite]);
                break;
            }
        }
        lbDisplay.rleft_button = lbDisplay.rright_button = 0;
    }
    if (lbDisplay.mleft_button || lbDisplay.mright_button) {
        for (i = 0; resvars.f48[i + 0x25].width != 0 && resvars.f48[i + 0x25].height != 0; i++) {
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.f48[i + 0x25].x, resvars.f48[i + 0x25].y,
                             resvars.f48[i + 0x25].width, resvars.f48[i + 0x25].height)) {
                reqspr_draw(resvars.f48[i + 0x25].unknownC, resvars.f48[i + 0x25].unknownE,
                            &screen_sprites_tab_start[resvars.f48[i + 0x25].sprite]);
                break;
            }
        }
    }
}

// FUNCTION: 0x0007a008
void process_graph_button(u8 button)
{
    switch (button) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        if (lbDisplay.rright_button) {
            graph[0].visible = 0;
            graph[1].visible = 0;
            graph[2].visible = 0;
            graph[3].visible = 0;
            graph[4].visible = 0;
            graph[5].visible = 0;
            graph[6].visible = 0;
            graph[7].visible = 0;
            graph[button].visible = 1;
        } else if (graph[button].visible) {
            graph[button].visible = 0;
        } else {
            graph[button].visible = 1;
        }
        play_sound(10, 0x43, -1, -1);
        break;
    case 8:
        switch (othervar.f2d) {
        case 0:
            othervar.f2d = 1;
            break;
        case 1:
            othervar.f2d = 2;
            break;
        case 2:
            othervar.f2d = 0;
            break;
        }
        play_sound(10, 0x43, -1, -1);
        break;
    case 9:
        graphs.unknown2 = 2;
        play_sound(10, 0x42, -1, -1);
        break;
    }
}
