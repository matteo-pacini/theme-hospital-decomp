#include "hosp.h"

struct GlassBox {
    s16 x;
    s16 y;
    s16 x1;
    s16 y1;
    u8 f8;
    u8 unknown9[5];
    // 0xe: Box style for draw_a_box: -1 or 0 background (0 translucent), 1 or 2
    // foreground.
    u8 type;
    // 0xf: Nonzero: drawn as a button.
    u8 button;
    // 0x10: Length of the zoom animation in frames.
    u8 steps;
    // 0x11: Frames before opening and closing, unless add_glass_box is given
    // its own.
    u8 delay;
    // 0x12: Where the zoom grows from: 0 the box's centre, 1 its top centre, 2
    // the screen centre.
    u8 origin;
};

struct SlistButton {
    u16 x;
    u16 y;
    u16 x1;
    u16 y1;
};

struct GlassFrame {
    s16 x;
    s16 y;
    s16 x1;
    s16 y1;
    u8 unknown8[11];
};

struct GlassScreen {
    u8 data[0x24d];
};

struct ZoomBox {
    u8 active;
    // 0x1: Animation step: 0 closed, steps - 1 fully open.
    s8 step;
    // 0x2: Animation length.
    s8 steps;
    // 0x3: Frames left before opening starts.
    u8 open_delay;
    // 0x4: Frames left before closing starts.
    u8 close_delay;
    s32 id;
    s8 type;
    u8 fa;
    // 0xb: Nonzero: drawn as a button.
    u8 button;
    s32 from[4];
    s32 to[4];
};

struct BalanceFileInfo {
    BalanceFileInfo *prev;
    BalanceFileInfo *next;
};

extern s32 CurrentDisplay;
extern s32 DesiredDisplay;
extern s32 PendingDisplay;
extern s32 RestoreDisplay;
extern ZoomBox ZoomBoxes[];
extern u8 playing_open_sound;
extern u8 playing_close_sound;
extern BalanceFileInfo *current_balance_file;
extern u32 b_b_colour;
extern u32 white_colour;
extern u32 black_colour;
extern u8 Gray[];

extern "C" {

void StopSample(u32 source_id, u32 number);
void LbTextSetJustifyWindow(s32 x, s32 y, s32 width);
void LbDrawLine(int x1, int y1, int x2, int y2, u8 colour);
}

s8 add_glass_box(GlassBox *box, s32 id, s8 open_delay, s8 close_delay);
void add_glass_frame(GlassFrame *frames, s32 mask);
void draw_a_box(u8 zoom);
s16 glass_index(s32 mask);
void draw_background_box(s16 x, s16 y, s16 w, s16 h, u8 style);
void draw_foreground_box(s16 x, s16 y, s16 w, s16 h);
void draw_foreground_box2(s16 x, s16 y, s16 w, s16 h);
void draw_button_box(s16 x, s16 y, s16 w, s16 h);
void draw_button_edges(s16 x, s16 y, s16 w, s16 h, u8 style);

// Resets the menu screen panels (zoom boxes) and opens those of each display
// bit in mask (screen layout sets by glass_index).
// FUNCTION: 0x000b8ab0
void init_zoom_boxes(s32 mask)
{
    s32 bit;
    s32 idx;

    PendingDisplay = DesiredDisplay = CurrentDisplay = RestoreDisplay = 0;
    memset(ZoomBoxes, 0, 0x7e8);
    while (mask) {
        bit = mask & -mask;
        mask ^= bit;
        idx = glass_index(bit);
        if (idx) {
            add_glass_frame((GlassFrame *)&resvars.glass_frames_GlassScreenp[idx], bit);
            DesiredDisplay |= bit;
        }
    }
}

// Returns 1 if zoom box zoom already ends at box's rectangle, else 0.
// FUNCTION: 0x000b8b50
extern "C" s32 glass_box_matches_zoom(GlassBox *box, ZoomBox *zoom)
{
    if (box->x != zoom->to[0])
        return 0;
    if (box->y != zoom->to[1])
        return 0;
    if (box->x1 - box->x + 1 != zoom->to[2])
        return 0;
    if (box->y1 - box->y + 1 != zoom->to[3])
        return 0;
    return 1;
}

