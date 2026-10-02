#include "hosp.h"

/*
 * The town map's entries of the screen box table (resvars.screen_boxes).
 */
struct TownView {
    u8 unknown0[0x10];
    // 0x10: X of the town map.
    s32 x;
    // 0x14: Y of the town map.
    s32 y;
    u8 unknown18[8];
    // 0x20: Clip window of the balance's rolling digits.
    s32 balance_clip_x;
    // 0x24: Its y.
    s32 balance_clip_y;
    // 0x28: Its width.
    s32 balance_clip_w;
    s32 f2c;
    // 0x30: Clip window of the balance digits that stay still while the others
    // roll.
    s32 balance_fixed_clip_x;
    // 0x34: Its y.
    s32 balance_fixed_clip_y;
    // 0x38: Its width before the extra digits.
    s32 balance_fixed_clip_w;
};

extern "C" {
extern Event D_001549d0[];
extern char **town_names;
extern char **town_details;
extern u8 D_00156910;
extern Menu overview;
extern Menu choice;
void LbMouseSetWindow(s32 x, s32 y, s32 width, s32 height);
void LbTextSetClipWindow(s32 x, s32 y, s32 width, s32 height);
void conv_string(char *dst, s8 *src, u8 mode);
void setup_bid_map(void);
void draw_hilight_plot(void);
void draw_bid_text(void);
void display_overhead_map(void);
void draw_town_select(void);
void process_keys_town_select(void);
void remove_town_bid(void);
void reset_screen_states(void);
void display_bid_map(void);
extern u8 *palette;
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
u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 with_margin);

u8 testcol = 0x94;

// Runs one frame of the town map (town.state: 0 opening, 7 shown, 1
// closing back to the game) and of the network land auction (0xb, 0xc
// setting it up, 0x45 running).
// FUNCTION: 0x00063680
void draw_town_select(void)
{
    u32 i;
    u32 total;

    switch (town.state) {
    case 0:
        lbDisplay.left_button = lbDisplay.rleft_button = 0;
        setup_abort_all_requesters();
        othervar.pending_heating_level = myhosp->heating_level;
        othervar.f6b_u32 = 0x32;
        load_ingame_pic(SCR_TOWN, 1, 0xa, level_number, SCREEN_MODE_13);
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        if (!town.value)
            town.value = 0x1f;
        calculate_land_amount();
        set_mouse_mode(MM_TOWN_MAP);
        set_font_from_cache(0);
        setup_overhead_map();
        town.state = 7;
        othervar.town_map_dirty = 1;
        display_overhead_map();
        draw_money_on_town();
        set_font_from_cache(0);
        othervar.heat_icon_time = 0x1e;
        LbMouseSetWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
        break;
    case 1:
        lbDisplay.left_button = lbDisplay.rleft_button = 0;
        LbGhostTableGenerate(palette, 0x42, "qdata/ghost66.dat");
        display_mode = DM_GAME;
        remove_town_bid();
        town.state = 0;
        LbMouseSetWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
        reset_screen_states();
        break;
    case 7:
        compute_help_text(0, 0, 0x13);
        display_overhead_map();
        draw_money_on_town();
        set_font_from_cache(0);
        draw_help_text();
        set_mouse_mode(MM_TOWN_MAP);
        if (lbDisplay.rleft_button) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, 0xd, 0x1a6, 0xc0, 0x2e)) {
                process_keys(0, 0x41);
                LbGhostTableGenerate(palette, 0x42, "qdata/ghost66.dat");
                town.state = 0;
            }
        }
        break;
    case 0xb:
        lbDisplay.left_button = lbDisplay.rleft_button = 0;
        othervar.pending_heating_level = 1;
        othervar.f6b_u32 = 0;
        othervar.pending_heating_level = myhosp->heating_level;
        othervar.f6b_u32 = 0x32;
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        town.value = 3;
        town.state = 0xc;
        calculate_land_amount();
        set_font_from_cache(0xc);
        LbMouseSetWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
        break;
    case 0x45:
        display_bid_map();
        process_bid_panel();
        process_bid_time();
        break;
    case 0xc:
        othervar.town_map_dirty = 1;
        setup_bid_map();
        town.state = 0x45;
        total = 0;
        for (i = 0; i < 0x4000; i++) {
            if (level->plots[i] == othervar.bid_plot)
                total += gbv.land_cost_per_tile;
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
        town.state = 0;
        break;
    }
    if (othervar.heat_icon_time)
        othervar.heat_icon_time--;
}

// Town map keys: Escape closes it.
// FUNCTION: 0x00063a64
void process_keys_town_select(void)
{
    switch (town.state) {
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
    case KC_ESCAPE:
        town.state = 1;
        break;
    default:
        break;
    }
    lbInkey = 0;
}

// Town map clicks: the panel buttons, a left click on a plot for sale buys
// it if affordable (EV_BUY_LAND, or an auction in a network game), and a
// right click elsewhere centres the view there and closes the map.
// FUNCTION: 0x00063ae8
void process_mouse_town_select(void)
{
    s32 old_x;
    s32 old_y;
    s32 x;
    s32 y;
    u16 plot;

    old_x = window.scroll_x[window.current_window];
    old_y = window.scroll_y[window.current_window];
    switch (town.state) {
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
            if (myhosp->balance > land_amount[plot] * gbv.land_cost_per_tile) {
                play_sound(SNDSRC_GENERAL, SFX_CASHREG, -1, -1);
                if (statvar.num_players > 1) {
                    D_001549d0[player].type = EV_SETUP_TOWN_BID;
                    D_001549d0[player].value = plot;
                    othervar.town_map_dirty = 1;
                } else {
                    D_001549d0[player].type = EV_BUY_LAND;
                    D_001549d0[player].value = plot;
                    othervar.town_map_dirty = 1;
                }
            }
            othervar.town_map_dirty = 1;
        }
        lbDisplay.left_button = 0;
    }
    if (lbDisplay.right_button) {
        x = (lbDisplay.mmouse_x - area_towns[myhosp->level].x) / area_towns[myhosp->level].width;
        y = (lbDisplay.mmouse_y - area_towns[myhosp->level].y) / area_towns[myhosp->level].height;
        goto_column_row_in_window(window.current_window, (u16)x, (u16)y);
        if (!check_if_point_beyond_scroll_limit(window.scroll_x[window.current_window],
                                                window.scroll_y[window.current_window], 0)) {
            town.state = 1;
            play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
        } else {
            window.scroll_x[window.current_window] = old_x;
            window.scroll_y[window.current_window] = old_y;
        }
        lbDisplay.right_button = 0;
    }
}

