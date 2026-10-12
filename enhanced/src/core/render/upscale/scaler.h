#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace th
{

// An image-scaling algorithm for the install-time upscaling.
struct Scaler {
    // Stable name stored in the settings and in a cache's header.
    const char *id;
    // Name shown on the setup screen.
    const char *name;
    // Version of the algorithm's code: part of a cache's key, so a different
    // one rebuilds the cache.
    int version;
    int min_factor;
    int max_factor;
    // Writes the `w * factor` x `h * factor` copy of the `w` x `h` image
    // `src` to `dst`. Pixels are 0xAARRGGBB with alpha 0 or 255; `factor` is
    // within [min_factor, max_factor].
    void (*scale)(const uint32_t *src, int w, int h, int factor, uint32_t *dst);
};

// The algorithms the setup screen offers, in the order it lists them.
std::span<const Scaler> scalers();

// The algorithm called `id`, or null.
const Scaler *find_scaler(std::string_view id);

// Every pixel copied `factor` x `factor` times: what the view's integer zoom
// does. Not on the setup screen's list; parse_upscale reads it as
// "nearest:N", for tests that compare a scaled drawing with the zoomed one.
const Scaler &nearest_scaler();

// The upscaling a run uses: an algorithm and a factor, or none.
struct UpscaleConfig {
    const Scaler *scaler = nullptr;
    int factor = 1;

    bool on() const { return scaler && factor > 1; }
    bool operator==(const UpscaleConfig &) const = default;
};

// Reads "off" or "<algorithm id>:<factor>" (as the settings file and
// --upscale spell it) into `out`; false, leaving `out` alone, for anything
// else, including a factor the algorithm does not support.
bool parse_upscale(std::string_view text, UpscaleConfig &out);

// The text parse_upscale reads back to `config`.
std::string upscale_to_string(const UpscaleConfig &config);

} // namespace th
