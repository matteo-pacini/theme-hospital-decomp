#include "bflib.h"

extern "C" {
extern s32 lbMouseInstalled;
// The screen area under the pointer, saved while it is drawn.
struct mouse_buffer {
    // 0x0: Nonzero while buffer holds the area (mouse_setup_range's result).
    s32 valid;
    s32 width;
    s32 height;
    // 0xc: Offset of the area's top left in the back buffer.
    u32 offset;
    u8 buffer[0x1000];
    // 0x1010: Screen position of the pointer sprite's top left.
    s32 x;
    s32 y;
    // 0x1018: First sprite column and row on screen, when clipped at the
    // left or top edge.
    s32 sprite_x;
    s32 sprite_y;
};

struct mouse_info {
    // 0x0: Mickeys per pixel (LbMouseChangeMoveRatio).
    s32 move_ratio_x;
    s32 move_ratio_y;
    // 0x8: Offset from the mouse position to the sprite's top left: minus
    // the hotspot.
    s32 sprite_offset_x;
    s32 sprite_offset_y;
    // 0x10: The pointer sprite drawn at full size, 0xfe where transparent.
    u8 sprite[0x1000];
};

extern mouse_buffer mbuffer;
extern mouse_info minfo;
extern s32 redraw_active;
extern s32 mouse_initialised;
extern volatile s32 mouse_dx;
extern volatile s32 mouse_dy;
extern s32 mouse_delta_pending;
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
        mbuffer.x = lbDisplay.mmouse_x + minfo.sprite_offset_x;
        mbuffer.y = lbDisplay.mmouse_y + minfo.sprite_offset_y;
        mbuffer.valid = mouse_setup_range();
        if (mbuffer.valid == 1) {
            save = mbuffer.buffer;
            sprite = minfo.sprite + mbuffer.sprite_x + mbuffer.sprite_y * lbDisplay.mouse_sprite->width;
            mbuffer.offset =
                mbuffer.x + mbuffer.sprite_x + (mbuffer.y + mbuffer.sprite_y) * lbDisplay.graphics_screen_width;
            screen = lbDisplay.wscreen + mbuffer.offset;
            for (y = 0; y < mbuffer.height; y++) {
                scr = screen;
                mask = sprite;
                for (x = 0; x < mbuffer.width; x++, save++, scr++, mask++) {
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

    if (mbuffer.valid) {
        save = mbuffer.buffer;
        screen = lbDisplay.wscreen + mbuffer.offset;
        for (y = 0; y < mbuffer.height; y++) {
            scr = screen;
            for (x = 0; x < mbuffer.width; x++)
                *scr++ = *save++;
            screen += lbDisplay.graphics_screen_width;
        }
    }
    if (mouse_delta_pending) {
        lbDisplay.mmouse_x += mouse_dx;
        lbDisplay.mmouse_y += mouse_dy;
        adjust_point((long *)&lbDisplay.mmouse_x, (long *)&lbDisplay.mmouse_y);
        if (!mouse_pos_change_saved) {
            screen_remove(0);
            screen_place();
        }
        mouse_delta_pending = 0;
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
    minfo.sprite_offset_x = x;
    minfo.sprite_offset_x = -minfo.sprite_offset_x;
    minfo.sprite_offset_y = y;
    minfo.sprite_offset_y = -minfo.sprite_offset_y;
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
    memset(minfo.sprite, 0xfe, 0x1000);
    if (sprite) {
        screen = lbDisplay.wscreen;
        lbDisplay.wscreen = minfo.sprite;
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
    minfo.move_ratio_x = ratio_x;
    minfo.move_ratio_y = ratio_y;
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
    minfo.move_ratio_x = 1;
    minfo.move_ratio_y = 1;
    minfo.sprite_offset_x = 0;
    minfo.sprite_offset_y = 0;
    memset(minfo.sprite, 0xfe, 0x1000);
    lbDisplay.mouse_sprite = 0;
    redraw_active = 0;
    memset(&mbuffer.valid, 0, 0x1020);
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
    memset(&mbuffer.valid, 0, 0x1020);
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
    memset(&mbuffer.valid, 0, 0x1020);
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
    if (mouse_delta_pending) {
        lbDisplay.mmouse_x += mouse_dx;
        lbDisplay.mmouse_y += mouse_dy;
        adjust_point((long *)&lbDisplay.mmouse_x, (long *)&lbDisplay.mmouse_y);
        mouse_delta_pending = 0;
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
            old_rx = dx % minfo.move_ratio_x;
            mouse_dx = dx / minfo.move_ratio_x;
        } else {
            mouse_dx = dx / minfo.move_ratio_x;
            old_rx += dx % minfo.move_ratio_x;
            if (old_rx < 0) {
                mouse_dx--;
                old_rx = minfo.move_ratio_x + old_rx % minfo.move_ratio_x;
            }
        }
        mouse_delta_pending = 1;
    }
    dy = mouse_mickey_y - old_my;
    if (dy != 0) {
        if (dy > 0) {
            dy += old_ry;
            old_ry = dy % minfo.move_ratio_y;
            mouse_dy = dy / minfo.move_ratio_y;
        } else {
            mouse_dy = dy / minfo.move_ratio_y;
            old_ry += dy % minfo.move_ratio_y;
            if (old_ry < 0) {
                mouse_dy--;
                old_ry = minfo.move_ratio_y + old_ry % minfo.move_ratio_y;
            }
        }
        mouse_delta_pending = 1;
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
        mouse_delta_pending = 0;
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
    mbuffer.width = lbDisplay.mouse_sprite->width;
    mbuffer.height = lbDisplay.mouse_sprite->height;
    mbuffer.sprite_x = 0;
    mbuffer.sprite_y = 0;
    if (!(-mbuffer.width < mbuffer.x && mbuffer.x < lbDisplay.graphics_screen_width))
        return 0;
    if (!(-mbuffer.height < mbuffer.y && mbuffer.y < lbDisplay.graphics_screen_height))
        return 0;
    if (mbuffer.x < 0) {
        mbuffer.sprite_x = mbuffer.x;
        mbuffer.sprite_x = -mbuffer.sprite_x;
        mbuffer.width -= mbuffer.sprite_x;
    }
    if (mbuffer.x + mbuffer.width > lbDisplay.graphics_screen_width)
        mbuffer.width = lbDisplay.graphics_screen_width - mbuffer.x;
    if (mbuffer.y < 0) {
        mbuffer.sprite_y = mbuffer.y;
        mbuffer.sprite_y = -mbuffer.sprite_y;
        mbuffer.height -= mbuffer.sprite_y;
    }
    if (mbuffer.y + mbuffer.height > lbDisplay.graphics_screen_height)
        mbuffer.height = lbDisplay.graphics_screen_height - mbuffer.y;
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
        mbuffer.x = lbDisplay.mmouse_x + minfo.sprite_offset_x;
        mbuffer.y = lbDisplay.mmouse_y + minfo.sprite_offset_y;
        mbuffer.valid = mouse_setup_range();
        if (mbuffer.valid == 1) {
            save = mbuffer.buffer;
            sprite = minfo.sprite + mbuffer.sprite_x + lbDisplay.mouse_sprite->width * mbuffer.sprite_y;
            mbuffer.offset =
                mbuffer.x + mbuffer.sprite_x + (mbuffer.y + mbuffer.sprite_y) * lbDisplay.physical_screen_width;
            if (!lbDisplay.vesa_is_set_up) {
                screen = lbDisplay.physical_screen + mbuffer.offset;
                for (rows_left = mbuffer.height; --rows_left != -1;
                     screen += lbDisplay.physical_screen_width, sprite += lbDisplay.mouse_sprite->width) {
                    scr = screen;
                    mask = sprite;
                    for (x = 0; x < mbuffer.width; x++, scr++, mask++) {
                        *save++ = *scr;
                        if (*mask != 0xfe)
                            *scr = *mask;
                    }
                }
            } else {
                page = mbuffer.offset >> 16;
                LbVesaSetPage(page);
                screen = lbDisplay.physical_screen + (mbuffer.offset & 0xffff);
                for (rows_left = mbuffer.height; --rows_left != -1;
                     screen += lbDisplay.physical_screen_width, sprite += lbDisplay.mouse_sprite->width) {
                    scr = screen;
                    mask = sprite;
                    after_bank = mbuffer.width;
                    if ((u32)(scr + mbuffer.width) >= 0xb0000) {
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
    if ((lbDisplay.mmouse_x != mbuffer.x || lbDisplay.mmouse_y != mbuffer.y || force) && mbuffer.valid) {
        save = mbuffer.buffer;
        if (!lbDisplay.vesa_is_set_up) {
            screen = lbDisplay.physical_screen + mbuffer.offset;
            for (rows_left = mbuffer.height; --rows_left != -1; screen += lbDisplay.physical_screen_width) {
                scr = screen;
                for (x = 0; x < mbuffer.width; x++)
                    *scr++ = *save++;
            }
        } else {
            page = mbuffer.offset >> 16;
            LbVesaSetPage(page);
            screen = lbDisplay.physical_screen + (mbuffer.offset & 0xffff);
            for (rows_left = mbuffer.height; --rows_left != -1; screen += lbDisplay.physical_screen_width) {
                scr = screen;
                after_bank = mbuffer.width;
                if ((u32)(scr + mbuffer.width) >= 0xb0000) {
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
        mbuffer.valid = 0;
    }
    return 1;
}