// Sets up the first free zoom box to grow into box's rectangle for display bit
// id, from its centre, its top centre or the screen centre (box->origin), with
// open_delay and close_delay frames of delay (-1: the box's own). An unused
// slot that already ends there is reused unchanged. Returns the slot index
// (0x2e if none was free).
// FUNCTION: 0x000b8bf0
s8 add_glass_box(GlassBox *box, s32 id, s8 open_delay, s8 close_delay)
{
    s16 i;

    for (i = 0; i < 0x2e; i++) {
        if (!ZoomBoxes[i].active)
            break;
    }
    if (i < 0x2e) {
        ZoomBoxes[i].active = 1;
        if (glass_box_matches_zoom(box, &ZoomBoxes[i]))
            return i;
        ZoomBoxes[i].step = 0;
        ZoomBoxes[i].steps = box->steps;
        ZoomBoxes[i].id = id;
        ZoomBoxes[i].type = box->type;
        ZoomBoxes[i].fa = box->f8;
        ZoomBoxes[i].button = box->button;
        if (open_delay == -1)
            ZoomBoxes[i].open_delay = box->delay;
        else
            ZoomBoxes[i].open_delay = open_delay;
        if (close_delay == -1)
            ZoomBoxes[i].close_delay = box->delay;
        else
            ZoomBoxes[i].close_delay = close_delay;
        switch (box->origin) {
        case 1:
            ZoomBoxes[i].from[0] = box->x + (box->x1 - box->x) / 2;
            ZoomBoxes[i].from[1] = box->y;
            ZoomBoxes[i].from[2] = 0;
            ZoomBoxes[i].from[3] = box->y1 - box->y + 1;
            break;
        case 0:
            ZoomBoxes[i].from[0] = box->x + (box->x1 - box->x) / 2;
            ZoomBoxes[i].from[1] = box->y + (box->y1 - box->y) / 2;
            ZoomBoxes[i].from[2] = 0;
            ZoomBoxes[i].from[3] = 0;
            break;
        case 2:
            ZoomBoxes[i].from[0] = lbDisplay.graphics_screen_width / 2;
            ZoomBoxes[i].from[1] = lbDisplay.graphics_screen_height / 2;
            ZoomBoxes[i].from[2] = 0;
            ZoomBoxes[i].from[3] = 0;
            break;
        default:
            break;
        }
        ZoomBoxes[i].to[0] = box->x;
        ZoomBoxes[i].to[1] = box->y;
        ZoomBoxes[i].to[2] = box->x1 - box->x + 1;
        ZoomBoxes[i].to[3] = box->y1 - box->y + 1;
    } else {
        hosp_dos_printf("Not enough ZoomBoxes!\n");
    }
    return i;
}

// Adds a zoom box for each rectangle in frames (ended by x1 0) under the
// lowest display bit of mask, and asks for that display to open.
// FUNCTION: 0x000b8f64
void add_glass_frame(GlassFrame *frames, s32 mask)
{
    s16 i;

    mask &= -mask;
    i = 0;
    while (frames[i].x1)
        add_glass_box((GlassBox *)&frames[i++], mask, -1, -1);
    DesiredDisplay |= mask;
}

// Draws zoom box zoom at its current animation step, as a panel or a
// button by its type (type 0 panels translucent, draw flag 4); nothing at
// step 0.
// FUNCTION: 0x000b8fcc
void draw_a_box(u8 zoom)
{
    s32 *from;
    s32 *to;
    s32 step;
    s32 total;
    s16 x;
    s16 y;
    s16 w;
    s16 h;

    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 4;
    from = ZoomBoxes[zoom].from;
    to = ZoomBoxes[zoom].to;
    step = ZoomBoxes[zoom].step;
    total = ZoomBoxes[zoom].steps - 1;
    x = (to[0] * step + (total - step) * from[0]) / total;
    y = ((total - step) * from[1] + to[1] * step) / total;
    w = ((total - step) * from[2] + to[2] * step) / total;
    h = ((total - step) * from[3] + to[3] * step) / total;
    if (step != 0) {
        lbDisplay.draw_flags = 0;
        if (ZoomBoxes[zoom].button) {
            if (!((((s8)ZoomBoxes[zoom].type != 2 && display_mode == DM_NICKNAME) ||
                   (to[0] == 0x213 && display_mode == DM_NETWORK_SELECT)) &&
                  current_balance_file && current_balance_file == current_balance_file->next)) {
                draw_button_box(x, y, w, h);
                draw_button_edges(x, y, w - 1, h - 1, 0);
            }
        } else {
            switch (ZoomBoxes[zoom].type) {
            case -1:
                draw_background_box(x, y, w, h, 1);
                break;
            case 0:
                lbDisplay.draw_flags = 4;
                draw_background_box(x, y, w, h, 0);
                break;
            case 1:
                draw_foreground_box(x, y, w, h);
                break;
            case 2:
                draw_foreground_box2(x, y, w, h);
                break;
            default:
                draw_foreground_box2(x, y, w, h);
                break;
            }
        }
    }
}

