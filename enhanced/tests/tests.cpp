// Engine tests. Those that need the game data read it from TH_TEST_DATA_DIR
// and are skipped (exit code 77) when it is unset or missing.

#include "core/audio/sounds.h"
#include "core/data/balance.h"
#include "core/data/blocks.h"
#include "core/data/fs.h"
#include "core/data/gamedata.h"
#include "core/data/rnc.h"
#include "core/data/smacker.h"
#include "core/data/text.h"
#include "core/render/blit.h"
#include "core/render/iso.h"
#include "core/render/panel.h"
#include "core/render/shadows.h"
#include "core/render/text.h"
#include "core/render/view.h"
#include "core/ui/buy_objects.h"
#include "core/ui/hire_staff.h"
#include "core/ui/menubar.h"
#include "core/ui/startup.h"
#include "core/ui/windows.h"
#include "core/world/arrivals.h"
#include "core/world/complex.h"
#include "core/world/game.h"
#include "core/world/nav.h"
#include "core/world/objects.h"
#include "core/world/staff.h"
#include "core/world/world.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace
{

int failures = 0;

#define CHECK(cond)                                                                                                    \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            std::fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond);                              \
            failures++;                                                                                                \
        }                                                                                                              \
    } while (0)

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

void test_rnc_rejects_garbage()
{
    const uint8_t junk[] = {'R', 'N', 'C', 1, 0, 0, 0, 4, 0, 0, 0, 0};
    CHECK(!th::rnc_unpack(junk).has_value());
}

void test_missing_data()
{
    // A folder without the game reports every required file.
    auto dir = std::filesystem::temp_directory_path() / "th_enhanced_empty_test_dir";
    std::filesystem::create_directories(dir);
    CHECK(th::missing_data_files(dir).size() == th::required_data_files().size());
    std::filesystem::remove(dir);
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

void test_game_data(const char *dir)
{
    for (std::string_view rel : th::required_data_files()) {
        std::string error;
        auto data = th::load_data_file(dir, rel, &error);
        if (!data)
            std::fprintf(stderr, "%s\n", error.c_str());
        CHECK(data.has_value());
    }
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    if (!error.empty())
        std::fprintf(stderr, "%s\n", error.c_str());
    CHECK(gd.blocks().size() > 0);
    CHECK(gd.panel().size() > 14);
    auto level = gd.load_level(1, &error);
    CHECK(level.has_value());
    if (level) {
        CHECK(level->player_count >= 1 && level->player_count <= 4);
        auto map = th::build_start_map(*level);
        int walls = 0;
        for (const th::MapCell &c : map) {
            CHECK(th::block_sprite(c.floor_block) >= 0);
            walls += c.wall_n != 0 || c.wall_w != 0;
        }
        CHECK(walls > 0);
    }
}

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
    th::ShadowSprites shadows(gd.blocks(), *ghost, map->data());
    th::add_static_shadows(*map);

    int floors = 0, walls = 0;
    for (const th::MapCell &c : *map) {
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
            CHECK(s.sprite.width == base.width && s.sprite.height == base.height);
            CHECK(rle_width(s.sprite) == base.width);
        }
    }
    CHECK(floors > 0);
    CHECK(walls > 0);

    // The overlay keeps the runs of the floor and changes some colours.
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

        int objects = 0;
        int in_level = 0;
        for (int i = 0; i < th::kMapCells; i++) {
            uint16_t plot = level->plots[size_t(i)];
            bool copied = plot == 0 || (plot <= 4 && level->player_count >= plot);
            in_level += copied && level->tiles[size_t(i)].first != 0;
        }
        for (size_t id = 1; id < world->things.size(); id++) {
            const th::Thing &t = world->things[id];
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
            for (int id = world->map[size_t(i)].first; id > 0; id = world->things[size_t(id)].cell_next) {
                const th::Thing &t = world->things[size_t(id)];
                CHECK(t.cell_prev == prev);
                CHECK(t.x.tile + t.y.tile * th::kMapSize == i);
                prev = id;
                linked++;
            }
        }
        // Two-part objects whose second part lands on a later cell lose it
        // from the list when that cell is copied, as in the game.
        CHECK(linked <= objects && objects >= in_level);

        auto base = th::build_start_map(*level);
        for (int i = 0; i < th::kMapCells; i++) {
            const th::MapCell &a = world->map[size_t(i)];
            const th::MapCell &b = base[size_t(i)];
            CHECK(a.floor_block == b.floor_block && a.owner_flags == b.owner_flags);
            CHECK(a.litter_shadow == b.litter_shadow);
            CHECK((a.flags & ~0x27) == (b.flags & ~0x27));
            CHECK(a.wall_n == b.wall_n || (b.wall_n == 0 && (a.wall_n == 0x8e || a.wall_n == 0x90)));
            CHECK(a.wall_w == b.wall_w || (b.wall_w == 0 && (a.wall_w == 0x8f || a.wall_w == 0x91)));
        }
        std::printf("level %d: %d objects from %d level cells, %d on cell lists\n", n, objects, in_level, linked);
    }

    // The levels hold only entrance doors; a made-up one tries the rest. All
    // of it is plot 1. A radiation shield (28) facing 0 brings its console
    // (15) on the cell before; an operating table (30) facing 1 would bring
    // its lamp on a cell after, not yet copied, so the lamp does not fit.
    auto level = std::make_unique<th::Level>();
    level->player_count = 1;
    level->plots.fill(1);
    const int shield = 10 + 10 * th::kMapSize;
    const int table = 20 + 10 * th::kMapSize;
    const int plant = 30 + 10 * th::kMapSize;
    level->tiles[shield].first = 28 << 8;
    level->tiles[table].first = 30 << 8 | 1;
    level->tiles[plant].first = 45 << 8;
    th::start_level(*world, *level, gd.anims());
    const th::Thing &shield_obj = world->things[1];
    const th::Thing &console = world->things[2];
    CHECK(shield_obj.subtype == 28 && console.subtype == 15 && console.facing == 1);
    CHECK(shield_obj.pair == 2 && console.pair == 1);
    CHECK(world->map[shield].first == 1 && world->map[shield - 1].first == 2);
    CHECK(world->things[3].subtype == 30 && world->things[3].pair == 0);
    CHECK(world->things[4].subtype == 45 && world->things[5].type == th::kThingFree);
    CHECK(world->things[4].sprite == gd.anims().starts[0x3cf].frame);
}

} // namespace

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
        th::ShadowSprites shadows(gd.blocks(), gd.ghost(), world->map.data());
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

        th::Surface with, without, scratch;
        with.resize(640, 480);
        without.resize(640, 480);
        scratch.resize(640, 480);
        scratch.fill(th::kMaskColour);
        for (int id = 1; id < th::kThingCount; id++) {
            const th::Thing &t = world->things[size_t(id)];
            if (t.type != th::kThingObject)
                continue;
            th::Point p = th::thing_to_world(t);
            th::Point camera{p.x - 320, p.y - 240};
            th::Rect view{0, 0, 640, 480};
            th::draw_world(with, view, camera, *world, gd, &shadows, scratch);
            th::draw_world(without, view, camera, *bare, gd, &shadows, scratch);
            CHECK(!std::equal(with.data(), with.data() + 640 * 480, without.data()));
            CHECK(std::all_of(scratch.data(), scratch.data() + 640 * 480,
                              [](uint8_t c) { return c == th::kMaskColour; }));
            th::Surface through_mask;
            through_mask.resize(640, 480);
            th::draw_world(through_mask, view, camera, *masked, gd, &shadows, scratch);
            CHECK(!std::equal(through_mask.data(), through_mask.data() + 640 * 480, without.data()));
            CHECK(std::all_of(scratch.data(), scratch.data() + 640 * 480,
                              [](uint8_t c) { return c == th::kMaskColour; }));
        }
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
    th::draw_text(surf, font, {0, 1, 20}, 0, 0, "A A", th::kTextCentre, {0, 0, 20, 4});
    CHECK(surf.row(1)[5] == 1 && surf.row(1)[6] == 5 && surf.row(2)[7] == 5 && surf.row(1)[8] == 1);
    CHECK(surf.row(1)[11] == 5 && surf.row(1)[12] == 5 && surf.row(1)[13] == 1 && surf.row(0)[6] == 1);
    // The clip window cuts the second 'A' and leaves the surface's clip alone.
    surf.fill(1);
    th::draw_text(surf, font, {0, 0, 20}, 0, 0, "A A", th::kTextCentre, {0, 0, 7, 4});
    CHECK(surf.row(0)[6] == 5 && surf.row(0)[11] == 1);
    CHECK(surf.clip().w == 20);
    // Wrapped at the window's width, the second line one line height down.
    surf.fill(1);
    th::draw_text(surf, font, {0, 0, 4}, 0, 0, "A A", th::kTextWrap, {0, 0, 20, 4});
    CHECK(surf.row(0)[0] == 5 && surf.row(2)[0] == 5 && surf.row(0)[5] == 1);
}

void test_language_and_fonts(const char *dir)
{
    th::LanguageText text;
    std::string error;
    CHECK(th::load_language(dir, th::kEnglish, text, &error));
    CHECK(text.section_count() == 0x44);
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    CHECK(text.size(th::TextSection::Months) == 12);
    for (int i = 0; i < 12; i++)
        CHECK(text.month(i) == months[i]);
    CHECK(text.month(12).empty());
    auto language = th::config_language(dir);
    CHECK(!language || (*language >= th::kEnglish && *language <= th::kSwedish));

    th::Font balance_font, date_font;
    CHECK(th::load_font(dir, 5, balance_font, &error));
    CHECK(th::load_font(dir, 6, date_font, &error));
    CHECK(!balance_font.empty() && !date_font.empty());

    int glyphs = 0;
    for (char c : std::string_view("15143"))
        glyphs += balance_font.glyph(uint8_t(c)).width;
    int width = th::text_width(balance_font, "15143");
    CHECK(width == glyphs && width > 0);

    th::Surface surf;
    surf.resize(200, 40);
    surf.fill(0);
    th::draw_text(surf, balance_font, {10, 5, 0}, 0, 0, "15143", 0, {0, 0, 200, 40});
    int drawn = 0, left = 200, right = -1;
    for (int y = 0; y < 40; y++) {
        for (int x = 0; x < 200; x++) {
            if (surf.row(y)[x] == 0)
                continue;
            drawn++;
            left = std::min(left, x);
            right = std::max(right, x);
            CHECK(y >= 5 && y < 5 + balance_font.line_height());
        }
    }
    CHECK(drawn > 0);
    CHECK(left >= 10 && right < 10 + width);

    // The panel's date: "%d %s" centred (0x100) in the date box.
    std::string date = "1 " + std::string(text.month(0));
    int date_width = th::text_width(date_font, date);
    surf.fill(0);
    th::draw_text(surf, date_font, {0, 0, 200}, 0, 0, date, th::kTextCentre, {0, 0, 200, 40});
    left = 200;
    right = -1;
    for (int y = 0; y < 40; y++) {
        for (int x = 0; x < 200; x++) {
            if (surf.row(y)[x] != 0) {
                left = std::min(left, x);
                right = std::max(right, x);
            }
        }
    }
    CHECK(left >= (200 - date_width) / 2 && right < (200 - date_width) / 2 + date_width);
}