// Returns plot if this player may buy it: it has no adjacency rule for this
// level, or borders land the player owns (for_sale). Returns 0 otherwise,
// and 1 for plot 0.
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

// Returns the plot under the mouse on the town map if this player may buy
// it, else 0.
// FUNCTION: 0x00063e54
u16 check_for_sale(void)
{
    s16 x;
    s16 y;

    if (area_towns[myhosp->level].x <= lbDisplay.mouse_x &&
        (area_towns[myhosp->level].width << 7) + area_towns[myhosp->level].x > lbDisplay.mouse_x) {
        if (area_towns[myhosp->level].y <= lbDisplay.mouse_y &&
            (area_towns[myhosp->level].height << 7) + area_towns[myhosp->level].y > lbDisplay.mouse_y) {
            x = (lbDisplay.mouse_x - area_towns[myhosp->level].x) / area_towns[myhosp->level].width;
            y = (lbDisplay.mouse_y - area_towns[myhosp->level].y) / area_towns[myhosp->level].height;
            if (level->plots[x + y * 128])
                return can_buy_plot(level->plots[x + y * 128]);
        }
    }
    return 0;
}

// Draws the hospital's walls (all but the 0x8e-0x91 kinds) onto the town
// map.
// FUNCTION: 0x00064030
void display_hospital_map_overhead(void)
{
    u32 x;
    u32 y;

    for (x = 0; x < 128; x++) {
        for (y = 0; y < 128; y++) {
            if (map[(s16)x + ((s16)y << 7)].wall_n) {
                if (map[(s16)x + ((s16)y << 7)].wall_n < 0x95) {
                    switch (map[(s16)x + ((s16)y << 7)].wall_n) {
                    case 0x8e:
                    case 0x8f:
                    case 0x90:
                    case 0x91:
                        break;
                    default:
                        LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                                  area_towns[myhosp->level].y + area_towns[myhosp->level].height * y,
                                  area_towns[myhosp->level].width, area_towns[myhosp->level].height / 2, color.white);
                        break;
                    }
                }
            }
            if (map[(s16)x + ((s16)y << 7)].wall_w) {
                if (map[(s16)x + ((s16)y << 7)].wall_w < 0x95) {
                    switch (map[(s16)x + ((s16)y << 7)].wall_w) {
                    case 0x8e:
                    case 0x8f:
                    case 0x90:
                    case 0x91:
                        break;
                    default:
                        LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                                  area_towns[myhosp->level].y + area_towns[myhosp->level].height * y,
                                  area_towns[myhosp->level].width / 2, area_towns[myhosp->level].height, color.white);
                        break;
                    }
                }
            }
        }
    }
}

// Draws the town map: each tile coloured by owner, plots for sale this
// player may buy highlighted, plot outlines and walls.
// FUNCTION: 0x00064338
void display_hospital_map_sale(void)
{
    u16 x;
    u16 y;
    u8 colour;

    for (x = 1; x < 128; x++) {
        for (y = 1; y < 128; y++) {
            colour = color.dark_blue;
            if (map[(s16)x + ((s16)y << 7)].owner_flags & 2)
                colour = color.purple;
            if (map[(s16)x + ((s16)y << 7)].owner_flags & 4)
                colour = color.orange;
            if (map[(s16)x + ((s16)y << 7)].owner_flags & 8)
                colour = color.green;
            if (background[map[(s16)x + ((s16)y << 7)].overlay].show_on_map)
                LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                          area_towns[myhosp->level].y + y * area_towns[myhosp->level].height,
                          area_towns[myhosp->level].width, area_towns[myhosp->level].height, colour);
            if (level->plots[(s16)x + ((s16)y << 7)] && can_buy_plot(level->plots[(s16)x + ((s16)y << 7)]))
                LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                          area_towns[myhosp->level].y + y * area_towns[myhosp->level].height,
                          area_towns[myhosp->level].width, area_towns[myhosp->level].height, color.red);
            draw_sale_area_outlines(x, y);
            if (map[(s16)x + ((s16)y << 7)].wall_n) {
                if ((blocks[map[(s16)x + ((s16)y << 7)].wall_n].width & 1) &&
                    !(blocks[map[(s16)x + ((s16)y << 7)].wall_n].width & 0x40))
                    LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                              area_towns[myhosp->level].y + y * area_towns[myhosp->level].height,
                              area_towns[myhosp->level].width, area_towns[myhosp->level].height / 2, color.white);
            }
            if (map[(s16)x + ((s16)y << 7)].wall_w) {
                if ((blocks[map[(s16)x + ((s16)y << 7)].wall_w].width & 1) &&
                    !(blocks[map[(s16)x + ((s16)y << 7)].wall_w].width & 0x40))
                    LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                              area_towns[myhosp->level].y + y * area_towns[myhosp->level].height,
                              area_towns[myhosp->level].width / 2, area_towns[myhosp->level].height, color.white);
            }
            if (map[(s16)(x - 1) + ((s16)y << 7)].wall_n && map[(s16)x + ((s16)(y - 1) << 7)].wall_w) {
                if ((blocks[map[(s16)(x - 1) + ((s16)y << 7)].wall_n].width & 1) &&
                    !(blocks[map[(s16)(x - 1) + ((s16)y << 7)].wall_n].width & 0x40)) {
                    if ((blocks[map[(s16)x + ((s16)(y - 1) << 7)].wall_w].width & 1) &&
                        !(blocks[map[(s16)x + ((s16)(y - 1) << 7)].wall_w].width & 0x40))
                        LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                                  area_towns[myhosp->level].y + y * area_towns[myhosp->level].height, 1, 1,
                                  color.white);
                }
            }
        }
    }
}

