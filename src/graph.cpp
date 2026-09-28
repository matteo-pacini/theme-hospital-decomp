#include "hosp.h"

struct Graphs {
    u8 unknown0;
    u8 visible;
    u8 colour;
    u8 unknown3;
};

extern Graphs graph[];

extern "C" {
extern Button *D_0015f43c;
extern u8 D_00188c7e;
extern u8 D_00188c7f;
extern u8 D_00188c81;
extern u8 D_00188c83;
extern volatile s16 D_00188c6c;
extern volatile s16 D_00188c70;
extern u16 D_001553e5;
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
extern GraphRect *D_0015f430;
extern s32 D_00188c38;
extern GraphLabel D_0015eba0[];
extern s16 D_0015eb9c;
extern u8 D_0015eb94[];
extern Event D_001549d0[];
extern u16 D_00188c74;
extern u8 D_00188c85;
extern char **D_001602b0;
extern char D_001205f8[];
extern char D_001205fe[];
extern char D_00120601[];
extern char D_00120604[];
void fn_00078d70(s16 kind, s16 value, s32 data);
void fn_00078e38(u16 gap);
void fn_000c9be0(s32 x0, s32 y0, s32 x1, s32 y1, u8 colour);
s32 LbTextWordWidth(char *text);
}

void process_graph_panel(void);

extern "C" {
}

