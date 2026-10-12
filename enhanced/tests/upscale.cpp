#include "common.h"

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/render/upscale/cache.h"
#include "core/render/upscale/scaler.h"
#include "core/render/upscale/snap.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace
{

// A palette of distinct greys and reds, so that every entry is the unique
// nearest to itself.
th::Palette test_palette()
{
    th::Palette p{};
    for (size_t i = 0; i < p.size(); i++)
        p[i] = 0xff000000u | uint32_t(i) << 16 | uint32_t(255 - i) << 8 | uint32_t(i / 2);
    return p;
}

// 4x3: "skip 1, pixels 10 20 30", "10 20 30 40", "skip 2, pixels 40 50".
th::Sprite test_sprite()
{
    static const uint8_t rle[] = {0xff, 3, 10, 20, 30, 0, 4, 10, 20, 30, 40, 0, 0xfe, 2, 40, 50, 0};
    return {rle, th::SpriteFormat::Rle, 4, 3};
}

} // namespace

void test_upscale_scaler_table()
{
    const th::Scaler *x = th::find_scaler("xbrz");
    CHECK(x != nullptr);
    CHECK(th::find_scaler("nearest") == nullptr);
    CHECK(th::find_scaler("") == nullptr);
    CHECK(th::nearest_scaler().min_factor <= 2);
    if (!x)
        return;
    CHECK(x->min_factor == 2 && x->max_factor == 6);
    CHECK(x->version > 0);
    for (const th::Scaler &s : th::scalers())
        CHECK(s.id && s.name && s.scale);
}

void test_upscale_xbrz_golden()
{
    const th::Scaler *x = th::find_scaler("xbrz");
    if (!x)
        return CHECK(false);
    // A diagonal edge between two flat colours, 8x8.
    std::array<uint32_t, 64> src{};
    for (size_t y = 0; y < 8; y++)
        for (size_t c = 0; c < 8; c++)
            src[y * 8 + c] = c >= y ? 0xff2040c0u : 0xffe0c020u;
    for (int factor = x->min_factor; factor <= x->max_factor; factor++) {
        std::vector<uint32_t> out(size_t(64 * factor * factor), 0xdeadbeefu);
        x->scale(src.data(), 8, 8, factor, out.data());
        uint64_t sum = 1469598103934665603ull;
        for (uint32_t v : out)
            sum = (sum ^ v) * 1099511628211ull;
        // Fully written; the flat parts away from the diagonal and the image
        // border (which blends with nothing) keep their colour.
        bool written = true;
        for (uint32_t v : out)
            written = written && v != 0xdeadbeefu;
        CHECK(written);
        CHECK(out[size_t(5 * factor)] == 0xff2040c0u);
        CHECK(out[size_t(8 * factor) * size_t(8 * factor - 1) + size_t(factor)] == 0xffe0c020u);
        static const uint64_t kGolden[] = {0,
                                           0,
                                           13960448337745624155ull,
                                           6897122233363481415ull,
                                           10140919101445673821ull,
                                           11567878932324872191ull,
                                           12207539019663912053ull};
        CHECK(sum == kGolden[factor]);
    }
}

void test_upscale_snapper()
{
    th::Palette p = test_palette();
    // Entries 0xfe and 0xff hold the colour of a query: they must lose to
    // a worse match below them.
    p[0xfe] = 0xff123456;
    p[0xff] = 0xff654321;
    th::PaletteSnapper snap(p);
    CHECK(snap.nearest(p[0]) == 0);
    CHECK(snap.nearest(p[77]) == 77);
    CHECK(snap.nearest(p[0xfd]) == 0xfd);
    CHECK(snap.nearest(0x123456) < th::kSnapColours);
    CHECK(snap.nearest(0x654321) < th::kSnapColours);
    CHECK(snap.nearest(p[77] ^ 0x01000000) == 77); // alpha is ignored

    // The first of two equal entries wins.
    p[10] = p[200];
    th::PaletteSnapper dup(p);
    CHECK(dup.nearest(p[200]) == 10);

    const uint32_t px[] = {p[5], 0x00ffffffu, 0x7fffffffu, 0x80000000u};
    uint8_t out[4];
    dup.snap(px, 4, out);
    CHECK(out[0] == 5);
    CHECK(out[1] == th::kUpscaleTransparent);
    CHECK(out[2] == th::kUpscaleTransparent);
    CHECK(out[3] != th::kUpscaleTransparent);
}

