#include "core/render/view.h"

#include "core/audio/sounds.h"
#include "core/data/blocks.h"
#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/frame.h"
#include "core/render/shadows.h"
#include "core/world/world.h"

#include <algorithm>
#include <array>
#include <climits>
#include <utility>
#include <vector>

namespace th
{

namespace
{

constexpr int kBuckets = 32;

long floor_div(long a, long b)
{
    return a >= 0 ? a / b : -((-a + b - 1) / b);
}

Sprite block(const SpriteSheet &blocks, uint8_t id)
{
    int index = block_sprite(id);
    return index < 0 ? Sprite{} : blocks.at(size_t(index));
}

bool on_map(int col, int row)
{
    return col >= 0 && col < kMapSize && row >= 0 && row < kMapSize;
}

// The cells whose sprites can reach part of the view. lines() calls
// line(col0, col1, line) for each diagonal line of cells (col + row = line)
// from the back, with the columns that can reach `area`; `reach` is how far
// above its base a cell's sprites can extend. x() is the left of a cell's
// diamond and base_y() the bottom of a line's, in `dst` coordinates.
struct CellScan {
    Rect view;
    Point camera;

    template <typename Line> void lines(Rect area, int reach, Line &&line) const
    {
        long wx0 = camera.x + area.x - view.x;
        long wy0 = camera.y + area.y - view.y;
        int first = int(floor_div(2 * wy0, kTileHeight)) - 2;
        int last = int(floor_div(2 * (wy0 + area.h + reach), kTileHeight));
        for (int l = first; l <= last; l++) {
            // Cells on line l have their left edge (2 col - l - 1) half tiles
            // right of the map origin.
            int col0 = int(floor_div(floor_div(2 * (wx0 - kTileWidth), kTileWidth) + l + 1, 2));
            int col1 = int(floor_div(floor_div(2 * (wx0 + area.w), kTileWidth) + l + 2, 2)) + 1;
            line(col0, col1, l);
        }
    }