// Steps the zoom box animations one frame and draws them: boxes of displays
// no longer wanted close, then those wanted open, the highest layer first; a
// display joins CurrentDisplay once all its boxes are open. Plays the opening
// and closing sounds while they run. Returns 1 while any box is moving.
// FUNCTION: 0x000b9284
s8 draw_zoom_boxes(void)
{
    s8 lo;
    s8 hi;
    u8 incomplete;
    u8 found;
    u8 result;
    u8 opening;
    u8 closing;
    s32 i;
    s32 j;

    lo = -128;
    hi = -128;
    result = 0;
    opening = 0;
    closing = 0;
    incomplete = 0;
    found = 0;
    for (i = 0; i < 0x2e; i++) {
        if (ZoomBoxes[i].active) {
            if ((ZoomBoxes[i].step > 0 && ZoomBoxes[i].step < ZoomBoxes[i].steps - 1) ||
                (ZoomBoxes[i].step && !(DesiredDisplay & ZoomBoxes[i].id))) {
                found = 1;
                if (ZoomBoxes[i].type > hi)
                    hi = ZoomBoxes[i].type;
            }
            if (ZoomBoxes[i].step == 0) {
                if (!found) {
                    hi = ZoomBoxes[i].type;
                    found = 1;
                }
            }
            if (ZoomBoxes[i].step > 0 && ZoomBoxes[i].step <= ZoomBoxes[i].steps - 1) {
                if (ZoomBoxes[i].type > lo)
                    lo = ZoomBoxes[i].type;
            }
            if (!(DesiredDisplay & ZoomBoxes[i].id))
                incomplete = 1;
        }
    }
    if (hi == -128)
        hi = lo;
    else if (hi > lo)
        lo = hi;
    for (i = 0; i < 0x2e; i++) {
        if (ZoomBoxes[i].active && ZoomBoxes[i].type <= lo) {
            if (ZoomBoxes[i].id & DesiredDisplay) {
                if (ZoomBoxes[i].id & CurrentDisplay) {
                    draw_a_box(i);
                } else if (!incomplete && ZoomBoxes[i].type <= hi) {
                    if (ZoomBoxes[i].type == hi) {
                        if (!ZoomBoxes[i].open_delay) {
                            if (++ZoomBoxes[i].step >= ZoomBoxes[i].steps - 1) {
                                for (j = 0; j < 0x2e; j++) {
                                    if (((ZoomBoxes[j].active && i != j) && ZoomBoxes[j].id == ZoomBoxes[i].id) &&
                                        ZoomBoxes[j].step != ZoomBoxes[j].steps - 1)
                                        break;
                                }
                                if (j == 0x2e)
                                    CurrentDisplay |= ZoomBoxes[i].id;
                                if (ZoomBoxes[i].step > ZoomBoxes[i].steps - 1)
                                    ZoomBoxes[i].step--;
                            } else {
                                opening = 1;
                                result = 1;
                            }
                        } else {
                            ZoomBoxes[i].open_delay--;
                        }
                    }
                    draw_a_box(i);
                }
            } else if (ZoomBoxes[i].type == hi) {
                if (!ZoomBoxes[i].close_delay) {
                    CurrentDisplay &= ~ZoomBoxes[i].id;
                    result = 1;
                    closing = 1;
                    if (--ZoomBoxes[i].step <= 0)
                        ZoomBoxes[i].active = 0;
                } else {
                    ZoomBoxes[i].close_delay--;
                }
                draw_a_box(i);
            } else {
                draw_a_box(i);
            }
        }
    }
    if (opening && !playing_open_sound) {
        playing_open_sound = 1;
        play_sound(SNDSRC_GENERAL, SFX_ZOOM_OPEN, -1, -1);
    } else if (!opening) {
        playing_open_sound = 0;
        StopSample(SNDSRC_GENERAL, 0x9c);
    }
    if (closing && !playing_close_sound) {
        playing_close_sound = 1;
        play_sound(SNDSRC_GENERAL, SFX_ZOOM_CLOSE, -1, -1);
    } else if (!closing) {
        playing_close_sound = 0;
        StopSample(SNDSRC_GENERAL, 0x9e);
    }
    return result;
}