void test_balance(const char *dir)
{
    auto text = [](std::string_view s) {
        return std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(s.data()), s.size());
    };

    // Every shipped file reads to its end.
    for (const char *id : {"EASY", "FULL", "HARD"}) {
        for (int level = 0; level <= 12; level++) {
            char rel[32];
            std::snprintf(rel, sizeof rel, "levels/%s%02d.sam", id, level);
            auto file = th::load_data_file(dir, rel);
            CHECK(file);
            th::Balance b;
            CHECK(file && th::read_balance_file(*file, b));
        }
    }

    for (int level = 1; level <= 12; level++) {
        th::Balance b;
        std::string error;
        CHECK(th::load_balance(dir, level, "", b, &error));
        CHECK(error.empty());
    }

    // levels/FULL00.SAM, then FULL01.SAM over it.
    th::Balance b;
    CHECK(th::load_balance(dir, 1, th::kDefaultBalanceId, b));
    CHECK(b.staff[0].min_salary == 60 && b.staff[1].min_salary == 75);
    CHECK(b.gbv.salary_add[3] == -30 && b.gbv.salary_add[4] == 30);
    CHECK(b.towns[1].start_cash == 40000 && b.towns[1].ill_rate == 2 && b.towns[1].interest_rate == 100);
    CHECK(b.towns[1].inflation_rate == 450); // no file sets it: the static value
    CHECK(b.towns[3].start_cash == 50000 && b.towns[3].interest_rate == 300);
    CHECK(b.gbv.hold_visual_peep_count == 2); // FULL00 says 6
    CHECK(b.gbv.open_countdown == 5000);
    CHECK(b.expertise[2].start_price == 850 && b.expertise[2].max_diag_diff == 200);
    CHECK(b.popn[0].month == 0 && b.popn[0].change == 3);
    CHECK(b.popn[4].month == 21 && b.popn[4].change == 0);
    CHECK(b.popn[5].month == 11 && b.popn[5].change == 0); // static, carried
    CHECK(b.win_criteria[4].criteria == 6 && b.win_criteria[4].value == 55000 && b.win_criteria[4].group == 1);
    CHECK(b.lose_criteria[1].value == -20000 && b.lose_criteria[1].group == 2 && b.lose_criteria[1].bound == 0);
    CHECK(b.staff_levels[1].seed == 8654 && b.staff_levels[1].receptionists == 6);
    CHECK(b.computer[2].playing == 1);
    CHECK(b.awards_trophies.cures_penalty == -3000);
    // balance_control: visuals 0 (5) and 3 (3), four non-visuals (19).
    CHECK(b.visuals_total == 8 && b.visuals[2] == 5 && b.visuals[13] == 8);
    CHECK(b.non_visuals_total == 19);
    CHECK(b.illness_kinds.size() == 22 && b.illness_kinds[0] == 2 && b.illness_kinds[1] == 5 &&
          b.illness_kinds[2] == 16);
    CHECK(b.objects[39].available_for_level == 1 && b.expertise[1].known == 1);

    th::Balance hard;
    CHECK(th::load_balance(dir, 1, "HARD", hard));
    CHECK(hard.towns[1].start_cash == 30000 && hard.towns[1].interest_rate == 500 && hard.staff[0].min_salary == 70);

    th::Balance missing;
    std::string error;
    CHECK(!th::load_balance(dir, 13, "FULL", missing, &error) && !error.empty());

    CHECK(th::balance_description(dir, "FULL", 0) == " Doctor (Medium)");
    CHECK(th::balance_description(dir, "EASY", 2) == " AIPler (Einfach)");
    CHECK(!th::balance_description(dir, "NONE", 0));

    // A value out of bounds: the rest of its line is still read, the rest of
    // the file is not.
    th::Balance s;
    CHECK(!th::read_balance_file(text("#staff[0].MinSalary 70\r\n"
                                      "#towns[2].StartCash.IllRate.InterestRate 100001 7 321\r\n"
                                      "#staff[1].MinSalary 71\r\n"),
                                 s));
    CHECK(s.staff[0].min_salary == 70 && s.staff[1].min_salary == 270);
    CHECK(s.towns[2].start_cash == 50010 && s.towns[2].ill_rate == 7 && s.towns[2].interest_rate == 321);
    // Unknown keys and indexes above the bound stop it too; the bound is
    // inclusive, and an index past the table (popn has 30) is passed over.
    th::Balance u;
    CHECK(!th::read_balance_file(text("#popn[30].Month 9\r\n#gbv.Nothing 1\r\n#staff[0].MinSalary 70\r\n"), u));
    CHECK(u.staff[0].min_salary == 150);
    CHECK(!th::read_balance_file(text("#popn[31].Month 9\r\n#staff[0].MinSalary 70\r\n"), u));
    CHECK(u.staff[0].min_salary == 150);
    // Lines are taken to end in CRLF: LF-only lines lose their last
    // character, and an empty one ends the file. Text before '#' is ignored.
    th::Balance lf;
    CHECK(th::read_balance_file(text("note #staff[0].MinSalary 70\n#gbv.SalaryAdd[5] -45\r\n\n#staff[2].MinSalary 9\n"),
                                lf));
    CHECK(lf.staff[0].min_salary == 7 && lf.gbv.salary_add[5] == -45 && lf.staff[2].min_salary == 95);
}

// The clock: a day every 50 turns, months and the year rolling over, the
// opening countdown, and three random draws a turn.
void test_game_clock(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto level = gd.load_level(1, &error);
    th::Balance settings;
    CHECK(level && th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error));
    if (!level)
        return;
    auto game = std::make_unique<th::Game>();
    game->start(*level, 1, settings, gd.anims(), gd.person_complexes());
    CHECK(game->balance == int(settings.towns[1].start_cash));
    CHECK(game->opening_countdown == settings.gbv.open_countdown && !game->open);

    uint32_t seed = game->world.seed;
    for (int i = 0; i < 49; i++)
        game->turn();
    CHECK(game->date.day == 2 && game->date.month == 0); // turn 50
    for (int i = 0; i < 3 * 49; i++)
        th::old_newrand(seed);
    CHECK(seed == game->world.seed);

    for (int i = 0; i < 30 * th::kTurnsPerDay; i++)
        game->turn();
    CHECK(game->date.day == 1 && game->date.month == 1);
    for (int i = 0; i < 334 * th::kTurnsPerDay; i++)
        game->turn();
    CHECK(game->date.day == 1 && game->date.month == 0 && game->date.year == 1);
    CHECK(game->open && game->opening_countdown == 0);

    game->paused = true;
    uint32_t turns = game->turn_count;
    game->turn();
    CHECK(game->turn_count == turns);
}

// Whether (x1, y1) can be reached from (x0, y0) in steps wall_in_way allows.
bool reachable(const th::World &w, int x0, int y0, int x1, int y1)
{
    std::vector<bool> seen(th::kMapCells);
    std::vector<int> queue = {x0 + y0 * th::kMapSize};
    seen[size_t(queue[0])] = true;
    for (size_t i = 0; i < queue.size(); i++) {
        int x = queue[i] % th::kMapSize;
        int y = queue[i] / th::kMapSize;
        if (x == x1 && y == y1)
            return true;
        for (uint32_t d = 0; d < 4; d++) {
            int next = x + th::kAroundXy[d][0] + (y + th::kAroundXy[d][1]) * th::kMapSize;
            if (!th::wall_in_way(w, x, y, d) && !seen[size_t(next)]) {
                seen[size_t(next)] = true;
                queue.push_back(next);
            }
        }
    }
    return false;
}

// A route found by wallhug: it ends at its end and each waypoint is in
// sight of the one before.
bool route_walkable(const th::World &w, const th::WallhugPath &path)
{
    th::Waypoint from = path.start;
    for (int i = 0; i < path.count; i++) {
        if (!th::line_of_sight(w, from, path.route[size_t(i)]))
            return false;
        from = path.route[size_t(i)];
    }
    return path.count > 0 && from.x == path.end.x && from.y == path.end.y;
}

void test_wall_in_way()
{
    auto w = std::make_unique<th::World>();
    CHECK(th::wall_in_way(*w, 5, 0, 0) && th::wall_in_way(*w, 0, 5, 3));
    CHECK(th::wall_in_way(*w, 127, 5, 1) && th::wall_in_way(*w, 5, 127, 2));
    CHECK(!th::wall_in_way(*w, 126, 5, 1) && !th::wall_in_way(*w, 5, 1, 0));
    for (uint32_t d = 0; d < 4; d++)
        CHECK(!th::wall_in_way(*w, 5, 5, d));

    w->map[5 + 5 * 128].wall_n = 1; // between (5, 4) and (5, 5)
    w->map[8 + 5 * 128].wall_w = 1; // between (7, 5) and (8, 5)
    CHECK(th::wall_in_way(*w, 5, 5, 0) && th::wall_in_way(*w, 5, 4, 2) && !th::wall_in_way(*w, 5, 5, 2));
    CHECK(th::wall_in_way(*w, 8, 5, 3) && th::wall_in_way(*w, 7, 5, 1) && !th::wall_in_way(*w, 8, 5, 1));

    // An object's footprint blocks steps onto and off it, unless it is also
    // a use cell (flag 2).
    w->map[10 + 10 * 128].flags = 1;
    CHECK(th::wall_in_way(*w, 10, 10, 2) && th::wall_in_way(*w, 9, 10, 1) && th::wall_in_way(*w, 10, 9, 2));
    w->map[10 + 10 * 128].flags = 3;
    CHECK(!th::wall_in_way(*w, 10, 10, 2) && !th::wall_in_way(*w, 9, 10, 1));

    CHECK(th::free_orthog_line(*w, 3, 5, 7, 5) && !th::free_orthog_line(*w, 9, 5, 3, 5));
    CHECK(th::free_orthog_line(*w, 5, 1, 5, 4) && !th::free_orthog_line(*w, 5, 8, 5, 1));
    CHECK(th::line_of_sight(*w, {1, 1}, {4, 9}) && !th::line_of_sight(*w, {5, 9}, {5, 1}));
}

