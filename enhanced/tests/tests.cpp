// Engine tests. Those that need the game data read it from TH_TEST_DATA_DIR
// and are skipped (exit code 77) when it is unset or missing.

#include "core/data/balance.h"
#include "core/data/blocks.h"
#include "core/data/gamedata.h"
#include "core/data/rnc.h"
#include "core/data/text.h"
#include "core/render/blit.h"
#include "core/render/iso.h"
#include "core/render/panel.h"
#include "core/render/shadows.h"
#include "core/render/text.h"
#include "core/render/view.h"
#include "core/ui/menubar.h"
#include "core/ui/windows.h"
#include "core/world/arrivals.h"
#include "core/world/complex.h"
#include "core/world/game.h"
#include "core/world/nav.h"
#include "core/world/world.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
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

    const char *dir = std::getenv("TH_TEST_DATA_DIR");
    bool have_data = dir && *dir && th::missing_data_files(dir).empty();
    if (have_data) {
        test_game_data(dir);
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
