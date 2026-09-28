#include "bflib.h"

extern "C" {
int abs(int n);
#pragma intrinsic(abs)

u8 fn_000c960c(u8 *palette, u8 r, u8 g, u8 b);
#pragma aux fn_000c960c "W?draw_bdem_unavailable_message$n()v"
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
    Rgb *p;
    u8 *out;
    u16 count2;
    u16 count1;
    u8 colour;
    u8 list1[256];
    u8 list2[256];

    ri = r;
    gi = g;
    bi = b;
    count1 = 0;
    count2 = 0;
    base = p = (Rgb *)palette;
    n = 0x100;
    for (i = 0, best = 999999; i < n; i++, p++) {
        dist = SQR(gi - p->g) + SQR(ri - p->r) + SQR(bi - p->b);
        if (dist < best) {
            best = dist;
            if (best == 0)
                return i;
        }
    }
    out = list1;
    p = (Rgb *)palette;
    for (i = 0; i < n; i++, p++) {
        dist = SQR(gi - p->g) + SQR(ri - p->r) + SQR(bi - p->b);
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
