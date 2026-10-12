#include "core/render/view.h"

#include "core/audio/sounds.h"
#include "core/data/blocks.h"
#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/frame.h"
#include "core/render/shadows.h"
#include "core/render/text.h"
#include "core/render/wibble.h"
#include "core/world/complex.h"
#include "core/world/objects.h"
#include "core/world/reaper.h"
#include "core/world/staff.h"
#include "core/world/world.h"

#include <algorithm>
#include <array>
#include <climits>
#include <cstdio>
#include <optional>
#include <utility>
#include <vector>

namespace th
{

namespace
{

constexpr int kBuckets = 32;

// OS_IN_USE (include/enums.h): an object someone is using.
constexpr uint8_t kObjectInUse = 4;

// ILL_JELLYITIS: the illness whose patients mapwho_person draws wobbling.
constexpr uint8_t kIllJellyitis = 12;

// ILL_SERIOUS_RADIATION: the illness whose patients mapwho_person draws glowing.
constexpr uint8_t kIllSeriousRadiation = 6;

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

// MapCell.flags: something on the cell covers more than one cell (the game's
// MAP_BIG_CELL, set by adjust_big_cell_flag).
constexpr uint8_t kCellBig = 0x20;

// The game's covers table (record_covering_cell, src/engine.cpp): which scan
// cell covers each position. It is a flat byte table of stride 4, as the
// original's `s8 covers[][4]`, so column 4 of a row aliases column 0 of the
// next. The original's is 0x38 bytes, 14 rows, holding one line of its
// 640-wide view; its `idx <= 14` bound reads a 15th row past that table, which
// the port treats as empty. A wider view's line holds more cells, so the table
// is sized per line here: one row per cell, plus one for the footprint offsets
// past its end (check_obscuring_column's (x - y) / 2 is -1..1 for a footprint
// point the table accepts).
struct CoverTable {
    std::vector<int8_t> cell;
    int rows = 0;
    void reset(int n)
    {
        rows = n + 1;
        cell.assign(size_t(rows + 1) * 4, int8_t(-1));
    }
    int8_t &at(int idx, int col) { return cell[size_t(idx) * 4 + size_t(col)]; }

    // check_obscuring_column: the scan cell already covering (x, y) relative
    // to `row`, or -1.
    int8_t check(int row, int x, int y) const
    {
        int idx = row + (x - y >= 0 ? (x - y) / 2 : (x - y - 1) / 2);
        int col = -(x + y);
        if (idx < 0 || idx >= rows || col < 0 || col > 4)
            return -1;
        for (; col >= 0; col += -2) {
            int8_t v = cell[size_t(idx) * 4 + size_t(col)];
            if (v != -1 && v != row)
                return v;
        }
        return -1;
    }