void test_upscale_sprite_nearest_is_replication()
{
    const th::Palette p = test_palette();
    const th::Sprite s = test_sprite();
    th::PaletteSnapper snap(p);
    for (int factor = 2; factor <= 4; factor++) {
        th::IndexedImage img = th::upscale_sprite(s, p, th::nearest_scaler(), factor, snap);
        CHECK(img.width == 4 * factor && img.height == 3 * factor);
        CHECK(img.pixels.size() == size_t(img.width) * size_t(img.height));
        // Row 0 of the sprite: transparent, then 10 20 30.
        const uint8_t row0[] = {th::kUpscaleTransparent, 10, 20, 30};
        const uint8_t row2[] = {th::kUpscaleTransparent, th::kUpscaleTransparent, 40, 50};
        bool same = true;
        for (int y = 0; y < img.height; y++)
            for (int x = 0; x < img.width; x++) {
                const uint8_t got = img.pixels[size_t(y * img.width + x)];
                const int sy = y / factor;
                const int sx = x / factor;
                uint8_t want = sy == 0 ? row0[sx] : sy == 2 ? row2[sx] : uint8_t(sx == 3 ? 40 : 10 + 10 * sx);
                same = same && got == want;
            }
        CHECK(same);
    }
}

void test_upscale_sprite_xbrz_keeps_box()
{
    const th::Palette p = test_palette();
    const th::Sprite s = test_sprite();
    th::PaletteSnapper snap(p);
    const th::Scaler *x = th::find_scaler("xbrz");
    if (!x)
        return CHECK(false);
    th::IndexedImage img = th::upscale_sprite(s, p, *x, 4, snap);
    CHECK(img.width == 16 && img.height == 12);
    bool no_mask = true;
    for (uint8_t v : img.pixels)
        no_mask = no_mask && v != 0xfe;
    CHECK(no_mask);
    // The top-left pixel of the sprite is transparent, and so is its zoomed
    // corner; an interior pixel of the first run is opaque.
    CHECK(img.pixels[0] == th::kUpscaleTransparent);
    CHECK(img.pixels[size_t(1 * 16 + 8)] != th::kUpscaleTransparent);
    th::Sprite empty;
    CHECK(th::upscale_sprite(empty, p, *x, 4, snap).pixels.empty());
}

namespace
{

// Three tiny sheets: a 4x3 sprite, an empty one and a 2x1 one in each.
th::MapSheets test_sheets()
{
    th::MapSheets s;
    s.palette = test_palette();
    s.content_hash = 0x1234;
    // TAB entries: offset u32, width, height. The sprites share one DAT.
    const std::vector<uint8_t> dat = {0xff, 3, 10, 20, 30, 0, 4, 10, 20, 30, 40, 0, 0xfe, 2, 40, 50, 0, 2, 7, 8, 0};
    const std::vector<uint8_t> tab = {0, 0, 0, 0, 4, 3, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 2, 1};
    for (th::SpriteSheet *sheet : {&s.blocks, &s.sprites, &s.money})
        sheet->load(tab, dat);
    return s;
}

} // namespace

// A cache file that is whole (its checksum holds) but disagrees with the 1x
// sheets, or has runs that leave its data, is refused.
void test_upscale_cache_rejects_inconsistent()
{
    const th::MapSheets sheets = test_sheets();
    th::UpscaleConfig config;
    CHECK(th::parse_upscale("xbrz:3", config));
    th::MapHires built;
    CHECK(th::build_map_hires(sheets, config, {}, built));

    const std::filesystem::path dir =
        std::filesystem::temp_directory_path() / ("th_upscale_cache_bad_" + std::to_string(long(&built) & 0xffff));
    std::filesystem::remove_all(dir);
    std::string error;
    th::MapHires read;
    auto refused = [&](auto alter) {
        th::MapHires bad = built;
        alter(bad.blocks);
        return th::write_upscale_cache(dir, config, sheets, bad, &error) &&
               !th::read_upscale_cache(dir, config, sheets, read);
    };
    CHECK(refused([](th::HiresSheet &s) { s.entries[0].width++; }));
    CHECK(refused([](th::HiresSheet &s) { s.entries[0].height = 0; }));
    CHECK(refused([](th::HiresSheet &s) { s.entries[1] = {0, 3, 3}; }));
    CHECK(refused([](th::HiresSheet &s) { s.entries.pop_back(); }));
    CHECK(refused([](th::HiresSheet &s) { s.entries[2].offset = uint32_t(s.data.size()); }));
    CHECK(refused([](th::HiresSheet &s) { s.data.resize(s.entries[2].offset + 3); }));
    CHECK(refused([](th::HiresSheet &s) {
        s.data[s.entries[0].offset] = 0xff;
        s.data[s.entries[0].offset + 1] = 0xff;
    }));
    CHECK(refused([](th::HiresSheet &s) {
        s.data[s.entries[0].offset + 4] = 0xff;
        s.data[s.entries[0].offset + 5] = 0x7f;
    }));
    CHECK(th::write_upscale_cache(dir, config, sheets, built, &error));
    CHECK(th::read_upscale_cache(dir, config, sheets, read));
    std::filesystem::remove_all(dir);
}

