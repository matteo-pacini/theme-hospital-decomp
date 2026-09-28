#include "hosp.h"

extern "C" {

extern s32 D_00188c3c;
extern s32 D_00188c40;
extern s32 D_00188c38;
extern volatile s32 D_00188c5c;
extern volatile s32 D_00188c60;
extern u8 D_00188c7e;
extern u8 D_00188c7f;
extern u8 D_00155420;
extern u8 D_00188c7b;
extern u8 D_00188c7c;

}

extern u32 tool_tip_count;
extern u16 old_mousex;
extern u16 old_mousey;
extern u8 update_state;
extern u16 text_position;
#pragma aux text_position "W?text_position$:.0$:?process_buttons$n(pnpn$Window$$pn$2$)vnus"

// FUNCTION: 0x0004c7a0
void reposition_buttons(Window *w, s32 dx, s32 dy)
{
    WindowButton *b;
    u16 width;
    u16 height;

    b = w->buttons;
    GetWindowDimensions(w, &width, &height);
    for (; b; b = b->next) {
        if (b->flags & 0x1000000)
            b->width += dx;
        else if (b->flags & 0x20000)
            b->x += dx;
        else if (b->flags & 0x40000)
            b->x = (width - b->width) / 2;
        if (b->flags & 0x2000000)
            b->height += dy;
        else if (b->flags & 0x10000)
            b->y += dy;
        else if (b->flags & 0x80000)
            b->y = (height - b->height) / 2;
    }
}

// FUNCTION: 0x0004c89c
void clear_buttons(Window *w)
{
    WindowButton *b;

    for (b = w->buttons; b; b = b->next) {
        if (!(b->flags & 0x4000000)) {
            if (b->flags & 4)
                b->flags &= ~4;
        }
        if (b->flags & 2)
            b->flags &= ~2;
    }
}

// FUNCTION: 0x0004c8fc
u8 mouse_over(s32 x1, s32 y1, s32 x2, s32 y2)
{
    if (mouse_dx - D_00188c3c >= x1 && mouse_dx - D_00188c3c <= x2 && mouse_dy - D_00188c40 >= y1
        && mouse_dy - D_00188c40 <= y2)
        return 1;
    return 0;
}

// FUNCTION: 0x0004c96c
u8 mouse_pressed_over(s32 x1, s32 y1, s32 x2, s32 y2)
{
    if (D_00188c5c - D_00188c3c >= x1 && D_00188c5c - D_00188c3c <= x2 && D_00188c60 - D_00188c40 >= y1
        && D_00188c60 - D_00188c40 <= y2)
        return 1;
    return 0;
}

// FUNCTION: 0x0004c9dc
u8 remove_window(Window **windows, Window *w)
{
    u16 i;
    u8 removed;
    u16 count;
    u16 height;
    u16 width;

    removed = 0;
    CurrentWindow = -1;
    for (i = 0; i < 2; i++) {
        if (windows[i] == w) {
            mouse_was_on_window = 0;
            GetWindowDimensions(w, &width, &height);
            count = CountWindows(windows);
            clear_buttons(w);
            removed = 1;
            for (; i < 1; i++)
                windows[i] = windows[i + 1];
            windows[i] = 0;
            break;
        }
    }
    return removed;
}

// FUNCTION: 0x0004cac0
void calculate_currentbuttonstore(Window *w)
{
    WindowButton *b;

    for (b = w->buttons; b; b = b->next) {
        if (!(b->flags & 0x1801)) {
            if (mouse_over(b->x + w->x, b->y + w->y, b->x + w->x + b->width - 1, b->y + w->y + b->height - 1)) {
                buttonstore = b;
                break;
            }
        }
    }
}

