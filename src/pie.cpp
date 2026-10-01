#include "hosp.h"

extern s32 pie_mouse_x;
extern s32 pie_mouse_y;
extern s32 pie_sector_return;
extern s32 side_off;
extern u8 done[];
#pragma aux done "W?done$:.0$:?pie_render$n(llllllpn$Sector$$)vn[]uc"

extern "C" {

void LbDrawTriangleFilled(s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3, u8 colour);
void LbDrawLine(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
}

/* 16.16 fixed-point product: (a * b) >> 16 over the full 64-bit result. */
s32 fixmul(s32 a, s32 b);
#pragma aux fixmul = "imul ebx" "mov ax,dx" "rol eax,16" parm [eax] [ebx] value [eax] modify [edx];

s32 point_in_triangle(s32 px, s32 py, s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2);
s32 point_in_quad(s32 px, s32 py, s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3);
s32 mouse_in_slice(s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, s32 h);

// Returns 1 if (px, py) lies inside or on the triangle (x0, y0), (x1, y1),
// (x2, y2), given in counterclockwise order on screen (y down), else 0.
// FUNCTION: 0x000b7c90
s32 point_in_triangle(s32 px, s32 py, s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2)
{
    s32 outside;

    outside = (x1 - x0) * (py - y0) > (y1 - y0) * (px - x0) || (x2 - x1) * (py - y1) > (y2 - y1) * (px - x1)
              || (x0 - x2) * (py - y2) > (y0 - y2) * (px - x2);
    return !outside;
}

// As point_in_triangle for a convex quadrilateral with corners in the same
// order.
// FUNCTION: 0x000b7d54
s32 point_in_quad(s32 px, s32 py, s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3)
{
    s32 outside;

    outside = (x1 - x0) * (py - y0) > (y1 - y0) * (px - x0) || (x2 - x1) * (py - y1) > (y2 - y1) * (px - x1)
              || (x3 - x2) * (py - y2) > (y3 - y2) * (px - x2) || (x0 - x3) * (py - y3) > (y0 - y3) * (px - x3);
    return !outside;
}

// Returns 1 if the pie mouse position (pie_mouse_x, pie_mouse_y) is over the
// pie segment with top corners (x0, y0), (x1, y1), (x2, y2) and side height
// h, including its sides.
// FUNCTION: 0x000b7e3c
s32 mouse_in_slice(s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, s32 h)
{
    s32 mx;
    s32 my;

    mx = pie_mouse_x;
    my = pie_mouse_y;
    return point_in_triangle(mx, my, x0, y0, x1, y1, x2, y2)
           || point_in_quad(mx, my, x0, y0, x0, y0 + h, x1, y1 + h, x1, y1)
           || point_in_quad(mx, my, x0, y0, x2, y2, x2, y2 + h, x0, y0 + h)
           || point_in_quad(mx, my, x1, y1, x1, y1 + h, x2, y2 + h, x2, y2);
}

// Draws sector i of a pie centred on (cx, cy) with radii rx, ry and
// thickness h (pixels): its top, rim and visible sides (side_off 1 or 2
// leaves out the start or end side), pulled out from the centre by its
// radius (1/256 of the radii). Sets its label position, and
// pie_sector_return to i if the mouse is over it. Angles: 0x800 a full turn.
// FUNCTION: 0x000b7f48
void sector_render(s32 cx, s32 cy, s32 rx, s32 ry, s32 h, Sector *sectors, s32 i)
{
    s32 ox;
    s32 oy;
    s32 start;
    s32 end;
    s32 mid;
    s32 sx;
    s32 sy;
    s32 ex;
    s32 ey;
    Sector *sec;
    s32 text_radius;
    s32 segments;
    s32 ang;
    s32 step;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;

    sec = &sectors[i];
    start = sec->start;
    end = sec->end;
    if (end < start)
        end += 0x800;
    mid = (start + end) >> 1;
    ox = cx + (fixmul(lbSinTable[(mid & 0x7ff) + 0x200], rx * sec->radius) >> 8);
    oy = cy - (fixmul(lbSinTable[mid & 0x7ff], ry * sec->radius) >> 8);
    sx = ox + fixmul(lbSinTable[(start & 0x7ff) + 0x200], rx);
    sy = oy - fixmul(lbSinTable[start & 0x7ff], ry);
    ex = ox + fixmul(lbSinTable[(end & 0x7ff) + 0x200], rx);
    ey = oy - fixmul(lbSinTable[end & 0x7ff], ry);
    text_radius = sec->text_offset + sec->radius;
    sec->text_x = cx + (fixmul(lbSinTable[(mid & 0x7ff) + 0x200], rx * text_radius) >> 8);
    sec->text_y = cy - (fixmul(lbSinTable[mid & 0x7ff], ry * text_radius) >> 8);
    sec->text_right = lbSinTable[(mid & 0x7ff) + 0x200] >= 0 ? 1 : 0;
    if (side_off != 1) {
        LbDrawTriangleFilled(ox, oy, sx, sy, sx, sy + h, sec->side_colour1);
        LbDrawTriangleFilled(sx, sy + h, ox, oy + h, ox, oy, sec->side_colour1);
    }
    if (side_off != 2) {
        LbDrawTriangleFilled(ox, oy, ox, oy + h, ex, ey, sec->side_colour2);
        LbDrawTriangleFilled(ex, ey, ox, oy + h, ex, ey + h, sec->side_colour2);
    }
    segments = (end - start) / 50;
    if (segments == 0)
        segments = 1;
    ang = start;
    step = (end - start) / segments;
    x2 = ox + fixmul(lbSinTable[(ang & 0x7ff) + 0x200], rx);
    y2 = oy - fixmul(lbSinTable[ang & 0x7ff], ry);
    while (--segments >= 0) {
        x1 = x2;
        y1 = y2;
        ang += step;
        if (segments == 0)
            ang = end;
        x2 = ox + fixmul(lbSinTable[(ang & 0x7ff) + 0x200], rx);
        y2 = oy - fixmul(lbSinTable[ang & 0x7ff], ry);
        LbDrawTriangleFilled(x1, y1 + h, x1, y1, x2, y2, sec->rim_colour);
        LbDrawTriangleFilled(x1, y1 + h, x2, y2, x2, y2 + h, sec->rim_colour);
        LbDrawTriangleFilled(ox, oy, x2, y2, x1, y1, sec->top_colour);
        LbDrawLine(x1 - 1, y1, x2 - 1, y2, sec->edge_colour);
        if (!(lbSinTable[ang & 0x7ff] > 0 || lbSinTable[(ang - step) & 0x7ff] > 0))
            LbDrawLine(x1 - 1, y1 + h, x2 - 1, y2 + h, sec->edge_colour);
        if (mouse_in_slice(ox, oy, x1, y1, x2, y2, h))
            pie_sector_return = i;
    }
    if (side_off != 1) {
        LbDrawLine(ox - 1, oy, sx - 1, sy, sec->edge_colour);
        if (lbSinTable[(start & 0x7ff) + 0x200] >= 0) {
            LbDrawLine(ox - 1, oy + h, sx - 1, sy + h, sec->edge_colour);
            LbDrawLine(ox - 1, oy, ox - 1, oy + h, sec->edge_colour);
            LbDrawLine(sx - 1, sy, sx - 1, sy + h, sec->edge_colour);
        }
    }
    if (side_off != 2) {
        LbDrawLine(ox - 1, oy, ex - 1, ey, sec->edge_colour);
        if (lbSinTable[(end & 0x7ff) + 0x200] <= 0) {
            LbDrawLine(ox - 1, oy + h, ex - 1, ey + h, sec->edge_colour);
            LbDrawLine(ox - 1, oy, ox - 1, oy + h, sec->edge_colour);
            LbDrawLine(ex - 1, ey, ex - 1, ey + h, sec->edge_colour);
        }
    }
}

// Returns 1 if sector a lies further back than sector b, so that it must be
// drawn first.
// FUNCTION: 0x000b852c
s32 sector_behind(Sector *sectors, s32 a, s32 b)
{
    s32 as;
    s32 ae;
    s32 bs;
    s32 be;
    s32 asin;
    s32 bsin;

    as = lbSinTable[(sectors[a].start & 0x7ff) + 0x200];
    ae = lbSinTable[(sectors[a].end & 0x7ff) + 0x200];
    bs = lbSinTable[(sectors[b].start & 0x7ff) + 0x200];
    be = lbSinTable[(sectors[b].end & 0x7ff) + 0x200];
    if (!(as > 0 || ae < 0))
        return 0;
    if (!(ae > 0 || as < 0))
        return 1;
    if (!(bs > 0 || be < 0))
        return 1;
    if (!(be > 0 || bs < 0))
        return 0;
    if (as < 0 && bs > 0)
        return 0;
    if (as > 0 && bs < 0)
        return 0;
    asin = -lbSinTable[sectors[a].start & 0x7ff];
    bsin = -lbSinTable[sectors[b].start & 0x7ff];
    return asin < bsin ? 1 : 0;
}

// Draws a 3D pie chart of count sectors (at most 0x101) back to front,
// splitting a sector over half the pie in two so that it overlaps correctly.
// Sets pie_sector_return to the sector under the mouse, or -1.
// FUNCTION: 0x000b86a8
void pie_render(s32 cx, s32 cy, s32 rx, s32 ry, s32 h, s32 count, Sector *sectors)
{
    s32 start_half_first;
    s32 oy;
    s32 ox;
    Sector copy;
    s32 span;
    s32 split;
    s32 i;
    s32 j;

    if (count > 0x101)
        return;
    pie_sector_return = -1;
    split = -1;
    for (i = 0; i < count; i++) {
        span = sectors[i].end - sectors[i].start;
        if (span < 0)
            span += 0x800;
        if (span > 0x400) {
            if (lbSinTable[(sectors[i].start & 0x7ff) + 0x200] < 0 && lbSinTable[(sectors[i].end & 0x7ff) + 0x200] > 0) {
            } else {
                Sector *sec;
                s32 mid;
                s32 text_radius;

                sec = &sectors[i];
                mid = sectors[i].start + (span >> 1);
                ox = cx + (fixmul(lbSinTable[(mid & 0x7ff) + 0x200], rx * sec->radius) >> 8);
                oy = cy - (fixmul(lbSinTable[mid & 0x7ff], ry * sec->radius) >> 8);
                text_radius = sec->text_offset + sec->radius;
                sec->text_x = cx + (fixmul(lbSinTable[(mid & 0x7ff) + 0x200], rx * text_radius) >> 8);
                sec->text_y = cy - (fixmul(lbSinTable[mid & 0x7ff], ry * text_radius) >> 8);
                sec->text_right = lbSinTable[(mid & 0x7ff) + 0x200] >= 0 ? 1 : 0;
                split = i;
                lib_memcpy(&copy, &sectors[split], 0x21);
                sectors[split].radius = 0;
                start_half_first = lbSinTable[(sectors[i].start & 0x7ff) + 0x200] >= 0 || lbSinTable[(mid & 0x7ff) + 0x200] <= 0;
                if (start_half_first) {
                    sectors[i].end = mid;
                    side_off = 2;
                    sector_render(ox, oy, rx, ry, h, sectors, i);
                    sectors[i].end = copy.end;
                    sectors[i].start = mid;
                } else {
                    sectors[i].start = mid;
                    side_off = 1;
                    sector_render(ox, oy, rx, ry, h, sectors, i);
                    sectors[i].start = copy.start;
                    sectors[i].end = mid;
                }
            }
            break;
        }
    }
    memset(done, 0, 0x101);
    j = count;
    while (--j >= 0) {
        s32 best;
        s32 k;

        best = -1;
        for (k = 0; k < count; k++) {
            if (done[k] == 0) {
                if (best == -1 || sector_behind(sectors, k, best))
                    best = k;
            }
        }
        done[best] = 1;
        if (best == split) {
            side_off = start_half_first ? 1 : 2;
            sector_render(ox, oy, rx, ry, h, sectors, best);
        } else {
            side_off = 0;
            sector_render(cx, cy, rx, ry, h, sectors, best);
        }
    }
    if (split != -1)
        lib_memcpy(&sectors[split], &copy, 0x21);
}
