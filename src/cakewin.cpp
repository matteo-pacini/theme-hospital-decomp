#include "hosp.h"

struct TrackSlot {
    u8 kind;
    u8 unknown1[0x1];
    u16 value;
    u8 unknown4[0x7];
};

struct ScrollLayout {
    u8 unknown0[0x558];
    s32 unknown558;
    s32 unknown55c;
};

extern "C" {
extern TrackSlot D_001581b2[];
extern u8 D_00156910;
}

extern s16 mapx;
extern s16 mapy;
extern s16 curx;
extern s16 cury;
extern s16 mcurx;
extern s16 mcury;

void give_absolute_xy_of_cell_at_column_row(const s32 x, const s32 y);
u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 n);
void set_window_animating(const u8 n, const u8 a, s32 b, const s32 x0, const s32 y0, const s32 x1, const s32 y1);
void animate_single_window(const u8 n);
void remove_window_from_list(const u8 n);
void copy_window_from_a_to_b(const u8 a, const u8 b);
void remove_window_from_screen(s32 x, s32 y, s32 w, s32 h);
void find_biggest_window(s32 x0, s32 y0, s32 x1, s32 y1, const s32 wx0, const s32 wy0, const s32 wx1, const s32 wy1);

// FUNCTION: 0x000ab610
void process_tracking_windows(void)
{
    u8 i;
    Thing *w;

    for (i = 0; i < 4; i++) {
        if (othervar.f34[i]) {
            w = &thing[D_001581b2[i].value];
            give_absolute_xy_of_cell_at_column_row(w->x.tile, w->y.tile);
            if (!check_if_point_beyond_scroll_limit(
                    info.unknown54 - resvars.f1c_ScrollLayoutp->unknown558
                        + (resvars.f0 == SCREEN_MODE_13 ? ((w->x.frac - w->y.frac) >> 3) + 32 : ((w->x.frac - w->y.frac) >> 4) + 16),
                    (resvars.f0 == SCREEN_MODE_13 ? (w->y.frac + w->x.frac) >> 4 : (w->y.frac + w->x.frac) >> 5)
                        + (info.unknown58 - resvars.f1c_ScrollLayoutp->unknown55c),
                    1)) {
                window.f0[othervar.f34[i]] = (resvars.f0 == SCREEN_MODE_13 ? ((w->x.frac - w->y.frac) >> 3) + 32 : ((w->x.frac - w->y.frac) >> 4) + 16)
                    + (info.unknown54 - resvars.f1c_ScrollLayoutp->unknown558);
                window.fc[othervar.f34[i]] = (resvars.f0 == SCREEN_MODE_13 ? (w->y.frac + w->x.frac) >> 4 : (w->y.frac + w->x.frac) >> 5)
                    + (info.unknown58 - resvars.f1c_ScrollLayoutp->unknown55c);
            } else {
                close_irequesters();
            }
            othervar.f34[i] = 0;
        }
    }
    if (othervar.f38) {
        w = &thing[D_001581b2[othervar.f38].value];
        give_absolute_xy_of_cell_at_column_row(w->x.tile, w->y.tile);
        if (!check_if_point_beyond_scroll_limit(
                info.unknown54 - resvars.f1c_ScrollLayoutp->unknown558
                    + (resvars.f0 == SCREEN_MODE_13 ? ((w->x.frac - w->y.frac) >> 3) + 32 : ((w->x.frac - w->y.frac) >> 4) + 16),
                (resvars.f0 == SCREEN_MODE_13 ? (w->y.frac + w->x.frac) >> 4 : (w->y.frac + w->x.frac) >> 5)
                    + (info.unknown58 - resvars.f1c_ScrollLayoutp->unknown558),
                0)) {
            window.f0[0] = (resvars.f0 == SCREEN_MODE_13 ? ((w->x.frac - w->y.frac) >> 3) + 32 : ((w->x.frac - w->y.frac) >> 4) + 16)
                + (info.unknown54 - resvars.f1c_ScrollLayoutp->unknown558);
            window.fc[0] = (resvars.f0 == SCREEN_MODE_13 ? (w->y.frac + w->x.frac) >> 4 : (w->y.frac + w->x.frac) >> 5)
                + (info.unknown58 - resvars.f1c_ScrollLayoutp->unknown55c);
        } else {
            close_irequesters();
        }
        othervar.f38 = 0;
    }
}

