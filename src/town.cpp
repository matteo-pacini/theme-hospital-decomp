#include "hosp.h"

struct TownView {
    u8 unknown0[0x10];
    s32 x;
    s32 y;
    u8 unknown18[8];
    s32 f20;
    s32 f24;
    s32 f28;
    s32 f2c;
    s32 f30;
    s32 f34;
    s32 f38;
};

extern "C" {
extern Event D_001549d0[];
extern char **D_001602c4;
extern char **D_001602c8;
extern u8 D_00156910;
extern u8 D_00158166;
extern u8 D_00158192;
void LbMouseSetWindow(s32 x, s32 y, s32 w, s32 h);
void LbTextSetClipWindow(s32 x, s32 y, s32 w, s32 h);
void fn_0001b7e0(char *dst, s8 *src, u8 mode);
void fn_0006680c(void);
void fn_00067ed4(void);
void fn_00068194(void);
void fn_00066914(void);
void fn_00063680(void);
void fn_00063a64(void);
void fn_000688e4(void);
void fn_00068a14(void);
void fn_000669fc(void);
extern u8 *D_00153f34;
}

extern ClickArea area_towns[];
extern PlotBid plotbid;
extern Menu objs;
extern Menu options;
extern Background background[];
extern u16 land_amount[];
extern PlotAdjacent adjacent_plots[];
extern u8 adjacent_plot_numbers[][6];

u8 process_town_panel(void);
void calculate_land_amount(void);
void draw_money_on_town(void);
void process_bid_panel(void);
void process_bid_time(void);
u16 check_for_sale(void);
u8 can_buy_plot(const u8 plot);
void draw_sale_area_outlines(u16 x, u16 y);
void adjust_mapwho(u16 tile);
void remove_partition_walls(void);
void place_partition_walls(void);
void mow_grass(void);
void recolour_all_rooms(void);
void precalculate_shadow_blocks(void);
void display_hospital_map_sale(void);
void draw_things_on_map(void);
void display_plot_under_mouse(void);
void setup_overhead_map(void);
void display_hospital_map_overhead(void);
u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 n);

u8 testcol = 0x94;

// FUNCTION: 0x00063680
void fn_00063680(void)
{
    u32 i;
    u32 total;

    switch (town.unknown2) {
    case 0:
        lbDisplay.left_button = lbDisplay.rleft_button = 0;
        setup_abort_all_requesters();
        othervar.f6a = myhosp->fc94;
        othervar.f6b_u32 = 0x32;
        load_ingame_pic(1, 1, 0xa, level_number, SCREEN_MODE_13);
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        if (!town.unknown0)
            town.unknown0 = 0x1f;
        calculate_land_amount();
        set_mouse_mode(0xa);
        set_font_from_cache(0);
        setup_overhead_map();
        town.unknown2 = 7;
        othervar.f69 = 1;
        fn_00066914();
        draw_money_on_town();
        set_font_from_cache(0);
        othervar.fa8a = 0x1e;
        LbMouseSetWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        break;
    case 1:
        lbDisplay.left_button = lbDisplay.rleft_button = 0;
        LbGhostTableGenerate(D_00153f34, 0x42, "qdata/ghost66.dat");
        display_mode = 0;
        fn_000688e4();
        town.unknown2 = 0;
        LbMouseSetWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        fn_00068a14();
        break;
    case 7:
        compute_help_text(0, 0, 0x13);
        fn_00066914();
        draw_money_on_town();
        set_font_from_cache(0);
        draw_help_text();
        set_mouse_mode(0xa);
        if (lbDisplay.rleft_button) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, 0xd, 0x1a6, 0xc0, 0x2e)) {
                fn_00074220(0, 0x41);
                LbGhostTableGenerate(D_00153f34, 0x42, "qdata/ghost66.dat");
                town.unknown2 = 0;
            }
        }
        break;
    case 0xb:
        lbDisplay.left_button = lbDisplay.rleft_button = 0;
        othervar.f6a = 1;
        othervar.f6b_u32 = 0;
        othervar.f6a = myhosp->fc94;
        othervar.f6b_u32 = 0x32;
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        town.unknown0 = 3;
        town.unknown2 = 0xc;
        calculate_land_amount();
        set_font_from_cache(0xc);
        LbMouseSetWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        break;
    case 0x45:
        fn_000669fc();
        process_bid_panel();
        process_bid_time();
        break;
    case 0xc:
        othervar.f69 = 1;
        fn_0006680c();
        town.unknown2 = 0x45;
        total = 0;
        for (i = 0; i < 0x4000; i++) {
            if (level->plots[i] == othervar.f78)
                total += gbv.f7c;
        }
        plotbid.price = total;
        if (total > 0) {
            plotbid.step = total / 10;
            if ((u32)plotbid.step < 10)
                plotbid.step = 10;
        }
        for (i = 0; i < 4; i++)
            plotbid.bids[i] = 0;
        plotbid.bidder = 0x63;
        plotbid.timer = 0x64;
        break;
    default:
        town.unknown2 = 0;
        break;
    }
    if (othervar.fa8a)
        othervar.fa8a--;
}

// FUNCTION: 0x00063a64
void fn_00063a64(void)
{
    switch (town.unknown2) {
    case 0:
    case 1:
    case 4:
    case 5:
    case 6:
    case 0xb:
    case 0xc:
    case 0x45:
        return;
    default:
        break;
    }
    switch (lbInkey) {
    case 1:
        town.unknown2 = 1;
        break;
    default:
        break;
    }
    lbInkey = 0;
}

