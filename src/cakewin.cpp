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
u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 with_margin);
void set_window_animating(const u8 win, const u8 a, s32 steps, const s32 x0, const s32 y0, const s32 x1, const s32 y1);
void animate_single_window(const u8 win);
void remove_window_from_list(const u8 win);
void copy_window_from_a_to_b(const u8 from, const u8 to);
void remove_window_from_screen(s32 x, s32 y, s32 w, s32 h);
void find_biggest_window(s32 x0, s32 y0, s32 x1, s32 y1, const s32 wx0, const s32 wy0, const s32 wx1, const s32 wy1);

// FUNCTION: 0x000ab610
void process_tracking_windows(void)
{
    u8 i;
    Thing *tracked;

    for (i = 0; i < 4; i++) {
        if (othervar.f34[i]) {
            tracked = &thing[D_001581b2[i].value];
            give_absolute_xy_of_cell_at_column_row(tracked->x.tile, tracked->y.tile);
            if (!check_if_point_beyond_scroll_limit(
                    info.unknown54 - resvars.f1c_ScrollLayoutp->unknown558
                        + (resvars.f0 == SCREEN_MODE_13 ? ((tracked->x.frac - tracked->y.frac) >> 3) + 32 : ((tracked->x.frac - tracked->y.frac) >> 4) + 16),
                    (resvars.f0 == SCREEN_MODE_13 ? (tracked->y.frac + tracked->x.frac) >> 4 : (tracked->y.frac + tracked->x.frac) >> 5)
                        + (info.unknown58 - resvars.f1c_ScrollLayoutp->unknown55c),
                    1)) {
                window.f0[othervar.f34[i]] = (resvars.f0 == SCREEN_MODE_13 ? ((tracked->x.frac - tracked->y.frac) >> 3) + 32 : ((tracked->x.frac - tracked->y.frac) >> 4) + 16)
                    + (info.unknown54 - resvars.f1c_ScrollLayoutp->unknown558);
                window.fc[othervar.f34[i]] = (resvars.f0 == SCREEN_MODE_13 ? (tracked->y.frac + tracked->x.frac) >> 4 : (tracked->y.frac + tracked->x.frac) >> 5)
                    + (info.unknown58 - resvars.f1c_ScrollLayoutp->unknown55c);
            } else {
                close_irequesters();
            }
            othervar.f34[i] = 0;
        }
    }
    if (othervar.f38) {
        tracked = &thing[D_001581b2[othervar.f38].value];
        give_absolute_xy_of_cell_at_column_row(tracked->x.tile, tracked->y.tile);
        if (!check_if_point_beyond_scroll_limit(
                info.unknown54 - resvars.f1c_ScrollLayoutp->unknown558
                    + (resvars.f0 == SCREEN_MODE_13 ? ((tracked->x.frac - tracked->y.frac) >> 3) + 32 : ((tracked->x.frac - tracked->y.frac) >> 4) + 16),
                (resvars.f0 == SCREEN_MODE_13 ? (tracked->y.frac + tracked->x.frac) >> 4 : (tracked->y.frac + tracked->x.frac) >> 5)
                    + (info.unknown58 - resvars.f1c_ScrollLayoutp->unknown558),
                0)) {
            window.f0[0] = (resvars.f0 == SCREEN_MODE_13 ? ((tracked->x.frac - tracked->y.frac) >> 3) + 32 : ((tracked->x.frac - tracked->y.frac) >> 4) + 16)
                + (info.unknown54 - resvars.f1c_ScrollLayoutp->unknown558);
            window.fc[0] = (resvars.f0 == SCREEN_MODE_13 ? (tracked->y.frac + tracked->x.frac) >> 4 : (tracked->y.frac + tracked->x.frac) >> 5)
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
void goto_cell_in_window(const u8 win, const s32 cell)
{
    goto_column_row_in_window(win, cell % 128, cell / 128);
}

// FUNCTION: 0x000abbc8
void goto_column_row_in_window(const u8 win, const s32 x, const s32 y)
{
    if (x >= 0 && x < 128 && y >= 0 && y < 128) {
        give_absolute_xy_of_cell_at_column_row(x, y);
        window.f0[win] = info.unknown54;
        window.fc[win] = info.unknown58;
    }
}

// FUNCTION: 0x000abc44
void move_windows(void)
{
    u8 i;
    s32 carry_x;
    s32 carry_y;
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
        carry_x = window.fc0[i] >> 16;
        carry_y = window.fcc[i] >> 16;
        window.fc0[i] -= carry_x << 16;
        window.fcc[i] -= carry_y << 16;
        dx += carry_x;
        dy += carry_y;
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
void find_cell_at_point(const u8 win, const s32 x, const s32 y)
{
    s32 area;
    s32 u;
    s32 v;
    s32 row;
    s32 col;

    area = resvars.f4 * resvars.f3;
    u = ((x + window.f0[win] - window.f90[win]) << 8) * resvars.f3;
    v = ((y + window.fc[win] - window.f9c[win]) << 8) * resvars.f4;
    row = (v - u) / area >> 8;
    col = (u + v) / area >> 8;
    info.unknown64 = col + (row << 7);
    info.unknown5c = col;
    info.unknown60 = row;
    info.unknown54 = resvars.f4 * (col - row - 1) / 2 - window.f0[win] + window.f90[win];
    info.unknown58 = resvars.f3 * (col + row + 2) / 2 - window.fc[win] + window.f9c[win];
}

// FUNCTION: 0x000ac3c4
void give_screen_xy_of_cell_at_column_row(const u8 win, const s32 x, const s32 y)
{
    info.unknown54 = resvars.f4 * (x - y - 1) / 2 - window.f0[win] + window.f90[win];
    info.unknown58 = (x + y + 2) * resvars.f3 / 2 - window.fc[win] + window.f9c[win];
}

// FUNCTION: 0x000ac464
void give_absolute_xy_of_cell_at_column_row(const s32 x, const s32 y)
{
    info.unknown54 = (x - y) * resvars.f4 / 2;
    info.unknown58 = (x + y) * resvars.f3 / 2;
}

// FUNCTION: 0x000ac4bc
u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 with_margin)
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
    if (with_margin)
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
void open_new_window(const u8 a, const u8 source, const u8 preset, const s32 x0, const s32 y0, const s32 x1, const s32 y1)
{
    s32 old_x0;
    s32 old_y0;
    s32 old_x1;
    s32 old_y1;

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
        if (source == 0)
            set_window_animating(window.unknown2ec - 1, window.f2c0[window.unknown2ec - 1], window.f224[window.unknown2ec - 1],
                                 x0, y0, x1, y1);
        else if (source == 1)
            set_window_animating(window.unknown2ec - 1, window.f2c0[window.unknown2ec - 1], window.f224[window.unknown2ec - 1],
                                 window.fe4[preset], window.f134[preset], window.f184[preset], window.f1d4[preset]);
        if (window.unknown2ed == 1) {
            while (window.f224[window.unknown2ec - 1] > 0) {
                old_x0 = window.zone_x[window.unknown2ec - 1];
                old_y0 = window.zone_y[window.unknown2ec - 1];
                old_x1 = window.zone_w[window.unknown2ec - 1];
                old_y1 = window.zone_h[window.unknown2ec - 1];
                animate_single_window(window.unknown2ec - 1);
                find_biggest_window(window.zone_x[window.unknown2ec - 1], window.zone_y[window.unknown2ec - 1],
                                    window.zone_w[window.unknown2ec - 1], window.zone_h[window.unknown2ec - 1], old_x0, old_y0, old_x1, old_y1);
                find_common_window(info.unknown68, info.unknown6c, info.unknown70, info.unknown74, window.unknown2c3,
                                   window.unknown2c7, window.unknown2cb, window.unknown2cf);
                old_x0 = info.unknown68;
                old_y0 = info.unknown6c;
                old_x1 = info.unknown70;
                old_y1 = info.unknown74;
            }
        }
        window.unknown2eb = window.unknown2ec - 1;
    }
}

// FUNCTION: 0x000acc4c
void set_window_animating(const u8 win, const u8 mode, s32 steps, const s32 x0, const s32 y0, const s32 x1, const s32 y1)
{
    if (steps == 0)
        steps = 1;
    if (x0 < x1 && y0 < y1 && steps > 0) {
        window.f2c0[win] = mode;
        window.f224[win] = steps + 1;
        window.f260[win] = x0;
        window.f26c[win] = y0;
        window.f278[win] = x1;
        window.f284[win] = y1;
        window.f290[win] = ((window.zone_x[win] - x0) << 16) / steps;
        window.f29c[win] = ((window.zone_y[win] - y0) << 16) / steps;
        window.f2a8[win] = ((window.zone_w[win] - x1) << 16) / steps;
        window.f2b4[win] = ((window.zone_h[win] - y1) << 16) / steps;
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
void animate_single_window(const u8 win)
{
    s32 x;
    s32 y;
    s32 w;
    s32 h;

    if (window.f224[win] > 0) {
        window.f224[win]--;
        window.zone_x[win] = (window.f290[win] * window.f224[win] >> 16) + window.f260[win];
        window.zone_y[win] = (window.f29c[win] * window.f224[win] >> 16) + window.f26c[win];
        window.zone_w[win] = window.f278[win] + (window.f2a8[win] * window.f224[win] >> 16);
        window.zone_h[win] = window.f284[win] + (window.f2b4[win] * window.f224[win] >> 16);
        window.f90[win] = (window.zone_x[win] + window.zone_w[win]) / 2;
        window.f9c[win] = (window.zone_y[win] + window.zone_h[win]) / 2;
    } else if (window.f2c0[win] == 2) {
        x = window.zone_x[win];
        y = window.zone_y[win];
        w = window.zone_w[win];
        h = window.zone_h[win];
        window.f2c0[win] = 0;
        remove_window_from_list(win);
        if ((int)window.unknown2eb > (int)win)
            window.unknown2eb--;
    } else {
        window.f2c0[win] = 0;
    }
}

// FUNCTION: 0x000ad284
void copy_window_from_a_to_b(const u8 from, const u8 to)
{
    window.f90[to] = window.f90[from];
    window.f9c[to] = window.f9c[from];
    window.zone_x[to] = window.zone_x[from];
    window.zone_w[to] = window.zone_w[from];
    window.zone_y[to] = window.zone_y[from];
    window.zone_h[to] = window.zone_h[from];
    window.f60[to] = window.f60[from];
    window.f6c[to] = window.f6c[from];
    window.f78[to] = window.f78[from];
    window.f84[to] = window.f84[from];
    window.fa8[to] = window.fa8[from];
    window.fb4[to] = window.fb4[from];
    window.fc0[to] = window.fc0[from];
    window.fcc[to] = window.fcc[from];
    window.f0[to] = window.f0[from];
    window.fc[to] = window.fc[from];
    window.f18[to] = window.f18[from];
    window.f24[to] = window.f24[from];
    window.fde[to] = window.fde[from];
    window.f230[to] = window.f230[from];
    window.f23c[to] = window.f23c[from];
    window.f248[to] = window.f248[from];
    window.f254[to] = window.f254[from];
    window.f260[to] = window.f260[from];
    window.f26c[to] = window.f26c[from];
    window.f278[to] = window.f278[from];
    window.f284[to] = window.f284[from];
    window.f290[to] = window.f290[from];
    window.f29c[to] = window.f29c[from];
    window.f2a8[to] = window.f2a8[from];
    window.f2b4[to] = window.f2b4[from];
    window.f2c0[to] = window.f2c0[from];
    window.f224[to] = window.f224[from];
}

// FUNCTION: 0x000ad6cc
void remove_window_from_list(const u8 win)
{
    u8 i;

    window.unknown2ec--;
    if ((int)win < (int)window.unknown2ec) {
        for (i = win; i < window.unknown2ec; i++)
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
