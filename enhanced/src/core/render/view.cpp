#include "core/render/view.h"

#include "core/audio/sounds.h"
#include "core/data/blocks.h"
#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/frame.h"
#include "core/render/shadows.h"
#include "core/world/complex.h"
#include "core/world/reaper.h"
#include "core/world/staff.h"
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

// OS_IN_USE (include/enums.h): an object someone is using.
constexpr uint8_t kObjectInUse = 4;

// Whether the game's mapwho_* tests `t` for being under the pointer: people
// always, objects and doors in the states mapwho_object draws whole.
bool pointed_testable(const Thing &t)
{
    if (t.type == kThingPatient || t.type == kThingStaff)
        return true;
    switch (t.state) {
    case 0x02: // OS_STATIC
    case 0x06: // OS_HELL
    case 0x0a: // OS_PLANT_AGES
    case 0x0e: // OS_BEING_WATERED
    case 0x10: // OS_DOUBLE_DOOR_SWINGS
    case 0x14: // OS_SLIDING_DOOR_CLOSED
    case 0x16: // OS_SLIDING_DOOR_OPENS
    case 0x18: // OS_SLIDING_DOOR_OPEN
    case 0x1a: // OS_SLIDING_DOOR_CLOSES
    case 0x1c: // OS_BOMB_TICKS
        return true;
    default:
        return false;
    }
}

// What draw_cell_things draws from and reports to.
struct CellDrawing {
    const World &world;
    const GameData &data;
    DrawnThings drawn;
};

// The frames draw_cell_things has drawn so far: their box (the game's
// frame_bounds) and, if `extent` is given, every pixel drawn.
struct CellFrames {
    Surface &dst;
    const GameData &data;
    Rect *extent;
    Rect bounds{};

    void draw_start(int fx, int fy, AnimStart start)
    {
        Rect sprites;
        grow(bounds, draw_frame(dst, {fx, fy}, start.frame, start.flip != 0, data, {.extent = &sprites}));
        if (extent)
            grow(*extent, sprites);
    }
};

// draw_cell_things' overlay (flags bits 6-7), blueprint marker and litter.
void draw_cell_overlays(CellFrames &frames, const MapCell &cell, int index, int16_t marker, Point at)
{
    const GameData &data = frames.data;
    const int x = at.x, y = at.y;
    if (cell.flags & 0xc0) {
        // Behind a wall the floor is drawn again under the overlay, as the
        // game does into the buffer it draws such cells in.
        if (cell.flags & 0x10) {
            Sprite f = block(data.blocks(), cell.floor_block);
            draw_sprite(frames.dst, x, y + kTileHeight - f.height, f);
            if (frames.extent)
                grow(*frames.extent, {x, y + kTileHeight - f.height, f.width, f.height});
        }
        frames.draw_start(x + kTileWidth / 2, y + kTileHeight / 2,
                          {anim_start(data, 625u + (cell.flags >> 6)).frame, 0});
    }
    if (marker < 0 && size_t(-marker) < kBlueprints.size())
        frames.draw_start(x + kTileWidth / 2, y + kTileHeight / 2, anim_start(data, kBlueprints[size_t(-marker)].anim));
    if (uint8_t kind = cell.litter_shadow & 0xf) {
        const LitterStart &litter = kLitterStarts[kind];
        if (litter.centred) {
            frames.draw_start(x + kTileWidth / 2, y + kTileHeight / 2, anim_start(data, litter.anim));
        } else {
            Point pos = kLitterPos[(uint32_t(uint16_t(index)) * 0x2e4a4321u >> 16) & 7];
            frames.draw_start(x + kTileWidth / 2 + pos.x, y + kTileHeight / 2 + pos.y, anim_start(data, litter.anim));
        }
    }
}

