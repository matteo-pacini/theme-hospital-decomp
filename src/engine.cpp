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

void draw_a_clipped_four_zone_floor_tile(s32 x, s32 y, const TbSprite *const sprite, s32 x0, s32 y0, s32 x1, s32 y1);
void draw_a_clipped_four_zone_wall_0(s32 x, s32 y, const TbSprite *const sprite, s32 x0, s32 y0, s32 x1, s32 y1);
void draw_a_clipped_four_zone_wall_1(s32 x, s32 y, const s32 cell, s32 x0, s32 y0, s32 x1, s32 y1);
void give_absolute_xy_of_cell_at_column_row(const s32 x, const s32 y);
void create_overlay_sprite(const TbSprite *sprite, const TbSprite *shadow, const TbSprite *out);

void process_tracking_windows(void);
s8 compute_cell_precedence(const s8 i, Thing *w);
s8 check_obscuring_column(const s8 row, s8 x, s8 y);
s8 record_covering_cell(const s8 row, const s16 pos);
void setup_stats_for_window_scan(const u8 n, s32 x, const s32 y, s32 w, const s32 h);
void do_cell(const u8 n, const s32 x, const s32 y, const s32 z, const s32 w, const s32 cell, const s32 wx, const s32 wy,
             const s32 cx0, const s32 cy0, const s32 cx1, const s32 cy1);
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
void copy_bscreen_window_to_wscreen_mk1(const s32 x, const s32 y, const s32 w, const s32 h, const s16 a, const s16 b);
void draw_window_not_using_bscreen(const u8 n);
void remove_old_window_image_from_wscreen_replacing_detail(s32 x0, s32 y0, s32 w0, s32 h0, s32 x1, s32 y1, s32 w1, s32 h1);
void remove_old_window_image_from_wscreen(s32 x0, s32 y0, s32 w0, s32 h0, s32 x1, s32 y1, s32 w1, s32 h1);
void draw_all_sprites_on_wscreen_selectively_using_pix_buffer(const u8 n, s32 x, s32 y, s32 w, s32 h);
void draw_boundry(const u8 n, s32 x, s32 y, s32 w, s32 h);
void draw_all_mk7_selective_sprites_wscreen(const u8 n, s32 x, s32 y, s32 w, s32 h);
void draw_all_mk7_no_sprites_bscreen(const u8 n, s32 x, s32 y, s32 w, s32 h);
void draw_window_into_pix_buffer(const u8 n, s32 x0, s32 y0, s32 x1, s32 y1, const s32 a, const s32 b);
void draw_window_on_wscreen_mk2(const u8 n, s32 x0, s32 y0, s32 x1, s32 y1, const s32 cell);

extern "C" {
extern s16 D_001567b0[8];
extern CellLink D_00157dc4[];
void fn_000c9be0(s32 x0, s32 y0, s32 x1, s32 y1, u8 colour);
void LbDrawHVLine(s32 x, s32 y, TbSprite *sprite, s32 colour);
void fn_000a6c08(void);
void fn_000a6c7c(void);
void fn_000a6ca4(void);
void fn_000a6cf0(u8 n);
void fn_000a70d0(u8 n);
}

// FUNCTION: 0x000a6b20
void draw_it(void)
{
    window.unknown2cf = statvar.unknown4;
    window.unknown2c3 = 0;
    window.unknown2c7 = 0;
    window.unknown2cb = lbDisplay.unknown18;
    process_tracking_windows();
    othervar.f41 = 0;
    othervar.f6f = othervar.f71 = 0;
    if (window.unknown2ec > 0) {
        animate_windows();
        move_windows();
        fn_000a6c7c();
    }
    draw_overdrawn_blueprints();
    update_panel();
    draw_menus();
    draw_popup_icons();
    draw_emergency();
    if (!debug.unknownd7)
        compute_help_text(0, 0, 3);
    if (gamevar.f12e == 1)
        draw_cell_no(othervar.f28, color.unknownD);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_map_cur_and_mcur_x_and_y();
}

// FUNCTION: 0x000a6c08
void fn_000a6c08(void)
{
    u8 i;

    for (i = 0; i < window.unknown2ec; i++)
        fn_000a70d0(i);
    for (i = 0; i < window.unknown2ec; i++) {
        if (i != window.f2ea)
            draw_window_not_using_bscreen(i);
        else
            fn_000a6cf0(i);
    }
}

// FUNCTION: 0x000a6c7c
void fn_000a6c7c(void)
{
    u8 i = 0;

    fn_000a6cf0(window.f2ea);
}

// FUNCTION: 0x000a6ca4
void fn_000a6ca4(void)
{
    u8 i = 0;

    for (i = 0; i < window.unknown2ec; i++) {
        if (i != window.f2ea)
            draw_window_not_using_bscreen(i);
    }
}

