#include "bflib.h"

extern "C" {
extern s32 D_00176bc0;
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
extern s32 D_00178bf4;
extern s32 D_00178bf8;
extern s32 D_00178c00;
extern s32 D_00178c04;
extern s32 D_00178c08;

int LbMouseSetWindow(s32 x, s32 y, s32 width, s32 height);
int LbMouseSetPointerHotspot(s32 x, s32 y);
void LbVesaSetPage(s16 page);
void fn_000bd8ac(void);
u16 FP_SEG(const volatile void __far *p);
#pragma aux FP_SEG = parm caller [eax dx] value [dx];
}

extern u32 mouse_pos_change_saved;

u32 adjust_point(long *x, long *y);
u32 mouse_setup_range(void);
long screen_place(void);
long screen_remove(u32 force);
long LbMouseChangeMoveRatio(u8 ratio_x, u8 ratio_y);

extern "C" {

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

    D_00178bf4 = 1;
    if (lbDisplay.mouse_sprite) {
        D_00177bd4 = lbDisplay.mmouse_x + D_00177bec;
        D_00177bd8 = lbDisplay.mmouse_y + D_00177bf0;
        D_00176bc4 = mouse_setup_range();
        if (D_00176bc4 == 1) {
            save = D_00176bd4;
            sprite = D_00177bf4 + D_00177bdc + D_00177be0 * lbDisplay.mouse_sprite->width;
            D_00176bd0 = D_00177bd4 + D_00177bdc + (D_00177bd8 + D_00177be0) * lbDisplay.graphics_screen_width;
            screen = lbDisplay.unknown4 + D_00176bd0;
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
        screen = lbDisplay.unknown4 + D_00176bd0;
        for (y = 0; y < D_00176bcc; y++) {
            scr = screen;
            for (x = 0; x < D_00176bc8; x++)
                *scr++ = *save++;
            screen += lbDisplay.graphics_screen_width;
        }
    }
    if (D_00178c08) {
        lbDisplay.mmouse_x += D_00178c00;
        lbDisplay.mmouse_y += D_00178c04;
        adjust_point((long *)&lbDisplay.mmouse_x, (long *)&lbDisplay.mmouse_y);
        if (!mouse_pos_change_saved) {
            screen_remove(0);
            screen_place();
        }
        D_00178c08 = 0;
    }
    D_00178bf4 = 0;
    return 1;
}

// FUNCTION: 0x000bd1f4
int LbMouseSetPointerHotspot(s32 x, s32 y)
{
    if (!D_00176bc0)
        return -1;
    D_00178bf4 = 1;
    if (!mouse_pos_change_saved)
        screen_remove(1);
    D_00177bec = x;
    D_00177bec = -D_00177bec;
    D_00177bf0 = y;
    D_00177bf0 = -D_00177bf0;
    if (!mouse_pos_change_saved)
        screen_place();
    D_00178bf4 = 0;
    return 1;
}

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

    if (!D_00176bc0)
        return -1;
    if (lbDisplay.mouse_sprite == sprite)
        return 1;
    if (sprite && (sprite->width > 64 || sprite->height > 64))
        return -1;
    D_00178bf4 = 1;
    if (!mouse_pos_change_saved)
        screen_remove(1);
    lbDisplay.mouse_sprite = sprite;
    memset(D_00177bf4, 0xfe, 0x1000);
    if (sprite) {
        screen = lbDisplay.unknown4;
        lbDisplay.unknown4 = D_00177bf4;
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
        lbDisplay.unknown4 = screen;
        lbDisplay.graphics_screen_width = screen_width;
        lbDisplay.draw_flags = flags;
        LbScreenSetGraphicsWindow(win_x, win_y, win_width, win_height);
    }
    if (!mouse_pos_change_saved)
        screen_place();
    D_00178bf4 = 0;
    return 1;
}

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

