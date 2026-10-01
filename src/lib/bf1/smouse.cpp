#include "bflib.h"

extern "C" {
extern s32 lbMouseInstalled;
extern s32 D_00176bc4;
extern s32 D_00176bc8;
extern s32 D_00176bcc;
extern u32 D_00176bd0;
extern u8 D_00176bd4[];
extern s32 D_00177bd4;
extern s32 D_00177bd8;
extern s32 D_00177bdc;
extern s32 D_00177be0;
extern s32 D_00177be4;
extern s32 D_00177be8;
extern s32 D_00177bec;
extern s32 D_00177bf0;
extern u8 D_00177bf4[];
extern s32 redraw_active;
extern s32 mouse_initialised;
extern volatile s32 mouse_dx;
extern volatile s32 mouse_dy;
extern s32 D_00178c08;
extern s16 old_mx;
extern s16 old_my;
extern s32 old_rx;
extern s32 old_ry;
extern volatile s16 mouse_mickey_x;
extern volatile s16 mouse_mickey_y;
extern s16 lbVesaPage;

int LbMouseSetWindow(s32 x, s32 y, s32 width, s32 height);
int LbMouseSetPointerHotspot(s32 x, s32 y);
void LbVesaSetPage(s16 page);
u16 FP_SEG(const volatile void __far *p);
#pragma aux FP_SEG = parm caller [eax dx] value [dx];
}

extern u32 mouse_pos_change_saved;

u32 adjust_point(long *x, long *y);
u32 mouse_setup_range(void);
long screen_place(void);
long screen_remove(u32 force);
long LbMouseChangeMoveRatio(u8 ratio_x, u8 ratio_y);
long LbMouseUpdatePosition(void);
void __interrupt __far MouseHandler256(int events, int raw_x, int raw_y);
#pragma aux MouseHandler256 parm [eax] [esi] [edi]