// FUNCTION: 0x000aba50
void set_map_cur_and_mcur_x_and_y(void)
{
    s32 x;
    s32 y;

    find_cell_at_point(window.unknown2eb, window.zone_x[window.unknown2eb], window.zone_y[window.unknown2eb]);
    mapx = info.unknown5c;
    mapy = info.unknown60;
    find_cell_at_point(window.unknown2eb, lbDisplay.mmouse_x, lbDisplay.mmouse_y);
    curx = info.unknown5c - mapx;
    cury = info.unknown60 - mapy;
    mcurx = info.unknown54;
    mcury = info.unknown58;
    if (info.unknown5c < 0)
        x = 0;
    else if (info.unknown5c >= 128)
        x = 127;
    else
        x = info.unknown5c;
    if (info.unknown60 >= 128)
        y = 127;
    else if (info.unknown60 < 0)
        y = 0;
    else
        y = info.unknown60;
    othervar.f2a = othervar.f28;
    othervar.f28 = x + (y << 7);
}

// FUNCTION: 0x000abb80
void goto_cell_in_window(const u8 a, const s32 b)
{
    goto_column_row_in_window(a, b % 128, b / 128);
}

// FUNCTION: 0x000abbc8
void goto_column_row_in_window(const u8 a, const s32 x, const s32 y)
{
    if (x >= 0 && x < 128 && y >= 0 && y < 128) {
        give_absolute_xy_of_cell_at_column_row(x, y);
        window.f0[a] = info.unknown54;
        window.fc[a] = info.unknown58;
    }
}

// FUNCTION: 0x000abc44
void move_windows(void)
{
    u8 i;
    s32 ex;
    s32 ey;
    s32 dx;
    s32 dy;

    i = window.unknown2ec;
    if (!(scroll.unknown2 || scroll.unknown3 || scroll.unknown0 || scroll.unknown1)) {
        window.fa8[window.unknown2eb] = 0;
        window.fb4[window.unknown2eb] = 0;
    } else {
        if (scroll.unknown2 == 1)
            window.fa8[window.unknown2eb] -= 0x80000;
        if (scroll.unknown3 == 1)
            window.fa8[window.unknown2eb] += 0x80000;
        if (scroll.unknown0 == 1)
            window.fb4[window.unknown2eb] -= 0x40000;
        if (scroll.unknown1 == 1)
            window.fb4[window.unknown2eb] += 0x40000;
    }
    for (i = 0; i < window.unknown2ec; i++) {
        dx = window.fa8[i] >> 16;
        dy = window.fb4[i] >> 16;
        window.fc0[i] += window.fa8[i] - (dx << 16);
        window.fcc[i] += window.fb4[i] - (dy << 16);
        ex = window.fc0[i] >> 16;
        ey = window.fcc[i] >> 16;
        window.fc0[i] -= ex << 16;
        window.fcc[i] -= ey << 16;
        dx += ex;
        dy += ey;
        if (check_if_point_beyond_scroll_limit(window.f0[i] + dx, window.fc[i] + dy, 0)) {
            window.fa8[i] = 0;
            window.fb4[i] = 0;
            window.fc0[i] = 0;
            window.fcc[i] = 0;
            dx = 0;
            dy = 0;
        }
        if (resvars.f4 < dx)
            dx = resvars.f4;
        else if (-resvars.f4 > dx)
            dx = -resvars.f4;
        if (resvars.f3 < dy)
            dy = resvars.f3;
        else if (-resvars.f3 > dy)
            dy = -resvars.f3;
        window.f0[i] += dx;
        window.fc[i] += dy;
        if (window.fa8[i] >= 0)
            window.fa8[i] = (window.fa8[i] * 0xcd) >> 8;
        else
            window.fa8[i] = (window.fa8[i] * 0xcd) >> 8;
        if (window.fb4[i] >= 0)
            window.fb4[i] = (window.fb4[i] * 0xcd) >> 8;
        else
            window.fb4[i] = (window.fb4[i] * 0xcd) >> 8;
        window.f90[i] = (window.zone_x[i] + window.zone_w[i]) / 2;
        window.f9c[i] = (window.zone_y[i] + window.zone_h[i]) / 2;
    }
    refresh.f0 += window.f0[window.f2ea] - window.f18[window.f2ea];
    if (refresh.f0 >= lbDisplay.unknown18)
        refresh.f0 -= lbDisplay.unknown18;
    else if (refresh.f0 < 0)
        refresh.f0 += lbDisplay.unknown18;
    refresh.f2 += window.fc[window.f2ea] - window.f24[window.f2ea];
    if (refresh.f2 >= lbDisplay.unknown1c)
        refresh.f2 -= lbDisplay.unknown1c;
    else if (refresh.f2 < 0)
        refresh.f2 += lbDisplay.unknown1c;
    scroll.unknown2 = 0;
    scroll.unknown3 = 0;
    scroll.unknown0 = 0;
    scroll.unknown1 = 0;
    for (i = 0; i < window.unknown2ec; i++) {
        if (window.f0[i] != window.f18[i] || window.fc[i] != window.f24[i])
            window.fd8[i] = 1;
        if (window.zone_x[i] != window.f60[i] || window.zone_y[i] != window.f6c[i] || window.zone_w[i] != window.f78[i]
            || window.zone_h[i] != window.f84[i])
            window.fdb[i] = 1;
    }
}