// NONMATCHING: target range also covers the next function (0xbd4b4)
// FUNCTION: 0x000bd43c
long LbMouseChangeMoveRatio(u8 ratio_x, u8 ratio_y)
{
    if (!D_00176bc0)
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

// NONMATCHING: the interrupt handler is referenced as LbMouseSetPosition_+0xd0 in the target
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
    if (D_00176bc0)
        return -1;
    if (!D_00178bf8) {
        segread(&sregs);
        in.w.ax = 0;
        int386(0x33, &in, &out);
        if (out.w.ax != 0xffff)
            return -1;
    }
    D_00176bc0 = 1;
    D_00177be4 = 1;
    D_00177bec = 0;
    D_00177bf0 = 0;
    D_00177be8 = 1;
    memset(D_00177bf4, 0xfe, 0x1000);
    lbDisplay.mouse_sprite = 0;
    D_00178bf4 = 0;
    memset(&D_00176bc4, 0, 0x1020);
    if (LbMouseSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height) != 1) {
        D_00176bc0 = 0;
        return -1;
    }
    if (LbMouseChangeMoveRatio(ratio_x, ratio_y) != 1) {
        D_00176bc0 = 0;
        return -1;
    }
    if (LbMouseSetPosition(lbDisplay.graphics_screen_width >> 1, lbDisplay.graphics_screen_height >> 1) != 1) {
        D_00176bc0 = 0;
        return -1;
    }
    if (LbMouseChangeSprite(sprite) != 1) {
        D_00176bc0 = 0;
        return -1;
    }
    if (!D_00178bf8) {
        in.w.ax = 0xc;
        in.w.cx = 0x7f;
        in.x.edx = (u32)fn_000bd8ac;
        sregs.es = FP_SEG(fn_000bd8ac);
        int386x(0x33, &in, &out, &sregs);
        D_00178bf8 = 1;
    }
    return 1;
}

// FUNCTION: 0x000bd688
int LbMouseReset(void)
{
    REGS in;
    REGS out;

    if (!D_00176bc0)
        return -1;
    D_00178bf4 = 1;
    if (!mouse_pos_change_saved)
        screen_remove(1);
    memset(&D_00176bc4, 0, 0x1020);
    in.w.ax = 0;
    int386(0x33, &in, &out);
    D_00178bf8 = 0;
    lbDisplay.mouse_sprite = 0;
    D_00176bc0 = 0;
    D_00178bf4 = 0;
    return 1;
}

// FUNCTION: 0x000bd70c
int LbMouseSuspend(void)
{
    if (!D_00176bc0)
        return -1;
    D_00178bf4 = 1;
    if (!mouse_pos_change_saved)
        screen_remove(1);
    memset(&D_00176bc4, 0, 0x1020);
    lbDisplay.mouse_sprite = 0;
    D_00176bc0 = 0;
    D_00178bf4 = 0;
    return 1;
}

// FUNCTION: 0x000bd770
int LbMouseSetWindow(s32 x, s32 y, s32 width, s32 height)
{
    if (!D_00176bc0)
        return -1;
    lbDisplay.mouse_window_x = x;
    lbDisplay.mouse_window_y = y;
    lbDisplay.mouse_window_width = width;
    lbDisplay.mouse_window_height = height;
    adjust_point((long *)&lbDisplay.mmouse_x, (long *)&lbDisplay.mmouse_y);
    adjust_point((long *)&lbDisplay.mouse_x, (long *)&lbDisplay.mouse_y);
    return 1;
}

// NONMATCHING: target range also covers the next two functions (0xbd868, interrupt handler 0xbd8ac)
// FUNCTION: 0x000bd7dc
int LbMouseSetPosition(s32 x, s32 y)
{
    if (!D_00176bc0)
        return -1;
    D_00178bf4 = 1;
    if (!mouse_pos_change_saved)
        screen_remove(1);
    adjust_point(&x, &y);
    lbDisplay.mouse_x = lbDisplay.mmouse_x = x;
    lbDisplay.mouse_y = lbDisplay.mmouse_y = y;
    if (!mouse_pos_change_saved)
        screen_place();
    D_00178bf4 = 0;
    return 1;
}

}

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