extern "C" {

// Draws the mouse pointer into the back buffer at the mouse position,
// saving what it covers for LbMouseRemove, and holds off the interrupt
// handler's own drawing. Returns 1.
// FUNCTION: 0x000bcfc0
int LbMousePlace(void)
{
    s32 y;
    s32 x;
    u8 *mask;
    u8 *scr;
    u8 *save;
    u8 *sprite;
    u8 *screen;

    redraw_active = 1;
    if (lbDisplay.mouse_sprite) {
        D_00177bd4 = lbDisplay.mmouse_x + D_00177bec;
        D_00177bd8 = lbDisplay.mmouse_y + D_00177bf0;
        D_00176bc4 = mouse_setup_range();
        if (D_00176bc4 == 1) {
            save = D_00176bd4;
            sprite = D_00177bf4 + D_00177bdc + D_00177be0 * lbDisplay.mouse_sprite->width;
            D_00176bd0 = D_00177bd4 + D_00177bdc + (D_00177bd8 + D_00177be0) * lbDisplay.graphics_screen_width;
            screen = lbDisplay.wscreen + D_00176bd0;
            for (y = 0; y < D_00176bcc; y++) {
                scr = screen;
                mask = sprite;
                for (x = 0; x < D_00176bc8; x++, save++, scr++, mask++) {
                    *save = *scr;
                    if (*mask != 0xfe)
                        *scr = *mask;
                }
                screen += lbDisplay.graphics_screen_width;
                sprite += lbDisplay.mouse_sprite->width;
            }
        }
    }
    return 1;
}

// Restores what LbMousePlace covered, applies any movement made meanwhile
// and lets the interrupt handler draw the pointer again. Returns 1.
// FUNCTION: 0x000bd118
int LbMouseRemove(void)
{
    s32 y;
    s32 x;
    u8 *save;
    u8 *scr;
    u8 *screen;

    if (D_00176bc4) {
        save = D_00176bd4;
        screen = lbDisplay.wscreen + D_00176bd0;
        for (y = 0; y < D_00176bcc; y++) {
            scr = screen;
            for (x = 0; x < D_00176bc8; x++)
                *scr++ = *save++;
            screen += lbDisplay.graphics_screen_width;
        }
    }
    if (D_00178c08) {
        lbDisplay.mmouse_x += mouse_dx;
        lbDisplay.mmouse_y += mouse_dy;
        adjust_point((long *)&lbDisplay.mmouse_x, (long *)&lbDisplay.mmouse_y);
        if (!mouse_pos_change_saved) {
            screen_remove(0);
            screen_place();
        }
        D_00178c08 = 0;
    }
    redraw_active = 0;
    return 1;
}

// Sets the pointer's hotspot to (x, y) within its sprite. Returns 1, or -1
// if the mouse is not set up.
// FUNCTION: 0x000bd1f4
int LbMouseSetPointerHotspot(s32 x, s32 y)
{
    if (!lbMouseInstalled)
        return -1;
    redraw_active = 1;
    if (!mouse_pos_change_saved)
        screen_remove(1);
    D_00177bec = x;
    D_00177bec = -D_00177bec;
    D_00177bf0 = y;
    D_00177bf0 = -D_00177bf0;
    if (!mouse_pos_change_saved)
        screen_place();
    redraw_active = 0;
    return 1;
}

// Makes sprite (at most 64 x 64; 0 for none) the mouse pointer. Returns 1,
// or -1 if the mouse is not set up or the sprite is too big.
// FUNCTION: 0x000bd278
int LbMouseChangeSprite(TbSprite *sprite)
{
    s32 win_x;
    s32 win_y;
    s32 win_width;
    s32 win_height;
    s32 screen_width;
    u8 *screen;
    u16 flags;

    if (!lbMouseInstalled)
        return -1;
    if (lbDisplay.mouse_sprite == sprite)
        return 1;
    if (sprite && (sprite->width > 64 || sprite->height > 64))
        return -1;
    redraw_active = 1;
    if (!mouse_pos_change_saved)
        screen_remove(1);
    lbDisplay.mouse_sprite = sprite;
    memset(D_00177bf4, 0xfe, 0x1000);
    if (sprite) {
        screen = lbDisplay.wscreen;
        lbDisplay.wscreen = D_00177bf4;
        win_x = lbDisplay.graphics_window_x;
        win_y = lbDisplay.graphics_window_y;
        win_width = lbDisplay.graphics_window_width;
        win_height = lbDisplay.graphics_window_height;
        screen_width = lbDisplay.graphics_screen_width;
        flags = lbDisplay.draw_flags;
        lbDisplay.graphics_screen_width = sprite->width;
        lbDisplay.draw_flags = 0;
        LbScreenSetGraphicsWindow(0, 0, sprite->width, sprite->height);
        LbSpriteDraw(0, 0, sprite);
        lbDisplay.wscreen = screen;
        lbDisplay.graphics_screen_width = screen_width;
        lbDisplay.draw_flags = flags;
        LbScreenSetGraphicsWindow(win_x, win_y, win_width, win_height);
    }
    if (!mouse_pos_change_saved)
        screen_place();
    redraw_active = 0;
    return 1;
}

// Changes the pointer sprite and sets its hotspot to (x, y). Returns 1, or
// -1 on failure.
// FUNCTION: 0x000bd400
int LbMouseChangeSpriteAndHotspot(TbSprite *sprite, s32 x, s32 y)
{
    if (LbMouseChangeSprite(sprite) == 1) {
        LbMouseSetPointerHotspot(x, y);
        return 1;
    }
    return -1;
}

}

// Sets the mickeys per pixel in x and y (1-63). Returns 1, or -1 if out of
// range or the mouse is not set up.
// FUNCTION: 0x000bd43c
long LbMouseChangeMoveRatio(u8 ratio_x, u8 ratio_y)
{
    if (!lbMouseInstalled)
        return -1;
    if (ratio_x < 1 || ratio_x > 63)
        return -1;
    if (ratio_y < 1 || ratio_y > 63)
        return -1;
    D_00177be4 = ratio_x;
    D_00177be8 = ratio_y;
    return 1;
}

