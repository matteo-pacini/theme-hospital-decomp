#include "common.h"

#include "core/audio/sounds.h"
#include "core/data/balance.h"
#include "core/data/blocks.h"
#include "core/data/gamedata.h"
#include "core/data/level.h"
#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/data/text.h"
#include "core/render/blit.h"
#include "core/render/iso.h"
#include "core/render/panel.h"
#include "core/render/shadows.h"
#include "core/render/surface.h"
#include "core/render/text.h"
#include "core/render/view.h"
#include "core/world/arrivals.h"
#include "core/world/complex_tables.h"
#include "core/world/object_tables.h"
#include "core/world/world.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

void test_blit_clips_and_skips()
{
    // 3x2 sprite: row 0 "skip 1, pixels 7 8", row 1 "pixels 9 9 9".
    const uint8_t rle[] = {0xff, 2, 7, 8, 0, 3, 9, 9, 9, 0};
    th::Sprite s{rle, th::SpriteFormat::Rle, 3, 2};
    th::Surface surf;
    surf.resize(4, 2);
    surf.fill(1);
    th::draw_sprite(surf, 1, 0, s);
    CHECK(surf.row(0)[1] == 1); // transparent run
    CHECK(surf.row(0)[2] == 7 && surf.row(0)[3] == 8);
    CHECK(surf.row(1)[1] == 9 && surf.row(1)[3] == 9);
    surf.fill(1);
    th::draw_sprite(surf, 2, 0, s, th::kDrawFlipX); // right column clipped
    CHECK(surf.row(0)[2] == 8 && surf.row(0)[3] == 7);
}

void test_req_format()
{
    // 70x1: copy 2 (5 6), skip 1, fill 4 with 3 (c = 64), fill 63 with 2 (255).
    const uint8_t req[] = {2, 5, 6, 0x81, 64, 3, 0xff, 63, 2, 0};
    th::Sprite s{req, th::SpriteFormat::Req, 70, 1};
    th::Surface surf;
    surf.resize(70, 1);
    surf.fill(1);
    th::draw_sprite(surf, 0, 0, s);
    CHECK(surf.row(0)[0] == 5 && surf.row(0)[1] == 6);
    CHECK(surf.row(0)[2] == 1);
    CHECK(surf.row(0)[3] == 3 && surf.row(0)[6] == 3);
    CHECK(surf.row(0)[7] == 2 && surf.row(0)[69] == 2);
}

void test_iso_round_trip()
{
    for (int col = 0; col < 128; col += 17)
        for (int row = 0; row < 128; row += 13) {
            th::Point w = th::cell_to_world(col, row);
            // A point just inside the diamond's top corner belongs to the cell.
            th::Point c = th::world_to_cell({w.x, w.y + 1});
            CHECK(c.x == col && c.y == row);
        }
}

// With the game data: every required file unpacks (RNC checks both CRCs),
// sprite tables cover their blocks, and a level builds.
void test_static_shadows()
{
    // Block 0x50 is a wall that casts a shadow (flags 0x01).
    auto map = std::make_unique<std::array<th::MapCell, th::kMapCells>>();
    const int cell = 10 + 10 * th::kMapSize;
    (*map)[cell].wall_w = 0x50;
    (*map)[cell].litter_shadow = 0x81;
    th::add_static_shadows(*map);
    CHECK((*map)[cell].litter_shadow == (0x81 | th::kShadowFloorHalf | th::kShadowWallN));
    CHECK((*map)[cell - th::kMapSize].litter_shadow == th::kShadowFloorFull);
    CHECK((*map)[cell - th::kMapSize - 1].litter_shadow == th::kShadowFloorFull);

    // Continued one row on: a full shadow.
    (*map)[cell + th::kMapSize].wall_w = 0x50;
    th::add_static_shadows(*map);
    CHECK((*map)[cell].litter_shadow == (0x81 | th::kShadowFloorFull | th::kShadowWallN));

    // An odd-facing door casts one too.
    *map = {};
    th::add_static_shadows(*map, [&](int i) { return i == cell ? 1 : -1; });
    CHECK((*map)[cell].litter_shadow == (th::kShadowFloorHalf | th::kShadowWallN));
}