// FUNCTION: 0x00063ae8
void process_mouse_town_select(void)
{
    s32 old_x;
    s32 old_y;
    s32 x;
    s32 y;
    u16 plot;

    old_x = window.f0[window.unknown2eb];
    old_y = window.fc[window.unknown2eb];
    switch (town.unknown2) {
    case 0:
    case 1:
    case 4:
    case 5:
    case 6:
    case 11:
    case 12:
    case 0x45:
        return;
    }
    if (process_town_panel())
        return;
    if (lbDisplay.left_button) {
        plot = check_for_sale();
        if (plot) {
            if (myhosp->f35 > land_amount[plot] * gbv.f7c) {
                play_sound(10, 0x45, -1, -1);
                if (statvar.unknownfd > 1) {
                    D_001549d0[player].type = 0x58;
                    D_001549d0[player].value = plot;
                    othervar.f69 = 1;
                } else {
                    D_001549d0[player].type = 0x36;
                    D_001549d0[player].value = plot;
                    othervar.f69 = 1;
                }
            }
            othervar.f69 = 1;
        }
        lbDisplay.left_button = 0;
    }
    if (lbDisplay.right_button) {
        x = (lbDisplay.mmouse_x - area_towns[myhosp->f29].x) / area_towns[myhosp->f29].width;
        y = (lbDisplay.mmouse_y - area_towns[myhosp->f29].y) / area_towns[myhosp->f29].height;
        goto_column_row_in_window(window.unknown2eb, (u16)x, (u16)y);
        if (!check_if_point_beyond_scroll_limit(window.f0[window.unknown2eb], window.fc[window.unknown2eb], 0)) {
            town.unknown2 = 1;
            play_sound(10, 0x41, -1, -1);
        } else {
            window.f0[window.unknown2eb] = old_x;
            window.fc[window.unknown2eb] = old_y;
        }
        lbDisplay.right_button = 0;
    }
}

// FUNCTION: 0x00063d6c
u8 can_buy_plot(const u8 plot)
{
    s32 i;
    s32 j;

    i = 0;
    if (plot == 0)
        return 1;
    while (adjacent_plots[i].level != 0) {
        if (adjacent_plots[i].level == level_number && adjacent_plots[i].plot == plot) {
            j = 0;
            while (adjacent_plot_numbers[i][j] != 0) {
                if (for_sale[adjacent_plot_numbers[i][j]] == 1 << player)
                    return plot;
                j++;
            }
            return 0;
        }
        i++;
    }
    return plot;
}

// FUNCTION: 0x00063e54
u16 check_for_sale(void)
{
    s16 x;
    s16 y;

    if (area_towns[myhosp->f29].x <= lbDisplay.mouse_x && (area_towns[myhosp->f29].width << 7) + area_towns[myhosp->f29].x > lbDisplay.mouse_x) {
        if (area_towns[myhosp->f29].y <= lbDisplay.mouse_y && (area_towns[myhosp->f29].height << 7) + area_towns[myhosp->f29].y > lbDisplay.mouse_y) {
            x = (lbDisplay.mouse_x - area_towns[myhosp->f29].x) / area_towns[myhosp->f29].width;
            y = (lbDisplay.mouse_y - area_towns[myhosp->f29].y) / area_towns[myhosp->f29].height;
            if (level->plots[x + y * 128])
                return can_buy_plot(level->plots[x + y * 128]);
        }
    }
    return 0;
}

// FUNCTION: 0x00064030
void display_hospital_map_overhead(void)
{
    u32 x;
    u32 y;

    for (x = 0; x < 128; x++) {
        for (y = 0; y < 128; y++) {
            if (map[(s16)x + ((s16)y << 7)].unknown3) {
            if (map[(s16)x + ((s16)y << 7)].unknown3 < 0x95) {
                switch (map[(s16)x + ((s16)y << 7)].unknown3) {
                case 0x8e:
                case 0x8f:
                case 0x90:
                case 0x91:
                    break;
                default:
                    LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                              area_towns[myhosp->f29].y + area_towns[myhosp->f29].height * y,
                              area_towns[myhosp->f29].width, area_towns[myhosp->f29].height / 2, color.unknown7);
                    break;
                }
            }
            }
            if (map[(s16)x + ((s16)y << 7)].unknown4) {
            if (map[(s16)x + ((s16)y << 7)].unknown4 < 0x95) {
                switch (map[(s16)x + ((s16)y << 7)].unknown4) {
                case 0x8e:
                case 0x8f:
                case 0x90:
                case 0x91:
                    break;
                default:
                    LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                              area_towns[myhosp->f29].y + area_towns[myhosp->f29].height * y,
                              area_towns[myhosp->f29].width / 2, area_towns[myhosp->f29].height, color.unknown7);
                    break;
                }
            }
            }
        }
    }
}

// FUNCTION: 0x00064338
void display_hospital_map_sale(void)
{
    u16 x;
    u16 y;
    u8 colour;

    for (x = 1; x < 128; x++) {
        for (y = 1; y < 128; y++) {
            colour = color.unknown9;
            if (map[(s16)x + ((s16)y << 7)].unknown7 & 2)
                colour = color.unknownF;
            if (map[(s16)x + ((s16)y << 7)].unknown7 & 4)
                colour = color.unknownB;
            if (map[(s16)x + ((s16)y << 7)].unknown7 & 8)
                colour = color.unknownA;
            if (background[map[(s16)x + ((s16)y << 7)].overlay].unknown0)
                LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                          area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height,
                          area_towns[myhosp->f29].width, area_towns[myhosp->f29].height, colour);
            if (level->plots[(s16)x + ((s16)y << 7)] && can_buy_plot(level->plots[(s16)x + ((s16)y << 7)]))
                LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                          area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height,
                          area_towns[myhosp->f29].width, area_towns[myhosp->f29].height, color.unknownD);
            draw_sale_area_outlines(x, y);
            if (map[(s16)x + ((s16)y << 7)].unknown3) {
                if ((blocks[map[(s16)x + ((s16)y << 7)].unknown3].width & 1) && !(blocks[map[(s16)x + ((s16)y << 7)].unknown3].width & 0x40))
                    LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                              area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height,
                              area_towns[myhosp->f29].width, area_towns[myhosp->f29].height / 2, color.unknown7);
            }
            if (map[(s16)x + ((s16)y << 7)].unknown4) {
                if ((blocks[map[(s16)x + ((s16)y << 7)].unknown4].width & 1) && !(blocks[map[(s16)x + ((s16)y << 7)].unknown4].width & 0x40))
                    LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                              area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height,
                              area_towns[myhosp->f29].width / 2, area_towns[myhosp->f29].height, color.unknown7);
            }
            if (map[(s16)(x - 1) + ((s16)y << 7)].unknown3 && map[(s16)x + ((s16)(y - 1) << 7)].unknown4) {
                if ((blocks[map[(s16)(x - 1) + ((s16)y << 7)].unknown3].width & 1) && !(blocks[map[(s16)(x - 1) + ((s16)y << 7)].unknown3].width & 0x40)) {
                    if ((blocks[map[(s16)x + ((s16)(y - 1) << 7)].unknown4].width & 1) && !(blocks[map[(s16)x + ((s16)(y - 1) << 7)].unknown4].width & 0x40))
                        LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                  area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height, 1, 1, color.unknown7);
                }
            }
        }
    }
}

