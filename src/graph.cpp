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
extern GraphLabel GraphTextPos[];
extern s16 NumText;
extern u8 colour[];
extern Event D_001549d0[];
extern char **report_text;
void insert_graph_label(s16 kind, s16 value, s32 data);
void spread_graph_labels(u16 gap);
void LbDrawLine(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
s32 LbTextWordWidth(char *text);
}

void process_graph_panel(void);

extern "C" {
}

// Adds a label for graph kind at screen y value (data is the number shown) to
// the graph labels, kept sorted by y.
// FUNCTION: 0x00078d70
void insert_graph_label(s16 kind, s16 value, s32 data)
{
    u8 i;

    i = 0;
    while (GraphTextPos[i].value <= value && i < NumText)
        i++;
    memmove(&GraphTextPos[i + 1], &GraphTextPos[i], (NumText - i) * 7);
    GraphTextPos[i].kind = kind;
    GraphTextPos[i].value = value;
    GraphTextPos[i].data = data;
    NumText++;
}

// Nudges the graph labels apart, a pixel at a time, until neighbours are at
// least gap pixels apart, keeping them within the graph area.
// FUNCTION: 0x00078e38
void spread_graph_labels(u16 gap)
{
    u8 moved;
    s16 i;
    u8 below;
    u8 above;

    moved = 1;
    while (moved) {
        moved = 0;
        for (i = 0; i < NumText; i++) {
            below = i > 0 && GraphTextPos[i].value - GraphTextPos[i - 1].value < gap ? (u8)1 : (u8)0;
            above = i < NumText - 1 && GraphTextPos[i + 1].value - GraphTextPos[i].value < gap ? (u8)1 : (u8)0;
            if (below && !above) {
                if (GraphTextPos[i].value < resvars.screen_boxes_GraphRectp[46].y)
                    GraphTextPos[i].value++;
                moved = 1;
            }
            if (!below && above) {
                if (GraphTextPos[i].value > resvars.screen_boxes_GraphRectp[46].y - resvars.screen_boxes_GraphRectp[46].h - gap)
                    GraphTextPos[i].value--;
                moved = 1;
            }
        }
    }
}

