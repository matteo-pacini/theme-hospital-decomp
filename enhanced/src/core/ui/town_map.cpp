#include "core/ui/town_map.h"

#include "core/audio/sounds.h"
#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/panel.h"
#include "core/render/shadows.h"
#include "core/render/text.h"
#include "core/ui/helpmsg.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/object_tables.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace th
{

namespace
{

constexpr uint8_t kSfxCashRegister = 0x45; // SFX_CASHREG

// Object kinds (OBJ_* in include/enums.h) the map marks.
constexpr uint8_t kObjDoor = 3;
constexpr uint8_t kObjFireExtinguisher = 43;
constexpr uint8_t kObjRadiator = 44;
constexpr uint8_t kObjPlant = 45;
constexpr uint8_t kObjSwingDoor = 52;
constexpr uint8_t kObjSwingDoor2 = 53;
constexpr uint8_t kObjEntranceLeftDoor = 58;
constexpr uint8_t kObjEntranceRightDoor = 59;

// The partition walls along land for sale, and the plain walls left where a
// room borders land bought (src/town.cpp).
constexpr uint8_t kPartitionWallN = 0x8c;
constexpr uint8_t kPartitionWallW = 0x8d;
constexpr uint8_t kRoomWallN = 0x52;
constexpr uint8_t kRoomWallW = 0x53;

// area_towns (src/tables.cpp): every hospital level's town map has map cell
// 0 at (230, 25) and 3 x 3 pixels a cell.
constexpr int kMapX = 230;
constexpr int kMapY = 25;
constexpr int kCellW = 3;
constexpr int kCellH = 3;

// The town map's buttons, screenbuttons[0x3b] on (src/scoords.cpp): the
// five overlays, heating down and up, and close, with the sprite drawn
// while one is held (0 for none).
struct TownButton {
    int x, y, w, h;
    int sprite_x, sprite_y, sprite;
};
constexpr std::array<TownButton, 8> kButtons = {{
    {140, 36, 54, 54, 0, 0, 0},
    {140, 90, 54, 54, 0, 0, 0},
    {140, 143, 54, 54, 0, 0, 0},
    {140, 194, 54, 54, 0, 0, 0},
    {140, 247, 54, 54, 0, 0, 0},
    {70, 315, 18, 18, 70, 314, 7},
    {171, 315, 18, 18, 171, 315, 6},
    {595, 438, 25, 25, 594, 437, 8},
}};

// ScreenCoords (src/scoords.cpp): [34]-[38] the overlays' "off" sprites
// (TOWN02V 1-5), [39]-[44] the counts of people, plants, fire
// extinguishers, machines and radiators and the heating's cost, [45] the
// heating bar (sprite 9 per pixel, 6 per level), [46]-[49] the details'
// labels and [50]-[53] their values, [54] a balance digit's width and [55]
// the balance.
constexpr std::array<Point, 5> kOverlayOff = {{{140, 37}, {140, 89}, {140, 141}, {140, 193}, {140, 246}}};
constexpr std::array<Point, 6> kCounts = {{{90, 55}, {90, 108}, {90, 157}, {90, 212}, {90, 268}, {100, 357}}};
constexpr Point kHeatingBar = {101, 319};
constexpr std::array<Point, 4> kDetailLabels = {{{228, 435}, {228, 451}, {404, 435}, {404, 451}}};
constexpr std::array<Point, 4> kDetailValues = {{{299, 435}, {299, 451}, {470, 435}, {470, 451}}};
constexpr int kDigitWidth = 13;
constexpr Point kBalance = {49, 431};
// ScreenBoxes[2] and [3] (TownView): the balance's clip window, and the
// part of it that stays still while the rest rolls, before the digits that
// do.
constexpr Rect kBalanceClip = {50, 431, 89, 0};
constexpr Rect kBalanceFixedClip = {50, 431, 37, 0};
// draw_town_select's hotspot on the balance, which opens the bank manager.
constexpr Rect kBankHotspot = {0xd, 0x1a6, 0xc0, 0x2e};
// display_plot_under_mouse's window for the town's name.
constexpr Rect kTownName = {0xe6, 0x30, 0x180, 0x18};
// The heat icons stay this many frames after the map opens or a heating
// button is clicked (othervar.heat_icon_time).
constexpr int kHeatIconFrames = 0x1e;

// background[].show_on_map (src/tables.cpp): the floor blocks the town map
// colours by owner.
bool shown_on_map(uint8_t floor)
{
    return (floor >= 15 && floor <= 18) || (floor >= 20 && floor <= 39) || floor == 65 || floor == 66 || floor == 75;
}

// The wall blocks of blocks[] (src/blocks.cpp) with flag 0x01 and without
// 0x40, which display_hospital_map_sale draws.
bool solid_wall(uint8_t block)
{
    return (block >= 80 && block <= 133) || block == 140 || block == 141 || block == 146 || block == 147 ||
           block == 150 || (block >= 195 && block <= 210) || (block >= 212 && block <= 219);
}

// adjacent_plots and adjacent_plot_numbers (src/tables.cpp): per level and
// plot with a neighbour rule, the plots it borders.
struct PlotAdjacent {
    uint8_t level;
    uint8_t plot;
    std::array<uint8_t, 6> neighbours;
};
constexpr std::array<PlotAdjacent, 54> kAdjacentPlots = {{
    {6, 7, {1, 6}},
    {7, 7, {2}},
    {8, 4, {5}},
    {9, 8, {4, 5, 6}},
    {9, 5, {2, 6, 8}},
    {9, 6, {7, 8}},
    {9, 7, {2, 6}},
    {10, 3, {10, 4}},
    {10, 7, {2}},
    {10, 9, {2}},
    {10, 4, {3, 12}},
    {10, 5, {3}},
    {10, 10, {2}},
    {10, 6, {3}},
    {10, 8, {3, 11}},
    {10, 11, {6, 8}},
    {10, 12, {1, 4}},
    {10, 13, {7}},
    {10, 14, {6}},
    {11, 4, {1, 2}},
    {11, 6, {1}},
    {11, 5, {1, 3}},
    {11, 7, {4, 6}},
    {11, 9, {8}},
    {12, 6, {4, 8}},
    {12, 9, {3, 7}},
    {12, 7, {5, 9}},
    {12, 10, {4, 11}},
    {12, 11, {4, 10}},
    {12, 13, {5, 14}},
    {12, 14, {7, 13}},
    {20, 13, {15, 7, 5}},
    {20, 15, {13, 14, 17}},
    {20, 14, {6, 15}},
    {20, 17, {16, 15}},
    {22, 3, {1, 8}},
    {22, 4, {2, 10}},
    {22, 9, {8}},
    {22, 11, {10}},
    {23, 10, {11}},
    {23, 11, {12}},
    {23, 12, {9}},
    {30, 8, {5, 7, 9}},
    {30, 12, {10, 11, 14}},
    {30, 13, {14}},
    {30, 4, {5}},
    {33, 5, {6, 8}},
    {34, 7, {4, 6, 8}},
    {34, 12, {10, 11, 14}},
    {40, 7, {5, 6, 8, 9}},
    {40, 11, {8, 14, 12}},
    {40, 10, {9, 13, 18}},
    {40, 26, {22, 23, 27, 28}},
    {40, 25, {17, 21, 24, 29}},
}};

// The candidates of `list` whose `key` is the smallest, in `list`'s order.
template <typename Key> std::vector<int> smallest(const std::vector<int> &list, Key &&key)
{
    int best = 999999;
    for (int i : list)
        best = std::min(best, key(i));
    std::vector<int> out;
    for (int i : list)
        if (key(i) == best)
            out.push_back(i);
    return out;
}

// LbPaletteFindColour (src/lib/bf1/gpalette.cpp): the nearest colour of a
// 6-bit palette by squared distance, ties broken by the sum of the
// differences, then by the darkest (green and red weighted double).
uint8_t find_colour(const std::array<uint8_t, 768> &pal, int r, int g, int b)
{
    auto at = [&](int i, int k) { return int(pal[size_t(i) * 3 + size_t(k)]); };
    auto sq = [](int v) { return v * v; };
    auto dist = [&](int i) { return sq(r - at(i, 0)) + sq(g - at(i, 1)) + sq(b - at(i, 2)); };
    auto manhattan = [&](int i) { return std::abs(r - at(i, 0)) + std::abs(g - at(i, 1)) + std::abs(b - at(i, 2)); };
    auto darkness = [&](int i) { return at(i, 1) * 2 * at(i, 1) + at(i, 0) * 2 * at(i, 0) + at(i, 2) * at(i, 2); };
    std::vector<int> list(256);
    for (int i = 0; i < 256; i++)
        list[size_t(i)] = i;
    list = smallest(list, dist);
    // An exact match is the first one, whatever the ties.
    if (list.size() == 1 || dist(list[0]) == 0)
        return uint8_t(list[0]);
    list = smallest(list, manhattan);
    if (list.size() == 1)
        return uint8_t(list[0]);
    return uint8_t(smallest(list, darkness)[0]);
}

bool over(const UiMouse &m, int x, int y, int w, int h)
{
    return m.x >= x && m.x < x + w && m.y >= y && m.y < y + h;
}

// which_room (src/build.cpp): whether a room's floor holds `tile`.
bool in_room(const World &world, int tile)
{
    int x = tile % kMapSize, y = tile / kMapSize;
    for (size_t r = 1; r < world.rooms.size(); r++) {
        const RoomInfo &room = world.rooms[r];
        if (room.type == kRoomFree)
            continue;
        int x0 = room.pos % kMapSize, y0 = room.pos / kMapSize;
        if (x0 <= x && x <= x0 + room.width - 1 && y0 <= y && y <= y0 + room.height - 1)
            return true;
    }
    return false;
}

// Cells past the map's ends, which the game reads beyond its map array,
// count as outdoors here.
bool indoors(const World &world, int cell)
{
    return cell >= 0 && cell < kMapCells && !(world.map[size_t(cell)].owner_flags & kCellOutdoor);
}

// remove_partition_walls (src/town.cpp): a partition wall with land
// bought on both sides goes, leaving a plain wall if a room is on either.
void remove_partition_walls(World &world)
{
    for (int i = 0; i < kMapCells; i++) {
        MapCell &c = world.map[size_t(i)];
        if (c.wall_n == kPartitionWallN && indoors(world, i + kMapSize) && indoors(world, i - kMapSize))
            c.wall_n = in_room(world, i + kMapSize) || in_room(world, i - kMapSize) ? kRoomWallN : 0;
        if (c.wall_w == kPartitionWallW && indoors(world, i + 1) && indoors(world, i - 1))
            c.wall_w = in_room(world, i + 1) || in_room(world, i - 1) ? kRoomWallW : 0;
    }
}

// place_partition_walls (src/town.cpp), as build_start_map does it.
void place_partition_walls(World &world, const Level &level)
{
    for (int x = 0; x < kMapSize; x++) {
        for (int y = 0; y < kMapSize; y++) {
            int i = x + y * kMapSize;
            if (world.map[size_t(i)].owner_flags & kCellOutdoor)
                continue;
            int east = i + 1, south = i + kMapSize;
            if (east < kMapCells && (world.map[size_t(east)].owner_flags & kCellOutdoor) &&
                level.tiles[size_t(east)].wall_w == kPartitionWallW)
                world.map[size_t(east)].wall_w = kPartitionWallW;
            if (south < kMapCells && (world.map[size_t(south)].owner_flags & kCellOutdoor) &&
                level.tiles[size_t(south)].wall_n == kPartitionWallN)
                world.map[size_t(south)].wall_n = kPartitionWallN;
        }
    }
}

// mow_grass (src/town.cpp).
void mow_grass(World &world)
{
    for (int y = 0; y < kMapSize; y++) {
        uint8_t grass = 2;
        for (int x = 0; x < kMapSize; x++) {
            grass = grass == 0 ? 2 : 0;
            MapCell &c = world.map[size_t(x + y * kMapSize)];
            if (c.floor_block == 0 || c.floor_block == 2)
                c.floor_block = grass;
        }
    }
}

// adjust_mapwho (src/town.cpp): places the object the cell's first holds
// (kind in the high byte, facing in the low) for player 0.
void adjust_mapwho(World &world, const AnimData &anims, int tile)
{
    MapCell &c = world.map[size_t(tile)];
    if (!c.first)
        return;
    int kind = uint16_t(c.first) >> 8;
    int dir = c.first & 0xff;
    c.first = 0;
    // Kinds and facings past the game's tables are not in its levels.
    if (kind >= kObjectKinds || dir >= 4)
        return;
    if (uint16_t id = place_object(world, anims, {tile, kind, dir}, 0, true))
        world.things[id].owner = 0;
}

bool load_picture(const std::filesystem::path &dir, std::string_view name, Surface &pixels,
                  std::array<uint8_t, 768> &vga, std::string *error)
{
    auto dat = load_data_file(dir, "qdata/" + std::string(name) + ".dat", error);
    auto pal = dat ? load_data_file(dir, "qdata/" + std::string(name) + ".pal", error) : std::nullopt;
    if (!pal)
        return false;
    if (dat->size() < size_t(kUiWidth) * kUiHeight || pal->size() < vga.size()) {
        if (error)
            *error = std::string(name) + ": not a full-screen picture";
        return false;
    }
    pixels.resize(kUiWidth, kUiHeight);
    for (int y = 0; y < kUiHeight; y++)
        std::memcpy(pixels.row(y), dat->data() + size_t(y) * kUiWidth, kUiWidth);
    std::memcpy(vga.data(), pal->data(), vga.size());
    return true;
}

} // namespace

void start_town_land(TownLand &land, const Level &level, int number)
{
    land.level = level;
    land.level_number = number;
    land.for_sale = {};
    for (uint16_t &plot : land.level.plots) {
        if (plot == 0) {
            land.for_sale[0] = 0x10;
        } else if (plot <= 4 && level.player_count >= plot) {
            land.for_sale[plot] = uint8_t(1 << (plot - 1));
            plot = 0;
        }
    }
}

int land_amount(const TownLand &land, int plot)
{
    return int(std::count(land.level.plots.begin(), land.level.plots.end(), uint16_t(plot)));
}

int can_buy_plot(const TownLand &land, int plot, int player)
{
    if (plot == 0)
        return 1;
    for (const PlotAdjacent &a : kAdjacentPlots) {
        if (a.level != land.level_number || a.plot != plot)
            continue;
        for (uint8_t n : a.neighbours) {
            if (n == 0)
                break;
            if (land.for_sale[n] == 1 << player)
                return plot;
        }
        return 0;
    }
    return plot;
}

void buy_land(Game &game, TownLand &land, int plot)
{
    if (plot <= 0 || plot >= kPlotCount)
        return;
    World &world = game.world;
    land.for_sale[size_t(plot)] = 1;
    int32_t total = 0;
    for (int i = 0; i < kMapCells; i++) {
        if (land.level.plots[size_t(i)] != plot)
            continue;
        const MapCell &src = land.level.tiles[size_t(i)];
        MapCell &c = world.map[size_t(i)];
        c.wall_n = src.wall_n;
        c.wall_w = src.wall_w;
        c.first = src.first;
        adjust_mapwho(world, game.anims(), i);
        c.floor_block = src.floor_block;
        c.flags = src.flags & 0x3f;
        c.litter_shadow = src.litter_shadow & 0xf0;
        c.owner_flags = src.owner_flags | 1;
        land.level.plots[size_t(i)] = 0;
        total += game.settings().gbv.land_cost_per_tile;
    }
    remove_partition_walls(world);
    place_partition_walls(world, land.level);
    mow_grass(world);
    if (total > 0)
        change_balance(game.finance, game.balance, game.date.day, game.date.month, {kTxBuyLand, -total});
    add_static_shadows(world.map);
}

bool apply_town_event(Game &game, TownLand &land, const Event &event)
{
    switch (event.type) {
    case EV_BUY_LAND:
        buy_land(game, land, event.value);
        return true;
    case EV_SET_HEATING:
        if (!game.world.heating_broken)
            game.finance.heating_level = uint8_t(event.value);
        return true;
    default:
        return false;
    }
}

bool TownMap::load(const std::filesystem::path &dir, const LanguageText &text, std::string *error)
{
    text_ = &text;
    if (!load_picture(dir, "town01v", picture_, vga_, error))
        return false;
    palette_from_vga(vga_, palette_);
    auto tab = load_data_file(dir, "qdata/town02v.tab", error);
    auto dat = tab ? load_data_file(dir, "qdata/town02v.dat", error) : std::nullopt;
    if (!dat)
        return false;
    if (!sprites_.load(std::move(*tab), std::move(*dat), SpriteFormat::Req)) {
        if (error)
            *error = "town02v: not a sprite table";
        return false;
    }
    auto ghost = load_data_file(dir, "qdata/ghost66.dat", error);
    if (!ghost)
        return false;
    if (ghost->size() < 0x10000) {
        if (error)
            *error = "qdata/ghost66.dat: too short";
        return false;
    }
    ghost_ = std::move(*ghost);
    if (!load_font(dir, 0x18, detail_font_, error) || !load_font(dir, 0x19, count_font_, error) ||
        !load_font(dir, 0x1a, money_font_, error))
        return false;
    // setup_colors, once the town's palette is in.
    Colours &c = colours_;
    c.white = find_colour(vga_, 0x3f, 0x3f, 0x3f);
    c.dark_blue = find_colour(vga_, 0, 0, 0x10);
    c.green = find_colour(vga_, 0, 0x3f, 0);
    c.orange = find_colour(vga_, 0x3f, 0x30, 0x17);
    c.light_red = find_colour(vga_, 0x3f, 0x17, 0x17);
    c.red = find_colour(vga_, 0x3f, 0, 0);
    c.cyan = find_colour(vga_, 0x1b, 0x37, 0x39);
    c.purple = find_colour(vga_, 0x35, 0x1f, 0x42);
    c.grey = find_colour(vga_, 0x26, 0x29, 0x2f);
    c.blue = find_colour(vga_, 0, 0x26, 0x56);
    c.pale_green = find_colour(vga_, 0x24, 0x50, 0x2f);
    surface_ = picture_;
    background_ = picture_;
    return true;
}

void TownMap::box(int x, int y, int w, int h, uint8_t colour)
{
    // LbDrawBox, filled and clipped to the screen.
    for (int yy = std::max(y, 0); yy < std::min(y + h, surface_.height()); yy++) {
        uint8_t *row = surface_.row(yy);
        for (int xx = std::max(x, 0); xx < std::min(x + w, surface_.width()); xx++)
            row[xx] = colour;
    }
}

void TownMap::text(const Font &font, int x, int y, std::string_view s)
{
    draw_text(surface_, font, x, y, s, {{0, 0, kUiWidth}, {0, 0, kUiWidth, kUiHeight}});
}

void TownMap::draw_overhead_walls(const Game &game)
{
    // setup_overhead_map: display_hospital_map_overhead into bscreen, kept
    // until the picture is loaded again.
    std::swap(surface_, background_);
    const auto skipped = [](uint8_t w) { return w >= 0x8e && w <= 0x91; };
    for (int x = 0; x < kMapSize; x++) {
        for (int y = 0; y < kMapSize; y++) {
            const MapCell &c = game.world.map[size_t(x + y * kMapSize)];
            if (c.wall_n && c.wall_n < 0x95 && !skipped(c.wall_n))
                box(kMapX + x * kCellW, kMapY + kCellH * y, kCellW, kCellH / 2, colours_.white);
            if (c.wall_w && c.wall_w < 0x95 && !skipped(c.wall_w))
                box(kMapX + x * kCellW, kMapY + kCellH * y, kCellW / 2, kCellH, colours_.white);
        }
    }
    std::swap(surface_, background_);
    dirty_ = false;
}

void TownMap::draw_map(const Game &game, const TownLand &land)
{
    // display_hospital_map_sale and draw_sale_area_outlines.
    for (int x = 1; x < kMapSize; x++)
        for (int y = 1; y < kMapSize; y++)
            draw_map_cell(game, land, x, y);
}

void TownMap::draw_map_cell(const Game &game, const TownLand &land, int x, int y)
{
    const auto &map = game.world.map;
    const auto &plots = land.level.plots;
    auto at = [](int cx, int cy) { return size_t(cx + cy * kMapSize); };
    const MapCell &c = map[at(x, y)];
    int px = kMapX + x * kCellW, py = kMapY + y * kCellH;
    uint8_t colour = colours_.dark_blue;
    if (c.owner_flags & 2)
        colour = colours_.purple;
    if (c.owner_flags & 4)
        colour = colours_.orange;
    if (c.owner_flags & 8)
        colour = colours_.green;
    if (shown_on_map(c.floor_block))
        box(px, py, kCellW, kCellH, colour);
    if (plots[at(x, y)] && can_buy_plot(land, plots[at(x, y)]))
        box(px, py, kCellW, kCellH, colours_.red);
    if (plots[at(x, y)] != plots[at(x - 1, y)])
        box(px, py, kCellW / 2, kCellH, colours_.white);
    if (plots[at(x, y)] != plots[at(x, y - 1)])
        box(px, py, kCellW, kCellH / 2, colours_.white);
    if (x > 1 && y > 1 && plots[at(x, y)] != plots[at(x - 1, y - 1)])
        box(px, py, 1, 1, colours_.white);
    if (c.wall_n && solid_wall(c.wall_n))
        box(px, py, kCellW, kCellH / 2, colours_.white);
    if (c.wall_w && solid_wall(c.wall_w))
        box(px, py, kCellW / 2, kCellH, colours_.white);
    uint8_t west_n = map[at(x - 1, y)].wall_n, north_w = map[at(x, y - 1)].wall_w;
    if (west_n && north_w && solid_wall(west_n) && solid_wall(north_w))
        box(px, py, 1, 1, colours_.white);
}

void TownMap::draw_things(const Game &game)
{
    // draw_things_on_map: the first thing on each cell.
    const World &world = game.world;
    const int heating = game.finance.heating_level;
    for (int x = 0; x < kMapSize; x++) {
        for (int y = 0; y < kMapSize; y++) {
            int16_t first = world.map[size_t(x + y * kMapSize)].first;
            if (first <= 0 || size_t(first) >= world.things.size())
                continue;
            const Thing &t = world.things[size_t(first)];
            int px = kMapX + x * kCellW, py = kMapY + y * kCellH;
            switch (t.type) {
            case kThingObject:
                draw_object_on_map(t, px, py, heating);
                break;
            case kThingStaff:
                if (value_ & 1)
                    box(px, py, 2, 2, colours_.blue);
                break;
            case kThingPatient:
                if ((value_ & 1) && !(world.map[size_t(t.x.tile + t.y.tile * kMapSize)].owner_flags & kCellOutdoor))
                    box(px, py, 2, 2, colours_.white);
                break;
            default:
                break;
            }
        }
    }
    draw_map_counts(game);
}

void TownMap::draw_object_on_map(const Thing &t, int px, int py, int heating)
{
    switch (t.subtype) {
    case kObjPlant:
        if (value_ & 2)
            box(px, py, 3, 3, colours_.pale_green);
        break;
    case kObjFireExtinguisher:
        if (value_ & 4)
            box(px, py, 3, 3, colours_.red);
        break;
    case kObjRadiator:
        if (!(value_ & 0x10))
            break;
        box(px, py, 1, 1, colours_.light_red);
        if (t.owner == 0 && heat_icon_time_ && heating) {
            Sprite icon = sprites_.at(size_t(heating + 9));
            draw_sprite(surface_, px - icon.width / 2, py - icon.height / 2, icon, kDrawGhost, ghost_.data());
        }
        break;
    case kObjEntranceLeftDoor:
    case kObjEntranceRightDoor:
        if (t.facing == 0 || t.facing == 2)
            box(px, py, 7, 3, colours_.grey);
        else
            box(px, py, 3, 7, colours_.grey);
        break;
    case kObjDoor:
    case kObjSwingDoor:
    case kObjSwingDoor2:
        box(px, py, 3, 3, colours_.grey);
        break;
    default:
        if (t.subtype < kObjectKinds && (kObjects[t.subtype].category == 1 || kObjects[t.subtype].category == 2) &&
            (value_ & 8))
            box(px, py, 3, 3, colours_.cyan);
        break;
    }
}

void TownMap::draw_map_counts(const Game &game)
{
    const World &world = game.world;
    const int heating = game.finance.heating_level;
    // The counts: adcounts.patients (the player's patients indoors, from 1,
    // as the statistics pass counts them) and the hospital's plants, fire
    // extinguishers, machines and radiators, counted here from the map.
    int patients = 1, plants = 0, extinguishers = 0, machines = 0;
    for (const Thing &t : world.things) {
        if (t.type == kThingPatient && t.owner == 0 &&
            !(world.map[size_t(t.x.tile + t.y.tile * kMapSize)].owner_flags & kCellOutdoor))
            patients++;
        if (t.type != kThingObject || t.owner != 0 || t.subtype >= kObjectKinds)
            continue;
        if (t.subtype == kObjPlant)
            plants++;
        else if (t.subtype == kObjFireExtinguisher)
            extinguishers++;
        else if (kObjects[t.subtype].category == 1 || kObjects[t.subtype].category == 2)
            machines++;
    }
    int radiators = count_radiators(world, 0);
    const int figures[6] = {patients, plants,    extinguishers,
                            machines, radiators, radiators * heating * game.finance.heating_price / 100};
    char buf[32];
    for (size_t i = 0; i < kCounts.size(); i++) {
        std::snprintf(buf, sizeof buf, "%d", figures[i]);
        text(count_font_, kCounts[i].x, kCounts[i].y, buf);
    }
    for (int i = 0; i < heating * 6; i++)
        draw_sprite(surface_, kHeatingBar.x + i, kHeatingBar.y, sprites_.at(9));
}

void TownMap::draw_plot_under_mouse(const Game &game, const TownLand &land, const UiMouse &mouse,
                                    std::string_view hospital_name)
{
    const Font &font = detail_font_;
    int h = font.glyph('A').height;
    const Rect name = {kTownName.x, kTownName.y - h / 2, kTownName.w, kTownName.h};
    draw_text(surface_, font, 0, 0, text_->at(TextSection::TownNames, land.level_number),
              {{name.x, name.y, name.w}, name, kTextCentre});
    for (size_t i = 0; i < kDetailLabels.size(); i++)
        text(font, kDetailLabels[i].x, kDetailLabels[i].y, text_->at(TextSection::TownDetails, 4 + int(i)));

    std::string values[4] = {": -", ": -", ": -", ": -"};
    // The game divides towards zero, so the pixels just left of and above
    // the map count as its first column and row.
    int x = (mouse.x - kMapX) / kCellW;
    int y = (mouse.y - kMapY) / kCellH;
    if (x >= 0 && y >= 0 && x < kMapSize && y < kMapSize) {
        int plot = land.level.plots[size_t(x + y * kMapSize)];
        if (plot) {
            int amount = land_amount_[size_t(plot)];
            values[0] = ": " + std::to_string(plot);
            values[1] = ": " + std::string(text_->at(TextSection::TownDetails, 2));
            values[2] = ": " + std::to_string(amount);
            values[3] = ": $" + std::to_string(amount * game.settings().gbv.land_cost_per_tile);
        } else {
            uint8_t owners = game.world.map[size_t(x + y * kMapSize)].owner_flags;
            values[1] = ": " + std::string(text_->at(TextSection::TownDetails, 3));
            // allhosps[i].name: only the player's hospital is named here.
            if (owners & 1)
                values[1] = ": " + std::string(hospital_name);
        }
    }
    for (size_t i = 0; i < kDetailValues.size(); i++)
        text(font, kDetailValues[i].x, kDetailValues[i].y, values[i]);
}

void TownMap::draw_money(PanelState &panel, int balance)
{
    // draw_money_on_town, which shares the panel's roll.
    const Font &font = money_font_;
    int h = font.glyph('1').height;
    char now[16], shown[16];
    std::snprintf(now, sizeof now, "%7d", balance);
    const TextWindow screen = {0, 0, kUiWidth};
    if (panel.shown_balance == balance) {
        draw_text(surface_, font, kBalance.x, kBalance.y, now,
                  {screen, {kBalanceClip.x, kBalanceClip.y, kBalanceClip.w, h}});
        return;
    }
    std::snprintf(shown, sizeof shown, "%7d", panel.shown_balance);
    uint32_t diff = uint32_t(std::abs(panel.shown_balance - balance));
    int digits = diff >= 1000000  ? 0
                 : diff >= 100000 ? 1
                 : diff >= 10000  ? 2
                 : diff >= 1000   ? 3
                 : diff >= 100    ? 4
                 : diff >= 10     ? 5
                                  : 6;
    const Rect roll = {kBalanceClip.x + kDigitWidth * digits, kBalanceClip.y, kBalanceClip.w - kDigitWidth * digits, h};
    int step = panel.balance_roll_step;
    draw_text(surface_, font, kBalance.x, kBalance.y - (h - step * 6), now, {screen, roll});
    draw_text(surface_, font, kBalance.x, kBalance.y + step * 6, shown, {screen, roll});
    if (digits)
        draw_text(
            surface_, font, kBalance.x, kBalance.y, now,
            {screen, {kBalanceFixedClip.x, kBalanceFixedClip.y, kDigitWidth * (digits - 3) + kBalanceFixedClip.w, h}});
    if (++panel.balance_roll_step == 4) {
        panel.balance_roll_step = 0;
        panel.shown_balance = balance;
    }
}

bool TownMap::process_panel(const UiMouse &mouse, const Game &game, TownMapResult &result)
{
    // process_town_panel and process_town_button.
    bool hit = false;
    if (mouse.left_released || mouse.right_released) {
        for (size_t i = 0; i < kButtons.size(); i++) {
            const TownButton &b = kButtons[i];
            if (!over(mouse, b.x, b.y, b.w, b.h))
                continue;
            hit = true;
            process_town_button(i, mouse.right_released, game.finance.heating_level, result);
            if (b.sprite)
                draw_sprite(surface_, b.sprite_x, b.sprite_y, sprites_.at(size_t(b.sprite)));
            break;
        }
    }
    if (mouse.left_held || mouse.right_held) {
        for (const TownButton &b : kButtons) {
            if (!over(mouse, b.x, b.y, b.w, b.h))
                continue;
            hit = true;
            if (b.sprite)
                draw_sprite(surface_, b.sprite_x, b.sprite_y, sprites_.at(size_t(b.sprite)));
            break;
        }
    }
    return hit;
}

void TownMap::process_town_button(size_t i, bool right, int heating, TownMapResult &result)
{
    if (i <= 4) {
        if (right)
            value_ = 0;
        value_ ^= 1u << i;
        if (i == 4 && (value_ & 0x10))
            heat_icon_time_ = kHeatIconFrames;
        result.sounds.push_back(kSfxSelectX);
    } else if (i == 5 || i == 6) {
        int to = i == 6 ? heating + 1 : heating - 1;
        if (i == 6 ? heating < 10 : heating > 1) {
            result.event = {EV_SET_HEATING, uint16_t(to), 0, 0};
            result.sounds.push_back(kSfxSelectX);
        }
        heat_icon_time_ = kHeatIconFrames;
    } else {
        state_ = State::Closing;
        result.sounds.push_back(kSfxNo4);
    }
}

void TownMap::open_map(const TownLand &land)
{
    if (!value_)
        value_ = 0x1f;
    for (int plot = 0; plot < kPlotCount; plot++)
        land_amount_[size_t(plot)] = uint16_t(land_amount(land, plot));
    // load_ingame_pic puts the picture back in bscreen.
    background_ = picture_;
    dirty_ = true;
    state_ = State::Shown;
    heat_icon_time_ = kHeatIconFrames;
}

void TownMap::buy_plot(const UiMouse &m, const Game &game, const TownLand &land, TownMapResult &result)
{
    int plot = 0;
    if (over(m, kMapX, kMapY, kCellW * kMapSize, kCellH * kMapSize)) {
        int x = (m.x - kMapX) / kCellW, y = (m.y - kMapY) / kCellH;
        if (int p = land.level.plots[size_t(x + y * kMapSize)])
            plot = can_buy_plot(land, p);
    }
    if (plot && game.balance > land_amount_[size_t(plot)] * game.settings().gbv.land_cost_per_tile) {
        result.sounds.push_back(kSfxCashRegister);
        result.event = {EV_BUY_LAND, uint16_t(plot), 0, 0};
    }
    if (plot)
        dirty_ = true;
}

void TownMap::display_overhead_map(const Game &game, const TownLand &land, const UiMouse &mouse,
                                   std::string_view hospital_name, PanelState &panel)
{
    // display_overhead_map and draw_money_on_town.
    if (dirty_)
        draw_overhead_walls(game);
    surface_ = background_;
    draw_map(game, land);
    draw_things(game);
    for (size_t i = 0; i < kOverlayOff.size(); i++) {
        if (!(value_ & (1u << i)))
            draw_sprite(surface_, kOverlayOff[i].x, kOverlayOff[i].y, sprites_.at(i + 1));
    }
    draw_plot_under_mouse(game, land, mouse, hospital_name);
    draw_money(panel, game.balance);
}

TownMapResult TownMap::frame(const TownMapInput &input, const Game &game, const TownLand &land, PanelState &panel,
                             std::string_view hospital_name, const std::function<bool(int column, int row)> &go_to)
{
    TownMapResult result;
    UiMouse m = input.mouse;
    // draw_town_select.
    switch (state_) {
    case State::Closed:
        return result;
    case State::Closing:
        state_ = State::Closed;
        result.closed = true;
        return result;
    case State::Opening:
        m.left_pressed = m.left_released = false;
        open_map(land);
        break;
    case State::Shown:
        break;
    }
    display_overhead_map(game, land, m, hospital_name, panel);
    if (help_ && help_ctx_ && state_ == State::Shown) {
        // draw_town_select's help (src/town.cpp): group 0x13 over the map.
        help_->compute_help_text(*help_ctx_, 0, 0, 0x13);
        HelpBox &box = help_->box();
        box.screen_pic_loaded = true;
        help_->draw(surface_, palette_);
        box.screen_pic_loaded = false;
    }
    const Rect &bank = kBankHotspot;
    if (m.left_released && over(m, bank.x, bank.y, bank.w, bank.h)) {
        // process_keys(0, KC_F7), then the map is set up again.
        if (!game.paused)
            result.event = {EV_OPEN_SCREEN, 6, 0, 0};
        state_ = State::Opening;
    }
    if (heat_icon_time_)
        heat_icon_time_--;

    if (state_ != State::Shown)
        return result;
    // process_keys_town_select.
    if (input.escape) {
        state_ = State::Closing;
        return result;
    }
    // process_mouse_town_select.
    if (process_panel(m, game, result))
        return result;
    if (m.left_pressed)
        buy_plot(m, game, land, result);
    if (m.right_pressed && go_to && go_to((m.x - kMapX) / kCellW, (m.y - kMapY) / kCellH)) {
        state_ = State::Closing;
        result.sounds.push_back(kSfxYesX);
    }
    return result;
}

} // namespace th
