#include "bflib.h"

extern "C" {

void fn_000ca028(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);

int abs(int n);
#pragma intrinsic(abs)

// FUNCTION: 0x000c9be0
u8 fn_000c9be0(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour)
{
    u8 clipped;

    if (x1 < 0) {
        if (x2 < 0)
            return 1;
        {
            s32 d1;
            s32 d2;
            s32 d3;

            d1 = x1 - x2;
            d2 = x1;
            d3 = y1 - y2;
            d3 = d3 * d2 / d1;
            x1 = 0;
            y1 -= d3;
            clipped = 1;
        }
        if (x2 >= lbDisplay.graphics_window_width) {
            s32 d1;
            s32 d2;
            s32 d3;

            d1 = x2 - x1;
            d2 = x2 - (lbDisplay.graphics_window_width - 1);
            d3 = y2 - y1;
            d3 = d3 * d2 / d1;
            x2 = lbDisplay.graphics_window_width - 1;
            y2 -= d3;
        }
    } else if (x1 >= lbDisplay.graphics_window_width) {
        if (x2 >= lbDisplay.graphics_window_width)
            return 1;
        {
            s32 d1;
            s32 d2;
            s32 d3;

            d1 = x1 - x2;
            d2 = x1 - (lbDisplay.graphics_window_width - 1);
            d3 = y1 - y2;
            d3 = d3 * d2 / d1;
            x1 = lbDisplay.graphics_window_width - 1;
            y1 -= d3;
            clipped = 1;
        }
        if (x2 < 0) {
            s32 d1;
            s32 d2;
            s32 d3;

            d1 = x2 - x1;
            d2 = x2;
            d3 = y2 - y1;
            d3 = d3 * d2 / d1;
            x2 = 0;
            y2 -= d3;
        }
    } else if (x2 < 0) {
        s32 d1;
        s32 d2;
        s32 d3;

        d1 = x2 - x1;
        d2 = x2;
        d3 = y2 - y1;
        d3 = d3 * d2 / d1;
        x2 = 0;
        y2 -= d3;
        clipped = 1;
    } else if (x2 >= lbDisplay.graphics_window_width) {
        s32 d1;
        s32 d2;
        s32 d3;

        d1 = x2 - x1;
        d2 = x2 - (lbDisplay.graphics_window_width - 1);
        d3 = y2 - y1;
        d3 = d3 * d2 / d1;
        x2 = lbDisplay.graphics_window_width - 1;
        y2 -= d3;
        clipped = 1;
    }
    if (y1 < 0) {
        if (y2 < 0)
            return 1;
        {
            s32 d1;
            s32 d2;
            s32 d3;

            d1 = y1 - y2;
            d2 = y1;
            d3 = x1 - x2;
            d3 = d3 * d2 / d1;
            y1 = 0;
            x1 -= d3;
            clipped = 1;
        }
        if (y2 >= lbDisplay.graphics_window_height) {
            s32 d1;
            s32 d2;
            s32 d3;

            d1 = y2 - y1;
            d2 = y2 - (lbDisplay.graphics_window_height - 1);
            d3 = x2 - x1;
            d3 = d3 * d2 / d1;
            y2 = lbDisplay.graphics_window_height - 1;
            x2 -= d3;
        }
    } else if (y1 >= lbDisplay.graphics_window_height) {
        if (y2 >= lbDisplay.graphics_window_height)
            return 1;
        {
            s32 d1;
            s32 d2;
            s32 d3;

            d1 = y1 - y2;
            d2 = y1 - (lbDisplay.graphics_window_height - 1);
            d3 = x1 - x2;
            d3 = d3 * d2 / d1;
            y1 = lbDisplay.graphics_window_height - 1;
            x1 -= d3;
            clipped = 1;
        }
        if (y2 < 0) {
            s32 d1;
            s32 d2;
            s32 d3;

            d1 = y2 - y1;
            d2 = y2;
            d3 = x2 - x1;
            d3 = d3 * d2 / d1;
            y2 = 0;
            x2 -= d3;
        }
    } else if (y2 < 0) {
        s32 d1;
        s32 d2;
        s32 d3;

        d1 = y2 - y1;
        d2 = y2;
        d3 = x2 - x1;
        d3 = d3 * d2 / d1;
        y2 = 0;
        x2 -= d3;
        clipped = 1;
    } else if (y2 >= lbDisplay.graphics_window_height) {
        s32 d1;
        s32 d2;
        s32 d3;

        d1 = y2 - y1;
        d2 = y2 - (lbDisplay.graphics_window_height - 1);
        d3 = x2 - x1;
        d3 = d3 * d2 / d1;
        y2 = lbDisplay.graphics_window_height - 1;
        x2 -= d3;
        clipped = 1;
    }
    fn_000ca028(x1, y1, x2, y2, colour);
    return clipped;
}

// FUNCTION: 0x000ca028
void fn_000ca028(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour)
{
    s32 d;
    s32 x;
    s32 y;
    s32 ax;
    s32 ay;
    s32 sx;
    s32 sy;
    s32 dx;
    s32 dy;
    s32 row;
    s32 pos;

    dx = x2 - x1;
    ax = abs(dx) * 2;
    sx = dx > 0 ? 1 : -1;
    dy = y2 - y1;
    ay = abs(dy) * 2;
    sy = dy > 0 ? 1 : -1;
    row = lbDisplay.graphics_screen_width * sy;
    pos = x1 + y1 * lbDisplay.graphics_screen_width;
    x = x1;
    y = y1;
    if (lbDisplay.draw_flags & 4) {
        if (ax > ay) {
            d = ay - (ax >> 1);
            for (;;) {
                lbDisplay.graphics_window[pos] = lbDisplay.ghost[(colour << 8) + lbDisplay.graphics_window[pos]];
                if (x == x2)
                    return;
                if (d >= 0) {
                    pos += row;
                    d -= ax;
                }
                x += sx;
                pos += sx;
                d += ay;
            }
        } else {
            d = ax - (ay >> 1);
            for (;;) {
                lbDisplay.graphics_window[pos] = lbDisplay.ghost[(colour << 8) + lbDisplay.graphics_window[pos]];
                if (y == y2)
                    return;
                if (d >= 0) {
                    pos += sx;
                    d -= ay;
                }
                y += sy;
                pos += row;
                d += ax;
            }
        }
    } else if (lbDisplay.draw_flags & 8) {
        if (ax > ay) {
            d = ay - (ax >> 1);
            for (;;) {
                lbDisplay.graphics_window[pos] = lbDisplay.ghost[(lbDisplay.graphics_window[pos] << 8) + colour];
                if (x == x2)
                    return;
                if (d >= 0) {
                    pos += row;
                    d -= ax;
                }
                x += sx;
                pos += sx;
                d += ay;
            }
        } else {
            d = ax - (ay >> 1);
            for (;;) {
                lbDisplay.graphics_window[pos] = lbDisplay.ghost[(lbDisplay.graphics_window[pos] << 8) + colour];
                if (y == y2)
                    return;
                if (d >= 0) {
                    pos += sx;
                    d -= ay;
                }
                y += sy;
                pos += row;
                d += ax;
            }
        }
    } else {
        if (ax > ay) {
            d = ay - (ax >> 1);
            for (;;) {
                lbDisplay.graphics_window[pos] = colour;
                if (x == x2)
                    return;
                if (d >= 0) {
                    pos += row;
                    d -= ax;
                }
                x += sx;
                pos += sx;
                d += ay;
            }
        } else {
            d = ax - (ay >> 1);
            for (;;) {
                lbDisplay.graphics_window[pos] = colour;
                if (y == y2)
                    return;
                if (d >= 0) {
                    pos += sx;
                    d -= ay;
                }
                y += sy;
                pos += row;
                d += ax;
            }
        }
    }
}

}