// FUNCTION: 0x00064a60
void draw_sale_area_outlines(u16 x, u16 y)
{
    if (x) {
        if (level->plots[(s16)x + ((s16)y << 7)] != level->plots[(s16)(x - 1) + ((s16)y << 7)])
            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                      area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height,
                      area_towns[myhosp->f29].width / 2, area_towns[myhosp->f29].height, color.unknown7);
    }
    if (y) {
        if (level->plots[(s16)x + ((s16)y << 7)] != level->plots[(s16)x + ((s16)(y - 1) << 7)])
            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                      area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height,
                      area_towns[myhosp->f29].width, area_towns[myhosp->f29].height / 2, color.unknown7);
    }
    if (x > 1 && y > 1) {
        if (level->plots[(s16)x + ((s16)y << 7)] != level->plots[((s16)(y - 1) << 7) + (s16)(x - 1)])
            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                      area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height, 1, 1, color.unknown7);
    }
}

// FUNCTION: 0x00064db0
void buy_land(u16 plot)
{
    s32 total;
    u16 i;

    for_sale[plot] = 1 << othervar.ff;
    total = 0;
    for (i = 0; i < 0x4000; i++) {
        if (level->plots[i] == plot) {
            map[i].unknown3 = level->tiles[i].unknown3;
            map[i].unknown4 = level->tiles[i].unknown4;
            map[i].first = level->tiles[i].first;
            adjust_mapwho(i);
            map[i].overlay = level->tiles[i].overlay;
            map[i].flags = level->tiles[i].flags & 0x3f;
            map[i].floor = level->tiles[i].floor & 0xf0;
            map[i].unknown7 = level->tiles[i].unknown7;
            map[i].unknown7 |= 1 << othervar.ff;
            level->plots[i] = 0;
            total += gbv.f7c;
        }
    }
    remove_partition_walls();
    place_partition_walls();
    mow_grass();
    if (statvar.unknownfd > 1)
        change_balance(6, -plotbid.bids[othervar.ff], 0, (u16)othervar.ff);
    else if (total > 0)
        change_balance(6, -total, 0, (u16)othervar.ff);
    othervar.f10 = 1;
    add_static_shadows();
}

// FUNCTION: 0x00065014
void mow_grass(void)
{
    u16 x;
    u16 y;
    u8 grass;

    for (y = 0; y < 128; y++) {
        grass = 2;
        for (x = 0; x < 128; x++) {
            if (grass == 0)
                grass = 2;
            else
                grass = 0;
            if (map[(s16)x + ((s16)y << 7)].overlay == 0 || map[(s16)x + ((s16)y << 7)].overlay == 2)
                map[(s16)x + ((s16)y << 7)].overlay = grass;
        }
    }
}

// FUNCTION: 0x000650f4
void place_partition_walls(void)
{
    u16 x;
    u16 y;

    for (x = 0; x < 128; x++) {
        for (y = 0; y < 128; y++) {
            if (!(map[(s16)x + ((s16)y << 7)].unknown7 & 0x10)) {
                if (map[(s16)(x + 1) + ((s16)y << 7)].unknown7 & 0x10) {
                    if (level->tiles[(s16)(x + 1) + ((s16)y << 7)].unknown4 == 0x8d)
                        map[(s16)(x + 1) + ((s16)y << 7)].unknown4 = 0x8d;
                }
                if (map[(s16)x + ((s16)(y + 1) << 7)].unknown7 & 0x10) {
                    if (level->tiles[(s16)x + ((s16)(y + 1) << 7)].unknown3 == 0x8c)
                        map[(s16)x + ((s16)(y + 1) << 7)].unknown3 = 0x8c;
                }
            }
        }
    }
}

// FUNCTION: 0x00065248
void remove_partition_walls(void)
{
    u16 i;
    u8 b;
    u8 a;

    for (i = 0; i < 0x4000; i++) {
        if (map[i].unknown3 == 0x8c) {
            b = a = 0;
            if (!(map[i + 128].unknown7 & 0x10))
                b = 1;
            if (!(map[i - 128].unknown7 & 0x10))
                a = 1;
            if (b && a) {
                if (which_room(i + 128) || which_room(i - 128))
                    map[i].unknown3 = 0x52;
                else
                    map[i].unknown3 = 0;
            }
        }
        if (map[i].unknown4 == 0x8d) {
            b = a = 0;
            if (!(map[i + 1].unknown7 & 0x10))
                b = 1;
            if (!(map[i - 1].unknown7 & 0x10))
                a = 1;
            if (b && a) {
                if (which_room(i + 1) || which_room(i - 1))
                    map[i].unknown4 = 0x53;
                else
                    map[i].unknown4 = 0;
            }
        }
    }
    recolour_all_rooms();
}