// FUNCTION: 0x000ac264
void find_cell_at_point(const u8 a, const s32 x, const s32 y)
{
    s32 area;
    s32 u;
    s32 v;
    s32 row;
    s32 col;

    area = resvars.f4 * resvars.f3;
    u = ((x + window.f0[a] - window.f90[a]) << 8) * resvars.f3;
    v = ((y + window.fc[a] - window.f9c[a]) << 8) * resvars.f4;
    row = (v - u) / area >> 8;
    col = (u + v) / area >> 8;
    info.unknown64 = col + (row << 7);
    info.unknown5c = col;
    info.unknown60 = row;
    info.unknown54 = resvars.f4 * (col - row - 1) / 2 - window.f0[a] + window.f90[a];
    info.unknown58 = resvars.f3 * (col + row + 2) / 2 - window.fc[a] + window.f9c[a];
}

// FUNCTION: 0x000ac3c4
void give_screen_xy_of_cell_at_column_row(const u8 a, const s32 x, const s32 y)
{
    info.unknown54 = resvars.f4 * (x - y - 1) / 2 - window.f0[a] + window.f90[a];
    info.unknown58 = (x + y + 2) * resvars.f3 / 2 - window.fc[a] + window.f9c[a];
}

// FUNCTION: 0x000ac464
void give_absolute_xy_of_cell_at_column_row(const s32 x, const s32 y)
{
    info.unknown54 = (x - y) * resvars.f4 / 2;
    info.unknown58 = (x + y) * resvars.f3 / 2;
}

// FUNCTION: 0x000ac4bc
u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 n)
{
    s32 area;
    s32 u;
    s32 v;
    s32 col;
    s32 row;
    s32 margin;

    area = resvars.f4 * resvars.f3;
    u = (x << 8) * resvars.f3;
    v = (y << 8) * resvars.f4;
    col = (u + v) / area >> 8;
    row = (v - u) / area >> 8;
    margin = 0;
    if (n)
        margin = 10;
    if (col + margin < gamevar.f132 % 128 || col - margin >= gamevar.f134 % 128)
        return 1;
    if (row + margin < gamevar.f132 / 128 || row - margin >= gamevar.f134 / 128)
        return 1;
    return 0;
}

// FUNCTION: 0x000ac5e4
void find_common_window(s32 x0, s32 y0, s32 x1, s32 y1, const s32 wx0, const s32 wy0, const s32 wx1, const s32 wy1)
{
    if (x0 > wx1)
        x0 = wx1;
    else if (x0 < wx0)
        x0 = wx0;
    if (x1 > wx1)
        x1 = wx1;
    else if (x1 < wx0)
        x1 = wx0;
    if (y0 > wy1)
        y0 = wy1;
    else if (y0 < wy0)
        y0 = wy0;
    if (y1 > wy1)
        y1 = wy1;
    else if (y1 < wy0)
        y1 = wy0;
    info.unknown68 = x0;
    info.unknown6c = y0;
    info.unknown70 = x1;
    info.unknown74 = y1;
    if (x0 == x1 || y0 == y1)
        info.unknown78 = 1;
    else
        info.unknown78 = 0;
}