// The wall-hugging path finder round a wall, into a closed box, and past
// its corner limit.
void test_wallhug()
{
    auto w = std::make_unique<th::World>();
    th::WallhugPath path;
    path.start = {10, 10};
    path.end = {20, 15};
    CHECK(th::wallhug_tricky(*w, path) == 15 && path.count == 1 && route_walkable(*w, path));

    // A wall from (15, 5) to (15, 20) across the way.
    for (int y = 5; y <= 20; y++)
        w->map[size_t(15 + y * 128)].wall_w = 1;
    path.start = {10, 12};
    path.end = {20, 12};
    CHECK(th::wallhug_trivial(*w, path) != th::kWallhugFailed && route_walkable(*w, path));
    int trivial = path.count;
    CHECK(th::wallhug_tricky(*w, path) != th::kWallhugFailed && route_walkable(*w, path));
    CHECK(path.count <= trivial);

    th::OrthonavPath nav;
    th::orthonav_init(*w, nav, 10, 12, 20, 12);
    CHECK(nav.count == std::min<int>(path.count, 4) && nav.x0 == 10 && nav.y1 == 12 && nav.x1 == 20);
    for (int i = 0; i < nav.count; i++)
        CHECK(nav.steps[size_t(i)].x == path.route[size_t(i)].x && nav.steps[size_t(i)].y == path.route[size_t(i)].y);

    // A closed box round (40, 40): no route in or out. A failed plan keeps
    // its old destination.
    for (int i = 39; i <= 41; i++) {
        w->map[size_t(i + 39 * 128)].wall_n = 1;
        w->map[size_t(i + 42 * 128)].wall_n = 1;
        w->map[size_t(39 + i * 128)].wall_w = 1;
        w->map[size_t(42 + i * 128)].wall_w = 1;
    }
    path.start = {10, 40};
    path.end = {40, 40};
    CHECK(th::wallhug_tricky(*w, path) == th::kWallhugFailed);
    path.start = {40, 40};
    path.end = {10, 40};
    CHECK(th::wallhug_tricky(*w, path) == th::kWallhugFailed);
    th::orthonav_init(*w, nav, 40, 40, 10, 40);
    CHECK(nav.count == 0 && nav.x1 == 20);

    // A wall down column 64 from the top to row 125, open below: found while
    // the wall is plain, but not when every cell along it has a spur: each
    // wall follower runs out of its 252 corners before getting round
    // (docs/BUGS.md).
    auto comb = std::make_unique<th::World>();
    for (int y = 0; y <= 125; y++)
        comb->map[size_t(64 + y * 128)].wall_w = 1;
    path.start = {50, 70};
    path.end = {80, 70};
    CHECK(th::wallhug_trivial(*comb, path) != th::kWallhugFailed && route_walkable(*comb, path));
    for (int y = 1; y <= 125; y++) {
        comb->map[size_t(63 + y * 128)].wall_n = 1;
        comb->map[size_t(64 + y * 128)].wall_n = 1;
    }
    CHECK(reachable(*comb, 50, 70, 80, 70));
    CHECK(th::wallhug_trivial(*comb, path) == th::kWallhugFailed);
}

// orthonav_whatnext steering a person round a wall one cell at a time, and
// its failures.
void test_orthonav_whatnext()
{
    auto w = std::make_unique<th::World>();
    for (int y = 5; y <= 20; y++)
        w->map[size_t(15 + y * 128)].wall_w = 1;
    th::Thing &p = w->things[1];
    p.type = th::kThingPatient;
    p.x.tile = 10;
    p.y.tile = 12;
    p.dest_x = 20;
    p.dest_y = 12;
    th::insert_node(*w, 1, p.x, p.y);
    int steps = 0;
    int dir = 0;
    while (steps < 200 && (dir = th::person_head_for(*w, 1)) >= 0 && dir < 4) {
        CHECK(!th::wall_in_way(*w, p.x.tile, p.y.tile, uint32_t(dir)));
        p.facing = uint8_t(dir);
        th::move_node(*w, 1, {0x80, uint8_t(p.x.tile + th::kAroundXy[size_t(dir)][0])},
                      {0x80, uint8_t(p.y.tile + th::kAroundXy[size_t(dir)][1])});
        steps++;
    }
    CHECK(dir == th::kNavArrived && p.x.tile == 20 && p.y.tile == 12);
    std::printf("orthonav: round the wall in %d steps\n", steps);

    // Walled in on its cell, or with the destination out of reach.
    w->map[size_t(30 + 30 * 128)].wall_n = 1;
    w->map[size_t(30 + 31 * 128)].wall_n = 1;
    w->map[size_t(30 + 30 * 128)].wall_w = 1;
    w->map[size_t(31 + 30 * 128)].wall_w = 1;
    th::move_node(*w, 1, {0x80, 30}, {0x80, 30});
    CHECK(th::person_head_for(*w, 1) == th::kNavWalledIn);
    th::move_node(*w, 1, {0x80, 50}, {0x80, 50});
    p.dest_x = 30;
    p.dest_y = 30;
    CHECK(th::person_head_for(*w, 1) == th::kNavNoPath);
}

// Every cell's list is well linked and holds the things on that cell.
bool cell_lists_consistent(const th::World &w)
{
    for (int i = 0; i < th::kMapCells; i++) {
        int prev = 0;
        for (int id = w.map[size_t(i)].first; id > 0; id = w.things[size_t(id)].cell_next) {
            const th::Thing &t = w.things[size_t(id)];
            if (t.cell_prev != prev || t.x.tile + t.y.tile * th::kMapSize != i)
                return false;
            prev = id;
        }
    }
    return true;
}

// With the game data: a patient walks on level 1 from where patients arrive
// (the level's entrance cell) in through the entrance door, and back out,
// in a fixed number of turns from the level's starting seed, keeping the
// cell lists consistent every turn; (40, 60), walled off from it, is out
// of reach.
void test_walk_level1(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto level = gd.load_level(1, &error);
    CHECK(level.has_value());
    if (!level)
        return;
    auto w = std::make_unique<th::World>();
    for (int run = 0; run < 2; run++) {
        th::start_level(*w, *level, gd.anims());
        uint16_t id = 3; // after the two parts of the entrance door
        CHECK(w->things[id].type == th::kThingFree);
        th::Thing &p = w->things[id];
        p.type = th::kThingPatient;
        p.speed = 0x20;
        p.anim_speed = 1;
        int entrance = level->entrances[0][0];
        p.x = {0x80, uint8_t(entrance % th::kMapSize)};
        p.y = {0x80, uint8_t(entrance / th::kMapSize)};
        th::insert_node(*w, id, p.x, p.y);
        th::play_frame(p, gd.anims(), th::kPatientAnims[0].walk);

        struct Leg {
            int x, y;
            th::WalkResult result;
            int turns;
        };
        const Leg legs[] = {
            {63, 78, th::WalkResult::Arrived, 305},
            {40, 60, th::WalkResult::NoPath, 1},
            {entrance % th::kMapSize, entrance / th::kMapSize, th::WalkResult::Arrived, 305},
        };
        uint32_t turn = 1;
        for (const Leg &leg : legs) {
            th::set_destination_coords(*w, id, leg.x + leg.y * th::kMapSize);
            int turns = 0;
            bool consistent = true;
            th::WalkResult r;
            do {
                r = th::walk_turn(*w, gd.anims(), id, ++turn);
                consistent &= cell_lists_consistent(*w);
                turns++;
            } while (r == th::WalkResult::Walking && turns < 5000);
            CHECK(consistent);
            CHECK(r == leg.result && turns == leg.turns);
            if (r == th::WalkResult::Arrived)
                CHECK(p.x.tile == leg.x && p.y.tile == leg.y && p.x.frac == 0x80 && p.y.frac == 0x80);
            std::printf("walk to %d,%d: result %d in %d turns\n", leg.x, leg.y, int(r), turns);
        }
        CHECK(w->seed == 0x2b6c015b);
    }
}

// Draws `n` values from `seed`, as the game does for those it discards.
uint32_t skip_draws(uint32_t seed, int n)
{
    for (int i = 0; i < n; i++)
        th::old_newrand(seed);
    return seed;
}

void test_create_an_illness()
{
    th::Balance b;
    b.gbv.hold_visual_months = 0;
    b.gbv.hold_visual_peep_count = 0;
    b.towns[1].ill_rate = 1;
    b.visuals.fill(0);
    b.visuals[3] = 5;
    b.non_visuals.fill(0);
    b.non_visuals[7] = 1;
    b.non_visuals[9] = 3;
    th::balance_control(b);
    th::ArrivalState s;

    // A visual illness: the ill_rate draw, then the visual one.
    uint32_t seed = 1234;
    CHECK(th::create_an_illness(seed, s, b, 1) == 5);
    CHECK(seed == skip_draws(1234, 2));
    // Not yet available: a non-visual one after both.
    b.visuals_available[3] = 1;
    seed = 1234;
    uint8_t ill = th::create_an_illness(seed, s, b, 1);
    CHECK(ill == 23 || ill == 25);
    CHECK(seed == skip_draws(1234, 3));
    // Held back early in the level: no visual draw.
    b.visuals_available[3] = 0;
    b.gbv.hold_visual_months = 1;
    seed = 1234;
    ill = th::create_an_illness(seed, s, b, 1);
    CHECK((ill == 23 || ill == 25) && seed == skip_draws(1234, 2));
    // No visual odds: only the non-visual draw, by weight 1 : 3.
    b.towns[1].ill_rate = 0;
    seed = 1234;
    int counts[2] = {};
    for (int i = 0; i < 4000; i++) {
        ill = th::create_an_illness(seed, s, b, 1);
        CHECK(ill == 23 || ill == 25);
        counts[ill == 25]++;
    }
    CHECK(seed == skip_draws(1234, 4000));
    CHECK(counts[0] > 850 && counts[0] < 1150);
}

void test_arrivals_level1(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto level = gd.load_level(1, &error);
    th::Balance settings;
    CHECK(level && th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error));
    if (!level)
        return;
    // Open from the start, so the rivals take patients too.
    settings.gbv.open_countdown = 0;
    const th::AnimData &anims = gd.anims();
    auto game = std::make_unique<th::Game>();
    for (int run = 0; run < 2; run++) {
        game->start(*level, 1, settings, anims, gd.person_complexes());
        const th::ArrivalState &state = game->arrivals;
        CHECK(state.visitors_per_month == 3 && state.peep_interval == 500); // popn[0]: month 0, change 3
        int ours = 0;
        bool valid = true;
        while (game->date.month < 6) {
            game->turn();
            for (uint16_t id : game->new_patients) {
                ours++;
                const th::Thing &t = game->world.things[id];
                int cell = t.x.tile + t.y.tile * th::kMapSize;
                const auto &doors = level->entrances[0];
                valid &= t.type == th::kThingPatient && t.owner == 0 && t.subtype < 15;
                // Set going in the turn it arrived: still at its entrance.
                valid &= std::find(doors.begin(), doors.end(), cell) != doors.end();
                valid &= t.sprite < anims.frames.size();
                valid &= t.layers[0] >= 1 && t.layers[0] <= 10 && t.layers[1] < 4 && t.layers[2] < 4 &&
                         t.layers[3] < 6 && t.layers[4] < 6;
                valid &= t.illness >= 2 && t.illness <= 35 && t.known_illness == 1 && t.treatment == 1;
                valid &= t.health >= 200 && t.health <= 1000 && t.cash >= 5000 && t.cash < 100000;
                valid &= t.speed == 0x20 && t.anim_speed == 1 && t.happiness == 100;
            }
        }
        CHECK(valid);
        CHECK(cell_lists_consistent(game->world));
        CHECK(ours == int(state.hospitals[0].visitors));
        std::printf("arrivals, level 1, 6 months: %d of %d patients ours, %d a month now, seed %#x\n", ours,
                    int(state.peeps_arrived), int(state.visitors_per_month), unsigned(game->world.seed));
        // Regression values, not checked against the game.
        CHECK(ours == 11 && state.peeps_arrived == 43 && state.visitors_per_month == 11);
        CHECK(game->world.seed == 0x70085bd9);
    }
}