namespace
{

// Pixel size of an RLE sprite's rows, or -1 if a row overruns its width.
int rle_width(const th::Sprite &s)
{
    const uint8_t *p = s.data;
    int widest = 0;
    for (int row = 0; row < s.height; row++) {
        int col = 0;
        for (int8_t n; (n = static_cast<int8_t>(*p++)) != 0;) {
            col += n < 0 ? -n : n;
            if (n > 0)
                p += n;
        }
        widest = std::max(widest, col);
    }
    return widest <= s.width ? s.width : -1;
}

// Every shaded floor and north wall of `map` has a shaded sprite the size of
// its plain one.
void check_cell_shadows(const th::GameData &gd, th::ShadowSprites &shadows,
                        const std::array<th::MapCell, th::kMapCells> &map)
{
    int floors = 0, walls = 0;
    for (const th::MapCell &c : map) {
        th::Sprite plain = gd.blocks().at(size_t(th::block_sprite(c.floor_block)));
        if (c.litter_shadow & (th::kShadowFloorFull | th::kShadowFloorHalf)) {
            floors++;
            th::ShadedSprite s = shadows.floor(c);
            CHECK(s.sprite.width == plain.width && s.sprite.height == plain.height && s.lift == plain.height);
            CHECK(rle_width(s.sprite) == plain.width);
            CHECK(s.sprite.data != plain.data);
        } else {
            CHECK(shadows.floor(c).sprite.data == plain.data);
        }
        if (c.wall_n && (c.litter_shadow & th::kShadowWallN)) {
            walls++;
            th::Sprite base = gd.blocks().at(size_t(th::block_sprite(c.wall_n)));
            th::ShadedSprite s = shadows.wall_n(c);
            CHECK(s.sprite.width == base.width && s.sprite.height == base.height && s.lift == base.height);
            CHECK(rle_width(s.sprite) == base.width);
        }
    }
    CHECK(floors > 0);
    CHECK(walls > 0);
}

// The overlay keeps the runs of the floor and changes some colours.
void check_shadow_overlay(const th::GameData &gd, th::ShadowSprites &shadows)
{
    th::MapCell grass;
    grass.litter_shadow = th::kShadowFloorFull;
    th::Sprite plain = gd.blocks().at(size_t(th::block_sprite(grass.floor_block)));
    th::Palette pal = gd.palette();
    std::vector<uint32_t> a(size_t(plain.width * plain.height)), b(a.size());
    th::sprite_to_argb(plain, pal, a.data(), plain.width * 4);
    th::sprite_to_argb(shadows.floor(grass).sprite, pal, b.data(), plain.width * 4);
    int changed = 0;
    long before = 0, after = 0;
    for (size_t i = 0; i < a.size(); i++) {
        CHECK((a[i] == 0) == (b[i] == 0));
        changed += a[i] != b[i];
        before += (a[i] >> 16 & 255) + (a[i] >> 8 & 255) + (a[i] & 255);
        after += (b[i] >> 16 & 255) + (b[i] >> 8 & 255) + (b[i] & 255);
    }
    CHECK(changed > 0);
    CHECK(after < before); // a shadow darkens
}

} // namespace

void test_shadow_sprites(const char *dir)
{
    th::GameData gd;
    auto level = gd.load(dir, nullptr) ? gd.load_level(1) : std::nullopt;
    CHECK(level.has_value());
    if (!level)
        return;
    auto map = std::make_unique<std::array<th::MapCell, th::kMapCells>>(th::build_start_map(*level));
    // The game draws the map with the 66% table of the game palette
    // (LbGhostTableGenerate in src/pchost.cpp).
    auto ghost = th::load_data_file(dir, "qdata/ghost66.dat");
    CHECK(ghost && ghost->size() >= 0x10000);
    if (!ghost || ghost->size() < 0x10000)
        return;
    th::ShadowSprites shadows(gd.blocks(), *ghost);
    th::add_static_shadows(*map);
    check_cell_shadows(gd, shadows, *map);
    check_shadow_overlay(gd, shadows);
}

