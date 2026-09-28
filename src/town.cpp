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
extern TownView *D_0015f430;
extern Event D_001549d0[];
extern char D_001204cc[];
extern char D_001204cf[];
extern char D_001204d2[];
extern char D_001204d5[];
extern char D_001204d8[];
extern char D_001204db[];
extern char D_001204e0[];
extern char D_001204e5[];
extern char D_001204ea[];
extern char D_001204f0[];
extern char D_001204f4[];
extern char D_001204f9[];
extern char D_001204fe[];
extern char D_00120502[];
extern char D_00120506[];
extern char D_0012050a[];
extern char D_0012050e[];
extern char D_00120512[];
extern char D_00120516[];
extern char D_00120519[];
extern char D_0012051c[];
extern char D_0012051f[];
extern char D_00120522[];
extern char D_00120525[];
extern char **D_001602c4;
extern u8 D_00155e42;
extern char **D_001602c8;
extern Point *D_0015f434;
extern Button *D_0015f43c;
extern u8 D_00155430;
extern u8 D_00155431;
extern u8 D_0012cc20[];
extern u8 D_0012cfbc[];
extern s32 D_001553f3;
extern u8 D_001553f7;
extern u8 D_00156910;
extern u8 D_00158166;
extern u8 D_00158192;
extern u16 D_00155da7;
extern char D_00120533[];
extern char D_00120537[];
extern char D_0012053b[];
extern char D_0012053f[];
void LbMouseSetWindow(s32 x, s32 y, s32 w, s32 h);
void LbTextSetClipWindow(s32 x, s32 y, s32 w, s32 h);
extern char D_00120528[];
extern char D_0012052c[];
extern char D_0012052f[];
extern volatile s16 D_00188c6c, D_00188c70;
extern u8 D_00188c7e, D_00188c7f, D_00188c81, D_00188c83;
extern u16 D_00188c74;
void fn_0001b7e0(char *dst, s8 *src, u8 mode);
void fn_0006680c(void);
void fn_00067ed4(void);
void fn_00068194(void);
extern u8 D_00155421;
extern u8 D_00155422;
extern s32 D_00155423;
extern u8 D_001553c7;
extern u8 D_001553c8;
extern u8 D_00155d73;
extern u8 *D_00155d8f;
extern s32 D_00155d93;
extern s32 D_00155d97;
extern s32 D_00188c38;
extern u8 D_00188c7b;
extern u8 D_00188c7c;
extern volatile s32 D_00188c5c;
extern volatile s32 D_00188c60;
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

// NONMATCHING: register choice
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
            if (level->plots[i] && map[i].overlay == 0 && ((rnd >> 8) & 0x7fff) % 50 == 0)
                map[i].unknown3 = ((rnd >> 8) & 0x7fff & 7) + 0xb6;
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
        D_00155d73 = 1;
        id = fn_00037098(tile, (u16)kind, dir, D_001553c7);
        if (id) {
            t = &thing[id];
            t->owner = 0;
        }
        D_00155d73 = 0;
    }
}

