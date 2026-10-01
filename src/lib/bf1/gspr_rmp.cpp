#include "bflib.h"

extern "C" {

// Draws RLE sprite spr at (x, y) in the graphics window with each pixel
// mapped through the colour table remap, clipped, honouring draw_flags (1
// mirror, 2 vertical flip, 4/8 blend with the ghost table).
// FUNCTION: 0xcbe20
// NONMATCHING: the byte locals' stack slots are permuted
int LbSpriteDrawRemap(s32 x, s32 y, TbSprite *spr, u8 *remap)
{
    u8 *dst;
    u8 *next;
    s8 *rle;
    s32 rows;
    s32 clip;
    s32 remain;
    u8 bottom;
    u8 run;
    u8 h;
    u8 w;
    u8 right;
    u8 xflip;
    s32 pitch;
    u8 top;
    u8 left;

    if (!spr->width || !spr->height)
        return 1;
    if (!lbDisplay.graphics_window_width || !lbDisplay.graphics_window_height)
        return 1;
    x += lbDisplay.graphics_window_x;
    y += lbDisplay.graphics_window_y;
    w = spr->width;
    clip = lbDisplay.graphics_window_x - x;
    if (clip > 0) {
        if (w <= clip)
            return 1;
        left = clip;
    } else {
        left = 0;
    }
    clip = w + x - (lbDisplay.graphics_window_x + lbDisplay.graphics_window_width);
    if (clip > 0) {
        if (w <= clip)
            return 1;
        right = w - clip;
    } else {
        right = w;
    }
    h = spr->height;
    clip = lbDisplay.graphics_window_y - y;
    if (clip > 0) {
        if (h <= clip)
            return 1;
        top = clip;
    } else {
        top = 0;
    }
    clip = h + y - (lbDisplay.graphics_window_y + lbDisplay.graphics_window_height);
    if (clip > 0) {
        if (h <= clip)
            return 1;
        bottom = h - clip;
    } else {
        bottom = h;
    }
    if (lbDisplay.draw_flags & 2) {
        u8 old_bottom;

        dst = x + (lbDisplay.wscreen + (bottom + y - 1) * lbDisplay.graphics_screen_width) + left;
        pitch = -lbDisplay.graphics_screen_width;
        old_bottom = bottom;
        bottom = h - top;
        top = h - old_bottom;
    } else {
        dst = lbDisplay.wscreen + lbDisplay.graphics_screen_width * (top + y) + x + left;
        pitch = lbDisplay.graphics_screen_width;
    }
    h = bottom - top;
    w = right - left;
    rle = (s8 *)spr->data;
    if (top) {
        rows = top;
        do {
            do {
                while (*rle > 0)
                    rle += *rle + 1;
            } while (*rle++ != 0);
        } while (--rows);
    }
    if (lbDisplay.draw_flags & 0xd) {
        if (lbDisplay.draw_flags & 1) {
            u8 full_width;
            u8 old_right;

            dst += w - 1;
            xflip = 1;
            full_width = spr->width;
            old_right = right;
            right = full_width - left;
            left = full_width - old_right;
        } else {
            xflip = 0;
        }
        next = dst + pitch;
        remain = w;
        rows = h;
        if (lbDisplay.draw_flags & 0xc) {
            u8 skip;

            if (left)
                goto t_left;
            for (;;) {
                if (remain > 0) {
                    if (*rle > 0) {
                        u8 *s;
                        u8 n;
                        u8 i;

                        n = *rle < remain ? *rle : remain;
                        s = (u8 *)rle + 1;
                        if (xflip) {
                            if (lbDisplay.draw_flags & 4) {
                                for (i = 0; i < n; i++, s++, dst--)
                                    *dst = *(lbDisplay.ghost + (remap[*s] << 8) + *dst);
                            } else {
                                for (i = 0; i < n; i++, s++, dst--)
                                    *dst = *(lbDisplay.ghost + (*dst << 8) + remap[*s]);
                            }
                        } else {
                            if (lbDisplay.draw_flags & 4) {
                                for (i = 0; i < n; i++, s++, dst++)
                                    *dst = *(lbDisplay.ghost + (remap[*s] << 8) + *dst);
                            } else {
                                for (i = 0; i < n; i++, s++, dst++)
                                    *dst = *(lbDisplay.ghost + (*dst << 8) + remap[*s]);
                            }
                        }
                        remain -= *rle;
                        rle += *rle + 1;
                    } else if (*rle < 0) {
                        remain += *rle;
                        if (xflip)
                            dst += *rle;
                        else
                            dst -= *rle;
                        rle++;
                    } else {
                        if (--rows) {
                            dst = next;
                            next += pitch;
                            remain = w;
                            rle++;
                            goto t_left;
                        }
                        return 1;
                    }
                } else {
                    if (!--rows)
                        return 1;
                    do {
                        while (*rle > 0)
                            rle += *rle + 1;
                    } while (*rle++ != 0);
                    dst = next;
                    next += pitch;
                    remain = w;
                t_left:
                    if (left) {
                        skip = left;
                        do {
                            if (*rle > 0) {
                                if (*rle > skip) {
                                    u8 *s;
                                    u8 i;

                                    s = (u8 *)rle + 1 + skip;
                                    run = *rle - skip;
                                    if (run > w)
                                        run = w;
                                    if (xflip) {
                                        if (lbDisplay.draw_flags & 4) {
                                            for (i = 0; i < run; i++, s++, dst--)
                                                *dst = *(lbDisplay.ghost + (remap[*s] << 8) + *dst);
                                        } else {
                                            for (i = 0; i < run; i++, s++, dst--)
                                                *dst = *(lbDisplay.ghost + (*dst << 8) + remap[*s]);
                                        }
                                    } else {
                                        if (lbDisplay.draw_flags & 4) {
                                            for (i = 0; i < run; i++, s++, dst++)
                                                *dst = *(lbDisplay.ghost + (remap[*s] << 8) + *dst);
                                        } else {
                                            for (i = 0; i < run; i++, s++, dst++)
                                                *dst = *(lbDisplay.ghost + (*dst << 8) + remap[*s]);
                                        }
                                    }
                                    rle += *rle + 1;
                                    remain -= run;
                                    break;
                                }
                                skip -= *rle;
                                rle += *rle + 1;
                            } else if (*rle == 0) {
                                remain = 0;
                                break;
                            } else {
                                if (*rle < -skip) {
                                    run = -*rle - skip;
                                    if (run > w)
                                        run = w;
                                    rle++;
                                    if (xflip)
                                        dst -= run;
                                    else
                                        dst += run;
                                    remain -= run;
                                    break;
                                }
                                skip += *rle;
                                rle++;
                            }
                        } while (skip);
                    }
                }
            }
        } else {
            u8 skip;

            if (left)
                goto f_left;
            for (;;) {
                if (remain > 0) {
                    if (*rle > 0) {
                        u8 n;
                        u8 *s;
                        u8 i;

                        n = *rle < remain ? *rle : remain;
                        s = (u8 *)rle + 1;
                        for (i = 0; i < n; i++, s++, dst--)
                            *dst = remap[*s];
                        remain -= *rle;
                        rle += *rle + 1;
                    } else if (*rle < 0) {
                        remain += *rle;
                        dst += *rle;
                        rle++;
                    } else {
                        if (--rows) {
                            dst = next;
                            next += pitch;
                            remain = w;
                            rle++;
                            goto f_left;
                        }
                        return 1;
                    }
                } else {
                    if (!--rows)
                        return 1;
                    do {
                        while (*rle > 0)
                            rle += *rle + 1;
                    } while (*rle++ != 0);
                    dst = next;
                    next += pitch;
                    remain = w;
                f_left:
                    if (left) {
                        skip = left;
                        do {
                            if (*rle > 0) {
                                if (*rle > skip) {
                                    u8 *s;
                                    u8 i;

                                    run = *rle - skip;
                                    if (run > w)
                                        run = w;
                                    s = (u8 *)rle + 1 + skip;
                                    for (i = 0; i < run; i++, s++, dst--)
                                        *dst = remap[*s];
                                    rle += *rle + 1;
                                    remain -= run;
                                    break;
                                }
                                skip -= *rle;
                                rle += *rle + 1;
                            } else if (*rle == 0) {
                                remain = 0;
                                break;
                            } else {
                                if (*rle < -skip) {
                                    run = -*rle - skip;
                                    if (run > w)
                                        run = w;
                                    rle++;
                                    dst -= run;
                                    remain -= run;
                                    break;
                                }
                                skip += *rle;
                                rle++;
                            }
                        } while (skip);
                    }
                }
            }
        }
    } else {
        u8 skip;

        next = dst + pitch;
        remain = w;
        rows = h;
        if (left)
            goto n_left;
        for (;;) {
            if (remain > 0) {
                if (*rle > 0) {
                    u8 *end;
                    u8 *d;
                    u8 *s;

                    end = dst + (*rle < remain ? *rle : remain);
                    for (d = dst, s = (u8 *)rle + 1; d < end; d++, s++)
                        *d = remap[*s];
                    remain -= *rle;
                    dst += *rle;
                    rle += *rle + 1;
                } else if (*rle < 0) {
                    remain += *rle;
                    dst -= *rle;
                    rle++;
                } else {
                    if (--rows) {
                        dst = next;
                        next += pitch;
                        remain = w;
                        rle++;
                        goto n_left;
                    }
                    return 1;
                }
            } else {
                if (!--rows)
                    return 1;
                do {
                    while (*rle > 0)
                        rle += *rle + 1;
                } while (*rle++ != 0);
                dst = next;
                next += pitch;
                remain = w;
            n_left:
                if (left) {
                    skip = left;
                    do {
                        if (*rle > 0) {
                            if (*rle > skip) {
                                u8 *end;
                                u8 *d;
                                u8 *s;

                                run = *rle - skip;
                                if (run > w)
                                    run = w;
                                end = dst + run;
                                d = dst;
                                s = (u8 *)rle + 1 + skip;
                                for (; d < end; d++, s++)
                                    *d = remap[*s];
                                dst += run;
                                rle += *rle + 1;
                                remain -= run;
                                break;
                            }
                            skip -= *rle;
                            rle += *rle + 1;
                        } else if (*rle == 0) {
                            remain = 0;
                            break;
                        } else {
                            if (*rle < -skip) {
                                run = -*rle - skip;
                                if (run > w)
                                    run = w;
                                rle++;
                                dst += run;
                                remain -= run;
                                break;
                            }
                            skip += *rle;
                            rle++;
                        }
                    } while (skip);
            }
            }
        }
    }
}

}