namespace
{

// `world`'s map is build_start_map's of `level` but for what objects mark
// (footprint and big-cell flags, invisible walls).
void check_map_as_level(const th::World &world, const th::Level &level)
{
    auto base = th::build_start_map(level);
    for (int i = 0; i < th::kMapCells; i++) {
        const th::MapCell &a = world.map[size_t(i)];
        const th::MapCell &b = base[size_t(i)];
        CHECK(a.floor_block == b.floor_block && a.owner_flags == b.owner_flags);
        CHECK(a.litter_shadow == b.litter_shadow);
        CHECK((a.flags & ~0x27) == (b.flags & ~0x27));
        CHECK(a.wall_n == b.wall_n || (b.wall_n == 0 && (a.wall_n == 0x8e || a.wall_n == 0x90)));
        CHECK(a.wall_w == b.wall_w || (b.wall_w == 0 && (a.wall_w == 0x8f || a.wall_w == 0x91)));
    }
}

// The objects of `world`, started from `level`, are its objects and are on
// well-linked cell lists, and the map is the level's but for what they mark.
// Returns the objects, the level cells holding one, and those linked.
std::array<int, 3> check_level_start(const th::World &world, const th::GameData &gd, const th::Level &level)
{
    int objects = 0;
    int in_level = 0;
    for (int i = 0; i < th::kMapCells; i++) {
        uint16_t plot = level.plots[size_t(i)];
        bool copied = plot == 0 || (plot <= 4 && level.player_count >= plot);
        in_level += copied && level.tiles[size_t(i)].first != 0;
    }
    for (size_t id = 1; id < world.things.size(); id++) {
        const th::Thing &t = world.things[id];
        if (t.type == th::kThingFree)
            continue;
        objects++;
        CHECK(t.type == th::kThingObject);
        CHECK(t.subtype < th::kObjectKinds && t.facing < 4);
        CHECK(t.sprite < gd.anims().frames.size());
        CHECK(t.x.frac == 0x80 && t.y.frac == 0x80);
    }

    // Every list is well linked and holds things of its own cell.
    int linked = 0;
    for (int i = 0; i < th::kMapCells; i++) {
        int prev = 0;
        for (int id = world.map[size_t(i)].first; id > 0; id = world.things[size_t(id)].cell_next) {
            const th::Thing &t = world.things[size_t(id)];
            CHECK(t.cell_prev == prev);
            CHECK(t.x.tile + t.y.tile * th::kMapSize == i);
            prev = id;
            linked++;
        }
    }
    // Two-part objects whose second part lands on a later cell lose it
    // from the list when that cell is copied, as in the game.
    CHECK(linked <= objects && objects >= in_level);

    check_map_as_level(world, level);
    return {objects, in_level, linked};
}

// The levels hold only entrance doors; a made-up one tries the rest. All
// of it is plot 1. A radiation shield (28) facing 0 brings its console
// (15) on the cell before; an operating table (30) facing 1 would bring
// its lamp on a cell after, not yet copied, so the lamp does not fit.
void check_made_up_level(th::World &world, const th::GameData &gd)
{
    auto level = std::make_unique<th::Level>();
    level->player_count = 1;
    level->plots.fill(1);
    const int shield = 10 + 10 * th::kMapSize;
    const int table = 20 + 10 * th::kMapSize;
    const int plant = 30 + 10 * th::kMapSize;
    level->tiles[shield].first = 28 << 8;
    level->tiles[table].first = 30 << 8 | 1;
    level->tiles[plant].first = 45 << 8;
    th::start_level(world, *level, gd.anims());
    const th::Thing &shield_obj = world.things[1];
    const th::Thing &console = world.things[2];
    CHECK(shield_obj.subtype == 28 && console.subtype == 15 && console.facing == 1);
    CHECK(shield_obj.pair == 2 && console.pair == 1);
    CHECK(world.map[shield].first == 1 && world.map[shield - 1].first == 2);
    CHECK(world.things[3].subtype == 30 && world.things[3].pair == 0);
    CHECK(world.things[4].subtype == 45 && world.things[5].type == th::kThingFree);
    CHECK(world.things[4].sprite == gd.anims().starts[0x3cf].frame);
}

} // namespace

// With the game data: each level's starting objects are placed, each on its
// cell's list, with a valid frame, and the map is build_start_map's but for
// what objects mark (footprint and big-cell flags, invisible walls).
void test_level_objects(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto world = std::make_unique<th::World>();
    for (int n = 1; n <= 12; n++) {
        auto level = gd.load_level(n, &error);
        CHECK(level.has_value());
        if (!level)
            continue;
        th::start_level(*world, *level, gd.anims());
        auto [objects, in_level, linked] = check_level_start(*world, gd, *level);
        std::printf("level %d: %d objects from %d level cells, %d on cell lists\n", n, objects, in_level, linked);
    }
    check_made_up_level(*world, gd);
}