    int x(int col, int row) const { return (col - row - 1) * kTileWidth / 2 - camera.x + view.x; }
    int base_y(int line) const { return (line + 2) * kTileHeight / 2 - camera.y + view.y; }
};

int tallest_block(const SpriteSheet &blocks)
{
    int tallest = 0;
    for (size_t i = 0; i < blocks.size(); i++)
        tallest = std::max(tallest, blocks.at(i).height);
    return tallest;
}

void grow(Rect &bounds, const Rect &r)
{
    if (r.w <= 0 || r.h <= 0)
        return;
    if (bounds.w <= 0 || bounds.h <= 0) {
        bounds = r;
        return;
    }
    int x1 = std::max(bounds.x + bounds.w, r.x + r.w), y1 = std::max(bounds.y + bounds.h, r.y + r.h);
    bounds.x = std::min(bounds.x, r.x);
    bounds.y = std::min(bounds.y, r.y);
    bounds.w = x1 - bounds.x;
    bounds.h = y1 - bounds.y;
}

Rect intersect(const Rect &a, const Rect &b)
{
    int x0 = std::max(a.x, b.x), y0 = std::max(a.y, b.y);
    int x1 = std::min(a.x + a.w, b.x + b.w), y1 = std::min(a.y + a.h, b.y + b.h);
    return {x0, y0, std::max(x1 - x0, 0), std::max(y1 - y0, 0)};
}

// The game's litter_starts (src/anitable.cpp), by litter kind (MapCell's
// litter nibble): the animation drawn, and whether it is drawn at the
// cell's middle rather than at one of kLitterPos. Kinds 9-15 are zero there.
struct LitterStart {
    uint16_t anim;
    bool centred;
};
constexpr std::array<LitterStart, 16> kLitterStarts = {{
    {9999, false},
    {947, false},
    {948, false},
    {949, false},
    {1121, true},
    {951, true},
    {1124, true},
    {952, true},
    {953, true},
}};

// The game's litter_pos (src/tables.cpp), its table for 64 x 32 cells:
// where uncentred litter is drawn, from the cell's middle, chosen by a hash
// of the cell's index.
constexpr std::array<Point, 8> kLitterPos = {{
    {0, 0},
    {10, 4},
    {-21, -2},
    {9, -8},
    {-10, 5},
    {-16, -3},
    {0, 9},
    {16, 3},
}};

// The first frame and mirroring of animation `anim` (AnimData::starts).
AnimStart anim_start(const GameData &data, uint32_t anim)
{
    const auto &starts = data.anims().starts;
    return anim < starts.size() ? starts[anim] : AnimStart{};
}

// do_mapwho for `cell` (map index `index`), whose diamond has its left edge
// at x and its top corner at y: its overlay (flags bits 6-7), the blueprint
// marker ending its list, its litter, then its things by ascending bucket
// and, within a bucket, from the end of the cell's list. Returns the box of the
// frames drawn (the game's frame_bounds); `extent`, if given, grows to
// every pixel drawn.
Rect draw_cell_things(Surface &dst, const MapCell &cell, int index, int x, int y, const World &world,
                      const GameData &data, Rect *extent, std::vector<FrameSound> *sounds)
{
    std::vector<std::pair<int, int16_t>> drawn; // (bucket, thing) in list order
    int16_t marker = cell.first;
    for (; marker > 0 && marker < kThingCount; marker = world.things[size_t(marker)].cell_next)
        // The game indexes 32 buckets without a check; keep to them.
        drawn.emplace_back(std::clamp(thing_bucket(world.things[size_t(marker)]), 0, kBuckets - 1), marker);
    Rect bounds{};
    auto draw_start = [&](int fx, int fy, AnimStart start) {
        Rect sprites;
        grow(bounds, draw_frame(dst, fx, fy, start.frame, start.flip != 0, 0, data, nullptr, &sprites));
        if (extent)
            grow(*extent, sprites);
    };
    if (cell.flags & 0xc0) {
        // Behind a wall the floor is drawn again under the overlay, as the
        // game does into the buffer it draws such cells in.
        if (cell.flags & 0x10) {
            Sprite f = block(data.blocks(), cell.floor_block);
            draw_sprite(dst, x, y + kTileHeight - f.height, f);
            if (extent)
                grow(*extent, {x, y + kTileHeight - f.height, f.width, f.height});
        }
        draw_start(x + kTileWidth / 2, y + kTileHeight / 2, {anim_start(data, 625u + (cell.flags >> 6)).frame, 0});
    }
    if (marker < 0 && size_t(-marker) < kBlueprints.size())
        draw_start(x + kTileWidth / 2, y + kTileHeight / 2, anim_start(data, kBlueprints[size_t(-marker)].anim));
    if (uint8_t kind = cell.litter_shadow & 0xf) {
        const LitterStart &litter = kLitterStarts[kind];
        if (litter.centred) {
            draw_start(x + kTileWidth / 2, y + kTileHeight / 2, anim_start(data, litter.anim));
        } else {
            Point at = kLitterPos[(uint32_t(uint16_t(index)) * 0x2e4a4321u >> 16) & 7];
            draw_start(x + kTileWidth / 2 + at.x, y + kTileHeight / 2 + at.y, anim_start(data, litter.anim));
        }
    }
    for (int b = 0; b < kBuckets; b++) {
        for (auto it = drawn.rbegin(); it != drawn.rend(); ++it) {
            if (it->first != b)
                continue;
            const Thing &t = world.things[size_t(it->second)];
            // mapwho_object (objects and doors) and mapwho_person's usual
            // case (draw_special_id_frame); people's glow, wobble and status
            // icons are not drawn yet.
            if (t.type != kThingObject && t.type != kThingDoor && t.type != kThingPatient && t.type != kThingStaff)
                continue;
            int tx = x + kTileWidth / 2 + ((int(t.x.frac) - int(t.y.frac)) >> 3);
            int ty = y + ((int(t.x.frac) + int(t.y.frac)) >> 4);
            Rect sprites;
            grow(bounds, draw_frame(dst, tx, ty, t.sprite, t.flip != 0, 0, data, t.layers.data(), &sprites));
            if (sounds && t.sprite < data.anims().frames.size())
                sounds->push_back({uint16_t(it->second), data.anims().frames[t.sprite].sound,
                                   t.type == kThingPatient || t.type == kThingStaff, tx, ty});
            if (extent)
                grow(*extent, sprites);
        }
    }
    return bounds;
}

} // namespace

void draw_world(Surface &dst, Rect view, Point camera, const World &world, const GameData &data,
                const ShadowSprites *shadows, Surface &scratch, std::vector<FrameSound> *sounds)
{
    const SpriteSheet &blocks = data.blocks();
    const int tallest = tallest_block(blocks);
    const MapCell *map = world.map.data();
    CellScan scan{view, camera};
    dst.set_clip(view);

    scan.lines(view, tallest, [&](int col0, int col1, int line) {
        int base_y = scan.base_y(line);
        for (int col = col0; col <= col1; col++) {
            int row = line - col, x = scan.x(col, row);
            if (on_map(col, row) && shadows) {
                ShadedSprite f = shadows->floor(map[col + row * kMapSize]);
                draw_sprite(dst, x, base_y - f.lift, f.sprite);
            } else {
                Sprite s = block(blocks, on_map(col, row) ? map[col + row * kMapSize].floor_block : kOffMapBlock);
                draw_sprite(dst, x, base_y - s.height, s);
            }
        }
    });

    auto draw_walls = [&](Surface &s, const MapCell &cell, int x, int base_y) {
        if (cell.wall_n && shadows) {
            ShadedSprite w = shadows->wall_n(cell);
            draw_sprite(s, x, base_y - w.lift, w.sprite);
        } else if (cell.wall_n) {
            Sprite w = block(blocks, cell.wall_n);
            draw_sprite(s, x, base_y - w.height, w);
        }
        if (cell.wall_w) {
            Sprite w = block(blocks, cell.wall_w);
            draw_sprite(s, x, base_y - w.height, w);
        }
    };

    scan.lines(view, tallest, [&](int col0, int col1, int line) {
        int base_y = scan.base_y(line);
        for (int col = col0; col <= col1; col++) {
            int row = line - col;
            if (on_map(col, row))
                draw_walls(dst, map[col + row * kMapSize], scan.x(col, row), base_y);
        }
        for (int col = col0; col <= col1; col++) {
            int row = line - col;
            if (!on_map(col, row))
                continue;
            const MapCell &cell = map[col + row * kMapSize];
            if (cell.first == 0 && !(cell.litter_shadow & 0xf) && !(cell.flags & 0xc0))
                continue;
            int x = scan.x(col, row), top = base_y - kTileHeight;
            if (!(cell.flags & 0x10)) {
                draw_cell_things(dst, cell, col + row * kMapSize, x, top, world, data, nullptr, sounds);
                continue;
            }
            // Behind a wall: draw into the scratch surface, blank out the
            // walls of every cell further east or south, then copy what is
            // left (draw_window_into_pix_buffer).
            scratch.set_clip(view);
            Rect touched{};
            Rect bounds = intersect(
                draw_cell_things(scratch, cell, col + row * kMapSize, x, top, world, data, &touched, sounds), view);
            touched = intersect(touched, view);
            scratch.set_clip(bounds);
            scan.lines(bounds, tallest, [&](int mc0, int mc1, int mline) {
                int mbase = scan.base_y(mline);
                for (int mc = mc0; mc <= mc1; mc++) {
                    int mr = mline - mc;
                    if (!on_map(mc, mr) || !(mc > col || mr > row))
                        continue;
                    const MapCell &m = map[mc + mr * kMapSize];
                    for (uint8_t wall : {m.wall_n, m.wall_w}) {
                        if (!wall)
                            continue;
                        Sprite w = block(blocks, wall);
                        draw_sprite_one_colour(scratch, scan.x(mc, mr), mbase - w.height, w, kMaskColour);
                    }
                }
            });
            // Only the frames' box is copied, as the game does; sprites reaching
            // outside it are cut off.
            for (int py = bounds.y; py < bounds.y + bounds.h; py++) {
                const uint8_t *src = scratch.row(py);
                uint8_t *out = dst.row(py);
                for (int px = bounds.x; px < bounds.x + bounds.w; px++)
                    if (src[px] != kMaskColour)
                        out[px] = src[px];
            }
            grow(touched, bounds);
            for (int py = touched.y; py < touched.y + touched.h; py++)
                std::fill(scratch.row(py) + touched.x, scratch.row(py) + touched.x + touched.w, kMaskColour);
        }
    });
    scratch.reset_clip();
    dst.reset_clip();
}

} // namespace th
