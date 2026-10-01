#include "hosp.h"

extern "C" {


}

extern u32 tool_tip_count;
extern u16 old_mousex;
extern u16 old_mousey;
extern u8 update_state;
extern u16 text_position;
#pragma aux text_position "W?text_position$:.0$:?process_buttons$n(pnpn$Window$$pn$2$)vnus"

// FUNCTION: 0x0004c7a0
void reposition_buttons(Window *win, s32 dx, s32 dy)
{
    WindowButton *button;
    u16 width;
    u16 height;

    button = win->buttons;
    GetWindowDimensions(win, &width, &height);
    for (; button; button = button->next) {
        if (button->flags & 0x1000000)
            button->width += dx;
        else if (button->flags & 0x20000)
            button->x += dx;
        else if (button->flags & 0x40000)
            button->x = (width - button->width) / 2;
        if (button->flags & 0x2000000)
            button->height += dy;
        else if (button->flags & 0x10000)
            button->y += dy;
        else if (button->flags & 0x80000)
            button->y = (height - button->height) / 2;
    }
}

// FUNCTION: 0x0004c89c
void clear_buttons(Window *win)
{
    WindowButton *button;

    for (button = win->buttons; button; button = button->next) {
        if (!(button->flags & 0x4000000)) {
            if (button->flags & 4)
                button->flags &= ~4;
        }
        if (button->flags & 2)
            button->flags &= ~2;
    }
}

// FUNCTION: 0x0004c8fc
u8 mouse_over(s32 x1, s32 y1, s32 x2, s32 y2)
{
    if (lbDisplay.mmouse_x - lbDisplay.graphics_window_x >= x1 && lbDisplay.mmouse_x - lbDisplay.graphics_window_x <= x2 && lbDisplay.mmouse_y - lbDisplay.graphics_window_y >= y1
        && lbDisplay.mmouse_y - lbDisplay.graphics_window_y <= y2)
        return 1;
    return 0;
}

// FUNCTION: 0x0004c96c
u8 mouse_pressed_over(s32 x1, s32 y1, s32 x2, s32 y2)
{
    if (lbDisplay.mouse_x - lbDisplay.graphics_window_x >= x1 && lbDisplay.mouse_x - lbDisplay.graphics_window_x <= x2 && lbDisplay.mouse_y - lbDisplay.graphics_window_y >= y1
        && lbDisplay.mouse_y - lbDisplay.graphics_window_y <= y2)
        return 1;
    return 0;
}