// Runs one frame of the graphs screen (graphs.state: 0 loads it, 1 shows
// it, 2 asks to return to the game): draws each visible statistic's last 12
// points from the local player's history at the chosen time scale
// (othervar.graph_period: months, years or 4-year spans) as a line, from right edge x
// and baseline y, h pixels tall and step pixels between points, with its
// latest value labelled, and the time axis.
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
    switch (graphs.state) {
    case 0:
        load_ingame_pic(SCR_GRAPHS, 1, 0x1f, 0, SCREEN_MODE_13);
        graphs.state = 1;
        lbDisplay.draw_flags = 0;
        break;
    case 1:
        copy_screen_to_wscreen(bscreen);
        process_graph_panel();
        if (exit_quick_requester())
            graphs.state = 2;
        break;
    case 2:
        D_001549d0[player].type = EV_RETURN_FROM_SCREEN;
        D_001549d0[player].value = 0;
        break;
    }
    set_font_from_cache(0xb);
    font_height = lbFontPtr ? lbFontPtr[34].height : 0;
    h2 = lbFontPtr ? lbFontPtr[34].height : 0;
    colour[0] = color.red;
    colour[1] = color.light_red;
    colour[2] = color.orange;
    colour[3] = color.green;
    colour[4] = color.blue;
    colour[5] = color.dark_blue;
    colour[6] = color.purple;
    colour[7] = color.salmon;
    for (i = 0; i < 8; i++) {
        if (!graph[i].visible)
            reqspr_draw(resvars.screen_buttons[i + 0x25].sprite_x, resvars.screen_buttons[i + 0x25].sprite_y,
                        &screen_sprites_tab_start[resvars.screen_buttons[i + 0x25].sprite]);
        LbTextSetWindow(resvars.screen_boxes_GraphRectp[i + 47].x, resvars.screen_boxes_GraphRectp[i + 47].y, resvars.screen_boxes_GraphRectp[i + 47].w, resvars.screen_boxes_GraphRectp[i + 47].h);
        lbDisplay.draw_flags |= 0x140;
        lbDisplay.draw_colour = colour[graph[i].colour];
        lbDisplay.draw_flags |= 0x40;
        LbTextDraw(0, (resvars.screen_boxes_GraphRectp[i + 47].h - font_height) >> 1, report_text[i + 3]);
    }
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    switch (othervar.graph_period) {
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
    NumText = 0;
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
                if (values[othervar.graph_period][k] > max)
                    max = values[othervar.graph_period][k];
                if (values[othervar.graph_period][k] < min)
                    min = values[othervar.graph_period][k];
            }
            if (max == min) {
                max <<= 1;
                min = 0;
            }
            range = max - min;
            if (range == 0)
                range = 1;
            if (graph[i].inverted == 0)
                py = y - (values[othervar.graph_period][11] - min) * h / range;
            else if (graph[i].inverted == 1)
                py = y + (values[othervar.graph_period][11] - min) * h / range - h;
            insert_graph_label(i, py - font_height / 2, values[othervar.graph_period][11]);
            for (k = 11; k > 0 && (12 - k) * 1500 * scale < actual_game_turn; k--) {
                nx = x - (12 - k) * step;
                if (graph[i].inverted == 0)
                    ny = y - (values[othervar.graph_period][k - 1] - min) * h / range;
                else if (graph[i].inverted == 1)
                    ny = y + (values[othervar.graph_period][k - 1] - min) * h / range - h;
                if (ny < y - h)
                    ny = y - h;
                else if (ny > y)
                    ny = y;
                LbDrawLine(px, py, nx, ny, colour[graph[i].colour]);
                LbDrawLine(px - 1, py, nx - 1, ny, colour[graph[i].colour]);
                px = nx;
                py = ny;
            }
        }
    }
    spread_graph_labels(font_height);
    for (k = 0; k < NumText; k++) {
        i = GraphTextPos[k].kind;
        sprintf(buf, "%s:%d", report_text[graph[i].label], GraphTextPos[k].data);
        lbDisplay.draw_colour = colour[graph[i].colour];
        lbDisplay.draw_flags |= 0x40;
        LbTextDraw(x + 4, GraphTextPos[k].value, buf);
    }
    month = mycontrol->month;
    year = actual_game_turn / 0x4650;
    quarter = actual_game_turn / 0x4650 - (actual_game_turn / 0x4650 & 3);
    lbDisplay.draw_flags = 0;
    for (i = 0; i < 12; i++) {
        if (i * 1500 * scale >= actual_game_turn)
            break;
        px = x - i * step;
        switch (othervar.graph_period) {
        case 0:
            sprintf(buf, "%s", month_text[month]);
            lbDisplay.draw_colour = color.black;
            lbDisplay.draw_flags |= 0x40;
            LbTextDraw(px + 2 - LbTextWordWidth(buf) / 2, y + 10, buf);
            month--;
            if (month < 0)
                month = 0xb;
            LbDrawLine(px, y + 2, px, y + 6, color.black);
            break;
        case 1:
            sprintf(buf, "%d", year);
            lbDisplay.draw_colour = color.black;
            lbDisplay.draw_flags |= 0x40;
            LbTextDraw(px + 2 - LbTextWordWidth(buf) / 2, y + 10, buf);
            year--;
            LbDrawLine(px, y + 2, px, y + 6, color.black);
            break;
        case 2:
            sprintf(buf, "%d", quarter);
            lbDisplay.draw_colour = color.black;
            lbDisplay.draw_flags |= 0x40;
            LbTextDraw(px + 2 - LbTextWordWidth(buf) / 2, y + 10, buf);
            quarter += -4;
            LbDrawLine(px, y + 2, px, y + 6, color.black);
            break;
        }
    }
    lbDisplay.draw_flags = 0;
}
// Handles clicks on the graphs screen's buttons, drawing the one under the
// mouse pressed.
// FUNCTION: 0x00079d30
void process_graph_panel(void)
{
    u8 i;

    if (lbDisplay.rleft_button || lbDisplay.rright_button) {
        for (i = 0; resvars.screen_buttons[i + 0x25].width != 0 && resvars.screen_buttons[i + 0x25].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_buttons[i + 0x25].x, resvars.screen_buttons[i + 0x25].y,
                             resvars.screen_buttons[i + 0x25].width, resvars.screen_buttons[i + 0x25].height)) {
                resvars.screen_buttons[i + 0x25].callback(i);
                reqspr_draw(resvars.screen_buttons[i + 0x25].sprite_x, resvars.screen_buttons[i + 0x25].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i + 0x25].sprite]);
                break;
            }
        }
        lbDisplay.rleft_button = lbDisplay.rright_button = 0;
    }
    if (lbDisplay.mleft_button || lbDisplay.mright_button) {
        for (i = 0; resvars.screen_buttons[i + 0x25].width != 0 && resvars.screen_buttons[i + 0x25].height != 0; i++) {
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.screen_buttons[i + 0x25].x, resvars.screen_buttons[i + 0x25].y,
                             resvars.screen_buttons[i + 0x25].width, resvars.screen_buttons[i + 0x25].height)) {
                reqspr_draw(resvars.screen_buttons[i + 0x25].sprite_x, resvars.screen_buttons[i + 0x25].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i + 0x25].sprite]);
                break;
            }
        }
    }
}

// Graphs screen button: 0-7 show or hide a statistic (right click shows only
// that one), 8 cycles the time scale, 9 closes the screen.
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
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    case 8:
        switch (othervar.graph_period) {
        case 0:
            othervar.graph_period = 1;
            break;
        case 1:
            othervar.graph_period = 2;
            break;
        case 2:
            othervar.graph_period = 0;
            break;
        }
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    case 9:
        graphs.state = 2;
        play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        break;
    }
}
