#include "core/data/level.h"

namespace th
{

namespace
{

constexpr size_t kTilesOffset = 0x22;
constexpr size_t kPlotsOffset = 0x20022;
constexpr size_t kCameraOffset = 0x28024;
constexpr size_t kEmergencyOffset = 0x2802c;
constexpr size_t kScrollMinOffset = 0x28034;
constexpr size_t kScrollMaxOffset = 0x28036;
constexpr size_t kLevelSize = 0x28038;

// Block ids of the partition walls between plots (src/town.cpp).
constexpr uint8_t kPartitionWallN = 0x8c;
constexpr uint8_t kPartitionWallW = 0x8d;
// First of the eight decoration blocks scattered over unbought land.
constexpr uint8_t kFirstDecoration = 0xb6;

uint16_t le16(std::span<const uint8_t> d, size_t at)
{
    return uint16_t(d[at] | d[at + 1] << 8);
}

} // namespace

bool parse_level(std::span<const uint8_t> data, Level &out)
{
    if (data.size() < kLevelSize)
        return false;
    out.player_count = le16(data, 0);
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            out.entrances[i][j] = le16(data, 2 + (i * 4 + j) * 2);
    for (int i = 0; i < kMapCells; i++) {
        size_t at = kTilesOffset + size_t(i) * 8;
        MapCell &c = out.tiles[i];
        c.first = static_cast<int16_t>(le16(data, at));
        c.floor_block = data[at + 2];
        c.wall_n = data[at + 3];
        c.wall_w = data[at + 4];
        c.flags = data[at + 5];
        c.litter_shadow = data[at + 6];
        c.owner_flags = data[at + 7];
        out.plots[i] = le16(data, kPlotsOffset + size_t(i) * 2);
    }
    for (int i = 0; i < 4; i++) {
        out.camera_start[i] = le16(data, kCameraOffset + i * 2);
        out.emergency_tile[i] = le16(data, kEmergencyOffset + i * 2);
    }
    out.scroll_min = le16(data, kScrollMinOffset);
    out.scroll_max = le16(data, kScrollMaxOffset);
    return true;
}

void build_start_map(std::array<MapCell, kMapCells> &map, const Level &level,
                     const std::function<void(int cell)> &copied)
{
    map = {};
    std::array<uint16_t, kMapCells> plots = level.plots;

    for (int i = 0; i < kMapCells; i++) {
        const MapCell &src = level.tiles[i];
        uint16_t plot = plots[i];
        if (plot == 0 || (plot <= 4 && level.player_count >= plot)) {
            MapCell &c = map[i];
            c.wall_n = src.wall_n;
            c.wall_w = src.wall_w;
            c.floor_block = src.floor_block;
            c.flags = src.flags & 0x3f;
            c.litter_shadow = src.litter_shadow & 0xf0;
            if (plot == 0)
                c.owner_flags = src.owner_flags & ~0x0f;
            else
                c.owner_flags = src.owner_flags | uint8_t(1 << (plot - 1));
            // The copy replaces whatever objects placed on earlier cells linked
            // or marked here, as in the game.
            c.first = 0;
            plots[i] = 0;
            if (copied)
                copied(i);
        } else {
            // Unbought land: background[0]'s defaults (src/tables.cpp);
            // litter_shadow is left as it was.
            MapCell &c = map[i];
            c.first = 0;
            c.floor_block = 0;
            c.wall_n = 0;
            c.wall_w = 0;
            c.flags = 1;
            c.owner_flags = kCellOutdoor;
        }
    }

    // mow_grass: alternate the two grass blocks along each row.
    for (int y = 0; y < kMapSize; y++) {
        uint8_t grass = 2;
        for (int x = 0; x < kMapSize; x++) {
            grass = grass == 0 ? 2 : 0;
            MapCell &c = map[x + y * kMapSize];
            if (c.floor_block == 0 || c.floor_block == 2)
                c.floor_block = grass;
        }
    }

    // The game's fixed-seed decoration scatter over land still for sale.
    uint32_t rnd = 0xbb40e62d;
    for (int i = 0; i < kMapCells; i++) {
        rnd *= 0xbb40e62du;
        uint32_t r = (rnd >> 8) & 0x7fff;
        if (plots[i] && map[i].floor_block == 0 && r % 50 == 0)
            map[i].wall_n = uint8_t((r & 7) + kFirstDecoration);
    }

    // place_partition_walls. The game indexes x + 1 and y + 1 without
    // wrapping, so x = 127 looks at the next row's first cell; indexes past
    // the map are skipped here.
    for (int x = 0; x < kMapSize; x++) {
        for (int y = 0; y < kMapSize; y++) {
            int i = x + y * kMapSize;
            if (map[i].owner_flags & kCellOutdoor)
                continue;
            int east = i + 1;
            if (east < kMapCells && (map[east].owner_flags & kCellOutdoor) &&
                level.tiles[east].wall_w == kPartitionWallW)
                map[east].wall_w = kPartitionWallW;
            int south = i + kMapSize;
            if (south < kMapCells && (map[south].owner_flags & kCellOutdoor) &&
                level.tiles[south].wall_n == kPartitionWallN)
                map[south].wall_n = kPartitionWallN;
        }
    }
}

std::array<MapCell, kMapCells> build_start_map(const Level &level)
{
    std::array<MapCell, kMapCells> map;
    build_start_map(map, level);
    return map;
}

} // namespace th
