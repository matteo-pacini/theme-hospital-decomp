#include "core/render/upscale/scaler.h"

#include "xbrz.h"

#include <array>
#include <charconv>
#include <system_error>

namespace th
{
namespace
{

void scale_xbrz(const uint32_t *src, int w, int h, int factor, uint32_t *dst)
{
    xbrz::scale(size_t(factor), src, dst, w, h, xbrz::ColorFormat::argb);
}

void scale_nearest(const uint32_t *src, int w, int h, int factor, uint32_t *dst)
{
    const size_t out_w = size_t(w) * size_t(factor);
    for (int y = 0; y < h * factor; y++)
        for (size_t x = 0; x < out_w; x++)
            dst[size_t(y) * out_w + x] = src[size_t(y / factor) * size_t(w) + x / size_t(factor)];
}

// xBRZ 1.9 (enhanced/third_party/xbrz/README).
constexpr Scaler kXbrz{"xbrz", "xBRZ", 19, 2, xbrz::SCALE_FACTOR_MAX, scale_xbrz};
constexpr Scaler kNearest{"nearest", "Nearest", 1, 2, 16, scale_nearest};

constexpr std::array<Scaler, 1> kListed{kXbrz};

} // namespace

std::span<const Scaler> scalers()
{
    return kListed;
}

const Scaler *find_scaler(std::string_view id)
{
    for (const Scaler &s : kListed)
        if (id == s.id)
            return &s;
    return nullptr;
}

const Scaler &nearest_scaler()
{
    return kNearest;
}

bool parse_upscale(std::string_view text, UpscaleConfig &out)
{
    if (text == "off") {
        out = {};
        return true;
    }
    const size_t colon = text.find(':');
    if (colon == std::string_view::npos)
        return false;
    const std::string_view id = text.substr(0, colon);
    const std::string_view digits = text.substr(colon + 1);
    const Scaler *scaler = id == kNearest.id ? &kNearest : find_scaler(id);
    int factor = 0;
    const auto [end, ec] = std::from_chars(digits.data(), digits.data() + digits.size(), factor);
    if (!scaler || ec != std::errc() || end != digits.data() + digits.size() || factor < scaler->min_factor ||
        factor > scaler->max_factor)
        return false;
    out = {scaler, factor};
    return true;
}

std::string upscale_to_string(const UpscaleConfig &config)
{
    return config.on() ? std::string(config.scaler->id) + ':' + std::to_string(config.factor) : "off";
}

} // namespace th
