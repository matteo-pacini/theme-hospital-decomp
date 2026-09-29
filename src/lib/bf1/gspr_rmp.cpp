#include "bflib.h"

extern "C" {

// FUNCTION: 0xcbe20
// NONMATCHING: the byte locals' stack slots are permuted
int LbSpriteDrawRemap(s32 x, s32 y, TbSprite *spr, u8 *remap)
{
    u8 *dst;
    u8 *next;
    s8 *sp;
    s32 rows;
    s32 tmp;
    s32 cnt;
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
    tmp = lbDisplay.graphics_window_x - x;
    if (tmp > 0) {
        if (w <= tmp)
            return 1;
        left = tmp;
    } else {
        left = 0;
    }
    tmp = w + x - (lbDisplay.graphics_window_x + lbDisplay.graphics_window_width);
    if (tmp > 0) {
        if (w <= tmp)
            return 1;
        right = w - tmp;
    } else {
        right = w;
    }
    h = spr->height;
    tmp = lbDisplay.graphics_window_y - y;
    if (tmp > 0) {
        if (h <= tmp)
            return 1;
        top = tmp;
    } else {
        top = 0;
    }
    tmp = h + y - (lbDisplay.graphics_window_y + lbDisplay.graphics_window_height);
    if (tmp > 0) {
        if (h <= tmp)
            return 1;
        bottom = h - tmp;
    } else {
        bottom = h;
    }
    if (lbDisplay.draw_flags & 2) {
        u8 t;

        dst = x + (lbDisplay.unknown4 + (bottom + y - 1) * lbDisplay.graphics_screen_width) + left;
        pitch = -lbDisplay.graphics_screen_width;
        t = bottom;
        bottom = h - top;
        top = h - t;
    } else {
        dst = lbDisplay.unknown4 + lbDisplay.graphics_screen_width * (top + y) + x + left;
        pitch = lbDisplay.graphics_screen_width;
    }
    h = bottom - top;
    w = right - left;
    sp = (s8 *)spr->data;
    if (top) {
        rows = top;
        do {
            do {
                while (*sp > 0)
                    sp += *sp + 1;
            } while (*sp++ != 0);
        } while (--rows);
    }
    if (lbDisplay.draw_flags & 0xd) {
        if (lbDisplay.draw_flags & 1) {
            u8 t1;
            u8 t2;

            dst += w - 1;
            xflip = 1;
            t1 = spr->width;
            t2 = right;
            right = t1 - left;
            left = t1 - t2;
        } else {
            xflip = 0;
        }
        next = dst + pitch;
        cnt = w;
        rows = h;
        if (lbDisplay.draw_flags & 0xc) {
            u8 l;

            if (left)
                goto t_left;
            for (;;) {
                if (cnt > 0) {
                    if (*sp > 0) {
                        u8 *s;
                        u8 n;
                        u8 i;

                        n = *sp < cnt ? *sp : cnt;
                        s = (u8 *)sp + 1;
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
                        cnt -= *sp;
                        sp += *sp + 1;
                    } else if (*sp < 0) {
                        cnt += *sp;
                        if (xflip)
                            dst += *sp;
                        else
                            dst -= *sp;
                        sp++;
                    } else {
                        if (--rows) {
                            dst = next;
                            next += pitch;
                            cnt = w;
                            sp++;
                            goto t_left;
                        }
                        return 1;
                    }
                } else {
                    if (!--rows)
                        return 1;
                    do {
                        while (*sp > 0)
                            sp += *sp + 1;
                    } while (*sp++ != 0);
                    dst = next;
                    next += pitch;
                    cnt = w;
                t_left:
                    if (left) {
                        l = left;
                        do {
                            if (*sp > 0) {
                                if (*sp > l) {
                                    u8 *s;
                                    u8 i;

                                    s = (u8 *)sp + 1 + l;
                                    run = *sp - l;
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
                                    sp += *sp + 1;
                                    cnt -= run;
                                    break;
                                }
                                l -= *sp;
                                sp += *sp + 1;
                            } else if (*sp == 0) {
                                cnt = 0;
                                break;
                            } else {
                                if (*sp < -l) {
                                    run = -*sp - l;
                                    if (run > w)
                                        run = w;
                                    sp++;
                                    if (xflip)
                                        dst -= run;
                                    else
                                        dst += run;
                                    cnt -= run;
                                    break;
                                }
                                l += *sp;
                                sp++;
                            }
                        } while (l);
                    }
                }
            }
        } else {
            u8 l;

            if (left)
                goto f_left;
            for (;;) {
                if (cnt > 0) {
                    if (*sp > 0) {
                        u8 n;
                        u8 *s;
                        u8 i;

                        n = *sp < cnt ? *sp : cnt;
                        s = (u8 *)sp + 1;
                        for (i = 0; i < n; i++, s++, dst--)
                            *dst = remap[*s];
                        cnt -= *sp;
                        sp += *sp + 1;
                    } else if (*sp < 0) {
                        cnt += *sp;
                        dst += *sp;
                        sp++;
                    } else {
                        if (--rows) {
                            dst = next;
                            next += pitch;
                            cnt = w;
                            sp++;
                            goto f_left;
                        }
                        return 1;
                    }
                } else {
                    if (!--rows)
                        return 1;
                    do {
                        while (*sp > 0)
                            sp += *sp + 1;
                    } while (*sp++ != 0);
                    dst = next;
                    next += pitch;
                    cnt = w;
                f_left:
                    if (left) {
                        l = left;
                        do {
                            if (*sp > 0) {
                                if (*sp > l) {
                                    u8 *s;
                                    u8 i;

                                    run = *sp - l;
                                    if (run > w)
                                        run = w;
                                    s = (u8 *)sp + 1 + l;
                                    for (i = 0; i < run; i++, s++, dst--)
                                        *dst = remap[*s];
                                    sp += *sp + 1;
                                    cnt -= run;
                                    break;
                                }
                                l -= *sp;
                                sp += *sp + 1;
                            } else if (*sp == 0) {
                                cnt = 0;
                                break;
                            } else {
                                if (*sp < -l) {
                                    run = -*sp - l;
                                    if (run > w)
                                        run = w;
                                    sp++;
                                    dst -= run;
                                    cnt -= run;
                                    break;
                                }
                                l += *sp;
                                sp++;
                            }
                        } while (l);
                    }
                }
            }
        }
    } else {
        u8 l;

        next = dst + pitch;
        cnt = w;
        rows = h;
        if (left)
            goto n_left;
        for (;;) {
            if (cnt > 0) {
                if (*sp > 0) {
                    u8 *end;
                    u8 *d;
                    u8 *s;

                    end = dst + (*sp < cnt ? *sp : cnt);
                    for (d = dst, s = (u8 *)sp + 1; d < end; d++, s++)
                        *d = remap[*s];
                    cnt -= *sp;
                    dst += *sp;
                    sp += *sp + 1;
                } else if (*sp < 0) {
                    cnt += *sp;
                    dst -= *sp;
                    sp++;
                } else {
                    if (--rows) {
                        dst = next;
                        next += pitch;
                        cnt = w;
                        sp++;
                        goto n_left;
                    }
                    return 1;
                }
            } else {
                if (!--rows)
                    return 1;
                do {
                    while (*sp > 0)
                        sp += *sp + 1;
                } while (*sp++ != 0);
                dst = next;
                next += pitch;
                cnt = w;
            n_left:
                if (left) {
                    l = left;
                    do {
                        if (*sp > 0) {
                            if (*sp > l) {
                                u8 *end;
                                u8 *d;
                                u8 *s;

                                run = *sp - l;
                                if (run > w)
                                    run = w;
                                end = dst + run;
                                d = dst;
                                s = (u8 *)sp + 1 + l;
                                for (; d < end; d++, s++)
                                    *d = remap[*s];
                                dst += run;
                                sp += *sp + 1;
                                cnt -= run;
                                break;
                            }
                            l -= *sp;
                            sp += *sp + 1;
                        } else if (*sp == 0) {
                            cnt = 0;
                            break;
                        } else {
                            if (*sp < -l) {
                                run = -*sp - l;
                                if (run > w)
                                    run = w;
                                sp++;
                                dst += run;
                                cnt -= run;
                                break;
                            }
                            l += *sp;
                            sp++;
                        }
                    } while (l);
            }
            }
        }
    }
}

}
