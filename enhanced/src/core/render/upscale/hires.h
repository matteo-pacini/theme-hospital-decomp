#pragma once

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/render/upscale/scaler.h"
#include "core/render/upscale/snap.h"

#include <functional>
#include <vector>

namespace th
{

// Appends `image` coded as HiSprite rows (core/data/sprites.h) to `out`.
void encode_hires(const IndexedImage &image, std::vector<uint8_t> &out);

// The upscaled version of every sprite of `sheet`: each through `scaler` at
// `factor` with `palette`'s colours, snapped back to them. Its `entries`
// match the sheet's sprite indices, one for each, a sprite with no pixels
// having width 0. `progress` is called after each sprite with the number done
// and stops the build, returning what is made so far, when it returns false.
HiresSheet build_hires(const SpriteSheet &sheet, const Palette &palette, const Scaler &scaler, int factor,
                       const std::function<bool(size_t)> &progress = {});

} // namespace th
