#include "hosp.h"

struct GlassBox {
    s16 x;
    s16 y;
    s16 x1;
    s16 y1;
    u8 f8;
    u8 unknown9[5];
    u8 fe;
    u8 ff;
    u8 f10;
    u8 f11;
    u8 f12;
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
    s8 f1;
    s8 f2;
    u8 f3;
    u8 f4;
    s32 id;
    s8 type;
    u8 fa;
    u8 fb;
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
extern GlassScreen *D_0015f444;
extern s32 D_00188c38;
extern u16 D_00188c74;
extern volatile s16 D_00188c6c, D_00188c70;
extern u8 D_00188c7e, D_00188c81;
extern char D_00121bd0[];

void StopSample(u32 a, u32 b);
void LbTextSetJustifyWindow(s32 x, s32 y, s32 w);
void fn_000c9be0(int x0, int y0, int x1, int y1, u8 color);
}

s8 add_glass_box(GlassBox *b, s32 id, s8 c1, s8 c2);
void add_glass_frame(GlassFrame *f, s32 mask);
void draw_a_box(u8 n);
s16 glass_index(s32 mask);
void draw_background_box(s16 x, s16 y, s16 w, s16 h, u8 style);
void draw_foreground_box(s16 x, s16 y, s16 w, s16 h);
void draw_foreground_box2(s16 x, s16 y, s16 w, s16 h);
void draw_button_box(s16 x, s16 y, s16 w, s16 h);
void draw_button_edges(s16 x, s16 y, s16 w, s16 h, u8 style);

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
            add_glass_frame((GlassFrame *)&D_0015f444[idx], bit);
            DesiredDisplay |= bit;
        }
    }
}

// FUNCTION: 0x000b8b50
extern "C" s32 fn_000b8b50(GlassBox *b, ZoomBox *z)
{
    if (b->x != z->to[0])
        return 0;
    if (b->y != z->to[1])
        return 0;
    if (b->x1 - b->x + 1 != z->to[2])
        return 0;
    if (b->y1 - b->y + 1 != z->to[3])
        return 0;
    return 1;
}

// FUNCTION: 0x000b8bf0
s8 add_glass_box(GlassBox *b, s32 id, s8 c1, s8 c2)
{
    s16 i;

    for (i = 0; i < 0x2e; i++) {
        if (!ZoomBoxes[i].active)
            break;
    }
    if (i < 0x2e) {
        ZoomBoxes[i].active = 1;
        if (fn_000b8b50(b, &ZoomBoxes[i]))
            return i;
        ZoomBoxes[i].f1 = 0;
        ZoomBoxes[i].f2 = b->f10;
        ZoomBoxes[i].id = id;
        ZoomBoxes[i].type = b->fe;
        ZoomBoxes[i].fa = b->f8;
        ZoomBoxes[i].fb = b->ff;
        if (c1 == -1)
            ZoomBoxes[i].f3 = b->f11;
        else
            ZoomBoxes[i].f3 = c1;
        if (c2 == -1)
            ZoomBoxes[i].f4 = b->f11;
        else
            ZoomBoxes[i].f4 = c2;
        switch (b->f12) {
        case 1:
            ZoomBoxes[i].from[0] = b->x + (b->x1 - b->x) / 2;
            ZoomBoxes[i].from[1] = b->y;
            ZoomBoxes[i].from[2] = 0;
            ZoomBoxes[i].from[3] = b->y1 - b->y + 1;
            break;
        case 0:
            ZoomBoxes[i].from[0] = b->x + (b->x1 - b->x) / 2;
            ZoomBoxes[i].from[1] = b->y + (b->y1 - b->y) / 2;
            ZoomBoxes[i].from[2] = 0;
            ZoomBoxes[i].from[3] = 0;
            break;
        case 2:
            ZoomBoxes[i].from[0] = D_00188c34 / 2;
            ZoomBoxes[i].from[1] = D_00188c38 / 2;
            ZoomBoxes[i].from[2] = 0;
            ZoomBoxes[i].from[3] = 0;
            break;
        default:
            break;
        }
        ZoomBoxes[i].to[0] = b->x;
        ZoomBoxes[i].to[1] = b->y;
        ZoomBoxes[i].to[2] = b->x1 - b->x + 1;
        ZoomBoxes[i].to[3] = b->y1 - b->y + 1;
    } else {
        hosp_dos_printf(D_00121bd0);
    }
    return i;
}

