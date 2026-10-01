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

// Scrolls each view window that follows a thing (othervar.track_window[i], thing
// D_001581b2[i].value) and the main view (othervar.center_on_slot) to centre on that
// thing, once each; if the thing is beyond the scroll limit the requesters are
// closed instead. The main view's limit test uses the layout's x offset
// (unknown558) for y too.
// FUNCTION: 0x000ab610
void process_tracking_windows(void)
{
    u8 i;
    Thing *tracked;

    for (i = 0; i < 4; i++) {
        if (othervar.track_window[i]) {
            tracked = &thing[D_001581b2[i].value];
            give_absolute_xy_of_cell_at_column_row(tracked->x.tile, tracked->y.tile);
            if (!check_if_point_beyond_scroll_limit(
                    info.cell_x - resvars.coords_ScrollLayoutp->unknown558
                        + (resvars.screen_mode == SCREEN_MODE_13 ? ((tracked->x.frac - tracked->y.frac) >> 3) + 32 : ((tracked->x.frac - tracked->y.frac) >> 4) + 16),
                    (resvars.screen_mode == SCREEN_MODE_13 ? (tracked->y.frac + tracked->x.frac) >> 4 : (tracked->y.frac + tracked->x.frac) >> 5)
                        + (info.cell_y - resvars.coords_ScrollLayoutp->unknown55c),
                    1)) {
                window.scroll_x[othervar.track_window[i]] = (resvars.screen_mode == SCREEN_MODE_13 ? ((tracked->x.frac - tracked->y.frac) >> 3) + 32 : ((tracked->x.frac - tracked->y.frac) >> 4) + 16)
                    + (info.cell_x - resvars.coords_ScrollLayoutp->unknown558);
                window.scroll_y[othervar.track_window[i]] = (resvars.screen_mode == SCREEN_MODE_13 ? (tracked->y.frac + tracked->x.frac) >> 4 : (tracked->y.frac + tracked->x.frac) >> 5)
                    + (info.cell_y - resvars.coords_ScrollLayoutp->unknown55c);
            } else {
                close_irequesters();
            }
            othervar.track_window[i] = 0;
        }
    }
    if (othervar.center_on_slot) {
        tracked = &thing[D_001581b2[othervar.center_on_slot].value];
        give_absolute_xy_of_cell_at_column_row(tracked->x.tile, tracked->y.tile);
        if (!check_if_point_beyond_scroll_limit(
                info.cell_x - resvars.coords_ScrollLayoutp->unknown558
                    + (resvars.screen_mode == SCREEN_MODE_13 ? ((tracked->x.frac - tracked->y.frac) >> 3) + 32 : ((tracked->x.frac - tracked->y.frac) >> 4) + 16),
                (resvars.screen_mode == SCREEN_MODE_13 ? (tracked->y.frac + tracked->x.frac) >> 4 : (tracked->y.frac + tracked->x.frac) >> 5)
                    + (info.cell_y - resvars.coords_ScrollLayoutp->unknown558),
                0)) {
            window.scroll_x[0] = (resvars.screen_mode == SCREEN_MODE_13 ? ((tracked->x.frac - tracked->y.frac) >> 3) + 32 : ((tracked->x.frac - tracked->y.frac) >> 4) + 16)
                + (info.cell_x - resvars.coords_ScrollLayoutp->unknown558);
            window.scroll_y[0] = (resvars.screen_mode == SCREEN_MODE_13 ? (tracked->y.frac + tracked->x.frac) >> 4 : (tracked->y.frac + tracked->x.frac) >> 5)
                + (info.cell_y - resvars.coords_ScrollLayoutp->unknown55c);
        } else {
            close_irequesters();
        }
        othervar.center_on_slot = 0;
    }
}