// FUNCTION: 0x000ac6bc
void find_biggest_window(s32 x0, s32 y0, s32 x1, s32 y1, const s32 wx0, const s32 wy0, const s32 wx1, const s32 wy1)
{
    if (x0 > wx0)
        x0 = wx0;
    if (x0 > wx1)
        x0 = wx1;
    if (y0 > wy0)
        y0 = wy0;
    if (y0 > wy1)
        y0 = wy1;
    if (x1 < wx0)
        x1 = wx0;
    if (x1 < wx1)
        x1 = wx1;
    if (y1 < wy0)
        y1 = wy0;
    if (y1 < wy1)
        y1 = wy1;
    info.unknown68 = x0;
    info.unknown6c = y0;
    info.unknown70 = x1;
    info.unknown74 = y1;
    if (x0 == x1 || y0 == y1)
        info.unknown78 = 1;
    else
        info.unknown78 = 0;
}

// FUNCTION: 0x000ac78c
void open_new_window(const u8 a, const u8 b, const u8 c, const s32 x0, const s32 y0, const s32 x1, const s32 y1)
{
    s32 t0;
    s32 t1;
    s32 t2;
    s32 t3;

    if (window.unknown2ec < 2) {
        window.unknown2ec++;
        window.f2e7 = 0;
        window.f2e8 = 0;
        window.f60[window.unknown2ec - 1] = 0;
        window.f6c[window.unknown2ec - 1] = 0;
        window.f78[window.unknown2ec - 1] = 0;
        window.f84[window.unknown2ec - 1] = 0;
        window.zone_x[window.unknown2ec - 1] = lbDisplay.mmouse_x - 5;
        window.zone_w[window.unknown2ec - 1] = lbDisplay.mmouse_x + 5;
        window.zone_y[window.unknown2ec - 1] = lbDisplay.mmouse_y - 5;
        window.zone_h[window.unknown2ec - 1] = lbDisplay.mmouse_y + 5;
        window.f90[window.unknown2ec - 1] = (window.zone_x[window.unknown2ec - 1] + window.zone_w[window.unknown2ec - 1]) / 2;
        window.f9c[window.unknown2ec - 1] = (window.zone_y[window.unknown2ec - 1] + window.zone_h[window.unknown2ec - 1]) / 2;
        window.f0[window.unknown2ec - 1] = window.f0[window.unknown2eb];
        window.fc[window.unknown2ec - 1] = window.fc[window.unknown2eb];
        window.fde[window.unknown2ec - 1] = a;
        window.f2c0[window.unknown2ec - 1] = 1;
        if (window.unknown2ed == 0)
            window.f224[window.unknown2ec - 1] = 12;
        else if (window.unknown2ed == 1)
            window.f224[window.unknown2ec - 1] = 0;
        window.f230[window.unknown2ec - 1] = window.zone_x[window.unknown2ec - 1];
        window.f23c[window.unknown2ec - 1] = window.zone_y[window.unknown2ec - 1];
        window.f248[window.unknown2ec - 1] = window.zone_w[window.unknown2ec - 1];
        window.f254[window.unknown2ec - 1] = window.zone_h[window.unknown2ec - 1];
        if (b == 0)
            set_window_animating(window.unknown2ec - 1, window.f2c0[window.unknown2ec - 1], window.f224[window.unknown2ec - 1],
                                 x0, y0, x1, y1);
        else if (b == 1)
            set_window_animating(window.unknown2ec - 1, window.f2c0[window.unknown2ec - 1], window.f224[window.unknown2ec - 1],
                                 window.fe4[c], window.f134[c], window.f184[c], window.f1d4[c]);
        if (window.unknown2ed == 1) {
            while (window.f224[window.unknown2ec - 1] > 0) {
                t0 = window.zone_x[window.unknown2ec - 1];
                t1 = window.zone_y[window.unknown2ec - 1];
                t2 = window.zone_w[window.unknown2ec - 1];
                t3 = window.zone_h[window.unknown2ec - 1];
                animate_single_window(window.unknown2ec - 1);
                find_biggest_window(window.zone_x[window.unknown2ec - 1], window.zone_y[window.unknown2ec - 1],
                                    window.zone_w[window.unknown2ec - 1], window.zone_h[window.unknown2ec - 1], t0, t1, t2, t3);
                find_common_window(info.unknown68, info.unknown6c, info.unknown70, info.unknown74, window.unknown2c3,
                                   window.unknown2c7, window.unknown2cb, window.unknown2cf);
                t0 = info.unknown68;
                t1 = info.unknown6c;
                t2 = info.unknown70;
                t3 = info.unknown74;
            }
        }
        window.unknown2eb = window.unknown2ec - 1;
    }
}

