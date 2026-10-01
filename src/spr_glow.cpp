#include "hosp.h"

extern "C" {
}


// Draws RLE sprite spr at (x, y) in the graphics window, clipped, honouring
// lbDisplay.draw_flags: 1 mirrors it, 2 flips it vertically, and 4 or 8
// blend each pixel with colour glow through the ghost table (4 with the
// pixel first, 8 with glow first). Always returns 1.
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
        s32 overhang;
        u8 *dst;
        s32 pitch;
        s8 *src;
        s32 rows;
        u8 *next_row;
        s32 remaining;

        x += lbDisplay.graphics_window_x;
        y += lbDisplay.graphics_window_y;
        w = spr->width;
        overhang = lbDisplay.graphics_window_x - x;
        if (overhang > 0) {
            if (w <= overhang)
                return 1;
            left = overhang;
        } else {
            left = 0;
        }
        overhang = x + w - (lbDisplay.graphics_window_x + lbDisplay.graphics_window_width);
        if (overhang > 0) {
            if (w <= overhang)
                return 1;
            right = w - overhang;
        } else {
            right = w;
        }
        h = spr->height;
        overhang = lbDisplay.graphics_window_y - y;
        if (overhang > 0) {
            if (h <= overhang)
                return 1;
            top = overhang;
        } else {
            top = 0;
        }
        overhang = y + h - (lbDisplay.graphics_window_y + lbDisplay.graphics_window_height);
        if (overhang > 0) {
            if (h <= overhang)
                return 1;
            bottom = h - overhang;
        } else {
            bottom = h;
        }
        if (lbDisplay.draw_flags & 2) {
            u8 old_bottom;

            dst = x + (lbDisplay.wscreen + (bottom + y - 1) * lbDisplay.graphics_screen_width) + left;
            pitch = lbDisplay.graphics_screen_width;
            pitch = -pitch;
            old_bottom = bottom;
            bottom = h - top;
            top = h - old_bottom;
        } else {
            dst = x + (lbDisplay.wscreen + (top + y) * lbDisplay.graphics_screen_width) + left;
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
            u8 old_right;
            u8 sprite_width;
            u8 xflip;

            if (lbDisplay.draw_flags & 1) {
                dst += w - 1;
                xflip = 1;
                sprite_width = spr->width;
                old_right = right;
                right = sprite_width - left;
                left = sprite_width - old_right;
            } else {
                xflip = 0;
            }
            next_row = dst + pitch;
            remaining = w;
            rows = h;
            if (lbDisplay.draw_flags & 0xc) {
                if (left)
                    goto glow_skip;
                for (;;) {
                    if (remaining > 0) {
                        if (*src > 0) {
                            u8 run;
                            u8 i;
                            u8 *pixel;

                            run = *src < remaining ? *src : remaining;
                            pixel = (u8 *)src + 1;
                            if (xflip) {
                                if (lbDisplay.draw_flags & 4) {
                                    for (i = 0; i < run; i++, pixel++, dst--)
                                        *dst = *(lbDisplay.ghost + (*pixel << 8) + glow);
                                } else {
                                    for (i = 0; i < run; i++, pixel++, dst--)
                                        *dst = *(lbDisplay.ghost + (glow << 8) + *pixel);
                                }
                            } else {
                                if (lbDisplay.draw_flags & 4) {
                                    for (i = 0; i < run; i++, pixel++, dst++)
                                        *dst = *(lbDisplay.ghost + (*pixel << 8) + glow);
                                } else {
                                    for (i = 0; i < run; i++, pixel++, dst++)
                                        *dst = *(lbDisplay.ghost + (glow << 8) + *pixel);
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
                                dst = next_row;
                                next_row += pitch;
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
                        dst = next_row;
                        next_row += pitch;
                        remaining = w;
                    glow_skip:
                        if (left) {
                            u8 skip;

                            skip = left;
                            do {
                                if (*src > 0) {
                                    if (*src > skip) {
                                        u8 j;
                                        u8 *pixel;

                                        pixel = (u8 *)src + 1 + skip;
                                        len = *src - skip;
                                        if (len > w)
                                            len = w;
                                        if (xflip) {
                                            if (lbDisplay.draw_flags & 4) {
                                                for (j = 0; j < len; j++, pixel++, dst--)
                                                    *dst = *(lbDisplay.ghost + (*pixel << 8) + glow);
                                            } else {
                                                for (j = 0; j < len; j++, pixel++, dst--)
                                                    *dst = *(lbDisplay.ghost + (glow << 8) + *pixel);
                                            }
                                        } else {
                                            if (lbDisplay.draw_flags & 4) {
                                                for (j = 0; j < len; j++, pixel++, dst++)
                                                    *dst = *(lbDisplay.ghost + (*pixel << 8) + glow);
                                            } else {
                                                for (j = 0; j < len; j++, pixel++, dst++)
                                                    *dst = *(lbDisplay.ghost + (glow << 8) + *pixel);
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
                            u8 run;
                            u8 i;
                            u8 *pixel;

                            run = *src < remaining ? *src : remaining;
                            pixel = (u8 *)src + 1;
                            for (i = 0; i < run; i++, pixel++, dst--)
                                *dst = *pixel;
                            remaining -= *src;
                            src += *src + 1;
                        } else if (*src < 0) {
                            remaining += *src;
                            dst += *src;
                            src++;
                        } else {
                            if (--rows != 0) {
                                dst = next_row;
                                next_row += pitch;
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
                        dst = next_row;
                        next_row += pitch;
                        remaining = w;
                    flip_skip:
                        if (left) {
                            u8 skip;

                            skip = left;
                            do {
                                if (*src > 0) {
                                    if (*src > skip) {
                                        u8 j;
                                        u8 *pixel;

                                        len = *src - skip;
                                        if (len > w)
                                            len = w;
                                        pixel = (u8 *)src + 1 + skip;
                                        for (j = 0; j < len; j++, pixel++, dst--)
                                            *dst = *pixel;
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
            next_row = dst + pitch;
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
                            dst = next_row;
                            next_row += pitch;
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
                    dst = next_row;
                    next_row += pitch;
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
