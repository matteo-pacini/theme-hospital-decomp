#include "hosp.h"

/* Links between the cells of one scan row, for drawing them back to front. */
struct CellLink {
    s8 behind;
    s8 front;
    s8 next;
};

extern s8 covers[][4];

/* A precalculated shadow sprite and the block it shades. */
struct PrecalcShadows {
    TbSprite sprite;
    u16 block;
};

extern PrecalcShadows InteriorTileFull;
extern PrecalcShadows InteriorTileHalf;
extern PrecalcShadows InteriorWallHalf;
extern PrecalcShadows RoomTileFull[];
extern PrecalcShadows RoomTileHalf[];
extern s8 ShadowBuffer[][85];

void draw_a_clipped_four_zone_floor_tile(s32 x, s32 y, const TbSprite *const spr, s32 x0, s32 y0, s32 x1, s32 y1);
void draw_a_clipped_four_zone_wall_0(s32 x, s32 y, const TbSprite *const spr, s32 x0, s32 y0, s32 x1, s32 y1);
void draw_a_clipped_four_zone_wall_1(s32 x, s32 y, const s32 cell, s32 x0, s32 y0, s32 x1, s32 y1);
void give_absolute_xy_of_cell_at_column_row(const s32 x, const s32 y);
void create_overlay_sprite(const TbSprite *src, const TbSprite *ovr, const TbSprite *dst);

void process_tracking_windows(void);
s8 compute_cell_precedence(const s8 row, Thing *t);
s8 check_obscuring_column(const s8 row, s8 x, s8 y);
s8 record_covering_cell(const s8 row, const s16 pos);
void setup_stats_for_window_scan(const u8 win, s32 x, const s32 y, s32 w, const s32 h);
void do_cell(const u8 win, const s32 x, const s32 y, const s32 z, const s32 w, const s32 cell, const s32 wx,
             const s32 wy, const s32 cx0, const s32 cy0, const s32 cx1, const s32 cy1);
void animate_windows(void);
void move_windows(void);
void draw_overdrawn_blueprints(void);
void update_panel(void);
void draw_menus(void);
void draw_popup_icons(void);
void draw_emergency(void);
void draw_cell_no(s32 cell, u8 colour);
void set_map_cur_and_mcur_x_and_y(void);
void refresh_wscreen(void);
void copy_bscreen_window_to_wscreen_mk1(const s32 x0, const s32 y0, const s32 x1, const s32 y1, const s16 dx,
                                        const s16 dy);
void draw_window_not_using_bscreen(const u8 win);
void remove_old_window_image_from_wscreen_replacing_detail(s32 x0, s32 y0, s32 x1, s32 y1, s32 nx0, s32 ny0, s32 nx1,
                                                           s32 ny1);
void remove_old_window_image_from_wscreen(s32 x0, s32 y0, s32 x1, s32 y1, s32 nx0, s32 ny0, s32 nx1, s32 ny1);
void draw_all_sprites_on_wscreen_selectively_using_pix_buffer(const u8 win, s32 x0, s32 y0, s32 x1, s32 y1);
void draw_boundry(const u8 win, s32 x0, s32 y0, s32 x1, s32 y1);
void draw_all_mk7_selective_sprites_wscreen(const u8 win, s32 x0, s32 y0, s32 x1, s32 y1);
void draw_all_mk7_no_sprites_bscreen(const u8 win, s32 x0, s32 y0, s32 x1, s32 y1);
void draw_window_into_pix_buffer(const u8 win, s32 x0, s32 y0, s32 x1, s32 y1, const s32 lx, const s32 ly);
void draw_window_on_wscreen_mk2(const u8 win, s32 x0, s32 y0, s32 x1, s32 y1, const s32 cell);

