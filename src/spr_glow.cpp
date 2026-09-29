#include "hosp.h"

extern "C" {
}


// FUNCTION: 0x0006ef70
s32 sprite_draw_glow(s32 x, s32 y, TbSprite *spr, u8 glow)
{
    u8 len;
    u8 h;
    u8 top;
    u8 bottom;
    u8 left;
    u8 right;

    if (!spr->width || !spr->height)
        return 1;
    if (lbDisplay.graphics_window_width == 0 || lbDisplay.graphics_window_height == 0)
        return 1;
    {
        u8 w;
        s32 d;
        u8 *dst;
        s32 pitch;
        s8 *src;
        s32 rows;
        u8 *next;
        s32 remaining;

        x += lbDisplay.graphics_window_x;
        y += lbDisplay.graphics_window_y;
        w = spr->width;
        d = lbDisplay.graphics_window_x - x;
        if (d > 0) {
            if (w <= d)
                return 1;
            left = d;
        } else {
            left = 0;
        }
        d = x + w - (lbDisplay.graphics_window_x + lbDisplay.graphics_window_width);
        if (d > 0) {
            if (w <= d)
                return 1;
            right = w - d;
        } else {
            right = w;
        }
        h = spr->height;
        d = lbDisplay.graphics_window_y - y;
        if (d > 0) {
            if (h <= d)
                return 1;
            top = d;
        } else {
            top = 0;
        }
        d = y + h - (lbDisplay.graphics_window_y + lbDisplay.graphics_window_height);
        if (d > 0) {
            if (h <= d)
                return 1;
            bottom = h - d;
        } else {
            bottom = h;
        }
        if (lbDisplay.draw_flags & 2) {
            u8 tmp;

            dst = x + (lbDisplay.unknown4 + (bottom + y - 1) * lbDisplay.graphics_screen_width) + left;
            pitch = lbDisplay.graphics_screen_width;
            pitch = -pitch;
            tmp = bottom;
            bottom = h - top;
            top = h - tmp;
        } else {
            dst = x + (lbDisplay.unknown4 + (top + y) * lbDisplay.graphics_screen_width) + left;
            pitch = lbDisplay.graphics_screen_width;
        }
        h = bottom - top;
        w = right - left;
        src = (s8 *)spr->data;
        if (top) {
            rows = top;
            for (;;) {
                if (*src > 0) {
                    src += *src + 1;
                } else if (*src++ == 0) {
                    if (--rows == 0)
                        break;
                }
            }
        }
        if (lbDisplay.draw_flags & 0xd) {
            u8 tmp2;
            u8 sw;
            u8 xflip;

            if (lbDisplay.draw_flags & 1) {
                dst += w - 1;
                xflip = 1;
                sw = spr->width;
                tmp2 = right;
                right = sw - left;
                left = sw - tmp2;
            } else {
                xflip = 0;
            }
            next = dst + pitch;
            remaining = w;
            rows = h;
            if (lbDisplay.draw_flags & 0xc) {
                if (left)
                    goto glow_skip;
                for (;;) {
                    if (remaining > 0) {
                        if (*src > 0) {
                            u8 n;
                            u8 i;
                            u8 *p;

                            n = *src < remaining ? *src : remaining;
                            p = (u8 *)src + 1;
                            if (xflip) {
                                if (lbDisplay.draw_flags & 4) {
                                    for (i = 0; i < n; i++, p++, dst--)
                                        *dst = *(lbDisplay.ghost + (*p << 8) + glow);
                                } else {
                                    for (i = 0; i < n; i++, p++, dst--)
                                        *dst = *(lbDisplay.ghost + (glow << 8) + *p);
                                }
                            } else {
                                if (lbDisplay.draw_flags & 4) {
                                    for (i = 0; i < n; i++, p++, dst++)
                                        *dst = *(lbDisplay.ghost + (*p << 8) + glow);
                                } else {
                                    for (i = 0; i < n; i++, p++, dst++)
                                        *dst = *(lbDisplay.ghost + (glow << 8) + *p);
                                }
                            }
                            remaining -= *src;
                            src += *src + 1;
                        } else if (*src < 0) {
                            remaining += *src;
                            if (xflip)
                                dst += *src;
                            else
                                dst -= *src;
                            src++;
                        } else {
                            if (--rows != 0) {
                                dst = next;
                                next += pitch;
                                remaining = w;
                                src++;
                                goto glow_skip;
                            }
                            return 1;
                        }
                    } else {
                        if (--rows == 0)
                            return 1;
                        for (;;) {
                            if (*src > 0) {
                                src += *src + 1;
                            } else if (*src++ == 0) {
                                break;
                            }
                        }
                        dst = next;
                        next += pitch;
                        remaining = w;
                    glow_skip:
                        if (left) {
                            u8 skip;

                            skip = left;
                            do {
                                if (*src > 0) {
                                    if (*src > skip) {
                                        u8 j;
                                        u8 *p;

                                        p = (u8 *)src + 1 + skip;
                                        len = *src - skip;
                                        if (len > w)
                                            len = w;
                                        if (xflip) {
                                            if (lbDisplay.draw_flags & 4) {
                                                for (j = 0; j < len; j++, p++, dst--)
                                                    *dst = *(lbDisplay.ghost + (*p << 8) + glow);
                                            } else {
                                                for (j = 0; j < len; j++, p++, dst--)
                                                    *dst = *(lbDisplay.ghost + (glow << 8) + *p);
                                            }
                                        } else {
                                            if (lbDisplay.draw_flags & 4) {
                                                for (j = 0; j < len; j++, p++, dst++)
                                                    *dst = *(lbDisplay.ghost + (*p << 8) + glow);
                                            } else {
                                                for (j = 0; j < len; j++, p++, dst++)
                                                    *dst = *(lbDisplay.ghost + (glow << 8) + *p);
                                            }
                                        }
                                        src += *src + 1;
                                        remaining -= len;
                                        break;
                                    }
                                    skip -= *src;
                                    src += *src + 1;
                                } else if (*src == 0) {
                                    remaining = 0;
                                    break;
                                } else if (*src < -skip) {
                                    len = -*src - skip;
                                    if (len > w)
                                        len = w;
                                    src++;
                                    if (xflip)
                                        dst -= len;
                                    else
                                        dst += len;
                                    remaining -= len;
                                    break;
                                } else {
                                    skip += *src;
                                    src++;
                                }
                            } while (skip);
                        }
                    }
                }
            } else {
                if (left)
                    goto flip_skip;
                for (;;) {
                    if (remaining > 0) {
                        if (*src > 0) {
                            u8 n;
                            u8 i;
                            u8 *p;

                            n = *src < remaining ? *src : remaining;
                            p = (u8 *)src + 1;
                            for (i = 0; i < n; i++, p++, dst--)
                                *dst = *p;
                            remaining -= *src;
                            src += *src + 1;
                        } else if (*src < 0) {
                            remaining += *src;
                            dst += *src;
                            src++;
                        } else {
                            if (--rows != 0) {
                                dst = next;
                                next += pitch;
                                remaining = w;
                                src++;
                                goto flip_skip;
                            }
                            return 1;
                        }
                    } else {
                        if (--rows == 0)
                            return 1;
                        for (;;) {
                            if (*src > 0) {
                                src += *src + 1;
                            } else if (*src++ == 0) {
                                break;
                            }
                        }
                        dst = next;
                        next += pitch;
                        remaining = w;
                    flip_skip:
                        if (left) {
                            u8 skip;

                            skip = left;
                            do {
                                if (*src > 0) {
                                    if (*src > skip) {
                                        u8 j;
                                        u8 *p;

                                        len = *src - skip;
                                        if (len > w)
                                            len = w;
                                        p = (u8 *)src + 1 + skip;
                                        for (j = 0; j < len; j++, p++, dst--)
                                            *dst = *p;
                                        src += *src + 1;
                                        remaining -= len;
                                        break;
                                    }
                                    skip -= *src;
                                    src += *src + 1;
                                } else if (*src == 0) {
                                    remaining = 0;
                                    break;
                                } else if (*src < -skip) {
                                    len = -*src - skip;
                                    if (len > w)
                                        len = w;
                                    src++;
                                    dst -= len;
                                    remaining -= len;
                                    break;
                                } else {
                                    skip += *src;
                                    src++;
                                }
                            } while (skip);
                        }
                    }
                }
            }
        } else {
            next = dst + pitch;
            remaining = w;
            rows = h;
            if (left)
                goto plain_skip;
            for (;;) {
                if (remaining > 0) {
                    if (*src > 0) {
                        fn_000c21e6(dst, src + 1, *src < remaining ? *src : remaining);
                        remaining -= *src;
                        dst += *src;
                        src += *src + 1;
                    } else if (*src < 0) {
                        remaining += *src;
                        dst -= *src;
                        src++;
                    } else {
                        if (--rows != 0) {
                            dst = next;
                            next += pitch;
                            remaining = w;
                            src++;
                            goto plain_skip;
                        }
                        return 1;
                    }
                } else {
                    if (--rows == 0)
                        return 1;
                    for (;;) {
                        if (*src > 0) {
                            src += *src + 1;
                        } else if (*src++ == 0) {
                            break;
                        }
                    }
                    dst = next;
                    next += pitch;
                    remaining = w;
                plain_skip:
                    if (left) {
                        u8 skip;

                        skip = left;
                        do {
                            if (*src > 0) {
                                if (*src > skip) {
                                    len = *src - skip;
                                    if (len > w)
                                        len = w;
                                    fn_000c21e6(dst, src + 1 + skip, len);
                                    dst += len;
                                    src += *src + 1;
                                    remaining -= len;
                                    break;
                                }
                                skip -= *src;
                                src += *src + 1;
                            } else if (*src == 0) {
                                remaining = 0;
                                break;
                            } else if (*src < -skip) {
                                len = -*src - skip;
                                if (len > w)
                                    len = w;
                                src++;
                                dst += len;
                                remaining -= len;
                                break;
                            } else {
                                skip += *src;
                                src++;
                            }
                        } while (skip);
                    }
                }
            }
        }
    }
    return 1;
}