namespace
{

// Centred on each object of `world` in turn, the view differs from `bare`'s
// (with no things) both plain and drawn through the mask, and the scratch
// surface is left blank.
void check_objects_drawn(const th::GameData &gd, th::ShadowSprites &shadows, const th::World &world,
                         const th::World &masked, const th::World &bare)
{
    th::Surface with, without, scratch;
    with.resize(640, 480);
    without.resize(640, 480);
    scratch.resize(640, 480);
    scratch.fill(th::kMaskColour);
    for (int id = 1; id < th::kThingCount; id++) {
        const th::Thing &t = world.things[size_t(id)];
        if (t.type != th::kThingObject)
            continue;
        th::Point p = th::thing_to_world(t);
        th::Point camera{p.x - 320, p.y - 240};
        th::Rect view{0, 0, 640, 480};
        th::draw_world({with, view, camera}, world, gd, &shadows, scratch);
        th::draw_world({without, view, camera}, bare, gd, &shadows, scratch);
        CHECK(!std::equal(with.data(), with.data() + 640 * 480, without.data()));
        CHECK(std::all_of(scratch.data(), scratch.data() + 640 * 480, [](uint8_t c) { return c == th::kMaskColour; }));
        th::Surface through_mask;
        through_mask.resize(640, 480);
        th::draw_world({through_mask, view, camera}, masked, gd, &shadows, scratch);
        CHECK(!std::equal(through_mask.data(), through_mask.data() + 640 * 480, without.data()));
        CHECK(std::all_of(scratch.data(), scratch.data() + 640 * 480, [](uint8_t c) { return c == th::kMaskColour; }));
    }
}

constexpr int kStableViewWidth = 1024, kStableViewHeight = 480;

// Renders `world` at camera `camera` and at `camera` moved `step` pixels
// right, and reports whether the second is the first shifted by `step`: the
// same cells are drawn in the same order however the view is positioned, as
// the game's covers table orders a whole line of its 640-wide view.
bool stable_when_moved(const th::GameData &gd, const th::World &world, th::ShadowSprites &shadows, th::Point camera,
                       int step)
{
    th::Rect view{0, 0, kStableViewWidth, kStableViewHeight};
    th::Surface a, b, scratch;
    a.resize(view.w, view.h);
    b.resize(view.w, view.h);
    scratch.resize(view.w, view.h);
    scratch.fill(th::kMaskColour);
    th::draw_world({a, view, camera}, world, gd, &shadows, scratch);
    th::draw_world({b, view, {camera.x + step, camera.y}}, world, gd, &shadows, scratch);
    for (int y = 0; y < view.h; y++)
        for (int x = 0; x + step < view.w; x++)
            if (a.row(y)[x + step] != b.row(y)[x])
                return false;
    return true;
}

} // namespace

// A view wider than the original's 640 pixels holds more cells per scan line
// than the game's covers table; moving the camera must not change how the
// things in it are ordered (the same view, shifted). Multi-cell objects (a
// block of desks; adjust_big_cell_flag) exercise the table: neighbouring big
// cells must be ordered the same at any camera position.
void test_draw_order_stable(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto level = gd.load_level(1, &error);
    CHECK(level.has_value());
    if (!level)
        return;
    auto world = std::make_unique<th::World>();
    th::start_level(*world, *level, gd.anims());
    th::ShadowSprites shadows(gd.blocks(), gd.ghost());
    th::add_static_shadows(world->map);
    int start = level->camera_start[0];
    int c0 = start % th::kMapSize, r0 = start / th::kMapSize;
    for (int dr = -3; dr <= 3; dr++)
        for (int dc = -3; dc <= 3; dc++)
            if (c0 + dc >= 0 && c0 + dc < th::kMapSize && r0 + dr >= 0 && r0 + dr < th::kMapSize)
                th::place_object(*world, gd.anims(), {(c0 + dc) + (r0 + dr) * th::kMapSize, 1, 0}, 0, true);
    th::Point centre = th::cell_to_world(c0, r0);
    for (int camy = -64; camy <= 64; camy += 64)
        for (int camx = -160; camx <= 160; camx += 32)
            for (int step : {32, 64})
                CHECK(stable_when_moved(
                    gd, *world, shadows,
                    {centre.x - kStableViewWidth / 2 + camx, centre.y - kStableViewHeight / 2 + camy}, step));
}

