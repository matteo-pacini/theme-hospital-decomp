#include "core/render/upscale/snap.h"

#include "core/render/blit.h"

namespace th
{

PaletteSnapper::PaletteSnapper(const Palette &palette) : palette_(palette)
{
}

uint8_t PaletteSnapper::nearest(uint32_t rgb)
{
    rgb &= 0xffffff;
    auto found = cache_.find(rgb);
    if (found != cache_.end())
        return found->second;

    const int r = int(rgb >> 16 & 0xff);
    const int g = int(rgb >> 8 & 0xff);
    const int b = int(rgb & 0xff);
    int best = 0;
    int best_distance = 1 << 30;
    for (int i = 0; i < kSnapColours; i++) {
        const uint32_t c = palette_[size_t(i)];
        const int dr = r - int(c >> 16 & 0xff);
        const int dg = g - int(c >> 8 & 0xff);
        const int db = b - int(c & 0xff);
        const int distance = dr * dr + dg * dg + db * db;
        if (distance < best_distance) {
            best_distance = distance;
            best = i;
        }
    }
    cache_.emplace(rgb, uint8_t(best));
    return uint8_t(best);
}

void PaletteSnapper::snap(const uint32_t *argb, size_t count, uint8_t *out)
{
    for (size_t i = 0; i < count; i++)
        out[i] = (argb[i] >> 24) < 128 ? kUpscaleTransparent : nearest(argb[i]);
}

IndexedImage upscale_sprite(const Sprite &sprite, const Palette &palette, const Scaler &scaler, int factor,
                            PaletteSnapper &snapper)
{
    IndexedImage out;
    if (sprite.empty())
        return out;

    const int bw = sprite.width + 2;
    const int bh = sprite.height + 2;
    std::vector<uint32_t> bordered(size_t(bw) * size_t(bh), 0);
    sprite_to_argb(sprite, palette, bordered.data() + bw + 1, bw * int(sizeof(uint32_t)));

    std::vector<uint32_t> big(size_t(bw) * size_t(bh) * size_t(factor) * size_t(factor));
    scaler.scale(bordered.data(), bw, bh, factor, big.data());

    out.width = sprite.width * factor;
    out.height = sprite.height * factor;
    out.pixels.resize(size_t(out.width) * size_t(out.height));
    std::vector<uint32_t> row(size_t(out.width));
    for (int y = 0; y < out.height; y++) {
        const uint32_t *from = big.data() + size_t(y + factor) * size_t(bw * factor) + size_t(factor);
        const uint32_t *source = bordered.data() + size_t(y / factor + 1) * size_t(bw) + 1;
        // A pixel the sprite itself draws stays opaque whatever the scaler
        // blended it with: tiles meet edge to edge, and a gap along an edge
        // would show the colour behind them.
        for (int x = 0; x < out.width; x++) {
            const bool opaque = (source[x / factor] >> 24) == 0xff;
            row[size_t(x)] = opaque ? from[x] | 0xff000000u : from[x];
        }
        snapper.snap(row.data(), row.size(), out.pixels.data() + size_t(y) * size_t(out.width));
    }
    return out;
}

} // namespace th
