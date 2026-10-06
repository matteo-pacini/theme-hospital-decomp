#include "core/render/iso.h"

#include "core/data/level.h"

namespace th
{

namespace
{

long floor_div(long a, long b)
{
    return a >= 0 ? a / b : -((-a + b - 1) / b);
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

bool beyond_scroll_limit(Point p, uint16_t min_cell, uint16_t max_cell, int margin)
{
    Point c = world_to_cell(p);
    return c.x + margin < min_cell % kMapSize || c.x - margin >= max_cell % kMapSize ||
           c.y + margin < min_cell / kMapSize || c.y - margin >= max_cell / kMapSize;
}

} // namespace th