// Updates the map cursor from the mouse in the current view: mapx,mapy (the
// cell at the view's top left), curx,cury (the mouse cell relative to it),
// mcurx,mcury (that cell's screen position), and othervar.mouse_cell (the mouse
// tile, clamped to the map; the previous one goes to othervar.prev_mouse_cell).
// FUNCTION: 0x000aba50
void set_map_cur_and_mcur_x_and_y(void)
{
    s32 x;
    s32 y;

    find_cell_at_point(window.current_window, window.zone_x[window.current_window], window.zone_y[window.current_window]);
    mapx = info.cell_col;
    mapy = info.cell_row;
    find_cell_at_point(window.current_window, lbDisplay.mmouse_x, lbDisplay.mmouse_y);
    curx = info.cell_col - mapx;
    cury = info.cell_row - mapy;
    mcurx = info.cell_x;
    mcury = info.cell_y;
    if (info.cell_col < 0)
        x = 0;
    else if (info.cell_col >= 128)
        x = 127;
    else
        x = info.cell_col;
    if (info.cell_row >= 128)
        y = 127;
    else if (info.cell_row < 0)
        y = 0;
    else
        y = info.cell_row;
    othervar.prev_mouse_cell = othervar.mouse_cell;
    othervar.mouse_cell = x + (y << 7);
}

// Scrolls view window win to map tile cell (y * 128 + x).
// FUNCTION: 0x000abb80
void goto_cell_in_window(const u8 win, const s32 cell)
{
    goto_column_row_in_window(win, cell % 128, cell / 128);
}

// Scrolls view window win to map tile x,y; ignored if off the map.
// FUNCTION: 0x000abbc8
void goto_column_row_in_window(const u8 win, const s32 x, const s32 y)
{
    if (x >= 0 && x < 128 && y >= 0 && y < 128) {
        give_absolute_xy_of_cell_at_column_row(x, y);
        window.scroll_x[win] = info.cell_x;
        window.scroll_y[win] = info.cell_y;
    }
}