// NONMATCHING: register choice
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
    D_00188c74 = 0x100;
    sprintf(buf, D_001204cc, D_001602c4[myhosp->f29]);
    fn_0001b7e0(buf, (s8 *)buf, 0);
    LbTextDraw(0, 0, buf);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    sprintf(buf, D_001204cf, D_001602c8[4]);
    LbTextDraw(D_0015f434[46].x, D_0015f434[46].y, buf);
    sprintf(buf, D_001204d2, D_001602c8[5]);
    LbTextDraw(D_0015f434[47].x, D_0015f434[47].y, buf);
    sprintf(buf, D_001204d5, D_001602c8[6]);
    LbTextDraw(D_0015f434[48].x, D_0015f434[48].y, buf);
    sprintf(buf, D_001204d8, D_001602c8[7]);
    LbTextDraw(D_0015f434[49].x, D_0015f434[49].y, buf);
    x = (mouse_dx - area_towns[myhosp->f29].x) / area_towns[myhosp->f29].width;
    y = (mouse_dy - area_towns[myhosp->f29].y) / area_towns[myhosp->f29].height;
    if ((x >= 0 && y >= 0) && (x < 128 && y < 128)) {
        plot = level->plots[x + (y << 7)];
        if (plot) {
            price = *(u16 *)&expertise[53].unknown10[3] * land_amount[plot];
            sprintf(buf, D_001204db, plot);
            LbTextDraw(D_0015f434[50].x, D_0015f434[50].y, buf);
            sprintf(buf, D_001204e0, D_001602c8[2]);
            LbTextDraw(D_0015f434[51].x, D_0015f434[51].y, buf);
            sprintf(buf, D_001204e5, land_amount[plot]);
            LbTextDraw(D_0015f434[52].x, D_0015f434[52].y, buf);
            sprintf(buf, D_001204ea, price);
            LbTextDraw(D_0015f434[53].x, D_0015f434[53].y, buf);
        } else {
            m = &map[x + (y << 7)];
            sprintf(buf, D_001204f0);
            LbTextDraw(D_0015f434[50].x, D_0015f434[50].y, buf);
            sprintf(buf, D_001204f4, D_001602c8[3]);
            for (i = 0; i < 4; i++) {
                if (m->unknown7 & (1 << i)) {
                    sprintf(buf, D_001204f9, allhosps[i].unknown0);
                    break;
                }
            }
            LbTextDraw(D_0015f434[51].x, D_0015f434[51].y, buf);
            sprintf(buf, D_001204fe);
            LbTextDraw(D_0015f434[52].x, D_0015f434[52].y, buf);
            sprintf(buf, D_00120502);
            LbTextDraw(D_0015f434[53].x, D_0015f434[53].y, buf);
        }
    } else {
        sprintf(buf, D_00120506);
        LbTextDraw(D_0015f434[50].x, D_0015f434[50].y, buf);
        sprintf(buf, D_0012050a);
        LbTextDraw(D_0015f434[51].x, D_0015f434[51].y, buf);
        sprintf(buf, D_0012050e);
        LbTextDraw(D_0015f434[52].x, D_0015f434[52].y, buf);
        sprintf(buf, D_00120512);
        LbTextDraw(D_0015f434[53].x, D_0015f434[53].y, buf);
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

    D_00155d8f = lbDisplay.unknown4;
    D_00155d93 = D_00188c34;
    D_00155d97 = D_00188c38;
    lbDisplay.unknown4 = bscreen;
    D_00188c34 = lbDisplay.unknown18;
    D_00188c38 = lbDisplay.unknown1c;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    display_hospital_map_overhead();
    lbDisplay.unknown4 = D_00155d8f;
    D_00188c34 = D_00155d93;
    D_00188c38 = D_00155d97;
    LbScreenSetGraphicsWindow(0, 0, D_00155d93, D_00155d97);
    D_00155421 = 0;
}

// FUNCTION: 0x0006680c
extern "C" void fn_0006680c(void)
{
    s32 unused;

    load_ingame_pic(10, 1, 10, level_number, SCREEN_MODE_13);
    D_00155d8f = lbDisplay.unknown4;
    D_00155d93 = D_00188c34;
    D_00155d97 = D_00188c38;
    lbDisplay.unknown4 = bscreen;
    D_00188c34 = lbDisplay.unknown18;
    D_00188c38 = lbDisplay.unknown1c;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    LbDrawBox(0x14c, 0x49, 0x103, 0x107, 0x6c);
    draw_frame(0x14b, 0x48, 0x104, 0x108, 0x68);
    fn_00067ed4();
    lbDisplay.unknown4 = D_00155d8f;
    D_00188c34 = D_00155d93;
    D_00188c38 = D_00155d97;
    LbScreenSetGraphicsWindow(0, 0, D_00155d93, D_00155d97);
    D_00155421 = 0;
}

// FUNCTION: 0x00066914
extern "C" void fn_00066914(void)
{
    u8 i;

    if (D_00155421)
        setup_overhead_map();
    copy_screen_to_wscreen(bscreen);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    display_hospital_map_sale();
    draw_things_on_map();
    for (i = 0; i < 5; i++) {
        if (!(town.unknown0 & (1 << i)))
            reqspr_draw(D_0015f434[34 + i].x, D_0015f434[34 + i].y, &screen_sprites_tab_start[i + 1]);
    }
    display_plot_under_mouse();
}

// FUNCTION: 0x000669fc
extern "C" void fn_000669fc(void)
{
    if (D_00155421)
        fn_0006680c();
    copy_screen_to_wscreen(bscreen);
    fn_00067ed4();
    fn_00068194();
}