// set_complex starts a complex on an idle person, queues two behind a
// running one (the second slot replaced by a third), and resumes one a step
// of action 24 left, queueing the new one.
void test_set_complex()
{
    auto w = std::make_unique<th::World>();
    th::Thing &p = w->things[1];
    p.type = th::kThingPatient;
    CHECK(th::set_complex(*w, 1, 20, 0));
    CHECK(p.complex_pc == 20 && p.complex_action == 20);
    CHECK(!th::set_complex(*w, 1, 30, 5 + 6 * th::kMapSize));
    CHECK(p.complex_queued == 30 && p.queued_arg_x == 5 && p.queued_arg_y == 6);
    CHECK(!th::set_complex(*w, 1, 40, 0));
    CHECK(!th::set_complex(*w, 1, 50, 7));
    CHECK(p.complex_queued2 == 50 && p.queued2_arg_x == 7 && p.complex_queued == 30);

    th::Thing &q = w->things[2];
    q.type = th::kThingPatient;
    q.complex_resume = 80;
    CHECK(th::set_complex(*w, 2, 60, 9));
    CHECK(q.complex_pc == 80 && q.complex_resume == 0 && q.complex_queued == 60 && q.queued_arg_x == 9);
}

// Runs patient `id` and the level's objects for up to `limit` turns from
// turn `turn` until the patient is gone. Returns the turns run; `opened`
// tells whether an entrance door started opening.
int run_patient(th::World &w, const th::AnimData &anims, const th::PersonComplexes &cx, uint16_t id, uint32_t &turn,
                int limit, bool &consistent, bool &opened)
{
    int turns = 0;
    bool exists = true;
    while (exists && turns < limit) {
        turn++;
        turns++;
        for (uint16_t i = 1; i < w.things.size(); i++) {
            if (i == id)
                exists = th::person_turn(w, anims, cx, i, turn);
            else if (w.things[i].type == th::kThingObject)
                th::sliding_door_turn(w, anims, i, turn);
            opened |= w.things[i].type == th::kThingObject && w.things[i].state == 0x16;
        }
        consistent &= cell_lists_consistent(w);
    }
    return turns;
}

// With the game data: P_COMP.LEX's complex 20 (walk, then leave the map);
// on level 1, with no reception, start_person_going sends a patient made at
// the entrance cell (where all four of level 1's entrances are) straight
// off the map, and one inside the entrance door out through it, the
// sliding doors opening for it, in a fixed number of turns.
void test_patient_leaves_level1(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    th::PersonComplexes cx;
    CHECK(th::load_person_complexes(dir, cx, &error));
    CHECK(cx.group.size() == 2000);
    CHECK(cx.at(20) == 12 && cx.at(21) == 15 && cx.at(22) == 0);
    auto level = gd.load_level(1, &error);
    CHECK(level.has_value());
    if (!level)
        return;
    const th::AnimData &anims = gd.anims();
    auto w = std::make_unique<th::World>();
    for (int run = 0; run < 2; run++) {
        th::start_level(*w, *level, anims);
        w->entrances = level->entrances;
        int entrance = level->entrances[0][0];
        struct Start {
            int x, y;
            int turns;
            bool opens;
        };
        // The second walk crosses indoor cells, where the game also runs the
        // patient's needs and reactions (not ported): a regression value.
        const Start starts[] = {
            {entrance % th::kMapSize, entrance / th::kMapSize, 2, false},
            {64, 76, 314, true},
        };
        uint32_t turn = 1;
        for (const Start &s : starts) {
            uint16_t id = 3; // after the two parts of the entrance door
            CHECK(w->things[id].type == th::kThingFree);
            th::Thing &p = w->things[id];
            p.type = th::kThingPatient;
            p.speed = 0x20;
            p.anim_speed = 1;
            p.x = {0x80, uint8_t(s.x)};
            p.y = {0x80, uint8_t(s.y)};
            th::insert_node(*w, id, p.x, p.y);
            th::play_frame(p, anims, th::kPatientAnims[0].walk);
            th::start_person_going(*w, anims, cx, id, turn);
            CHECK(p.state == th::kPsGotoDestination && p.complex_pc == 21);
            CHECK(p.dest_x + p.dest_y * th::kMapSize == entrance);
            bool consistent = true;
            bool opened = false;
            int turns = run_patient(*w, anims, cx, id, turn, 5000, consistent, opened);
            CHECK(consistent);
            CHECK(w->things[id].type == th::kThingFree);
            CHECK(w->map[size_t(entrance)].first == 0);
            CHECK(opened == s.opens);
            std::printf("patient from %d,%d left in %d turns, doors %s\n", s.x, s.y, turns,
                        opened ? "opened" : "closed");
            CHECK(turns == s.turns);
        }
        // The doors close again 50 turns after the patient has passed.
        for (int i = 0; i < 100; i++) {
            turn++;
            th::sliding_door_turn(*w, anims, 1, turn);
            th::sliding_door_turn(*w, anims, 2, turn);
        }
        for (uint16_t door : {1, 2}) {
            const th::Thing &d = w->things[door];
            CHECK(d.state == 0x14 && d.sprite == anims.starts[th::kObjects[d.subtype].anim_start + d.facing].frame);
        }
        std::printf("seed %#x\n", unsigned(w->seed));
        CHECK(w->seed == 0xebe44801);
    }
}

th::MenuEvent menu_step(th::MenuBar &bar, int x, int y, bool held, bool released, const th::MenuContext &ctx = {})
{
    th::UiMouse m;
    m.x = x;
    m.y = y;
    m.left_held = held;
    m.left_released = released;
    return th::menu_frame(bar, m, ctx);
}

size_t drawn_pixels(const th::MenuBar &bar, const th::Palette &palette)
{
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    th::draw_menu_bar(layer, bar);
    auto argb = layer.to_argb(palette);
    return size_t(std::count_if(argb.begin(), argb.end(), [](uint32_t p) { return p != 0; }));
}

// With the game data: the menu bar built from English pulldown_text
// (setup_menus), opened, dragged through and chosen from as process_menus
// does, and drawn with the PULLDV art. Every language builds.
void test_menu_bar(const char *dir)
{
    std::string error;
    for (int language = 0; language < 6; language++) {
        th::LanguageText lang;
        th::MenuBar bar;
        CHECK(th::load_language(dir, language, lang, &error));
        CHECK(bar.load(dir, lang, &error));
        CHECK(bar.count == 6 && bar.subcount == 12);
    }
    th::GameData gd;
    CHECK(gd.load(dir, &error, th::kEnglish));
    th::MenuBar bar;
    CHECK(bar.load(dir, gd.text(), &error));
    if (bar.count != 6)
        return;
    const char *titles[] = {"  FILE  ", "  OPTIONS  ", "  DISPLAY  ", "  CHARTS  ", "  DEBUG  "};
    for (int i = 1; i < 6; i++) {
        CHECK(bar.text[size_t(bar.pulldowns[size_t(i)].text)] == titles[i - 1]);
        CHECK(bar.pulldowns[size_t(i)].active == (i != 5));
    }
    CHECK(bar.line_height == 14 && bar.char_width == 9);
    CHECK(bar.pulldowns[1].count == 4 && bar.pulldowns[2].count == 9 && bar.pulldowns[4].count == 9);
    // Load, Save under File; the volumes and speed under Options.
    const int parents[] = {1, 1, 2, 2, 2, 2};
    for (int s = 6; s < 12; s++)
        CHECK(bar.pulldowns[size_t(s)].parent == parents[s - 6]);
    CHECK(bar.pulldowns[2].submenu[7] == 11 && bar.pulldowns[11].count == 5 && bar.pulldowns[11].y == 14 + 7 * 14);

    // Hidden until the mouse reaches the top 22 lines.
    menu_step(bar, 300, 200, false, false);
    CHECK(!bar.active && drawn_pixels(bar, gd.palette()) == 0);
    menu_step(bar, 300, 21, false, false);
    CHECK(bar.active && bar.open == 0);
    size_t bar_only = drawn_pixels(bar, gd.palette());
    CHECK(bar_only >= size_t(th::kUiWidth) * 16);

    // Hold on File, drag down to Quit and release.
    auto ev = menu_step(bar, 20, 5, true, false);
    CHECK(bar.open == 1 && ev.sound == th::MenuSound::Highlight);
    menu_step(bar, 20, 14 + 3 * 14 + 5, true, false);
    CHECK(bar.open == 1 && bar.active);
    CHECK(drawn_pixels(bar, gd.palette()) > bar_only);
    ev = menu_step(bar, 20, 14 + 3 * 14 + 5, false, true);
    CHECK(ev.action == th::MenuAction::Quit && ev.sound == th::MenuSound::Select);
    CHECK(bar.open == 0 && bar.active); // the release frame still belongs to the bar
    menu_step(bar, 20, 200, false, false);
    CHECK(!bar.active);

    // Options > Game speed > each speed.
    const int options_x = 8 * 9 + 5;
    const int submenu_x = 8 * 9 + bar.pulldowns[2].width * 9 + 10 + 5;
    for (int item = 0; item < 5; item++) {
        menu_step(bar, options_x, 5, true, false);
        CHECK(bar.open == 2);
        menu_step(bar, options_x, 14 + 7 * 14 + 5, true, false);
        CHECK(bar.open == 11);
        int y = 14 + 7 * 14 + item * 14 + 5;
        menu_step(bar, submenu_x, y, true, false);
        CHECK(bar.open == 11);
        ev = menu_step(bar, submenu_x, y, false, true);
        CHECK(ev.action == th::MenuAction::Speed && ev.value == 10 * (item + 1));
    }
    // Refused while paused; Options > Sound is not.
    th::MenuContext paused;
    paused.paused = true;
    menu_step(bar, options_x, 5, true, false, paused);
    menu_step(bar, options_x, 14 + 7 * 14 + 5, true, false, paused);
    menu_step(bar, submenu_x, 14 + 7 * 14 + 5, true, false, paused);
    ev = menu_step(bar, submenu_x, 14 + 7 * 14 + 5, false, true, paused);
    CHECK(ev.action == th::MenuAction::None && ev.sound == th::MenuSound::Wrong);
    menu_step(bar, options_x, 5, true, false, paused);
    ev = menu_step(bar, options_x, 14 + 5, false, true, paused);
    CHECK(ev.action == th::MenuAction::ToggleSound);

    // Back from a submenu to its parent; released outside the menus.
    menu_step(bar, options_x, 5, true, false);
    menu_step(bar, options_x, 14 + 3 * 14 + 5, true, false);
    CHECK(bar.open == 8);
    menu_step(bar, options_x, 14 + 5, true, false);
    CHECK(bar.open == 2);
    ev = menu_step(bar, 600, 300, false, true);
    CHECK(ev.action == th::MenuAction::None && bar.open == 0);

    // File > Save > Game 3; a modal requester hides the bar.
    menu_step(bar, 20, 5, true, false);
    menu_step(bar, 20, 14 + 14 + 5, true, false);
    CHECK(bar.open == 7);
    const int save_x = bar.pulldowns[1].width * 9 + 10 + 5;
    ev = menu_step(bar, save_x, 28 + 2 * 14 + 5, false, true);
    CHECK(ev.action == th::MenuAction::Save && ev.value == 3);
    th::MenuContext modal;
    modal.disabled = true;
    menu_step(bar, 20, 5, true, false, modal);
    CHECK(!bar.active && bar.open == 0);
}