// FUNCTION: 0x0004cb80
u8 process_buttons_normal_state(Window **windows, Window *w, u16 *pos)
{
    u8 pressed;
    s16 ox;
    u8 over;
    u8 notover;
    s16 oy;
    WindowButton *b;
    s32 val;

    val = 0;
    notover = 1;
    pressed = 0;
    buttonstore = 0;
    for (b = w->buttons; b; b = b->next) {
        if (!(b->flags & 0x1801)) {
            over = mouse_over(b->x + w->x, b->y + w->y, b->x + w->x + b->width - 1, b->y + w->y + b->height - 1);
            if (over) {
                notover = 0;
                if (!buttonstore)
                    buttonstore = b;
            }
            if (over
                && ((b->flags & 8) || ((b->flags & 0x10) && D_00188c7e) || ((b->flags & 0x20) && D_00188c7f))) {
                if (!(b->flags & 2))
                    b->flags |= 2;
            } else {
                if (b->flags & 2)
                    b->flags &= ~2;
            }
            if (over
                    && ((b->flags & 0x40) || (pressed = (b->flags & 0x80) && D_00188c7e)
                        || (pressed = (b->flags & 0x8000000) && D_00188c7b)
                        || (pressed = (b->flags & 0x200) && (OldLeftButton && !D_00188c7e))
                        || ((b->flags & 0x100) && D_00188c7f) || ((b->flags & 0x10000000) && D_00188c7c)
                        || ((b->flags & 0x400) && (OldRightButton && !D_00188c7f)))
                || lbKeyOn[0x20]) {
                tool_tip_count = 0;
                if (b->flags & 0x4000000)
                    b->flags ^= 4;
                else
                    b->flags |= 4;
                if (b->flags & 0x200000) {
                    if (pressed) {
                        if (w->flags & 0x80) {
                            w->flags &= ~0x180;
                            w->x = w->x4;
                            w->y = w->y6;
                            ox = w->width - w->widthC;
                            oy = w->height - w->heightE;
                        } else if (w->flags & 0x100) {
                            w->flags = w->flags & ~0x100 | 0x80;
                            w->x = w->x4;
                            w->y = w->y6;
                            if (w->x < 0)
                                w->x = 0;
                            else if (w->x > D_00188c34 - w->widthC)
                                w->x = D_00188c34 - w->widthC;
                            if (w->y < 0)
                                w->y = 0;
                            else if (w->y > D_00188c38 - w->heightE)
                                w->y = D_00188c38 - w->heightE;
                            ox = -(w->width10 - w->widthC);
                            oy = -(w->height12 - w->heightE);
                        } else {
                            w->flags = w->flags & ~0x80 | 0x100;
                            w->x4 = w->x;
                            w->y6 = w->y;
                            if (w->x < 0)
                                w->x = 0;
                            else if (w->x > D_00188c34 - w->width10)
                                w->x = D_00188c34 - w->width10;
                            if (w->y < 0)
                                w->y = 0;
                            else if (w->y > D_00188c38 - w->height12)
                                w->y = D_00188c38 - w->height12;
                            ox = w->width10 - w->width;
                            oy = w->height12 - w->height;
                        }
                    } else {
                        if (w->flags & 0x80) {
                            w->flags = w->flags & ~0x80 | 0x100;
                            if (w->x < 0)
                                w->x = 0;
                            else if (w->x > D_00188c34 - w->width10)
                                w->x = D_00188c34 - w->width10;
                            if (w->y < 0)
                                w->y = 0;
                            else if (w->y > D_00188c38 - w->height12)
                                w->y = D_00188c38 - w->height12;
                            ox = w->width10 - w->widthC;
                            oy = w->height12 - w->heightE;
                        } else if (w->flags & 0x100) {
                            w->flags &= ~0x180;
                            w->x = w->x4;
                            w->y = w->y6;
                            ox = -(w->width10 - w->width);
                            oy = -(w->height12 - w->height);
                        } else {
                            w->flags = w->flags & ~0x100 | 0x80;
                            w->x4 = w->x;
                            w->y6 = w->y;
                            ox = -(w->width - w->widthC);
                            oy = -(w->height - w->heightE);
                            if (w->x < 0)
                                w->x = 0;
                            else if (w->x > D_00188c34 - w->widthC)
                                w->x = D_00188c34 - w->widthC;
                            if (w->y < 0)
                                w->y = 0;
                            else if (w->y > D_00188c38 - w->heightE)
                                w->y = D_00188c38 - w->heightE;
                        }
                    }
                    reposition_buttons(w, ox, oy);
                } else if (b->flags & 0x800000) {
                    update_state = 1;
                    if (w->flags & 0x80) {
                        w->flags &= ~0x180;
                        w->width = w->widthC;
                        w->height = w->heightE;
                    } else if (w->flags & 0x100) {
                        w->flags &= ~0x180;
                        w->width = w->width10;
                        w->height = w->height12;
                    }
                    buttonstore = b;
                } else if (b->flags & 0x100000) {
                    update_state = 2;
                    buttonstore = b;
                } else if (b->flags & 0x400000) {
                    remove_window(windows, w);
                }
                if (b->max > b->min) {
                    if (b->width > button_sprite[b->sprite18].width) {
                        ox = b->x + w->x
                            + (b->width - button_sprite[b->sprite18].width) * (*b->value - b->min) / (b->max - b->min)
                            + button_sprite[b->sprite18].width / 2;
                        ox = mouse_dx - D_00188c3c - ox;
                        val = ox * (b->max - b->min) / (b->width - button_sprite[b->sprite18].width);
                    }
                    if (b->height > button_sprite[b->sprite18].height) {
                        oy = b->y + w->y + button_sprite[b->sprite18].height / 2
                            + (b->height - button_sprite[b->sprite18].height) * (b->max - *b->value)
                                / (b->max - b->min);
                        oy = -(mouse_dy - D_00188c40 - oy);
                        if (val) {
                            val += oy * (b->max - b->min) / (b->height - button_sprite[b->sprite18].height);
                            val /= 2;
                        } else {
                            val += oy * (b->max - b->min) / (b->height - button_sprite[b->sprite18].height);
                        }
                    }
                    if (b->max > b->min) {
                        if (pressed) {
                            update_state = 3;
                            *b->value += val;
                            if (b->callback)
                                b->callback(w);
                            buttonstore = b;
                        } else if (val > 0) {
                            (*b->value)++;
                        } else if (val < 0) {
                            (*b->value)--;
                        }
                        if (*b->value < b->min)
                            *b->value = b->min;
                        else if (*b->value > b->max)
                            *b->value = b->max;
                    }
                }
                if (b->callback)
                    b->callback(w);
                b = b->next;
                break;
            } else {
                if (!(b->flags & 0x4000000)) {
                    if (b->flags & 4)
                        b->flags &= ~4;
                }
            }
        } else if (!buttonstore
                   && mouse_over(b->x + w->x, b->y + w->y, b->x + w->x + b->width - 1, b->y + w->y + b->height - 1)) {
            buttonstore = b;
        }
    }
    for (; b; b = b->next) {
        if (!(b->flags & 0x4000000)) {
            if (b->flags & 4)
                b->flags &= ~4;
        }
    }
    if (notover) {
        if ((D_00188c7b || D_00188c7c) && (w->flags & 0x10)) {
            if (MousePressedOnWindow(w))
                update_state = 2;
        }
    }
    return update_state;
}

