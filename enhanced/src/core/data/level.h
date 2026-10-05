#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace th
{

constexpr int kMapSize = 128;
constexpr int kMapCells = kMapSize * kMapSize;

// Map.owner_flags bit for outdoor cells.
constexpr uint8_t kCellOutdoor = 0x10;

// One map cell, laid out as the game's Map (include/hosp.h). Cells are
// indexed column + row * 128.
struct MapCell {
    int16_t first = 0; // things on the cell; in a level file, an object to place
    uint8_t floor_block = 0;
    uint8_t wall_n = 0; // wall on the cell's north edge
    uint8_t wall_w = 0; // wall on the cell's west edge
    uint8_t flags = 0;
    uint8_t litter_shadow = 0;
    uint8_t owner_flags = 0;
};

// The contents of a levels/level.lN file (the game's Level).
struct Level {
    uint16_t player_count = 0;
    std::array<std::array<uint16_t, 4>, 4> entrances{};
    std::array<MapCell, kMapCells> tiles{};
    std::array<uint16_t, kMapCells> plots{}; // plot number per cell, 0 public land
    std::array<uint16_t, 4> camera_start{};  // per hospital, the cell the view starts on
    std::array<uint16_t, 4> emergency_tile{};
    uint16_t scroll_min = 0; // cells bounding the scroll area
    uint16_t scroll_max = 0;
};

// Parses an unpacked level file. Returns false if it is too short.
bool parse_level(std::span<const uint8_t> data, Level &out);

// The playable map built from a level for a single-player game, as the
// game's recreate_map does when a level starts (src/town.cpp): public land
// and plot 1 as in the file, other plots blank grass with random decorations
// and partition walls along unbought land. Objects in the level file and
// shadows are not handled yet.
std::array<MapCell, kMapCells> build_start_map(const Level &level);

} // namespace th