// FUNCTION: 0x0006541c
void recreate_map(const u8 flag)
{
    u16 i;
    u16 j;
    u8 found;
    u32 rnd;

    if (flag) {
        for (i = 0; i < 200; i++) {
            if (for_sale[i]) {
                for (j = 0; j < 0x4000; j++) {
                    if (level->plots[j] == i)
                        level->plots[j] = 0;
                }
            }
        }
    } else {
        for (i = 0; i < 200; i++)
            for_sale[i] = 0;
        for (i = 0; i < 0x4000; i++) {
            found = 0;
            switch (level->plots[i]) {
            case 0:
                found = 1;
                map[i].unknown3 = level->tiles[i].unknown3;
                map[i].unknown4 = level->tiles[i].unknown4;
                map[i].first = level->tiles[i].first;
                map[i].overlay = level->tiles[i].overlay;
                map[i].flags = level->tiles[i].flags & 0x3f;
                map[i].floor = level->tiles[i].floor & 0xf0;
                map[i].unknown7 = level->tiles[i].unknown7;
                map[i].unknown7 &= ~1;
                map[i].unknown7 &= ~2;
                map[i].unknown7 &= ~4;
                map[i].unknown7 &= ~8;
                level->plots[i] = 0;
                map[i].first = level->tiles[i].first;
                adjust_mapwho(i);
                for_sale[0] = 0x10;
                break;
            case 1:
                if (level->f0 >= 1) {
                    found = 1;
                    map[i].unknown3 = level->tiles[i].unknown3;
                    map[i].unknown4 = level->tiles[i].unknown4;
                    map[i].first = level->tiles[i].first;
                    map[i].overlay = level->tiles[i].overlay;
                    map[i].flags = level->tiles[i].flags & 0x3f;
                    map[i].floor = level->tiles[i].floor & 0xf0;
                    map[i].unknown7 = level->tiles[i].unknown7;
                    map[i].unknown7 |= 1;
                    level->plots[i] = 0;
                    map[i].first = level->tiles[i].first;
                    adjust_mapwho(i);
                    for_sale[1] = 1;
                }
                break;
            case 2:
                if (level->f0 >= 2) {
                    found = 1;
                    map[i].unknown3 = level->tiles[i].unknown3;
                    map[i].unknown4 = level->tiles[i].unknown4;
                    map[i].first = level->tiles[i].first;
                    map[i].overlay = level->tiles[i].overlay;
                    map[i].flags = level->tiles[i].flags & 0x3f;
                    map[i].floor = level->tiles[i].floor & 0xf0;
                    map[i].unknown7 = level->tiles[i].unknown7;
                    map[i].unknown7 |= 2;
                    level->plots[i] = 0;
                    map[i].first = level->tiles[i].first;
                    adjust_mapwho(i);
                    for_sale[2] = 2;
                }
                break;
            case 3:
                if (level->f0 >= 3) {
                    found = 1;
                    map[i].unknown3 = level->tiles[i].unknown3;
                    map[i].unknown4 = level->tiles[i].unknown4;
                    map[i].first = level->tiles[i].first;
                    map[i].overlay = level->tiles[i].overlay;
                    map[i].flags = level->tiles[i].flags & 0x3f;
                    map[i].floor = level->tiles[i].floor & 0xf0;
                    map[i].unknown7 = level->tiles[i].unknown7;
                    map[i].unknown7 |= 4;
                    level->plots[i] = 0;
                    map[i].first = level->tiles[i].first;
                    adjust_mapwho(i);
                    for_sale[3] = 4;
                }
                break;
            case 4:
                if (level->f0 >= 4) {
                    found = 1;
                    map[i].unknown3 = level->tiles[i].unknown3;
                    map[i].unknown4 = level->tiles[i].unknown4;
                    map[i].first = level->tiles[i].first;
                    map[i].overlay = level->tiles[i].overlay;
                    map[i].flags = level->tiles[i].flags & 0x3f;
                    map[i].floor = level->tiles[i].floor & 0xf0;
                    map[i].unknown7 = level->tiles[i].unknown7;
                    map[i].unknown7 |= 8;
                    level->plots[i] = 0;
                    map[i].first = level->tiles[i].first;
                    adjust_mapwho(i);
                    for_sale[4] = 8;
                }
                break;
            }
            if (!found) {
                map[i].first = 0;
                map[i].overlay = 0;
                map[i].unknown3 = 0;
                map[i].unknown4 = 0;
                map[i].flags = background[map[i].overlay].unknown1[1] & 0x3f;
                map[i].unknown7 = background[map[i].overlay].unknown1[0];
            }
        }
        mow_grass();
        rnd = 0xbb40e62d;
        for (i = 0; i < 0x4000; i++) {
            rnd *= 0xbb40e62d;
            if (level->plots[i]) {
                if (map[i].overlay == 0) {
                    if (((rnd >> 8) & 0x7fff) % 50 == 0)
                        map[i].unknown3 = ((rnd >> 8) & 0x7fff & 7) + 0xb6;
                }
            }
        }
    }
    place_partition_walls();
    if (!flag)
        precalculate_shadow_blocks();
}

// FUNCTION: 0x00065fc0
void adjust_mapwho(u16 tile)
{
    u8 dir;
    u8 kind;
    u16 id;
    Thing *t;

    if (map[tile].first) {
        kind = map[tile].first >> 8;
        dir = map[tile].first;
        map[tile].first = 0;
        othervar.f9bb = 1;
        id = fn_00037098(tile, (u16)kind, dir, othervar.ff);
        if (id) {
            t = &thing[id];
            t->owner = 0;
        }
        othervar.f9bb = 0;
    }
}