    // record_covering_cell: record `row` as covering `pos` (x + y * 128),
    // unless a row covering x,y+1, x+1,y or x+1,y+1 already does; return that
    // row, else -1.
    int8_t record(int row, int pos)
    {
        int x = pos % 128;
        int y = pos / 128;
        if (x < -0x40) {
            x += 0x80;
            y += -1;
        }
        int idx = row + (x - y >= 0 ? (x - y) / 2 : (x - y - 1) / 2);
        int col = -(x + y);
        if (idx < 0 || idx >= rows || col < 0 || col > 4)
            return -1;
        int8_t v = check(row, x, y + 1);
        if (v < 0)
            v = check(row, x + 1, y);
        if (v < 0)
            v = check(row, x + 1, y + 1);
        if (v >= 0)
            return v;
        at(idx, col) = int8_t(row);
        return -1;
    }
};

// The object footprint's shift from the object's own cell to the cell of a
// person using it (compute_cell_precedence's person case).
int16_t person_footprint_offset(const ShapeInfo &shape)
{
    return int16_t(-int8_t(shape.use_dx) + (-int8_t(shape.use_dy)) * 128);
}

// ror13 (src/draw.cpp): a 32-bit value rotated right 13 bits, the energise
// speckle's step.
uint32_t ror13(uint32_t v)
{
    return (v >> 13) | (v << 19);
}

// DrawPixBuffer for one pixel: flags bit 0 blends through `ghost`, bit 1 is
// opaque (else the mask colour 0xfe is transparent) and bit 2 blends with src
// and dst swapped.
uint8_t pixbuf_pixel(uint8_t dst, uint8_t src, uint8_t flags, const uint8_t *ghost)
{
    if (flags & 1) {
        if (!(flags & 2) && src == kMaskColour)
            return dst;
        return (flags & 4) ? ghost[(src << 8) + dst] : ghost[(dst << 8) + src];
    }
    if (flags & 2)
        return src;
    return src == kMaskColour ? dst : src;
}

// compute_cell_precedence's person case: records the cell the person stands on
// and, if it uses `obj`, the object's footprint shifted by the use position;
// returns a row that must be drawn before it, or -1.
int8_t person_covers(const World &world, int row, uint16_t id, const Thing &t, CoverTable &covers)
{
    const Thing *obj = t.object && t.object < kThingCount ? &world.things[size_t(t.object)] : nullptr;
    if (!obj || obj->user != id)
        return covers.record(row, 0);
    uint8_t shape_index = kObjects[obj->subtype % kObjects.size()].shapes[obj->facing & 3];
    const ShapeInfo &shape = kShapes[shape_index % kShapes.size()];
    covers.record(row, 0);
    const int16_t off = person_footprint_offset(shape);
    for (int k = 0; k < int(shape.count); k++) {
        int8_t v = covers.record(row, shape.points[size_t(k)] + off);
        if (v >= 0)
            return v;
    }
    return -1;
}

// compute_cell_precedence's object case: records its footprint unless it is
// being used (its user draws it).
int8_t object_covers(int row, const Thing &t, CoverTable &covers)
{
    if (t.state == kObjectInUse)
        return -1;
    uint8_t shape_index = kObjects[t.subtype % kObjects.size()].shapes[t.facing & 3];
    const ShapeInfo &shape = kShapes[shape_index % kShapes.size()];
    for (int k = 0; k < int(shape.count); k++) {
        int8_t v = covers.record(row, shape.points[size_t(k)]);
        if (v >= 0)
            return v;
    }
    return -1;
}

// compute_cell_precedence: records the cells the things on `first` cover for
// scan position `row`, returning a position that must be drawn before it
// (they overlap), or -1.
int8_t cell_precedence(const World &world, int row, int16_t first, CoverTable &covers)
{
    for (int16_t id = first; id > 0 && id < kThingCount; id = world.things[size_t(id)].cell_next) {
        const Thing &t = world.things[size_t(id)];
        int8_t v = -1;
        if (t.type == kThingPatient && t.state != kPsMerged)
            v = person_covers(world, row, uint16_t(id), t, covers);
        else if (t.type == kThingStaff && t.state != 0x2a && t.state != kStaffApplicant)
            v = person_covers(world, row, uint16_t(id), t, covers);
        else if (t.type == kThingObject)
            v = object_covers(row, t, covers);
        if (v >= 0)
            return v;
    }
    return -1;
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

// Whether the game's mapwho_* tests `t` for being under the pointer: people,
// rats, the mayor and the inspector always, objects and doors in the states
// mapwho_object draws whole, nothing else whatever its state.
bool pointed_testable(const Thing &t)
{
    if (t.type == kThingPatient || t.type == kThingStaff || t.type == kThingRat || t.type == kThingMayor ||
        t.type == kThingInspector)
        return true;
    if (t.type != kThingObject && t.type != kThingDoor)
        return false;
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

// What draw_cell_things draws from and reports to. `pixbuf` is set while a
// cell is drawn into the scratch surface (the game's othervar.drawing_pixbuf),
// which changes how an energising patient is drawn; `energise` is the 64x64
// buffer draw_energise_frame works in.
struct CellDrawing {
    const World &world;
    const GameData &data;
    DrawnThings drawn;
    bool pixbuf = false;
    Surface *energise = nullptr;
    uint32_t game_turn = 0;
    // The things drawn flashing (othervar.pickup_thing and
    // queue_hilight_thing), 0 for none.
    uint16_t pickup_thing = 0;
    uint16_t queue_hilight_thing = 0;
    // The 180x180 buffer draw_wibble_frame works in.
    Surface *wibble = nullptr;
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

// mapwho_numfx (src/mapwho.cpp): a floating money bag (draw_money) or text
// (draw_text_on_map), risen by its speed; `x, y` are the thing's draw
// position.
void draw_numfx(CellFrames &frames, const GameData &data, const Thing &t, int x, int y)
{
    const int top = y + int(t.speed) - int(t.anim_speed);
    if (t.numfx_type == kNumfxMoney) {
        // draw_money: the bag at x - 2 with the value's digits centred in
        // money_text_width at money_text_x/y (HiresCoords 172-173,
        // src/coords.cpp).
        constexpr int kMoneyTextX = 11, kMoneyTextY = 5, kMoneyTextWidth = 19;
        const SpriteSheet &money = data.money();
        char buf[16];
        std::snprintf(buf, sizeof buf, "%d", int(t.numfx_amount));
        int width = -1;
        for (const char *p = buf; *p; p++)
            width += money.at(size_t(2 + (*p - '0'))).width + 1;
        Sprite bag = money.at(1);
        draw_sprite(frames.dst, x - 2, top, bag);
        grow(frames.bounds, {x - 2, top, bag.width, bag.height});
        int px = x - 2 + kMoneyTextX + (kMoneyTextWidth - width) / 2;
        int py = top + kMoneyTextY;
        for (const char *p = buf; *p; p++) {
            Sprite digit = money.at(size_t(2 + (*p - '0')));
            draw_sprite(frames.dst, px, py, digit);
            grow(frames.bounds, {px, py, digit.width, digit.height});
            px += digit.width + 1;
        }
        return;
    }
    // draw_text_on_map: the number in font 0, in the thing's colour.
    char buf[100];
    std::snprintf(buf, sizeof buf, "%d", int(t.numfx_amount));
    TextSettings settings;
    settings.window = {0, 0, frames.dst.width()};
    settings.clip = {0, 0, frames.dst.width(), frames.dst.height()};
    settings.flags = kTextOneColour;
    settings.colour = t.subtype;
    const Font &font = data.font(0);
    draw_text(frames.dst, font, x, top, buf, settings);
    grow(frames.bounds, {x, top, text_width(font, buf), text_height(font, buf)});
}

// Whether draw_cell_things draws thing `t`. mapwho_object (objects and
// doors), mapwho_person (draw_special_id_frame, draw_split_frame,
// draw_energise_frame and draw_wibble_frame), mapwho_reaper, mapwho_bang
// (explosion debris), mapwho_numfx and mapwho_mayor (the mayor and the
// inspector) are drawn.
bool cell_thing_drawn(const Thing &t)
{
    if (t.type != kThingObject && t.type != kThingDoor && t.type != kThingPatient && t.type != kThingStaff &&
        t.type != kThingReaper && t.type != kThingRat && t.type != kThingDebris && t.type != kThingHelicopter &&
        t.type != kThingShadow && t.type != kThingMayor && t.type != kThingInspector && t.type != kThingNumfx)
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

// The patient kind of an alien (PK_ALIEN), and the animation starts of the
// overlay drawn at the machine's split line over one, by whether it faces 0,
// the game turn's remainder of 3 added.
constexpr uint8_t kPatientAlien = 0xe;
constexpr size_t kAlienOverlayFacingZero = 0x787;
constexpr size_t kAlienOverlayOther = 0x784;

// draw_split_frame (src/draw.cpp): the patient `t` using machine `obj_id`,
// its own frame above the machine's split line and the machine's aux_sprite
// below it, so that it appears inside the machine. An alien has the machine's
// own frame drawn first (its sound queued for the machine, as
// draw_special_id_frame plays it) and, last, a looping overlay at the split
// line. Grows `frames` and returns the box of the patient's frame and the
// machine's, or an alien's overlay's: the game's frame_bounds as the last
// draw leaves them, the hit box.
Rect draw_split_frames(CellFrames &frames, const CellDrawing &drawing, const Thing &t, uint16_t obj_id, Point at,
                       const FrameOptions &opts)
{
    Surface &dst = frames.dst;
    const Thing &obj = drawing.world.things[obj_id];
    Rect saved = dst.clip();
    const bool alien = t.subtype == kPatientAlien;
    if (alien) {
        Rect machine_sprites;
        FrameOptions machine = {.layers = obj.layers.data(), .extent = &machine_sprites};
        if (obj.icon_id > 0) {
            machine.icon = obj.icon_frame;
            machine.icon_over_frame = true;
        }
        grow(frames.bounds, draw_frame(dst, at, obj.sprite, obj.flip != 0, frames.data, machine));
        if (frames.extent)
            grow(*frames.extent, machine_sprites);
        if (std::vector<FrameSound> *sounds = drawing.drawn.sounds;
            sounds && obj.sprite < frames.data.anims().frames.size())
            sounds->push_back({obj_id, frames.data.anims().frames[obj.sprite].sound, false, at.x, at.y});
    }
    const int split = at.y - int(obj.split_height);
    FrameOptions person = opts;
    person.icon = 0;
    Rect person_sprites;
    person.extent = &person_sprites;
    dst.set_clip(intersect(saved, {saved.x, saved.y, saved.w, split - saved.y}));
    Rect box = draw_frame(dst, at, t.sprite, t.flip != 0, frames.data, person);
    dst.set_clip(intersect(saved, {saved.x, split, saved.w, saved.y + saved.h - split}));
    Rect aux_sprites;
    // The machine is drawn with the patient's layer choices, as
    // full_draw_frame does (src/draw.cpp:614).
    grow(box, draw_frame(dst, at, obj.aux_sprite, obj.aux_flip != 0, frames.data,
                         {.layers = opts.layers, .extent = &aux_sprites}));
    dst.set_clip(saved);
    grow(person_sprites, aux_sprites);
    grow(frames.bounds, box);
    const size_t start = (t.facing == 0 ? kAlienOverlayFacingZero : kAlienOverlayOther) + drawing.game_turn % 3;
    if (alien && start < frames.data.anims().starts.size()) {
        const AnimStart &overlay = frames.data.anims().starts[start];
        Rect overlay_sprites;
        box =
            draw_frame(dst, {at.x, split}, overlay.frame, overlay.flip != 0, frames.data, {.extent = &overlay_sprites});
        grow(frames.bounds, box);
        grow(person_sprites, overlay_sprites);
    }
    if (frames.extent)
        grow(*frames.extent, person_sprites);
    return box;
}

// draw_energise_frame's second copy: the patient's frame in a 64x64 buffer
// (its mask colour transparent), holes speckled in as the machine's
// split_height grows, blended over the screen through the ghost table
// (DrawPixBuffer).
void draw_energise_ghost(CellFrames &frames, Surface &pixbuf, const Thing &t, const Thing &obj, Point at,
                         const FrameOptions &opts)
{
    const int n = frames.dst.scale();
    if (pixbuf.scale() != n)
        pixbuf.resize(0x40, 0x40, n);
    pixbuf.fill(kMaskColour);
    pixbuf.reset_clip();
    draw_frame(pixbuf, {0x20, 0x3a}, t.sprite, t.flip != 0, frames.data, {.layers = opts.layers});
    uint32_t rng = 0x546b1;
    for (int i = int(obj.split_height) * 10; i > 0; i--) {
        for (int shift : {0, 10, 20})
            pixbuf.put(((rng >> shift) & 0xfff) % 0x40, ((rng >> shift) & 0xfff) / 0x40, kMaskColour);
        rng = ror13(rng * 0x24a1 + 0x24df);
    }
    const uint8_t *ghost = frames.data.ghost().data();
    const Rect clip = frames.dst.clip();
    const int first = std::max(0, clip.x - (at.x - 0x20));
    const int last = std::min(0x40, clip.x + clip.w - (at.x - 0x20));
    for (int py = 0; py < 0x40; py++) {
        int sy = at.y - 0x3a + py;
        if (sy < clip.y || sy >= clip.y + clip.h)
            continue;
        // The game pixels first..last of the buffer's row, each n indices.
        for (int j = 0; j < n; j++) {
            uint8_t *out = frames.dst.row(sy * n + j);
            const int shift = (at.x - 0x20) * n;
            const uint8_t *in = pixbuf.row(py * n + j);
            for (int k = first * n; k < last * n; k++)
                out[shift + k] = pixbuf_pixel(out[shift + k], in[k], obj.aux_param, ghost);
        }
    }
}

// draw_energise_frame (src/draw.cpp): the patient `t` using machine `obj`,
// drawn with the machine's aux_draw_flags (the first copy is skipped when
// 0xff) and the ghost copy, in the colour/flags aux_param.
Rect draw_energise_frames(CellFrames &frames, Surface &pixbuf, const Thing &t, const Thing &obj, Point at,
                          const FrameOptions &opts)
{
    FrameOptions person = opts;
    person.icon = 0;
    Rect sprites;
    person.extent = &sprites;
    Rect box;
    if (obj.aux_draw_flags != 0xff) {
        person.flags = obj.aux_draw_flags;
        box = draw_frame(frames.dst, at, t.sprite, t.flip != 0, frames.data, person);
        grow(frames.bounds, box);
    }
    if (frames.extent)
        grow(*frames.extent, sprites);
    draw_energise_ghost(frames, pixbuf, t, obj, at, opts);
    if (box.w <= 0)
        box = intersect(frames.dst.clip(), {at.x - 0x20, at.y - 0x3a, 0x40, 0x40});
    return box;
}

// The patient states in which mapwho_person draws a patient glowing,
// wobbling or highlighted rather than split or energised.
constexpr std::array<uint8_t, 26> kPlainPersonStates = {0x04, 0x06, 0x08, 0x0c, 0x10, 0x12, 0x14, 0x16, 0x18,
                                                        0x1a, 0x1c, 0x20, 0x22, 0x24, 0x26, 0x2a, 0x2c, 0x32,
                                                        0x36, 0x38, 0x3a, 0x3c, 0x40, 0x44, 0x46, 0xf0};

bool plain_person_state(const Thing &t)
{
    return t.type == kThingPatient &&
           std::find(kPlainPersonStates.begin(), kPlainPersonStates.end(), t.state) != kPlainPersonStates.end();
}

// Whether a thing's own frame was drawn, and its box.
struct ThingFrame {
    Rect box;
    bool drawn = false;
};

// mapwho_person's glowing and wobbling patients: a patient with radiation in
// a plain state is drawn glowing (draw_glow_frame); one with jellyitis not
// using an object, drawn to the screen rather than the pixbuf, wobbling
// (draw_wibble_frame). Returns whether `t` is one, with its frame in `frame`.
bool draw_glowing_or_wobbling(CellFrames &frames, const CellDrawing &drawing, const Thing &t, Point at,
                              const FrameOptions &opts, ThingFrame &frame)
{
    if (!plain_person_state(t))
        return false;
    if (t.illness == kIllSeriousRadiation) {
        FrameOptions glowing = opts;
        glowing.glow = glow_colour(drawing.game_turn, t.layers.data());
        frame.box = draw_frame(frames.dst, at, t.sprite, t.flip != 0, frames.data, glowing);
    } else if (t.illness == kIllJellyitis && drawing.wibble && !drawing.pixbuf && t.object == 0) {
        frame.box = draw_wibble_frame(frames.dst, *drawing.wibble, {at, t.sprite, t.flip != 0, drawing.game_turn},
                                      frames.data, opts);
    } else {
        return false;
    }
    grow(frames.bounds, frame.box);
    frame.drawn = true;
    return true;
}

// Whether `t` (thing `id`) is drawn flashing: a patient or member of staff
// that is the picked-up thing (mapwho_person, mapwho_staff), a patient hovered
// in the queue window, an object that is or whose pair is the picked-up thing
// (mapwho_object).
bool hilighted(const CellDrawing &drawing, const Thing &t, int id)
{
    const uint16_t pickup = drawing.pickup_thing;
    switch (t.type) {
    case kThingPatient:
        return plain_person_state(t) &&
               ((pickup && id == pickup) || (drawing.queue_hilight_thing && id == drawing.queue_hilight_thing));
    case kThingStaff:
        return pickup && id == pickup;
    case kThingObject:
    case kThingDoor:
        return pickup && (id == pickup || t.pair == pickup);
    default:
        return false;
    }
}

// draw_hilight_frame's options: the layers and extent of `opts` without a
// status icon, the draw flags cycling over six turns as normal, flag 4,
// flag 8, hidden, flag 8, flag 4.
FrameOptions hilight_options(const FrameOptions &opts, uint32_t turn)
{
    FrameOptions lit = opts;
    lit.icon = 0;
    uint32_t phase = turn % 6;
    if (phase >= 4)
        phase = 6 - phase;
    lit.flags = phase == 1 ? unsigned(kDrawGhost) : phase == 2 ? unsigned(kDrawGhostUnder) : 0u;
    lit.hidden = phase == 3;
    return lit;
}

// Draws a thing's own frame as mapwho_person/mapwho_object do: a filling
// patient split around its machine, an energising patient, or the thing's
// frame when it is not an object being used, with `lit` instead of `opts` if
// given (draw_hilight_frame). Returns the frame's box.
ThingFrame draw_thing_frame(CellFrames &frames, const CellDrawing &drawing, const Thing &t, Point at,
                            const FrameOptions &opts, const FrameOptions *lit)
{
    ThingFrame frame;
    const Thing *obj = (t.type == kThingPatient || t.type == kThingStaff) && t.object && t.object < kThingCount
                           ? &drawing.world.things[size_t(t.object)]
                           : nullptr;
    if (t.type == kThingPatient && t.state == kPsFills) {
        // draw_split_frame; with no object it draws nothing.
        if (obj) {
            frame.box = draw_split_frames(frames, drawing, t, t.object, at, opts);
            frame.drawn = true;
        }
    } else if (t.type == kThingPatient && t.state == kPsEnergises) {
        // draw_special_id_frame while drawing into the pixbuf, else
        // draw_energise_frame; with no object it draws nothing.
        if (drawing.pixbuf) {
            frame.box = draw_frame(frames.dst, at, t.sprite, t.flip != 0, frames.data, opts);
            grow(frames.bounds, frame.box);
            frame.drawn = true;
        } else if (obj && drawing.energise) {
            frame.box = draw_energise_frames(frames, *drawing.energise, t, *obj, at, opts);
            frame.drawn = true;
        }
    } else if (draw_glowing_or_wobbling(frames, drawing, t, at, opts, frame)) {
        // Drawn.
    } else if (!((t.type == kThingObject || t.type == kThingDoor) && t.state == kObjectInUse)) {
        // mapwho_object draws an object's own frame only when it is not in
        // use; while one is, its user's animation carries it.
        frame.box = draw_frame(frames.dst, at, t.sprite, t.flip != 0, frames.data, lit ? *lit : opts);
        grow(frames.bounds, frame.box);
        frame.drawn = true;
    }
    return frame;
}

void finish_cell_thing(CellFrames &frames, const CellDrawing &drawing, const Thing &t, int16_t id, Point at,
                       ThingFrame frame);

// draw_cell_things for one thing `id` of the cell whose diamond's top-left
// is `at`.
void draw_cell_thing(CellFrames &frames, const CellDrawing &drawing, int16_t id, Point at)
{
    const GameData &data = drawing.data;
    const Thing &t = drawing.world.things[size_t(id)];
    int tx = at.x + kTileWidth / 2 + ((int(t.x.frac) - int(t.y.frac)) >> 3);
    int ty = at.y + ((int(t.x.frac) + int(t.y.frac)) >> 4);
    // A floating number is drawn by mapwho_numfx, not as a frame.
    if (t.type == kThingNumfx) {
        draw_numfx(frames, data, t, tx, ty);
        return;
    }
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
    Rect sprites;
    FrameOptions opts;
    opts.layers = t.type == kThingDebris || t.type == kThingShadow ? nullptr : t.layers.data();
    opts.extent = &sprites;
    opts.scale = scale;
    // A person's status icon hangs over their head, a machine's over its
    // frame (draw_special_id_frame).
    if (t.icon_id > 0 && (t.type == kThingPatient || t.type == kThingStaff || t.type == kThingObject)) {
        opts.icon = t.icon_frame;
        opts.icon_over_frame = t.type == kThingObject;
    }
    std::optional<FrameOptions> lit;
    if (hilighted(drawing, t, id))
        lit = hilight_options(opts, drawing.game_turn);
    finish_cell_thing(frames, drawing, t, id, {tx, ty},
                      draw_thing_frame(frames, drawing, t, {tx, ty}, opts, lit ? &*lit : nullptr));
    if (frames.extent)
        grow(*frames.extent, sprites);
}

// draw_cell_thing's tail: mapwho_object's smoke of a worn machine over it, the
// pointer recorded if the thing's frame is under it, and the frame's sound
// queued (only when the frame was drawn; the game plays it inside the
// drawing).
void finish_cell_thing(CellFrames &frames, const CellDrawing &drawing, const Thing &t, int16_t id, Point at,
                       ThingFrame frame)
{
    const GameData &data = drawing.data;
    const int tx = at.x, ty = at.y;
    // The game draws the smoke with the layer choices of the bytes holding
    // Thing::smoke, a puff on each of layers 10-12 shown once its byte is 1.
    if (t.type == kThingObject && t.overlay_frame && (pointed_testable(t) || t.state == kObjectInUse)) {
        std::array<uint8_t, kLayerCount> smoke{};
        std::copy(t.smoke.begin(), t.smoke.end(), smoke.begin() + 10);
        Rect overlay_sprites;
        grow(frames.bounds, draw_frame(frames.dst, {tx, ty}, t.overlay_frame, t.flip != 0, data,
                                       {.layers = smoke.data(), .extent = &overlay_sprites}));
        if (frames.extent)
            grow(*frames.extent, overlay_sprites);
    }
    PointedThing *pointed = drawing.drawn.pointed;
    // mapwho_rat: a rat's hit box is its frame within World::Rats::hit_margin
    // and its near box within near_margin; the near box without the hit one
    // sets othervar.near_rat.
    int margin = t.type == kThingRat ? drawing.world.rats.hit_margin : 0;
    bool in_box = pointed && frame.box.x - margin < pointed->at.x &&
                  frame.box.x + frame.box.w + margin > pointed->at.x && frame.box.y - margin < pointed->at.y &&
                  frame.box.y + frame.box.h + margin > pointed->at.y;
    if (in_box && pointed_testable(t)) {
        pointed->thing = uint16_t(id);
    } else if (pointed && t.type == kThingRat && drawing.drawn.near_rat) {
        int near = drawing.world.rats.near_margin;
        if (frame.box.x - near < pointed->at.x && frame.box.x + frame.box.w + near > pointed->at.x &&
            frame.box.y - near < pointed->at.y && frame.box.y + frame.box.h + near > pointed->at.y)
            *drawing.drawn.near_rat = true;
    }
    if (std::vector<FrameSound> *sounds = drawing.drawn.sounds;
        frame.drawn && sounds && t.sprite < data.anims().frames.size())
        sounds->push_back({uint16_t(id), data.anims().frames[t.sprite].sound,
                           t.type == kThingPatient || t.type == kThingStaff, tx, ty});
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
        pixbuf_.resize(0x40, 0x40);
        drawing_.energise = &pixbuf_;
        drawing_.game_turn = target.game_turn;
        drawing_.pickup_thing = target.overlays.pickup_thing;
        drawing_.queue_hilight_thing = target.overlays.queue_hilight_thing;
        wibble_ = make_wibble_buffer();
        drawing_.wibble = &wibble_;
        if (const auto *marks = target.overlays.marks; marks && !marks->empty()) {
            marked_.assign(world.map.begin(), world.map.end());
            for (const CellMark &mark : *marks)
                marked_[mark.cell].flags |= mark.flags;
            map_ = marked_.data();
        }
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
            draw_line_things(col0, col1, line, base_y);
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

    // The game's draw_precedence for one line: a big cell's behind/front chain
    // (a cell covering another is drawn after it) and, for a big cell no other
    // covers, the `next` chain with `head` its first; `last` is the last cell
    // that is not big.
    struct LineOrder {
        std::vector<int8_t> behind, front, next;
        int head = -1;
        int last = -1;
    };

    // do_mapwho's order for the cells of `line` from `col0` to `col1`
    // (draw_all_sprites_on_wscreen_selectively_using_pix_buffer). The covers
    // table is sized to the line, so every cell is ordered against the ones
    // before it.
    LineOrder order_line(int col0, int col1, int line) const
    {
        const int n = col1 - col0 + 1;
        LineOrder order;
        if (n <= 0)
            return order;
        order.behind.assign(size_t(n), -1);
        order.front.assign(size_t(n), -1);
        order.next.assign(size_t(n), -1);
        covers_.reset(n);
        int tail = -1;
        for (int i = 0; i < n; i++) {
            int col = col0 + i, row = line - col;
            if (!on_map(col, row))
                continue;
            const MapCell &cell = map_[col + row * kMapSize];
            if (!cell.first && !(cell.litter_shadow & 0xf) && !(cell.flags & 0xc0))
                continue;
            if (!(cell.flags & kCellBig)) {
                order.next[size_t(i)] = int8_t(order.last);
                order.last = i;
                continue;
            }
            int8_t prec = cell_precedence(drawing_.world, i, cell.first, covers_);
            if (prec < 0) {
                if (order.head >= 0)
                    order.next[size_t(tail)] = int8_t(i);
                else
                    order.head = i;
                tail = i;
                continue;
            }
            order.behind[size_t(i)] = order.behind[size_t(prec)];
            order.front[size_t(i)] = prec;
            order.behind[size_t(prec)] = int8_t(i);
        }
        return order;
    }

    // Draws one scan line's cells in do_mapwho's order: the ones holding more
    // than one cell (MAP_BIG_CELL) with their footprints' overlap resolved,
    // then the rest from the far end of the line.
    void draw_line_things(int col0, int col1, int line, int base_y)
    {
        const LineOrder order = order_line(col0, col1, line);
        for (int k = order.head; k >= 0; k = order.next[size_t(k)]) {
            int i = k;
            while (order.behind[size_t(i)] >= 0)
                i = order.behind[size_t(i)];
            for (; i >= 0; i = order.front[size_t(i)])
                cell_things(col0 + i, line - (col0 + i), base_y);
        }
        for (int i = order.last; i >= 0; i = order.next[size_t(i)])
            cell_things(col0 + i, line - (col0 + i), base_y);
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
        CellDrawing pixbuf = drawing_;
        pixbuf.pixbuf = true;
        Rect touched{};
        Rect bounds =
            intersect(draw_cell_things(scratch_, pixbuf, cell, col + row * kMapSize, {x, top}, &touched), view_);
        touched = intersect(touched, view_);
        scratch_.set_clip(bounds);
        mask_walls_in_front(bounds, col, row);
        // Only the frames' box is copied, as the game does; sprites reaching
        // outside it are cut off.
        const int n = dst_.scale();
        for (int py = bounds.y * n; py < (bounds.y + bounds.h) * n; py++) {
            const uint8_t *src = scratch_.row(py);
            uint8_t *out = dst_.row(py);
            for (int px = bounds.x * n; px < (bounds.x + bounds.w) * n; px++)
                if (src[px] != kMaskColour)
                    out[px] = src[px];
        }
        grow(touched, bounds);
        for (int py = touched.y * n; py < (touched.y + touched.h) * n; py++)
            std::fill(scratch_.row(py) + touched.x * n, scratch_.row(py) + (touched.x + touched.w) * n, kMaskColour);
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
    // The map as drawn: the world's, or `marked_` when cells are marked.
    const MapCell *map_;
    std::vector<MapCell> marked_;
    const CellScan scan_;
    CellDrawing drawing_;
    const ShadowSprites *shadows_;
    Surface &scratch_;
    // The 64x64 buffer draw_energise_frame works in, and the game's covers
    // table, reset for each line.
    Surface pixbuf_;
    Surface wibble_;
    mutable CoverTable covers_;
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
