#include "hosp.h"

extern "C" {
extern Event D_001549d0[];
extern u8 D_00155421;
extern u8 D_00188c7b;
extern u8 D_00188c7c;
extern volatile s32 D_00188c5c;
extern volatile s32 D_00188c60;
}

extern ClickArea area_towns[];
extern Background background[];
extern u16 land_amount[];
extern PlotAdjacent adjacent_plots[];
extern u8 adjacent_plot_numbers[][6];

u8 process_town_panel(void);
u16 check_for_sale(void);
u8 can_buy_plot(const u8 plot);
void draw_sale_area_outlines(u16 x, u16 y);
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

// FUNCTION: 0x00064338
void display_hospital_map_sale(void)
{
    u16 x;
    u16 y;
    u8 colour;

    for (x = 1; x < 128; x++) {
        for (y = 1; y < 128; y++) {
            colour = color.unknown9;
            if (map[((s16)y << 7) + (s16)x].unknown7 & 2)
                colour = color.unknownF;
            if (map[((s16)y << 7) + (s16)x].unknown7 & 4)
                colour = color.unknownB;
            if (map[((s16)y << 7) + (s16)x].unknown7 & 8)
                colour = color.unknownA;
            if (background[map[((s16)y << 7) + (s16)x].overlay].unknown0)
                LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                          area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height,
                          area_towns[myhosp->f29].width, area_towns[myhosp->f29].height, colour);
            if (level->plots[((s16)y << 7) + (s16)x] && can_buy_plot(level->plots[((s16)y << 7) + (s16)x]))
                LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                          area_towns[myhosp->f29].y + y * area_towns[myhosp->f29].height,
                          area_towns[myhosp->f29].width, area_towns[myhosp->f29].height, color.unknownD);
            draw_sale_area_outlines(x, y);
            if (map[((s16)y << 7) + (s16)x].unknown3) {
                if ((blocks[map[((s16)y << 7) + (s16)x].unknown3].width & 1) && !(blocks[map[((s16)y << 7) + (s16)x].unknown3].width & 0x40))
                    LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                              area_towns[myhosp->f29].height * y + area_towns[myhosp->f29].y,
                              area_towns[myhosp->f29].width, area_towns[myhosp->f29].height / 2, color.unknown7);
            }
            if (map[((s16)y << 7) + (s16)x].unknown4) {
                if ((blocks[map[((s16)y << 7) + (s16)x].unknown4].width & 1) && !(blocks[map[((s16)y << 7) + (s16)x].unknown4].width & 0x40))
                    LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                              area_towns[myhosp->f29].height * y + area_towns[myhosp->f29].y,
                              area_towns[myhosp->f29].width / 2, area_towns[myhosp->f29].height, color.unknown7);
            }
            if (map[((s16)y << 7) + (s16)(x - 1)].unknown3 && map[((s16)(y - 1) << 7) + (s16)x].unknown4) {
                if ((blocks[map[((s16)y << 7) + (s16)(x - 1)].unknown3].width & 1) && !(blocks[map[((s16)y << 7) + (s16)(x - 1)].unknown3].width & 0x40)) {
                    if ((blocks[map[((s16)(y - 1) << 7) + (s16)x].unknown4].width & 1) && !(blocks[map[((s16)(y - 1) << 7) + (s16)x].unknown4].width & 0x40))
                        LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                  area_towns[myhosp->f29].height * y + area_towns[myhosp->f29].y, 1, 1, color.unknown7);
                }
            }
        }
    }
}