// FUNCTION: 0x000bde14
long screen_place(void)
{
    s32 h;
    s32 n;
    s32 k;
    u8 *mask;
    u8 *scr;
    u8 *save;
    u8 *sprite;
    u8 *screen;
    s32 x;
    s32 i;
    s32 j;
    s16 page;

    if (!D_00176bc0)
        return -1;
    if (lbDisplay.mouse_sprite) {
        D_00177bd4 = lbDisplay.mmouse_x + D_00177bec;
        D_00177bd8 = lbDisplay.mmouse_y + D_00177bf0;
        D_00176bc4 = mouse_setup_range();
        if (D_00176bc4 == 1) {
            save = D_00176bd4;
            sprite = D_00177bf4 + D_00177bdc + lbDisplay.mouse_sprite->width * D_00177be0;
            D_00176bd0 = D_00177bd4 + D_00177bdc + (D_00177bd8 + D_00177be0) * lbDisplay.unknown18;
            if (!lbDisplay.vesa_is_set_up) {
                screen = lbDisplay.physical_screen + D_00176bd0;
                for (h = D_00176bcc; --h != -1; screen += lbDisplay.unknown18, sprite += lbDisplay.mouse_sprite->width) {
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
                for (h = D_00176bcc; --h != -1; screen += lbDisplay.unknown18, sprite += lbDisplay.mouse_sprite->width) {
                    scr = screen;
                    mask = sprite;
                    n = D_00176bc8;
                    if ((u32)(scr + D_00176bc8) >= 0xb0000) {
                        k = 0xb0000 - (u32)scr;
                        if (k > 0) {
                            for (i = 0; i < k; i++, scr++, mask++) {
                                *save++ = *scr;
                                if (*mask != 0xfe)
                                    *scr = *mask;
                            }
                            n -= k;
                        }
                        ++page;
                        LbVesaSetPage(page);
                        scr -= 0x10000;
                        screen -= 0x10000;
                    }
                    for (j = 0; j < n; j++, scr++, mask++) {
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

// NONMATCHING: register choice: the target tests D_00176bc4 in memory; here it is loaded into edi (saved and
// restored in the prologue/epilogue)
// FUNCTION: 0x000be0cc
long screen_remove(u32 force)
{
    s32 h;
    s32 n;
    s32 k;
    u8 *save;
    u8 *scr;
    u8 *screen;
    s32 x;
    s32 i;
    s32 j;
    s16 page;

    if (!D_00176bc0)
        return -1;
    if (lbDisplay.mmouse_x != D_00177bd4 || lbDisplay.mmouse_y != D_00177bd8 || force) {
        if (D_00176bc4) {
            save = D_00176bd4;
            if (!lbDisplay.vesa_is_set_up) {
                screen = lbDisplay.physical_screen + D_00176bd0;
                for (h = D_00176bcc; --h != -1; screen += lbDisplay.unknown18) {
                    scr = screen;
                    for (x = 0; x < D_00176bc8; x++)
                        *scr++ = *save++;
                }
            } else {
                page = D_00176bd0 >> 16;
                LbVesaSetPage(page);
                screen = lbDisplay.physical_screen + (D_00176bd0 & 0xffff);
                for (h = D_00176bcc; --h != -1; screen += lbDisplay.unknown18) {
                    scr = screen;
                    n = D_00176bc8;
                    if ((u32)(scr + D_00176bc8) >= 0xb0000) {
                        k = 0xb0000 - (u32)scr;
                        if (k > 0) {
                            for (i = 0; i < k; i++)
                                *scr++ = *save++;
                            n -= k;
                        }
                        ++page;
                        LbVesaSetPage(page);
                        scr -= 0x10000;
                        screen -= 0x10000;
                    }
                    for (j = 0; j < n; j++)
                        *scr++ = *save++;
                }
            }
            D_00176bc4 = 0;
        }
    }
    return 1;
}