// Applies a frame of scrolling: the scroll keys accelerate the current view,
// every view moves by its velocity (in 1/65536 pixel, at most one tile a
// frame, stopped at the scroll limit) which then decays, the background
// buffer's wrap offset follows the drawn view, and moved or resized views are
// flagged for redrawing (window.scrolled, fdb). Clears the scroll keys.
// FUNCTION: 0x000abc44
void move_windows(void)
{
    u8 i;
    s32 carry_x;
    s32 carry_y;
    s32 dx;
    s32 dy;

    i = window.window_count;
    if (!(scroll.left || scroll.right || scroll.up || scroll.down)) {
        window.scroll_vel_x[window.current_window] = 0;
        window.scroll_vel_y[window.current_window] = 0;
    } else {
        if (scroll.left == 1)
            window.scroll_vel_x[window.current_window] -= 0x80000;
        if (scroll.right == 1)
            window.scroll_vel_x[window.current_window] += 0x80000;
        if (scroll.up == 1)
            window.scroll_vel_y[window.current_window] -= 0x40000;
        if (scroll.down == 1)
            window.scroll_vel_y[window.current_window] += 0x40000;
    }
    for (i = 0; i < window.window_count; i++) {
        dx = window.scroll_vel_x[i] >> 16;
        dy = window.scroll_vel_y[i] >> 16;
        window.scroll_frac_x[i] += window.scroll_vel_x[i] - (dx << 16);
        window.scroll_frac_y[i] += window.scroll_vel_y[i] - (dy << 16);
        carry_x = window.scroll_frac_x[i] >> 16;
        carry_y = window.scroll_frac_y[i] >> 16;
        window.scroll_frac_x[i] -= carry_x << 16;
        window.scroll_frac_y[i] -= carry_y << 16;
        dx += carry_x;
        dy += carry_y;
        if (check_if_point_beyond_scroll_limit(window.scroll_x[i] + dx, window.scroll_y[i] + dy, 0)) {
            window.scroll_vel_x[i] = 0;
            window.scroll_vel_y[i] = 0;
            window.scroll_frac_x[i] = 0;
            window.scroll_frac_y[i] = 0;
            dx = 0;
            dy = 0;
        }
        if (resvars.tile_width < dx)
            dx = resvars.tile_width;
        else if (-resvars.tile_width > dx)
            dx = -resvars.tile_width;
        if (resvars.tile_height < dy)
            dy = resvars.tile_height;
        else if (-resvars.tile_height > dy)
            dy = -resvars.tile_height;
        window.scroll_x[i] += dx;
        window.scroll_y[i] += dy;
        if (window.scroll_vel_x[i] >= 0)
            window.scroll_vel_x[i] = (window.scroll_vel_x[i] * 0xcd) >> 8;
        else
            window.scroll_vel_x[i] = (window.scroll_vel_x[i] * 0xcd) >> 8;
        if (window.scroll_vel_y[i] >= 0)
            window.scroll_vel_y[i] = (window.scroll_vel_y[i] * 0xcd) >> 8;
        else
            window.scroll_vel_y[i] = (window.scroll_vel_y[i] * 0xcd) >> 8;
        window.center_x[i] = (window.zone_x[i] + window.zone_x1[i]) / 2;
        window.center_y[i] = (window.zone_y[i] + window.zone_y1[i]) / 2;
    }
    refresh.scroll_x += window.scroll_x[window.bscreen_window] - window.prev_scroll_x[window.bscreen_window];
    if (refresh.scroll_x >= lbDisplay.physical_screen_width)
        refresh.scroll_x -= lbDisplay.physical_screen_width;
    else if (refresh.scroll_x < 0)
        refresh.scroll_x += lbDisplay.physical_screen_width;
    refresh.scroll_y += window.scroll_y[window.bscreen_window] - window.prev_scroll_y[window.bscreen_window];
    if (refresh.scroll_y >= lbDisplay.physical_screen_height)
        refresh.scroll_y -= lbDisplay.physical_screen_height;
    else if (refresh.scroll_y < 0)
        refresh.scroll_y += lbDisplay.physical_screen_height;
    scroll.left = 0;
    scroll.right = 0;
    scroll.up = 0;
    scroll.down = 0;
    for (i = 0; i < window.window_count; i++) {
        if (window.scroll_x[i] != window.prev_scroll_x[i] || window.scroll_y[i] != window.prev_scroll_y[i])
            window.scrolled[i] = 1;
        if (window.zone_x[i] != window.prev_zone_x[i] || window.zone_y[i] != window.prev_zone_y[i] || window.zone_x1[i] != window.prev_zone_x1[i]
            || window.zone_y1[i] != window.prev_zone_y1[i])
            window.zone_changed[i] = 1;
    }
}

// Finds the map cell under screen point x,y of view window win. Sets
// info.cell_col,unknown60 to its column and row (unclamped), unknown64 to
// row * 128 + column, and unknown54,unknown58 to its screen position.
// FUNCTION: 0x000ac264
void find_cell_at_point(const u8 win, const s32 x, const s32 y)
{
    s32 area;
    s32 u;
    s32 v;
    s32 row;
    s32 col;

    area = resvars.tile_width * resvars.tile_height;
    u = ((x + window.scroll_x[win] - window.center_x[win]) << 8) * resvars.tile_height;
    v = ((y + window.scroll_y[win] - window.center_y[win]) << 8) * resvars.tile_width;
    row = (v - u) / area >> 8;
    col = (u + v) / area >> 8;
    info.cell = col + (row << 7);
    info.cell_col = col;
    info.cell_row = row;
    info.cell_x = resvars.tile_width * (col - row - 1) / 2 - window.scroll_x[win] + window.center_x[win];
    info.cell_y = resvars.tile_height * (col + row + 2) / 2 - window.scroll_y[win] + window.center_y[win];
}

// Sets info.cell_x,unknown58 to the screen position in view window win of
// map cell column x, row y.
// FUNCTION: 0x000ac3c4
void give_screen_xy_of_cell_at_column_row(const u8 win, const s32 x, const s32 y)
{
    info.cell_x = resvars.tile_width * (x - y - 1) / 2 - window.scroll_x[win] + window.center_x[win];
    info.cell_y = (x + y + 2) * resvars.tile_height / 2 - window.scroll_y[win] + window.center_y[win];
}