// Draws the outline between tile (x, y) and its neighbours where they
// belong to different plots.
// FUNCTION: 0x00064a60
void draw_sale_area_outlines(u16 x, u16 y)
{
    if (x) {
        if (level->plots[(s16)x + ((s16)y << 7)] != level->plots[(s16)(x - 1) + ((s16)y << 7)])
            LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                      area_towns[myhosp->level].y + y * area_towns[myhosp->level].height,
                      area_towns[myhosp->level].width / 2, area_towns[myhosp->level].height, color.white);
    }
    if (y) {
        if (level->plots[(s16)x + ((s16)y << 7)] != level->plots[(s16)x + ((s16)(y - 1) << 7)])
            LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                      area_towns[myhosp->level].y + y * area_towns[myhosp->level].height,
                      area_towns[myhosp->level].width, area_towns[myhosp->level].height / 2, color.white);
    }
    if (x > 1 && y > 1) {
        if (level->plots[(s16)x + ((s16)y << 7)] != level->plots[((s16)(y - 1) << 7) + (s16)(x - 1)])
            LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                      area_towns[myhosp->level].y + y * area_towns[myhosp->level].height, 1, 1, color.white);
    }
}

// Gives plot `plot` to player othervar.cur_player: copies its tiles from the level
// (placing its objects), marks it as theirs, rebuilds the partition walls
// and grass, and charges the price (the winning bid in a network game).
// FUNCTION: 0x00064db0
void buy_land(u16 plot)
{
    s32 total;
    u16 i;

    for_sale[plot] = 1 << othervar.cur_player;
    total = 0;
    for (i = 0; i < 0x4000; i++) {
        if (level->plots[i] == plot) {
            map[i].wall_n = level->tiles[i].wall_n;
            map[i].wall_w = level->tiles[i].wall_w;
            map[i].first = level->tiles[i].first;
            adjust_mapwho(i);
            map[i].overlay = level->tiles[i].overlay;
            map[i].flags = level->tiles[i].flags & 0x3f;
            map[i].floor = level->tiles[i].floor & 0xf0;
            map[i].owner_flags = level->tiles[i].owner_flags;
            map[i].owner_flags |= 1 << othervar.cur_player;
            level->plots[i] = 0;
            total += gbv.land_cost_per_tile;
        }
    }
    remove_partition_walls();
    place_partition_walls();
    mow_grass();
    if (statvar.num_players > 1)
        change_balance(TX_BUY_LAND, -plotbid.bids[othervar.cur_player], 0, (u16)othervar.cur_player);
    else if (total > 0)
        change_balance(TX_BUY_LAND, -total, 0, (u16)othervar.cur_player);
    othervar.full_redraw = 1;
    add_static_shadows();
}

// Lays the two grass tiles in a chequer pattern over all grass cells.
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

// Puts back the partition walls along the edges of land not yet bought.
// FUNCTION: 0x000650f4
void place_partition_walls(void)
{
    u16 x;
    u16 y;

    for (x = 0; x < 128; x++) {
        for (y = 0; y < 128; y++) {
            if (!(map[(s16)x + ((s16)y << 7)].owner_flags & MAP_OUTDOOR)) {
                if (map[(s16)(x + 1) + ((s16)y << 7)].owner_flags & MAP_OUTDOOR) {
                    if (level->tiles[(s16)(x + 1) + ((s16)y << 7)].wall_w == 0x8d)
                        map[(s16)(x + 1) + ((s16)y << 7)].wall_w = 0x8d;
                }
                if (map[(s16)x + ((s16)(y + 1) << 7)].owner_flags & MAP_OUTDOOR) {
                    if (level->tiles[(s16)x + ((s16)(y + 1) << 7)].wall_n == 0x8c)
                        map[(s16)x + ((s16)(y + 1) << 7)].wall_n = 0x8c;
                }
            }
        }
    }
}

// Removes partition walls that no longer separate unbought land (keeping a
// plain wall where a room borders it), and recolours the rooms.
// FUNCTION: 0x00065248
void remove_partition_walls(void)
{
    u16 i;
    u8 next_open;
    u8 prev_open;

    for (i = 0; i < 0x4000; i++) {
        if (map[i].wall_n == 0x8c) {
            next_open = prev_open = 0;
            if (!(map[i + 128].owner_flags & MAP_OUTDOOR))
                next_open = 1;
            if (!(map[i - 128].owner_flags & MAP_OUTDOOR))
                prev_open = 1;
            if (next_open && prev_open) {
                if (which_room(i + 128) || which_room(i - 128))
                    map[i].wall_n = 0x52;
                else
                    map[i].wall_n = 0;
            }
        }
        if (map[i].wall_w == 0x8d) {
            next_open = prev_open = 0;
            if (!(map[i + 1].owner_flags & MAP_OUTDOOR))
                next_open = 1;
            if (!(map[i - 1].owner_flags & MAP_OUTDOOR))
                prev_open = 1;
            if (next_open && prev_open) {
                if (which_room(i + 1) || which_room(i - 1))
                    map[i].wall_w = 0x53;
                else
                    map[i].wall_w = 0;
            }
        }
    }
    recolour_all_rooms();
}