// FUNCTION: 0x0004c9dc
u8 remove_window(Window **windows, Window *win)
{
    u16 i;
    u8 removed;
    u16 count;
    u16 height;
    u16 width;

    removed = 0;
    CurrentWindow = -1;
    for (i = 0; i < 2; i++) {
        if (windows[i] == win) {
            mouse_was_on_window = 0;
            GetWindowDimensions(win, &width, &height);
            count = CountWindows(windows);
            clear_buttons(win);
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
void calculate_currentbuttonstore(Window *win)
{
    WindowButton *button;

    for (button = win->buttons; button; button = button->next) {
        if (!(button->flags & 0x1801)) {
            if (mouse_over(button->x + win->x, button->y + win->y, button->x + win->x + button->width - 1, button->y + win->y + button->height - 1)) {
                buttonstore = button;
                break;
            }
        }
    }
}

// FUNCTION: 0x0004cb80
u8 process_buttons_normal_state(Window **windows, Window *win, u16 *pos)
{
    u8 pressed;
    s16 ox;
    u8 over;
    u8 notover;
    s16 oy;
    WindowButton *button;
    s32 slider_delta;

    slider_delta = 0;
    notover = 1;
    pressed = 0;
    buttonstore = 0;
    for (button = win->buttons; button; button = button->next) {
        if (!(button->flags & 0x1801)) {
            over = mouse_over(button->x + win->x, button->y + win->y, button->x + win->x + button->width - 1, button->y + win->y + button->height - 1);
            if (over) {
                notover = 0;
                if (!buttonstore)
                    buttonstore = button;
            }
            if (over
                && ((button->flags & 8) || ((button->flags & 0x10) && lbDisplay.mleft_button) || ((button->flags & 0x20) && lbDisplay.mright_button))) {
                if (!(button->flags & 2))
                    button->flags |= 2;
            } else {
                if (button->flags & 2)
                    button->flags &= ~2;
            }
            if (over
                    && ((button->flags & 0x40) || (pressed = (button->flags & 0x80) && lbDisplay.mleft_button)
                        || (pressed = (button->flags & 0x8000000) && lbDisplay.left_button)
                        || (pressed = (button->flags & 0x200) && (OldLeftButton && !lbDisplay.mleft_button))
                        || ((button->flags & 0x100) && lbDisplay.mright_button) || ((button->flags & 0x10000000) && lbDisplay.right_button)
                        || ((button->flags & 0x400) && (OldRightButton && !lbDisplay.mright_button)))
                || lbKeyOn[KC_D]) {
                tool_tip_count = 0;
                if (button->flags & 0x4000000)
                    button->flags ^= 4;
                else
                    button->flags |= 4;
                if (button->flags & 0x200000) {
                    if (pressed) {
                        if (win->flags & 0x80) {
                            win->flags &= ~0x180;
                            win->x = win->x4;
                            win->y = win->y6;
                            ox = win->width - win->widthC;
                            oy = win->height - win->heightE;
                        } else if (win->flags & 0x100) {
                            win->flags = win->flags & ~0x100 | 0x80;
                            win->x = win->x4;
                            win->y = win->y6;
                            if (win->x < 0)
                                win->x = 0;
                            else if (win->x > lbDisplay.graphics_screen_width - win->widthC)
                                win->x = lbDisplay.graphics_screen_width - win->widthC;
                            if (win->y < 0)
                                win->y = 0;
                            else if (win->y > lbDisplay.graphics_screen_height - win->heightE)
                                win->y = lbDisplay.graphics_screen_height - win->heightE;
                            ox = -(win->width10 - win->widthC);
                            oy = -(win->height12 - win->heightE);
                        } else {
                            win->flags = win->flags & ~0x80 | 0x100;
                            win->x4 = win->x;
                            win->y6 = win->y;
                            if (win->x < 0)
                                win->x = 0;
                            else if (win->x > lbDisplay.graphics_screen_width - win->width10)
                                win->x = lbDisplay.graphics_screen_width - win->width10;
                            if (win->y < 0)
                                win->y = 0;
                            else if (win->y > lbDisplay.graphics_screen_height - win->height12)
                                win->y = lbDisplay.graphics_screen_height - win->height12;
                            ox = win->width10 - win->width;
                            oy = win->height12 - win->height;
                        }
                    } else {
                        if (win->flags & 0x80) {
                            win->flags = win->flags & ~0x80 | 0x100;
                            if (win->x < 0)
                                win->x = 0;
                            else if (win->x > lbDisplay.graphics_screen_width - win->width10)
                                win->x = lbDisplay.graphics_screen_width - win->width10;
                            if (win->y < 0)
                                win->y = 0;
                            else if (win->y > lbDisplay.graphics_screen_height - win->height12)
                                win->y = lbDisplay.graphics_screen_height - win->height12;
                            ox = win->width10 - win->widthC;
                            oy = win->height12 - win->heightE;
                        } else if (win->flags & 0x100) {
                            win->flags &= ~0x180;
                            win->x = win->x4;
                            win->y = win->y6;
                            ox = -(win->width10 - win->width);
                            oy = -(win->height12 - win->height);
                        } else {
                            win->flags = win->flags & ~0x100 | 0x80;
                            win->x4 = win->x;
                            win->y6 = win->y;
                            ox = -(win->width - win->widthC);
                            oy = -(win->height - win->heightE);
                            if (win->x < 0)
                                win->x = 0;
                            else if (win->x > lbDisplay.graphics_screen_width - win->widthC)
                                win->x = lbDisplay.graphics_screen_width - win->widthC;
                            if (win->y < 0)
                                win->y = 0;
                            else if (win->y > lbDisplay.graphics_screen_height - win->heightE)
                                win->y = lbDisplay.graphics_screen_height - win->heightE;
                        }
                    }
                    reposition_buttons(win, ox, oy);
                } else if (button->flags & 0x800000) {
                    update_state = 1;
                    if (win->flags & 0x80) {
                        win->flags &= ~0x180;
                        win->width = win->widthC;
                        win->height = win->heightE;
                    } else if (win->flags & 0x100) {
                        win->flags &= ~0x180;
                        win->width = win->width10;
                        win->height = win->height12;
                    }
                    buttonstore = button;
                } else if (button->flags & 0x100000) {
                    update_state = 2;
                    buttonstore = button;
                } else if (button->flags & 0x400000) {
                    remove_window(windows, win);
                }
                if (button->max > button->min) {
                    if (button->width > button_sprite[button->sprite18].width) {
                        ox = button->x + win->x
                            + (button->width - button_sprite[button->sprite18].width) * (*button->value - button->min) / (button->max - button->min)
                            + button_sprite[button->sprite18].width / 2;
                        ox = lbDisplay.mmouse_x - lbDisplay.graphics_window_x - ox;
                        slider_delta = ox * (button->max - button->min) / (button->width - button_sprite[button->sprite18].width);
                    }
                    if (button->height > button_sprite[button->sprite18].height) {
                        oy = button->y + win->y + button_sprite[button->sprite18].height / 2
                            + (button->height - button_sprite[button->sprite18].height) * (button->max - *button->value)
                                / (button->max - button->min);
                        oy = -(lbDisplay.mmouse_y - lbDisplay.graphics_window_y - oy);
                        if (slider_delta) {
                            slider_delta += oy * (button->max - button->min) / (button->height - button_sprite[button->sprite18].height);
                            slider_delta /= 2;
                        } else {
                            slider_delta += oy * (button->max - button->min) / (button->height - button_sprite[button->sprite18].height);
                        }
                    }
                    if (button->max > button->min) {
                        if (pressed) {
                            update_state = 3;
                            *button->value += slider_delta;
                            if (button->callback)
                                button->callback(win);
                            buttonstore = button;
                        } else if (slider_delta > 0) {
                            (*button->value)++;
                        } else if (slider_delta < 0) {
                            (*button->value)--;
                        }
                        if (*button->value < button->min)
                            *button->value = button->min;
                        else if (*button->value > button->max)
                            *button->value = button->max;
                    }
                }
                if (button->callback)
                    button->callback(win);
                button = button->next;
                break;
            } else {
                if (!(button->flags & 0x4000000)) {
                    if (button->flags & 4)
                        button->flags &= ~4;
                }
            }
        } else if (!buttonstore
                   && mouse_over(button->x + win->x, button->y + win->y, button->x + win->x + button->width - 1, button->y + win->y + button->height - 1)) {
            buttonstore = button;
        }
    }
    for (; button; button = button->next) {
        if (!(button->flags & 0x4000000)) {
            if (button->flags & 4)
                button->flags &= ~4;
        }
    }
    if (notover) {
        if ((lbDisplay.left_button || lbDisplay.right_button) && (win->flags & 0x10)) {
            if (MousePressedOnWindow(win))
                update_state = 2;
        }
    }
    return update_state;
}

// FUNCTION: 0x0004d740
void process_buttons(Window **windows, Window *win)
{
    s16 mdy;
    s16 ox;
    s16 oy;
    s16 mdx;
    s32 slider_delta;

    slider_delta = 0;
    mdx = lbDisplay.mmouse_x - lbDisplay.graphics_window_x - old_mousex;
    mdy = lbDisplay.mmouse_y - lbDisplay.graphics_window_y - old_mousey;
    old_mousex = lbDisplay.mmouse_x - lbDisplay.graphics_window_x;
    old_mousey = lbDisplay.mmouse_y - lbDisplay.graphics_window_y;
    if (mdx || mdy)
        tool_tip_count = 0;
    else
        tool_tip_count++;
    switch (update_state) {
    case 1:
        if ((buttonstore->flags & 0x80) && lbDisplay.mleft_button || (buttonstore->flags & 0x100) && lbDisplay.mright_button) {
            if ((mdx > 0 && lbDisplay.mmouse_x - lbDisplay.graphics_window_x <= win->x + buttonstore->x)
                || (mdx < 0 && lbDisplay.mmouse_x - lbDisplay.graphics_window_x >= win->x + buttonstore->x + buttonstore->width)) {
                ox = 0;
            } else if (win->flags & 0x20) {
                ox = mdx * 2;
            } else {
                ox = mdx;
                if (ox < -(win->x + buttonstore->x + buttonstore->width - 3))
                    ox = -(win->x + buttonstore->x + buttonstore->width - 3);
                else if (ox > lbDisplay.graphics_screen_width - 3 - fn_000cb2db(win->x + buttonstore->x))
                    ox = lbDisplay.graphics_screen_width - 3 - fn_000cb2db(win->x + buttonstore->x);
            }
            if ((mdy > 0 && lbDisplay.mmouse_y - lbDisplay.graphics_window_y <= win->y + buttonstore->y)
                || (mdy < 0 && lbDisplay.mmouse_y - lbDisplay.graphics_window_y >= win->y + buttonstore->y + buttonstore->height)) {
                oy = 0;
            } else if (win->flags & 0x40) {
                oy = mdy * 2;
            } else {
                oy = mdy;
                if (oy < -(win->y + buttonstore->y + buttonstore->height - 3))
                    oy = -(win->y + buttonstore->y + buttonstore->height - 3);
                else if (oy > lbDisplay.graphics_screen_height - 3 - fn_000cb2db(win->y + buttonstore->y))
                    oy = lbDisplay.graphics_screen_height - 3 - fn_000cb2db(win->y + buttonstore->y);
            }
            if (ox < -(win->width - win->widthC))
                ox = -(win->width - win->widthC);
            else if (ox > win->width10 - win->width)
                ox = win->width10 - win->width;
            if (oy < -(win->height - win->heightE))
                oy = -(win->height - win->heightE);
            else if (oy > win->height12 - win->height)
                oy = win->height12 - win->height;
            if (ox || oy) {
                win->width += ox;
                win->height += oy;
                reposition_buttons(win, ox, oy);
            }
            if (buttonstore && buttonstore->callback)
                buttonstore->callback(win);
        } else {
            buttonstore->flags &= ~4;
            update_state = 0;
            buttonstore = 0;
        }
        break;
    case 3:
        if (lbDisplay.mleft_button) {
            if (mdx || mdy) {
                if (buttonstore->width > button_sprite[buttonstore->sprite18].width) {
                    ox = buttonstore->x + win->x
                        + (buttonstore->width - button_sprite[buttonstore->sprite18].width)
                            * (*buttonstore->value - buttonstore->min) / (buttonstore->max - buttonstore->min)
                        + button_sprite[buttonstore->sprite18].width / 2;
                    ox = lbDisplay.mmouse_x - lbDisplay.graphics_window_x - ox;
                    slider_delta = ox * (buttonstore->max - buttonstore->min)
                        / (buttonstore->width - button_sprite[buttonstore->sprite18].width);
                }
                if (buttonstore->height > button_sprite[buttonstore->sprite18].height) {
                    oy = buttonstore->y + win->y + button_sprite[buttonstore->sprite18].height / 2
                        + (buttonstore->max - *buttonstore->value)
                            * (buttonstore->height - button_sprite[buttonstore->sprite18].height)
                            / (buttonstore->max - buttonstore->min);
                    oy = -(lbDisplay.mmouse_y - lbDisplay.graphics_window_y - oy);
                    if (slider_delta) {
                        slider_delta += oy * (buttonstore->max - buttonstore->min)
                            / (buttonstore->height - button_sprite[buttonstore->sprite18].height);
                        slider_delta /= 2;
                    } else {
                        slider_delta += oy * (buttonstore->max - buttonstore->min)
                            / (buttonstore->height - button_sprite[buttonstore->sprite18].height);
                    }
                }
                if (buttonstore->max > buttonstore->min) {
                    *buttonstore->value += slider_delta;
                    if (*buttonstore->value < buttonstore->min)
                        *buttonstore->value = buttonstore->min;
                    else if (*buttonstore->value > buttonstore->max)
                        *buttonstore->value = buttonstore->max;
                }
            }
            if (buttonstore && buttonstore->callback)
                buttonstore->callback(win);
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
        process_buttons_normal_state(windows, win, &text_position);
        break;
    }
}

// FUNCTION: 0x0004df88
u8 insert_window(Window **windows, Window *win)
{
    u16 i;

    for (i = 0; i < 2; i++) {
        if (windows[i] == 0) {
            for (; i; i--)
                windows[i] = windows[i - 1];
            windows[i] = win;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0004e024
u8 make_front_window(Window **windows, Window *win)
{
    u16 i;

    for (i = 0; i < 2; i++) {
        if (windows[i] == win) {
            for (; i; i--)
                windows[i] = windows[i - 1];
            windows[i] = win;
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
void GetWindowDimensions(Window *win, u16 *width, u16 *height)
{
    if (win) {
        if (win->flags & 0x100) {
            *width = win->width10;
            *height = win->height12;
        } else if (win->flags & 0x80) {
            *width = win->widthC;
            *height = win->heightE;
        } else {
            *width = win->width;
            *height = win->height;
        }
    } else {
        *width = 1;
        *height = 1;
    }
}

// FUNCTION: 0x0004e240
u8 MouseOnWindow(Window *win)
{
    u16 width;
    u16 height;

    if (win) {
        GetWindowDimensions(win, &width, &height);
        return mouse_over(win->x, win->y, win->x + width - 1, win->y + height - 1);
    }
    return 0;
}

// FUNCTION: 0x0004e2b4
u8 MousePressedOnWindow(Window *win)
{
    u16 width;
    u16 height;

    if (win) {
        GetWindowDimensions(win, &width, &height);
        return mouse_pressed_over(win->x, win->y, win->x + width - 1, win->y + height - 1);
    }
    return 0;
}

// FUNCTION: 0x0004e328
void process_window(Window **windows, Window *win)
{
    process_buttons(windows, win);
    OldLeftButton = lbDisplay.mleft_button;
    OldRightButton = lbDisplay.mright_button;
    if (win->flags & 0x100) {
        if (win->flags & 0x20)
            win->x = (lbDisplay.graphics_screen_width - win->width10) / 2;
        if (win->flags & 0x40)
            win->y = (lbDisplay.graphics_screen_height - win->height12) / 2;
    } else if (win->flags & 0x80) {
        if (win->flags & 0x20)
            win->x = (lbDisplay.graphics_screen_width - win->widthC) / 2;
        if (win->flags & 0x40)
            win->y = (lbDisplay.graphics_screen_height - win->heightE) / 2;
    } else {
        if (win->flags & 0x20)
            win->x = (lbDisplay.graphics_screen_width - win->width) / 2;
        if (win->flags & 0x40)
            win->y = (lbDisplay.graphics_screen_height - win->height) / 2;
    }
}

// FUNCTION: 0x0004e47c
extern "C" void fn_0004e47c(Window **windows)
{
    u8 front;
    s32 i;
    s32 count;
    Window *win;

    win = windows[0];
    front = 0;
    if (gamevar.f12e == 0x13)
        return;
    CurrentWindow = -1;
    if (!win)
        return;
    if (MouseOnWindow(win)) {
        if (!mouse_was_on_window) {
            othervar.f68 = gamevar.f12e;
            set_mouse_mode(MM_NORMAL);
            mouse_was_on_window = 1;
        }
    } else if (mouse_was_on_window) {
        mouse_was_on_window = 0;
        set_mouse_mode(othervar.f68);
    }
    if (win->flags & 0x600) {
        count = CountWindows(windows);
        for (i = count - 1; i > 0; i--) {
            win = windows[i];
            if (MouseOnWindow(win))
                CurrentWindow = i;
            clear_buttons(win);
        }
        win = windows[0];
        if (MouseOnWindow(win))
            CurrentWindow = 0;
        process_window(windows, win);
    } else {
        if (lbDisplay.left_button || lbDisplay.right_button) {
            for (i = 0; win; i++) {
                win = windows[i];
                if (win) {
                    if (MousePressedOnWindow(win)) {
                        CurrentWindow = i;
                        if (win != windows[0]) {
                            make_front_window(windows, win);
                            front = 1;
                        }
                        break;
                    }
                } else {
                    break;
                }
            }
        } else {
            for (i = 0; win; i++) {
                win = windows[i];
                if (win) {
                    if (MouseOnWindow(win)) {
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
            win = windows[i];
            clear_buttons(win);
        }
        if (!front) {
            win = windows[0];
            if (MouseOnWindow(win))
                CurrentWindow = 0;
            process_window(windows, win);
        } else {
            win = windows[0];
            process_window(windows, win);
        }
    }
    if (CurrentWindow > 0)
        calculate_currentbuttonstore(windows[CurrentWindow]);
}

// FUNCTION: 0x0004e6f8
extern "C" s16 fn_0004e6f8(WindowButton *button)
{
    s32 i;

    for (i = 0; i < 500; i++) {
        if (&resvars.f24_WindowButtonp[i] == button)
            break;
    }
    if (i == 500)
        return -1;
    return i;
}
