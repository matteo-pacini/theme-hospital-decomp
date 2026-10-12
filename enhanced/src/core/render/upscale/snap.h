#pragma once

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/render/upscale/scaler.h"

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace th
{

// Palette entries from this index up are never the result of snapping: 0xfe
// is the colour the wall mask is drawn in (kMaskColour, core/render/view.h)
// and 0xff stays unused, as in the game's sources.
constexpr int kSnapColours = 0xfe;

// What an upscaled image holds where it is transparent. It is not a colour:
// no snapped pixel has this value.
constexpr uint8_t kUpscaleTransparent = 0xff;

// Maps colours to the nearest entry of a palette (squared RGB distance, the
// lowest index on a tie), the entries below kSnapColours only. Remembers each
// colour it has answered for, so one snapper serves a whole sheet; it is not
// safe to share between threads.
class PaletteSnapper
{
  public:
    explicit PaletteSnapper(const Palette &palette);

    // The index nearest to the colour in the low 24 bits of `rgb`.
    uint8_t nearest(uint32_t rgb);

    // Snaps `count` ARGB pixels to indices: alpha under 128 gives
    // kUpscaleTransparent, any other pixel the nearest colour.
    void snap(const uint32_t *argb, size_t count, uint8_t *out);

  private:
    Palette palette_;
    std::unordered_map<uint32_t, uint8_t> cache_;
};

// An 8-bit image of any size; kUpscaleTransparent marks transparent pixels.
struct IndexedImage {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels;
};

// `sprite` at `factor` times its size, through `scaler` with the colours of
// `palette`, snapped back to its entries by `snapper` (made from the same
// palette). The image is exactly `factor` times the sprite's box: the scaler
// sees a transparent border one pixel wide so that the edges blend as
// anywhere else, and the result is cut back to the box. An empty sprite gives
// an empty image.
IndexedImage upscale_sprite(const Sprite &sprite, const Palette &palette, const Scaler &scaler, int factor,
                            PaletteSnapper &snapper);

} // namespace th
