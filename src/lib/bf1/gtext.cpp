#include "bflib.h"

struct TbTextWindow {
    s32 x;
    s32 y;
    s32 width;
    s32 height;
    u8 *ptr;
};

struct TbGraphicsWindow {
    s32 x;
    s32 y;
    s32 width;
    s32 height;
    u8 *ptr;
};

void put_down_sprites(char *sbegin, char *send, long x, long y, long space_len);

int LbAssertMessage(const char *expr, const char *file, unsigned long line);

extern "C" {
extern TbTextWindow lbTextJustifyWindow;
extern TbTextWindow lbTextClipWindow;
u8 lbSpacesPerTab = 4;

void int3(void);
#pragma aux int3 = 0xcc;


void LbDrawHVLine(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
void LbSpriteDrawOneColour(s32 x, s32 y, TbSprite *spr, u8 colour);

int LbTextSetJustifyWindow(s32 x, s32 y, s32 width);
int LbTextSetClipWindow(s32 x, s32 y, s32 width, s32 height);

// Sets both the text justify window and the clip window to the box at (x,
// y), width x height. Returns 1.
// FUNCTION: 0x000c84f0
int LbTextSetWindow(s32 x, s32 y, s32 width, s32 height)
{
    LbTextSetJustifyWindow(x, y, width);
    LbTextSetClipWindow(x, y, width, height);
    return 1;
}

// Sets the text justify window (where lines start and wrap) to x, y and
// width. Returns 1.
// FUNCTION: 0x000c852c
int LbTextSetJustifyWindow(s32 x, s32 y, s32 width)
{
    lbTextJustifyWindow.x = x;
    lbTextJustifyWindow.y = y;
    lbTextJustifyWindow.width = width;
    lbTextJustifyWindow.ptr = x + (lbDisplay.wscreen + y * lbDisplay.graphics_screen_width);
    return 1;
}

// Sets the text clip window to the box at (x, y), width x height, clipped
// to the screen; its ptr points at (x, y) as passed, neither swapped nor
// clipped. Returns 1.
// FUNCTION: 0x000c857c
int LbTextSetClipWindow(s32 x, s32 y, s32 width, s32 height)
{
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;

    x1 = x;
    y1 = y;
    x2 = x + width;
    y2 = y + height;
    if (x2 < x1) {
        x2 ^= x1;
        x1 ^= x2;
        x2 ^= x1;
    }
    if (y2 < y1) {
        y2 ^= y1;
        y1 ^= y2;
        y2 ^= y1;
    }
    if (x1 < 0)
        x1 = 0;
    if (x2 < 0)
        x2 = 0;
    if (y1 < 0)
        y1 = 0;
    if (y2 < 0)
        y2 = 0;
    if (x1 > lbDisplay.graphics_screen_width)
        x1 = lbDisplay.graphics_screen_width;
    if (x2 > lbDisplay.graphics_screen_width)
        x2 = lbDisplay.graphics_screen_width;
    if (y1 > lbDisplay.graphics_screen_height)
        y1 = lbDisplay.graphics_screen_height;
    if (y2 > lbDisplay.graphics_screen_height)
        y2 = lbDisplay.graphics_screen_height;
    lbTextClipWindow.x = x1;
    lbTextClipWindow.y = y1;
    lbTextClipWindow.width = x2 - x1;
    lbTextClipWindow.height = y2 - y1;
    lbTextClipWindow.ptr = x + (lbDisplay.wscreen + y * lbDisplay.graphics_screen_width);
    return 1;
}

// Returns the width in pixels of the widest line of text in the current
// font (tabs count lbSpacesPerTab spaces), 0 with no font.
// FUNCTION: 0x000c86a4
s32 LbTextStringWidth(char *text)
{
    s32 max;
    s32 width;

    max = 0;
    if (lbFontPtr) {
        for (width = 0; *text; text++) {
            if ((u8)*text > 31) {
                width += lbFontPtr ? lbFontPtr[(u8)*text - 31].width : 0;
            } else if ((s8)*text == '\t') {
                width += (lbFontPtr ? lbFontPtr[1].width : 0) * lbSpacesPerTab;
            } else if ((s8)*text == '\n') {
                if (width > max)
                    max = width;
                width = 0;
            }
        }
        if (width > max)
            return width;
    }
    return max;
}

// Returns the width in pixels of the word starting at text, up to a space,
// tab, newline or the end.
// FUNCTION: 0x000c877c
s32 LbTextWordWidth(char *text)
{
    s32 width;

    width = 0;
    if (lbFontPtr) {
        for (; (s8)*text != ' ' && (s8)*text != '\t' && (s8)*text != '\n' && *text; text++) {
            if ((u8)*text > 32)
                width += lbFontPtr ? lbFontPtr[(u8)*text - 31].width : 0;
        }
    }
    return width;
}

// Returns the height in pixels of text's lines in the current font
// (asserting on a null text); 0 with no font or a null text.
// FUNCTION: 0x000c8800
s32 LbTextStringHeight(char *text)
{
    s32 lines;

    lines = 1;
    do {
        if (!text) {
            if (LbAssertMessage("LbTextStringHeight was passed a NULL pointer", "gtext.cpp", 198))
                int3();
        }
    } while (0);
    if (lbFontPtr && text) {
        for (; *text; text++) {
            if ((s8)*text == '\n')
                lines++;
        }
        return (lbFontPtr ? lbFontPtr[1].height : 0) * lines;
    }
    return 0;
}

// Draws text at (x, y) relative to the justify window, clipped to the clip
// window, wrapping words at its width: draw_flags 0x20 wraps left-aligned,
// 0x80 right-aligned, 0x100 centred, 0x200 justified (none: no wrapping).
// Codes 6-9 in text toggle those flags. Returns 1.
// NONMATCHING: the register zeroing count differs per line-break tail, so the
// tails are merged differently; jump table placement
// FUNCTION: 0x000c889c
int LbTextDraw(s32 x, s32 y, char *text)
{
    TbGraphicsWindow grwnd;
    s16 count;
    s32 start_x;
    s32 dx;
    s32 dy;
    char *sbegin;
    s32 space_width;

    if (lbFontPtr) {
        sbegin = text;
        count = 0;
        grwnd.x = lbDisplay.graphics_window_x;
        grwnd.y = lbDisplay.graphics_window_y;
        grwnd.width = lbDisplay.graphics_window_width;
        grwnd.height = lbDisplay.graphics_window_height;
        grwnd.ptr = lbDisplay.graphics_window;
        lbDisplay.graphics_window_x = lbTextClipWindow.x;
        lbDisplay.graphics_window_y = lbTextClipWindow.y;
        lbDisplay.graphics_window_width = lbTextClipWindow.width;
        lbDisplay.graphics_window_height = lbTextClipWindow.height;
        lbDisplay.graphics_window = lbTextClipWindow.ptr;
        dx = lbTextJustifyWindow.x - lbTextClipWindow.x;
        dy = lbTextJustifyWindow.y - lbTextClipWindow.y;
        x += dx;
        y += dy;
        start_x = x;
        for (; *text; text++) {
            if ((u8)*text > 32) {
                x += lbFontPtr ? lbFontPtr[(u8)*text - 31].width : 0;
            } else if ((s8)*text == ' ') {
                x += lbFontPtr ? lbFontPtr[1].width : 0;
                if ((x - dx) + LbTextWordWidth(text + 1) > lbTextJustifyWindow.width) {
                    space_width = lbFontPtr ? lbFontPtr[1].width : 0;
                    if (lbDisplay.draw_flags & 0x20) {
                        x = start_x;
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else if (lbDisplay.draw_flags & 0x80) {
                        x = start_x + (lbTextJustifyWindow.width - (x - (lbFontPtr ? lbFontPtr[1].width : 0) - dx));
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        x = start_x;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else if (lbDisplay.draw_flags & 0x100) {
                        x = start_x + (lbTextJustifyWindow.width - (x - (lbFontPtr ? lbFontPtr[1].width : 0) - dx)) / 2;
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        x = start_x;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else if (lbDisplay.draw_flags & 0x200) {
                        if (count > 0)
                            space_width += (u32)(lbTextJustifyWindow.width - (x - (lbFontPtr ? lbFontPtr[1].width : 0) - dx)) / count;
                        x = start_x;
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else {
                        count = 0;
                    }
                } else {
                    count++;
                }
            } else if ((s8)*text == '\n') {
                if (!(lbDisplay.draw_flags & 0x20)) {
                    if (lbDisplay.draw_flags & 0x80)
                        x = start_x + (lbTextJustifyWindow.width - (x - dx));
                    else if (lbDisplay.draw_flags & 0x100)
                        x = start_x + (lbTextJustifyWindow.width - (x - dx)) / 2;
                    else
                        x = start_x;
                } else {
                    x = start_x;
                }
                put_down_sprites(sbegin, text, x, y, lbFontPtr ? lbFontPtr[1].width : 0);
                sbegin = text;
                x = start_x;
                y += lbFontPtr ? lbFontPtr[1].height : 0;
                count = 0;
            } else if ((s8)*text == '\t') {
                x += (lbFontPtr ? lbFontPtr[1].width : 0) * lbSpacesPerTab;
                if (LbTextWordWidth(text + 1) + (x - dx) > lbTextJustifyWindow.width) {
                    space_width = lbFontPtr ? lbFontPtr[1].width : 0;
                    if (lbDisplay.draw_flags & 0x20) {
                        x = start_x;
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else if (lbDisplay.draw_flags & 0x80) {
                        x = start_x + (lbTextJustifyWindow.width - (x - (lbFontPtr ? lbFontPtr[1].width : 0) * lbSpacesPerTab - dx));
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        x = start_x;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else if (lbDisplay.draw_flags & 0x100) {
                        x = start_x + (lbTextJustifyWindow.width - (x - lbSpacesPerTab * (lbFontPtr ? lbFontPtr[1].width : 0) - dx)) / 2;
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        x = start_x;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else if (lbDisplay.draw_flags & 0x200) {
                        if (count > 0)
                            space_width += (u32)(lbTextJustifyWindow.width - (x - (lbFontPtr ? lbFontPtr[1].width : 0) - dx)) / count;
                        x = start_x;
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else {
                        count = 0;
                    }
                } else {
                    count += lbSpacesPerTab;
                }
            } else if ((s8)*text == 6 || (s8)*text == 7 || (s8)*text == 8 || (s8)*text == 9) {
                if (x - dx > lbTextJustifyWindow.width) {
                    x = start_x;
                    put_down_sprites(sbegin, text, x, y, lbFontPtr ? lbFontPtr[1].width : 0);
                    sbegin = text;
                    y += lbFontPtr ? lbFontPtr[1].height : 0;
                    count = 0;
                }
                switch ((u8)*text) {
                case 6:
                    lbDisplay.draw_flags ^= 0x20;
                    break;
                case 7:
                    lbDisplay.draw_flags ^= 0x80;
                    break;
                case 8:
                    lbDisplay.draw_flags ^= 0x100;
                    break;
                case 9:
                    lbDisplay.draw_flags ^= 0x200;
                    break;
                }
            } else if ((s8)*text == 14) {
                text++;
            }
        }
        if (!(lbDisplay.draw_flags & 0x20)) {
            if (lbDisplay.draw_flags & 0x80)
                x = start_x + (lbTextJustifyWindow.width - (x - dx));
            else if (lbDisplay.draw_flags & 0x100)
                x = start_x + (lbTextJustifyWindow.width - (x - dx)) / 2;
            else
                x = start_x;
        } else {
            x = start_x;
        }
        put_down_sprites(sbegin, text, x, y, lbFontPtr ? lbFontPtr[1].width : 0);
        lbDisplay.graphics_window_x = grwnd.x;
        lbDisplay.graphics_window_y = grwnd.y;
        lbDisplay.graphics_window_width = grwnd.width;
        lbDisplay.graphics_window_height = grwnd.height;
        lbDisplay.graphics_window = grwnd.ptr;
    }
    return 1;
}

}

// Draws the characters from sbegin up to send at (x, y), spaces space_len
// wide, in the font's sprites or (draw_flags 0x40) in draw_colour,
// underlined with 0x400. Control codes toggle drawing flags (1-5, 11, 12)
// or set draw_colour from the next byte (14).
// FUNCTION: 0x000c90f0
void put_down_sprites(char *sbegin, char *send, long x, long y, long space_len)
{
    TbSprite *spr;

    for (; sbegin < send; sbegin++) {
        if ((u8)*sbegin > 32) {
            spr = lbFontPtr + (u8)*sbegin - 31;
            if (lbDisplay.draw_flags & 0x40)
                LbSpriteDrawOneColour(x, y, spr, lbDisplay.draw_colour);
            else
                LbSpriteDraw(x, y, spr);
            if (lbDisplay.draw_flags & 0x400)
                LbDrawHVLine(x, y + (lbFontPtr ? lbFontPtr[1].height : 0), spr->width + x,
                           y + (lbFontPtr ? lbFontPtr[1].height : 0), lbDisplay.draw_colour);
            x += spr->width;
        } else if ((s8)*sbegin == ' ') {
            if (lbDisplay.draw_flags & 0x400)
                LbDrawHVLine(x, (lbFontPtr ? lbFontPtr[1].height : 0) + y, x + space_len,
                           (lbFontPtr ? lbFontPtr[1].height : 0) + y, lbDisplay.draw_colour);
            x += space_len;
        } else if ((s8)*sbegin == '\t') {
            if (lbDisplay.draw_flags & 0x400)
                LbDrawHVLine(x, (lbFontPtr ? lbFontPtr[1].height : 0) + y, x + lbSpacesPerTab * space_len,
                           (lbFontPtr ? lbFontPtr[1].height : 0) + y, lbDisplay.draw_colour);
            x += lbSpacesPerTab * space_len;
        } else {
            switch ((u8)*sbegin) {
            case 1:
                lbDisplay.draw_flags ^= 0x04;
                break;
            case 2:
                lbDisplay.draw_flags ^= 0x08;
                break;
            case 3:
                lbDisplay.draw_flags ^= 0x10;
                break;
            case 4:
                lbDisplay.draw_flags ^= 0x01;
                break;
            case 5:
                lbDisplay.draw_flags ^= 0x02;
                break;
            case 11:
                lbDisplay.draw_flags ^= 0x400;
                break;
            case 12:
                lbDisplay.draw_flags ^= 0x40;
                break;
            case 14:
                sbegin++;
                lbDisplay.draw_colour = *sbegin;
                break;
            }
        }
    }
}