// FUNCTION: 0x00066094
void display_plot_under_mouse(void)
{
    s16 x;
    s16 y;
    u16 i;
    u16 plot;
    u16 h;
    s32 price;
    Map *m;
    char buf[100];

    i = 5;
    set_font_from_cache(0x18);
    h = lbFontPtr ? lbFontPtr[34].height : 0;
    LbTextSetWindow(0xe6, 0x30 - h / 2, 0x180, 0x18);
    lbDisplay.draw_flags = 0x100;
    sprintf(buf, "%s", D_001602c4[myhosp->f29]);
    fn_0001b7e0(buf, (s8 *)buf, 0);
    LbTextDraw(0, 0, buf);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    sprintf(buf, "%s", D_001602c8[4]);
    LbTextDraw(resvars.f40[46].x, resvars.f40[46].y, buf);
    sprintf(buf, "%s", D_001602c8[5]);
    LbTextDraw(resvars.f40[47].x, resvars.f40[47].y, buf);
    sprintf(buf, "%s", D_001602c8[6]);
    LbTextDraw(resvars.f40[48].x, resvars.f40[48].y, buf);
    sprintf(buf, "%s", D_001602c8[7]);
    LbTextDraw(resvars.f40[49].x, resvars.f40[49].y, buf);
    x = (lbDisplay.mmouse_x - area_towns[myhosp->f29].x) / area_towns[myhosp->f29].width;
    y = (lbDisplay.mmouse_y - area_towns[myhosp->f29].y) / area_towns[myhosp->f29].height;
    if ((x >= 0 && y >= 0) && (x < 128 && y < 128)) {
        plot = level->plots[x + (y << 7)];
        if (plot) {
            price = land_amount[plot] * gbv.f7c;
            sprintf(buf, ": %d", plot);
            LbTextDraw(resvars.f40[50].x, resvars.f40[50].y, buf);
            sprintf(buf, ": %s", D_001602c8[2]);
            LbTextDraw(resvars.f40[51].x, resvars.f40[51].y, buf);
            sprintf(buf, ": %d", land_amount[plot]);
            LbTextDraw(resvars.f40[52].x, resvars.f40[52].y, buf);
            sprintf(buf, ": $%d", price);
            LbTextDraw(resvars.f40[53].x, resvars.f40[53].y, buf);
        } else {
            m = &map[x + (y << 7)];
            sprintf(buf, ": -");
            LbTextDraw(resvars.f40[50].x, resvars.f40[50].y, buf);
            sprintf(buf, ": %s", D_001602c8[3]);
            for (i = 0; i < 4; i++) {
                if (m->unknown7 & (1 << i)) {
                    sprintf(buf, ": %s", allhosps[i].unknown0);
                    break;
                }
            }
            LbTextDraw(resvars.f40[51].x, resvars.f40[51].y, buf);
            sprintf(buf, ": -");
            LbTextDraw(resvars.f40[52].x, resvars.f40[52].y, buf);
            sprintf(buf, ": -");
            LbTextDraw(resvars.f40[53].x, resvars.f40[53].y, buf);
        }
    } else {
        sprintf(buf, ": -");
        LbTextDraw(resvars.f40[50].x, resvars.f40[50].y, buf);
        sprintf(buf, ": -");
        LbTextDraw(resvars.f40[51].x, resvars.f40[51].y, buf);
        sprintf(buf, ": -");
        LbTextDraw(resvars.f40[52].x, resvars.f40[52].y, buf);
        sprintf(buf, ": -");
        LbTextDraw(resvars.f40[53].x, resvars.f40[53].y, buf);
    }
}

// FUNCTION: 0x000666d4
void calculate_land_amount(void)
{
    u16 i;
    u16 j;

    for (i = 0; i < 200; i++) {
        land_amount[i] = 0;
        for (j = 0; j < 0x4000; j++) {
            if (level->plots[j] == i)
                land_amount[i]++;
        }
    }
}

// FUNCTION: 0x00066764
void setup_overhead_map(void)
{
    s32 unused;

    othervar.f9d7 = lbDisplay.unknown4;
    othervar.f9db = lbDisplay.graphics_screen_width;
    othervar.f9df = lbDisplay.graphics_screen_height;
    lbDisplay.unknown4 = bscreen;
    lbDisplay.graphics_screen_width = lbDisplay.unknown18;
    lbDisplay.graphics_screen_height = lbDisplay.unknown1c;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    display_hospital_map_overhead();
    lbDisplay.unknown4 = othervar.f9d7;
    lbDisplay.graphics_screen_width = othervar.f9db;
    lbDisplay.graphics_screen_height = othervar.f9df;
    LbScreenSetGraphicsWindow(0, 0, othervar.f9db, othervar.f9df);
    othervar.f69 = 0;
}

// FUNCTION: 0x0006680c
extern "C" void fn_0006680c(void)
{
    s32 unused;

    load_ingame_pic(10, 1, 10, level_number, SCREEN_MODE_13);
    othervar.f9d7 = lbDisplay.unknown4;
    othervar.f9db = lbDisplay.graphics_screen_width;
    othervar.f9df = lbDisplay.graphics_screen_height;
    lbDisplay.unknown4 = bscreen;
    lbDisplay.graphics_screen_width = lbDisplay.unknown18;
    lbDisplay.graphics_screen_height = lbDisplay.unknown1c;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    LbDrawBox(0x14c, 0x49, 0x103, 0x107, 0x6c);
    draw_frame(0x14b, 0x48, 0x104, 0x108, 0x68);
    fn_00067ed4();
    lbDisplay.unknown4 = othervar.f9d7;
    lbDisplay.graphics_screen_width = othervar.f9db;
    lbDisplay.graphics_screen_height = othervar.f9df;
    LbScreenSetGraphicsWindow(0, 0, othervar.f9db, othervar.f9df);
    othervar.f69 = 0;
}

// FUNCTION: 0x00066914
extern "C" void fn_00066914(void)
{
    u8 i;

    if (othervar.f69)
        setup_overhead_map();
    copy_screen_to_wscreen(bscreen);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    display_hospital_map_sale();
    draw_things_on_map();
    for (i = 0; i < 5; i++) {
        if (!(town.unknown0 & (1 << i)))
            reqspr_draw(resvars.f40[34 + i].x, resvars.f40[34 + i].y, &screen_sprites_tab_start[i + 1]);
    }
    display_plot_under_mouse();
}

// FUNCTION: 0x000669fc
extern "C" void fn_000669fc(void)
{
    if (othervar.f69)
        fn_0006680c();
    copy_screen_to_wscreen(bscreen);
    fn_00067ed4();
    fn_00068194();
}