// Whether draw_cell_things draws thing `t`. mapwho_object (objects and
// doors), mapwho_person's usual case (draw_special_id_frame), mapwho_reaper
// and mapwho_bang (explosion debris) are drawn; people's glow, wobble and
// status icons, and a machine's user drawn split (draw_split_frame) or
// energised, are not drawn yet.
bool cell_thing_drawn(const Thing &t)
{
    if (t.type != kThingObject && t.type != kThingDoor && t.type != kThingPatient && t.type != kThingStaff &&
        t.type != kThingReaper && t.type != kThingDebris && t.type != kThingHelicopter && t.type != kThingShadow &&
        t.type != kThingMayor)
        return false;
    // mapwho_person draws no patient gone, merged into a member of staff's
    // animation or gone into the gates of hell; mapwho_reaper no reaper being
    // removed.
    if (t.type == kThingPatient && (t.state == kPsGone || t.state == kPsMerged || t.state == kPsAtHellGate))
        return false;
    if (t.type == kThingReaper && t.state == kReaperRemove)
        return false;
    // mapwho_staff draws no applicant, nor staff going back to the applicants
    // or in state 42.
    return !(t.type == kThingStaff &&
             (t.state == 42 || t.state == kStaffApplicant || t.state == kStaffBackToApplicants));
}

// draw_cell_things for one thing `id` of the cell whose diamond's top-left
// is `at`.
void draw_cell_thing(CellFrames &frames, const CellDrawing &drawing, int16_t id, Point at)
{
    const GameData &data = drawing.data;
    const Thing &t = drawing.world.things[size_t(id)];
    int tx = at.x + kTileWidth / 2 + ((int(t.x.frac) - int(t.y.frac)) >> 3);
    int ty = at.y + ((int(t.x.frac) + int(t.y.frac)) >> 4);
    // A dead patient floating away is drawn its height up (fly_height),
    // debris its altitude in 1/256 pixel, all its elements (mapwho_bang's
    // draw_a_frame).
    if (t.type == kThingPatient && t.state == kPsFlys)
        ty -= t.countdown;
    if (t.type == kThingDebris)
        ty -= t.altitude >> 8;
    // mapwho_helicopter: a helicopter its altitude up; mapwho_shadow: its
    // shadow, all its elements, smaller as the helicopter rises
    // (draw_a_frame_scaled).
    if (t.type == kThingHelicopter)
        ty -= t.altitude;
    int32_t scale = 0x10000;
    if (t.type == kThingShadow)
        scale = 0x640000 / (drawing.world.things[t.caster].altitude + 100);
    const uint8_t *layers = t.type == kThingDebris || t.type == kThingShadow ? nullptr : t.layers.data();
    Rect sprites;
    FrameOptions opts;
    opts.layers = layers;
    opts.extent = &sprites;
    opts.scale = scale;
    // A person's status icon hangs over their head, a machine's over its
    // frame (draw_special_id_frame).
    if (t.icon_id > 0 && (t.type == kThingPatient || t.type == kThingStaff || t.type == kThingObject)) {
        opts.icon = t.icon_frame;
        opts.icon_over_frame = t.type == kThingObject;
    }
    Rect box = draw_frame(frames.dst, {tx, ty}, t.sprite, t.flip != 0, data, opts);
    grow(frames.bounds, box);
    // mapwho_object's smoke of a worn machine, over it: the game draws it
    // with the layer choices of the bytes holding Thing::smoke, a puff on
    // each of layers 10-12 shown once its byte is 1.
    if (t.type == kThingObject && t.overlay_frame && (pointed_testable(t) || t.state == kObjectInUse)) {
        std::array<uint8_t, kLayerCount> smoke{};
        std::copy(t.smoke.begin(), t.smoke.end(), smoke.begin() + 10);
        grow(frames.bounds, draw_frame(frames.dst, {tx, ty}, t.overlay_frame, t.flip != 0, data,
                                       {.layers = smoke.data(), .extent = &sprites}));
    }
    PointedThing *pointed = drawing.drawn.pointed;
    if (pointed && pointed_testable(t) && box.x < pointed->at.x && box.x + box.w > pointed->at.x &&
        box.y < pointed->at.y && box.y + box.h > pointed->at.y)
        pointed->thing = uint16_t(id);
    if (std::vector<FrameSound> *sounds = drawing.drawn.sounds; sounds && t.sprite < data.anims().frames.size())
        sounds->push_back({uint16_t(id), data.anims().frames[t.sprite].sound,
                           t.type == kThingPatient || t.type == kThingStaff, tx, ty});
    if (frames.extent)
        grow(*frames.extent, sprites);
}