// Sets info.cell_x,unknown58 to the map pixel position of cell column x,
// row y, before any scrolling.
// FUNCTION: 0x000ac464
void give_absolute_xy_of_cell_at_column_row(const s32 x, const s32 y)
{
    info.cell_x = (x - y) * resvars.tile_width / 2;
    info.cell_y = (x + y) * resvars.tile_height / 2;
}

// Returns 1 if map pixel position x,y falls outside the scroll limits
// (gamevar.scroll_min_tile to f134), widened by 10 tiles with with_margin; else 0.
// FUNCTION: 0x000ac4bc
u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 with_margin)
{
    s32 area;
    s32 u;
    s32 v;
    s32 col;
    s32 row;
    s32 margin;

    area = resvars.tile_width * resvars.tile_height;
    u = (x << 8) * resvars.tile_height;
    v = (y << 8) * resvars.tile_width;
    col = (u + v) / area >> 8;
    row = (v - u) / area >> 8;
    margin = 0;
    if (with_margin)
        margin = 10;
    if (col + margin < gamevar.scroll_min_tile % 128 || col - margin >= gamevar.scroll_max_tile % 128)
        return 1;
    if (row + margin < gamevar.scroll_min_tile / 128 || row - margin >= gamevar.scroll_max_tile / 128)
        return 1;
    return 0;
}

// Clamps the rectangle x0,y0 to x1,y1 to the window wx0,wy0 to wx1,wy1 and
// stores it in info.clip_x0 to unknown74; info.clip_empty is set if the
// result is empty.
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
    info.clip_x0 = x0;
    info.clip_y0 = y0;
    info.clip_x1 = x1;
    info.clip_y1 = y1;
    if (x0 == x1 || y0 == y1)
        info.clip_empty = 1;
    else
        info.clip_empty = 0;
}

// Stores in info.clip_x0 to unknown74 the rectangle covering both x0,y0 to
// x1,y1 and wx0,wy0 to wx1,wy1; info.clip_empty is set if it is empty.
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
    info.clip_x0 = x0;
    info.clip_y0 = y0;
    info.clip_x1 = x1;
    info.clip_y1 = y1;
    if (x0 == x1 || y0 == y1)
        info.clip_empty = 1;
    else
        info.clip_empty = 0;
}