// With flag set, drops already-sold plots from the level's plot map.
// Otherwise rebuilds the map from the level: the public land and each
// player's starting plot, everything else blank with grass and random
// decorations, then the partition walls and shadow blocks.
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
                map[i].wall_n = level->tiles[i].wall_n;
                map[i].wall_w = level->tiles[i].wall_w;
                map[i].first = level->tiles[i].first;
                map[i].overlay = level->tiles[i].overlay;
                map[i].flags = level->tiles[i].flags & 0x3f;
                map[i].floor = level->tiles[i].floor & 0xf0;
                map[i].owner_flags = level->tiles[i].owner_flags;
                map[i].owner_flags &= ~1;
                map[i].owner_flags &= ~2;
                map[i].owner_flags &= ~4;
                map[i].owner_flags &= ~8;
                level->plots[i] = 0;
                map[i].first = level->tiles[i].first;
                adjust_mapwho(i);
                for_sale[0] = 0x10;
                break;
            case 1:
                if (level->player_count >= 1) {
                    found = 1;
                    map[i].wall_n = level->tiles[i].wall_n;
                    map[i].wall_w = level->tiles[i].wall_w;
                    map[i].first = level->tiles[i].first;
                    map[i].overlay = level->tiles[i].overlay;
                    map[i].flags = level->tiles[i].flags & 0x3f;
                    map[i].floor = level->tiles[i].floor & 0xf0;
                    map[i].owner_flags = level->tiles[i].owner_flags;
                    map[i].owner_flags |= 1;
                    level->plots[i] = 0;
                    map[i].first = level->tiles[i].first;
                    adjust_mapwho(i);
                    for_sale[1] = 1;
                }
                break;
            case 2:
                if (level->player_count >= 2) {
                    found = 1;
                    map[i].wall_n = level->tiles[i].wall_n;
                    map[i].wall_w = level->tiles[i].wall_w;
                    map[i].first = level->tiles[i].first;
                    map[i].overlay = level->tiles[i].overlay;
                    map[i].flags = level->tiles[i].flags & 0x3f;
                    map[i].floor = level->tiles[i].floor & 0xf0;
                    map[i].owner_flags = level->tiles[i].owner_flags;
                    map[i].owner_flags |= 2;
                    level->plots[i] = 0;
                    map[i].first = level->tiles[i].first;
                    adjust_mapwho(i);
                    for_sale[2] = 2;
                }
                break;
            case 3:
                if (level->player_count >= 3) {
                    found = 1;
                    map[i].wall_n = level->tiles[i].wall_n;
                    map[i].wall_w = level->tiles[i].wall_w;
                    map[i].first = level->tiles[i].first;
                    map[i].overlay = level->tiles[i].overlay;
                    map[i].flags = level->tiles[i].flags & 0x3f;
                    map[i].floor = level->tiles[i].floor & 0xf0;
                    map[i].owner_flags = level->tiles[i].owner_flags;
                    map[i].owner_flags |= 4;
                    level->plots[i] = 0;
                    map[i].first = level->tiles[i].first;
                    adjust_mapwho(i);
                    for_sale[3] = 4;
                }
                break;
            case 4:
                if (level->player_count >= 4) {
                    found = 1;
                    map[i].wall_n = level->tiles[i].wall_n;
                    map[i].wall_w = level->tiles[i].wall_w;
                    map[i].first = level->tiles[i].first;
                    map[i].overlay = level->tiles[i].overlay;
                    map[i].flags = level->tiles[i].flags & 0x3f;
                    map[i].floor = level->tiles[i].floor & 0xf0;
                    map[i].owner_flags = level->tiles[i].owner_flags;
                    map[i].owner_flags |= 8;
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
                map[i].wall_n = 0;
                map[i].wall_w = 0;
                map[i].flags = background[map[i].overlay].map_defaults[1] & 0x3f;
                map[i].owner_flags = background[map[i].overlay].map_defaults[0];
            }
        }
        mow_grass();
        rnd = 0xbb40e62d;
        for (i = 0; i < 0x4000; i++) {
            rnd *= 0xbb40e62d;
            if (level->plots[i]) {
                if (map[i].overlay == 0) {
                    if (((rnd >> 8) & 0x7fff) % 50 == 0)
                        map[i].wall_n = ((rnd >> 8) & 0x7fff & 7) + 0xb6;
                }
            }
        }
    }
    place_partition_walls();
    if (!flag)
        precalculate_shadow_blocks();
}

// Places the object a level tile starts with (kind and direction packed in
// Map.first), owned by player 0, and clears it from the cell.
// FUNCTION: 0x00065fc0
void adjust_mapwho(u16 tile)
{
    u8 dir;
    u8 kind;
    u16 id;
    Thing *obj;

    if (map[tile].first) {
        kind = map[tile].first >> 8;
        dir = map[tile].first;
        map[tile].first = 0;
        othervar.force_place_objects = 1;
        id = place_object(tile, (u16)kind, dir, othervar.cur_player);
        if (id) {
            obj = &thing[id];
            obj->owner = 0;
        }
        othervar.force_place_objects = 0;
    }
}

// Draws the town map's details for the tile under the mouse: plot number,
// whether it is for sale or whose it is, its size and price.
// FUNCTION: 0x00066094
void display_plot_under_mouse(void)
{
    s16 x;
    s16 y;
    u16 i;
    u16 plot;
    u16 line_height;
    s32 price;
    Map *cell;
    char buf[100];

    i = 5;
    set_font_from_cache(0x18);
    line_height = lbFontPtr ? lbFontPtr[34].height : 0;
    LbTextSetWindow(0xe6, 0x30 - line_height / 2, 0x180, 0x18);
    lbDisplay.draw_flags = 0x100;
    sprintf(buf, "%s", town_names[myhosp->level]);
    conv_string(buf, (s8 *)buf, 0);
    LbTextDraw(0, 0, buf);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    sprintf(buf, "%s", town_details[4]);
    LbTextDraw(resvars.screen_coords[46].x, resvars.screen_coords[46].y, buf);
    sprintf(buf, "%s", town_details[5]);
    LbTextDraw(resvars.screen_coords[47].x, resvars.screen_coords[47].y, buf);
    sprintf(buf, "%s", town_details[6]);
    LbTextDraw(resvars.screen_coords[48].x, resvars.screen_coords[48].y, buf);
    sprintf(buf, "%s", town_details[7]);
    LbTextDraw(resvars.screen_coords[49].x, resvars.screen_coords[49].y, buf);
    x = (lbDisplay.mmouse_x - area_towns[myhosp->level].x) / area_towns[myhosp->level].width;
    y = (lbDisplay.mmouse_y - area_towns[myhosp->level].y) / area_towns[myhosp->level].height;
    if ((x >= 0 && y >= 0) && (x < 128 && y < 128)) {
        plot = level->plots[x + (y << 7)];
        if (plot) {
            price = land_amount[plot] * gbv.land_cost_per_tile;
            sprintf(buf, ": %d", plot);
            LbTextDraw(resvars.screen_coords[50].x, resvars.screen_coords[50].y, buf);
            sprintf(buf, ": %s", town_details[2]);
            LbTextDraw(resvars.screen_coords[51].x, resvars.screen_coords[51].y, buf);
            sprintf(buf, ": %d", land_amount[plot]);
            LbTextDraw(resvars.screen_coords[52].x, resvars.screen_coords[52].y, buf);
            sprintf(buf, ": $%d", price);
            LbTextDraw(resvars.screen_coords[53].x, resvars.screen_coords[53].y, buf);
        } else {
            cell = &map[x + (y << 7)];
            sprintf(buf, ": -");
            LbTextDraw(resvars.screen_coords[50].x, resvars.screen_coords[50].y, buf);
            sprintf(buf, ": %s", town_details[3]);
            for (i = 0; i < 4; i++) {
                if (cell->owner_flags & (1 << i)) {
                    sprintf(buf, ": %s", allhosps[i].name);
                    break;
                }
            }
            LbTextDraw(resvars.screen_coords[51].x, resvars.screen_coords[51].y, buf);
            sprintf(buf, ": -");
            LbTextDraw(resvars.screen_coords[52].x, resvars.screen_coords[52].y, buf);
            sprintf(buf, ": -");
            LbTextDraw(resvars.screen_coords[53].x, resvars.screen_coords[53].y, buf);
        }
    } else {
        sprintf(buf, ": -");
        LbTextDraw(resvars.screen_coords[50].x, resvars.screen_coords[50].y, buf);
        sprintf(buf, ": -");
        LbTextDraw(resvars.screen_coords[51].x, resvars.screen_coords[51].y, buf);
        sprintf(buf, ": -");
        LbTextDraw(resvars.screen_coords[52].x, resvars.screen_coords[52].y, buf);
        sprintf(buf, ": -");
        LbTextDraw(resvars.screen_coords[53].x, resvars.screen_coords[53].y, buf);
    }
}