// The requester sheets share one numbering: each holds its own requester's
// sprites among 1 x 1 placeholders.
void test_requester_art(const char *dir)
{
    th::RequesterArt art;
    std::string error;
    CHECK(art.load(dir, &error));
    const th::SpriteSheet &yes_no = art.sheet(int(th::Requester::YesNo));
    CHECK(yes_no.size() == 415);
    CHECK(yes_no.at(357).width == 183 && yes_no.at(357).height == 22);
    CHECK(yes_no.at(362).width == 93 && yes_no.at(362).height == 53);
    CHECK(art.sheet(int(th::Requester::Jukebox)).at(389).width == 30);
    CHECK(art.sheet(7).size() == 0);
    CHECK(!art.font().empty());
}

void test_panel_buttons(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    CHECK(th::panel_top(gd) == 432);
    const int xs[] = {20, 40, 41, 100, 220, 260, 300, 340, 380, 420, 460, 500, 540, 580, 620};
    const int icons[] = {0, 0, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    for (size_t i = 0; i < std::size(xs); i++)
        CHECK(th::panel_icon_at(gd, true, xs[i], 460) == icons[i]);
    CHECK(th::panel_icon_at(gd, true, 220, 431) == th::kNoPanelIcon);
    CHECK(th::panel_icon_at(gd, false, 220, 470) == th::kIconBuildRoom);

    th::PanelState state;
    th::UiMouse m;
    m.x = 220;
    m.y = 300;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::None && !state.raised);
    m.y = 460;
    m.left_pressed = m.left_held = true;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::None);
    CHECK(state.raised && state.pressed == th::kIconBuildRoom);
    m.left_pressed = false;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::None && state.pressed == th::kIconBuildRoom);
    m.left_held = false;
    m.left_released = true;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::BuildRoom);
    CHECK(state.pressed == th::kNoPanelIcon);
    m.left_released = false;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::None);

    m.x = 100;
    m.left_pressed = m.left_released = true;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::Refused);
    m.left_pressed = m.left_released = false;
    m.right_pressed = true;
    m.x = 20;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::BankStatement);
    m.x = 340;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::StaffManagement);
    m.x = 620;
    CHECK(th::panel_frame(state, m, gd, false) == th::PanelAction::Refused);
    CHECK(th::panel_frame(state, m, gd, true) == th::PanelAction::None && !state.raised);
    CHECK(th::panel_frame(state, m, gd, false, false) == th::PanelAction::None && state.raised);

    // The raised set with a pressed icon draws the pressed sprite.
    th::UiLayer a, b;
    a.resize(th::kUiWidth, th::kUiHeight);
    b.resize(th::kUiWidth, th::kUiHeight);
    a.clear();
    b.clear();
    th::PanelState raised;
    raised.raised = true;
    th::draw_panel_ui(a, raised, nullptr, gd);
    raised.pressed = th::kIconTownMap;
    th::draw_panel_ui(b, raised, nullptr, gd);
    std::vector<uint32_t> pa = a.to_argb(gd.palette()), pb = b.to_argb(gd.palette());
    CHECK(pa != pb);
    CHECK(std::equal(pa.begin(), pa.begin() + 432 * th::kUiWidth, pb.begin()));
}

// The yes/no requester drawn from its sheet, answered by clicks, Escape and
// a right click outside it.
void test_yes_no_requester(const char *dir)
{
    th::GameData gd;
    th::RequesterArt art;
    std::string error;
    CHECK(gd.load(dir, &error) && art.load(dir, &error));
    th::Interface ui;
    ui.open_requester(th::Requester::BuildRoom);
    ui.open_yes_no(gd.text().at(th::TextSection::YesNo, 0));
    CHECK(ui.modal() && ui.requester() == th::Requester::YesNo && ui.window_count() == 1);

    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    th::draw_interface(layer, ui, art);
    std::vector<uint32_t> argb = layer.to_argb(gd.palette());
    const th::Window &win = ui.window(int(th::Requester::YesNo));
    int inside = 0, outside = 0;
    for (int y = 0; y < th::kUiHeight; y++) {
        for (int x = 0; x < th::kUiWidth; x++) {
            bool in = x >= win.x && x < win.x + win.width && y >= win.y && y < win.y + win.height;
            if (argb[size_t(y * th::kUiWidth + x)])
                (in ? inside : outside)++;
        }
    }
    CHECK(inside > 180 * 190 && outside == 0);

    // Yes: held while pressed, answered on release; the requester stays.
    th::UiMouse m;
    m.x = win.x + 92 + 46;
    m.y = win.y + 148 + 26;
    th::InterfaceEvents ev = ui.frame(m);
    CHECK(ev.mouse_on_window && ev.yes_no == th::YesNoAnswer::None);
    m.left_pressed = m.left_held = true;
    ev = ui.frame(m);
    CHECK(ev.yes_no == th::YesNoAnswer::None && (win.buttons[11].flags & th::kButtonHeld));
    th::UiLayer held;
    held.resize(th::kUiWidth, th::kUiHeight);
    held.clear();
    th::draw_interface(held, ui, art);
    CHECK(held.to_argb(gd.palette()) != argb);
    m.left_pressed = m.left_held = false;
    m.left_released = true;
    ev = ui.frame(m);
    CHECK(ev.yes_no == th::YesNoAnswer::Yes && ev.triggered_button == 12);
    CHECK(ui.requester() == th::Requester::YesNo);
    ui.close_yes_no(false);
    CHECK(!ui.modal() && ui.window_count() == 0 && ui.requester() == th::Requester::None);

    // No reopens the requester the question covered.
    ui.open_requester(th::Requester::BuildRoom);
    ui.open_yes_no("?");
    m = {};
    m.x = win.x + 40;
    m.y = win.y + 170;
    m.left_pressed = m.left_held = true;
    ui.frame(m);
    m.left_pressed = m.left_held = false;
    m.left_released = true;
    ev = ui.frame(m);
    CHECK(ev.yes_no == th::YesNoAnswer::No && !ui.modal());
    CHECK(ui.requester() == th::Requester::BuildRoom && ui.window_count() == 1);
    ui.close_requester(th::Requester::BuildRoom);

    ui.open_yes_no("?");
    m = {};
    m.x = 10;
    m.y = 10;
    CHECK(ui.frame(m, true).yes_no == th::YesNoAnswer::No && !ui.modal());
    ui.open_yes_no("?");
    m.right_pressed = true;
    m.x = win.x + 10;
    m.y = win.y + 10;
    CHECK(ui.frame(m).yes_no == th::YesNoAnswer::None && ui.modal());
    m.x = 10;
    CHECK(ui.frame(m).yes_no == th::YesNoAnswer::No && ui.window_count() == 0);
}

// Hire staff (milestone 4 step 3): starts level 1 with its job market made
// as setup_new_level makes it.
struct HireFixture {
    th::GameData gd;
    std::optional<th::Level> level;
    th::Balance settings;
    std::unique_ptr<th::Game> game = std::make_unique<th::Game>();
    th::StaffMarket market;
    // The simulation's generator after the level started.
    uint32_t seed = 0;

    bool start(const char *dir)
    {
        std::string error;
        if (!gd.load(dir, &error))
            return false;
        level = gd.load_level(1, &error);
        if (!level || !th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error))
            return false;
        // Game::start makes the first applicants, as the game does.
        game->staff_market.ref_counts = th::reference_counts(gd.text());
        game->start(*level, 1, settings, gd.anims(), gd.person_complexes());
        seed = game->world.seed;
        market = game->staff_market;
        return true;
    }
};

// Carries out `e` as the app does: the interface's part, then the game's.
void hire_command(HireFixture &f, th::HireStaff &hire, th::Interface &ui, th::Event e)
{
    f.game->event = e;
    th::hire_staff_event(hire, ui, f.game->world, f.game->event);
    f.game->process_turn();
    th::hire_staff_after_turn(hire, f.game->world);
}

// Presses and releases button `button` (counted from 1) of the hire staff
// requester; returns what the interface did on the release.
th::InterfaceEvents click_hire_button(th::Interface &ui, int button)
{
    const th::Window &win = ui.window(int(th::Requester::HireStaff));
    const th::WindowButton &b = win.buttons[size_t(button - 1)];
    th::UiMouse m;
    m.x = win.x + b.x + b.width / 2;
    m.y = win.y + b.y + b.height / 2;
    m.left_pressed = m.left_held = true;
    ui.frame(m);
    m.left_pressed = m.left_held = false;
    m.left_released = true;
    return ui.frame(m);
}

void test_applicants_level1(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    const th::World &w = f.game->world;
    const th::Balance::Gbv &gbv = f.settings.gbv;
    CHECK(f.market.ref_counts[0] > 0 && f.market.ref_counts[1] > 0 && f.market.ref_counts[2] > 0);
    // FULL01.SAM staff_levels[0]: 8 nurses, 8 doctors, 3 handymen, 8
    // receptionists.
    CHECK(th::count_applicants(w, th::kStaffNurse) == 8 && th::count_applicants(w, th::kStaffDoctor) == 8);
    CHECK(th::count_applicants(w, th::kStaffHandyman) == 3 && th::count_applicants(w, th::kStaffReceptionist) == 8);
    bool valid = true;
    long wages = 0;
    int psychiatrists = 0, juniors = 0;
    for (const th::Thing &t : w.things) {
        if (t.type != th::kThingStaff)
            continue;
        valid &= t.state == th::kStaffApplicant && t.owner == th::kNoOwner && t.cell_next == 0 && t.cell_prev == 0;
        valid &= t.hire_expiry >= 1 + 2500 && t.hire_expiry < 1 + 5000;
        valid &= t.initial >= 'A' && t.initial <= 'Z' && t.surname_a < 33 && t.surname_b < 26;
        valid &= t.ref_general < f.market.ref_counts[0] && t.ref_good < f.market.ref_counts[1] &&
                 t.ref_bad < f.market.ref_counts[2];
        valid &= t.face_head < 18 && t.face_body < 18 && t.face_legs < 18;
        valid &= t.work_delay >= 1 && t.work_delay <= 10 && t.happiness <= 100;
        // The wage: ability / divisor + the kind's minimum + the skills'
        // additions, between the game's bounds for the kind.
        int min_wage = f.settings.staff[t.subtype].min_salary;
        int max_wage = min_wage + 1000 / gbv.salary_ability_divisor;
        min_wage += 30 / gbv.salary_ability_divisor;
        for (size_t i = 3; i < 9; i++) {
            min_wage += std::min<int>(gbv.salary_add[i], 0);
            max_wage += std::max<int>(gbv.salary_add[i], 0);
        }
        valid &= t.salary == t.wage_demand && t.salary >= min_wage && t.salary <= max_wage;
        if (t.subtype == th::kStaffDoctor) {
            valid &= (t.skills & th::kSkillDoctor) != 0;
            bool junior = t.skills & th::kSkillJunior;
            valid &= junior ? t.ability >= 30 && t.ability <= gbv.doctor_threshold
                            : t.ability >= gbv.doctor_threshold && t.ability <= gbv.consultant_threshold;
            psychiatrists += (t.skills & th::kSkillPsychiatrist) != 0;
            juniors += junior;
            // FULL01.SAM: no surgeons, researchers or consultants yet.
            valid &= !(t.skills & (th::kSkillSurgeon | th::kSkillResearcher | th::kSkillConsultant));
        } else {
            valid &= t.ability >= 30 && t.ability <= 1000;
        }
        wages += t.salary;
    }
    CHECK(valid);
    std::printf("applicants, level 1: wages %ld, %d psychiatrists, %d juniors, staff seed %#x\n", wages, psychiatrists,
                juniors, unsigned(f.market.seed));
    // Regression values, not checked against the game.
    CHECK(wages == 2936 && psychiatrists == 2 && juniors == 1 && f.market.seed == 0x47138585);
    // No draws from the simulation's generator.
    CHECK(w.seed == f.seed);
    // Again in month 0: everyone wanted is there, so no one is made.
    th::StaffMarket again = f.market;
    th::create_job_market(f.game->world, again, f.settings, 1, f.game->turn_count);
    CHECK(again.seed == f.market.seed && th::count_applicants(w, th::kStaffNurse) == 8);
}