// Returns the display bits to change next: those to close first, then (all at
// once if bit 8 is involved) the lowest one to open; 0 when CurrentDisplay
// matches DesiredDisplay.
// FUNCTION: 0x000b96cc
s32 next_map(void)
{
    s32 diff;
    s32 closing;

    diff = CurrentDisplay ^ DesiredDisplay;
    closing = diff & CurrentDisplay;
    if (closing)
        return closing;
    if ((CurrentDisplay | DesiredDisplay) & 8)
        return CurrentDisplay ^ DesiredDisplay;
    return diff & -diff;
}

// Returns the resvars.glass_frames screen layout index of the panels for the lowest
// display bit in mask, or 0 if it has none.
// FUNCTION: 0x000b973c
s16 glass_index(s32 mask)
{
    switch ((u32)(mask & -mask)) {
    case 4:
        return 1;
    case 0x80:
        return 2;
    case 0x40:
        return 5;
    case 0x10:
        return 3;
    case 0x20:
        return 4;
    case 0x100:
        return 6;
    case 0x200:
        return 7;
    case 0x400:
        return 8;
    case 1:
        return 10;
    case 2:
        return 11;
    case 0x800:
        return 9;
    case 0x2000:
        return 12;
    case 0x4000:
        return 13;
    case 0x8000:
    case 0x10000:
        return 14;
    default:
        return 0;
    }
}

// Returns the resvars.screen_buttons index of the first button for the lowest display bit
// in mask, or 0 if it has none.
// FUNCTION: 0x000b991c
s16 button_index(s32 mask)
{
    switch ((u32)(mask & -mask)) {
    case 0x80:
        return 0x59;
    case 0x100:
        return 0x5e;
    case 0x200:
        return 0x61;
    case 0x10:
        return 0x65;
    case 0x20:
        return 0x6d;
    case 0x40:
        return 0x74;
    case 0x400:
        return 0x80;
    case 0x1000:
        return 0x8c;
    case 8:
        return 0x93;
    case 0x800:
        return 0x7e;
    case 1:
        return 0xa0;
    case 0x2000:
        return 0x95;
    case 0x4000:
        return 0xad;
    case 0x8000:
    case 0x10000:
        return 0x7b;
    default:
        return 0;
    }
}

// Draws text centred on button, in the pressed font and shifted by a pixel
// while the mouse holds it down.
// FUNCTION: 0x000b9b04
void draw_glass_text(SlistButton *button, char *text)
{
    s16 x;

    lbDisplay.draw_flags &= 4;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetJustifyWindow(button->x, 0, button->x1 - button->x + 1);
    if ((lbDisplay.mleft_button && over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, button->x, button->y,
                                                button->x1 - button->x + 1, button->y1 - button->y + 1)) ||
        (lbDisplay.rleft_button && over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, button->x, button->y,
                                                button->x1 - button->x + 1, button->y1 - button->y + 1))) {
        set_font_from_cache(0x22);
        x = (button->x1 - button->x + 1 - LbTextStringWidth(text)) >> 1;
        LbTextDraw(x + 1, ((button->y1 + button->y + 1 - (lbFontPtr ? lbFontPtr[34].height : 0)) >> 1) + 1, text);
    } else {
        set_font_from_cache(0x21);
        x = (button->x1 - button->x + 1 - LbTextStringWidth(text)) >> 1;
        LbTextDraw(x, (button->y1 + button->y + 1 - (lbFontPtr ? lbFontPtr[34].height : 0)) >> 1, text);
    }
}