extern "C" {

// Returns 1 if the mouse is set up, else -1.
// FUNCTION: 0x000bd4b4
int LbMouseIsInstalled(void)
{
    if (lbMouseInstalled)
        return 1;
    return -1;
}

// Sets up the mouse driver (INT 33h) with pointer sprite, the whole screen
// as its window, centred, and installs the event handler. Returns 1, or -1
// if already set up or there is no mouse.
// FUNCTION: 0x000bd4d0
int LbMouseSetup(TbSprite *sprite)
{
    SREGS sregs;
    REGS out;
    REGS in;
    u8 ratio_x;
    u8 ratio_y;

    ratio_x = 1;
    ratio_y = 1;
    if (lbMouseInstalled)
        return -1;
    if (!mouse_initialised) {
        segread(&sregs);
        in.w.ax = 0;
        int386(0x33, &in, &out);
        if (out.w.ax != 0xffff)
            return -1;
    }
    lbMouseInstalled = 1;
    D_00177be4 = 1;
    D_00177be8 = 1;
    D_00177bec = 0;
    D_00177bf0 = 0;
    memset(D_00177bf4, 0xfe, 0x1000);
    lbDisplay.mouse_sprite = 0;
    redraw_active = 0;
    memset(&D_00176bc4, 0, 0x1020);
    if (LbMouseSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height) != 1) {
        lbMouseInstalled = 0;
        return -1;
    }
    if (LbMouseChangeMoveRatio(ratio_x, ratio_y) != 1) {
        lbMouseInstalled = 0;
        return -1;
    }
    if (LbMouseSetPosition(lbDisplay.graphics_screen_width >> 1, lbDisplay.graphics_screen_height >> 1) != 1) {
        lbMouseInstalled = 0;
        return -1;
    }
    if (LbMouseChangeSprite(sprite) != 1) {
        lbMouseInstalled = 0;
        return -1;
    }
    if (!mouse_initialised) {
        in.w.ax = 0xc;
        in.w.cx = 0x7f;
        in.x.edx = (u32)MouseHandler256;
        sregs.es = FP_SEG(MouseHandler256);
        int386x(0x33, &in, &out, &sregs);
        mouse_initialised = 1;
    }
    return 1;
}

// Removes the pointer and resets the mouse driver. Returns 1, or -1 if the
// mouse is not set up.
// FUNCTION: 0x000bd688
int LbMouseReset(void)
{
    REGS in;
    REGS out;

    if (!lbMouseInstalled)
        return -1;
    redraw_active = 1;
    if (!mouse_pos_change_saved)
        screen_remove(1);
    memset(&D_00176bc4, 0, 0x1020);
    in.w.ax = 0;
    int386(0x33, &in, &out);
    mouse_initialised = 0;
    lbDisplay.mouse_sprite = 0;
    lbMouseInstalled = 0;
    redraw_active = 0;
    return 1;
}

// Removes the pointer and marks the mouse as not set up, without resetting
// the driver. Returns 1, or -1 if it was not set up.
// FUNCTION: 0x000bd70c
int LbMouseSuspend(void)
{
    if (!lbMouseInstalled)
        return -1;
    redraw_active = 1;
    if (!mouse_pos_change_saved)
        screen_remove(1);
    memset(&D_00176bc4, 0, 0x1020);
    lbDisplay.mouse_sprite = 0;
    lbMouseInstalled = 0;
    redraw_active = 0;
    return 1;
}

// Confines the mouse to the box at (x, y), width x height. Returns 1, or -1
// if the mouse is not set up.
// FUNCTION: 0x000bd770
int LbMouseSetWindow(s32 x, s32 y, s32 width, s32 height)
{
    if (!lbMouseInstalled)
        return -1;
    lbDisplay.mouse_window_x = x;
    lbDisplay.mouse_window_y = y;
    lbDisplay.mouse_window_width = width;
    lbDisplay.mouse_window_height = height;
    adjust_point((long *)&lbDisplay.mmouse_x, (long *)&lbDisplay.mmouse_y);
    adjust_point((long *)&lbDisplay.mouse_x, (long *)&lbDisplay.mouse_y);
    return 1;
}

