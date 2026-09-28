#include "hosp.h"

extern "C" {

extern s32 D_00188c3c;
extern s32 D_00188c40;
extern u16 D_00188c74;

}

// NONMATCHING: register choice for the LbScreenSetGraphicsWindow arguments
// FUNCTION: 0x0004bcc0
void draw_buttons(Window *w)
{
    s32 unused1c;
    s32 unused18;
    s32 unused14;
    WindowButton *b;
    s32 x;
    s32 y;

    b = w->buttons;
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
                              window.unknown2cf - window.unknown2c7);
    for (; b; b = b->next) {
        if (b->flags & 0x20001000)
            continue;
        x = w->x - D_00188c3c;
        y = w->y - D_00188c40;
        if (b->flags & 1)
            reqspr_draw((s16)x + b->x20, (s16)y + b->y22, &button_sprite[b->sprite1c]);
        else if ((b->flags & 6) && (!(b->flags & 2) || !(b->flags & 4)))
            reqspr_draw((s16)x + b->x24, (s16)y + b->y26, &button_sprite[b->sprite18]);
        else
            reqspr_draw((s16)x + b->x, (s16)y + b->y, &button_sprite[b->sprite]);
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0004be18
void draw_a_window(s16 x, s16 y, s16 w, s16 h, u8 style)
{
    s32 dx;
    s32 dy;
    s32 len;
    u16 nx;
    s32 len2;
    u16 ny;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    u16 saved;
    u16 i;
    u16 j;

    style <<= 3;
    dx = 0;
    dy = 0;
    len = w - bord[style + 1].width - bord[style + 3].width;
    nx = len / bord[style + 2].width;
    if (nx * bord[style + 1].width < len)
        nx++;
    len2 = h - bord[style + 1].height - bord[style + 6].height;
    ny = len2 / bord[17].height;
    if (ny * bord[17].height < len2)
        ny++;
    x0 = x + bord[style + 1].width;
    y0 = y + bord[style + 1].height;
    x1 = x0 + len;
    y1 = y0 + len2;
    find_common_window(x0, y0, x1, y1, window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
    if (info.unknown78 == 0) {
        LbScreenSetGraphicsWindow(info.unknown68, info.unknown6c, info.unknown70 - info.unknown68,
                                  info.unknown74 - info.unknown6c);
        dx += x0 - info.unknown68;
        dy += y0 - info.unknown6c;
        len = dx;
        saved = D_00188c74;
        D_00188c74 = 8;
        for (i = 0; i < ny; i++) {
            for (j = 0; j < nx; j++) {
                LbSpriteDraw(dx, dy, bord + 17);
                dx += bord[17].width;
            }
            dx = len;
            dy += bord[17].height;
        }
        D_00188c74 = saved;
    }

    dx = 0;
    dy = 0;
    len = w - bord[style + 1].width - bord[style + 3].width;
    nx = len / bord[style + 2].width;
    if (nx * bord[style + 1].width < len)
        nx++;
    x0 = x + bord[style + 1].width;
    y0 = y;
    x1 = x0 + len;
    y1 = y0 + h;
    find_common_window(x0, y0, x1, y1, window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
    if (info.unknown78 == 0) {
        LbScreenSetGraphicsWindow(info.unknown68, info.unknown6c, info.unknown70 - info.unknown68,
                                  info.unknown74 - info.unknown6c);
        dx += x0 - info.unknown68;
        dy += y0 - info.unknown6c;
        for (j = 0; j < nx; j++) {
            LbSpriteDraw(dx, dy, &bord[style + 2]);
            LbSpriteDraw(dx, h + dy - bord[style + 6].height, &bord[style + 7]);
            dx += bord[style + 2].width;
        }
    }

    dx = 0;
    dy = 0;
    len = h - bord[style + 1].height - bord[style + 6].height;
    ny = len / bord[style + 4].height;
    if (ny * bord[style + 4].height < len)
        ny++;
    x0 = x;
    y0 = y + bord[style + 1].height;
    x1 = x0 + w;
    y1 = y0 + len;
    find_common_window(x0, y0, x1, y1, window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
    if (info.unknown78 == 0) {
        LbScreenSetGraphicsWindow(info.unknown68, info.unknown6c, info.unknown70 - info.unknown68,
                                  info.unknown74 - info.unknown6c);
        dx += x0 - info.unknown68;
        dy += y0 - info.unknown6c;
        for (j = 0; j < ny; j++) {
            LbSpriteDraw(dx, dy, &bord[style + 4]);
            LbSpriteDraw(w + dx - bord[style + 5].width, dy, &bord[style + 5]);
            dy += bord[style + 4].height;
        }
    }

    LbScreenSetGraphicsWindow(x, y, w, h);
    dx = 0;
    dy = 0;
    x0 = x;
    y0 = y;
    x1 = x0 + w;
    y1 = y0 + h;
    find_common_window(x0, y0, x1, y1, window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
    if (info.unknown78 == 0) {
        LbScreenSetGraphicsWindow(info.unknown68, info.unknown6c, info.unknown70 - info.unknown68,
                                  info.unknown74 - info.unknown6c);
        dx += x0 - info.unknown68;
        dy += y0 - info.unknown6c;
        LbSpriteDraw(dx, dy, &bord[style + 1]);
        LbSpriteDraw(w + dx - bord[style + 3].width, dy, &bord[style + 3]);
        LbSpriteDraw(dx, h + dy - bord[style + 6].height, &bord[style + 6]);
        LbSpriteDraw(w + dx - bord[style + 8].width, h + dy - bord[style + 8].height, &bord[style + 8]);
    }
    set_all_refresh_zones_in_window(x, y, x + w, y + h, 3);
}

// FUNCTION: 0x0004c670
void draw_window(Window *w)
{
    s16 width;
    s16 height;
    s16 x;
    s16 y;

    GetWindowDimensions(w, (u16 *)&width, (u16 *)&height);
    x = w->x;
    y = w->y;
    if (w->flags & 1) {
        D_00188c74 = 0;
        set_all_refresh_zones_in_window(x, y, x + width, y + height, 3);
    } else if (w->flags & 2) {
        draw_a_window(x, y, width, height, 0);
    } else if (w->flags & 4) {
        draw_a_window(x, y, width, height, 1);
    }
}

// FUNCTION: 0x0004c730
void DrawWindows(Window **windows)
{
    s16 i;
    u16 count;
    Window *w;

    count = CountWindows(windows);
    for (i = count - 1; i >= 0; i--) {
        w = windows[i];
        draw_window(w);
        draw_buttons(w);
        if (w->draw)
            w->draw(w);
    }
}