// Opens a second view window (at most two exist) at the mouse with the current
// view's scroll position, growing it to x0,y0 to x1,y1 (source 0) or to preset
// rectangle preset (source 1); a is kept in window.fde. With window.instant_open
// 1 the growth completes at once. Makes the new window current.
// FUNCTION: 0x000ac78c
void open_new_window(const u8 a, const u8 source, const u8 preset, const s32 x0, const s32 y0, const s32 x1, const s32 y1)
{
    s32 old_x0;
    s32 old_y0;
    s32 old_x1;
    s32 old_y1;

    if (window.window_count < 2) {
        window.window_count++;
        window.f2e7 = 0;
        window.f2e8 = 0;
        window.prev_zone_x[window.window_count - 1] = 0;
        window.prev_zone_y[window.window_count - 1] = 0;
        window.prev_zone_x1[window.window_count - 1] = 0;
        window.prev_zone_y1[window.window_count - 1] = 0;
        window.zone_x[window.window_count - 1] = lbDisplay.mmouse_x - 5;
        window.zone_x1[window.window_count - 1] = lbDisplay.mmouse_x + 5;
        window.zone_y[window.window_count - 1] = lbDisplay.mmouse_y - 5;
        window.zone_y1[window.window_count - 1] = lbDisplay.mmouse_y + 5;
        window.center_x[window.window_count - 1] = (window.zone_x[window.window_count - 1] + window.zone_x1[window.window_count - 1]) / 2;
        window.center_y[window.window_count - 1] = (window.zone_y[window.window_count - 1] + window.zone_y1[window.window_count - 1]) / 2;
        window.scroll_x[window.window_count - 1] = window.scroll_x[window.current_window];
        window.scroll_y[window.window_count - 1] = window.scroll_y[window.current_window];
        window.fde[window.window_count - 1] = a;
        window.anim_mode[window.window_count - 1] = 1;
        if (window.instant_open == 0)
            window.anim_steps[window.window_count - 1] = 12;
        else if (window.instant_open == 1)
            window.anim_steps[window.window_count - 1] = 0;
        window.open_x0[window.window_count - 1] = window.zone_x[window.window_count - 1];
        window.open_y0[window.window_count - 1] = window.zone_y[window.window_count - 1];
        window.open_x1[window.window_count - 1] = window.zone_x1[window.window_count - 1];
        window.open_y1[window.window_count - 1] = window.zone_y1[window.window_count - 1];
        if (source == 0)
            set_window_animating(window.window_count - 1, window.anim_mode[window.window_count - 1], window.anim_steps[window.window_count - 1],
                                 x0, y0, x1, y1);
        else if (source == 1)
            set_window_animating(window.window_count - 1, window.anim_mode[window.window_count - 1], window.anim_steps[window.window_count - 1],
                                 window.preset_x0[preset], window.preset_y0[preset], window.preset_x1[preset], window.preset_y1[preset]);
        if (window.instant_open == 1) {
            while (window.anim_steps[window.window_count - 1] > 0) {
                old_x0 = window.zone_x[window.window_count - 1];
                old_y0 = window.zone_y[window.window_count - 1];
                old_x1 = window.zone_x1[window.window_count - 1];
                old_y1 = window.zone_y1[window.window_count - 1];
                animate_single_window(window.window_count - 1);
                find_biggest_window(window.zone_x[window.window_count - 1], window.zone_y[window.window_count - 1],
                                    window.zone_x1[window.window_count - 1], window.zone_y1[window.window_count - 1], old_x0, old_y0, old_x1, old_y1);
                find_common_window(info.clip_x0, info.clip_y0, info.clip_x1, info.clip_y1, window.clip_x0,
                                   window.clip_y0, window.clip_x1, window.clip_y1);
                old_x0 = info.clip_x0;
                old_y0 = info.clip_y0;
                old_x1 = info.clip_x1;
                old_y1 = info.clip_y1;
            }
        }
        window.current_window = window.window_count - 1;
    }
}

// Starts view window win moving from its zone to x0,y0 to x1,y1 over steps
// frames (0 counts as 1), with mode kept in window.anim_mode (2 closes it at the
// end). Ignored for an empty target.
// FUNCTION: 0x000acc4c
void set_window_animating(const u8 win, const u8 mode, s32 steps, const s32 x0, const s32 y0, const s32 x1, const s32 y1)
{
    if (steps == 0)
        steps = 1;
    if (x0 < x1 && y0 < y1 && steps > 0) {
        window.anim_mode[win] = mode;
        window.anim_steps[win] = steps + 1;
        window.anim_to_x0[win] = x0;
        window.anim_to_y0[win] = y0;
        window.anim_to_x1[win] = x1;
        window.anim_to_y1[win] = y1;
        window.anim_step_x0[win] = ((window.zone_x[win] - x0) << 16) / steps;
        window.anim_step_y0[win] = ((window.zone_y[win] - y0) << 16) / steps;
        window.anim_step_x1[win] = ((window.zone_x1[win] - x1) << 16) / steps;
        window.anim_step_y1[win] = ((window.zone_y1[win] - y1) << 16) / steps;
    }
}