// FUNCTION: 0x00066a34
void draw_things_on_map(void)
{
    u16 x;
    u16 i;
    u16 y;
    Thing *t;
    char buf[50];

    for (x = 0; x < 128; x++) {
        for (y = 0; y < 128; y++) {
            if (map[(s16)x + ((s16)y << 7)].first > 0) {
                t = &thing[map[(s16)x + ((s16)y << 7)].first];
                switch (t->renderer) {
                case 6:
                    switch (t->subtype) {
                    case 0x2d:
                        if (town.unknown0 & 2)
                            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                      area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height, 3, 3, color.unknown13);
                        break;
                    case 0x2b:
                        if (town.unknown0 & 4)
                            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                      area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height, 3, 3, color.unknownD);
                        break;
                    case 0x2c:
                        if (town.unknown0 & 0x10) {
                            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                      area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height, 1, 1, color.unknownC);
                            lbDisplay.draw_flags = 4;
                            if (t->owner == player) {
                                if (othervar.fa8a) {
                                    if (myhosp->fc94)
                                        reqspr_draw(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width - screen_sprites_tab_start[myhosp->fc94 + 9].width / 2,
                                            area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height - screen_sprites_tab_start[myhosp->fc94 + 9].height / 2,
                                            &screen_sprites_tab_start[myhosp->fc94 + 9]);
                                }
                            }
                            lbDisplay.draw_flags = 0;
                        }
                        break;
                    case 0x3a:
                    case 0x3b:
                        if (t->unknown17 == 0 || t->unknown17 == 2)
                            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                      area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height, 7, 3, color.unknown11);
                        else
                            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                      area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height, 3, 7, color.unknown11);
                        break;
                    case 3:
                    case 0x34:
                    case 0x35:
                        LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                  area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height, 3, 3, color.unknown11);
                        break;
                    default:
                        if (objects[t->subtype].unknown14 == 1 || objects[t->subtype].unknown14 == 2) {
                            if (town.unknown0 & 8)
                            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                      area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height, 3, 3, color.unknownE);
                        }
                        break;
                    }
                    break;
                case 4:
                case 0x14:
                case 0x16:
                    if (town.unknown0 & 1)
                        LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                  area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height, 2, 2, color.unknown12);
                    break;
                case 2:
                    if ((town.unknown0 & 1) && !(map[((s16)(u16)t->y.tile << 7) + (s16)(u16)t->x.tile].unknown7 & 0x10))
                        LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                  area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height, 2, 2, color.unknown7);
                    break;
                }
            }
        }
    }
    set_font_from_cache(0x19);
    sprintf(buf, "%d", adcounts.f0);
    LbTextDraw(resvars.f40[39].x, resvars.f40[39].y, buf);
    sprintf(buf, "%d", myhosp->object_stats[45].fA);
    LbTextDraw(resvars.f40[40].x, resvars.f40[40].y, buf);
    sprintf(buf, "%d", myhosp->object_stats[43].fA);
    LbTextDraw(resvars.f40[41].x, resvars.f40[41].y, buf);
    sprintf(buf, "%d", myhosp->fc97);
    LbTextDraw(resvars.f40[42].x, resvars.f40[42].y, buf);
    sprintf(buf, "%d", myhosp->fc92);
    LbTextDraw(resvars.f40[43].x, resvars.f40[43].y, buf);
    sprintf(buf, "%d", myhosp->fc92 * myhosp->fc94 * mycontrol->f64c / 100);
    LbTextDraw(resvars.f40[44].x, resvars.f40[44].y, buf);
    if (myhosp->fc94) {
        for (i = 0; i < myhosp->fc94 * 6; i++)
            reqspr_draw(resvars.f40[45].x + i, resvars.f40[45].y, &screen_sprites_tab_start[9]);
    }
}

// FUNCTION: 0x000675a8
u8 process_town_panel(void)
{
    u8 i;
    u8 hit;

    hit = 0;
    if (lbDisplay.rleft_button || lbDisplay.rright_button) {
        for (i = 0; resvars.f48[i + 0x3b].width != 0 && resvars.f48[i + 0x3b].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f48[i + 0x3b].x, resvars.f48[i + 0x3b].y,
                             (s16)resvars.f48[i + 0x3b].width, (s16)resvars.f48[i + 0x3b].height)) {
                hit = 1;
                resvars.f48[i + 0x3b].callback(i);
                if (resvars.f48[i + 0x3b].sprite)
                    reqspr_draw(resvars.f48[i + 0x3b].unknownC, resvars.f48[i + 0x3b].unknownE,
                                &screen_sprites_tab_start[resvars.f48[i + 0x3b].sprite]);
                break;
            }
        }
        lbDisplay.rleft_button = lbDisplay.rright_button = 0;
    }
    if (lbDisplay.mleft_button || lbDisplay.mright_button) {
        for (i = 0; resvars.f48[i + 0x3b].width != 0 && resvars.f48[i + 0x3b].height != 0; i++) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f48[i + 0x3b].x, resvars.f48[i + 0x3b].y,
                             (s16)resvars.f48[i + 0x3b].width, (s16)resvars.f48[i + 0x3b].height)) {
                hit = 1;
                if (resvars.f48[i + 0x3b].sprite)
                    reqspr_draw(resvars.f48[i + 0x3b].unknownC, resvars.f48[i + 0x3b].unknownE,
                                &screen_sprites_tab_start[resvars.f48[i + 0x3b].sprite]);
                lbDisplay.left_button = lbDisplay.right_button = 0;
                break;
            }
        }
    }
    return hit;
}