// Moves the mouse to (x, y), within its window. Returns 1, or -1 if the
// mouse is not set up.
// FUNCTION: 0x000bd7dc
int LbMouseSetPosition(s32 x, s32 y)
{
    if (!lbMouseInstalled)
        return -1;
    redraw_active = 1;
    if (!mouse_pos_change_saved)
        screen_remove(1);
    adjust_point(&x, &y);
    lbDisplay.mouse_x = lbDisplay.mmouse_x = x;
    lbDisplay.mouse_y = lbDisplay.mmouse_y = y;
    if (!mouse_pos_change_saved)
        screen_place();
    redraw_active = 0;
    return 1;
}

}

// Applies movement the interrupt handler has stored but not yet applied.
// Returns 1.
// FUNCTION: 0x000bd868
long LbMouseUpdatePosition(void)
{
    if (D_00178c08) {
        lbDisplay.mmouse_x += mouse_dx;
        lbDisplay.mmouse_y += mouse_dy;
        adjust_point((long *)&lbDisplay.mmouse_x, (long *)&lbDisplay.mmouse_y);
        D_00178c08 = 0;
    }
    return 1;
}

// Mouse driver event handler: records button presses and releases (with
// their positions) in lbDisplay, turns movement into pixels by the move
// ratio, and redraws the pointer on screen unless LbMousePlace is active.
// FUNCTION: 0x000bd8ac
void __interrupt __far MouseHandler256(int events, int raw_x, int raw_y)
{
    s32 dx;
    s32 dy;
    u32 force;
    s16 x;
    s16 page;
    s16 y;

    if (!lbMouseInstalled)
        return;
    if (events != 1) {
        if (events & 2) {
            lbDisplay.mleft_button = 1;
            if (!lbDisplay.left_button) {
                lbDisplay.left_button = 1;
                lbDisplay.mouse_x = lbDisplay.mmouse_x;
                lbDisplay.mouse_y = lbDisplay.mmouse_y;
                lbDisplay.rleft_button = 0;
            }
        }
        if (events & 4) {
            lbDisplay.mleft_button = 0;
            if (!lbDisplay.rleft_button) {
                lbDisplay.rleft_button = 1;
                lbDisplay.rmouse_x = lbDisplay.mmouse_x;
                lbDisplay.rmouse_y = lbDisplay.mmouse_y;
            }
        }
        if (events & 8) {
            lbDisplay.mright_button = 1;
            if (!lbDisplay.right_button) {
                lbDisplay.right_button = 1;
                lbDisplay.mouse_x = lbDisplay.mmouse_x;
                lbDisplay.mouse_y = lbDisplay.mmouse_y;
                lbDisplay.rright_button = 0;
            }
        }
        if (events & 0x10) {
            lbDisplay.mright_button = 0;
            if (!lbDisplay.rright_button) {
                lbDisplay.rright_button = 1;
                lbDisplay.rmouse_x = lbDisplay.mmouse_x;
                lbDisplay.rmouse_y = lbDisplay.mmouse_y;
            }
        }
        if (events & 0x20) {
            lbDisplay.mmiddle_button = 1;
            if (!lbDisplay.middle_button) {
                lbDisplay.middle_button = 1;
                lbDisplay.mouse_x = lbDisplay.mmouse_x;
                lbDisplay.mouse_y = lbDisplay.mmouse_y;
                lbDisplay.rmiddle_button = 0;
            }
        }
        if (events & 0x40) {
            lbDisplay.mmiddle_button = 0;
            if (!lbDisplay.rmiddle_button) {
                lbDisplay.rmiddle_button = 1;
                lbDisplay.rmouse_x = lbDisplay.mmouse_x;
                lbDisplay.rmouse_y = lbDisplay.mmouse_y;
            }
        }
    }
    if (!(events & 1))
        return;
    x = raw_x;
    y = raw_y;
    old_mx = mouse_mickey_x;
    old_my = mouse_mickey_y;
    mouse_mickey_x = x;
    mouse_mickey_y = y;
    mouse_dx = mouse_dy = 0;
    dx = mouse_mickey_x - old_mx;
    if (dx != 0) {
        if (dx > 0) {
            dx += old_rx;
            old_rx = dx % D_00177be4;
            mouse_dx = dx / D_00177be4;
        } else {
            mouse_dx = dx / D_00177be4;
            old_rx += dx % D_00177be4;
            if (old_rx < 0) {
                mouse_dx--;
                old_rx = D_00177be4 + old_rx % D_00177be4;
            }
        }
        D_00178c08 = 1;
    }
    dy = mouse_mickey_y - old_my;
    if (dy != 0) {
        if (dy > 0) {
            dy += old_ry;
            old_ry = dy % D_00177be8;
            mouse_dy = dy / D_00177be8;
        } else {
            mouse_dy = dy / D_00177be8;
            old_ry += dy % D_00177be8;
            if (old_ry < 0) {
                mouse_dy--;
                old_ry = D_00177be8 + old_ry % D_00177be8;
            }
        }
        D_00178c08 = 1;
    }
    if (!redraw_active && !mouse_pos_change_saved) {
        lbDisplay.mmouse_x += mouse_dx;
        lbDisplay.mmouse_y += mouse_dy;
        force = adjust_point((long *)&lbDisplay.mmouse_x, (long *)&lbDisplay.mmouse_y);
        page = lbVesaPage;
        screen_remove(force);
        screen_place();
        if (lbDisplay.vesa_is_set_up)
            LbVesaSetPage(page);
        D_00178c08 = 0;
    }
}