// do_mapwho for `cell` (map index `index`), whose diamond has its left edge
// at `at.x` and its top corner at `at.y`: its overlay (flags bits 6-7), the blueprint
// marker ending its list, its litter, then its things by ascending bucket
// and, within a bucket, from the end of the cell's list. Returns the box of the
// frames drawn (the game's frame_bounds); `extent`, if given, grows to
// every pixel drawn.
Rect draw_cell_things(Surface &dst, const CellDrawing &drawing, const MapCell &cell, int index, Point at, Rect *extent)
{
    const World &world = drawing.world;
    std::vector<std::pair<int, int16_t>> drawn; // (bucket, thing) in list order
    int16_t marker = cell.first;
    for (; marker > 0 && marker < kThingCount; marker = world.things[size_t(marker)].cell_next)
        // The game indexes 32 buckets without a check; keep to them.
        drawn.emplace_back(std::clamp(thing_bucket(world.things[size_t(marker)]), 0, kBuckets - 1), marker);
    CellFrames frames{dst, drawing.data, extent};
    draw_cell_overlays(frames, cell, index, marker, at);
    for (int b = 0; b < kBuckets; b++) {
        for (auto it = drawn.rbegin(); it != drawn.rend(); ++it) {
            if (it->first == b && cell_thing_drawn(world.things[size_t(it->second)]))
                draw_cell_thing(frames, drawing, it->second, at);
        }
    }
    return frames.bounds;
}

// draw_world's passes over the cells in view: floors, then line by line
// each cell's walls and things.
class WorldDrawing
{
  public:
    WorldDrawing(const ViewTarget &target, const World &world, const GameData &data, const ShadowSprites *shadows,
                 Surface &scratch, const DrawnThings &drawn)
        : dst_(target.dst), view_(target.view), blocks_(data.blocks()), tallest_(tallest_block(blocks_)),
          map_(world.map.data()), scan_{view_, target.camera}, drawing_{world, data, drawn}, shadows_(shadows),
          scratch_(scratch)
    {
    }

    void floors() const
    {
        scan_.lines(view_, tallest_, [&](int col0, int col1, int line) {
            int base_y = scan_.base_y(line);
            for (int col = col0; col <= col1; col++) {
                int row = line - col, x = scan_.x(col, row);
                if (on_map(col, row) && shadows_) {
                    ShadedSprite f = shadows_->floor(map_[col + row * kMapSize]);
                    draw_sprite(dst_, x, base_y - f.lift, f.sprite);
                } else {
                    Sprite s = block(blocks_, on_map(col, row) ? map_[col + row * kMapSize].floor_block : kOffMapBlock);
                    draw_sprite(dst_, x, base_y - s.height, s);
                }
            }
        });
    }

    void walls_and_things()
    {
        scan_.lines(view_, tallest_, [&](int col0, int col1, int line) {
            int base_y = scan_.base_y(line);
            for (int col = col0; col <= col1; col++) {
                int row = line - col;
                if (on_map(col, row))
                    draw_walls(map_[col + row * kMapSize], scan_.x(col, row), base_y);
            }
            for (int col = col0; col <= col1; col++)
                cell_things(col, line - col, base_y);
        });
    }