// FUNCTION: 0x000678dc
void process_town_button(u8 n)
{
    switch (n) {
    case 0:
        if (lbDisplay.rright_button)
            town.unknown0 = 0;
        town.unknown0 ^= 1;
        play_sound(10, 0x43, -1, -1);
        break;
    case 1:
        if (lbDisplay.rright_button)
            town.unknown0 = 0;
        town.unknown0 ^= 2;
        play_sound(10, 0x43, -1, -1);
        break;
    case 2:
        if (lbDisplay.rright_button)
            town.unknown0 = 0;
        town.unknown0 ^= 4;
        play_sound(10, 0x43, -1, -1);
        break;
    case 3:
        if (lbDisplay.rright_button)
            town.unknown0 = 0;
        town.unknown0 ^= 8;
        play_sound(10, 0x43, -1, -1);
        break;
    case 4:
        if (lbDisplay.rright_button)
            town.unknown0 = 0;
        town.unknown0 ^= 0x10;
        if (town.unknown0 & 0x10)
            othervar.fa8a = 0x1e;
        play_sound(10, 0x43, -1, -1);
        break;
    case 6:
        if (myhosp->fc94 < 10) {
            D_001549d0[player].type = 0x4f;
            D_001549d0[player].value = (u16)myhosp->fc94 + 1;
            othervar.f6a = myhosp->fc94 + 1;
            othervar.f6b = 0x32;
            play_sound(10, 0x43, -1, -1);
        }
        othervar.fa8a = 0x1e;
        break;
    case 5:
        if (myhosp->fc94 > 1) {
            D_001549d0[player].type = 0x4f;
            D_001549d0[player].value = (u16)myhosp->fc94 - 1;
            othervar.f6a = myhosp->fc94 - 1;
            othervar.f6b = 0x32;
            play_sound(10, 0x43, -1, -1);
        }
        othervar.fa8a = 0x1e;
        break;
    case 7:
        town.unknown2 = 1;
        play_sound(10, 0x42, -1, -1);
        break;
    }
}

// FUNCTION: 0x00067b80
void process_bid_panel(void)
{
    u8 i;

    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.f48[i + 0x44].width != 0 && resvars.f48[i + 0x44].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f48[i + 0x44].x, resvars.f48[i + 0x44].y,
                             (s16)resvars.f48[i + 0x44].width, (s16)resvars.f48[i + 0x44].height)) {
                resvars.f48[i + 0x44].callback(i);
                reqspr_draw(resvars.f48[i + 0x44].unknownC, resvars.f48[i + 0x44].unknownE,
                            &screen_sprites_tab_start[resvars.f48[i + 0x44].sprite]);
            }
        }
        lbDisplay.rleft_button = lbDisplay.left_button = 0;
    }
    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.f48[i + 0x44].width != 0 && resvars.f48[i + 0x44].height != 0; i++) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f48[i + 0x44].x, resvars.f48[i + 0x44].y,
                             (s16)resvars.f48[i + 0x44].width, (s16)resvars.f48[i + 0x44].height))
                reqspr_draw(resvars.f48[i + 0x44].unknownC, resvars.f48[i + 0x44].unknownE,
                            &screen_sprites_tab_start[resvars.f48[i + 0x44].sprite]);
        }
    }
}

// FUNCTION: 0x00067e44
void process_bid_button(u8 n)
{
    switch (n) {
    case 0:
    case 1:
        if (myhosp->f35 >= plotbid.price) {
            D_001549d0[player].type = 0x59;
            play_sound(10, 0x41, -1, -1);
        } else {
            play_sound(10, 0x42, -1, -1);
        }
        break;
    case 2:
        break;
    }
}

// FUNCTION: 0x00067ed4
extern "C" void fn_00067ed4(void)
{
    u16 x;
    u16 y;
    u8 colour;

    for (x = 0; x < 128; x++) {
        for (y = 0; y < 128; y++) {
            colour = color.unknown9;
            if (map[(s16)x + ((s16)y << 7)].unknown7 & 2)
                colour = color.unknownF;
            if (map[(s16)x + ((s16)y << 7)].unknown7 & 4)
                colour = color.unknownB;
            if (map[(s16)x + ((s16)y << 7)].unknown7 & 8)
                colour = color.unknownA;
            if (background[map[(s16)x + ((s16)y << 7)].overlay].unknown0)
                LbDrawBox(resvars.f3c_TownViewp->x + x * 2, resvars.f3c_TownViewp->y + y * 2, 2, 2, colour);
            if (level->plots[(s16)x + ((s16)y << 7)] == othervar.f78)
                LbDrawBox(resvars.f3c_TownViewp->x + x * 2, resvars.f3c_TownViewp->y + y * 2, 2, 2, color.unknownD);
            if (map[(s16)x + ((s16)y << 7)].unknown3) {
                if (map[(s16)x + ((s16)y << 7)].unknown3 < 0x95)
                    LbDrawBox(resvars.f3c_TownViewp->x + x * 2, resvars.f3c_TownViewp->y + y * 2, 2, 1, 0);
            }
            if (map[(s16)x + ((s16)y << 7)].unknown4) {
                if (map[(s16)x + ((s16)y << 7)].unknown4 < 0x95)
                    LbDrawBox(resvars.f3c_TownViewp->x + x * 2, resvars.f3c_TownViewp->y + y * 2, 1, 2, 0);
            }
        }
    }
    othervar.f79++;
    if (!hilight_plot[othervar.f79])
        othervar.f79 = 0;
}

