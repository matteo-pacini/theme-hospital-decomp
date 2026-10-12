#pragma once

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/render/upscale/scaler.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>

namespace th
{

// The upscaled versions of the map's three sheets (VBLK-0, VSPR-0, MONEY01V).
struct MapHires {
    HiresSheet blocks;
    HiresSheet sprites;
    HiresSheet money;
};

// The 1x data an upscaled copy is made from, read from a game folder.
struct MapSheets {
    Palette palette{};
    SpriteSheet blocks;
    SpriteSheet sprites;
    SpriteSheet money;
    // A hash of the palette and the three sheets' files: what a cache is
    // valid for.
    uint64_t content_hash = 0;
};

// Reads data/mpalette.dat and the three sheets from `game_dir`. On failure
// says why in `error`.
bool load_map_sheets(const std::filesystem::path &game_dir, MapSheets &out, std::string *error);

// Progress of a build: `done` of `total` sprites. Returns false to stop.
using UpscaleProgress = std::function<bool(size_t done, size_t total)>;

// Upscales the three sheets as `config` says. Returns false when `progress`
// asked to stop.
bool build_map_hires(const MapSheets &sheets, const UpscaleConfig &config, const UpscaleProgress &progress,
                     MapHires &out);

// The file for `config` inside the cache folder `dir`.
std::filesystem::path upscale_cache_file(const std::filesystem::path &dir, const UpscaleConfig &config);

// Writes `hires` as the cache of `config` for `sheets`, then removes the
// folder's cache files of other settings. Returns false, with `error`, when
// it cannot.
bool write_upscale_cache(const std::filesystem::path &dir, const UpscaleConfig &config, const MapSheets &sheets,
                         const MapHires &hires, std::string *error);

// Reads the cache of `config`. False when there is none, it is for other
// data, another algorithm version or another factor, or it is damaged.
bool read_upscale_cache(const std::filesystem::path &dir, const UpscaleConfig &config, const MapSheets &sheets,
                        MapHires &out);

// The upscaled sheets for `config` of the game in `game_dir`: the cache in
// `cache_dir` when it is valid for this data, otherwise built (reporting to
// `progress`) and written there, unless `cache_dir` is empty. `config` is
// lowered as far as memory needs when it asks for more than can be had; not
// even 2x ending the call. False, with `error` set, when that is so, when
// the sheets cannot be read or when `progress` stopped it.
bool upscaled_map_sheets(const std::filesystem::path &game_dir, const std::filesystem::path &cache_dir,
                         UpscaleConfig &config, const UpscaleProgress &progress, MapHires &out, std::string *error);

} // namespace th