// NONMATCHING: operand evaluation order and registers in the drag-resize bound checks (case 1)
// FUNCTION: 0x0004d740
void process_buttons(Window **windows, Window *w)
{
    s16 mdy;
    s16 ox;
    s16 oy;
    s16 mdx;
    s32 val;

    val = 0;
    mdx = mouse_dx - D_00188c3c - old_mousex;
    mdy = mouse_dy - D_00188c40 - old_mousey;
    old_mousex = mouse_dx - D_00188c3c;
    old_mousey = mouse_dy - D_00188c40;
    if (mdx || mdy)
        tool_tip_count = 0;
    else
        tool_tip_count++;
    switch (update_state) {
    case 1:
        if ((buttonstore->flags & 0x80) && D_00188c7e || (buttonstore->flags & 0x100) && D_00188c7f) {
            if ((mdx > 0 && mouse_dx - D_00188c3c <= w->x + buttonstore->x)
                || (mdx < 0 && mouse_dx - D_00188c3c >= w->x + buttonstore->x + buttonstore->width)) {
                ox = 0;
            } else if (w->flags & 0x20) {
                ox = mdx * 2;
            } else {
                ox = mdx;
                if (ox < -(w->x + buttonstore->x + buttonstore->width - 3))
                    ox = -(w->x + buttonstore->x + buttonstore->width - 3);
                else if (ox > D_00188c34 - 3 - fn_000cb2db(w->x + buttonstore->x))
                    ox = D_00188c34 - 3 - fn_000cb2db(w->x + buttonstore->x);
            }
            if ((mdy > 0 && mouse_dy - D_00188c40 <= w->y + buttonstore->y)
                || (mdy < 0 && mouse_dy - D_00188c40 >= w->y + buttonstore->y + buttonstore->height)) {
                oy = 0;
            } else if (w->flags & 0x40) {
                oy = mdy * 2;
            } else {
                oy = mdy;
                if (oy < -(w->y + buttonstore->y + buttonstore->height - 3))
                    oy = -(w->y + buttonstore->y + buttonstore->height - 3);
                else if (oy > D_00188c38 - 3 - fn_000cb2db(w->y + buttonstore->y))
                    oy = D_00188c38 - 3 - fn_000cb2db(w->y + buttonstore->y);
            }
            if (ox < -(w->width - w->widthC))
                ox = -(w->width - w->widthC);
            else if (ox > w->width10 - w->width)
                ox = w->width10 - w->width;
            if (oy < -(w->height - w->heightE))
                oy = -(w->height - w->heightE);
            else if (oy > w->height12 - w->height)
                oy = w->height12 - w->height;
            if (ox || oy) {
                w->width += ox;
                w->height += oy;
                reposition_buttons(w, ox, oy);
            }
            if (buttonstore && buttonstore->callback)
                buttonstore->callback(w);
        } else {
            buttonstore->flags &= ~4;
            update_state = 0;
            buttonstore = 0;
        }
        break;
    case 3:
        if (D_00188c7e) {
            if (mdx || mdy) {
                if (buttonstore->width > button_sprite[buttonstore->sprite18].width) {
                    ox = buttonstore->x + w->x
                        + (buttonstore->width - button_sprite[buttonstore->sprite18].width)
                            * (*buttonstore->value - buttonstore->min) / (buttonstore->max - buttonstore->min)
                        + button_sprite[buttonstore->sprite18].width / 2;
                    ox = mouse_dx - D_00188c3c - ox;
                    val = ox * (buttonstore->max - buttonstore->min)
                        / (buttonstore->width - button_sprite[buttonstore->sprite18].width);
                }
                if (buttonstore->height > button_sprite[buttonstore->sprite18].height) {
                    oy = buttonstore->y + w->y + button_sprite[buttonstore->sprite18].height / 2
                        + (buttonstore->max - *buttonstore->value)
                            * (buttonstore->height - button_sprite[buttonstore->sprite18].height)
                            / (buttonstore->max - buttonstore->min);
                    oy = -(mouse_dy - D_00188c40 - oy);
                    if (val) {
                        val += oy * (buttonstore->max - buttonstore->min)
                            / (buttonstore->height - button_sprite[buttonstore->sprite18].height);
                        val /= 2;
                    } else {
                        val += oy * (buttonstore->max - buttonstore->min)
                            / (buttonstore->height - button_sprite[buttonstore->sprite18].height);
                    }
                }
                if (buttonstore->max > buttonstore->min) {
                    *buttonstore->value += val;
                    if (*buttonstore->value < buttonstore->min)
                        *buttonstore->value = buttonstore->min;
                    else if (*buttonstore->value > buttonstore->max)
                        *buttonstore->value = buttonstore->max;
                }
            }
            if (buttonstore && buttonstore->callback)
                buttonstore->callback(w);
        } else {
            buttonstore->flags &= ~4;
            update_state = 0;
            buttonstore = 0;
        }
        break;
    case 4:
        break;
    case 5:
        tool_tip_count = 0;
        break;
    case 2:
    default:
        process_buttons_normal_state(windows, w, &text_position);
        break;
    }
}