void test_upscale_cache_round_trip()
{
    const th::MapSheets sheets = test_sheets();
    th::UpscaleConfig config;
    CHECK(th::parse_upscale("xbrz:3", config));
    th::MapHires built;
    size_t last = 0;
    CHECK(th::build_map_hires(
        sheets, config,
        [&](size_t done, size_t total) {
            last = done;
            return total == 9;
        },
        built));
    CHECK(last == 9);
    CHECK(built.blocks.entries.size() == 3 && built.blocks.entries[1].width == 0);
    CHECK(built.blocks.entries[0].width == 12 && built.blocks.entries[0].height == 9);

    const std::filesystem::path dir =
        std::filesystem::temp_directory_path() / ("th_upscale_cache_test_" + std::to_string(long(&last) & 0xffff));
    std::filesystem::remove_all(dir);
    std::string error;
    CHECK(th::write_upscale_cache(dir, config, sheets, built, &error));

    th::MapHires read;
    CHECK(th::read_upscale_cache(dir, config, sheets, read));
    CHECK(read.blocks.data == built.blocks.data && read.sprites.data == built.sprites.data);
    CHECK(read.money.entries.size() == 3 && read.money.factor == 3);

    // Other data, another factor and another algorithm version rebuild.
    th::MapSheets other = sheets;
    other.content_hash++;
    CHECK(!th::read_upscale_cache(dir, config, other, read));
    th::UpscaleConfig four;
    CHECK(th::parse_upscale("xbrz:4", four));
    CHECK(!th::read_upscale_cache(dir, four, sheets, read));

    // A damaged file is refused, not read.
    const std::filesystem::path file = th::upscale_cache_file(dir, config);
    {
        std::fstream f(file, std::ios::binary | std::ios::in | std::ios::out);
        f.seekp(60);
        f.put('\x55');
    }
    CHECK(!th::read_upscale_cache(dir, config, sheets, read));

    // Writing another setting removes this one's file.
    CHECK(th::write_upscale_cache(dir, four, sheets, built, &error));
    CHECK(!std::filesystem::exists(file));
    CHECK(std::filesystem::exists(th::upscale_cache_file(dir, four)));
    std::filesystem::remove_all(dir);
}

void test_upscale_game_data(const char *dir)
{
    th::MapSheets sheets;
    std::string error;
    CHECK(th::load_map_sheets(dir, sheets, &error));
    th::UpscaleConfig config;
    CHECK(th::parse_upscale("xbrz:4", config));
    th::MapHires built;
    CHECK(th::build_map_hires(sheets, config, {}, built));

    // Every sprite of the three sheets has its upscaled version, exactly
    // the factor times its box, and none holds the mask colour.
    int wrong = 0, masks = 0;
    size_t bytes = 0;
    auto check_sheet = [&](th::SpriteSheet &sheet, const th::HiresSheet &hires) {
        bytes += hires.data.size();
        sheet.attach_hires(hires);
        for (size_t i = 0; i < sheet.size(); i++) {
            const th::Sprite s = sheet.at(i);
            wrong += s.empty() ? !s.hi.empty() : s.hi.width != s.width * 4 || s.hi.height != s.height * 4;
            if (!s.hi.empty())
                th::decode_hires(s.hi, [&](int, int, uint8_t colour) { masks += colour == 0xfe; });
        }
    };
    check_sheet(sheets.blocks, built.blocks);
    check_sheet(sheets.sprites, built.sprites);
    check_sheet(sheets.money, built.money);
    CHECK(wrong == 0);
    CHECK(masks == 0);
    std::printf("upscale xbrz:4: %zu sprites, %.1f MB of indices\n",
                sheets.blocks.size() + sheets.sprites.size() + sheets.money.size(), double(bytes) / 1e6);

    const std::filesystem::path cache = std::filesystem::temp_directory_path() / "th_upscale_game_data_test";
    std::filesystem::remove_all(cache);
    CHECK(th::write_upscale_cache(cache, config, sheets, built, &error));
    std::printf("upscale xbrz:4: cache file %.1f MB\n",
                double(std::filesystem::file_size(th::upscale_cache_file(cache, config))) / 1e6);
    th::MapHires read;
    CHECK(th::read_upscale_cache(cache, config, sheets, read));
    CHECK(read.blocks.data == built.blocks.data && read.sprites.data == built.sprites.data);
    std::filesystem::remove_all(cache);
}
