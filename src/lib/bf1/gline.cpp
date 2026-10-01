#include "bflib.h"

extern "C" {

void draw_b_line(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);

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
            s32 span;
            s32 overshoot;
            s32 shift;

            span = x1 - x2;
            overshoot = x1;
            shift = y1 - y2;
            shift = shift * overshoot / span;
            x1 = 0;
            y1 -= shift;
            clipped = 1;
        }
        if (x2 >= lbDisplay.graphics_window_width) {
            s32 span;
            s32 overshoot;
            s32 shift;

            span = x2 - x1;
            overshoot = x2 - (lbDisplay.graphics_window_width - 1);
            shift = y2 - y1;
            shift = shift * overshoot / span;
            x2 = lbDisplay.graphics_window_width - 1;
            y2 -= shift;
        }
    } else if (x1 >= lbDisplay.graphics_window_width) {
        if (x2 >= lbDisplay.graphics_window_width)
            return 1;
        {
            s32 span;
            s32 overshoot;
            s32 shift;

            span = x1 - x2;
            overshoot = x1 - (lbDisplay.graphics_window_width - 1);
            shift = y1 - y2;
            shift = shift * overshoot / span;
            x1 = lbDisplay.graphics_window_width - 1;
            y1 -= shift;
            clipped = 1;
        }
        if (x2 < 0) {
            s32 span;
            s32 overshoot;
            s32 shift;

            span = x2 - x1;
            overshoot = x2;
            shift = y2 - y1;
            shift = shift * overshoot / span;
            x2 = 0;
            y2 -= shift;
        }
    } else if (x2 < 0) {
        s32 span;
        s32 overshoot;
        s32 shift;

        span = x2 - x1;
        overshoot = x2;
        shift = y2 - y1;
        shift = shift * overshoot / span;
        x2 = 0;
        y2 -= shift;
        clipped = 1;
    } else if (x2 >= lbDisplay.graphics_window_width) {
        s32 span;
        s32 overshoot;
        s32 shift;

        span = x2 - x1;
        overshoot = x2 - (lbDisplay.graphics_window_width - 1);
        shift = y2 - y1;
        shift = shift * overshoot / span;
        x2 = lbDisplay.graphics_window_width - 1;
        y2 -= shift;
        clipped = 1;
    }
    if (y1 < 0) {
        if (y2 < 0)
            return 1;
        {
            s32 span;
            s32 overshoot;
            s32 shift;

            span = y1 - y2;
            overshoot = y1;
            shift = x1 - x2;
            shift = shift * overshoot / span;
            y1 = 0;
            x1 -= shift;
            clipped = 1;
        }
        if (y2 >= lbDisplay.graphics_window_height) {
            s32 span;
            s32 overshoot;
            s32 shift;

            span = y2 - y1;
            overshoot = y2 - (lbDisplay.graphics_window_height - 1);
            shift = x2 - x1;
            shift = shift * overshoot / span;
            y2 = lbDisplay.graphics_window_height - 1;
            x2 -= shift;
        }
    } else if (y1 >= lbDisplay.graphics_window_height) {
        if (y2 >= lbDisplay.graphics_window_height)
            return 1;
        {
            s32 span;
            s32 overshoot;
            s32 shift;

            span = y1 - y2;
            overshoot = y1 - (lbDisplay.graphics_window_height - 1);
            shift = x1 - x2;
            shift = shift * overshoot / span;
            y1 = lbDisplay.graphics_window_height - 1;
            x1 -= shift;
            clipped = 1;
        }
        if (y2 < 0) {
            s32 span;
            s32 overshoot;
            s32 shift;

            span = y2 - y1;
            overshoot = y2;
            shift = x2 - x1;
            shift = shift * overshoot / span;
            y2 = 0;
            x2 -= shift;
        }
    } else if (y2 < 0) {
        s32 span;
        s32 overshoot;
        s32 shift;

        span = y2 - y1;
        overshoot = y2;
        shift = x2 - x1;
        shift = shift * overshoot / span;
        y2 = 0;
        x2 -= shift;
        clipped = 1;
    } else if (y2 >= lbDisplay.graphics_window_height) {
        s32 span;
        s32 overshoot;
        s32 shift;

        span = y2 - y1;
        overshoot = y2 - (lbDisplay.graphics_window_height - 1);
        shift = x2 - x1;
        shift = shift * overshoot / span;
        y2 = lbDisplay.graphics_window_height - 1;
        x2 -= shift;
        clipped = 1;
    }
    draw_b_line(x1, y1, x2, y2, colour);
    return clipped;
}

// FUNCTION: 0x000ca028
void draw_b_line(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour)
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