// FUNCTION: 0x000b8f64
void add_glass_frame(GlassFrame *f, s32 mask)
{
    s16 i;

    mask &= -mask;
    i = 0;
    while (f[i].x1)
        add_glass_box((GlassBox *)&f[i++], mask, -1, -1);
    DesiredDisplay |= mask;
}

// FUNCTION: 0x000b8fcc
void draw_a_box(u8 n)
{
    s32 *from;
    s32 *to;
    s32 step;
    s32 total;
    s16 x;
    s16 y;
    s16 w;
    s16 h;

    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 4;
    from = ZoomBoxes[n].from;
    to = ZoomBoxes[n].to;
    step = ZoomBoxes[n].f1;
    total = ZoomBoxes[n].f2 - 1;
    x = ((total - step) * from[0] + to[0] * step) / total;
    y = ((total - step) * from[1] + to[1] * step) / total;
    w = ((total - step) * from[2] + to[2] * step) / total;
    h = ((total - step) * from[3] + to[3] * step) / total;
    if (step != 0) {
        D_00188c74 = 0;
        if (ZoomBoxes[n].fb) {
            if (!((((s8)ZoomBoxes[n].type != 2 && display_mode == 10) || (to[0] == 0x213 && display_mode == 5))
                  && current_balance_file && current_balance_file == current_balance_file->next)) {
                draw_button_box(x, y, w, h);
                draw_button_edges(x, y, w - 1, h - 1, 0);
            }
        } else {
            switch (ZoomBoxes[n].type) {
            case -1:
                draw_background_box(x, y, w, h, 1);
                break;
            case 0:
                D_00188c74 = 4;
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
            if ((ZoomBoxes[i].f1 > 0 && ZoomBoxes[i].f1 < ZoomBoxes[i].f2 - 1)
                || (ZoomBoxes[i].f1 && !(DesiredDisplay & ZoomBoxes[i].id))) {
                found = 1;
                if (ZoomBoxes[i].type > hi)
                    hi = ZoomBoxes[i].type;
            }
            if (ZoomBoxes[i].f1 == 0) {
                if (!found) {
                    hi = ZoomBoxes[i].type;
                    found = 1;
                }
            }
            if (ZoomBoxes[i].f1 > 0 && ZoomBoxes[i].f1 <= ZoomBoxes[i].f2 - 1) {
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
                        if (!ZoomBoxes[i].f3) {
                            ZoomBoxes[i].f1++;
                            if (ZoomBoxes[i].f1 >= ZoomBoxes[i].f2 - 1) {
                                for (j = 0; j < 0x2e; j++) {
                                    if (((ZoomBoxes[j].active && i != j) && ZoomBoxes[j].id == ZoomBoxes[i].id)
                                        && ZoomBoxes[j].f1 != ZoomBoxes[j].f2 - 1)
                                        break;
                                }
                                if (j == 0x2e)
                                    CurrentDisplay |= ZoomBoxes[i].id;
                                if (ZoomBoxes[i].f1 > ZoomBoxes[i].f2 - 1)
                                    ZoomBoxes[i].f1--;
                            } else {
                                opening = 1;
                                result = 1;
                            }
                        } else {
                            ZoomBoxes[i].f3--;
                        }
                    }
                    draw_a_box(i);
                }
            } else if (ZoomBoxes[i].type == hi) {
                if (!ZoomBoxes[i].f4) {
                    CurrentDisplay &= ~ZoomBoxes[i].id;
                    result = 1;
                    closing = 1;
                    if (--ZoomBoxes[i].f1 <= 0)
                        ZoomBoxes[i].active = 0;
                } else {
                    ZoomBoxes[i].f4--;
                }
                draw_a_box(i);
            } else {
                draw_a_box(i);
            }
        }
    }
    if (opening && !playing_open_sound) {
        playing_open_sound = 1;
        play_sound(10, 0x9c, -1, -1);
    } else if (!opening) {
        playing_open_sound = 0;
        StopSample(10, 0x9c);
    }
    if (closing && !playing_close_sound) {
        playing_close_sound = 1;
        play_sound(10, 0x9e, -1, -1);
    } else if (!closing) {
        playing_close_sound = 0;
        StopSample(10, 0x9e);
    }
    return result;
}

// FUNCTION: 0x000b96cc
s32 next_map(void)
{
    s32 r;
    s32 diff;

    diff = CurrentDisplay ^ DesiredDisplay;
    r = diff & CurrentDisplay;
    if (r)
        return r;
    if ((CurrentDisplay | DesiredDisplay) & 8)
        return CurrentDisplay ^ DesiredDisplay;
    return diff & -diff;
}