// FUNCTION: 0x000acc4c
void set_window_animating(const u8 n, const u8 a, s32 steps, const s32 x0, const s32 y0, const s32 x1, const s32 y1)
{
    if (steps == 0)
        steps = 1;
    if (x0 < x1 && y0 < y1 && steps > 0) {
        window.f2c0[n] = a;
        window.f224[n] = steps + 1;
        window.f260[n] = x0;
        window.f26c[n] = y0;
        window.f278[n] = x1;
        window.f284[n] = y1;
        window.f290[n] = ((window.zone_x[n] - x0) << 16) / steps;
        window.f29c[n] = ((window.zone_y[n] - y0) << 16) / steps;
        window.f2a8[n] = ((window.zone_w[n] - x1) << 16) / steps;
        window.f2b4[n] = ((window.zone_h[n] - y1) << 16) / steps;
    }
}

// FUNCTION: 0x000acdb0
void animate_windows(void)
{
    u8 i;
    s32 w;
    s32 h;
    s32 x;
    s32 y;

    i = 0;
    while (i < window.unknown2ec) {
        if (window.f224[i] > 0) {
            window.f224[i]--;
            window.zone_x[i] = (window.f290[i] * window.f224[i] >> 16) + window.f260[i];
            window.zone_y[i] = (window.f29c[i] * window.f224[i] >> 16) + window.f26c[i];
            window.zone_w[i] = window.f278[i] + (window.f2a8[i] * window.f224[i] >> 16);
            window.zone_h[i] = window.f284[i] + (window.f2b4[i] * window.f224[i] >> 16);
            window.f90[i] = (window.zone_x[i] + window.zone_w[i]) / 2;
            window.f9c[i] = (window.zone_y[i] + window.zone_h[i]) / 2;
            i++;
        } else if (window.f2c0[i] == 2) {
            x = window.f60[i];
            y = window.f6c[i];
            w = window.f78[i];
            h = window.f84[i];
            remove_window_from_list(i);
            remove_window_from_screen(x, y, w, h);
            if (window.unknown2eb >= window.unknown2ec && window.unknown2eb > 0)
                window.unknown2eb--;
        } else {
            window.f2c0[i] = 0;
            i++;
        }
    }
}

// FUNCTION: 0x000ad034
void animate_single_window(const u8 n)
{
    s32 x;
    s32 y;
    s32 w;
    s32 h;

    if (window.f224[n] > 0) {
        window.f224[n]--;
        window.zone_x[n] = (window.f290[n] * window.f224[n] >> 16) + window.f260[n];
        window.zone_y[n] = (window.f29c[n] * window.f224[n] >> 16) + window.f26c[n];
        window.zone_w[n] = window.f278[n] + (window.f2a8[n] * window.f224[n] >> 16);
        window.zone_h[n] = window.f284[n] + (window.f2b4[n] * window.f224[n] >> 16);
        window.f90[n] = (window.zone_x[n] + window.zone_w[n]) / 2;
        window.f9c[n] = (window.zone_y[n] + window.zone_h[n]) / 2;
    } else if (window.f2c0[n] == 2) {
        x = window.zone_x[n];
        y = window.zone_y[n];
        w = window.zone_w[n];
        h = window.zone_h[n];
        window.f2c0[n] = 0;
        remove_window_from_list(n);
        if ((int)window.unknown2eb > (int)n)
            window.unknown2eb--;
    } else {
        window.f2c0[n] = 0;
    }
}