void test_hire_receptionist(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::World &w = f.game->world;
    th::Interface ui;
    th::HireStaff hire;

    hire_command(f, hire, ui, {th::EV_OPEN_HIRE_STAFF});
    CHECK(ui.requester() == th::Requester::HireStaff && hire.mode == 1);
    // Hire is refused before a kind is chosen.
    CHECK(th::hire_staff_click(hire, click_hire_button(ui, 0x2b), *f.game).type == th::EV_NONE);

    th::Event e = th::hire_staff_click(hire, click_hire_button(ui, 0x29), *f.game);
    CHECK(e.type == th::EV_HIRE_SELECT_KIND && e.value == th::kStaffReceptionist);
    hire_command(f, hire, ui, e);
    uint16_t first = th::alter_applicant(w, true, 0, th::kStaffReceptionist);
    CHECK(hire.mode == 2 && hire.shown == first && first != 0);
    // Previous from the first is refused; next pages on.
    CHECK(th::hire_staff_click(hire, click_hire_button(ui, 0x2a), *f.game).type == th::EV_NONE);
    e = th::hire_staff_click(hire, click_hire_button(ui, 0x2d), *f.game);
    CHECK(e.type == th::EV_HIRE_NEXT_APPLICANT && e.value == 1);
    hire_command(f, hire, ui, e);
    uint16_t second = hire.shown;
    CHECK(second > first && w.things[second].subtype == th::kStaffReceptionist);

    e = th::hire_staff_click(hire, click_hire_button(ui, 0x2b), *f.game);
    CHECK(e.type == th::EV_MARK_EMPLOYING && e.arg == second && e.value == th::kStaffReceptionist);
    hire_command(f, hire, ui, e);
    CHECK(ui.window_count() == 0 && ui.requester() == th::Requester::None);
    CHECK(hire.placing() && hire.held == second && w.things[second].state == th::kStaffHeld);
    CHECK(th::count_applicants(w, th::kStaffReceptionist) == 8);

    // Off the hospital's land the click does nothing.
    int outside = f.level->entrances[0][0];
    CHECK(th::place_staff_click(hire, w, outside).type == th::EV_NONE && !hire.place_ok);
    int cell = -1;
    int start = f.level->camera_start[0];
    for (int r = 0; r < 20 && cell < 0; r++)
        for (int dy = -r; dy <= r && cell < 0; dy++)
            for (int dx = -r; dx <= r && cell < 0; dx++) {
                int c = start + dx + dy * th::kMapSize;
                if (c >= 0 && c < th::kMapCells && w.map[size_t(c)].first == 0 && th::staff_place_ok(w, c))
                    cell = c;
            }
    CHECK(cell >= 0);
    if (cell < 0)
        return;
    int balance = f.game->balance;
    int32_t salary = w.things[second].salary;
    e = th::place_staff_click(hire, w, cell);
    CHECK(e.type == th::EV_PLACE_STAFF && e.value == cell && e.arg == second && hire.place_ok);
    hire_command(f, hire, ui, e);
    const th::Thing &s = w.things[second];
    CHECK(!hire.placing());
    CHECK(s.type == th::kThingStaff && s.owner == 0 && s.state == th::kStaffReceptionistFindsReception);
    CHECK(s.x.tile + s.y.tile * th::kMapSize == cell && s.x.frac == 0x80 && s.y.frac == 0x80);
    CHECK(s.hire_expiry == 0 && s.hire_time == int16_t(f.game->turn_count) && s.speed != 0);
    CHECK(f.game->balance == balance - salary * 12 / 10);
    CHECK(th::count_applicants(w, th::kStaffReceptionist) == 7);
    bool listed = false;
    for (int id = w.map[size_t(cell)].first; id > 0; id = w.things[size_t(id)].cell_next)
        listed |= id == second;
    CHECK(listed && cell_lists_consistent(w));
    // With no reception desk, she goes to wander (and, not ported further,
    // stands).
    th::staff_turn(w, f.gd.anims(), second, f.game->turn_count);
    CHECK(s.state == th::kStaffWanders && s.substate == 6);
    CHECK(s.sprite == f.gd.anims().starts[th::kStaffAnims[th::kStaffReceptionist].stand + s.facing].frame);

    // A nurse taken to place, then dropped: back on the list.
    hire_command(f, hire, ui, {th::EV_OPEN_HIRE_STAFF});
    hire_command(f, hire, ui, {th::EV_HIRE_SELECT_KIND, th::kStaffNurse});
    uint16_t nurse = hire.shown;
    hire_command(f, hire, ui, th::hire_staff_click(hire, click_hire_button(ui, 0x2b), *f.game));
    CHECK(hire.held == nurse && w.things[nurse].state == th::kStaffHeld);
    hire_command(f, hire, ui, th::hire_staff_cancel(hire));
    CHECK(!hire.placing() && w.things[nurse].state == th::kStaffApplicant && ui.window_count() == 0);

    // An applicant withdraws once past their expiry.
    uint32_t expiry = w.things[nurse].hire_expiry;
    th::staff_turn(w, f.gd.anims(), nurse, expiry);
    CHECK(w.things[nurse].type == th::kThingStaff);
    th::staff_turn(w, f.gd.anims(), nurse, expiry + 1);
    CHECK(w.things[nurse].type == th::kThingFree && th::count_applicants(w, th::kStaffNurse) == 7);
}

void write_ppm(const char *path, const std::vector<uint32_t> &argb, int width, int height)
{
    if (FILE *out = std::fopen(path, "wb")) {
        std::fprintf(out, "P6\n%d %d\n255\n", width, height);
        for (uint32_t p : argb) {
            unsigned char rgb[3] = {uint8_t(p >> 16), uint8_t(p >> 8), uint8_t(p)};
            std::fwrite(rgb, 1, 3, out);
        }
        std::fclose(out);
    }
}

void test_hire_requester_image(const char *dir)
{
    HireFixture f;
    th::RequesterArt art;
    th::HireStaffArt hire_art;
    std::string error;
    CHECK(f.start(dir) && art.load(dir, &error) && hire_art.load(dir, &error));
    if (!f.level)
        return;
    CHECK(hire_art.faces().size() == 65 * (29 + 24 + 22) * 18 && !hire_art.count_font().empty());
    CHECK(art.sheet(int(th::Requester::HireStaff)).size() > 0x128);
    th::Interface ui;
    th::HireStaff hire;
    hire_command(f, hire, ui, {th::EV_OPEN_HIRE_STAFF});
    const th::Window &win = ui.window(int(th::Requester::HireStaff));

    auto render = [&](const char *name) {
        th::UiLayer layer;
        layer.resize(th::kUiWidth, th::kUiHeight);
        layer.clear();
        th::draw_interface(layer, ui, art);
        th::draw_hire_staff(layer, hire, ui, art, hire_art, f.game->world, f.gd.text());
        std::vector<uint32_t> argb = layer.to_argb(f.gd.palette());
        int inside = 0, outside = 0;
        for (int y = 0; y < th::kUiHeight; y++)
            for (int x = 0; x < th::kUiWidth; x++)
                if (argb[size_t(y * th::kUiWidth + x)])
                    (x >= win.x && x < win.x + win.width && y >= win.y && y < win.y + win.height ? inside : outside)++;
        CHECK(inside > win.width * win.height * 9 / 10 && outside == 0);
        if (const char *scratch = std::getenv("TH_TEST_SCRATCH"))
            write_ppm((std::string(scratch) + "/" + name).c_str(), argb, th::kUiWidth, th::kUiHeight);
        return argb;
    };
    std::vector<uint32_t> blank = render("hire_blank.ppm");
    hire_command(f, hire, ui, {th::EV_HIRE_SELECT_KIND, th::kStaffDoctor});
    std::vector<uint32_t> doctor = render("hire_doctor.ppm");
    CHECK(doctor != blank);
    // The face is the faces file's pixels, inside its border.
    const th::Thing &d = f.game->world.things[hire.shown];
    size_t head = size_t(d.face_head) * 65 * 29;
    uint32_t want = f.gd.palette()[hire_art.faces()[head + 65 * 10 + 30]];
    CHECK(doctor[size_t((win.y + 128 - 75 - 3 + 10) * th::kUiWidth + win.x + 157 + 3 + 30)] == want);
    hire_command(f, hire, ui, {th::EV_HIRE_SELECT_KIND, th::kStaffHandyman});
    CHECK(render("hire_handyman.ppm") != doctor);

    // The held applicant under the pointer, coloured where they may be put
    // down and greyed where not.
    hire_command(f, hire, ui, th::hire_staff_click(hire, click_hire_button(ui, 0x2b), *f.game));
    CHECK(hire.placing());
    th::Surface view, scratch, base;
    view.resize(320, 240);
    scratch.resize(320, 240);
    scratch.fill(th::kMaskColour);
    int start = f.level->camera_start[0];
    th::Point centre = th::cell_to_world(start % th::kMapSize, start / th::kMapSize);
    th::Point camera = {centre.x - 160, centre.y - 120};
    th::draw_world(view, {0, 0, 320, 240}, camera, f.game->world, f.gd, nullptr, scratch);
    base = view;
    th::update_staff_cursor(hire, f.game->world, start);
    th::draw_held_staff(view, {0, 0, 320, 240}, camera, hire, f.game->world, start, f.gd, scratch);
    int changed = 0;
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            changed += view.row(y)[x] != base.row(y)[x];
    CHECK(changed > 100);
    th::Surface coloured = view;
    view = base;
    hire.place_ok = false;
    th::draw_held_staff(view, {0, 0, 320, 240}, camera, hire, f.game->world, start, f.gd, scratch);
    std::array<uint8_t, 256> grey = th::grey_remap(f.gd.palette());
    // All but the see-through pixels (shadows) are the coloured ones greyed.
    int greyed = 0, drawn = 0, differs = 0;
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            if (coloured.row(y)[x] != base.row(y)[x]) {
                greyed += view.row(y)[x] == grey[coloured.row(y)[x]];
                drawn++;
                differs += view.row(y)[x] != coloured.row(y)[x];
            }
    CHECK(drawn > 100 && greyed > drawn * 8 / 10 && differs > 0);
    bool clean = true;
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            clean &= scratch.row(y)[x] == th::kMaskColour;
    CHECK(clean);
}