// FUNCTION: 0x00078fc4
void draw_graphs(u16 x, u16 y, u16 h, u16 step)
{
    s8 scale;
    s8 k;
    s8 m;
    s8 j;
    s8 i;
    u16 px;
    u16 nx;
    u16 h1;
    u16 h2;
    s16 year;
    u16 py;
    s16 quarter;
    s32 values[3][12];
    char buf[128];

    compute_help_text(0, 0, 0x11);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    switch (graphs.unknown2) {
    case 0:
        load_ingame_pic(4, 1, 0x1f, 0, SCREEN_MODE_13);
        graphs.unknown2 = 1;
        D_00188c74 = 0;
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
    h1 = lbFontPtr ? lbFontPtr[34].height : 0;
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
            reqspr_draw(D_0015f43c[i + 0x25].unknownC, D_0015f43c[i + 0x25].unknownE,
                        &screen_sprites_tab_start[D_0015f43c[i + 0x25].sprite]);
        LbTextSetWindow(D_0015f430[i + 47].x, D_0015f430[i + 47].y, D_0015f430[i + 47].w, D_0015f430[i + 47].h);
        D_00188c74 |= 0x140;
        D_00188c85 = D_0015eb94[graph[i].colour];
        D_00188c74 |= 0x40;
        LbTextDraw(0, (D_0015f430[i + 47].h - h1) >> 1, D_001602b0[i + 3]);
    }
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    switch (D_001553e5) {
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
                if (values[D_001553e5][k] > max)
                    max = values[D_001553e5][k];
                if (values[D_001553e5][k] < min)
                    min = values[D_001553e5][k];
            }
            if (max == min) {
                max <<= 1;
                min = 0;
            }
            range = max - min;
            if (range == 0)
                range = 1;
            if (graph[i].unknown0 == 0)
                py = y - (values[D_001553e5][11] - min) * h / range;
            else if (graph[i].unknown0 == 1)
                py = y + (values[D_001553e5][11] - min) * h / range - h;
            fn_00078d70(i, py - h1 / 2, values[D_001553e5][11]);
            for (k = 11; k > 0 && (12 - k) * 1500 * scale < actual_game_turn; k--) {
                nx = x - (12 - k) * step;
                if (graph[i].unknown0 == 0)
                    ny = y - (values[D_001553e5][k - 1] - min) * h / range;
                else if (graph[i].unknown0 == 1)
                    ny = y + (values[D_001553e5][k - 1] - min) * h / range - h;
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
    fn_00078e38(h1);
    for (k = 0; k < D_0015eb9c; k++) {
        i = D_0015eba0[k].kind;
        sprintf(buf, D_001205f8, D_001602b0[graph[i].unknown3], D_0015eba0[k].data);
        D_00188c85 = D_0015eb94[graph[i].colour];
        D_00188c74 |= 0x40;
        LbTextDraw(x + 4, D_0015eba0[k].value, buf);
    }
    m = mycontrol->unknown650;
    year = actual_game_turn / 0x4650;
    quarter = actual_game_turn / 0x4650 - (actual_game_turn / 0x4650 & 3);
    D_00188c74 = 0;
    for (i = 0; i < 12; i++) {
        if (i * 1500 * scale >= actual_game_turn)
            break;
        px = x - i * step;
        switch (D_001553e5) {
        case 0:
            sprintf(buf, D_001205fe, month_text[m]);
            D_00188c85 = color.unknown8;
            D_00188c74 |= 0x40;
            LbTextDraw(px + 2 - LbTextWordWidth(buf) / 2, y + 10, buf);
            m--;
            if (m < 0)
                m = 0xb;
            fn_000c9be0(px, y + 2, px, y + 6, color.unknown8);
            break;
        case 1:
            sprintf(buf, D_00120601, year);
            D_00188c85 = color.unknown8;
            D_00188c74 |= 0x40;
            LbTextDraw(px + 2 - LbTextWordWidth(buf) / 2, y + 10, buf);
            year--;
            fn_000c9be0(px, y + 2, px, y + 6, color.unknown8);
            break;
        case 2:
            sprintf(buf, D_00120604, quarter);
            D_00188c85 = color.unknown8;
            D_00188c74 |= 0x40;
            LbTextDraw(px + 2 - LbTextWordWidth(buf) / 2, y + 10, buf);
            quarter += -4;
            fn_000c9be0(px, y + 2, px, y + 6, color.unknown8);
            break;
        }
    }
    D_00188c74 = 0;
}
// FUNCTION: 0x00079d30
void process_graph_panel(void)
{
    u8 i;

    if (D_00188c81 || D_00188c83) {
        for (i = 0; D_0015f43c[i + 0x25].width != 0 && D_0015f43c[i + 0x25].height != 0; i++) {
            if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i + 0x25].x, D_0015f43c[i + 0x25].y,
                             D_0015f43c[i + 0x25].width, D_0015f43c[i + 0x25].height)) {
                D_0015f43c[i + 0x25].callback(i);
                reqspr_draw(D_0015f43c[i + 0x25].unknownC, D_0015f43c[i + 0x25].unknownE,
                            &screen_sprites_tab_start[D_0015f43c[i + 0x25].sprite]);
                break;
            }
        }
        D_00188c81 = D_00188c83 = 0;
    }
    if (D_00188c7e || D_00188c7f) {
        for (i = 0; D_0015f43c[i + 0x25].width != 0 && D_0015f43c[i + 0x25].height != 0; i++) {
            if (over_hotspot((s16)mouse_dx, (s16)mouse_dy, D_0015f43c[i + 0x25].x, D_0015f43c[i + 0x25].y,
                             D_0015f43c[i + 0x25].width, D_0015f43c[i + 0x25].height)) {
                reqspr_draw(D_0015f43c[i + 0x25].unknownC, D_0015f43c[i + 0x25].unknownE,
                            &screen_sprites_tab_start[D_0015f43c[i + 0x25].sprite]);
                break;
            }
        }
    }
}

// FUNCTION: 0x0007a008
void process_graph_button(u8 n)
{
    switch (n) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        if (D_00188c83) {
            graph[0].visible = 0;
            graph[1].visible = 0;
            graph[2].visible = 0;
            graph[3].visible = 0;
            graph[4].visible = 0;
            graph[5].visible = 0;
            graph[6].visible = 0;
            graph[7].visible = 0;
            graph[n].visible = 1;
        } else if (graph[n].visible) {
            graph[n].visible = 0;
        } else {
            graph[n].visible = 1;
        }
        play_sound(10, 0x43, -1, -1);
        break;
    case 8:
        switch (D_001553e5) {
        case 0:
            D_001553e5 = 1;
            break;
        case 1:
            D_001553e5 = 2;
            break;
        case 2:
            D_001553e5 = 0;
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