// FUNCTION: 0x000ad284
void copy_window_from_a_to_b(const u8 a, const u8 b)
{
    window.f90[b] = window.f90[a];
    window.f9c[b] = window.f9c[a];
    window.zone_x[b] = window.zone_x[a];
    window.zone_w[b] = window.zone_w[a];
    window.zone_y[b] = window.zone_y[a];
    window.zone_h[b] = window.zone_h[a];
    window.f60[b] = window.f60[a];
    window.f6c[b] = window.f6c[a];
    window.f78[b] = window.f78[a];
    window.f84[b] = window.f84[a];
    window.fa8[b] = window.fa8[a];
    window.fb4[b] = window.fb4[a];
    window.fc0[b] = window.fc0[a];
    window.fcc[b] = window.fcc[a];
    window.f0[b] = window.f0[a];
    window.fc[b] = window.fc[a];
    window.f18[b] = window.f18[a];
    window.f24[b] = window.f24[a];
    window.fde[b] = window.fde[a];
    window.f230[b] = window.f230[a];
    window.f23c[b] = window.f23c[a];
    window.f248[b] = window.f248[a];
    window.f254[b] = window.f254[a];
    window.f260[b] = window.f260[a];
    window.f26c[b] = window.f26c[a];
    window.f278[b] = window.f278[a];
    window.f284[b] = window.f284[a];
    window.f290[b] = window.f290[a];
    window.f29c[b] = window.f29c[a];
    window.f2a8[b] = window.f2a8[a];
    window.f2b4[b] = window.f2b4[a];
    window.f2c0[b] = window.f2c0[a];
    window.f224[b] = window.f224[a];
}

// FUNCTION: 0x000ad6cc
void remove_window_from_list(const u8 n)
{
    u8 i;

    window.unknown2ec--;
    if ((int)n < (int)window.unknown2ec) {
        for (i = n; i < window.unknown2ec; i++)
            copy_window_from_a_to_b(i + 1, i);
    }
}

// FUNCTION: 0x000ad72c
void init_andys_variables(void)
{
    u8 i;

    i = 0;
    set_all_refresh_zones_in_window(0, 0, 0x27f, 0x1df, 3);
    D_00156910 = 1;
    debug.f102 = 1;
    debug.f104 = 0;
    debug.f103 = 0;
    debug.f105 = 0;
    debug.f106 = 0;
    window.f0[0] = 0;
    window.fc[0] = 1000;
    window.f18[0] = window.f0[0];
    window.f24[0] = window.fc[0];
    window.f60[0] = 0;
    window.f6c[0] = 0;
    window.f78[0] = 0;
    window.f84[0] = 0;
    window.fa8[0] = 0;
    window.fb4[0] = 0;
    window.fa8[1] = 0;
    window.fb4[1] = 0;
    window.fc0[0] = 0;
    window.fcc[0] = 0;
    window.fde[0] = 0;
    window.f230[0] = 0x13b;
    window.f23c[0] = 0xeb;
    window.f248[0] = 0x145;
    window.f254[0] = 0xf5;
    window.f2c0[0] = 0;
    window.f2e7 = 0;
    window.f2e8 = 0;
    window.f2e9 = 0;
    refresh.f0 = 0;
    refresh.f2 = 0;
    window.unknown2eb = 0;
    window.unknown2ec = 1;
    window.f2ea = 0;
    window.f2c0[0] = 0;
    window.f2c0[1] = 0;
    for (; i < 4; i++) {
        window.fe4[i * 4] = 0x23;
        window.f134[i * 4] = i * 0x5a + 0x28;
        window.f184[i * 4] = 0xa5;
        window.f1d4[i * 4] = i * 0x5a + 0x6e;
        window.fe4[i * 4 + 1] = 0xb9;
        window.f134[i * 4 + 1] = i * 0x5a + 0x28;
        window.f184[i * 4 + 1] = 0x13b;
        window.f1d4[i * 4 + 1] = i * 0x5a + 0x6e;
        window.fe4[i * 4 + 2] = 0x14f;
        window.f134[i * 4 + 2] = i * 0x5a + 0x28;
        window.f184[i * 4 + 2] = 0x1d1;
        window.f1d4[i * 4 + 2] = i * 0x5a + 0x6e;
        window.fe4[i * 4 + 3] = 0x1e5;
        window.f134[i * 4 + 3] = i * 0x5a + 0x28;
        window.f184[i * 4 + 3] = 0x267;
        window.f1d4[i * 4 + 3] = i * 0x5a + 0x6e;
    }
    window.unknown2ed = 1;
    window.zone_x[0] = 0;
    window.zone_w[0] = lbDisplay.unknown18;
    window.zone_y[0] = 0;
    window.zone_h[0] = statvar.unknown4;
    window.f90[0] = (window.zone_x[0] + window.zone_w[0]) / 2;
    window.f9c[0] = (window.zone_y[0] + window.zone_h[0]) / 2;
    debug.ffc = 0;
    debug.ffd = 0;
    debug.ffe = 0;
    debug.fff = 0;
    debug.f100 = 0;
    debug.f101 = 0;
}