// Furnishing level 1's corridor: the buy objects requester, buying and
// selling, and placing a reception desk through the commands.
void test_furnish_corridor(const char *dir)
{
    th::GameData gd;
    th::RequesterArt art;
    std::string error;
    CHECK(gd.load(dir, &error) && art.load(dir, &error));
    auto level = gd.load_level(1, &error);
    th::Balance settings;
    CHECK(level && th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error));
    if (!level)
        return;
    auto game = std::make_unique<th::Game>();
    game->start(*level, 1, settings, gd.anims(), gd.person_complexes());
    th::World &world = game->world;
    th::ObjectPlacing &p = world.placing;
    th::Interface ui;
    th::ObjectUi oui;
    th::init_object_ui(oui, gd.palette());
    const int desk = 11, bench = 4;
    const int desk_cost = settings.objects[desk].start_cost;
    CHECK(desk_cost > 0);

    th::furnish_corridor(*game);
    game->process_turn();
    CHECK(p.requester == th::ObjectRequester::BuyObjects && p.menu_disabled && p.arg == 1);
    std::vector<int> offered;
    for (uint8_t kind : p.offered)
        if (kind)
            offered.push_back(kind);
    // Reception desk, bench, drinks machine, fire extinguisher, radiator and
    // plant; level 1 makes no litter bombs available.
    CHECK((offered == std::vector<int>{11, 4, 7, 43, 44, 45}));
    CHECK(p.item == desk);
    th::sync_object_requesters(ui, *game);
    CHECK(ui.requester() == th::Requester::BuyObjects && ui.window_count() == 1);
    const th::Window &win = ui.window(int(th::Requester::BuyObjects));

    // A press then a release at (x, y), then the turn's commands.
    auto click = [&](int x, int y, bool right) {
        th::UiMouse m;
        m.x = x;
        m.y = y;
        (right ? m.right_pressed : m.left_pressed) = true;
        (right ? m.right_held : m.left_held) = true;
        th::object_requesters_input(ui, oui, *game, ui.frame(m), m, false, false);
        m = {};
        m.x = x;
        m.y = y;
        (right ? m.right_released : m.left_released) = true;
        th::object_requesters_input(ui, oui, *game, ui.frame(m), m, false, false);
        game->process_turn();
        th::sync_object_requesters(ui, *game);
    };
    int balance = game->balance;
    click(win.x + 331 + 8, win.y + 37 + 8, false); // the desk's plus
    CHECK(p.buy_count[desk] == 1 && game->balance == balance);
    click(win.x + 200, win.y + 55 + 9, true); // sell a bench: none bought
    CHECK(p.buy_count[bench] == 0 && p.item == bench);
    click(win.x + 200, win.y + 55 + 9, false); // the bench's entry: buys
    CHECK(p.buy_count[bench] == 1);
    click(win.x + 287 + 8, win.y + 56 + 8, false); // its minus
    CHECK(p.buy_count[bench] == 0);
    game->balance = desk_cost + desk_cost / 2;
    click(win.x + 331 + 8, win.y + 37 + 8, false); // a second desk: too dear
    CHECK(p.buy_count[desk] == 1);
    game->balance = balance;

    // The requester's own drawing: text and the selected kind's picture.
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    th::draw_interface(layer, ui, art);
    std::vector<uint32_t> plain = layer.to_argb(gd.palette());
    th::draw_object_requesters(layer, ui, oui, *game, art, gd);
    std::vector<uint32_t> drawn = layer.to_argb(gd.palette());
    int changed = 0;
    for (size_t i = 0; i < drawn.size(); i++)
        changed += drawn[i] != plain[i];
    CHECK(changed > 1000);

    click(win.x + 156 + 50, win.y + 240 + 10, false); // accept
    CHECK(p.placing && p.kind == desk && p.mode == 2 && !p.menu_disabled);
    CHECK(p.requester == th::ObjectRequester::EditRoom && ui.requester() == th::Requester::EditRoom);

    // The first cell the desk may go on.
    int tile = -1;
    for (int t = 0; t < th::kMapCells && tile < 0; t++) {
        th::update_placement(world, p, t);
        if (p.ok && p.tile == t)
            tile = t;
    }
    CHECK(tile >= 0);
    if (tile < 0)
        return;
    th::update_placement(world, p, 0); // public land, off the hospital
    CHECK(!p.ok);
    game->event = {th::EV_PLACE_OBJECT, 0, 0, uint16_t(desk | p.dir << 8)};
    game->process_turn();
    CHECK(world.first_of_kind[desk] == 0 && game->balance == balance && p.buy_count[desk] == 1);

    // The desk following the pointer, in colour and, where it may not go,
    // in greys.
    th::UiMouse m;
    th::placement_mouse(oui, *game, tile, m, true);
    CHECK(oui.cursor_shown && p.ok);
    th::Surface view, base;
    view.resize(320, 240);
    view.fill(0);
    base = view;
    th::Point w = th::cell_to_world(tile % th::kMapSize, tile / th::kMapSize);
    th::Point camera = {w.x - 160, w.y - 120};
    th::draw_placement(view, {0, 0, 320, 240}, camera, p, oui, gd);
    int coloured = 0;
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            coloured += view.row(y)[x] != 0;
    CHECK(coloured > 200);
    th::Surface colour_view = view;
    view = base;
    p.ok = false;
    th::draw_placement(view, {0, 0, 320, 240}, camera, p, oui, gd);
    int greyed = 0;
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            greyed += colour_view.row(y)[x] && view.row(y)[x] == oui.grey[colour_view.row(y)[x]];
    CHECK(greyed > coloured * 8 / 10);
    p.redraw = true;

    // A left click places it: paid for, on the map and in the desks' list.
    m.left_pressed = m.left_held = true;
    th::placement_mouse(oui, *game, tile, m, true);
    CHECK(game->event.type == th::EV_PLACE_OBJECT && game->event.value == tile);
    game->process_turn();
    th::sync_object_requesters(ui, *game);
    CHECK(game->balance == balance - desk_cost);
    uint16_t id = world.first_of_kind[desk];
    CHECK(id != 0 && world.map[size_t(tile)].first == int16_t(id));
    if (!id)
        return;
    const th::Thing &placed = world.things[id];
    CHECK(placed.type == th::kThingObject && placed.subtype == desk && placed.owner == 0);
    CHECK(placed.x.tile == tile % th::kMapSize && placed.y.tile == tile / th::kMapSize);
    CHECK(world.map[size_t(tile)].flags & 3);
    CHECK((world.map[placed.use_pos[0]].flags & 4) && (world.map[placed.use_pos[1]].flags & 4));
    CHECK(!p.placing && p.requester == th::ObjectRequester::None && p.buy_count[desk] == 0);
    CHECK(ui.window_count() == 0 && ui.requester() == th::Requester::None);

    // A second desk may not go on the first; Escape drops it unbought.
    game->event = {th::EV_BUY_OBJECT, uint16_t(desk), 0, 1};
    game->process_turn();
    game->event = {th::EV_PLACE_NEXT_OBJECT, 0, 0, 0};
    game->process_turn();
    CHECK(p.placing && p.kind == desk);
    th::update_placement(world, p, tile);
    CHECK(!p.ok);
    CHECK(th::object_escape(*game));
    game->process_turn();
    CHECK(!p.placing && p.requester == th::ObjectRequester::None && p.buy_count[desk] == 0);
    CHECK(game->balance == balance - desk_cost && world.things[world.first_of_kind[desk]].type_obj_next == 0);
}

