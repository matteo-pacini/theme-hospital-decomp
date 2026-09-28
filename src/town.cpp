#include "hosp.h"

extern "C" {
extern Event D_001549d0[];
extern u8 D_00155421;
extern u8 D_001553c7;
extern u8 D_001553c8;
extern u8 D_00188c7b;
extern u8 D_00188c7c;
extern volatile s32 D_00188c5c;
extern volatile s32 D_00188c60;
}

extern ClickArea area_towns[];
extern PlotBid plotbid;
extern Background background[];
extern u16 land_amount[];
extern PlotAdjacent adjacent_plots[];
extern u8 adjacent_plot_numbers[][6];

u8 process_town_panel(void);
u16 check_for_sale(void);
u8 can_buy_plot(const u8 plot);
void draw_sale_area_outlines(u16 x, u16 y);
void adjust_mapwho(u16 tile);
void remove_partition_walls(void);
void place_partition_walls(void);
void mow_grass(void);
void recolour_all_rooms(void);
u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 n);

// NONMATCHING: register choice
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
    if (D_00188c7b) {
        plot = check_for_sale();
        if (plot) {
            if (*(u16 *)&expertise[53].unknown10[3] * land_amount[plot] < myhosp->f35) {
                play_sound(10, 0x45, -1, -1);
                if (statvar.unknownfd > 1) {
                    D_001549d0[player].type = 0x58;
                    D_001549d0[player].value = plot;
                    D_00155421 = 1;
                } else {
                    D_001549d0[player].type = 0x36;
                    D_001549d0[player].value = plot;
                    D_00155421 = 1;
                }
            }
            D_00155421 = 1;
        }
        D_00188c7b = 0;
    }
    if (D_00188c7c) {
        x = (mouse_dx - area_towns[myhosp->f29].x) / area_towns[myhosp->f29].width;
        y = (mouse_dy - area_towns[myhosp->f29].y) / area_towns[myhosp->f29].height;
        goto_column_row_in_window(window.unknown2eb, (u16)x, (u16)y);
        if (!check_if_point_beyond_scroll_limit(window.f0[window.unknown2eb], window.fc[window.unknown2eb], 0)) {
            town.unknown2 = 1;
            play_sound(10, 0x41, -1, -1);
        } else {
            window.f0[window.unknown2eb] = old_x;
            window.fc[window.unknown2eb] = old_y;
        }
        D_00188c7c = 0;
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

// NONMATCHING: register choice
// FUNCTION: 0x00063e54
u16 check_for_sale(void)
{
    s16 x;
    s16 y;

    if (area_towns[myhosp->f29].x <= D_00188c5c && (area_towns[myhosp->f29].width << 7) + area_towns[myhosp->f29].x > D_00188c5c) {
        if (area_towns[myhosp->f29].y <= D_00188c60 && (area_towns[myhosp->f29].height << 7) + area_towns[myhosp->f29].y > D_00188c60) {
            x = (D_00188c5c - area_towns[myhosp->f29].x) / area_towns[myhosp->f29].width;
            y = (D_00188c60 - area_towns[myhosp->f29].y) / area_towns[myhosp->f29].height;
            if (level->plots[x + y * 128])
                return can_buy_plot(level->plots[x + y * 128]);
        }
    }
    return 0;
}

// NONMATCHING: register choice
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
                              area_towns[myhosp->f29].height * y + area_towns[myhosp->f29].y,
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
                              area_towns[myhosp->f29].height * y + area_towns[myhosp->f29].y,
                              area_towns[myhosp->f29].width / 2, area_towns[myhosp->f29].height, color.unknown7);
                    break;
                }
            }
            }
        }
    }
}

// NONMATCHING: register choice
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
                              area_towns[myhosp->f29].height * y + area_towns[myhosp->f29].y,
                              area_towns[myhosp->f29].width, area_towns[myhosp->f29].height / 2, color.unknown7);
            }
            if (map[(s16)x + ((s16)y << 7)].unknown4) {
                if ((blocks[map[(s16)x + ((s16)y << 7)].unknown4].width & 1) && !(blocks[map[(s16)x + ((s16)y << 7)].unknown4].width & 0x40))
                    LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                              area_towns[myhosp->f29].height * y + area_towns[myhosp->f29].y,
                              area_towns[myhosp->f29].width / 2, area_towns[myhosp->f29].height, color.unknown7);
            }
            if (map[(s16)(x - 1) + ((s16)y << 7)].unknown3 && map[(s16)x + ((s16)(y - 1) << 7)].unknown4) {
                if ((blocks[map[(s16)(x - 1) + ((s16)y << 7)].unknown3].width & 1) && !(blocks[map[(s16)(x - 1) + ((s16)y << 7)].unknown3].width & 0x40)) {
                    if ((blocks[map[(s16)x + ((s16)(y - 1) << 7)].unknown4].width & 1) && !(blocks[map[(s16)x + ((s16)(y - 1) << 7)].unknown4].width & 0x40))
                        LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                  area_towns[myhosp->f29].height * y + area_towns[myhosp->f29].y, 1, 1, color.unknown7);
                }
            }
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x00064a60
void draw_sale_area_outlines(u16 x, u16 y)
{
    if (x) {
        if (level->plots[(s16)x + ((s16)y << 7)] != level->plots[(s16)(x - 1) + ((s16)y << 7)])
            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                      area_towns[myhosp->f29].height * y + area_towns[myhosp->f29].y,
                      area_towns[myhosp->f29].width / 2, area_towns[myhosp->f29].height, color.unknown7);
    }
    if (y) {
        if (level->plots[(s16)x + ((s16)y << 7)] != level->plots[(s16)x + ((s16)(y - 1) << 7)])
            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                      area_towns[myhosp->f29].height * y + area_towns[myhosp->f29].y,
                      area_towns[myhosp->f29].width, area_towns[myhosp->f29].height / 2, color.unknown7);
    }
    if (x > 1 && y > 1) {
        if (level->plots[(s16)x + ((s16)y << 7)] != level->plots[((s16)(y - 1) << 7) + (s16)(x - 1)])
            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                      area_towns[myhosp->f29].height * y + area_towns[myhosp->f29].y, 1, 1, color.unknown7);
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x00064db0
void buy_land(u16 plot)
{
    s32 total;
    u16 i;

    for_sale[plot] = 1 << D_001553c7;
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
            map[i].unknown7 |= 1 << D_001553c7;
            level->plots[i] = 0;
            total += *(u16 *)&expertise[53].unknown10[3];
        }
    }
    remove_partition_walls();
    place_partition_walls();
    mow_grass();
    if (statvar.unknownfd > 1)
        change_balance(6, -plotbid.bids[D_001553c7], 0, (u16)D_001553c7);
    else if (total > 0)
        change_balance(6, -total, 0, (u16)D_001553c7);
    D_001553c8 = 1;
    add_static_shadows();
}

// NONMATCHING: register choice
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