extern "C" {
extern s16 D_001567b0[8];
extern CellLink draw_precedence[];
void LbDrawLine(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
void LbSpriteDrawOneColour(s32 x, s32 y, TbSprite *spr, s32 colour);
void draw_all_windows(void);
void draw_main_window(void);
void draw_other_windows(void);
void draw_window_using_bscreen(u8 win);
void remove_moved_windows_not_using_bscreen_from_wscreen(u8 win);
}

// Draws one game frame: follows tracked things, animates and scrolls the view
// windows and draws the main one, then the blueprints, panel, menus, popup
// icons and emergency display, updates the help text and blueprint cursor,
// and reads the map cursor from the mouse.
// FUNCTION: 0x000a6b20
void draw_it(void)
{
    window.clip_y1 = statvar.view_height;
    window.clip_x0 = 0;
    window.clip_y0 = 0;
    window.clip_x1 = lbDisplay.physical_screen_width;
    process_tracking_windows();
    othervar.pointed_thing = 0;
    othervar.pointed_rat = othervar.near_rat = 0;
    if (window.window_count > 0) {
        animate_windows();
        move_windows();
        draw_main_window();
    }
    draw_overdrawn_blueprints();
    update_panel();
    draw_menus();
    draw_popup_icons();
    draw_emergency();
    if (!debug.suppress_help)
        compute_help_text(0, 0, 3);
    if (gamevar.mouse_mode == MM_BLUEPRINT)
        draw_cell_no(othervar.mouse_cell, color.red);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_map_cur_and_mcur_x_and_y();
}

// Redraws every view window: clears the old images of moved secondary views,
// then draws the main one through the background buffer and the others
// directly.
// FUNCTION: 0x000a6c08
void draw_all_windows(void)
{
    u8 i;

    for (i = 0; i < window.window_count; i++)
        remove_moved_windows_not_using_bscreen_from_wscreen(i);
    for (i = 0; i < window.window_count; i++) {
        if (i != window.bscreen_window)
            draw_window_not_using_bscreen(i);
        else
            draw_window_using_bscreen(i);
    }
}

// Draws the main view window through the background buffer.
// FUNCTION: 0x000a6c7c
void draw_main_window(void)
{
    u8 i = 0;

    draw_window_using_bscreen(window.bscreen_window);
}

// Draws every view window except the main one, directly.
// FUNCTION: 0x000a6ca4
void draw_other_windows(void)
{
    u8 i = 0;

    for (i = 0; i < window.window_count; i++) {
        if (i != window.bscreen_window)
            draw_window_not_using_bscreen(i);
    }
}

// Draws view window win through the background buffer: if nothing changed (not
// moved or resized, window.blank 0 and unchanged, othervar.full_redraw clear) only the
// dirty cells are copied; otherwise the floor and walls are redrawn into
// bscreen and copied whole. Then draws the things on top and records the
// window's position and zone as drawn.
// FUNCTION: 0x000a6cf0
void draw_window_using_bscreen(u8 win)
{
    if ((((window.scrolled[win] == 0 && window.zone_changed[win] == 0) &&
          window.prev_blank[win] == window.blank[win]) &&
         window.blank[win] == 0) &&
        !othervar.full_redraw) {
        refresh_wscreen();
    } else {
        s32 x;
        s32 y;
        s32 w;
        s32 h;
        u8 empty;

        find_common_window(window.zone_x[win], window.zone_y[win], window.zone_x1[win], window.zone_y1[win],
                           window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
        x = info.clip_x0;
        y = info.clip_y0;
        w = info.clip_x1;
        h = info.clip_y1;
        empty = info.clip_empty;
        refresh.scroll_x = 0;
        refresh.scroll_y = 0;
        draw_all_mk7_no_sprites_bscreen(0, 0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
        find_common_window(window.prev_zone_x[win], window.prev_zone_y[win], window.prev_zone_x1[win],
                           window.prev_zone_y1[win], window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
        if (info.clip_empty == 0)
            set_all_refresh_zones_in_window(info.clip_x0, info.clip_y0, info.clip_x1, info.clip_y1, 1);
        find_common_window(window.zone_x[win], window.zone_y[win], window.zone_x1[win], window.zone_y1[win],
                           window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
        if (info.clip_empty == 0)
            set_all_refresh_zones_in_window(info.clip_x0, info.clip_y0, info.clip_x1, info.clip_y1, 1);
        remove_old_window_image_from_wscreen(window.prev_zone_x[win], window.prev_zone_y[win], window.prev_zone_x1[win],
                                             window.prev_zone_y1[win], window.zone_x[win], window.zone_y[win],
                                             window.zone_x1[win], window.zone_y1[win]);
        if (empty == 0)
            copy_bscreen_window_to_wscreen_mk1(x, y, w, h, refresh.scroll_x, refresh.scroll_y);
    }
    draw_all_sprites_on_wscreen_selectively_using_pix_buffer(0, window.clip_x0, window.clip_y0, window.clip_x1,
                                                             window.clip_y1);
    othervar.full_redraw = 0;
    window.prev_scroll_x[win] = window.scroll_x[win];
    window.prev_scroll_y[win] = window.scroll_y[win];
    window.prev_zone_x[win] = window.zone_x[win];
    window.prev_zone_y[win] = window.zone_y[win];
    window.prev_zone_x1[win] = window.zone_x1[win];
    window.prev_zone_y1[win] = window.zone_y1[win];
    window.prev_blank[win] = window.blank[win];
    window.zone_changed[win] = 0;
    window.scrolled[win] = 0;
}

// If view window win was resized, clears what its old zone left uncovered and
// flags it for redraw.
// FUNCTION: 0x000a70d0
void remove_moved_windows_not_using_bscreen_from_wscreen(u8 win)
{
    if (window.zone_changed[win] == 1) {
        remove_old_window_image_from_wscreen_replacing_detail(
            window.prev_zone_x[win], window.prev_zone_y[win], window.prev_zone_x1[win], window.prev_zone_y1[win],
            window.zone_x[win], window.zone_y[win], window.zone_x1[win], window.zone_y1[win]);
        set_all_refresh_zones_in_window(window.prev_zone_x[win], window.prev_zone_y[win], window.prev_zone_x1[win],
                                        window.prev_zone_y1[win], 2);
    }
}

// Draws secondary view window win straight onto the work screen (floor, walls
// and things, unless window.blank is set), with its border, and records its
// position and zone as drawn.
// FUNCTION: 0x000a71bc
void draw_window_not_using_bscreen(const u8 win)
{
    set_all_refresh_zones_in_window(window.zone_x[win], window.zone_y[win], window.zone_x1[win], window.zone_y1[win],
                                    2);
    if (window.zone_changed[win] == 1)
        set_all_refresh_zones_in_window(window.prev_zone_x[win], window.prev_zone_y[win], window.prev_zone_x1[win],
                                        window.prev_zone_y1[win], 3);
    if (window.blank[win] == 0)
        draw_all_mk7_selective_sprites_wscreen(win, window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
    draw_boundry(win, window.zone_x[win], window.zone_y[win], window.zone_x1[win], window.zone_y1[win]);
    window.prev_scroll_x[win] = window.scroll_x[win];
    window.prev_scroll_y[win] = window.scroll_y[win];
    window.prev_zone_x[win] = window.zone_x[win];
    window.prev_zone_y[win] = window.zone_y[win];
    window.prev_zone_x1[win] = window.zone_x1[win];
    window.prev_zone_y1[win] = window.zone_y1[win];
    window.zone_changed[win] = 0;
    window.scrolled[win] = 0;
}

// Fills with the background colour the parts of the old zone x0,y0 to x1,y1
// that the new zone nx0,ny0 to nx1,ny1 does not cover, then copies back the
// main view under them.
// FUNCTION: 0x000a73c0
void remove_old_window_image_from_wscreen_replacing_detail(s32 x0, s32 y0, s32 x1, s32 y1, s32 nx0, s32 ny0, s32 nx1,
                                                           s32 ny1)
{
    u8 colour;

    colour = color.black;
    find_common_window(x0, y0, x1, y1, window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
    x0 = info.clip_x0;
    y0 = info.clip_y0;
    x1 = info.clip_x1;
    y1 = info.clip_y1;
    find_common_window(nx0, ny0, nx1, ny1, window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
    nx0 = info.clip_x0;
    ny0 = info.clip_y0;
    nx1 = info.clip_x1;
    ny1 = info.clip_y1;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    if (y1 > y0) {
        if (x0 < nx0) {
            LbDrawBox(x0, y0, nx0 - x0, y1 - y0, colour);
            if (window.blank[window.bscreen_window] == 0) {
                find_common_window(x0, y0, nx0, y1, window.zone_x[window.bscreen_window],
                                   window.zone_y[window.bscreen_window], window.zone_x1[window.bscreen_window],
                                   window.zone_y1[window.bscreen_window]);
                if (info.clip_empty == 0)
                    copy_bscreen_window_to_wscreen_mk1(info.clip_x0, info.clip_y0, info.clip_x1, info.clip_y1,
                                                       refresh.scroll_x, refresh.scroll_y);
            }
        }
        if (x1 > nx1) {
            LbDrawBox(nx1, y0, x1 - nx1, y1 - y0, colour);
            if (window.blank[window.bscreen_window] == 0) {
                find_common_window(nx1, y0, x1, y1, window.zone_x[window.bscreen_window],
                                   window.zone_y[window.bscreen_window], window.zone_x1[window.bscreen_window],
                                   window.zone_y1[window.bscreen_window]);
                if (info.clip_empty == 0)
                    copy_bscreen_window_to_wscreen_mk1(info.clip_x0, info.clip_y0, info.clip_x1, info.clip_y1,
                                                       refresh.scroll_x, refresh.scroll_y);
            }
        }
    }
    if (x1 > x0) {
        if (y0 < ny0) {
            LbDrawBox(x0, y0, x1 - x0, ny0 - y0, colour);
            if (window.blank[window.bscreen_window] == 0) {
                find_common_window(x0, y0, x1, ny0, window.zone_x[window.bscreen_window],
                                   window.zone_y[window.bscreen_window], window.zone_x1[window.bscreen_window],
                                   window.zone_y1[window.bscreen_window]);
                if (info.clip_empty == 0)
                    copy_bscreen_window_to_wscreen_mk1(info.clip_x0, info.clip_y0, info.clip_x1, info.clip_y1,
                                                       refresh.scroll_x, refresh.scroll_y);
            }
        }
        if (y1 > ny1) {
            LbDrawBox(x0, ny1, x1 - x0, y1 - ny1, colour);
            if (window.blank[window.bscreen_window] == 0) {
                find_common_window(x0, ny1, x1, y1, window.zone_x[window.bscreen_window],
                                   window.zone_y[window.bscreen_window], window.zone_x1[window.bscreen_window],
                                   window.zone_y1[window.bscreen_window]);
                if (info.clip_empty == 0)
                    copy_bscreen_window_to_wscreen_mk1(info.clip_x0, info.clip_y0, info.clip_x1, info.clip_y1,
                                                       refresh.scroll_x, refresh.scroll_y);
            }
        }
    }
}

// Fills with the background colour the parts of the old zone x0,y0 to x1,y1
// above and below the new zone nx0,ny0 to nx1,ny1. The left and right parts
// are never filled: their test compares y1 with itself.
// FUNCTION: 0x000a77dc
void remove_old_window_image_from_wscreen(s32 x0, s32 y0, s32 x1, s32 y1, s32 nx0, s32 ny0, s32 nx1, s32 ny1)
{
    u8 colour;

    colour = color.black;
    find_common_window(x0, y0, x1, y1, window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
    x0 = info.clip_x0;
    y0 = info.clip_y0;
    x1 = info.clip_x1;
    y1 = info.clip_y1;
    find_common_window(nx0, ny0, nx1, ny1, window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
    nx0 = info.clip_x0;
    ny0 = info.clip_y0;
    nx1 = info.clip_x1;
    ny1 = info.clip_y1;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    if (y1 > y1) {
        if (x0 < nx0)
            LbDrawBox(x0, y0, nx0 - x0, y1 - y0, colour);
        if (x1 > nx1)
            LbDrawBox(nx1, y0, x1 - nx1, y1 - y0, colour);
    }
    if (x1 > x0) {
        if (y0 < ny0)
            LbDrawBox(x0, y0, x1 - x0, ny0 - y0, colour);
        if (y1 > ny1)
            LbDrawBox(x0, ny1, x1 - x0, y1 - ny1, colour);
    }
}

// Fills the screen rectangle x,y to w,h (right and bottom edges) with the
// background colour and flags it for redraw.
// FUNCTION: 0x000a7950
void remove_window_from_screen(s32 x, s32 y, s32 w, s32 h)
{
    find_common_window(x, y, w, h, window.clip_x0, window.clip_y0, window.clip_x1, window.clip_y1);
    if (info.clip_empty == 0) {
        set_all_refresh_zones_in_window(info.clip_x0, info.clip_y0, info.clip_x1, info.clip_y1, 3);
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbDrawBox(info.clip_x0, info.clip_y0, info.clip_x1 - info.clip_x0, info.clip_y1 - info.clip_y0, color.black);
    }
}

// Prepares the scan of the map cells covering screen rectangle x,y to w,h
// (right and bottom edges) in view window win, row by row from the top: sets
// info.scan_cell to f24 to the first cell, its screen position, the row length, the
// row counts for floors and for walls and things (taller), and how rows
// alternate.
// FUNCTION: 0x000a7a0c
void setup_stats_for_window_scan(const u8 win, s32 x, const s32 y, s32 w, const s32 h)
{
    u8 odd;
    u8 left;
    u8 right;
    u8 bottom;
    u8 top;
    s32 extra;
    s32 dx;
    s32 dy;
    s32 span;
    s32 cnt;
    s32 prod;
    s32 rem;

    left = 1;
    odd = 1;
    top = 1;
    right = 1;
    bottom = 1;
    extra = resvars.tile_height * 19 / 10;
    x -= resvars.tile_width / 2;
    w += resvars.tile_width / 2;
    find_cell_at_point(win, x, y);
    info.scan_x = info.cell_x;
    info.scan_y = info.cell_y;
    info.scan_col = info.cell_col;
    info.scan_row = info.cell_row;
    info.scan_cell = info.cell;
    dx = x - info.scan_x - resvars.tile_width / 2;
    if (dx >= 0)
        left = 0;
    dy = y - info.scan_y + resvars.tile_height / 2;
    if (dy >= 0)
        top = 0;
    span = w - x + (dx - resvars.tile_width / 2);
    cnt = span / resvars.tile_width;
    prod = cnt * resvars.tile_width;
    if (prod < span) {
        rem = span - prod;
        if (resvars.tile_width / 2 >= rem)
            odd = 0;
        cnt++;
    }
    info.line_len = cnt + 1;
    dy -= resvars.tile_height / 2;
    span = dy + (h - y);
    cnt = span / resvars.tile_height;
    prod = cnt * resvars.tile_height;
    if (prod < span) {
        rem = span - prod;
        if (resvars.tile_height / 2 >= rem)
            right = 0;
        cnt++;
    }
    info.floor_lines = cnt * 2 + 1;
    span = dy + (h + extra - y);
    cnt = span / resvars.tile_height;
    prod = cnt * resvars.tile_height;
    if (prod < span) {
        rem = span - prod;
        if (resvars.tile_height / 2 >= rem)
            bottom = 0;
        cnt++;
    }
    info.full_lines = cnt * 2 + 1;
    info.scan_dx = resvars.tile_width / 2;
    info.line_len_step = -1;
    if (top) {
        info.full_lines++;
        info.floor_lines++;
        info.scan_y -= resvars.tile_height / 2;
        if (left) {
            info.scan_col--;
            info.scan_cell--;
            info.scan_x -= resvars.tile_width / 2;
            if (odd)
                info.line_len++;
            else
                info.line_len_step = 0;
        } else {
            info.scan_row--;
            info.scan_cell -= 0x80;
            info.scan_x += resvars.tile_width / 2;
            info.scan_dx = resvars.tile_width / 2;
            info.scan_dx = -info.scan_dx;
            if (odd) {
                info.line_len_step = 0;
            } else {
                info.line_len_step = 1;
                info.line_len--;
            }
        }
    } else if (left) {
        info.scan_dx = resvars.tile_width / 2;
        info.scan_dx = -info.scan_dx;
        if (odd)
            info.line_len_step = 1;
        else
            info.line_len_step = 0;
    } else if (odd) {
        info.line_len_step = 0;
    }
    if (right)
        info.floor_lines++;
    if (bottom)
        info.full_lines++;
}

// Draws the things on map cell cell at screen x,y (z,w are the cell's column
// and row) clipped to cx0,cy0 to cx1,cy1, then redraws the walls in front of
// them; for cells near walls (map flag 0x10) through the pixel buffer, so that
// only the visible parts show. wx,wy offset the redraw area. Flags the area
// for redraw.
// FUNCTION: 0x000a7df0
void do_cell(const u8 win, const s32 x, const s32 y, const s32 z, const s32 w, const s32 cell, const s32 wx,
             const s32 wy, const s32 cx0, const s32 cy0, const s32 cx1, const s32 cy1)
{
    Map *m;
    s32 ox;
    s32 oy;
    s32 px;
    s32 py;
    s32 bx0;
    s32 by0;
    s32 bx1;
    s32 by1;

    m = &map[cell];
    if ((m->first || (m->floor & 0xf)) || (m->flags & 0xc0)) {
        if (m->flags & 0x10) {
            ox = 0x5a - resvars.tile_width / 2;
            oy = 0xb4 - resvars.tile_height * 3 / 2;
            DrawIntoPixBuffer();
            do_mapwho(m, ox, oy, cx0 + x - ox, cy0 + y - 0xb4);
            px = D_001567b0[0];
            py = D_001567b0[1];
            D_001567b0[0] += x - ox;
            D_001567b0[1] += y - 0xb4 + resvars.tile_height * 3 / 2 - resvars.tile_height;
            D_001567b0[2] += x - ox;
            D_001567b0[3] += y - 0xb4 + resvars.tile_height * 3 / 2 - resvars.tile_height;
            pixBuf.x = x - ox;
            pixBuf.y = y - 0xb4 + resvars.tile_height * 3 / 2 - resvars.tile_height;
            find_common_window(D_001567b0[0] + wx, D_001567b0[1] + wy, D_001567b0[2] + wx, D_001567b0[3] + wy, cx0, cy0,
                               cx1, cy1);
            if (info.clip_empty == 0) {
                bx0 = info.clip_x0;
                by0 = info.clip_y0;
                bx1 = info.clip_x1;
                by1 = info.clip_y1;
                draw_window_into_pix_buffer(0, bx0, by0, bx1, by1, z, w);
                set_all_refresh_zones_in_window(bx0, by0, bx1, by1, 3);
            }
            RestoreFromPixBuffer();
            LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
            DrawPixBuffer(0, px, py, D_001567b0[2] - D_001567b0[0], D_001567b0[3] - D_001567b0[1]);
            FillBoxInPixBuffer(0xfe, px, py, D_001567b0[2] - D_001567b0[0], D_001567b0[3] - D_001567b0[1]);
        } else {
            LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
            do_mapwho(m, x, y - resvars.tile_height, 0, 0);
            find_common_window(D_001567b0[0] + wx, D_001567b0[1] + wy, D_001567b0[2] + wx, D_001567b0[3] + wy, cx0, cy0,
                               cx1, cy1);
            if (info.clip_empty == 0) {
                bx0 = info.clip_x0;
                by0 = info.clip_y0;
                bx1 = info.clip_x1;
                by1 = info.clip_y1;
                draw_window_on_wscreen_mk2(win, bx0, by0, bx1, by1, cell);
                set_all_refresh_zones_in_window(bx0, by0, bx1, by1, 3);
            }
        }
    }
}

// Draws all things in screen rectangle x0,y0 to x1,y1 of view window win onto
// the work screen, row by row; cells holding things that span several cells
// (map flag 0x20) are ordered so that the ones in front draw last.
// FUNCTION: 0x000a8198
void draw_all_sprites_on_wscreen_selectively_using_pix_buffer(const u8 win, s32 x0, s32 y0, s32 x1, s32 y1)
{
    s32 line_len;
    s32 floor_lines;
    s32 full_lines;
    s32 step;
    s32 base;
    s32 bx;
    s32 by;
    s32 dx;
    s32 px;
    s32 py;
    s32 minx;
    s32 miny;
    s32 j;

    find_common_window(x0, y0, x1, y1, window.zone_x[win], window.zone_y[win], window.zone_x1[win],
                       window.zone_y1[win]);
    if (info.clip_empty == 0) {
        x0 = info.clip_x0;
        y0 = info.clip_y0;
        x1 = info.clip_x1;
        y1 = info.clip_y1;
        setup_stats_for_window_scan(win, x0, y0, x1, y1);
        line_len = info.line_len;
        floor_lines = info.floor_lines;
        full_lines = info.full_lines;
        step = info.line_len_step;
        base = info.scan_cell;
        bx = info.scan_col;
        by = info.scan_row;
        dx = info.scan_dx;
        px = info.scan_x;
        py = info.scan_y;
        minx = 0;
        miny = 0;
        LbScreenSetGraphicsWindow(x0, y0, x1 - x0, y1 - y0);
        px -= x0;
        py -= y0;
        if (window.zone_x[win] > 0)
            minx = window.zone_x[win];
        if (window.zone_y[win] > 0)
            miny = window.zone_y[win];
        if (window.clip_x0 > minx)
            minx = window.clip_x0;
        if (window.clip_y0 > miny)
            miny = window.clip_y0;
        for (j = 0; j < full_lines; j++) {
            s32 c;
            s32 x;
            s32 y;
            s8 k;
            s8 head;
            s8 tail;
            s8 last;
            s32 i;
            Map *m;

            head = -1;
            tail = -1;
            last = -1;
            c = base;
            x = bx;
            y = by;
            memset(covers, 0xff, 0x38);
            for (i = 0; i < line_len; i++) {
                m = &map[c];
                if ((((x >= 0 && x < 0x80) && y >= 0) && y < 0x80) &&
                    ((m->first || (m->floor & 0xf)) || (m->flags & 0xc0))) {
                    if (m->flags & MAP_BIG_CELL) {
                        s8 prec;

                        prec = compute_cell_precedence(i, &thing[m->first]);
                        if (prec >= 0) {
                            draw_precedence[i].behind = draw_precedence[prec].behind;
                            draw_precedence[i].front = prec;
                            draw_precedence[prec].behind = i;
                        } else {
                            draw_precedence[i].behind = -1;
                            draw_precedence[i].front = -1;
                            draw_precedence[i].next = -1;
                            if (head >= 0) {
                                draw_precedence[tail].next = i;
                                tail = i;
                            } else {
                                head = i;
                                tail = i;
                            }
                        }
                    } else {
                        draw_precedence[i].next = last;
                        last = i;
                    }
                }
                x++;
                y--;
                c += -0x7f;
            }
            for (k = head; k >= 0; k = draw_precedence[k].next) {
                i = k;
                while (draw_precedence[i].behind >= 0)
                    i = draw_precedence[i].behind;
                for (; i >= 0; i = draw_precedence[i].front) {
                    x = bx + i;
                    y = by - i;
                    c = base - i * 0x7f;
                    do_cell(win, px + resvars.tile_width * i, py, x, y, c, minx, miny, x0, y0, x1, y1);
                }
            }
            for (i = last; i >= 0; i = draw_precedence[i].next) {
                x = bx + i;
                y = by - i;
                c = base - i * 0x7f;
                do_cell(win, px + resvars.tile_width * i, py, x, y, c, minx, miny, x0, y0, x1, y1);
            }
            px += dx;
            py += resvars.tile_height / 2;
            if (dx < 0) {
                by++;
                base += 0x80;
                line_len += step;
            } else if (dx > 0) {
                bx++;
                base++;
                line_len -= step;
            }
            dx = -dx;
        }
    }
}

// Redraws the walls of view window win within x0,y0 to x1,y1 from the row
// after the one containing map cell cell, so that they cover things drawn
// behind them.
// FUNCTION: 0x000a8688
void draw_window_on_wscreen_mk2(const u8 win, s32 x0, s32 y0, s32 x1, s32 y1, const s32 cell)
{
    Map *mp;
    s32 line_len;
    s32 floor_lines;
    s32 full_lines;
    s32 step;
    s32 base;
    s32 bx;
    s32 by;
    s32 px;
    s32 py;
    s32 dx;
    s32 half;
    s32 sx;
    s32 sy;
    u8 active;
    u8 reached;
    s32 j;
    s32 x;
    s32 c;
    s32 cx;
    s32 cy;
    s32 i;
    Map *m;
    TbSprite overlay;

    mp = map;
    find_common_window(x0, y0, x1, y1, window.zone_x[win], window.zone_y[win], window.zone_x1[win],
                       window.zone_y1[win]);
    if (info.clip_empty == 0) {
        x0 = info.clip_x0;
        y0 = info.clip_y0;
        x1 = info.clip_x1;
        y1 = info.clip_y1;
        setup_stats_for_window_scan(win, x0, y0, x1, y1);
        line_len = info.line_len;
        floor_lines = info.floor_lines;
        full_lines = info.full_lines;
        step = info.line_len_step;
        base = info.scan_cell;
        bx = info.scan_col;
        by = info.scan_row;
        px = info.scan_x;
        py = info.scan_y;
        dx = info.scan_dx;
        half = resvars.tile_height / 2;
        sx = px;
        sy = py;
        LbScreenSetGraphicsWindow(x0, y0, x1 - x0, y1 - y0);
        sx -= x0;
        sy -= y0;
        active = 0;
        reached = 0;
        for (j = 0; j < full_lines; j++) {
            x = sx;
            c = base;
            cx = bx;
            cy = by;
            for (i = 0; i < line_len; i++) {
                if ((((active == 1 && cx >= 0) && cx < 0x80) && cy >= 0) && cy < 0x80) {
                    m = &mp[c];
                    if (m->wall_n) {
                        if (myplayer.shadows_on) {
                            if (!(m->floor & 0x20))
                                LbSpriteDraw(x, sy - blocks[m->wall_n].info->height, blocks[m->wall_n].info);
                            if (m->floor & 0x20) {
                                if (m->wall_n == InteriorWallHalf.block) {
                                    LbSpriteDraw(x, sy - InteriorWallHalf.sprite.height, &InteriorWallHalf.sprite);
                                } else {
                                    overlay.width = blocks[m->wall_n].info->width;
                                    overlay.height = blocks[m->wall_n].info->height;
                                    overlay.data = (u32)ShadowBuffer;
                                    create_overlay_sprite(blocks[m->wall_n].info, blocks[150].info, &overlay);
                                    LbSpriteDraw(x, sy - overlay.height, &overlay);
                                }
                            }
                        } else {
                            LbSpriteDraw(x, sy - blocks[m->wall_n].info->height, blocks[m->wall_n].info);
                        }
                    }
                    if (m->wall_w)
                        LbSpriteDraw(x, sy - blocks[m->wall_w].info->height, blocks[m->wall_w].info);
                }
                if (c == cell)
                    reached = 1;
                x += resvars.tile_width;
                cx++;
                cy--;
                c += -0x7f;
            }
            if (reached == 1)
                active = 1;
            sx += dx;
            sy += half;
            px += dx;
            py += half;
            if (dx < 0) {
                by++;
                base += 0x80;
                line_len += step;
            } else if (dx > 0) {
                bx++;
                base++;
                line_len -= step;
            }
            dx = -dx;
        }
    }
}

// Masks out in the pixel buffer (colour 0xfe) the walls within x0,y0 to x1,y1
// that lie in front of map cell lx,ly, for drawing a thing behind them.
// FUNCTION: 0x000a8a8c
void draw_window_into_pix_buffer(const u8 win, s32 x0, s32 y0, s32 x1, s32 y1, const s32 lx, const s32 ly)
{
    Map *mp;
    s32 line_len;
    s32 floor_lines;
    s32 full_lines;
    s32 step;
    s32 base;
    s32 bx;
    s32 by;
    s32 px;
    s32 py;
    s32 dx;
    s32 half;
    s32 ax;
    s32 ay;
    s32 j;
    s32 x;
    s32 sx;
    s32 c;
    s32 cx;
    s32 cy;
    s32 i;
    Map *m;

    mp = map;
    setup_stats_for_window_scan(win, x0, y0, x1, y1);
    line_len = info.line_len;
    floor_lines = info.floor_lines;
    full_lines = info.full_lines;
    step = info.line_len_step;
    base = info.scan_cell;
    bx = info.scan_col;
    by = info.scan_row;
    px = info.scan_x;
    py = info.scan_y;
    dx = info.scan_dx;
    half = resvars.tile_height / 2;
    give_absolute_xy_of_cell_at_column_row(bx, by);
    ax = info.cell_x;
    ay = info.cell_y;
    give_absolute_xy_of_cell_at_column_row(lx, ly);
    ax -= info.cell_x;
    ay -= info.cell_y;
    for (j = 0; j < full_lines; j++) {
        x = ax;
        sx = px;
        c = base;
        cx = bx;
        cy = by;
        for (i = 0; i < line_len; i++) {
            if ((((cx >= 0 && cx < 0x80) && cy >= 0) && cy < 0x80) && (cx > lx || cy > ly)) {
                m = &mp[c];
                if (m->wall_n)
                    LbSpriteDrawOneColour(0x5a - resvars.tile_width / 2 + x,
                                          ay - blocks[m->wall_n].info->height + 0xb4 - resvars.tile_height * 3 / 2 +
                                              resvars.tile_height,
                                          blocks[m->wall_n].info, 0xfe);
                if (m->wall_w)
                    LbSpriteDrawOneColour(0x5a - resvars.tile_width / 2 + x,
                                          ay - blocks[m->wall_w].info->height + 0xb4 - resvars.tile_height * 3 / 2 +
                                              resvars.tile_height,
                                          blocks[m->wall_w].info, 0xfe);
            }
            x += resvars.tile_width;
            sx += resvars.tile_width;
            cx++;
            cy--;
            c += -0x7f;
        }
        ax += dx;
        ay += half;
        px += dx;
        py += half;
        if (dx < 0) {
            by++;
            base += 0x80;
            line_len += step;
        } else if (dx > 0) {
            bx++;
            base++;
            line_len -= step;
        }
        dx = -dx;
    }
}

// Draws a border (colour 3) around view window win's zone, clipped to x0,y0
// to x1,y1.
// FUNCTION: 0x000a8d98
void draw_boundry(const u8 win, s32 x0, s32 y0, s32 x1, s32 y1)
{
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;

    find_common_window(x0, y0, x1, y1, window.zone_x[win], window.zone_y[win], window.zone_x1[win],
                       window.zone_y1[win]);
    find_common_window(info.clip_x0, info.clip_y0, info.clip_x1, info.clip_y1, window.clip_x0, window.clip_y0,
                       window.clip_x1, window.clip_y1);
    LbScreenSetGraphicsWindow(info.clip_x0, info.clip_y0, info.clip_x1 - info.clip_x0, info.clip_y1 - info.clip_y0);
    left = 0;
    top = 0;
    right = window.zone_x1[win] - window.zone_x[win] - 1;
    bottom = window.zone_y1[win] - window.zone_y[win] - 1;
    if (window.zone_x[win] < info.clip_x0) {
        left = window.zone_x[win] - info.clip_x0;
        right += left;
    }
    if (window.zone_y[win] < info.clip_y0) {
        top = window.zone_y[win] - info.clip_y0;
        bottom += top;
    }
    LbDrawLine(left, top, right, top, 3);
    LbDrawLine(right, top, right, bottom, 3);
    LbDrawLine(right, bottom, left, bottom, 3);
    LbDrawLine(left, bottom, left, top, 3);
}

// Sets map flag 0x20 on cell if anything on it covers more than one cell (an
// object larger than a tile, or a person using an object); else clears it.
// FUNCTION: 0x000a8f78
void adjust_big_cell_flag(const u16 cell)
{
    Thing *t;
    Thing *obj;
    u16 shape;

    for (t = &thing[map[cell].first]; t > thing; t = &thing[t->cell_next]) {
        switch (t->type) {
        case THING_PATIENT:
        case THING_STAFF:
            obj = &thing[t->object];
            if (obj > thing && (u16)(t - thing) == obj->user) {
                map[cell].flags |= MAP_BIG_CELL;
                return;
            }
            break;
        case THING_OBJECT:
            shape = objects[t->subtype].shapes[t->facing];
            if (shapes[shape].count > 1) {
                map[cell].flags |= MAP_BIG_CELL;
                return;
            }
            break;
        }
    }
    map[cell].flags &= ~MAP_BIG_CELL;
}

// Returns the scan cell that already covers the position x,y relative to
// row's cell (covers table), or -1 if none other than row does.
// FUNCTION: 0x000a9100
s8 check_obscuring_column(const s8 row, s8 x, s8 y)
{
    s8 idx;
    s8 col;
    s32 unused;

    idx = row + (x - y >= 0 ? (x - y) / 2 : (x - y - 1) / 2);
    col = x + y;
    col = -col;
    if (((idx < 0 || idx > 14) || col < 0) || col > 4)
        return -1;
    for (; col >= 0; col += -2) {
        s8 v;

        v = covers[idx][col];
        if (v == -1 || v == row)
            continue;
        return v;
    }
    return -1;
}

// Records scan cell row as covering the position pos relative to it (y * 128
// + x), unless another cell already covers it or a neighbour: then returns
// that cell, else -1.
// FUNCTION: 0x000a91f0
s8 record_covering_cell(const s8 row, const s16 pos)
{
    s8 x;
    s8 y;
    s8 idx;
    s8 col;
    s8 v;

    x = pos % 128;
    y = pos / 128;
    if (x < -0x40) {
        x += 0x80;
        y += -1;
    }
    idx = row + (x - y >= 0 ? (x - y) / 2 : (x - y - 1) / 2);
    col = x + y;
    col = -col;
    if (((idx < 0 || idx > 14) || col < 0) || col > 4)
        return -1;
    v = check_obscuring_column(row, x, y + 1);
    if (v >= 0 || (v = check_obscuring_column(row, x + 1, y)) >= 0 ||
        (v = check_obscuring_column(row, x + 1, y + 1)) >= 0)
        return v;
    covers[idx][col] = row;
    return -1;
}

// Records the cells covered by the things in a cell's list starting at t, for
// scan cell row. Returns a scan cell that must be drawn before row (one whose
// things overlap), or -1.
// FUNCTION: 0x000a9358
s8 compute_cell_precedence(const s8 row, Thing *t)
{
    Thing *obj;
    s8 v;

    for (; t > thing; t = &thing[t->cell_next]) {
        switch (t->type) {
        case THING_STAFF:
            if (t->state == 0x2a || t->state == SS_APPLICANT)
                break;
            obj = &thing[t->object];
            if (obj > thing && (u16)(t - thing) == obj->user) {
                u16 shape;
                s16 off;
                u8 k;
                s16 pos;

                shape = objects[obj->subtype].shapes[obj->facing];
                record_covering_cell(row, 0);
                off = (s16) - (s8)shapes[shape].use_xy_depth[0] + -(s8)shapes[shape].use_xy_depth[1] * 128;
                for (k = 0; k < shapes[shape].count; k++) {
                    pos = shapes[shape].points[k] + off;
                    v = record_covering_cell(row, pos);
                    if (v >= 0)
                        return v;
                }
                break;
            }
            v = record_covering_cell(row, 0);
            if (v >= 0)
                return v;
            break;
        case THING_PATIENT:
            if (t->state == PS_MERGED)
                break;
            obj = &thing[t->object];
            if (obj > thing && (u16)(t - thing) == obj->user) {
                u16 shape;
                s16 off;
                u8 k;
                s16 pos;

                shape = objects[obj->subtype].shapes[obj->facing];
                record_covering_cell(row, 0);
                off = (s16) - (s8)shapes[shape].use_xy_depth[0] + -(s8)shapes[shape].use_xy_depth[1] * 128;
                for (k = 0; k < shapes[shape].count; k++) {
                    pos = shapes[shape].points[k] + off;
                    v = record_covering_cell(row, pos);
                    if (v >= 0)
                        return v;
                }
                break;
            }
            v = record_covering_cell(row, 0);
            if (v >= 0)
                return v;
            break;
        case THING_OBJECT: {
            u16 shape;
            u8 k;
            s16 pos;

            shape = objects[t->subtype].shapes[t->facing];
            if (t->state == OS_IN_USE)
                break;
            for (k = 0; k < shapes[shape].count; k++) {
                pos = shapes[shape].points[k];
                v = record_covering_cell(row, pos);
                if (v >= 0)
                    return v;
            }
            break;
        }
        }
    }
    return -1;
}

// Draws view window win within x0,y0 to x1,y1 straight onto the work screen:
// floor (with shadows if enabled, myplayer.shadows_on), then walls and things row by
// row.
// FUNCTION: 0x000a9744
void draw_all_mk7_selective_sprites_wscreen(const u8 win, s32 x0, s32 y0, s32 x1, s32 y1)
{
    Map *mp;
    s32 line_len;
    s32 floor_lines;
    s32 full_lines;
    s32 step;
    s32 base;
    s32 bx;
    s32 by;
    s32 px;
    s32 py;
    s32 dx;
    s32 half;
    s32 sx;
    s32 sy;
    s32 line_len0;
    s32 px0;
    s32 py0;
    s32 sx0;
    s32 sy0;
    s32 step0;
    s32 dx0;
    s32 bx0;
    s32 by0;
    s32 base0;
    s32 minx;
    s32 miny;
    s32 j;
    s32 x;
    s32 wx;
    s32 c;
    s32 cx;
    s32 cy;
    s32 i;
    Map *m;
    u8 tile;
    TbSprite overlay;
    s32 ppx;
    s32 ppy;
    s32 b0;
    s32 b1;
    s32 b2;
    s32 b3;

    mp = map;
    find_common_window(x0, y0, x1, y1, window.zone_x[win], window.zone_y[win], window.zone_x1[win],
                       window.zone_y1[win]);
    if (info.clip_empty != 0)
        return;
    x0 = info.clip_x0;
    y0 = info.clip_y0;
    x1 = info.clip_x1;
    y1 = info.clip_y1;
    setup_stats_for_window_scan(win, x0, y0, x1, y1);
    line_len = info.line_len;
    floor_lines = info.floor_lines;
    full_lines = info.full_lines;
    step = info.line_len_step;
    base = info.scan_cell;
    bx = info.scan_col;
    by = info.scan_row;
    px = info.scan_x;
    py = info.scan_y;
    dx = info.scan_dx;
    half = resvars.tile_height / 2;
    sx = px;
    sy = py;
    LbScreenSetGraphicsWindow(x0, y0, x1 - x0, y1 - y0);
    sx -= x0;
    sy -= y0;
    line_len0 = line_len;
    px0 = px;
    py0 = py;
    sx0 = sx;
    sy0 = sy;
    step0 = step;
    dx0 = dx;
    bx0 = bx;
    by0 = by;
    base0 = base;
    minx = 0;
    miny = 0;
    if (window.zone_x[win] > 0)
        minx = window.zone_x[win];
    if (window.zone_y[win] > 0)
        miny = window.zone_y[win];
    if (window.clip_x0 > minx)
        minx = window.clip_x0;
    if (window.clip_y0 > miny)
        miny = window.clip_y0;
    for (j = 0; j < floor_lines; j++) {
        x = sx;
        wx = px;
        c = base;
        cx = bx;
        cy = by;
        for (i = 0; i < line_len; i++) {
            if ((((cx >= 0 && cx < 0x80) && cy >= 0) && cy < 0x80)) {
                m = &mp[c];
                tile = m->overlay;
                if (myplayer.shadows_on) {
                    if (!(m->floor & 0x10) && !(m->floor & 0x40)) {
                        LbSpriteDraw(x, sy - blocks[tile].info->height, blocks[tile].info);
                    } else if (m->floor & 0x40) {
                        if (m->overlay == InteriorTileFull.block) {
                            LbSpriteDraw(x, sy - InteriorTileFull.sprite.height, &InteriorTileFull.sprite);
                        } else if (m->overlay == RoomTileFull[0].block) {
                            LbSpriteDraw(x, sy - RoomTileFull[0].sprite.height, &RoomTileFull[0].sprite);
                        } else if (m->overlay == RoomTileFull[1].block) {
                            LbSpriteDraw(x, sy - RoomTileFull[1].sprite.height, &RoomTileFull[1].sprite);
                        } else if (m->overlay == RoomTileFull[2].block) {
                            LbSpriteDraw(x, sy - RoomTileFull[2].sprite.height, &RoomTileFull[2].sprite);
                        } else if (m->overlay == RoomTileFull[3].block) {
                            LbSpriteDraw(x, sy - RoomTileFull[3].sprite.height, &RoomTileFull[3].sprite);
                        } else {
                            overlay.width = blocks[m->overlay].info->width;
                            overlay.height = blocks[m->overlay].info->height;
                            overlay.data = (u32)ShadowBuffer;
                            create_overlay_sprite(blocks[m->overlay].info, blocks[73].info, &overlay);
                            LbSpriteDraw(x, sy - overlay.height, &overlay);
                        }
                    } else if (m->floor & 0x10) {
                        if (m->overlay == InteriorTileHalf.block) {
                            LbSpriteDraw(x, sy - InteriorTileHalf.sprite.height, &InteriorTileHalf.sprite);
                        } else if (m->overlay == RoomTileHalf[0].block) {
                            LbSpriteDraw(x, sy - RoomTileHalf[0].sprite.height, &RoomTileHalf[0].sprite);
                        } else if (m->overlay == RoomTileHalf[1].block) {
                            LbSpriteDraw(x, sy - RoomTileHalf[1].sprite.height, &RoomTileHalf[1].sprite);
                        } else if (m->overlay == RoomTileHalf[2].block) {
                            LbSpriteDraw(x, sy - RoomTileHalf[2].sprite.height, &RoomTileHalf[2].sprite);
                        } else if (m->overlay == RoomTileHalf[3].block) {
                            LbSpriteDraw(x, sy - RoomTileHalf[3].sprite.height, &RoomTileHalf[3].sprite);
                        } else {
                            overlay.width = blocks[m->overlay].info->width;
                            overlay.height = blocks[m->overlay].info->height;
                            overlay.data = (u32)ShadowBuffer;
                            create_overlay_sprite(blocks[m->overlay].info, blocks[74].info, &overlay);
                            LbSpriteDraw(x, sy - overlay.height, &overlay);
                        }
                    }
                } else {
                    LbSpriteDraw(x, sy - blocks[tile].info->height, blocks[tile].info);
                }
            } else {
                LbSpriteDraw(x, sy - blocks[4].info->height, blocks[4].info);
            }
            x += resvars.tile_width;
            wx += resvars.tile_width;
            cx++;
            cy--;
            c += -0x7f;
        }
        sx += dx;
        sy += half;
        px += dx;
        py += half;
        if (dx < 0) {
            by++;
            base += 0x80;
            line_len += step;
        } else if (dx > 0) {
            bx++;
            base++;
            line_len -= step;
        }
        dx = -dx;
    }
    px = px0;
    py = py0;
    sx = sx0;
    sy = sy0;
    step = step0;
    dx = dx0;
    line_len = line_len0;
    bx = bx0;
    by = by0;
    base = base0;
    for (j = 0; j < full_lines; j++) {
        x = sx;
        wx = px;
        c = base;
        cx = bx;
        cy = by;
        for (i = 0; i < line_len; i++) {
            if ((((cx >= 0 && cx < 0x80) && cy >= 0) && cy < 0x80)) {
                m = &mp[c];
                if (m->wall_n) {
                    if (myplayer.shadows_on) {
                        if (!(m->floor & 0x20)) {
                            LbSpriteDraw(x, sy - blocks[m->wall_n].info->height, blocks[m->wall_n].info);
                        } else if (m->floor & 0x20) {
                            if (m->wall_n == InteriorWallHalf.block) {
                                LbSpriteDraw(x, sy - InteriorWallHalf.sprite.height, &InteriorWallHalf.sprite);
                            } else if (m->wall_n == InteriorWallHalf.block) {
                                LbSpriteDraw(x, sy - InteriorWallHalf.sprite.height, &InteriorWallHalf.sprite);
                            } else {
                                overlay.width = blocks[m->wall_n].info->width;
                                overlay.height = blocks[m->wall_n].info->height;
                                overlay.data = (u32)ShadowBuffer;
                                create_overlay_sprite(blocks[m->wall_n].info, blocks[150].info, &overlay);
                                LbSpriteDraw(x, sy - blocks[150].info->height, &overlay);
                            }
                        }
                    } else {
                        LbSpriteDraw(x, sy - blocks[m->wall_n].info->height, blocks[m->wall_n].info);
                    }
                }
                if (m->wall_w)
                    LbSpriteDraw(x, sy - blocks[m->wall_w].info->height, blocks[m->wall_w].info);
            } else {
                break;
            }
            x += resvars.tile_width;
            wx += resvars.tile_width;
            cx++;
            cy--;
            c += -0x7f;
        }
        x = sx;
        wx = px;
        c = base;
        cx = bx;
        cy = by;
        for (i = 0; i < line_len; i++) {
            if ((((cx >= 0 && cx < 0x80) && cy >= 0) && cy < 0x80)) {
                m = &mp[c];
                if ((m->first || (m->floor & 0xf)) || (m->flags & 0xc0)) {
                    if (!(m->flags & 0x10)) {
                        do_mapwho(m, x, sy - resvars.tile_height, 0, 0);
                    } else {
                        DrawIntoPixBuffer();
                        do_mapwho(m, 0x5a - resvars.tile_width / 2, 0xb4 - resvars.tile_height * 3 / 2, 0, 0);
                        ppx = D_001567b0[0];
                        ppy = D_001567b0[1];
                        D_001567b0[0] += x - (0x5a - resvars.tile_width / 2);
                        D_001567b0[1] += sy - 0xb4 + resvars.tile_height * 3 / 2 - resvars.tile_height;
                        D_001567b0[2] += x - (0x5a - resvars.tile_width / 2);
                        D_001567b0[3] += sy - 0xb4 + resvars.tile_height * 3 / 2 - resvars.tile_height;
                        pixBuf.x = x - 0x5a + resvars.tile_width / 2;
                        pixBuf.y = sy - 0xb4 + resvars.tile_height * 3 / 2 - resvars.tile_height;
                        find_common_window(D_001567b0[0] + minx, D_001567b0[1] + miny, D_001567b0[2] + minx,
                                           D_001567b0[3] + miny, x0, y0, x1, y1);
                        b0 = info.clip_x0;
                        b1 = info.clip_y0;
                        b2 = info.clip_x1;
                        b3 = info.clip_y1;
                        if (info.clip_empty == 0) {
                            draw_window_into_pix_buffer(win, b0, b1, b2, b3, cx, cy);
                            if (win == 0)
                                set_all_refresh_zones_in_window(b0, b1, b2, b3, 3);
                        }
                        RestoreFromPixBuffer();
                        LbScreenSetGraphicsWindow(x0, y0, x1 - x0, y1 - y0);
                        DrawPixBuffer(0, ppx, ppy, D_001567b0[2] - D_001567b0[0], D_001567b0[3] - D_001567b0[1]);
                        FillBoxInPixBuffer(0xfe, ppx, ppy, D_001567b0[2] - D_001567b0[0],
                                           D_001567b0[3] - D_001567b0[1]);
                    }
                }
            }
            x += resvars.tile_width;
            wx += resvars.tile_width;
            cx++;
            cy--;
            c += -0x7f;
        }
        sx += dx;
        sy += half;
        px += dx;
        py += half;
        if (dx < 0) {
            by++;
            base += 0x80;
            line_len += step;
        } else if (dx > 0) {
            bx++;
            base++;
            line_len -= step;
        }
        dx = -dx;
    }
}

// Draws the floor and walls of view window win within x0,y0 to x1,y1 into the
// background buffer (bscreen), wrapping at its edges, with shadows if enabled.
// FUNCTION: 0x000aa67c
void draw_all_mk7_no_sprites_bscreen(const u8 win, s32 x0, s32 y0, s32 x1, s32 y1)
{
    s32 cy;
    Map *mp;
    s32 line_len;
    s32 floor_lines;
    s32 full_lines;
    s32 step;
    s32 base;
    s32 bx;
    s32 by;
    s32 px;
    s32 py;
    s32 dx;
    s32 half;
    s32 sx;
    s32 sy;
    s32 line_len0;
    s32 px0;
    s32 py0;
    s32 sx0;
    s32 sy0;
    s32 step0;
    s32 dx0;
    s32 bx0;
    s32 by0;
    s32 base0;
    s32 j;
    s32 x;
    s32 c;
    s32 cx;
    u8 tile;
    s32 i;
    Map *m;
    TbSprite overlay;

    mp = map;
    find_common_window(x0, y0, x1, y1, window.zone_x[win], window.zone_y[win], window.zone_x1[win],
                       window.zone_y1[win]);
    if (info.clip_empty != 0)
        return;
    x0 = info.clip_x0;
    y0 = info.clip_y0;
    x1 = info.clip_x1;
    y1 = info.clip_y1;
    setup_stats_for_window_scan(win, x0, y0, x1, y1);
    line_len = info.line_len;
    floor_lines = info.floor_lines;
    full_lines = info.full_lines;
    step = info.line_len_step;
    base = info.scan_cell;
    bx = info.scan_col;
    by = info.scan_row;
    px = info.scan_x;
    py = info.scan_y;
    dx = info.scan_dx;
    half = resvars.tile_height / 2;
    sx = px;
    sy = py;
    LbScreenSetGraphicsWindow(x0, y0, x1 - x0, y1 - y0);
    sx -= x0;
    sy -= y0;
    line_len0 = line_len;
    px0 = px;
    py0 = py;
    sx0 = sx;
    sy0 = sy;
    step0 = step;
    dx0 = dx;
    bx0 = bx;
    by0 = by;
    base0 = base;
    othervar.saved_screen_ptr = lbDisplay.wscreen;
    othervar.saved_screen_width = lbDisplay.graphics_screen_width;
    othervar.saved_screen_height = lbDisplay.graphics_screen_height;
    lbDisplay.wscreen = bscreen;
    lbDisplay.graphics_screen_width = lbDisplay.physical_screen_width;
    lbDisplay.graphics_screen_height = lbDisplay.physical_screen_height;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
    for (j = floor_lines; --j >= 0;) {
        x = px;
        c = base;
        cx = bx;
        cy = by;
        for (i = line_len; --i >= 0;) {
            if ((((cx >= 0 && cx < 0x80) && cy >= 0) && cy < 0x80)) {
                m = &mp[c];
                tile = m->overlay;
                if (myplayer.shadows_on) {
                    if (!(m->floor & 0x10) && !(m->floor & 0x40)) {
                        draw_a_clipped_four_zone_floor_tile(x, py, blocks[tile].info, x0, y0, x1, y1);
                    } else if (m->floor & 0x40) {
                        if (m->overlay == InteriorTileFull.block) {
                            draw_a_clipped_four_zone_floor_tile(x, py, &InteriorTileFull.sprite, x0, y0, x1, y1);
                        } else if (m->overlay == RoomTileFull[0].block) {
                            draw_a_clipped_four_zone_floor_tile(x, py, &RoomTileFull[0].sprite, x0, y0, x1, y1);
                        } else if (m->overlay == RoomTileFull[1].block) {
                            draw_a_clipped_four_zone_floor_tile(x, py, &RoomTileFull[1].sprite, x0, y0, x1, y1);
                        } else if (m->overlay == RoomTileFull[2].block) {
                            draw_a_clipped_four_zone_floor_tile(x, py, &RoomTileFull[2].sprite, x0, y0, x1, y1);
                        } else if (m->overlay == RoomTileFull[3].block) {
                            draw_a_clipped_four_zone_floor_tile(x, py, &RoomTileFull[3].sprite, x0, y0, x1, y1);
                        } else {
                            overlay.width = blocks[m->overlay].info->width;
                            overlay.height = blocks[m->overlay].info->height;
                            overlay.data = (u32)ShadowBuffer;
                            create_overlay_sprite(blocks[m->overlay].info, blocks[73].info, &overlay);
                            draw_a_clipped_four_zone_floor_tile(x, py, &overlay, x0, y0, x1, y1);
                        }
                    } else if (m->floor & 0x10) {
                        if (m->overlay == InteriorTileHalf.block) {
                            draw_a_clipped_four_zone_floor_tile(x, py, &InteriorTileHalf.sprite, x0, y0, x1, y1);
                        } else if (m->overlay == RoomTileHalf[0].block) {
                            draw_a_clipped_four_zone_floor_tile(x, py, &RoomTileHalf[0].sprite, x0, y0, x1, y1);
                        } else if (m->overlay == RoomTileHalf[1].block) {
                            draw_a_clipped_four_zone_floor_tile(x, py, &RoomTileHalf[1].sprite, x0, y0, x1, y1);
                        } else if (m->overlay == RoomTileHalf[2].block) {
                            draw_a_clipped_four_zone_floor_tile(x, py, &RoomTileHalf[2].sprite, x0, y0, x1, y1);
                        } else if (m->overlay == RoomTileHalf[3].block) {
                            draw_a_clipped_four_zone_floor_tile(x, py, &RoomTileHalf[3].sprite, x0, y0, x1, y1);
                        } else {
                            overlay.width = blocks[m->overlay].info->width;
                            overlay.height = blocks[m->overlay].info->height;
                            overlay.data = (u32)ShadowBuffer;
                            create_overlay_sprite(blocks[m->overlay].info, blocks[74].info, &overlay);
                            draw_a_clipped_four_zone_floor_tile(x, py, &overlay, x0, y0, x1, y1);
                        }
                    }
                } else {
                    draw_a_clipped_four_zone_floor_tile(x, py, blocks[tile].info, x0, y0, x1, y1);
                }
            } else {
                draw_a_clipped_four_zone_floor_tile(x, py, blocks[4].info, x0, y0, x1, y1);
            }
            x += resvars.tile_width;
            cx++;
            cy--;
            c += -0x7f;
        }
        sx += dx;
        sy += half;
        px += dx;
        py += half;
        if (dx < 0) {
            by++;
            base += 0x80;
            line_len += step;
        } else if (dx > 0) {
            bx++;
            base++;
            line_len -= step;
        }
        dx = -dx;
    }
    px = px0;
    py = py0;
    step = step0;
    dx = dx0;
    line_len = line_len0;
    bx = bx0;
    by = by0;
    base = base0;
    for (j = full_lines; --j >= 0;) {
        x = px;
        c = base;
        cx = bx;
        cy = by;
        for (i = line_len; --i >= 0;) {
            if ((((cx >= 0 && cx < 0x80) && cy >= 0) && cy < 0x80)) {
                m = &mp[c];
                if (m->wall_n) {
                    if (myplayer.shadows_on) {
                        if (!(m->floor & 0x20)) {
                            tile = blocks[m->wall_n].data;
                            draw_a_clipped_four_zone_wall_0(x, py, blocks[m->wall_n].info, x0, y0, x1, y1);
                        } else if (m->floor & 0x20) {
                            if (m->wall_n == InteriorWallHalf.block) {
                                draw_a_clipped_four_zone_wall_0(x, py, &InteriorWallHalf.sprite, x0, y0, x1, y1);
                            } else {
                                tile = blocks[m->wall_n].data;
                                overlay.width = blocks[m->wall_n].info->width;
                                overlay.height = blocks[m->wall_n].info->height;
                                overlay.data = (u32)ShadowBuffer;
                                create_overlay_sprite(blocks[m->wall_n].info, blocks[150].info, &overlay);
                                draw_a_clipped_four_zone_wall_0(x, py, &overlay, x0, y0, x1, y1);
                            }
                        }
                    } else {
                        tile = blocks[m->wall_n].data;
                        draw_a_clipped_four_zone_wall_0(x, py, blocks[m->wall_n].info, x0, y0, x1, y1);
                    }
                }
                if (m->wall_w) {
                    tile = blocks[m->wall_w].data;
                    draw_a_clipped_four_zone_wall_1(x, py, c, x0, y0, x1, y1);
                }
                x += resvars.tile_width;
                cx++;
                cy--;
                c += -0x7f;
            } else {
                break;
            }
        }
        px += dx;
        py += half;
        if (dx < 0) {
            by++;
            base += 0x80;
            line_len += step;
        } else if (dx > 0) {
            bx++;
            base++;
            line_len -= step;
        }
        dx = -dx;
    }
    lbDisplay.wscreen = othervar.saved_screen_ptr;
    lbDisplay.graphics_screen_width = othervar.saved_screen_width;
    lbDisplay.graphics_screen_height = othervar.saved_screen_height;
    LbScreenSetGraphicsWindow(0, 0, othervar.saved_screen_width, othervar.saved_screen_height);
}
