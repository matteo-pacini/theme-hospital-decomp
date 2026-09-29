#include "bflib.h"

extern u8 fade_started;

extern "C" {
extern u8 D_00189ad4[];
extern u8 D_001897d4[];
extern s32 D_00189dd4;
extern char D_001242c4[];

int LbScreenWaitVbi(void);
int abs(int n);
#pragma intrinsic(abs)

u8 fn_000c960c(u8 *palette, u8 r, u8 g, u8 b);
#pragma aux fn_000c960c "W?draw_bdem_unavailable_message$n()v"
}

extern "C" {

// FUNCTION: 0x000c9400
s32 fn_000c9400(u8 *to_pal, u8 fade_steps, TbPaletteFadeFlag flag)
{
    s16 i;
    s16 diff;
    u8 palette[768];

    if (flag == Lb_PALETTE_FADE_CLOSED) {
        LbPaletteGet(D_00189ad4);
        if (!to_pal) {
            to_pal = D_001897d4;
            memset(to_pal, 0, 768);
        }
        for (D_00189dd4 = 0; D_00189dd4 <= fade_steps; D_00189dd4++) {
            for (i = 0; i < 768; i++) {
                diff = to_pal[i] - D_00189ad4[i];
                palette[i] = diff * D_00189dd4 / (s16)fade_steps + D_00189ad4[i];
            }
            LbScreenWaitVbi();
            LbPaletteSet(palette);
        }
        fade_started = 0;
    } else {
        if (!fade_started) {
            D_00189dd4 = 0;
            fade_started = 1;
            LbPaletteGet(D_00189ad4);
            if (!to_pal)
                memset(D_001897d4, 0, 768);
        } else {
            D_00189dd4++;
            if (fade_steps == D_00189dd4)
                fade_started = 0;
        }
        if (!to_pal)
            to_pal = D_001897d4;
        for (i = 0; i < 768; i++) {
            diff = to_pal[i] - D_00189ad4[i];
            palette[i] = diff * D_00189dd4 / fade_steps + D_00189ad4[i];
        }
        LbScreenWaitVbi();
        LbPaletteSet(palette);
    }
    return D_00189dd4;
}

// FUNCTION: 0x000c95f8
int fn_000c95f8(void)
{
    fade_started = 0;
    return 1;
}

}

struct Rgb {
    u8 r;
    u8 g;
    u8 b;
};

#define SQR(x) ((x) * (x))

// NONMATCHING: stack slot order, register choice
// FUNCTION: 0x000c960c
u8 fn_000c960c(u8 *palette, u8 r, u8 g, u8 b)
{
    s32 dist;
    Rgb *base;
    u32 n;
    s32 i;
    s32 ri;
    s32 gi;
    s32 bi;
    s32 best;
    u8 list2[256];
    u8 colour;
    u16 count2;
    u16 count1;
    Rgb *p;
    u8 list1[256];
    u8 *out;

    ri = r;
    gi = g;
    bi = b;
    count1 = 0;
    count2 = 0;
    base = p = (Rgb *)palette;
    n = 0x100;
    for (i = 0, best = 999999; i < n; i++, p++) {
        dist = SQR(ri - p->r) + SQR(gi - p->g) + SQR(bi - p->b);
        if (dist < best) {
            best = dist;
            if (best == 0)
                return i;
        }
    }
    out = list1;
    p = (Rgb *)palette;
    for (i = 0; i < n; i++, p++) {
        dist = SQR(ri - p->r) + SQR(gi - p->g) + SQR(bi - p->b);
        if (dist == best) {
            count1++;
            *out++ = i;
        }
    }
    if (count1 == 1)
        return list1[0];
    for (i = 0, best = 999999; i < count1; i++) {
        p = &base[list1[i]];
        dist = abs(ri - p->r) + abs(gi - p->g) + abs(bi - p->b);
        if (dist < best)
            best = dist;
    }
    out = list2;
    for (i = 0; i < count1; i++) {
        p = &base[list1[i]];
        dist = abs(ri - p->r) + abs(gi - p->g) + abs(bi - p->b);
        if (dist == best) {
            count2++;
            *out++ = list1[i];
        }
    }
    if (count2 == 1)
        return list2[0];
    count1 = 0;
    out = list1;
    for (i = 0, best = 999999; i < count2; i++) {
        p = &base[list2[i]];
        dist = SQR(p->g) * 2 + SQR(p->r) * 2 + SQR(p->b);
        if (dist < best) {
            best = dist;
            colour = list2[i];
        }
    }
    return colour;
}

extern "C" {

// FUNCTION: 0x000c99d8
int fn_000c99d8(s32 x, s32 y, s32 w, s32 h, u8 show_numbers)
{
    u16 old_flags;
    char text[8];
    u32 j;
    u32 i;
    s32 px;
    u8 colour;
    u8 old_colour;

    colour = 0;
    old_flags = lbDisplay.draw_flags;
    old_colour = lbDisplay.draw_colour;
    lbDisplay.draw_flags = 0x40;
    for (i = 0; i < 16; i++, y += h) {
        for (j = 0, px = x; j < 16; j++, px += w) {
            LbDrawBox(px, y, w, h, colour);
            if (show_numbers) {
                sprintf(text, D_001242c4, colour);
                lbDisplay.draw_colour = 255 - colour;
                LbTextDraw(px, y, text);
            }
            colour++;
        }
    }
    lbDisplay.draw_flags = old_flags;
    lbDisplay.draw_colour = old_colour;
    return 1;
}

}