// Counts the tiles of each plot (land_amount).
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

// Draws the hospital's walls into the town map's background (bscreen).
// FUNCTION: 0x00066764
void setup_overhead_map(void)
{
    s32 unused;

    othervar.saved_screen_ptr = lbDisplay.wscreen;
    othervar.saved_screen_width = lbDisplay.graphics_screen_width;
    othervar.saved_screen_height = lbDisplay.graphics_screen_height;
    lbDisplay.wscreen = bscreen;
    lbDisplay.graphics_screen_width = lbDisplay.physical_screen_width;
    lbDisplay.graphics_screen_height = lbDisplay.physical_screen_height;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
    display_hospital_map_overhead();
    lbDisplay.wscreen = othervar.saved_screen_ptr;
    lbDisplay.graphics_screen_width = othervar.saved_screen_width;
    lbDisplay.graphics_screen_height = othervar.saved_screen_height;
    LbScreenSetGraphicsWindow(0, 0, othervar.saved_screen_width, othervar.saved_screen_height);
    othervar.town_map_dirty = 0;
}

// Loads the auction screen and draws the map with the plot for sale into
// its background.
// FUNCTION: 0x0006680c
extern "C" void setup_bid_map(void)
{
    s32 unused;

    load_ingame_pic(SCR_TOWN_BID, 1, 10, level_number, SCREEN_MODE_13);
    othervar.saved_screen_ptr = lbDisplay.wscreen;
    othervar.saved_screen_width = lbDisplay.graphics_screen_width;
    othervar.saved_screen_height = lbDisplay.graphics_screen_height;
    lbDisplay.wscreen = bscreen;
    lbDisplay.graphics_screen_width = lbDisplay.physical_screen_width;
    lbDisplay.graphics_screen_height = lbDisplay.physical_screen_height;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
    LbDrawBox(0x14c, 0x49, 0x103, 0x107, 0x6c);
    draw_frame(0x14b, 0x48, 0x104, 0x108, 0x68);
    draw_hilight_plot();
    lbDisplay.wscreen = othervar.saved_screen_ptr;
    lbDisplay.graphics_screen_width = othervar.saved_screen_width;
    lbDisplay.graphics_screen_height = othervar.saved_screen_height;
    LbScreenSetGraphicsWindow(0, 0, othervar.saved_screen_width, othervar.saved_screen_height);
    othervar.town_map_dirty = 0;
}

// Draws the town map with the shown overlays (town.value bits) and the
// details under the mouse.
// FUNCTION: 0x00066914
extern "C" void display_overhead_map(void)
{
    u8 i;

    if (othervar.town_map_dirty)
        setup_overhead_map();
    copy_screen_to_wscreen(bscreen);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    display_hospital_map_sale();
    draw_things_on_map();
    for (i = 0; i < 5; i++) {
        if (!(town.value & (1 << i)))
            reqspr_draw(resvars.screen_coords[34 + i].x, resvars.screen_coords[34 + i].y,
                        &screen_sprites_tab_start[i + 1]);
    }
    display_plot_under_mouse();
}

// Draws the auction screen: the map with the plot for sale and the bids.
// FUNCTION: 0x000669fc
extern "C" void display_bid_map(void)
{
    if (othervar.town_map_dirty)
        setup_bid_map();
    copy_screen_to_wscreen(bscreen);
    draw_hilight_plot();
    draw_bid_text();
}