// Draws text centred on button in the highlighted font, shifted as if pressed.
// FUNCTION: 0x000b9d20
void draw_glass_text_hilight(SlistButton *button, char *text)
{
    s16 x;

    lbDisplay.draw_flags &= 4;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetJustifyWindow(button->x, 0, button->x1 - button->x + 1);
    set_font_from_cache(0x22);
    x = (button->x1 - button->x + 1 - LbTextStringWidth(text)) >> 1;
    LbTextDraw(x + 1, ((button->y1 + button->y + 1 - (lbFontPtr ? lbFontPtr[34].height : 0)) >> 1) + 1, text);
}

// Draws text centred on button in the highlighted font, shifted only while
// the mouse holds it down.
// FUNCTION: 0x000b9e14
void draw_glass_text_hilight_no_jump(SlistButton *button, char *text)
{
    s16 x;

    lbDisplay.draw_flags &= 4;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetJustifyWindow(button->x, 0, button->x1 - button->x + 1);
    set_font_from_cache(0x22);
    x = (button->x1 - button->x + 1 - LbTextStringWidth(text)) >> 1;
    if ((lbDisplay.mleft_button && over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, button->x, button->y,
                                                button->x1 - button->x + 1, button->y1 - button->y + 1)) ||
        (lbDisplay.rleft_button && over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, button->x, button->y,
                                                button->x1 - button->x + 1, button->y1 - button->y + 1)))
        LbTextDraw(x + 1, ((button->y1 + button->y + 1 - (lbFontPtr ? lbFontPtr[34].height : 0)) >> 1) + 1, text);
    else
        LbTextDraw(x, (button->y1 + button->y + 1 - (lbFontPtr ? lbFontPtr[34].height : 0)) >> 1, text);
}

// Draws a panel at x,y of size w,h: a white fill (with the draw flags in
// effect), a black outline (draw flag 0x10) and a 3-pixel border in the
// blue-grey (style 0) or colour 0xca.
// FUNCTION: 0x000b9ffc
void draw_background_box(s16 x, s16 y, s16 w, s16 h, u8 style)
{
    u8 colour;

    LbDrawBox(x, y, w, h, white_colour);
    lbDisplay.draw_flags = 0x10;
    LbDrawBox(x, y, w, h, black_colour);
    if (style)
        colour = 0xca;
    else
        colour = b_b_colour;
    LbDrawBox(x + 1, y + 1, w - 2, h - 2, colour);
    LbDrawBox(x + 2, y + 2, w - 4, h - 4, colour);
    LbDrawBox(x + 3, y + 3, w - 6, h - 6, colour);
    lbDisplay.draw_flags = 0;
}

// Draws a panel at x,y of size w,h: a white fill, an outline in colour 0xf8
// and a 3-pixel blue-grey border.
// FUNCTION: 0x000ba0ec
void draw_foreground_box(s16 x, s16 y, s16 w, s16 h)
{
    lbDisplay.draw_flags = 0;
    LbDrawBox(x, y, w, h, white_colour);
    lbDisplay.draw_flags = 0x10;
    LbDrawBox(x, y, w, h, 0xf8);
    LbDrawBox(x + 1, y + 1, w - 2, h - 2, b_b_colour);
    LbDrawBox(x + 2, y + 2, w - 4, h - 4, b_b_colour);
    LbDrawBox(x + 3, y + 3, w - 6, h - 6, b_b_colour);
    lbDisplay.draw_flags = 0;
}

// Draws a panel at x,y of size w,h: a white fill and a 2-pixel blue-grey
// border.
// FUNCTION: 0x000ba1d4
void draw_foreground_box2(s16 x, s16 y, s16 w, s16 h)
{
    lbDisplay.draw_flags = 0;
    LbDrawBox(x, y, w, h, white_colour);
    lbDisplay.draw_flags = 0x10;
    LbDrawBox(x, y, w, h, b_b_colour);
    LbDrawBox(x + 1, y + 1, w - 2, h - 2, b_b_colour);
    lbDisplay.draw_flags = 0;
}