// Advances every view window's zone animation by a frame, closing those that
// finished closing.
// FUNCTION: 0x000acdb0
void animate_windows(void)
{
    u8 i;
    s32 w;
    s32 h;
    s32 x;
    s32 y;

    i = 0;
    while (i < window.window_count) {
        if (window.anim_steps[i] > 0) {
            window.anim_steps[i]--;
            window.zone_x[i] = (window.anim_step_x0[i] * window.anim_steps[i] >> 16) + window.anim_to_x0[i];
            window.zone_y[i] = (window.anim_step_y0[i] * window.anim_steps[i] >> 16) + window.anim_to_y0[i];
            window.zone_x1[i] = window.anim_to_x1[i] + (window.anim_step_x1[i] * window.anim_steps[i] >> 16);
            window.zone_y1[i] = window.anim_to_y1[i] + (window.anim_step_y1[i] * window.anim_steps[i] >> 16);
            window.center_x[i] = (window.zone_x[i] + window.zone_x1[i]) / 2;
            window.center_y[i] = (window.zone_y[i] + window.zone_y1[i]) / 2;
            i++;
        } else if (window.anim_mode[i] == 2) {
            x = window.prev_zone_x[i];
            y = window.prev_zone_y[i];
            w = window.prev_zone_x1[i];
            h = window.prev_zone_y1[i];
            remove_window_from_list(i);
            remove_window_from_screen(x, y, w, h);
            if (window.current_window >= window.window_count && window.current_window > 0)
                window.current_window--;
        } else {
            window.anim_mode[i] = 0;
            i++;
        }
    }
}

// Advances view window win's zone animation by a frame, removing it from the
// list if it finished closing.
// FUNCTION: 0x000ad034
void animate_single_window(const u8 win)
{
    s32 x;
    s32 y;
    s32 w;
    s32 h;

    if (window.anim_steps[win] > 0) {
        window.anim_steps[win]--;
        window.zone_x[win] = (window.anim_step_x0[win] * window.anim_steps[win] >> 16) + window.anim_to_x0[win];
        window.zone_y[win] = (window.anim_step_y0[win] * window.anim_steps[win] >> 16) + window.anim_to_y0[win];
        window.zone_x1[win] = window.anim_to_x1[win] + (window.anim_step_x1[win] * window.anim_steps[win] >> 16);
        window.zone_y1[win] = window.anim_to_y1[win] + (window.anim_step_y1[win] * window.anim_steps[win] >> 16);
        window.center_x[win] = (window.zone_x[win] + window.zone_x1[win]) / 2;
        window.center_y[win] = (window.zone_y[win] + window.zone_y1[win]) / 2;
    } else if (window.anim_mode[win] == 2) {
        x = window.zone_x[win];
        y = window.zone_y[win];
        w = window.zone_x1[win];
        h = window.zone_y1[win];
        window.anim_mode[win] = 0;
        remove_window_from_list(win);
        if ((int)window.current_window > (int)win)
            window.current_window--;
    } else {
        window.anim_mode[win] = 0;
    }
}

// Copies view window from's state to slot to.
// FUNCTION: 0x000ad284
void copy_window_from_a_to_b(const u8 from, const u8 to)
{
    window.center_x[to] = window.center_x[from];
    window.center_y[to] = window.center_y[from];
    window.zone_x[to] = window.zone_x[from];
    window.zone_x1[to] = window.zone_x1[from];
    window.zone_y[to] = window.zone_y[from];
    window.zone_y1[to] = window.zone_y1[from];
    window.prev_zone_x[to] = window.prev_zone_x[from];
    window.prev_zone_y[to] = window.prev_zone_y[from];
    window.prev_zone_x1[to] = window.prev_zone_x1[from];
    window.prev_zone_y1[to] = window.prev_zone_y1[from];
    window.scroll_vel_x[to] = window.scroll_vel_x[from];
    window.scroll_vel_y[to] = window.scroll_vel_y[from];
    window.scroll_frac_x[to] = window.scroll_frac_x[from];
    window.scroll_frac_y[to] = window.scroll_frac_y[from];
    window.scroll_x[to] = window.scroll_x[from];
    window.scroll_y[to] = window.scroll_y[from];
    window.prev_scroll_x[to] = window.prev_scroll_x[from];
    window.prev_scroll_y[to] = window.prev_scroll_y[from];
    window.fde[to] = window.fde[from];
    window.open_x0[to] = window.open_x0[from];
    window.open_y0[to] = window.open_y0[from];
    window.open_x1[to] = window.open_x1[from];
    window.open_y1[to] = window.open_y1[from];
    window.anim_to_x0[to] = window.anim_to_x0[from];
    window.anim_to_y0[to] = window.anim_to_y0[from];
    window.anim_to_x1[to] = window.anim_to_x1[from];
    window.anim_to_y1[to] = window.anim_to_y1[from];
    window.anim_step_x0[to] = window.anim_step_x0[from];
    window.anim_step_y0[to] = window.anim_step_y0[from];
    window.anim_step_x1[to] = window.anim_step_x1[from];
    window.anim_step_y1[to] = window.anim_step_y1[from];
    window.anim_mode[to] = window.anim_mode[from];
    window.anim_steps[to] = window.anim_steps[from];
}