// Clamps (*x, *y) into the mouse window. Returns 1 if it moved, else 0.
// FUNCTION: 0x000bdc58
u32 adjust_point(long *x, long *y)
{
    long vx;
    long vy;
    u32 changed;

    changed = 0;
    vx = *x;
    if (vx < lbDisplay.mouse_window_x) {
        *x = lbDisplay.mouse_window_x;
        changed = 1;
    } else if (lbDisplay.mouse_window_x + lbDisplay.mouse_window_width <= vx) {
        *x = lbDisplay.mouse_window_x + lbDisplay.mouse_window_width - 1;
        changed = 1;
    }
    vy = *y;
    if (vy < lbDisplay.mouse_window_y) {
        *y = lbDisplay.mouse_window_y;
        changed = 1;
    } else if (lbDisplay.mouse_window_y + lbDisplay.mouse_window_height <= vy) {
        *y = lbDisplay.mouse_window_y + lbDisplay.mouse_window_height - 1;
        changed = 1;
    }
    return changed;
}

// Works out the part of the pointer sprite that lies on screen at its
// position. Returns 0 if none of it does, else 1.
// FUNCTION: 0x000bdcfc
u32 mouse_setup_range(void)
{
    D_00176bc8 = lbDisplay.mouse_sprite->width;
    D_00176bcc = lbDisplay.mouse_sprite->height;
    D_00177bdc = 0;
    D_00177be0 = 0;
    if (!(-D_00176bc8 < D_00177bd4 && D_00177bd4 < lbDisplay.graphics_screen_width))
        return 0;
    if (!(-D_00176bcc < D_00177bd8 && D_00177bd8 < lbDisplay.graphics_screen_height))
        return 0;
    if (D_00177bd4 < 0) {
        D_00177bdc = D_00177bd4;
        D_00177bdc = -D_00177bdc;
        D_00176bc8 -= D_00177bdc;
    }
    if (D_00177bd4 + D_00176bc8 > lbDisplay.graphics_screen_width)
        D_00176bc8 = lbDisplay.graphics_screen_width - D_00177bd4;
    if (D_00177bd8 < 0) {
        D_00177be0 = D_00177bd8;
        D_00177be0 = -D_00177be0;
        D_00176bcc -= D_00177be0;
    }
    if (D_00177bd8 + D_00176bcc > lbDisplay.graphics_screen_height)
        D_00176bcc = lbDisplay.graphics_screen_height - D_00177bd8;
    return 1;
}