// FUNCTION: 0x000b973c
s16 glass_index(s32 mask)
{
    u32 bit;

    bit = mask & -mask;
    switch (bit) {
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

// FUNCTION: 0x000b991c
s16 button_index(s32 mask)
{
    u32 bit;

    bit = mask & -mask;
    switch (bit) {
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

// NONMATCHING: register choice
// FUNCTION: 0x000b9b04
void draw_glass_text(SlistButton *b, char *text)
{
    s16 x;

    D_00188c74 &= 4;
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetJustifyWindow(b->x, 0, b->x1 - b->x + 1);
    if ((D_00188c7e
         && over_hotspot((s16)mouse_dx, (s16)mouse_dy, b->x, b->y, b->x1 - b->x + 1, b->y1 - b->y + 1))
        || (D_00188c81
            && over_hotspot(D_00188c6c, D_00188c70, b->x, b->y, b->x1 - b->x + 1, b->y1 - b->y + 1))) {
        set_font_from_cache(0x22);
        x = (b->x1 - b->x + 1 - LbTextStringWidth(text)) >> 1;
        LbTextDraw(x + 1, ((b->y1 + b->y + 1 - (lbFontPtr ? lbFontPtr[34].height : 0)) >> 1) + 1, text);
    } else {
        set_font_from_cache(0x21);
        x = (b->x1 - b->x + 1 - LbTextStringWidth(text)) >> 1;
        LbTextDraw(x, (b->y1 + b->y + 1 - (lbFontPtr ? lbFontPtr[34].height : 0)) >> 1, text);
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x000b9d20
void draw_glass_text_hilight(SlistButton *b, char *text)
{
    s16 x;

    D_00188c74 &= 4;
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetJustifyWindow(b->x, 0, b->x1 - b->x + 1);
    set_font_from_cache(0x22);
    x = (b->x1 - b->x + 1 - LbTextStringWidth(text)) >> 1;
    LbTextDraw(x + 1, ((b->y1 + b->y + 1 - (lbFontPtr ? lbFontPtr[34].height : 0)) >> 1) + 1, text);
}

// NONMATCHING: register choice
// FUNCTION: 0x000b9e14
void draw_glass_text_hilight_no_jump(SlistButton *b, char *text)
{
    s16 x;

    D_00188c74 &= 4;
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetJustifyWindow(b->x, 0, b->x1 - b->x + 1);
    set_font_from_cache(0x22);
    x = (b->x1 - b->x + 1 - LbTextStringWidth(text)) >> 1;
    if ((D_00188c7e
         && over_hotspot((s16)mouse_dx, (s16)mouse_dy, b->x, b->y, b->x1 - b->x + 1, b->y1 - b->y + 1))
        || (D_00188c81
            && over_hotspot(D_00188c6c, D_00188c70, b->x, b->y, b->x1 - b->x + 1, b->y1 - b->y + 1)))
        LbTextDraw(x + 1, ((b->y1 + b->y + 1 - (lbFontPtr ? lbFontPtr[34].height : 0)) >> 1) + 1, text);
    else
        LbTextDraw(x, (b->y1 + b->y + 1 - (lbFontPtr ? lbFontPtr[34].height : 0)) >> 1, text);
}

// FUNCTION: 0x000b9ffc
void draw_background_box(s16 x, s16 y, s16 w, s16 h, u8 style)
{
    u8 colour;

    LbDrawBox(x, y, w, h, white_colour);
    D_00188c74 = 0x10;
    LbDrawBox(x, y, w, h, black_colour);
    if (style)
        colour = 0xca;
    else
        colour = b_b_colour;
    LbDrawBox(x + 1, y + 1, w - 2, h - 2, colour);
    LbDrawBox(x + 2, y + 2, w - 4, h - 4, colour);
    LbDrawBox(x + 3, y + 3, w - 6, h - 6, colour);
    D_00188c74 = 0;
}

// FUNCTION: 0x000ba0ec
void draw_foreground_box(s16 x, s16 y, s16 w, s16 h)
{
    D_00188c74 = 0;
    LbDrawBox(x, y, w, h, white_colour);
    D_00188c74 = 0x10;
    LbDrawBox(x, y, w, h, 0xf8);
    LbDrawBox(x + 1, y + 1, w - 2, h - 2, b_b_colour);
    LbDrawBox(x + 2, y + 2, w - 4, h - 4, b_b_colour);
    LbDrawBox(x + 3, y + 3, w - 6, h - 6, b_b_colour);
    D_00188c74 = 0;
}

// FUNCTION: 0x000ba1d4
void draw_foreground_box2(s16 x, s16 y, s16 w, s16 h)
{
    D_00188c74 = 0;
    LbDrawBox(x, y, w, h, white_colour);
    D_00188c74 = 0x10;
    LbDrawBox(x, y, w, h, b_b_colour);
    LbDrawBox(x + 1, y + 1, w - 2, h - 2, b_b_colour);
    D_00188c74 = 0;
}

// FUNCTION: 0x000ba270
void draw_button_box(s16 x, s16 y, s16 w, s16 h)
{
    D_00188c74 = 0;
    LbDrawBox(x, y, w, h, white_colour);
    D_00188c74 = 0x10;
    LbDrawBox(x, y, w, h, 0xf8);
    D_00188c74 = 0;
}

// FUNCTION: 0x000ba2e0
void draw_button_edges(s16 x, s16 y, s16 w, s16 h, u8 style)
{
    D_00188c74 = 0;
    if (!style) {
        fn_000c9be0(x + 1, y + 1, x + w - 1, y + 1, Gray[2]);
        fn_000c9be0(x + 2, y + 2, x + w - 2, y + 2, Gray[3]);
        fn_000c9be0(x + 3, y + 3, x + w - 3, y + 3, Gray[4]);
        fn_000c9be0(x + 1, y + 2, x + 1, y + h - 1, Gray[2]);
        fn_000c9be0(x + 2, y + 3, x + 2, y + h - 2, Gray[3]);
        fn_000c9be0(x + 3, y + 4, x + 3, y + h - 3, Gray[4]);
        fn_000c9be0(x + 2, y + h - 1, x + w - 1, y + h - 1, Gray[5]);
        fn_000c9be0(x + 3, y + h - 2, x + w - 2, y + h - 2, Gray[6]);
        fn_000c9be0(x + 4, y + h - 3, x + w - 3, y + h - 3, Gray[7]);
        fn_000c9be0(x + w - 1, y + 2, x + w - 1, y + h - 2, Gray[5]);
        fn_000c9be0(x + w - 2, y + 3, x + w - 2, y + h - 3, Gray[6]);
        fn_000c9be0(x + w - 3, y + 4, x + w - 3, y + h - 4, Gray[7]);
    } else {
        fn_000c9be0(x + 1, y + 1, x + w - 1, y + 1, Gray[5]);
        fn_000c9be0(x + 2, y + 2, x + w - 2, y + 2, Gray[6]);
        fn_000c9be0(x + 3, y + 3, x + w - 3, y + 3, Gray[7]);
        fn_000c9be0(x + 1, y + 2, x + 1, y + h - 1, Gray[5]);
        fn_000c9be0(x + 2, y + 3, x + 2, y + h - 2, Gray[6]);
        fn_000c9be0(x + 3, y + 4, x + 3, y + h - 3, Gray[7]);
        fn_000c9be0(x + 2, y + h - 1, x + w - 1, y + h - 1, Gray[2]);
        fn_000c9be0(x + 3, y + h - 2, x + w - 2, y + h - 2, Gray[3]);
        fn_000c9be0(x + 4, y + h - 3, x + w - 3, y + h - 3, Gray[4]);
        fn_000c9be0(x + w - 1, y + 2, x + w - 1, y + h - 2, Gray[2]);
        fn_000c9be0(x + w - 2, y + 3, x + w - 2, y + h - 3, Gray[3]);
        fn_000c9be0(x + w - 3, y + 4, x + w - 3, y + h - 4, Gray[4]);
    }
}

// FUNCTION: 0x000ba800
void calculate_grey(u8 *palette)
{
    b_b_colour = fn_000c960c(palette, 3, 0x2f, 0x3b);
    white_colour = fn_000c960c(palette, 0x3f, 0x3f, 0x3f);
    black_colour = fn_000c960c(palette, 0, 0, 0);
    Gray[0] = 0;
    Gray[1] = 0xff;
    Gray[2] = 0xf5;
    Gray[3] = 0xf5;
    Gray[4] = 0xf5;
    Gray[5] = 0xf8;
    Gray[6] = 0xf8;
    Gray[7] = 0xf6;
}