// Draws the overlay markers on the town map (people, plants, fire
// extinguishers, machines, radiators by town.value bits; doors always)
// and the hospital's counts and heating level.
// FUNCTION: 0x00066a34
void draw_things_on_map(void)
{
    u16 x;
    u16 i;
    u16 y;
    Thing *occupant;
    char buf[50];

    for (x = 0; x < 128; x++) {
        for (y = 0; y < 128; y++) {
            if (map[(s16)x + ((s16)y << 7)].first > 0) {
                occupant = &thing[map[(s16)x + ((s16)y << 7)].first];
                switch (occupant->type) {
                case THING_OBJECT:
                    switch (occupant->subtype) {
                    case OBJ_PLANT:
                        if (town.value & 2)
                            LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                                      area_towns[myhosp->level].y + y * area_towns[myhosp->level].height, 3, 3,
                                      color.pale_green);
                        break;
                    case OBJ_FIRE_EXTINGUISHER:
                        if (town.value & 4)
                            LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                                      area_towns[myhosp->level].y + y * area_towns[myhosp->level].height, 3, 3,
                                      color.red);
                        break;
                    case OBJ_RADIATOR:
                        if (town.value & 0x10) {
                            LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                                      area_towns[myhosp->level].y + y * area_towns[myhosp->level].height, 1, 1,
                                      color.light_red);
                            lbDisplay.draw_flags = 4;
                            if (occupant->owner == player) {
                                if (othervar.heat_icon_time) {
                                    if (myhosp->heating_level)
                                        reqspr_draw(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width -
                                                        screen_sprites_tab_start[myhosp->heating_level + 9].width / 2,
                                                    area_towns[myhosp->level].y + y * area_towns[myhosp->level].height -
                                                        screen_sprites_tab_start[myhosp->heating_level + 9].height / 2,
                                                    &screen_sprites_tab_start[myhosp->heating_level + 9]);
                                }
                            }
                            lbDisplay.draw_flags = 0;
                        }
                        break;
                    case OBJ_ENTRANCE_LEFT_DOOR:
                    case OBJ_ENTRANCE_RIGHT_DOOR:
                        if (occupant->facing == 0 || occupant->facing == 2)
                            LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                                      area_towns[myhosp->level].y + y * area_towns[myhosp->level].height, 7, 3,
                                      color.grey);
                        else
                            LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                                      area_towns[myhosp->level].y + y * area_towns[myhosp->level].height, 3, 7,
                                      color.grey);
                        break;
                    case OBJ_DOOR:
                    case OBJ_SWING_DOOR:
                    case OBJ_SWING_DOOR2:
                        LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                                  area_towns[myhosp->level].y + y * area_towns[myhosp->level].height, 3, 3, color.grey);
                        break;
                    default:
                        if (objects[occupant->subtype].category == 1 || objects[occupant->subtype].category == 2) {
                            if (town.value & 8)
                                LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                                          area_towns[myhosp->level].y + y * area_towns[myhosp->level].height, 3, 3,
                                          color.cyan);
                        }
                        break;
                    }
                    break;
                case THING_STAFF:
                case THING_MAYOR:
                case THING_INSPECTOR:
                    if (town.value & 1)
                        LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                                  area_towns[myhosp->level].y + y * area_towns[myhosp->level].height, 2, 2, color.blue);
                    break;
                case THING_PATIENT:
                    if ((town.value & 1) &&
                        !(map[((s16)(u16)occupant->y.tile << 7) + (s16)(u16)occupant->x.tile].owner_flags &
                          MAP_OUTDOOR))
                        LbDrawBox(area_towns[myhosp->level].x + x * area_towns[myhosp->level].width,
                                  area_towns[myhosp->level].y + y * area_towns[myhosp->level].height, 2, 2,
                                  color.white);
                    break;
                }
            }
        }
    }
    set_font_from_cache(0x19);
    sprintf(buf, "%d", adcounts.patients);
    LbTextDraw(resvars.screen_coords[39].x, resvars.screen_coords[39].y, buf);
    sprintf(buf, "%d", myhosp->object_stats[45].count);
    LbTextDraw(resvars.screen_coords[40].x, resvars.screen_coords[40].y, buf);
    sprintf(buf, "%d", myhosp->object_stats[43].count);
    LbTextDraw(resvars.screen_coords[41].x, resvars.screen_coords[41].y, buf);
    sprintf(buf, "%d", myhosp->machine_count);
    LbTextDraw(resvars.screen_coords[42].x, resvars.screen_coords[42].y, buf);
    sprintf(buf, "%d", myhosp->radiator_count);
    LbTextDraw(resvars.screen_coords[43].x, resvars.screen_coords[43].y, buf);
    sprintf(buf, "%d", myhosp->radiator_count * myhosp->heating_level * mycontrol->heating_price / 100);
    LbTextDraw(resvars.screen_coords[44].x, resvars.screen_coords[44].y, buf);
    if (myhosp->heating_level) {
        for (i = 0; i < myhosp->heating_level * 6; i++)
            reqspr_draw(resvars.screen_coords[45].x + i, resvars.screen_coords[45].y, &screen_sprites_tab_start[9]);
    }
}

// Town map panel clicks: runs the callback of the button released over,
// drawing it pressed while held. Returns 1 if the mouse was on a button.
// FUNCTION: 0x000675a8
u8 process_town_panel(void)
{
    u8 i;
    u8 hit;

    hit = 0;
    if (lbDisplay.rleft_button || lbDisplay.rright_button) {
        for (i = 0; resvars.screen_buttons[i + 0x3b].width != 0 && resvars.screen_buttons[i + 0x3b].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_buttons[i + 0x3b].x,
                             resvars.screen_buttons[i + 0x3b].y, (s16)resvars.screen_buttons[i + 0x3b].width,
                             (s16)resvars.screen_buttons[i + 0x3b].height)) {
                hit = 1;
                resvars.screen_buttons[i + 0x3b].callback(i);
                if (resvars.screen_buttons[i + 0x3b].sprite)
                    reqspr_draw(resvars.screen_buttons[i + 0x3b].sprite_x, resvars.screen_buttons[i + 0x3b].sprite_y,
                                &screen_sprites_tab_start[resvars.screen_buttons[i + 0x3b].sprite]);
                break;
            }
        }
        lbDisplay.rleft_button = lbDisplay.rright_button = 0;
    }
    if (lbDisplay.mleft_button || lbDisplay.mright_button) {
        for (i = 0; resvars.screen_buttons[i + 0x3b].width != 0 && resvars.screen_buttons[i + 0x3b].height != 0; i++) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_buttons[i + 0x3b].x,
                             resvars.screen_buttons[i + 0x3b].y, (s16)resvars.screen_buttons[i + 0x3b].width,
                             (s16)resvars.screen_buttons[i + 0x3b].height)) {
                hit = 1;
                if (resvars.screen_buttons[i + 0x3b].sprite)
                    reqspr_draw(resvars.screen_buttons[i + 0x3b].sprite_x, resvars.screen_buttons[i + 0x3b].sprite_y,
                                &screen_sprites_tab_start[resvars.screen_buttons[i + 0x3b].sprite]);
                lbDisplay.left_button = lbDisplay.right_button = 0;
                break;
            }
        }
    }
    return hit;
}