// Draws the pointer directly into video memory (banked under VESA), saving
// what it covers. Returns 1, or -1 if the mouse is not set up.
// FUNCTION: 0x000bde14
long screen_place(void)
{
    s32 rows_left;
    s32 after_bank;
    s32 before_bank;
    u8 *mask;
    u8 *scr;
    u8 *save;
    u8 *sprite;
    u8 *screen;
    s32 x;
    s32 i;
    s32 j;
    s16 page;

    if (!lbMouseInstalled)
        return -1;
    if (lbDisplay.mouse_sprite) {
        D_00177bd4 = lbDisplay.mmouse_x + D_00177bec;
        D_00177bd8 = lbDisplay.mmouse_y + D_00177bf0;
        D_00176bc4 = mouse_setup_range();
        if (D_00176bc4 == 1) {
            save = D_00176bd4;
            sprite = D_00177bf4 + D_00177bdc + lbDisplay.mouse_sprite->width * D_00177be0;
            D_00176bd0 = D_00177bd4 + D_00177bdc + (D_00177bd8 + D_00177be0) * lbDisplay.physical_screen_width;
            if (!lbDisplay.vesa_is_set_up) {
                screen = lbDisplay.physical_screen + D_00176bd0;
                for (rows_left = D_00176bcc; --rows_left != -1; screen += lbDisplay.physical_screen_width, sprite += lbDisplay.mouse_sprite->width) {
                    scr = screen;
                    mask = sprite;
                    for (x = 0; x < D_00176bc8; x++, scr++, mask++) {
                        *save++ = *scr;
                        if (*mask != 0xfe)
                            *scr = *mask;
                    }
                }
            } else {
                page = D_00176bd0 >> 16;
                LbVesaSetPage(page);
                screen = lbDisplay.physical_screen + (D_00176bd0 & 0xffff);
                for (rows_left = D_00176bcc; --rows_left != -1; screen += lbDisplay.physical_screen_width, sprite += lbDisplay.mouse_sprite->width) {
                    scr = screen;
                    mask = sprite;
                    after_bank = D_00176bc8;
                    if ((u32)(scr + D_00176bc8) >= 0xb0000) {
                        before_bank = 0xb0000 - (u32)scr;
                        if (before_bank > 0) {
                            for (i = 0; i < before_bank; i++, scr++, mask++) {
                                *save++ = *scr;
                                if (*mask != 0xfe)
                                    *scr = *mask;
                            }
                            after_bank -= before_bank;
                        }
                        ++page;
                        LbVesaSetPage(page);
                        scr -= 0x10000;
                        screen -= 0x10000;
                    }
                    for (j = 0; j < after_bank; j++, scr++, mask++) {
                        *save++ = *scr;
                        if (*mask != 0xfe)
                            *scr = *mask;
                    }
                }
            }
        }
    }
    return 1;
}

// Restores the video memory under the pointer if it has moved or force is
// set. Returns 1, or -1 if the mouse is not set up.
// FUNCTION: 0x000be0cc
long screen_remove(u32 force)
{
    s32 rows_left;
    s32 after_bank;
    s32 before_bank;
    u8 *save;
    u8 *scr;
    u8 *screen;
    s32 x;
    s32 i;
    s32 j;
    s16 page;

    if (!lbMouseInstalled)
        return -1;
    if ((lbDisplay.mmouse_x != D_00177bd4 || lbDisplay.mmouse_y != D_00177bd8 || force) && D_00176bc4) {
        save = D_00176bd4;
        if (!lbDisplay.vesa_is_set_up) {
            screen = lbDisplay.physical_screen + D_00176bd0;
            for (rows_left = D_00176bcc; --rows_left != -1; screen += lbDisplay.physical_screen_width) {
                scr = screen;
                for (x = 0; x < D_00176bc8; x++)
                    *scr++ = *save++;
            }
        } else {
            page = D_00176bd0 >> 16;
            LbVesaSetPage(page);
            screen = lbDisplay.physical_screen + (D_00176bd0 & 0xffff);
            for (rows_left = D_00176bcc; --rows_left != -1; screen += lbDisplay.physical_screen_width) {
                scr = screen;
                after_bank = D_00176bc8;
                if ((u32)(scr + D_00176bc8) >= 0xb0000) {
                    before_bank = 0xb0000 - (u32)scr;
                    if (before_bank > 0) {
                        for (i = 0; i < before_bank; i++)
                            *scr++ = *save++;
                        after_bank -= before_bank;
                    }
                    ++page;
                    LbVesaSetPage(page);
                    scr -= 0x10000;
                    screen -= 0x10000;
                }
                for (j = 0; j < after_bank; j++)
                    *scr++ = *save++;
            }
        }
        D_00176bc4 = 0;
    }
    return 1;
}