// NONMATCHING: register choice
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
                                      y * area_towns[myhosp->f29].height + area_towns[myhosp->f29].y, 3, 3, color.unknown13);
                        break;
                    case 0x2b:
                        if (town.unknown0 & 4)
                            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                      y * area_towns[myhosp->f29].height + area_towns[myhosp->f29].y, 3, 3, color.unknownD);
                        break;
                    case 0x2c:
                        if (town.unknown0 & 0x10) {
                            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                      y * area_towns[myhosp->f29].height + area_towns[myhosp->f29].y, 1, 1, color.unknownC);
                            D_00188c74 = 4;
                            if (t->owner == player) {
                                if (D_00155e42) {
                                    if (myhosp->fc94)
                                        reqspr_draw(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width - screen_sprites_tab_start[myhosp->fc94 + 9].width / 2,
                                            area_towns[myhosp->f29].y + area_towns[myhosp->f29].height * y - screen_sprites_tab_start[myhosp->fc94 + 9].height / 2,
                                            &screen_sprites_tab_start[myhosp->fc94 + 9]);
                                }
                            }
                            D_00188c74 = 0;
                        }
                        break;
                    case 0x3a:
                    case 0x3b:
                        if (t->unknown17 == 0 || t->unknown17 == 2)
                            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                      y * area_towns[myhosp->f29].height + area_towns[myhosp->f29].y, 7, 3, color.unknown11);
                        else
                            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                      y * area_towns[myhosp->f29].height + area_towns[myhosp->f29].y, 3, 7, color.unknown11);
                        break;
                    case 3:
                    case 0x34:
                    case 0x35:
                        LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                  y * area_towns[myhosp->f29].height + area_towns[myhosp->f29].y, 3, 3, color.unknown11);
                        break;
                    default:
                        if (objects[t->subtype].unknown14 == 1 || objects[t->subtype].unknown14 == 2) {
                            if (town.unknown0 & 8)
                            LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                      y * area_towns[myhosp->f29].height + area_towns[myhosp->f29].y, 3, 3, color.unknownE);
                        }
                        break;
                    }
                    break;
                case 4:
                case 0x14:
                case 0x16:
                    if (town.unknown0 & 1)
                        LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                  y * area_towns[myhosp->f29].height + area_towns[myhosp->f29].y, 2, 2, color.unknown12);
                    break;
                case 2:
                    if ((town.unknown0 & 1) && !(map[((s16)(u16)t->y.tile << 7) + (s16)(u16)t->x.tile].unknown7 & 0x10))
                        LbDrawBox(area_towns[myhosp->f29].x + x * area_towns[myhosp->f29].width,
                                  y * area_towns[myhosp->f29].height + area_towns[myhosp->f29].y, 2, 2, color.unknown7);
                    break;
                }
            }
        }
    }
    set_font_from_cache(0x19);
    sprintf(buf, D_00120516, adcounts.f0);
    LbTextDraw(D_0015f434[39].x, D_0015f434[39].y, buf);
    sprintf(buf, D_00120519, myhosp->object_stats[45].fA);
    LbTextDraw(D_0015f434[40].x, D_0015f434[40].y, buf);
    sprintf(buf, D_0012051c, myhosp->object_stats[43].fA);
    LbTextDraw(D_0015f434[41].x, D_0015f434[41].y, buf);
    sprintf(buf, D_0012051f, myhosp->fc97);
    LbTextDraw(D_0015f434[42].x, D_0015f434[42].y, buf);
    sprintf(buf, D_00120522, myhosp->fc92);
    LbTextDraw(D_0015f434[43].x, D_0015f434[43].y, buf);
    sprintf(buf, D_00120525, myhosp->fc92 * myhosp->fc94 * mycontrol->f64c / 100);
    LbTextDraw(D_0015f434[44].x, D_0015f434[44].y, buf);
    if (myhosp->fc94) {
        for (i = 0; i < myhosp->fc94 * 6; i++)
            reqspr_draw(D_0015f434[45].x + i, D_0015f434[45].y, &screen_sprites_tab_start[9]);
    }
}