// FUNCTION: 0x00068194
extern "C" void fn_00068194(void)
{
    u8 i;
    u8 row;
    u16 y;
    u16 colour;
    char small[20];
    char big[100];

    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0xc);
    lbDisplay.draw_flags = 0;
    sprintf(small, "%ld", plotbid.price);
    LbTextDraw(resvars.f40[67].x, resvars.f40[67].y, small);
    row = 0;
    for (i = 0; i < 4; i++) {
        switch (i) {
        case 0:
            colour = color.unknown9;
            y = 0x61;
            break;
        case 1:
            colour = color.unknownF;
            y = 0xa0;
            break;
        case 2:
            colour = color.unknownB;
            y = 0xde;
            break;
        case 3:
            colour = color.unknownA;
            y = 0x11c;
            break;
        }
        if (D_001609c4[i].active) {
            sprintf(big, "%s", allhosps[i].unknown0);
            sprintf(small, "%ld", plotbid.bids[i]);
            if (i == plotbid.bidder) {
                LbDrawBox(0x10e, 0x187, 0xe, 0x19, colour);
                draw_frame(0x10e, 0x187, 0xe, 0x19, (u16)color.unknown9);
                LbTextDraw(resvars.f40[68].x, resvars.f40[68].y, big);
                LbTextDraw(resvars.f40[69].x, resvars.f40[69].y, small);
            }
            LbDrawBox(0x10e, y, 0xe, 0x19, colour);
            draw_frame(0x10e, y, 0xe, 0x19, (u16)color.unknown9);
            LbTextDraw(resvars.f40[56 + row].x, resvars.f40[56 + row].y, big);
            LbTextDraw(resvars.f40[61 + row].x, resvars.f40[61 + row].y, small);
            row++;
        }
    }
}

// FUNCTION: 0x00068480
void plot_bid_money(u8 who)
{
    plotbid.bids[who] = plotbid.price;
    plotbid.bidder = who;
    plotbid.price += plotbid.step;
    plotbid.timer = 100;
}

// FUNCTION: 0x000684d0
void process_bid_time(void)
{
    if (plotbid.timer > 0) {
        plotbid.timer--;
    } else {
        plotbid.timer = 100;
        if (plotbid.bidder != 0x63) {
            LbMouseSetWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
            if (plotbid.bidder == player) {
                D_001549d0[player].type = 0x36;
                D_001549d0[player].value = (u16)othervar.f78;
            } else {
                town.unknown2 = 1;
            }
        } else {
            town.unknown2 = 1;
        }
    }
    if (bid_hammer[plotbid.timer] == 3)
        play_sound(10, 0xa5, -1, -1);
    reqspr_draw(resvars.f40[70].x, resvars.f40[70].y, &screen_sprites_tab_start[bid_hammer[plotbid.timer]]);
}

// FUNCTION: 0x000685cc
void draw_money_on_town(void)
{
    u8 digits;
    u8 h;
    u32 diff;
    char buf[100];

    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetClipWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    set_font_from_cache(0x1a);
    h = lbFontPtr ? lbFontPtr[18].height : 0;
    if (othervar.f3b != myhosp->f35) {
        diff = fn_000cb2db(othervar.f3b - myhosp->f35);
        if (diff >= 1000000)
            digits = 0;
        else if (diff >= 100000)
            digits = 1;
        else if (diff >= 10000)
            digits = 2;
        else if (diff >= 1000)
            digits = 3;
        else if (diff >= 100)
            digits = 4;
        else if (diff >= 10)
            digits = 5;
        else
            digits = 6;
        LbTextSetClipWindow(resvars.f3c_TownViewp->f20 + resvars.f40[54].x * digits, resvars.f3c_TownViewp->f24,
                            resvars.f3c_TownViewp->f28 - resvars.f40[54].x * digits, h);
        sprintf(buf, "%7d", myhosp->f35);
        LbTextDraw(resvars.f40[55].x, resvars.f40[55].y - (h - othervar.f3f * 6), buf);
        sprintf(buf, "%7d", othervar.f3b);
        LbTextDraw(resvars.f40[55].x, resvars.f40[55].y + othervar.f3f * 6, buf);
        if (digits) {
            LbTextSetClipWindow(resvars.f3c_TownViewp->f30, resvars.f3c_TownViewp->f34, resvars.f40[54].x * (digits - 3) + resvars.f3c_TownViewp->f38, h);
            sprintf(buf, "%7d", myhosp->f35);
            LbTextDraw(resvars.f40[55].x, resvars.f40[55].y, buf);
        }
        othervar.f3f++;
        if (othervar.f3f == 4) {
            othervar.f3f = 0;
            othervar.f3b = myhosp->f35;
            D_00156910 = 1;
        }
    } else {
        LbTextSetClipWindow(resvars.f3c_TownViewp->f20, resvars.f3c_TownViewp->f24, resvars.f3c_TownViewp->f28, h);
        sprintf(buf, "%7d", myhosp->f35);
        LbTextDraw(resvars.f40[55].x, resvars.f40[55].y, buf);
    }
}

// FUNCTION: 0x000688c0
void setup_town_bid(void)
{
    othervar.f9ef = display_mode;
}

// FUNCTION: 0x000688e4
extern "C" void fn_000688e4(void)
{
    if (othervar.f9ef) {
        display_mode = othervar.f9ef;
        switch (display_mode) {
        case 9:
            D_00158192 = 0x5a;
            break;
        case 0:
        case 11:
            return_from_screen(0, 1, 1);
            break;
        case 1:
            bank.unknown2 = 0;
            drugs.unknown2 = 0;
            pol.unknown2 = 0;
            research.unknown2 = 0;
            graphs.unknown2 = 0;
            slist.unknown2 = 0;
            manager.unknown2 = 0;
            reputation.unknown2 = 0;
            variables.unknown2 = 0;
            objs.unknown2 = 1;
            options.unknown2 = 1;
            que.unknown2 = 0;
            break;
        case 4:
            town.unknown2 = 0;
            break;
        case 2:
        case 3:
        case 5:
        case 6:
        case 7:
        case 8:
        case 10:
        default:
            break;
        }
        othervar.f9ef = 0;
    } else {
        return_from_screen(0, 1, 1);
    }
}

// FUNCTION: 0x00068a14
extern "C" void fn_00068a14(void)
{
    bank.unknown2 = 0;
    drugs.unknown2 = 0;
    pol.unknown2 = 0;
    research.unknown2 = 0;
    graphs.unknown2 = 0;
    slist.unknown2 = 0;
    manager.unknown2 = 0;
    D_00158166 = 0;
    reputation.unknown2 = 0;
    variables.unknown2 = 0;
    yearend.unknown2 = 0;
    hall.unknown2 = 0;
}