  private:
    void draw_walls(const MapCell &cell, int x, int base_y) const
    {
        if (cell.wall_n && shadows_) {
            ShadedSprite w = shadows_->wall_n(cell);
            draw_sprite(dst_, x, base_y - w.lift, w.sprite);
        } else if (cell.wall_n) {
            Sprite w = block(blocks_, cell.wall_n);
            draw_sprite(dst_, x, base_y - w.height, w);
        }
        if (cell.wall_w) {
            Sprite w = block(blocks_, cell.wall_w);
            draw_sprite(dst_, x, base_y - w.height, w);
        }
    }

    void cell_things(int col, int row, int base_y)
    {
        if (!on_map(col, row))
            return;
        const MapCell &cell = map_[col + row * kMapSize];
        if (cell.first == 0 && !(cell.litter_shadow & 0xf) && !(cell.flags & 0xc0))
            return;
        int x = scan_.x(col, row), top = base_y - kTileHeight;
        if (!(cell.flags & 0x10)) {
            draw_cell_things(dst_, drawing_, cell, col + row * kMapSize, {x, top}, nullptr);
            return;
        }
        // Behind a wall: draw into the scratch surface, blank out the walls of
        // every cell further east or south, then copy what is left
        // (draw_window_into_pix_buffer).
        scratch_.set_clip(view_);
        Rect touched{};
        Rect bounds =
            intersect(draw_cell_things(scratch_, drawing_, cell, col + row * kMapSize, {x, top}, &touched), view_);
        touched = intersect(touched, view_);
        scratch_.set_clip(bounds);
        mask_walls_in_front(bounds, col, row);
        // Only the frames' box is copied, as the game does; sprites reaching
        // outside it are cut off.
        for (int py = bounds.y; py < bounds.y + bounds.h; py++) {
            const uint8_t *src = scratch_.row(py);
            uint8_t *out = dst_.row(py);
            for (int px = bounds.x; px < bounds.x + bounds.w; px++)
                if (src[px] != kMaskColour)
                    out[px] = src[px];
        }
        grow(touched, bounds);
        for (int py = touched.y; py < touched.y + touched.h; py++)
            std::fill(scratch_.row(py) + touched.x, scratch_.row(py) + touched.x + touched.w, kMaskColour);
    }

    // Masks out on the scratch surface the walls of every cell east or south
    // of (col, row) within `bounds`.
    void mask_walls_in_front(Rect bounds, int col, int row)
    {
        scan_.lines(bounds, tallest_, [&](int mc0, int mc1, int mline) {
            int mbase = scan_.base_y(mline);
            for (int mc = mc0; mc <= mc1; mc++) {
                int mr = mline - mc;
                if (!on_map(mc, mr) || !(mc > col || mr > row))
                    continue;
                const MapCell &m = map_[mc + mr * kMapSize];
                for (uint8_t wall : {m.wall_n, m.wall_w}) {
                    if (!wall)
                        continue;
                    Sprite w = block(blocks_, wall);
                    draw_sprite_one_colour(scratch_, scan_.x(mc, mr), mbase - w.height, w, kMaskColour);
                }
            }
        });
    }

    Surface &dst_;
    const Rect view_;
    const SpriteSheet &blocks_;
    const int tallest_;
    const MapCell *map_;
    const CellScan scan_;
    const CellDrawing drawing_;
    const ShadowSprites *shadows_;
    Surface &scratch_;
};

} // namespace

void draw_world(const ViewTarget &target, const World &world, const GameData &data, const ShadowSprites *shadows,
                Surface &scratch, const DrawnThings &drawn)
{
    if (drawn.pointed)
        drawn.pointed->thing = 0;
    WorldDrawing drawing(target, world, data, shadows, scratch, drawn);
    target.dst.set_clip(target.view);
    drawing.floors();
    drawing.walls_and_things();
    scratch.reset_clip();
    target.dst.reset_clip();
}

} // namespace th