// Each level's starting objects show up in the view, and the masked drawing
// leaves its scratch surface blank.
void test_draw_world(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    for (int n = 1; n <= 12; n++) {
        auto level = gd.load_level(n, &error);
        CHECK(level.has_value());
        if (!level)
            continue;
        auto world = std::make_unique<th::World>();
        th::start_level(*world, *level, gd.anims());
        th::ShadowSprites shadows(gd.blocks(), gd.ghost());
        th::add_static_shadows(world->map);
        // Nothing at a level's start is behind a wall (map flag 0x10); mark the
        // objects' cells so the masked drawing is exercised too.
        auto masked = std::make_unique<th::World>(*world);
        for (th::MapCell &c : masked->map)
            if (c.first > 0)
                c.flags |= 0x10;
        auto bare = std::make_unique<th::World>(*world);
        for (th::MapCell &c : bare->map)
            c.first = 0;

        check_objects_drawn(gd, shadows, *world, *masked, *bare);
    }
}

// The panel drawn apart (as an ARGB image) matches the panel drawn into the
// view, transparent where nothing covers it.
void test_panel_image(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    int w = 0, h = 0;
    std::vector<uint32_t> argb = th::panel_to_argb(gd, nullptr, w, h);
    CHECK(w == 640 && h > 0);
    th::Surface s;
    s.resize(w, h);
    s.fill(0);
    th::Rect r = th::draw_panel(s, gd.panel());
    CHECK(r.x == 0 && r.y + r.h == h && r.w == w);
    int opaque = 0;
    for (int i = 0; i < w * h; i++) {
        if (argb[size_t(i)] == 0)
            continue;
        opaque++;
        CHECK(argb[size_t(i)] == gd.palette()[s.data()[i]]);
    }
    CHECK(opaque >= r.w * r.h / 2);
}

void test_text_layout()
{
    // A font with a 3x2 space (sprite 1) and a 2x2 'A' (sprite 'A' - 31);
    // every other sprite empty.
    const uint8_t a_rle[] = {2, 5, 5, 0, 2, 5, 5, 0};
    std::vector<uint8_t> tab(6 * 35, 0);
    tab[6 * 1 + 4] = 3;
    tab[6 * 1 + 5] = 2;
    tab[6 * 34 + 4] = 2;
    tab[6 * 34 + 5] = 2;
    th::SpriteSheet sheet;
    CHECK(sheet.load(tab, std::vector<uint8_t>(std::begin(a_rle), std::end(a_rle))));
    th::Font font(std::move(sheet));
    CHECK(th::text_width(font, "A A") == 7);
    CHECK(th::text_width(font, "A\nA A\tA") == 2 + 3 + 2 + 12 + 2);
    CHECK(th::text_height(font, "A\nA") == 4);

    th::Surface surf;
    surf.resize(20, 4);
    surf.fill(1);
    // Centred in a 20-wide window: (20 - 7) / 2 = 6.
    th::draw_text(surf, font, 0, 0, "A A", {{0, 1, 20}, {0, 0, 20, 4}, th::kTextCentre});
    CHECK(surf.row(1)[5] == 1 && surf.row(1)[6] == 5 && surf.row(2)[7] == 5 && surf.row(1)[8] == 1);
    CHECK(surf.row(1)[11] == 5 && surf.row(1)[12] == 5 && surf.row(1)[13] == 1 && surf.row(0)[6] == 1);
    // The clip window cuts the second 'A' and leaves the surface's clip alone.
    surf.fill(1);
    th::draw_text(surf, font, 0, 0, "A A", {{0, 0, 20}, {0, 0, 7, 4}, th::kTextCentre});
    CHECK(surf.row(0)[6] == 5 && surf.row(0)[11] == 1);
    CHECK(surf.clip().w == 20);
    // Wrapped at the window's width, the second line one line height down.
    surf.fill(1);
    th::draw_text(surf, font, 0, 0, "A A", {{0, 0, 4}, {0, 0, 20, 4}, th::kTextWrap});
    CHECK(surf.row(0)[0] == 5 && surf.row(2)[0] == 5 && surf.row(0)[5] == 1);
}