// FUNCTION: 0x000a6cf0
void fn_000a6cf0(u8 n)
{
    if ((((window.fd8[n] == 0 && window.fdb[n] == 0) && window.fe1[n] == window.fde[n]) && window.fde[n] == 0) && !othervar.f10) {
        refresh_wscreen();
    } else {
        s32 x;
        s32 y;
        s32 w;
        s32 h;
        u8 flag;

        find_common_window(window.zone_x[n], window.zone_y[n], window.zone_w[n], window.zone_h[n],
                           window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
        x = info.unknown68;
        y = info.unknown6c;
        w = info.unknown70;
        h = info.unknown74;
        flag = info.unknown78;
        refresh.f0 = 0;
        refresh.f2 = 0;
        draw_all_mk7_no_sprites_bscreen(0, 0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        find_common_window(window.f60[n], window.f6c[n], window.f78[n], window.f84[n],
                           window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
        if (info.unknown78 == 0)
            set_all_refresh_zones_in_window(info.unknown68, info.unknown6c, info.unknown70, info.unknown74, 1);
        find_common_window(window.zone_x[n], window.zone_y[n], window.zone_w[n], window.zone_h[n],
                           window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
        if (info.unknown78 == 0)
            set_all_refresh_zones_in_window(info.unknown68, info.unknown6c, info.unknown70, info.unknown74, 1);
        remove_old_window_image_from_wscreen(window.f60[n], window.f6c[n], window.f78[n], window.f84[n],
                                             window.zone_x[n], window.zone_y[n], window.zone_w[n], window.zone_h[n]);
        if (flag == 0)
            copy_bscreen_window_to_wscreen_mk1(x, y, w, h, refresh.f0, refresh.f2);
    }
    draw_all_sprites_on_wscreen_selectively_using_pix_buffer(0, window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
    othervar.f10 = 0;
    window.f18[n] = window.f0[n];
    window.f24[n] = window.fc[n];
    window.f60[n] = window.zone_x[n];
    window.f6c[n] = window.zone_y[n];
    window.f78[n] = window.zone_w[n];
    window.f84[n] = window.zone_h[n];
    window.fe1[n] = window.fde[n];
    window.fdb[n] = 0;
    window.fd8[n] = 0;
}

// FUNCTION: 0x000a70d0
void fn_000a70d0(u8 n)
{
    if (window.fdb[n] == 1) {
        remove_old_window_image_from_wscreen_replacing_detail(window.f60[n], window.f6c[n], window.f78[n], window.f84[n],
                                                              window.zone_x[n], window.zone_y[n], window.zone_w[n], window.zone_h[n]);
        set_all_refresh_zones_in_window(window.f60[n], window.f6c[n], window.f78[n], window.f84[n], 2);
    }
}

// FUNCTION: 0x000a71bc
void draw_window_not_using_bscreen(const u8 n)
{
    set_all_refresh_zones_in_window(window.zone_x[n], window.zone_y[n], window.zone_w[n], window.zone_h[n], 2);
    if (window.fdb[n] == 1)
        set_all_refresh_zones_in_window(window.f60[n], window.f6c[n], window.f78[n], window.f84[n], 3);
    if (window.fde[n] == 0)
        draw_all_mk7_selective_sprites_wscreen(n, window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
    draw_boundry(n, window.zone_x[n], window.zone_y[n], window.zone_w[n], window.zone_h[n]);
    window.f18[n] = window.f0[n];
    window.f24[n] = window.fc[n];
    window.f60[n] = window.zone_x[n];
    window.f6c[n] = window.zone_y[n];
    window.f78[n] = window.zone_w[n];
    window.f84[n] = window.zone_h[n];
    window.fdb[n] = 0;
    window.fd8[n] = 0;
}

// FUNCTION: 0x000a73c0
void remove_old_window_image_from_wscreen_replacing_detail(s32 x0, s32 y0, s32 x1, s32 y1, s32 nx0, s32 ny0, s32 nx1, s32 ny1)
{
    u8 colour;

    colour = color.unknown8;
    find_common_window(x0, y0, x1, y1, window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
    x0 = info.unknown68;
    y0 = info.unknown6c;
    x1 = info.unknown70;
    y1 = info.unknown74;
    find_common_window(nx0, ny0, nx1, ny1, window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
    nx0 = info.unknown68;
    ny0 = info.unknown6c;
    nx1 = info.unknown70;
    ny1 = info.unknown74;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    if (y1 > y0) {
        if (x0 < nx0) {
            LbDrawBox(x0, y0, nx0 - x0, y1 - y0, colour);
            if (window.fde[window.f2ea] == 0) {
                find_common_window(x0, y0, nx0, y1, window.zone_x[window.f2ea], window.zone_y[window.f2ea],
                                   window.zone_w[window.f2ea], window.zone_h[window.f2ea]);
                if (info.unknown78 == 0)
                    copy_bscreen_window_to_wscreen_mk1(info.unknown68, info.unknown6c, info.unknown70, info.unknown74, refresh.f0, refresh.f2);
            }
        }
        if (x1 > nx1) {
            LbDrawBox(nx1, y0, x1 - nx1, y1 - y0, colour);
            if (window.fde[window.f2ea] == 0) {
                find_common_window(nx1, y0, x1, y1, window.zone_x[window.f2ea], window.zone_y[window.f2ea],
                                   window.zone_w[window.f2ea], window.zone_h[window.f2ea]);
                if (info.unknown78 == 0)
                    copy_bscreen_window_to_wscreen_mk1(info.unknown68, info.unknown6c, info.unknown70, info.unknown74, refresh.f0, refresh.f2);
            }
        }
    }
    if (x1 > x0) {
        if (y0 < ny0) {
            LbDrawBox(x0, y0, x1 - x0, ny0 - y0, colour);
            if (window.fde[window.f2ea] == 0) {
                find_common_window(x0, y0, x1, ny0, window.zone_x[window.f2ea], window.zone_y[window.f2ea],
                                   window.zone_w[window.f2ea], window.zone_h[window.f2ea]);
                if (info.unknown78 == 0)
                    copy_bscreen_window_to_wscreen_mk1(info.unknown68, info.unknown6c, info.unknown70, info.unknown74, refresh.f0, refresh.f2);
            }
        }
        if (y1 > ny1) {
            LbDrawBox(x0, ny1, x1 - x0, y1 - ny1, colour);
            if (window.fde[window.f2ea] == 0) {
                find_common_window(x0, ny1, x1, y1, window.zone_x[window.f2ea], window.zone_y[window.f2ea],
                                   window.zone_w[window.f2ea], window.zone_h[window.f2ea]);
                if (info.unknown78 == 0)
                    copy_bscreen_window_to_wscreen_mk1(info.unknown68, info.unknown6c, info.unknown70, info.unknown74, refresh.f0, refresh.f2);
            }
        }
    }
}

// FUNCTION: 0x000a77dc
void remove_old_window_image_from_wscreen(s32 x0, s32 y0, s32 x1, s32 y1, s32 nx0, s32 ny0, s32 nx1, s32 ny1)
{
    u8 colour;

    colour = color.unknown8;
    find_common_window(x0, y0, x1, y1, window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
    x0 = info.unknown68;
    y0 = info.unknown6c;
    x1 = info.unknown70;
    y1 = info.unknown74;
    find_common_window(nx0, ny0, nx1, ny1, window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
    nx0 = info.unknown68;
    ny0 = info.unknown6c;
    nx1 = info.unknown70;
    ny1 = info.unknown74;
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

// FUNCTION: 0x000a7950
void remove_window_from_screen(s32 x, s32 y, s32 w, s32 h)
{
    find_common_window(x, y, w, h, window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
    if (info.unknown78 == 0) {
        set_all_refresh_zones_in_window(info.unknown68, info.unknown6c, info.unknown70, info.unknown74, 3);
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbDrawBox(info.unknown68, info.unknown6c, info.unknown70 - info.unknown68, info.unknown74 - info.unknown6c, color.unknown8);
    }
}

// FUNCTION: 0x000a7a0c
void setup_stats_for_window_scan(const u8 n, s32 x, const s32 y, s32 w, const s32 h)
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
    extra = resvars.f3 * 19 / 10;
    x -= resvars.f4 / 2;
    w += resvars.f4 / 2;
    find_cell_at_point(n, x, y);
    info.fc = info.unknown54;
    info.f14 = info.unknown58;
    info.f4 = info.unknown5c;
    info.f8 = info.unknown60;
    info.f0 = info.unknown64;
    dx = x - info.fc - resvars.f4 / 2;
    if (dx >= 0)
        left = 0;
    dy = y - info.f14 + resvars.f3 / 2;
    if (dy >= 0)
        top = 0;
    span = w - x + (dx - resvars.f4 / 2);
    cnt = span / resvars.f4;
    prod = cnt * resvars.f4;
    if (prod < span) {
        rem = span - prod;
        if (resvars.f4 / 2 >= rem)
            odd = 0;
        cnt++;
    }
    info.f18 = cnt + 1;
    dy -= resvars.f3 / 2;
    span = dy + (h - y);
    cnt = span / resvars.f3;
    prod = cnt * resvars.f3;
    if (prod < span) {
        rem = span - prod;
        if (resvars.f3 / 2 >= rem)
            right = 0;
        cnt++;
    }
    info.f20 = cnt * 2 + 1;
    span = dy + (h + extra - y);
    cnt = span / resvars.f3;
    prod = cnt * resvars.f3;
    if (prod < span) {
        rem = span - prod;
        if (resvars.f3 / 2 >= rem)
            bottom = 0;
        cnt++;
    }
    info.f24 = cnt * 2 + 1;
    info.f10 = resvars.f4 / 2;
    info.f1c = -1;
    if (top) {
        info.f24++;
        info.f20++;
        info.f14 -= resvars.f3 / 2;
        if (left) {
            info.f4--;
            info.f0--;
            info.fc -= resvars.f4 / 2;
            if (odd)
                info.f18++;
            else
                info.f1c = 0;
        } else {
            info.f8--;
            info.f0 -= 0x80;
            info.fc += resvars.f4 / 2;
            info.f10 = resvars.f4 / 2;
            info.f10 = -info.f10;
            if (odd) {
                info.f1c = 0;
            } else {
                info.f1c = 1;
                info.f18--;
            }
        }
    } else if (left) {
        info.f10 = resvars.f4 / 2;
        info.f10 = -info.f10;
        if (odd)
            info.f1c = 1;
        else
            info.f1c = 0;
    } else if (odd) {
        info.f1c = 0;
    }
    if (right)
        info.f20++;
    if (bottom)
        info.f24++;
}

// FUNCTION: 0x000a7df0
void do_cell(const u8 n, const s32 x, const s32 y, const s32 z, const s32 w, const s32 cell, const s32 wx, const s32 wy,
             const s32 cx0, const s32 cy0, const s32 cx1, const s32 cy1)
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
            ox = 0x5a - resvars.f4 / 2;
            oy = 0xb4 - resvars.f3 * 3 / 2;
            DrawIntoPixBuffer();
            do_mapwho(m, ox, oy, cx0 + x - ox, cy0 + y - 0xb4);
            px = D_001567b0[0];
            py = D_001567b0[1];
            D_001567b0[0] += x - ox;
            D_001567b0[1] += y - 0xb4 + resvars.f3 * 3 / 2 - resvars.f3;
            D_001567b0[2] += x - ox;
            D_001567b0[3] += y - 0xb4 + resvars.f3 * 3 / 2 - resvars.f3;
            pixBuf.x = x - ox;
            pixBuf.y = y - 0xb4 + resvars.f3 * 3 / 2 - resvars.f3;
            find_common_window(D_001567b0[0] + wx, D_001567b0[1] + wy, D_001567b0[2] + wx, D_001567b0[3] + wy, cx0, cy0, cx1, cy1);
            if (info.unknown78 == 0) {
                bx0 = info.unknown68;
                by0 = info.unknown6c;
                bx1 = info.unknown70;
                by1 = info.unknown74;
                draw_window_into_pix_buffer(0, bx0, by0, bx1, by1, z, w);
                set_all_refresh_zones_in_window(bx0, by0, bx1, by1, 3);
            }
            RestoreFromPixBuffer();
            LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
            DrawPixBuffer(0, px, py, D_001567b0[2] - D_001567b0[0], D_001567b0[3] - D_001567b0[1]);
            FillBoxInPixBuffer(0xfe, px, py, D_001567b0[2] - D_001567b0[0], D_001567b0[3] - D_001567b0[1]);
        } else {
            LbScreenSetGraphicsWindow(cx0, cy0, cx1 - cx0, cy1 - cy0);
            do_mapwho(m, x, y - resvars.f3, 0, 0);
            find_common_window(D_001567b0[0] + wx, D_001567b0[1] + wy, D_001567b0[2] + wx, D_001567b0[3] + wy, cx0, cy0, cx1, cy1);
            if (info.unknown78 == 0) {
                bx0 = info.unknown68;
                by0 = info.unknown6c;
                bx1 = info.unknown70;
                by1 = info.unknown74;
                draw_window_on_wscreen_mk2(n, bx0, by0, bx1, by1, cell);
                set_all_refresh_zones_in_window(bx0, by0, bx1, by1, 3);
            }
        }
    }
}

// FUNCTION: 0x000a8198
void draw_all_sprites_on_wscreen_selectively_using_pix_buffer(const u8 n, s32 x0, s32 y0, s32 x1, s32 y1)
{
    s32 rows;
    s32 f20;
    s32 count;
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

    find_common_window(x0, y0, x1, y1, window.zone_x[n], window.zone_y[n], window.zone_w[n], window.zone_h[n]);
    if (info.unknown78 == 0) {
        x0 = info.unknown68;
        y0 = info.unknown6c;
        x1 = info.unknown70;
        y1 = info.unknown74;
        setup_stats_for_window_scan(n, x0, y0, x1, y1);
        rows = info.f18;
        f20 = info.f20;
        count = info.f24;
        step = info.f1c;
        base = info.f0;
        bx = info.f4;
        by = info.f8;
        dx = info.f10;
        px = info.fc;
        py = info.f14;
        minx = 0;
        miny = 0;
        LbScreenSetGraphicsWindow(x0, y0, x1 - x0, y1 - y0);
        px -= x0;
        py -= y0;
        if (window.zone_x[n] > 0)
            minx = window.zone_x[n];
        if (window.zone_y[n] > 0)
            miny = window.zone_y[n];
        if (window.unknown2c3 > minx)
            minx = window.unknown2c3;
        if (window.unknown2c7 > miny)
            miny = window.unknown2c7;
        for (j = 0; j < count; j++) {
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
            for (i = 0; i < rows; i++) {
                m = &map[c];
                if ((((x >= 0 && x < 0x80) && y >= 0) && y < 0x80) && ((m->first || (m->floor & 0xf)) || (m->flags & 0xc0))) {
                    if (m->flags & 0x20) {
                        s8 prec;

                        prec = compute_cell_precedence(i, &thing[m->first]);
                        if (prec >= 0) {
                            D_00157dc4[i].behind = D_00157dc4[prec].behind;
                            D_00157dc4[i].front = prec;
                            D_00157dc4[prec].behind = i;
                        } else {
                            D_00157dc4[i].behind = -1;
                            D_00157dc4[i].front = -1;
                            D_00157dc4[i].next = -1;
                            if (head >= 0) {
                                D_00157dc4[tail].next = i;
                                tail = i;
                            } else {
                                head = i;
                                tail = i;
                            }
                        }
                    } else {
                        D_00157dc4[i].next = last;
                        last = i;
                    }
                }
                x++;
                y--;
                c += -0x7f;
            }
            for (k = head; k >= 0; k = D_00157dc4[k].next) {
                i = k;
                while (D_00157dc4[i].behind >= 0)
                    i = D_00157dc4[i].behind;
                for (; i >= 0; i = D_00157dc4[i].front) {
                    x = bx + i;
                    y = by - i;
                    c = base - i * 0x7f;
                    do_cell(n, px + resvars.f4 * i, py, x, y, c, minx, miny, x0, y0, x1, y1);
                }
            }
            for (i = last; i >= 0; i = D_00157dc4[i].next) {
                x = bx + i;
                y = by - i;
                c = base - i * 0x7f;
                do_cell(n, px + resvars.f4 * i, py, x, y, c, minx, miny, x0, y0, x1, y1);
            }
            px += dx;
            py += resvars.f3 / 2;
            if (dx < 0) {
                by++;
                base += 0x80;
                rows += step;
            } else if (dx > 0) {
                bx++;
                base++;
                rows -= step;
            }
            dx = -dx;
        }
    }
}

// FUNCTION: 0x000a8688
void draw_window_on_wscreen_mk2(const u8 n, s32 x0, s32 y0, s32 x1, s32 y1, const s32 cell)
{
    Map *mp;
    s32 rows;
    s32 f20;
    s32 count;
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
    find_common_window(x0, y0, x1, y1, window.zone_x[n], window.zone_y[n], window.zone_w[n], window.zone_h[n]);
    if (info.unknown78 == 0) {
        x0 = info.unknown68;
        y0 = info.unknown6c;
        x1 = info.unknown70;
        y1 = info.unknown74;
        setup_stats_for_window_scan(n, x0, y0, x1, y1);
        rows = info.f18;
        f20 = info.f20;
        count = info.f24;
        step = info.f1c;
        base = info.f0;
        bx = info.f4;
        by = info.f8;
        px = info.fc;
        py = info.f14;
        dx = info.f10;
        half = resvars.f3 / 2;
        sx = px;
        sy = py;
        LbScreenSetGraphicsWindow(x0, y0, x1 - x0, y1 - y0);
        sx -= x0;
        sy -= y0;
        active = 0;
        reached = 0;
        for (j = 0; j < count; j++) {
            x = sx;
            c = base;
            cx = bx;
            cy = by;
            for (i = 0; i < rows; i++) {
                if ((((active == 1 && cx >= 0) && cx < 0x80) && cy >= 0) && cy < 0x80) {
                    m = &mp[c];
                    if (m->unknown3) {
                        if (myplayer.f1d) {
                            if (!(m->floor & 0x20))
                                LbSpriteDraw(x, sy - blocks[m->unknown3].info->height, blocks[m->unknown3].info);
                            if (m->floor & 0x20) {
                                if (m->unknown3 == InteriorWallHalf.block) {
                                    LbSpriteDraw(x, sy - InteriorWallHalf.sprite.height, &InteriorWallHalf.sprite);
                                } else {
                                    overlay.width = blocks[m->unknown3].info->width;
                                    overlay.height = blocks[m->unknown3].info->height;
                                    overlay.data = (u32)ShadowBuffer;
                                    create_overlay_sprite(blocks[m->unknown3].info, blocks[150].info, &overlay);
                                    LbSpriteDraw(x, sy - overlay.height, &overlay);
                                }
                            }
                        } else {
                            LbSpriteDraw(x, sy - blocks[m->unknown3].info->height, blocks[m->unknown3].info);
                        }
                    }
                    if (m->unknown4)
                        LbSpriteDraw(x, sy - blocks[m->unknown4].info->height, blocks[m->unknown4].info);
                }
                if (c == cell)
                    reached = 1;
                x += resvars.f4;
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
                rows += step;
            } else if (dx > 0) {
                bx++;
                base++;
                rows -= step;
            }
            dx = -dx;
        }
    }
}

// FUNCTION: 0x000a8a8c
void draw_window_into_pix_buffer(const u8 n, s32 x0, s32 y0, s32 x1, s32 y1, const s32 lx, const s32 ly)
{
    Map *mp;
    s32 rows;
    s32 f20;
    s32 count;
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
    setup_stats_for_window_scan(n, x0, y0, x1, y1);
    rows = info.f18;
    f20 = info.f20;
    count = info.f24;
    step = info.f1c;
    base = info.f0;
    bx = info.f4;
    by = info.f8;
    px = info.fc;
    py = info.f14;
    dx = info.f10;
    half = resvars.f3 / 2;
    give_absolute_xy_of_cell_at_column_row(bx, by);
    ax = info.unknown54;
    ay = info.unknown58;
    give_absolute_xy_of_cell_at_column_row(lx, ly);
    ax -= info.unknown54;
    ay -= info.unknown58;
    for (j = 0; j < count; j++) {
        x = ax;
        sx = px;
        c = base;
        cx = bx;
        cy = by;
        for (i = 0; i < rows; i++) {
            if ((((cx >= 0 && cx < 0x80) && cy >= 0) && cy < 0x80) && (cx > lx || cy > ly)) {
                m = &mp[c];
                if (m->unknown3)
                    LbDrawHVLine(0x5a - resvars.f4 / 2 + x,
                                 ay - blocks[m->unknown3].info->height + 0xb4 - resvars.f3 * 3 / 2 + resvars.f3,
                                 blocks[m->unknown3].info, 0xfe);
                if (m->unknown4)
                    LbDrawHVLine(0x5a - resvars.f4 / 2 + x,
                                 ay - blocks[m->unknown4].info->height + 0xb4 - resvars.f3 * 3 / 2 + resvars.f3,
                                 blocks[m->unknown4].info, 0xfe);
            }
            x += resvars.f4;
            sx += resvars.f4;
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
            rows += step;
        } else if (dx > 0) {
            bx++;
            base++;
            rows -= step;
        }
        dx = -dx;
    }
}

// FUNCTION: 0x000a8d98
void draw_boundry(const u8 n, s32 x0, s32 y0, s32 x1, s32 y1)
{
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;

    find_common_window(x0, y0, x1, y1, window.zone_x[n], window.zone_y[n], window.zone_w[n], window.zone_h[n]);
    find_common_window(info.unknown68, info.unknown6c, info.unknown70, info.unknown74,
                       window.unknown2c3, window.unknown2c7, window.unknown2cb, window.unknown2cf);
    LbScreenSetGraphicsWindow(info.unknown68, info.unknown6c, info.unknown70 - info.unknown68, info.unknown74 - info.unknown6c);
    left = 0;
    top = 0;
    right = window.zone_w[n] - window.zone_x[n] - 1;
    bottom = window.zone_h[n] - window.zone_y[n] - 1;
    if (window.zone_x[n] < info.unknown68) {
        left = window.zone_x[n] - info.unknown68;
        right += left;
    }
    if (window.zone_y[n] < info.unknown6c) {
        top = window.zone_y[n] - info.unknown6c;
        bottom += top;
    }
    fn_000c9be0(left, top, right, top, 3);
    fn_000c9be0(right, top, right, bottom, 3);
    fn_000c9be0(right, bottom, left, bottom, 3);
    fn_000c9be0(left, bottom, left, top, 3);
}

// FUNCTION: 0x000a8f78
void adjust_big_cell_flag(const u16 cell)
{
    Thing *t;
    Thing *o;
    u16 shape;

    for (t = &thing[map[cell].first]; t > thing; t = &thing[t->next]) {
        switch (t->renderer) {
        case 2:
        case 4:
            o = &thing[t->unknown69];
            if (o > thing && (u16)(t - thing) == o->unknown91) {
                map[cell].flags |= 0x20;
                return;
            }
            break;
        case 6:
            shape = objects[t->subtype].shapes[t->unknown17];
            if (shapes[shape].count > 1) {
                map[cell].flags |= 0x20;
                return;
            }
            break;
        }
    }
    map[cell].flags &= ~0x20;
}

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
    if (v >= 0 || (v = check_obscuring_column(row, x + 1, y)) >= 0 || (v = check_obscuring_column(row, x + 1, y + 1)) >= 0)
        return v;
    covers[idx][col] = row;
    return -1;
}

// FUNCTION: 0x000a9358
s8 compute_cell_precedence(const s8 row, Thing *t)
{
    Thing *o;
    s8 v;

    for (; t > thing; t = &thing[t->next]) {
        switch (t->renderer) {
        case 4:
            if (t->kind == 0x2a || t->kind == 0x2e)
                break;
            o = &thing[t->unknown69];
            if (o > thing && (u16)(t - thing) == o->unknown91) {
                u16 shape;
                s16 off;
                u8 k;
                s16 pos;

                shape = objects[o->subtype].shapes[o->unknown17];
                record_covering_cell(row, 0);
                off = (s16)-(s8)shapes[shape].unknown1f[0] + -(s8)shapes[shape].unknown1f[1] * 128;
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
        case 2:
            if (t->kind == 0x28)
                break;
            o = &thing[t->unknown69];
            if (o > thing && (u16)(t - thing) == o->unknown91) {
                u16 shape;
                s16 off;
                u8 k;
                s16 pos;

                shape = objects[o->subtype].shapes[o->unknown17];
                record_covering_cell(row, 0);
                off = (s16)-(s8)shapes[shape].unknown1f[0] + -(s8)shapes[shape].unknown1f[1] * 128;
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
        case 6: {
            u16 shape;
            u8 k;
            s16 pos;

            shape = objects[t->subtype].shapes[t->unknown17];
            if (t->kind == 4)
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

// FUNCTION: 0x000a9744
void draw_all_mk7_selective_sprites_wscreen(const u8 n, s32 x0, s32 y0, s32 x1, s32 y1)
{
    Map *mp;
    s32 rows;
    s32 f20;
    s32 count;
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
    s32 rows0;
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
    find_common_window(x0, y0, x1, y1, window.zone_x[n], window.zone_y[n], window.zone_w[n], window.zone_h[n]);
    if (info.unknown78 != 0)
        return;
    x0 = info.unknown68;
    y0 = info.unknown6c;
    x1 = info.unknown70;
    y1 = info.unknown74;
    setup_stats_for_window_scan(n, x0, y0, x1, y1);
    rows = info.f18;
    f20 = info.f20;
    count = info.f24;
    step = info.f1c;
    base = info.f0;
    bx = info.f4;
    by = info.f8;
    px = info.fc;
    py = info.f14;
    dx = info.f10;
    half = resvars.f3 / 2;
    sx = px;
    sy = py;
    LbScreenSetGraphicsWindow(x0, y0, x1 - x0, y1 - y0);
    sx -= x0;
    sy -= y0;
    rows0 = rows;
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
    if (window.zone_x[n] > 0)
        minx = window.zone_x[n];
    if (window.zone_y[n] > 0)
        miny = window.zone_y[n];
    if (window.unknown2c3 > minx)
        minx = window.unknown2c3;
    if (window.unknown2c7 > miny)
        miny = window.unknown2c7;
    for (j = 0; j < f20; j++) {
        x = sx;
        wx = px;
        c = base;
        cx = bx;
        cy = by;
        for (i = 0; i < rows; i++) {
            if ((((cx >= 0 && cx < 0x80) && cy >= 0) && cy < 0x80)) {
                m = &mp[c];
                tile = m->overlay;
                if (myplayer.f1d) {
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
            x += resvars.f4;
            wx += resvars.f4;
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
            rows += step;
        } else if (dx > 0) {
            bx++;
            base++;
            rows -= step;
        }
        dx = -dx;
    }
    px = px0;
    py = py0;
    sx = sx0;
    sy = sy0;
    step = step0;
    dx = dx0;
    rows = rows0;
    bx = bx0;
    by = by0;
    base = base0;
    for (j = 0; j < count; j++) {
        x = sx;
        wx = px;
        c = base;
        cx = bx;
        cy = by;
        for (i = 0; i < rows; i++) {
            if ((((cx >= 0 && cx < 0x80) && cy >= 0) && cy < 0x80)) {
                m = &mp[c];
                if (m->unknown3) {
                    if (myplayer.f1d) {
                        if (!(m->floor & 0x20)) {
                            LbSpriteDraw(x, sy - blocks[m->unknown3].info->height, blocks[m->unknown3].info);
                        } else if (m->floor & 0x20) {
                            if (m->unknown3 == InteriorWallHalf.block) {
                                LbSpriteDraw(x, sy - InteriorWallHalf.sprite.height, &InteriorWallHalf.sprite);
                            } else if (m->unknown3 == InteriorWallHalf.block) {
                                LbSpriteDraw(x, sy - InteriorWallHalf.sprite.height, &InteriorWallHalf.sprite);
                            } else {
                                overlay.width = blocks[m->unknown3].info->width;
                                overlay.height = blocks[m->unknown3].info->height;
                                overlay.data = (u32)ShadowBuffer;
                                create_overlay_sprite(blocks[m->unknown3].info, blocks[150].info, &overlay);
                                LbSpriteDraw(x, sy - blocks[150].info->height, &overlay);
                            }
                        }
                    } else {
                        LbSpriteDraw(x, sy - blocks[m->unknown3].info->height, blocks[m->unknown3].info);
                    }
                }
                if (m->unknown4)
                    LbSpriteDraw(x, sy - blocks[m->unknown4].info->height, blocks[m->unknown4].info);
            } else {
                break;
            }
            x += resvars.f4;
            wx += resvars.f4;
            cx++;
            cy--;
            c += -0x7f;
        }
        x = sx;
        wx = px;
        c = base;
        cx = bx;
        cy = by;
        for (i = 0; i < rows; i++) {
            if ((((cx >= 0 && cx < 0x80) && cy >= 0) && cy < 0x80)) {
                m = &mp[c];
                if ((m->first || (m->floor & 0xf)) || (m->flags & 0xc0)) {
                    if (!(m->flags & 0x10)) {
                        do_mapwho(m, x, sy - resvars.f3, 0, 0);
                    } else {
                        DrawIntoPixBuffer();
                        do_mapwho(m, 0x5a - resvars.f4 / 2, 0xb4 - resvars.f3 * 3 / 2, 0, 0);
                        ppx = D_001567b0[0];
                        ppy = D_001567b0[1];
                        D_001567b0[0] += x - (0x5a - resvars.f4 / 2);
                        D_001567b0[1] += sy - 0xb4 + resvars.f3 * 3 / 2 - resvars.f3;
                        D_001567b0[2] += x - (0x5a - resvars.f4 / 2);
                        D_001567b0[3] += sy - 0xb4 + resvars.f3 * 3 / 2 - resvars.f3;
                        pixBuf.x = x - 0x5a + resvars.f4 / 2;
                        pixBuf.y = sy - 0xb4 + resvars.f3 * 3 / 2 - resvars.f3;
                        find_common_window(D_001567b0[0] + minx, D_001567b0[1] + miny, D_001567b0[2] + minx, D_001567b0[3] + miny, x0, y0, x1, y1);
                        b0 = info.unknown68;
                        b1 = info.unknown6c;
                        b2 = info.unknown70;
                        b3 = info.unknown74;
                        if (info.unknown78 == 0) {
                            draw_window_into_pix_buffer(n, b0, b1, b2, b3, cx, cy);
                            if (n == 0)
                                set_all_refresh_zones_in_window(b0, b1, b2, b3, 3);
                        }
                        RestoreFromPixBuffer();
                        LbScreenSetGraphicsWindow(x0, y0, x1 - x0, y1 - y0);
                        DrawPixBuffer(0, ppx, ppy, D_001567b0[2] - D_001567b0[0], D_001567b0[3] - D_001567b0[1]);
                        FillBoxInPixBuffer(0xfe, ppx, ppy, D_001567b0[2] - D_001567b0[0], D_001567b0[3] - D_001567b0[1]);
                    }
                }
            }
            x += resvars.f4;
            wx += resvars.f4;
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
            rows += step;
        } else if (dx > 0) {
            bx++;
            base++;
            rows -= step;
        }
        dx = -dx;
    }
}

// FUNCTION: 0x000aa67c
void draw_all_mk7_no_sprites_bscreen(const u8 n, s32 x0, s32 y0, s32 x1, s32 y1)
{
    s32 cy;
    Map *mp;
    s32 rows;
    s32 f20;
    s32 count;
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
    s32 rows0;
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
    find_common_window(x0, y0, x1, y1, window.zone_x[n], window.zone_y[n], window.zone_w[n], window.zone_h[n]);
    if (info.unknown78 != 0)
        return;
    x0 = info.unknown68;
    y0 = info.unknown6c;
    x1 = info.unknown70;
    y1 = info.unknown74;
    setup_stats_for_window_scan(n, x0, y0, x1, y1);
    rows = info.f18;
    f20 = info.f20;
    count = info.f24;
    step = info.f1c;
    base = info.f0;
    bx = info.f4;
    by = info.f8;
    px = info.fc;
    py = info.f14;
    dx = info.f10;
    half = resvars.f3 / 2;
    sx = px;
    sy = py;
    LbScreenSetGraphicsWindow(x0, y0, x1 - x0, y1 - y0);
    sx -= x0;
    sy -= y0;
    rows0 = rows;
    px0 = px;
    py0 = py;
    sx0 = sx;
    sy0 = sy;
    step0 = step;
    dx0 = dx;
    bx0 = bx;
    by0 = by;
    base0 = base;
    othervar.f9d7 = lbDisplay.unknown4;
    othervar.f9db = lbDisplay.graphics_screen_width;
    othervar.f9df = lbDisplay.graphics_screen_height;
    lbDisplay.unknown4 = bscreen;
    lbDisplay.graphics_screen_width = lbDisplay.unknown18;
    lbDisplay.graphics_screen_height = lbDisplay.unknown1c;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    for (j = f20; --j >= 0;) {
        x = px;
        c = base;
        cx = bx;
        cy = by;
        for (i = rows; --i >= 0;) {
            if ((((cx >= 0 && cx < 0x80) && cy >= 0) && cy < 0x80)) {
                m = &mp[c];
                tile = m->overlay;
                if (myplayer.f1d) {
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
            x += resvars.f4;
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
            rows += step;
        } else if (dx > 0) {
            bx++;
            base++;
            rows -= step;
        }
        dx = -dx;
    }
    px = px0;
    py = py0;
    step = step0;
    dx = dx0;
    rows = rows0;
    bx = bx0;
    by = by0;
    base = base0;
    for (j = count; --j >= 0;) {
        x = px;
        c = base;
        cx = bx;
        cy = by;
        for (i = rows; --i >= 0;) {
            if ((((cx >= 0 && cx < 0x80) && cy >= 0) && cy < 0x80)) {
                m = &mp[c];
                if (m->unknown3) {
                    if (myplayer.f1d) {
                        if (!(m->floor & 0x20)) {
                            tile = blocks[m->unknown3].data;
                            draw_a_clipped_four_zone_wall_0(x, py, blocks[m->unknown3].info, x0, y0, x1, y1);
                        } else if (m->floor & 0x20) {
                            if (m->unknown3 == InteriorWallHalf.block) {
                                draw_a_clipped_four_zone_wall_0(x, py, &InteriorWallHalf.sprite, x0, y0, x1, y1);
                            } else {
                                tile = blocks[m->unknown3].data;
                                overlay.width = blocks[m->unknown3].info->width;
                                overlay.height = blocks[m->unknown3].info->height;
                                overlay.data = (u32)ShadowBuffer;
                                create_overlay_sprite(blocks[m->unknown3].info, blocks[150].info, &overlay);
                                draw_a_clipped_four_zone_wall_0(x, py, &overlay, x0, y0, x1, y1);
                            }
                        }
                    } else {
                        tile = blocks[m->unknown3].data;
                        draw_a_clipped_four_zone_wall_0(x, py, blocks[m->unknown3].info, x0, y0, x1, y1);
                    }
                }
                if (m->unknown4) {
                    tile = blocks[m->unknown4].data;
                    draw_a_clipped_four_zone_wall_1(x, py, c, x0, y0, x1, y1);
                }
                x += resvars.f4;
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
            rows += step;
        } else if (dx > 0) {
            bx++;
            base++;
            rows -= step;
        }
        dx = -dx;
    }
    lbDisplay.unknown4 = othervar.f9d7;
    lbDisplay.graphics_screen_width = othervar.f9db;
    lbDisplay.graphics_screen_height = othervar.f9df;
    LbScreenSetGraphicsWindow(0, 0, othervar.f9db, othervar.f9df);
}