// Town map buttons: 0-4 toggle the people, plant, fire extinguisher,
// machine and radiator overlays (a right click shows only that one), 5/6
// lower or raise the heating (event EV_SET_HEATING, 1-10), 7 closes the map.
// FUNCTION: 0x000678dc
void process_town_button(u8 button)
{
    switch (button) {
    case 0:
        if (lbDisplay.rright_button)
            town.value = 0;
        town.value ^= 1;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    case 1:
        if (lbDisplay.rright_button)
            town.value = 0;
        town.value ^= 2;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    case 2:
        if (lbDisplay.rright_button)
            town.value = 0;
        town.value ^= 4;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    case 3:
        if (lbDisplay.rright_button)
            town.value = 0;
        town.value ^= 8;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    case 4:
        if (lbDisplay.rright_button)
            town.value = 0;
        town.value ^= 0x10;
        if (town.value & 0x10)
            othervar.heat_icon_time = 0x1e;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    case 6:
        if (myhosp->heating_level < 10) {
            D_001549d0[player].type = EV_SET_HEATING;
            D_001549d0[player].value = (u16)myhosp->heating_level + 1;
            othervar.pending_heating_level = myhosp->heating_level + 1;
            othervar.f6b = 0x32;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        }
        othervar.heat_icon_time = 0x1e;
        break;
    case 5:
        if (myhosp->heating_level > 1) {
            D_001549d0[player].type = EV_SET_HEATING;
            D_001549d0[player].value = (u16)myhosp->heating_level - 1;
            othervar.pending_heating_level = myhosp->heating_level - 1;
            othervar.f6b = 0x32;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        }
        othervar.heat_icon_time = 0x1e;
        break;
    case 7:
        town.state = 1;
        play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        break;
    }
}

// Auction screen clicks: runs the callback of the button released over,
// drawing it pressed while held.
// FUNCTION: 0x00067b80
void process_bid_panel(void)
{
    u8 i;

    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.screen_buttons[i + 0x44].width != 0 && resvars.screen_buttons[i + 0x44].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_buttons[i + 0x44].x,
                             resvars.screen_buttons[i + 0x44].y, (s16)resvars.screen_buttons[i + 0x44].width,
                             (s16)resvars.screen_buttons[i + 0x44].height)) {
                resvars.screen_buttons[i + 0x44].callback(i);
                reqspr_draw(resvars.screen_buttons[i + 0x44].sprite_x, resvars.screen_buttons[i + 0x44].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i + 0x44].sprite]);
            }
        }
        lbDisplay.rleft_button = lbDisplay.left_button = 0;
    }
    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.screen_buttons[i + 0x44].width != 0 && resvars.screen_buttons[i + 0x44].height != 0; i++) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_buttons[i + 0x44].x,
                             resvars.screen_buttons[i + 0x44].y, (s16)resvars.screen_buttons[i + 0x44].width,
                             (s16)resvars.screen_buttons[i + 0x44].height))
                reqspr_draw(resvars.screen_buttons[i + 0x44].sprite_x, resvars.screen_buttons[i + 0x44].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i + 0x44].sprite]);
        }
    }
}

// Auction buttons 0 and 1: bid the current price (EV_PLOT_BID_MONEY) if
// affordable.
// FUNCTION: 0x00067e44
void process_bid_button(u8 button)
{
    switch (button) {
    case 0:
    case 1:
        if (myhosp->balance >= plotbid.price) {
            D_001549d0[player].type = EV_PLOT_BID_MONEY;
            play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
        } else {
            play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        }
        break;
    case 2:
        break;
    }
}

// Draws the auction map: each tile coloured by owner, the plot for sale
// (othervar.bid_plot) highlighted and walls.
// FUNCTION: 0x00067ed4
extern "C" void draw_hilight_plot(void)
{
    u16 x;
    u16 y;
    u8 colour;

    for (x = 0; x < 128; x++) {
        for (y = 0; y < 128; y++) {
            colour = color.dark_blue;
            if (map[(s16)x + ((s16)y << 7)].owner_flags & 2)
                colour = color.purple;
            if (map[(s16)x + ((s16)y << 7)].owner_flags & 4)
                colour = color.orange;
            if (map[(s16)x + ((s16)y << 7)].owner_flags & 8)
                colour = color.green;
            if (background[map[(s16)x + ((s16)y << 7)].overlay].show_on_map)
                LbDrawBox(resvars.screen_boxes_TownViewp->x + x * 2, resvars.screen_boxes_TownViewp->y + y * 2, 2, 2,
                          colour);
            if (level->plots[(s16)x + ((s16)y << 7)] == othervar.bid_plot)
                LbDrawBox(resvars.screen_boxes_TownViewp->x + x * 2, resvars.screen_boxes_TownViewp->y + y * 2, 2, 2,
                          color.red);
            if (map[(s16)x + ((s16)y << 7)].wall_n) {
                if (map[(s16)x + ((s16)y << 7)].wall_n < 0x95)
                    LbDrawBox(resvars.screen_boxes_TownViewp->x + x * 2, resvars.screen_boxes_TownViewp->y + y * 2, 2,
                              1, 0);
            }
            if (map[(s16)x + ((s16)y << 7)].wall_w) {
                if (map[(s16)x + ((s16)y << 7)].wall_w < 0x95)
                    LbDrawBox(resvars.screen_boxes_TownViewp->x + x * 2, resvars.screen_boxes_TownViewp->y + y * 2, 1,
                              2, 0);
            }
        }
    }
    othervar.hilight_plot_idx++;
    if (!hilight_plot[othervar.hilight_plot_idx])
        othervar.hilight_plot_idx = 0;
}