// FUNCTION: 0x000675a8
u8 process_town_panel(void)
{
    u8 i;
    u8 hit;

    hit = 0;
    if (D_00188c81 || D_00188c83) {
        for (i = 0; D_0015f43c[i + 0x3b].width != 0 && D_0015f43c[i + 0x3b].height != 0; i++) {
            if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i + 0x3b].x, D_0015f43c[i + 0x3b].y,
                             (s16)D_0015f43c[i + 0x3b].width, (s16)D_0015f43c[i + 0x3b].height)) {
                hit = 1;
                D_0015f43c[i + 0x3b].callback(i);
                if (D_0015f43c[i + 0x3b].sprite)
                    reqspr_draw(D_0015f43c[i + 0x3b].unknownC, D_0015f43c[i + 0x3b].unknownE,
                                &screen_sprites_tab_start[D_0015f43c[i + 0x3b].sprite]);
                break;
            }
        }
        D_00188c81 = D_00188c83 = 0;
    }
    if (D_00188c7e || D_00188c7f) {
        for (i = 0; D_0015f43c[i + 0x3b].width != 0 && D_0015f43c[i + 0x3b].height != 0; i++) {
            if (over_hotspot(mouse_dx, mouse_dy, D_0015f43c[i + 0x3b].x, D_0015f43c[i + 0x3b].y,
                             (s16)D_0015f43c[i + 0x3b].width, (s16)D_0015f43c[i + 0x3b].height)) {
                hit = 1;
                if (D_0015f43c[i + 0x3b].sprite)
                    reqspr_draw(D_0015f43c[i + 0x3b].unknownC, D_0015f43c[i + 0x3b].unknownE,
                                &screen_sprites_tab_start[D_0015f43c[i + 0x3b].sprite]);
                D_00188c7b = D_00188c7c = 0;
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
        if (D_00188c83)
            town.unknown0 = 0;
        town.unknown0 ^= 1;
        play_sound(10, 0x43, -1, -1);
        break;
    case 1:
        if (D_00188c83)
            town.unknown0 = 0;
        town.unknown0 ^= 2;
        play_sound(10, 0x43, -1, -1);
        break;
    case 2:
        if (D_00188c83)
            town.unknown0 = 0;
        town.unknown0 ^= 4;
        play_sound(10, 0x43, -1, -1);
        break;
    case 3:
        if (D_00188c83)
            town.unknown0 = 0;
        town.unknown0 ^= 8;
        play_sound(10, 0x43, -1, -1);
        break;
    case 4:
        if (D_00188c83)
            town.unknown0 = 0;
        town.unknown0 ^= 0x10;
        if (town.unknown0 & 0x10)
            D_00155e42 = 0x1e;
        play_sound(10, 0x43, -1, -1);
        break;
    case 6:
        if (myhosp->fc94 < 10) {
            D_001549d0[player].type = 0x4f;
            D_001549d0[player].value = (u16)myhosp->fc94 + 1;
            D_00155422 = myhosp->fc94 + 1;
            D_00155423 = 0x32;
            play_sound(10, 0x43, -1, -1);
        }
        D_00155e42 = 0x1e;
        break;
    case 5:
        if (myhosp->fc94 > 1) {
            D_001549d0[player].type = 0x4f;
            D_001549d0[player].value = (u16)myhosp->fc94 - 1;
            D_00155422 = myhosp->fc94 - 1;
            D_00155423 = 0x32;
            play_sound(10, 0x43, -1, -1);
        }
        D_00155e42 = 0x1e;
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

    if (D_00188c81) {
        for (i = 0; D_0015f43c[i + 0x44].width != 0 && D_0015f43c[i + 0x44].height != 0; i++) {
            if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i + 0x44].x, D_0015f43c[i + 0x44].y,
                             (s16)D_0015f43c[i + 0x44].width, (s16)D_0015f43c[i + 0x44].height)) {
                D_0015f43c[i + 0x44].callback(i);
                reqspr_draw(D_0015f43c[i + 0x44].unknownC, D_0015f43c[i + 0x44].unknownE,
                            &screen_sprites_tab_start[D_0015f43c[i + 0x44].sprite]);
            }
        }
        D_00188c81 = D_00188c7b = 0;
    }
    if (D_00188c7e) {
        for (i = 0; D_0015f43c[i + 0x44].width != 0 && D_0015f43c[i + 0x44].height != 0; i++) {
            if (over_hotspot(mouse_dx, mouse_dy, D_0015f43c[i + 0x44].x, D_0015f43c[i + 0x44].y,
                             (s16)D_0015f43c[i + 0x44].width, (s16)D_0015f43c[i + 0x44].height))
                reqspr_draw(D_0015f43c[i + 0x44].unknownC, D_0015f43c[i + 0x44].unknownE,
                            &screen_sprites_tab_start[D_0015f43c[i + 0x44].sprite]);
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
                LbDrawBox(D_0015f430->x + x * 2, D_0015f430->y + y * 2, 2, 2, colour);
            if (level->plots[(s16)x + ((s16)y << 7)] == D_00155430)
                LbDrawBox(D_0015f430->x + x * 2, D_0015f430->y + y * 2, 2, 2, color.unknownD);
            if (map[(s16)x + ((s16)y << 7)].unknown3) {
                if (map[(s16)x + ((s16)y << 7)].unknown3 < 0x95)
                    LbDrawBox(D_0015f430->x + x * 2, D_0015f430->y + y * 2, 2, 1, 0);
            }
            if (map[(s16)x + ((s16)y << 7)].unknown4) {
                if (map[(s16)x + ((s16)y << 7)].unknown4 < 0x95)
                    LbDrawBox(D_0015f430->x + x * 2, D_0015f430->y + y * 2, 1, 2, 0);
            }
        }
    }
    D_00155431++;
    if (!D_0012cc20[D_00155431])
        D_00155431 = 0;
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

    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(0xc);
    D_00188c74 = 0;
    sprintf(small, D_00120528, plotbid.price);
    LbTextDraw(D_0015f434[67].x, D_0015f434[67].y, small);
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
            sprintf(big, D_0012052c, allhosps[i].unknown0);
            sprintf(small, D_0012052f, plotbid.bids[i]);
            if (i == plotbid.bidder) {
                LbDrawBox(0x10e, 0x187, 0xe, 0x19, colour);
                draw_frame(0x10e, 0x187, 0xe, 0x19, (u16)color.unknown9);
                LbTextDraw(D_0015f434[68].x, D_0015f434[68].y, big);
                LbTextDraw(D_0015f434[69].x, D_0015f434[69].y, small);
            }
            LbDrawBox(0x10e, y, 0xe, 0x19, colour);
            draw_frame(0x10e, y, 0xe, 0x19, (u16)color.unknown9);
            LbTextDraw(D_0015f434[56 + row].x, D_0015f434[56 + row].y, big);
            LbTextDraw(D_0015f434[61 + row].x, D_0015f434[61 + row].y, small);
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
                D_001549d0[player].value = (u16)D_00155430;
            } else {
                town.unknown2 = 1;
            }
        } else {
            town.unknown2 = 1;
        }
    }
    if (D_0012cfbc[plotbid.timer] == 3)
        play_sound(10, 0xa5, -1, -1);
    reqspr_draw(D_0015f434[70].x, D_0015f434[70].y, &screen_sprites_tab_start[D_0012cfbc[plotbid.timer]]);
}