// Draws a button face at x,y of size w,h: a white fill and an outline in
// colour 0xf8.
// FUNCTION: 0x000ba270
void draw_button_box(s16 x, s16 y, s16 w, s16 h)
{
    lbDisplay.draw_flags = 0;
    LbDrawBox(x, y, w, h, white_colour);
    lbDisplay.draw_flags = 0x10;
    LbDrawBox(x, y, w, h, 0xf8);
    lbDisplay.draw_flags = 0;
}

// Draws the bevelled edges of a button at x,y (inner size w,h): raised with
// style 0, sunken otherwise.
// FUNCTION: 0x000ba2e0
void draw_button_edges(s16 x, s16 y, s16 w, s16 h, u8 style)
{
    lbDisplay.draw_flags = 0;
    if (!style) {
        LbDrawLine(x + 1, y + 1, x + w - 1, y + 1, Gray[2]);
        LbDrawLine(x + 2, y + 2, x + w - 2, y + 2, Gray[3]);
        LbDrawLine(x + 3, y + 3, x + w - 3, y + 3, Gray[4]);
        LbDrawLine(x + 1, y + 2, x + 1, y + h - 1, Gray[2]);
        LbDrawLine(x + 2, y + 3, x + 2, y + h - 2, Gray[3]);
        LbDrawLine(x + 3, y + 4, x + 3, y + h - 3, Gray[4]);
        LbDrawLine(x + 2, y + h - 1, x + w - 1, y + h - 1, Gray[5]);
        LbDrawLine(x + 3, y + h - 2, x + w - 2, y + h - 2, Gray[6]);
        LbDrawLine(x + 4, y + h - 3, x + w - 3, y + h - 3, Gray[7]);
        LbDrawLine(x + w - 1, y + 2, x + w - 1, y + h - 2, Gray[5]);
        LbDrawLine(x + w - 2, y + 3, x + w - 2, y + h - 3, Gray[6]);
        LbDrawLine(x + w - 3, y + 4, x + w - 3, y + h - 4, Gray[7]);
    } else {
        LbDrawLine(x + 1, y + 1, x + w - 1, y + 1, Gray[5]);
        LbDrawLine(x + 2, y + 2, x + w - 2, y + 2, Gray[6]);
        LbDrawLine(x + 3, y + 3, x + w - 3, y + 3, Gray[7]);
        LbDrawLine(x + 1, y + 2, x + 1, y + h - 1, Gray[5]);
        LbDrawLine(x + 2, y + 3, x + 2, y + h - 2, Gray[6]);
        LbDrawLine(x + 3, y + 4, x + 3, y + h - 3, Gray[7]);
        LbDrawLine(x + 2, y + h - 1, x + w - 1, y + h - 1, Gray[2]);
        LbDrawLine(x + 3, y + h - 2, x + w - 2, y + h - 2, Gray[3]);
        LbDrawLine(x + 4, y + h - 3, x + w - 3, y + h - 3, Gray[4]);
        LbDrawLine(x + w - 1, y + 2, x + w - 1, y + h - 2, Gray[2]);
        LbDrawLine(x + w - 2, y + 3, x + w - 2, y + h - 3, Gray[3]);
        LbDrawLine(x + w - 3, y + 4, x + w - 3, y + h - 4, Gray[4]);
    }
}

// Finds the menu panel colours (blue-grey, white, black) in palette and sets
// the button bevel greys.
// FUNCTION: 0x000ba800
void calculate_grey(u8 *palette)
{
    b_b_colour = LbPaletteFindColour(palette, 3, 0x2f, 0x3b);
    white_colour = LbPaletteFindColour(palette, 0x3f, 0x3f, 0x3f);
    black_colour = LbPaletteFindColour(palette, 0, 0, 0);
    Gray[0] = 0;
    Gray[1] = 0xff;
    Gray[2] = 0xf5;
    Gray[3] = 0xf5;
    Gray[4] = 0xf5;
    Gray[5] = 0xf8;
    Gray[6] = 0xf8;
    Gray[7] = 0xf6;
}