// Draws the auction's current price and each player's last bid, marking
// the leading bidder.
// FUNCTION: 0x00068194
extern "C" void draw_bid_text(void)
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
    LbTextDraw(resvars.screen_coords[67].x, resvars.screen_coords[67].y, small);
    row = 0;
    for (i = 0; i < 4; i++) {
        switch (i) {
        case 0:
            colour = color.dark_blue;
            y = 0x61;
            break;
        case 1:
            colour = color.purple;
            y = 0xa0;
            break;
        case 2:
            colour = color.orange;
            y = 0xde;
            break;
        case 3:
            colour = color.green;
            y = 0x11c;
            break;
        }
        if (D_001609c4[i].active) {
            sprintf(big, "%s", allhosps[i].name);
            sprintf(small, "%ld", plotbid.bids[i]);
            if (i == plotbid.bidder) {
                LbDrawBox(0x10e, 0x187, 0xe, 0x19, colour);
                draw_frame(0x10e, 0x187, 0xe, 0x19, (u16)color.dark_blue);
                LbTextDraw(resvars.screen_coords[68].x, resvars.screen_coords[68].y, big);
                LbTextDraw(resvars.screen_coords[69].x, resvars.screen_coords[69].y, small);
            }
            LbDrawBox(0x10e, y, 0xe, 0x19, colour);
            draw_frame(0x10e, y, 0xe, 0x19, (u16)color.dark_blue);
            LbTextDraw(resvars.screen_coords[56 + row].x, resvars.screen_coords[56 + row].y, big);
            LbTextDraw(resvars.screen_coords[61 + row].x, resvars.screen_coords[61 + row].y, small);
            row++;
        }
    }
}

// Player who bids the current price; the price rises by a step and the
// auction clock restarts (100 frames).
// FUNCTION: 0x00068480
void plot_bid_money(u8 who)
{
    plotbid.bids[who] = plotbid.price;
    plotbid.bidder = who;
    plotbid.price += plotbid.step;
    plotbid.timer = 100;
}

// Runs the auction clock: when it runs out the plot goes to the leading
// bidder (EV_BUY_LAND if it is this player) and the auction closes. Draws
// the hammer.
// FUNCTION: 0x000684d0
void process_bid_time(void)
{
    if (plotbid.timer > 0) {
        plotbid.timer--;
    } else {
        plotbid.timer = 100;
        if (plotbid.bidder != 0x63) {
            LbMouseSetWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
            if (plotbid.bidder == player) {
                D_001549d0[player].type = EV_BUY_LAND;
                D_001549d0[player].value = (u16)othervar.bid_plot;
            } else {
                town.state = 1;
            }
        } else {
            town.state = 1;
        }
    }
    if (bid_hammer[plotbid.timer] == 3)
        play_sound(SNDSRC_GENERAL, SFX_AUCTION, -1, -1);
    reqspr_draw(resvars.screen_coords[70].x, resvars.screen_coords[70].y,
                &screen_sprites_tab_start[bid_hammer[plotbid.timer]]);
}

// Draws this player's balance on the town map, rolling the changed digits
// as draw_money_on_panel does.
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
    if (othervar.shown_balance != myhosp->balance) {
        diff = abs(othervar.shown_balance - myhosp->balance);
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
        LbTextSetClipWindow(resvars.screen_boxes_TownViewp->balance_clip_x + resvars.screen_coords[54].x * digits,
                            resvars.screen_boxes_TownViewp->balance_clip_y,
                            resvars.screen_boxes_TownViewp->balance_clip_w - resvars.screen_coords[54].x * digits, h);
        sprintf(buf, "%7d", myhosp->balance);
        LbTextDraw(resvars.screen_coords[55].x, resvars.screen_coords[55].y - (h - othervar.balance_roll_step * 6),
                   buf);
        sprintf(buf, "%7d", othervar.shown_balance);
        LbTextDraw(resvars.screen_coords[55].x, resvars.screen_coords[55].y + othervar.balance_roll_step * 6, buf);
        if (digits) {
            LbTextSetClipWindow(
                resvars.screen_boxes_TownViewp->balance_fixed_clip_x,
                resvars.screen_boxes_TownViewp->balance_fixed_clip_y,
                resvars.screen_coords[54].x * (digits - 3) + resvars.screen_boxes_TownViewp->balance_fixed_clip_w, h);
            sprintf(buf, "%7d", myhosp->balance);
            LbTextDraw(resvars.screen_coords[55].x, resvars.screen_coords[55].y, buf);
        }
        othervar.balance_roll_step++;
        if (othervar.balance_roll_step == 4) {
            othervar.balance_roll_step = 0;
            othervar.shown_balance = myhosp->balance;
            D_00156910 = 1;
        }
    } else {
        LbTextSetClipWindow(resvars.screen_boxes_TownViewp->balance_clip_x,
                            resvars.screen_boxes_TownViewp->balance_clip_y,
                            resvars.screen_boxes_TownViewp->balance_clip_w, h);
        sprintf(buf, "%7d", myhosp->balance);
        LbTextDraw(resvars.screen_coords[55].x, resvars.screen_coords[55].y, buf);
    }
}

// Remembers the display mode to return to after a land auction.
// FUNCTION: 0x000688c0
void setup_town_bid(void)
{
    othervar.town_bid_return_mode = display_mode;
}

// Leaves the town map or auction: back to the display mode saved by
// setup_town_bid, or to the game.
// FUNCTION: 0x000688e4
extern "C" void remove_town_bid(void)
{
    if (othervar.town_bid_return_mode) {
        display_mode = othervar.town_bid_return_mode;
        switch (display_mode) {
        case 9:
            choice.state = 0x5a;
            break;
        case 0:
        case 11:
            return_from_screen(0, 1, 1);
            break;
        case 1:
            bank.state = 0;
            drugs.state = 0;
            pol.state = 0;
            research.state = 0;
            graphs.state = 0;
            slist.state = 0;
            manager.state = 0;
            reputation.state = 0;
            variables.state = 0;
            objs.state = 1;
            options.state = 1;
            que.state = 0;
            break;
        case 4:
            town.state = 0;
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
        othervar.town_bid_return_mode = 0;
    } else {
        return_from_screen(0, 1, 1);
    }
}

// Resets the full-screen windows' states so they reopen from scratch.
// FUNCTION: 0x00068a14
extern "C" void reset_screen_states(void)
{
    bank.state = 0;
    drugs.state = 0;
    pol.state = 0;
    research.state = 0;
    graphs.state = 0;
    slist.state = 0;
    manager.state = 0;
    overview.state = 0;
    reputation.state = 0;
    variables.state = 0;
    yearend.state = 0;
    hall.state = 0;
}
