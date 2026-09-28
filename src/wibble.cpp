#include "hosp.h"

extern "C" {

extern s32 lbSinTable[];
extern s32 D_00188c3c;
extern s32 D_00188c40;
extern s32 D_00188c44;
extern s32 D_00188c48;

}

extern s32 *xoffptr;
extern s32 *yoffptr;
extern s32 srcwidth;
extern s32 aamp;

// FUNCTION: 0x0006ec10
void init_wibble(void *xoff, void *yoff, s32 w, s32 h, s32 width, s32 freq, s32 amp)
{
    s32 d;
    s32 m;
    s32 n;
    s32 *q;
    s32 *p;
    s32 step;
    s32 angle;

    aamp = amp;
    yoffptr = (s32 *)yoff;
    p = yoffptr;
    step = 0x800 / freq;
    for (n = h, angle = 0; n; p++, angle = (angle + step) & 0x7ff, n--)
        *p = amp + (lbSinTable[angle] * amp >> 16);
    xoffptr = (s32 *)xoff;
    q = xoffptr;
    for (m = w, angle = 0; m; q++, angle = (angle + step) & 0x7ff, m--)
        *q = lbSinTable[angle] * amp >> 16;
    q = xoffptr;
    for (m = 0; m < w - 1; q++, m++) {
        d = q[1] - q[0];
        *q = d = width * d + 1;
    }
    srcwidth = width;
}

// FUNCTION: 0x0006ed58
void draw_wibble(s32 x, s32 y, u8 *buf, s32 w, s32 h, s32 phase)
{
    u8 *s;
    u8 *dst;
    u8 *src;
    s32 *xp;
    s32 *yp;
    s32 k;
    s32 rows;
    s32 dstskip;
    s32 skipx;
    s32 skipy;

    dst = lbDisplay.unknown4 + D_00188c3c + D_00188c40 * D_00188c34;
    src = buf + (yoffptr[phase] - aamp) * srcwidth;
    if (x >= 0) {
        if (x >= D_00188c44)
            return;
        skipx = 0;
        dst += x;
        if (x + w > D_00188c44)
            w = D_00188c44 - x;
    } else {
        w += x;
        if (w <= 0)
            return;
        skipx = -x;
    }
    if (y >= 0) {
        if (y >= D_00188c48)
            return;
        dst += D_00188c34 * y;
        if (y + h > D_00188c48)
            h = D_00188c48 - y;
        skipy = 0;
        yp = &yoffptr[phase];
    } else {
        h += y;
        if (h <= 0)
            return;
        skipy = -y;
        src += srcwidth * skipy;
        yp = &yoffptr[phase + skipy];
    }
    dstskip = D_00188c34 - w;
    src -= aamp;
    for (rows = h; rows; dst += dstskip, yp++, src += srcwidth, rows--) {
        s = src + *yp;
        xp = &xoffptr[phase];
        for (k = skipx; k; k--)
            s += *xp++;
        for (k = w; k; dst++, k--) {
            u8 c;

            c = *s;
            if (*s != 0xfe)
                *dst = *s;
            s += *xp++;
        }
    }
}
