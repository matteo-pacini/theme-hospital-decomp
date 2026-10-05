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
#include "core/world/game.h"
#include "core/world/world.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>

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
    game->start(*level, 1, settings, gd.anims());
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

int main()
{
    test_blit_clips_and_skips();
    test_req_format();
    test_iso_round_trip();
    test_rnc_rejects_garbage();
    test_missing_data();
    test_static_shadows();
    test_text_layout();

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