// NONMATCHING: register choice
// FUNCTION: 0x000685cc
void draw_money_on_town(void)
{
    u8 digits;
    u8 h;
    u32 diff;
    char buf[100];

    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetClipWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    set_font_from_cache(0x1a);
    h = lbFontPtr ? lbFontPtr[18].height : 0;
    if (D_001553f3 != myhosp->f35) {
        diff = fn_000cb2db(D_001553f3 - myhosp->f35);
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
        LbTextSetClipWindow(D_0015f430->f20 + D_0015f434[54].x * digits, D_0015f430->f24,
                            D_0015f430->f28 - digits * D_0015f434[54].x, h);
        sprintf(buf, D_00120533, myhosp->f35);
        LbTextDraw(D_0015f434[55].x, D_0015f434[55].y - (h - D_001553f7 * 6), buf);
        sprintf(buf, D_00120537, D_001553f3);
        LbTextDraw(D_0015f434[55].x, D_0015f434[55].y + D_001553f7 * 6, buf);
        if (digits) {
            LbTextSetClipWindow(D_0015f430->f30, D_0015f430->f34, D_0015f434[54].x * (digits - 3) + D_0015f430->f38, h);
            sprintf(buf, D_0012053b, myhosp->f35);
            LbTextDraw(D_0015f434[55].x, D_0015f434[55].y, buf);
        }
        D_001553f7++;
        if (D_001553f7 == 4) {
            D_001553f7 = 0;
            D_001553f3 = myhosp->f35;
            D_00156910 = 1;
        }
    } else {
        LbTextSetClipWindow(D_0015f430->f20, D_0015f430->f24, D_0015f430->f28, h);
        sprintf(buf, D_0012053f, myhosp->f35);
        LbTextDraw(D_0015f434[55].x, D_0015f434[55].y, buf);
    }
}

// FUNCTION: 0x000688c0
void setup_town_bid(void)
{
    D_00155da7 = display_mode;
}

// FUNCTION: 0x000688e4
extern "C" void fn_000688e4(void)
{
    if (D_00155da7) {
        display_mode = D_00155da7;
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
        D_00155da7 = 0;
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