// A sound bank file holding one sample, `pcm` (16-bit mono at `rate`), as
// sample 1 with priority tag `priority`.
std::vector<uint8_t> make_sound_bank(const std::vector<int16_t> &pcm, int rate, uint8_t priority)
{
    auto put32 = [](std::vector<uint8_t> &v, uint32_t x) {
        for (int i = 0; i < 4; i++)
            v.push_back(uint8_t(x >> (8 * i)));
    };
    auto put16 = [](std::vector<uint8_t> &v, uint16_t x) {
        v.push_back(uint8_t(x));
        v.push_back(uint8_t(x >> 8));
    };
    std::vector<uint8_t> wav;
    uint32_t data_size = uint32_t(pcm.size() * 2);
    wav.insert(wav.end(), {'R', 'I', 'F', 'F'});
    put32(wav, 36 + data_size);
    wav.insert(wav.end(), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
    put32(wav, 16);
    put16(wav, 1);
    put16(wav, 1);
    put32(wav, uint32_t(rate));
    put32(wav, uint32_t(rate * 2));
    put16(wav, 2);
    put16(wav, 16);
    wav.insert(wav.end(), {'d', 'a', 't', 'a'});
    put32(wav, data_size);
    for (int16_t s : pcm)
        put16(wav, uint16_t(s));

    std::vector<uint8_t> file = wav;
    uint32_t records = uint32_t(file.size());
    file.resize(file.size() + 64); // record 0 unused, record 1 the sample
    uint8_t *rec = file.data() + records + 32;
    std::memcpy(rec, "TEST.WAV", 8);
    for (int i = 0; i < 4; i++)
        rec[0x1a + i] = uint8_t(wav.size() >> (8 * i));
    rec[0x1e] = priority;
    uint32_t table = uint32_t(file.size());
    file.resize(file.size() + 0x12);
    for (int e = 0; e < 9; e++) {
        bool ours = e == 2;
        put32(file, ours ? records : 0xffffffff);
        put32(file, ours ? 0 : 0xffffffff);
        put32(file, ours ? 64 : 0xffffffff);
        put32(file, ours ? records : 0xffffffff);
    }
    put32(file, table);
    return file;
}

void test_sound_player()
{
    auto path = std::filesystem::temp_directory_path() / "th_enhanced_test_bank.dat";
    {
        std::vector<uint8_t> bytes = make_sound_bank({16384, 16384, 16384, 16384}, 100, 0);
        std::FILE *f = std::fopen(path.string().c_str(), "wb");
        CHECK(f != nullptr);
        if (!f)
            return;
        std::fwrite(bytes.data(), 1, bytes.size(), f);
        std::fclose(f);
    }
    th::SoundBank bank;
    std::string error;
    CHECK(bank.load_file(path, &error));
    std::filesystem::remove(path);
    CHECK(bank.count() == 2);
    CHECK(!bank.sample(0));
    const th::Sample *s = bank.sample(1);
    CHECK(s && s->name == "TEST.WAV" && s->frames() == 4 && s->rate == 100 && s->bits == 16);

    th::SoundPlayer player;
    player.set_bank(&bank);
    std::vector<float> out(16);
    // Centred at full volume: both sides, once, then the voice is free.
    CHECK(player.play_sample(th::kSourceGeneral, 1, 127, 64, 100, 0, 1));
    CHECK(player.playing(th::kSourceGeneral, 1));
    player.mix(out.data(), 8, 100);
    CHECK(out[0] == 0.5f && out[1] == 0.5f && out[6] == 0.5f && out[7] == 0.5f);
    CHECK(out[8] == 0.0f && out[9] == 0.0f);
    CHECK(!player.playing(th::kSourceGeneral, 0));
    // Panned hard left at half volume.
    CHECK(player.play_sample(th::kSourceGeneral, 1, 0x3f, 0, 100, 0, 1));
    player.mix(out.data(), 1, 100);
    CHECK(out[0] > 0.24f && out[0] < 0.25f && out[1] == 0.0f);
    player.stop_all();
    // Type 2 refuses a sample the source is playing; type 3 reuses its voice.
    CHECK(player.play_sample(th::kSourceGeneral, 1, 127, 64, 100, -1, 2));
    CHECK(!player.play_sample(th::kSourceGeneral, 1, 127, 64, 100, -1, 2));
    CHECK(player.play_sample(th::kSourceClock, 1, 127, 64, 100, -1, 2));
    CHECK(player.play_sample(th::kSourceGeneral, 1, 127, 64, 100, -1, 3));
    player.mix(out.data(), 8, 100); // two voices looping forever
    CHECK(out[14] == 1.0f && player.playing(th::kSourceClock, 1));
    player.stop_sample(th::kSourceClock, 1);
    CHECK(!player.playing(th::kSourceClock, 1) && player.playing(th::kSourceGeneral, 1));
    // Ten voices at most.
    player.stop_all();
    for (int i = 0; i < th::SoundPlayer::kVoices; i++)
        CHECK(player.play_sample(th::kSourceGeneral, 1, 127, 64, 100, 0, 1));
    CHECK(!player.play_sample(th::kSourceGeneral, 1, 127, 64, 100, 0, 1));
    // Half the output rate at pitch 200 plays a frame per output frame.
    player.stop_all();
    CHECK(player.play_sample(th::kSourceGeneral, 1, 127, 64, 200, 0, 1));
    player.mix(out.data(), 8, 200);
    CHECK(out[6] == 0.5f && out[8] == 0.0f);

    // play_sound: SFX_YESX plays sample 0x41, absent here; off, nothing plays.
    player.play_sound(th::kSourceGeneral, th::kSfxYesX);
    CHECK(!player.playing(th::kSourceGeneral, 0));
    player.toggle_sound();
    CHECK(!player.sfx_on());
    player.toggle_sound();
    CHECK(player.sfx_on());
    player.set_volume_item(9);
    CHECK(player.sfx_volume() == 12 && player.volume_item() == 9);
    player.set_volume_item(0);
    CHECK(player.sfx_volume() == 127 && player.volume_item() == 0);
}

void test_sound_bank(const char *dir)
{
    th::SoundBank bank;
    std::string error;
    CHECK(bank.load(dir, 0, &error));
    if (!error.empty())
        std::fprintf(stderr, "%s\n", error.c_str());
    CHECK(bank.count() == 747);
    int usable = 0;
    for (int i = 1; i < bank.count(); i++) {
        const th::Sample *s = bank.sample(i);
        // init_sound_heap refuses a bank with a sample of 0x4b000 bytes or more.
        if (s && s->rate == 22050 && s->frames() > 0 && s->data.size() < 0x4b000)
            usable++;
    }
    CHECK(usable == bank.count() - 1);
    const th::Sample *hlight = bank.sample(th::kSfxHighlightP2);
    CHECK(hlight && hlight->name == "HLIGHTP2.WAV" && hlight->frames() == 0x88c0 / 2);
    const th::Sample *yes = bank.sample(th::kSfxYesX);
    CHECK(yes && yes->name == "YESX.WAV");
    for (int language = 1; language < 6; language++) {
        th::SoundBank other;
        CHECK(other.load(dir, language) && other.count() > 500);
    }

    // Menu sounds play centred; a frame sound plays once until the frame
    // changes, placed by its x.
    th::SoundPlayer player;
    player.set_bank(&bank);
    player.play_sound(th::kSourceGeneral, th::kSfxSelectX);
    CHECK(player.playing(th::kSourceGeneral, th::kSfxSelectX));
    th::FrameSound frame{5, th::kSfxYesX, true, 0, 100};
    player.frames_drawn({&frame, 1}, 640);
    CHECK(player.playing(th::thing_source(5), th::kSfxYesX));
    std::vector<float> out(2 * 64);
    player.mix(out.data(), 64, 44100);
    player.stop_all();
    player.frames_drawn({&frame, 1}, 640);
    CHECK(!player.playing(th::thing_source(5), 0));
    frame.sound = 0;
    player.frames_drawn({&frame, 1}, 640);
    frame.sound = th::kSfxYesX;
    frame.x = 320;
    player.frames_drawn({&frame, 1}, 640);
    CHECK(player.playing(th::thing_source(5), th::kSfxYesX));
}

// The intro movie decodes whole: every frame, its palette and its audio
// (22 kHz stereo 16-bit, as long as the frames).
void test_smacker_intro(const char *dir)
{
    auto path = th::resolve_ci(dir, th::movie_info(th::kMovieIntroSm4).file);
    CHECK(path.has_value());
    if (!path)
        return;
    th::SmackerVideo video;
    std::string error;
    CHECK(video.open(*path, &error));
    CHECK(video.width() == 320 && video.height() == 200 && video.frame_count() == 1559);
    CHECK(video.frame_duration_us() == 57000);
    th::SmackerAudioFormat f = video.audio_format(0);
    CHECK(f.rate == 22050 && f.channels == 2 && f.bits == 16);
    size_t samples = 0;
    int frames = 0;
    while (video.next_frame()) {
        frames++;
        samples += video.audio(0).size();
        if (frames == 1)
            CHECK(video.palette_changed());
        if (frames == 300) {
            std::vector<bool> used(256);
            for (uint8_t p : video.pixels())
                used[p] = true;
            CHECK(std::count(used.begin(), used.end(), true) > 64);
        }
    }
    CHECK(frames == 1559 && video.frame_index() == 1558);
    // 1559 frames of 57 ms.
    CHECK(samples / 2 > 22050 * 88 && samples / 2 < 22050 * 90);
}

// The main menu's picture, palette and hover sprites, and the other screens'.
void test_main_menu_art(const char *dir)
{
    auto pic = th::load_data_file(dir, "qdata/main01m.dat");
    auto pal = th::load_data_file(dir, "qdata/main01m.pal");
    CHECK(pic && pic->size() == 320 * 200 && pal && pal->size() == 768);
    th::LanguageText text;
    CHECK(th::load_language(dir, 0, text));
    th::Startup startup;
    std::string error;
    CHECK(startup.load(dir, text, 0, &error));
}

// From the intro to level 1 on the default difficulty with clicks and keys
// as a player gives them, skipping each movie.
void test_startup_new_game(const char *dir)
{
    th::LanguageText text;
    CHECK(th::load_language(dir, 0, text));
    th::Startup s;
    if (!s.load(dir, text, 0))
        return;
    th::StartupResult r;
    auto run = [&](th::UiMouse m, std::vector<th::StartupKey> keys = {}, int frames = 1) {
        for (int i = 0; i < frames && r.kind == th::StartupResult::Kind::Running; i++) {
            r = s.frame(m, keys, 0.04);
            keys.clear();
            m.left_pressed = m.left_released = false;
        }
    };
    auto click = [](int x, int y) {
        th::UiMouse m;
        m.x = x;
        m.y = y;
        m.left_pressed = m.left_released = true;
        return m;
    };
    th::UiMouse still;
    still.x = still.y = 0;
    const th::StartupKey space{th::StartupKey::Space, 0};

    run(still);
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieIntroSm4);
    CHECK(s.surface().width() == 320 && s.surface().height() == 200);
    CHECK(s.movie_audio_format().rate == 22050);
    run(still, {space});
    CHECK(s.screen() == th::StartupScreen::Title && s.surface().width() == 640);
    run(click(5, 5));
    run(still, {}, 60);
    CHECK(s.screen() == th::StartupScreen::Name);
    run(still, {{th::StartupKey::Letter, 'a'}, {th::StartupKey::Return, 0}}, 3);
    CHECK(s.player_name() == "A");
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieMainMenu1);
    run(still, {space}, 2);
    CHECK(s.screen() == th::StartupScreen::MainMenu && s.surface().width() == 320);

    // New Game is the diamond's top quarter; Continue, refused without a
    // save, its left one.
    run(click(0x2e, 0x3a), {}, 2);
    CHECK(s.screen() == th::StartupScreen::MainMenu);
    run(click(0x55, 0x20), {}, 2);
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieMainMenu2);
    run(still, {space}, 80);
    CHECK(s.screen() == th::StartupScreen::Preferences);
    run(click(350, 390), {}, 2); // accept
    CHECK(s.screen() == th::StartupScreen::Movie && s.movie() == th::kMovieArea);
    run(still, {space}, 2);
    CHECK(s.screen() == th::StartupScreen::Briefing);
    run(click(320, 240));
    CHECK(r.kind == th::StartupResult::Kind::StartLevel);
    CHECK(r.level == 1 && r.balance_id == "FULL" && !r.tutorial);
    auto sounds = s.take_sounds();
    CHECK(std::count(sounds.begin(), sounds.end(), uint8_t(0x08)) == 1); // SFX_WRONG2 for Continue

    auto page = th::level_briefing(text, 1);
    CHECK(!page.empty());
}

int main()
{
    test_blit_clips_and_skips();
    test_req_format();
    test_iso_round_trip();
    test_rnc_rejects_garbage();
    test_missing_data();
    test_static_shadows();
    test_text_layout();
    test_wall_in_way();
    test_wallhug();
    test_orthonav_whatnext();
    test_create_an_illness();
    test_set_complex();
    test_sound_player();

    const char *dir = std::getenv("TH_TEST_DATA_DIR");
    bool have_data = dir && *dir && th::missing_data_files(dir).empty();
    if (have_data) {
        test_game_data(dir);
        test_sound_bank(dir);
        test_shadow_sprites(dir);
        test_level_objects(dir);
        test_draw_world(dir);
        test_panel_image(dir);
        test_game_clock(dir);
        test_language_and_fonts(dir);
        test_balance(dir);
        test_walk_level1(dir);
        test_arrivals_level1(dir);
        test_patient_leaves_level1(dir);
        test_menu_bar(dir);
        test_requester_art(dir);
        test_panel_buttons(dir);
        test_yes_no_requester(dir);
        test_applicants_level1(dir);
        test_hire_receptionist(dir);
        test_hire_requester_image(dir);
        test_furnish_corridor(dir);
        test_smacker_intro(dir);
        test_main_menu_art(dir);
        test_startup_new_game(dir);
    }

    if (failures) {
        std::fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    if (!have_data) {
        std::fprintf(stderr, "game data not found: skipped the data tests\n");
        return 77;
    }
    std::printf("all tests passed\n");
    return 0;
}