// Removes view window win from the list, moving later ones down.
// FUNCTION: 0x000ad6cc
void remove_window_from_list(const u8 win)
{
    u8 i;

    window.window_count--;
    if ((int)win < (int)window.window_count) {
        for (i = win; i < window.window_count; i++)
            copy_window_from_a_to_b(i + 1, i);
    }
}

// Resets the view windows to a single full-screen view, the background wrap
// offset to 0, and the preset rectangles; flags the whole screen for redraw.
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
    window.scroll_x[0] = 0;
    window.scroll_y[0] = 1000;
    window.prev_scroll_x[0] = window.scroll_x[0];
    window.prev_scroll_y[0] = window.scroll_y[0];
    window.prev_zone_x[0] = 0;
    window.prev_zone_y[0] = 0;
    window.prev_zone_x1[0] = 0;
    window.prev_zone_y1[0] = 0;
    window.scroll_vel_x[0] = 0;
    window.scroll_vel_y[0] = 0;
    window.scroll_vel_x[1] = 0;
    window.scroll_vel_y[1] = 0;
    window.scroll_frac_x[0] = 0;
    window.scroll_frac_y[0] = 0;
    window.fde[0] = 0;
    window.open_x0[0] = 0x13b;
    window.open_y0[0] = 0xeb;
    window.open_x1[0] = 0x145;
    window.open_y1[0] = 0xf5;
    window.anim_mode[0] = 0;
    window.f2e7 = 0;
    window.f2e8 = 0;
    window.f2e9 = 0;
    refresh.scroll_x = 0;
    refresh.scroll_y = 0;
    window.current_window = 0;
    window.window_count = 1;
    window.bscreen_window = 0;
    window.anim_mode[0] = 0;
    window.anim_mode[1] = 0;
    for (; i < 4; i++) {
        window.preset_x0[i * 4] = 0x23;
        window.preset_y0[i * 4] = i * 0x5a + 0x28;
        window.preset_x1[i * 4] = 0xa5;
        window.preset_y1[i * 4] = i * 0x5a + 0x6e;
        window.preset_x0[i * 4 + 1] = 0xb9;
        window.preset_y0[i * 4 + 1] = i * 0x5a + 0x28;
        window.preset_x1[i * 4 + 1] = 0x13b;
        window.preset_y1[i * 4 + 1] = i * 0x5a + 0x6e;
        window.preset_x0[i * 4 + 2] = 0x14f;
        window.preset_y0[i * 4 + 2] = i * 0x5a + 0x28;
        window.preset_x1[i * 4 + 2] = 0x1d1;
        window.preset_y1[i * 4 + 2] = i * 0x5a + 0x6e;
        window.preset_x0[i * 4 + 3] = 0x1e5;
        window.preset_y0[i * 4 + 3] = i * 0x5a + 0x28;
        window.preset_x1[i * 4 + 3] = 0x267;
        window.preset_y1[i * 4 + 3] = i * 0x5a + 0x6e;
    }
    window.instant_open = 1;
    window.zone_x[0] = 0;
    window.zone_x1[0] = lbDisplay.physical_screen_width;
    window.zone_y[0] = 0;
    window.zone_y1[0] = statvar.view_height;
    window.center_x[0] = (window.zone_x[0] + window.zone_x1[0]) / 2;
    window.center_y[0] = (window.zone_y[0] + window.zone_y1[0]) / 2;
    debug.ffc = 0;
    debug.ffd = 0;
    debug.ffe = 0;
    debug.fff = 0;
    debug.f100 = 0;
    debug.f101 = 0;
}
