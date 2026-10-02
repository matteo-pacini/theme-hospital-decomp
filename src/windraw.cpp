#include "hosp.h"

extern "C" {
}

// Draws win's visible buttons in their pressed, highlighted or normal
// sprite.
// FUNCTION: 0x0004bcc0
void draw_buttons(Window *win)
{
    s32 unused1c;
    s32 unused18;
    s32 unused14;
    WindowButton *button;
    s32 x;
    s32 y;

    button = win->buttons;
    LbScreenSetGraphicsWindow(window.clip_x0, window.clip_y0, window.clip_x1 - window.clip_x0,
                              window.clip_y1 - window.clip_y0);
    for (; button; button = button->next) {
        if (button->flags & 0x20001000)
            continue;
        x = win->x - lbDisplay.graphics_window_x;
        y = win->y - lbDisplay.graphics_window_y;
        if (button->flags & 1)
            reqspr_draw((s16)x + button->disabled_x, (s16)y + button->disabled_y,
                        &button_sprite[button->disabled_sprite]);
        else if ((button->flags & 6) && (!(button->flags & 2) || !(button->flags & 4)))
            reqspr_draw((s16)x + button->pressed_x, (s16)y + button->pressed_y, &button_sprite[button->pressed_sprite]);
        else
            reqspr_draw((s16)x + button->x, (s16)y + button->y, &button_sprite[button->sprite]);
    }
}

// Draws a window frame of style `style` (0 or 1) at (x, y), w x h: the
// tiled background, edges and corners from the bord sprites, clipped to the
// view area; marks it for redraw.
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
    find_common_window(x0, y0, x1, y1, window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
    if (info.clip_empty == 0) {
        LbScreenSetGraphicsWindow(info.clip_x0, info.clip_y0, info.clip_x1 - info.clip_x0, info.clip_y1 - info.clip_y0);
        dx += x0 - info.clip_x0;
        dy += y0 - info.clip_y0;
        len = dx;
        saved = lbDisplay.draw_flags;
        lbDisplay.draw_flags = 8;
        for (i = 0; i < ny; i++) {
            for (j = 0; j < nx; j++) {
                LbSpriteDraw(dx, dy, bord + 17);
                dx += bord[17].width;
            }
            dx = len;
            dy += bord[17].height;
        }
        lbDisplay.draw_flags = saved;
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
    find_common_window(x0, y0, x1, y1, window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
    if (info.clip_empty == 0) {
        LbScreenSetGraphicsWindow(info.clip_x0, info.clip_y0, info.clip_x1 - info.clip_x0, info.clip_y1 - info.clip_y0);
        dx += x0 - info.clip_x0;
        dy += y0 - info.clip_y0;
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
    find_common_window(x0, y0, x1, y1, window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
    if (info.clip_empty == 0) {
        LbScreenSetGraphicsWindow(info.clip_x0, info.clip_y0, info.clip_x1 - info.clip_x0, info.clip_y1 - info.clip_y0);
        dx += x0 - info.clip_x0;
        dy += y0 - info.clip_y0;
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
    find_common_window(x0, y0, x1, y1, window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
    if (info.clip_empty == 0) {
        LbScreenSetGraphicsWindow(info.clip_x0, info.clip_y0, info.clip_x1 - info.clip_x0, info.clip_y1 - info.clip_y0);
        dx += x0 - info.clip_x0;
        dy += y0 - info.clip_y0;
        LbSpriteDraw(dx, dy, &bord[style + 1]);
        LbSpriteDraw(w + dx - bord[style + 3].width, dy, &bord[style + 3]);
        LbSpriteDraw(dx, h + dy - bord[style + 6].height, &bord[style + 6]);
        LbSpriteDraw(w + dx - bord[style + 8].width, h + dy - bord[style + 8].height, &bord[style + 8]);
    }
    set_all_refresh_zones_in_window(x, y, x + w, y + h, 3);
}

// Draws win's frame by its flags: 1 none (only marks the area for redraw),
// 2 frame style 0, 4 frame style 1.
// FUNCTION: 0x0004c670
void draw_window(Window *win)
{
    s16 width;
    s16 height;
    s16 x;
    s16 y;

    GetWindowDimensions(win, (u16 *)&width, (u16 *)&height);
    x = win->x;
    y = win->y;
    if (win->flags & 1) {
        lbDisplay.draw_flags = 0;
        set_all_refresh_zones_in_window(x, y, x + width, y + height, 3);
    } else if (win->flags & 2) {
        draw_a_window(x, y, width, height, 0);
    } else if (win->flags & 4) {
        draw_a_window(x, y, width, height, 1);
    }
}

// Draws the open windows (bottom one first): frame, buttons and each one's
// own drawing routine.
// FUNCTION: 0x0004c730
void DrawWindows(Window **windows)
{
    s16 i;
    u16 count;
    Window *win;

    count = CountWindows(windows);
    for (i = count - 1; i >= 0; i--) {
        win = windows[i];
        draw_window(win);
        draw_buttons(win);
        if (win->draw)
            win->draw(win);
    }
}
