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

extern "C" {
extern TbTextWindow lbTextJustifyWindow;
extern TbTextWindow lbTextClipWindow;
extern u8 lbSpacesPerTab;
extern s32 D_00188c34;
#pragma aux D_00188c34 "W?research_names$npnpna"
extern s32 D_00188c38;
extern char D_00124250[];
extern char D_0012425a[];

int fn_000d0848(char *expr, char *file, int line);
void int3(void);
#pragma aux int3 = 0xcc;

extern u16 D_00188c74;
extern u8 D_00188c75;
extern u8 D_00188c85;
extern s32 D_00188c3c;
extern s32 D_00188c40;
extern s32 D_00188c44;
extern s32 D_00188c48;

void LbDrawLine(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
#pragma aux LbDrawLine "__wcpp_2_undefed_cdtor__"
void LbSpriteDrawOneColour(s32 x, s32 y, TbSprite *sprite, u8 colour);
#pragma aux LbSpriteDrawOneColour "LbDrawHVLine_"

int LbTextSetJustifyWindow(s32 x, s32 y, s32 width);
int LbTextSetClipWindow(s32 x, s32 y, s32 width, s32 height);

// FUNCTION: 0x000c84f0
int LbTextSetWindow(s32 x, s32 y, s32 width, s32 height)
{
    LbTextSetJustifyWindow(x, y, width);
    LbTextSetClipWindow(x, y, width, height);
    return 1;
}

// FUNCTION: 0x000c852c
int LbTextSetJustifyWindow(s32 x, s32 y, s32 width)
{
    lbTextJustifyWindow.x = x;
    lbTextJustifyWindow.y = y;
    lbTextJustifyWindow.width = width;
    lbTextJustifyWindow.ptr = x + (lbDisplay.unknown4 + y * D_00188c34);
    return 1;
}

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
    if (x1 > D_00188c34)
        x1 = D_00188c34;
    if (x2 > D_00188c34)
        x2 = D_00188c34;
    if (y1 > D_00188c38)
        y1 = D_00188c38;
    if (y2 > D_00188c38)
        y2 = D_00188c38;
    lbTextClipWindow.x = x1;
    lbTextClipWindow.y = y1;
    lbTextClipWindow.width = x2 - x1;
    lbTextClipWindow.height = y2 - y1;
    lbTextClipWindow.ptr = x + (lbDisplay.unknown4 + y * D_00188c34);
    return 1;
}

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

// NONMATCHING: register choice; target range also holds LbTextDraw's jump table
// FUNCTION: 0x000c8800
s32 LbTextStringHeight(char *text)
{
    s32 lines;

    lines = 1;
    if (!text) {
        if (fn_000d0848(D_0012425a, D_00124250, 198))
            int3();
    }
    if (lbFontPtr && text) {
        for (; *text; text++) {
            if ((s8)*text == '\n')
                lines++;
        }
        return (lbFontPtr ? lbFontPtr[1].height : 0) * lines;
    }
    return 0;
}

// NONMATCHING: stack slots of y, duplicated line-break tails; jump table placement
// FUNCTION: 0x000c889c
int LbTextDraw(s32 x, s32 y, char *text)
{
    s16 count;
    s32 start_x;
    s32 dx;
    s32 dy;
    char *sbegin;
    s32 space_width;
    TbGraphicsWindow grwnd;

    if (lbFontPtr) {
        sbegin = text;
        count = 0;
        grwnd.x = D_00188c3c;
        grwnd.y = D_00188c40;
        grwnd.width = D_00188c44;
        grwnd.height = D_00188c48;
        grwnd.ptr = lbDisplay.graphics_window;
        D_00188c3c = lbTextClipWindow.x;
        D_00188c40 = lbTextClipWindow.y;
        D_00188c44 = lbTextClipWindow.width;
        D_00188c48 = lbTextClipWindow.height;
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
                if (LbTextWordWidth(text + 1) + (x - dx) > lbTextJustifyWindow.width) {
                    space_width = lbFontPtr ? lbFontPtr[1].width : 0;
                    if (D_00188c74 & 0x20) {
                        x = start_x;
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else if (D_00188c74 & 0x80) {
                        x = start_x + (lbTextJustifyWindow.width - (x - (lbFontPtr ? lbFontPtr[1].width : 0) - dx));
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        x = start_x;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else if (D_00188c75 & 1) {
                        x = start_x + (lbTextJustifyWindow.width - (x - (lbFontPtr ? lbFontPtr[1].width : 0) - dx)) / 2;
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        x = start_x;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else if (D_00188c75 & 2) {
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
                if (!(D_00188c74 & 0x20)) {
                    if (D_00188c74 & 0x80)
                        x = start_x + (lbTextJustifyWindow.width - (x - dx));
                    else if (D_00188c75 & 1)
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
                    if (D_00188c74 & 0x20) {
                        x = start_x;
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else if (D_00188c74 & 0x80) {
                        x = start_x + (lbTextJustifyWindow.width - (x - (lbFontPtr ? lbFontPtr[1].width : 0) * lbSpacesPerTab - dx));
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        x = start_x;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else if (D_00188c75 & 1) {
                        x = start_x + (lbTextJustifyWindow.width - (x - lbSpacesPerTab * (lbFontPtr ? lbFontPtr[1].width : 0) - dx)) / 2;
                        put_down_sprites(sbegin, text, x, y, space_width);
                        sbegin = text + 1;
                        x = start_x;
                        y += lbFontPtr ? lbFontPtr[1].height : 0;
                        count = 0;
                    } else if (D_00188c75 & 2) {
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
                    D_00188c74 ^= 0x20;
                    break;
                case 7:
                    D_00188c74 ^= 0x80;
                    break;
                case 8:
                    D_00188c75 ^= 1;
                    break;
                case 9:
                    D_00188c75 ^= 2;
                    break;
                }
            } else if ((s8)*text == 14) {
                text++;
            }
        }
        if (!(D_00188c74 & 0x20)) {
            if (D_00188c74 & 0x80)
                x = start_x + (lbTextJustifyWindow.width - (x - dx));
            else if (D_00188c75 & 1)
                x = start_x + (lbTextJustifyWindow.width - (x - dx)) / 2;
            else
                x = start_x;
        } else {
            x = start_x;
        }
        put_down_sprites(sbegin, text, x, y, lbFontPtr ? lbFontPtr[1].width : 0);
        D_00188c3c = grwnd.x;
        D_00188c40 = grwnd.y;
        D_00188c44 = grwnd.width;
        D_00188c48 = grwnd.height;
        lbDisplay.graphics_window = grwnd.ptr;
    }
    return 1;
}

}

// NONMATCHING: register choice; jump table placement (target keeps it after LbTextDraw)
// FUNCTION: 0x000c90f0
void put_down_sprites(char *sbegin, char *send, long x, long y, long space_len)
{
    TbSprite *spr;

    for (; sbegin < send; sbegin++) {
        if ((u8)*sbegin > 32) {
            spr = lbFontPtr + (u8)*sbegin - 31;
            if (D_00188c74 & 0x40)
                LbSpriteDrawOneColour(x, y, spr, D_00188c85);
            else
                LbSpriteDraw(x, y, spr);
            if (D_00188c75 & 4)
                LbDrawLine(x, y + (lbFontPtr ? lbFontPtr[1].height : 0), spr->width + x,
                           y + (lbFontPtr ? lbFontPtr[1].height : 0), D_00188c85);
            x += spr->width;
        } else if ((s8)*sbegin == ' ') {
            if (D_00188c75 & 4)
                LbDrawLine(x, (lbFontPtr ? lbFontPtr[1].height : 0) + y, x + space_len,
                           (lbFontPtr ? lbFontPtr[1].height : 0) + y, D_00188c85);
            x += space_len;
        } else if ((s8)*sbegin == '\t') {
            if (D_00188c75 & 4)
                LbDrawLine(x, (lbFontPtr ? lbFontPtr[1].height : 0) + y, x + lbSpacesPerTab * space_len,
                           (lbFontPtr ? lbFontPtr[1].height : 0) + y, D_00188c85);
            x += lbSpacesPerTab * space_len;
        } else {
            switch ((u8)*sbegin) {
            case 1:
                D_00188c74 ^= 0x04;
                break;
            case 2:
                D_00188c74 ^= 0x08;
                break;
            case 3:
                D_00188c74 ^= 0x10;
                break;
            case 4:
                D_00188c74 ^= 0x01;
                break;
            case 5:
                D_00188c74 ^= 0x02;
                break;
            case 11:
                D_00188c75 ^= 4;
                break;
            case 12:
                D_00188c74 ^= 0x40;
                break;
            case 14:
                sbegin++;
                D_00188c85 = *sbegin;
                break;
            }
        }
    }
}
