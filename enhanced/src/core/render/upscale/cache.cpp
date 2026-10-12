#include "core/render/upscale/cache.h"

#include "core/data/gamedata.h"
#include "core/render/upscale/hires.h"
#include "core/render/upscale/snap.h"

#include <miniz_tdef.h>
#include <miniz_tinfl.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <system_error>

namespace th
{

namespace
{

constexpr char kMagic[4] = {'T', 'H', 'U', 'P'};
constexpr uint32_t kFormatVersion = 1;
constexpr size_t kAlgorithmField = 16;
constexpr std::string_view kExtension = ".thup";
// What an upscale of factor 1 would take to have in memory, to be squared by
// the factor.
constexpr size_t kBytesPerFactorSquared = 6 * 1000 * 1000;

uint64_t fnv(uint64_t hash, const uint8_t *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
        hash = (hash ^ data[i]) * 1099511628211ull;
    return hash;
}

void put32(std::vector<uint8_t> &out, uint32_t v)
{
    for (int i = 0; i < 4; i++)
        out.push_back(uint8_t(v >> (8 * i)));
}

void put64(std::vector<uint8_t> &out, uint64_t v)
{
    put32(out, uint32_t(v));
    put32(out, uint32_t(v >> 32));
}

// Reads little-endian values from a byte range, noting when it runs out.
class Reader
{
  public:
    Reader(const uint8_t *p, size_t size) : p_(p), end_(p + size) {}

    bool ok() const { return ok_; }
    const uint8_t *pos() const { return p_; }
    size_t left() const { return size_t(end_ - p_); }

    uint32_t u32()
    {
        if (left() < 4) {
            ok_ = false;
            return 0;
        }
        uint32_t v = uint32_t(p_[0]) | uint32_t(p_[1]) << 8 | uint32_t(p_[2]) << 16 | uint32_t(p_[3]) << 24;
        p_ += 4;
        return v;
    }

    uint16_t u16()
    {
        if (left() < 2) {
            ok_ = false;
            return 0;
        }
        uint16_t v = uint16_t(p_[0] | p_[1] << 8);
        p_ += 2;
        return v;
    }

    uint64_t u64()
    {
        uint64_t low = u32();
        return low | uint64_t(u32()) << 32;
    }

    const uint8_t *take(size_t n)
    {
        if (left() < n) {
            ok_ = false;
            return nullptr;
        }
        const uint8_t *at = p_;
        p_ += n;
        return at;
    }