namespace
{

// A level 12 world with a DNA fixer and an alien filling it (PS_FILLS), both
// on one cell.
struct FillingAlien {
    std::unique_ptr<th::World> world = std::make_unique<th::World>();
    uint16_t machine = 0;
    uint16_t alien = 0;
};

FillingAlien make_filling_alien(const th::GameData &gd, const th::Level &level)
{
    FillingAlien f;
    th::start_level(*f.world, level, gd.anims());
    const th::Balance settings;
    th::ArrivalState arrivals;
    const int cell = 64 + 70 * th::kMapSize;
    f.machine = th::place_object(*f.world, gd.anims(), {cell, 23, 0}, 0, true);
    f.alien = th::make_a_peep(*f.world, arrivals, {settings, gd.anims(), 12}, uint16_t(cell), 8, 0);
    if (!f.machine || !f.alien)
        return f;
    th::Thing &p = f.world->things[f.alien];
    th::Thing &m = f.world->things[f.machine];
    p.state = th::kPsFills;
    p.object = f.machine;
    const auto &starts = gd.anims().starts;
    p.sprite = uint16_t(starts[th::kPatientAnimStarts[14][0x79] + p.facing].frame);
    m.aux_sprite = uint16_t(starts[th::kPatientAnimStarts[p.orig_subtype][0x79] + p.facing].frame);
    m.split_height = 20;
    return f;
}

// Whether the thing picked at some point of the view round the alien is the
// alien.
bool alien_picked(const th::GameData &gd, const FillingAlien &f, th::Surface &dst, th::Surface &scratch, th::Rect view,
                  th::Point camera)
{
    for (int dy = -40; dy <= 40; dy += 4) {
        for (int dx = -24; dx <= 24; dx += 4) {
            th::PointedThing pointed{{320 + dx, 240 + dy}, 0};
            th::DrawnThings picking;
            picking.pointed = &pointed;
            th::draw_world({dst, view, camera, 0}, *f.world, gd, nullptr, scratch, picking);
            if (pointed.thing == f.alien)
                return true;
        }
    }
    return false;
}

} // namespace

// An alien filling a DNA fixer (PS_FILLS, PK_ALIEN) is drawn with a looping
// overlay that follows the game turn (three turns a loop), the machine's frame
// sound is queued for the machine, and the thing picked at the overlay's box is
// the alien.
void test_alien_filling(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto level = gd.load_level(12, &error);
    CHECK(level.has_value());
    if (!level)
        return;
    FillingAlien f = make_filling_alien(gd, *level);
    CHECK(f.machine != 0 && f.alien != 0);
    if (!f.machine || !f.alien)
        return;
    CHECK(f.world->things[f.alien].subtype == 14);
    th::Point at = th::thing_to_world(f.world->things[f.alien]);
    th::Rect view{0, 0, 640, 480};
    th::Point camera{at.x - 320, at.y - 240};
    th::Surface shots[4], scratch;
    scratch.resize(640, 480);
    scratch.fill(th::kMaskColour);
    std::vector<th::FrameSound> sounds;
    for (uint32_t turn = 0; turn < 4; turn++) {
        shots[turn].resize(640, 480);
        sounds.clear();
        th::DrawnThings drawn;
        drawn.sounds = &sounds;
        th::draw_world({shots[turn], view, camera, turn}, *f.world, gd, nullptr, scratch, drawn);
    }
    auto same = [&](int a, int b) { return std::equal(shots[a].data(), shots[a].data() + 640 * 480, shots[b].data()); };
    CHECK(!same(0, 1) && !same(1, 2));
    CHECK(same(0, 3));
    CHECK(std::any_of(sounds.begin(), sounds.end(), [&](const th::FrameSound &s) { return s.thing == f.machine; }));
    CHECK(alien_picked(gd, f, shots[0], scratch, view, camera));
}

// The mayor and the inspector are drawn and picked by their type whatever
// their state (mapwho_mayor draws and tests both; MAYOR_PAUSES and
// MAYOR_INSPECTS are not object states).
void test_mayor_pointed(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto level = gd.load_level(12, &error);
    CHECK(level.has_value());
    if (!level)
        return;
    FillingAlien f = make_filling_alien(gd, *level);
    CHECK(f.alien != 0);
    if (!f.alien)
        return;
    th::Thing &visitor = f.world->things[f.alien];
    th::Point at = th::thing_to_world(visitor);
    th::Rect view{0, 0, 640, 480};
    th::Point camera{at.x - 320, at.y - 240};
    th::Surface shot, scratch;
    shot.resize(640, 480);
    scratch.resize(640, 480);
    scratch.fill(th::kMaskColour);
    for (uint8_t type : {th::kThingMayor, th::kThingInspector}) {
        for (uint8_t state : {2, 4, 6, 8}) {
            visitor.type = type;
            visitor.state = state;
            CHECK(alien_picked(gd, f, shot, scratch, view, camera));
        }
    }
}
