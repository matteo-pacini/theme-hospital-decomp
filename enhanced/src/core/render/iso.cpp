#include "core/render/iso.h"

#include "core/data/blocks.h"
#include "core/render/blit.h"

#include <algorithm>

namespace th
{

namespace
{

long floor_div(long a, long b)
{
    return a >= 0 ? a / b : -((-a + b - 1) / b);
}

Sprite block(const SpriteSheet &blocks, uint8_t id)
{
    int index = block_sprite(id);
    return index < 0 ? Sprite{} : blocks.at(size_t(index));
}

// Tallest block sprite: how far below the view a wall's base can be and
// still reach into it.
int tallest_block(const SpriteSheet &blocks)
{
    int tallest = 0;
    for (size_t i = 0; i < blocks.size(); i++)
        tallest = std::max(tallest, blocks.at(i).height);
    return tallest;
}

} // namespace

Point world_to_cell(Point p)
{
    // As find_cell_at_point, rounding towards minus infinity off the map.
    long u = long(p.x) * kTileHeight;
    long v = long(p.y) * kTileWidth;
    long area = long(kTileWidth) * kTileHeight;
    return {int(floor_div(u + v, area)), int(floor_div(v - u, area))};
}

void draw_map(Surface &dst, Rect view, Point camera, const MapCell *map, const SpriteSheet &blocks)
{
    dst.set_clip(view);
    const int tallest = tallest_block(blocks);

    // A cell's sprites stand on the bottom of its diamond, (col + row + 2)
    // half tiles down, and start half a tile left of its top corner.
    const int first_line = int(floor_div(2L * camera.y, kTileHeight)) - 2;
    const int last_line = int(floor_div(2L * (camera.y + view.h + tallest), kTileHeight));

    auto for_each_cell = [&](auto &&draw) {
        for (int line = first_line; line <= last_line; line++) {
            // Cells on this line have col + row = line and their left edge
            // (2 col - line - 1) half tiles right of the map origin; keep the
            // columns whose sprites can overlap the view.
            int col0 = int(floor_div(floor_div(2L * (camera.x - kTileWidth), kTileWidth) + line + 1, 2));
            int col1 = int(floor_div(floor_div(2L * (camera.x + view.w), kTileWidth) + line + 2, 2)) + 1;
            int base_y = (line + 2) * kTileHeight / 2 - camera.y + view.y;
            for (int col = col0; col <= col1; col++) {
                int row = line - col;
                int x = (col - row - 1) * kTileWidth / 2 - camera.x + view.x;
                draw(col, row, x, base_y);
            }
        }
    };

    for_each_cell([&](int col, int row, int x, int base_y) {
        bool on_map = col >= 0 && col < kMapSize && row >= 0 && row < kMapSize;
        Sprite s = block(blocks, on_map ? map[col + row * kMapSize].floor_block : kOffMapBlock);
        draw_sprite(dst, x, base_y - s.height, s);
    });
    for_each_cell([&](int col, int row, int x, int base_y) {
        if (col < 0 || col >= kMapSize || row < 0 || row >= kMapSize)
            return;
        const MapCell &cell = map[col + row * kMapSize];
        if (cell.wall_n) {
            Sprite s = block(blocks, cell.wall_n);
            draw_sprite(dst, x, base_y - s.height, s);
        }
        if (cell.wall_w) {
            Sprite s = block(blocks, cell.wall_w);
            draw_sprite(dst, x, base_y - s.height, s);
        }
    });
    dst.reset_clip();
}

} // namespace th