  private:
    const uint8_t *p_;
    const uint8_t *end_;
    bool ok_ = true;
};

// Loads `base`.tab and `base`.dat into `sheet`, adding their bytes to `hash`.
bool load_hashed_sheet(const std::filesystem::path &dir, std::string_view base, SpriteSheet &sheet, uint64_t &hash,
                       std::string *error)
{
    auto tab = load_data_file(dir, std::string(base) + ".tab", error);
    auto dat = tab ? load_data_file(dir, std::string(base) + ".dat", error) : std::nullopt;
    if (!tab || !dat)
        return false;
    hash = fnv(hash, tab->data(), tab->size());
    hash = fnv(hash, dat->data(), dat->size());
    if (!sheet.load(std::move(*tab), std::move(*dat), SpriteFormat::Rle)) {
        if (error)
            *error = std::string(base) + ".tab: not a sprite table";
        return false;
    }
    return true;
}

void put_sheet(std::vector<uint8_t> &out, const HiresSheet &sheet)
{
    put32(out, uint32_t(sheet.entries.size()));
    for (const HiresSheet::Entry &e : sheet.entries) {
        put32(out, e.offset);
        put32(out, uint32_t(e.width) | uint32_t(e.height) << 16);
    }
    put32(out, uint32_t(sheet.data.size()));
    std::vector<uint8_t> packed(sheet.data.size() + sheet.data.size() / 50 + 1024);
    size_t n = tdefl_compress_mem_to_mem(packed.data(), packed.size(), sheet.data.data(), sheet.data.size(),
                                         TDEFL_DEFAULT_MAX_PROBES);
    packed.resize(n);
    put32(out, uint32_t(n));
    out.insert(out.end(), packed.begin(), packed.end());
}

// Whether `entry`'s rows, as decode_hires and draw_hires_solid read them, lie
// inside `data` and no run leaves the entry's width.
bool hires_rows_inside(const std::vector<uint8_t> &data, const HiresSheet::Entry &entry)
{
    const size_t size = data.size();
    size_t at = entry.offset;
    auto u16 = [&](size_t p) { return size_t(data[p]) | size_t(data[p + 1]) << 8; };
    for (size_t row = 0; row < entry.height; row++) {
        if (at > size || size - at < 2)
            return false;
        const size_t runs = u16(at);
        at += 2;
        size_t col = 0;
        for (size_t r = 0; r < runs; r++) {
            if (size - at < 4)
                return false;
            col += u16(at);
            const size_t length = u16(at + 2);
            at += 4;
            if (col + length > entry.width || size - at < length)
                return false;
            col += length;
            at += length;
        }
    }
    return true;
}

// Reads one upscaled sheet that must match `base`, the 1x sheet it was made
// from: one entry per sprite, each `factor` times the sprite's size, or
// absent for a sprite with no pixels.
bool get_sheet(Reader &in, int factor, const SpriteSheet &base, HiresSheet &sheet)
{
    sheet = {};
    sheet.factor = factor;
    const uint32_t count = in.u32();
    if (!in.ok() || count != base.size())
        return false;
    sheet.entries.resize(count);
    for (HiresSheet::Entry &e : sheet.entries) {
        e.offset = in.u32();
        const uint32_t size = in.u32();
        e.width = uint16_t(size);
        e.height = uint16_t(size >> 16);
    }
    const uint32_t raw = in.u32();
    const uint32_t packed = in.u32();
    const uint8_t *src = in.take(packed);
    if (!in.ok() || raw > (1u << 30))
        return false;
    sheet.data.resize(raw);
    const size_t got = tinfl_decompress_mem_to_mem(sheet.data.data(), raw, src, packed, 0);
    if (got != raw)
        return false;
    for (size_t i = 0; i < count; i++) {
        const HiresSheet::Entry &e = sheet.entries[i];
        const Sprite one = base.at(i);
        if (one.empty()) {
            if (e.width != 0 || e.height != 0)
                return false;
        } else if (e.width != one.width * factor || e.height != one.height * factor ||
                   !hires_rows_inside(sheet.data, e)) {
            return false;
        }
    }
    return true;
}

} // namespace

bool load_map_sheets(const std::filesystem::path &game_dir, MapSheets &out, std::string *error)
{
    uint64_t hash = 1469598103934665603ull;
    auto pal = load_data_file(game_dir, "data/mpalette.dat", error);
    if (!pal)
        return false;
    if (!palette_from_vga(*pal, out.palette)) {
        if (error)
            *error = "data/mpalette.dat: too short for a palette";
        return false;
    }
    hash = fnv(hash, pal->data(), pal->size());
    if (!load_hashed_sheet(game_dir, "data/vblk-0", out.blocks, hash, error) ||
        !load_hashed_sheet(game_dir, "data/vspr-0", out.sprites, hash, error) ||
        !load_hashed_sheet(game_dir, "data/money01v", out.money, hash, error))
        return false;
    out.content_hash = hash;
    return true;
}

bool build_map_hires(const MapSheets &sheets, const UpscaleConfig &config, const UpscaleProgress &progress,
                     MapHires &out)
{
    if (!config.on())
        return false;
    const size_t total = sheets.blocks.size() + sheets.sprites.size() + sheets.money.size();
    size_t base = 0;
    bool stopped = false;
    auto one = [&](const SpriteSheet &sheet, HiresSheet &into) {
        into = build_hires(sheet, sheets.palette, *config.scaler, config.factor, [&](size_t done) {
            stopped = progress && !progress(base + done, total);
            return !stopped;
        });
        base += sheet.size();
    };
    one(sheets.blocks, out.blocks);
    if (!stopped)
        one(sheets.sprites, out.sprites);
    if (!stopped)
        one(sheets.money, out.money);
    return !stopped;
}

std::filesystem::path upscale_cache_file(const std::filesystem::path &dir, const UpscaleConfig &config)
{
    std::string name = upscale_to_string(config);
    std::replace(name.begin(), name.end(), ':', '-');
    return dir / (name + std::string(kExtension));
}

bool write_upscale_cache(const std::filesystem::path &dir, const UpscaleConfig &config, const MapSheets &sheets,
                         const MapHires &hires, std::string *error)
{
    std::vector<uint8_t> out(kMagic, kMagic + 4);
    put32(out, kFormatVersion);
    std::string id = config.scaler->id;
    id.resize(kAlgorithmField, '\0');
    out.insert(out.end(), id.begin(), id.end());
    put32(out, uint32_t(config.scaler->version));
    put32(out, uint32_t(config.factor));
    put64(out, sheets.content_hash);
    put_sheet(out, hires.blocks);
    put_sheet(out, hires.sprites);
    put_sheet(out, hires.money);
    put64(out, fnv(1469598103934665603ull, out.data(), out.size()));

    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    const std::filesystem::path file = upscale_cache_file(dir, config);
    const std::filesystem::path partial = file.string() + ".partial";
    {
        std::ofstream f(partial, std::ios::binary | std::ios::trunc);
        f.write(reinterpret_cast<const char *>(out.data()), std::streamsize(out.size()));
        if (!f) {
            if (error)
                *error = partial.string() + ": cannot be written";
            return false;
        }
    }
    std::filesystem::rename(partial, file, ec);
    if (ec) {
        if (error)
            *error = file.string() + ": " + ec.message();
        return false;
    }
    for (const auto &entry : std::filesystem::directory_iterator(dir, ec))
        if (entry.path() != file && entry.path().extension() == kExtension)
            std::filesystem::remove(entry.path(), ec);
    return true;
}

bool read_upscale_cache(const std::filesystem::path &dir, const UpscaleConfig &config, const MapSheets &sheets,
                        MapHires &out)
{
    std::ifstream f(upscale_cache_file(dir, config), std::ios::binary);
    if (!f)
        return false;
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (bytes.size() < 8)
        return false;
    Reader tail(bytes.data() + bytes.size() - 8, 8);
    if (tail.u64() != fnv(1469598103934665603ull, bytes.data(), bytes.size() - 8))
        return false;
    Reader in(bytes.data(), bytes.size() - 8);
    const uint8_t *magic = in.take(4);
    if (!magic || std::memcmp(magic, kMagic, 4) != 0 || in.u32() != kFormatVersion)
        return false;
    const uint8_t *id = in.take(kAlgorithmField);
    if (!id || std::strncmp(reinterpret_cast<const char *>(id), config.scaler->id, kAlgorithmField) != 0)
        return false;
    if (in.u32() != uint32_t(config.scaler->version) || in.u32() != uint32_t(config.factor) ||
        in.u64() != sheets.content_hash || !in.ok())
        return false;
    return get_sheet(in, config.factor, sheets.blocks, out.blocks) &&
           get_sheet(in, config.factor, sheets.sprites, out.sprites) &&
           get_sheet(in, config.factor, sheets.money, out.money);
}

namespace
{

// Whether `bytes` can be had now: the probe is allocated and given back.
// nothrow, because the browser's build has no exceptions to catch.
bool memory_for(size_t bytes)
{
    void *probe = std::malloc(bytes);
    std::free(probe);
    return probe != nullptr;
}

} // namespace

bool upscaled_map_sheets(const std::filesystem::path &game_dir, const std::filesystem::path &cache_dir,
                         UpscaleConfig &config, const UpscaleProgress &progress, MapHires &out, std::string *error)
{
    // The sheets, the shaded blocks and the larger view cost about the
    // square of the factor; a lower factor is used when this cannot be had.
    while (config.on() && !memory_for(size_t(config.factor) * size_t(config.factor) * kBytesPerFactorSquared)) {
        std::fprintf(stderr, "upscale: no memory for %s, trying %dx\n", upscale_to_string(config).c_str(),
                     config.factor - 1);
        config.factor--;
    }
    if (!config.on()) {
        if (error)
            *error = "not enough memory";
        return false;
    }
    MapSheets sheets;
    if (!load_map_sheets(game_dir, sheets, error))
        return false;
    if (!cache_dir.empty() && read_upscale_cache(cache_dir, config, sheets, out))
        return true;
    if (!build_map_hires(sheets, config, progress, out)) {
        if (error)
            *error = "stopped";
        return false;
    }
    std::string store_error;
    if (!cache_dir.empty() && !write_upscale_cache(cache_dir, config, sheets, out, &store_error))
        std::fprintf(stderr, "upscale: %s\n", store_error.c_str());
    return true;
}

} // namespace th