// FUNCTION: 0x0004df88
u8 insert_window(Window **windows, Window *w)
{
    u16 i;

    for (i = 0; i < 2; i++) {
        if (windows[i] == 0) {
            for (; i; i--)
                windows[i] = windows[i - 1];
            windows[i] = w;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0004e024
u8 make_front_window(Window **windows, Window *w)
{
    u16 i;

    for (i = 0; i < 2; i++) {
        if (windows[i] == w) {
            for (; i; i--)
                windows[i] = windows[i - 1];
            windows[i] = w;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0004e0c0
u8 OpenWindow(Window **windows, Window *list, u16 idx)
{
    CurrentWindow = -1;
    mouse_was_on_window = 0;
    OldRightButton = OldLeftButton = 0;
    if (windows[0] && (windows[0]->flags & 0x600))
        return 0;
    if (!make_front_window(windows, &list[idx]))
        return insert_window(windows, &list[idx]);
    return 1;
}

// FUNCTION: 0x0004e154
u16 CountWindows(Window **windows)
{
    u16 i;

    for (i = 0; windows[i] && i < 2; i++)
        ;
    return i;
}

// FUNCTION: 0x0004e1a8
void GetWindowDimensions(Window *w, u16 *width, u16 *height)
{
    if (w) {
        if (w->flags & 0x100) {
            *width = w->width10;
            *height = w->height12;
        } else if (w->flags & 0x80) {
            *width = w->widthC;
            *height = w->heightE;
        } else {
            *width = w->width;
            *height = w->height;
        }
    } else {
        *width = 1;
        *height = 1;
    }
}

// FUNCTION: 0x0004e240
u8 MouseOnWindow(Window *w)
{
    u16 width;
    u16 height;

    if (w) {
        GetWindowDimensions(w, &width, &height);
        return mouse_over(w->x, w->y, w->x + width - 1, w->y + height - 1);
    }
    return 0;
}

// FUNCTION: 0x0004e2b4
u8 MousePressedOnWindow(Window *w)
{
    u16 width;
    u16 height;

    if (w) {
        GetWindowDimensions(w, &width, &height);
        return mouse_pressed_over(w->x, w->y, w->x + width - 1, w->y + height - 1);
    }
    return 0;
}

// FUNCTION: 0x0004e328
void process_window(Window **windows, Window *w)
{
    process_buttons(windows, w);
    OldLeftButton = D_00188c7e;
    OldRightButton = D_00188c7f;
    if (w->flags & 0x100) {
        if (w->flags & 0x20)
            w->x = (D_00188c34 - w->width10) / 2;
        if (w->flags & 0x40)
            w->y = (D_00188c38 - w->height12) / 2;
    } else if (w->flags & 0x80) {
        if (w->flags & 0x20)
            w->x = (D_00188c34 - w->widthC) / 2;
        if (w->flags & 0x40)
            w->y = (D_00188c38 - w->heightE) / 2;
    } else {
        if (w->flags & 0x20)
            w->x = (D_00188c34 - w->width) / 2;
        if (w->flags & 0x40)
            w->y = (D_00188c38 - w->height) / 2;
    }
}

// FUNCTION: 0x0004e47c
extern "C" void fn_0004e47c(Window **windows)
{
    u8 front;
    s32 i;
    s32 count;
    Window *w;

    w = windows[0];
    front = 0;
    if (gamevar.f12e == 0x13)
        return;
    CurrentWindow = -1;
    if (!w)
        return;
    if (MouseOnWindow(w)) {
        if (!mouse_was_on_window) {
            D_00155420 = gamevar.f12e;
            set_mouse_mode(0);
            mouse_was_on_window = 1;
        }
    } else if (mouse_was_on_window) {
        mouse_was_on_window = 0;
        set_mouse_mode(D_00155420);
    }
    if (w->flags & 0x600) {
        count = CountWindows(windows);
        for (i = count - 1; i > 0; i--) {
            w = windows[i];
            if (MouseOnWindow(w))
                CurrentWindow = i;
            clear_buttons(w);
        }
        w = windows[0];
        if (MouseOnWindow(w))
            CurrentWindow = 0;
        process_window(windows, w);
    } else {
        if (D_00188c7b || D_00188c7c) {
            for (i = 0; w; i++) {
                w = windows[i];
                if (w) {
                    if (MousePressedOnWindow(w)) {
                        CurrentWindow = i;
                        if (w != windows[0]) {
                            make_front_window(windows, w);
                            front = 1;
                        }
                        break;
                    }
                } else {
                    break;
                }
            }
        } else {
            for (i = 0; w; i++) {
                w = windows[i];
                if (w) {
                    if (MouseOnWindow(w)) {
                        CurrentWindow = i;
                        break;
                    }
                } else {
                    break;
                }
            }
        }
        count = CountWindows(windows);
        for (i = count - 1; i > 0; i--) {
            w = windows[i];
            clear_buttons(w);
        }
        if (!front) {
            w = windows[0];
            if (MouseOnWindow(w))
                CurrentWindow = 0;
            process_window(windows, w);
        } else {
            w = windows[0];
            process_window(windows, w);
        }
    }
    if (CurrentWindow > 0)
        calculate_currentbuttonstore(windows[CurrentWindow]);
}

// FUNCTION: 0x0004e6f8
extern "C" s16 fn_0004e6f8(WindowButton *b)
{
    s32 i;

    for (i = 0; i < 500; i++) {
        if (&resvars.f24_WindowButtonp[i] == b)
            break;
    }
    if (i == 500)
        return -1;
    return i;
}
