#pragma once

#include "core/data/level.h"
#include "core/data/sprites.h"
#include "core/render/surface.h"

namespace th
{

// Size of a floor tile in the high-resolution data (resvars.tile_width and
// tile_height).
constexpr int kTileWidth = 64;
constexpr int kTileHeight = 32;

struct Point {
    int x = 0;
    int y = 0;
};

// Map pixel position of cell (col, row) before scrolling, as the game's
// give_absolute_xy_of_cell_at_column_row: the top corner of its diamond.
inline Point cell_to_world(int col, int row)
{
    return {(col - row) * kTileWidth / 2, (col + row) * kTileHeight / 2};
}

// The cell under map pixel position p (unclamped), as find_cell_at_point.
Point world_to_cell(Point p);

// Draws the map's floors, then its walls row by row from the back, into
// `view` of `dst`, with `camera` the map pixel position at the view's
// top-left. Cells outside the map are drawn as grass, as the game does.
void draw_map(Surface &dst, Rect view, Point camera, const MapCell *map, const SpriteSheet &blocks);

} // namespace th
