#pragma once

#include "core/render/upscale/scaler.h"

#include <SDL3/SDL.h>

#include <cstdint>
#include <vector>

namespace th
{

// An image composed at the game's own size, shown `factor` times larger
// through the scaler. The scaling runs when the pixels, the size or the
// config differ from the last call's, so a screen that does not change costs
// a comparison per frame.
class ScaledTexture
{
  public:
    ScaledTexture() = default;
    ScaledTexture(const ScaledTexture &) = delete;
    ScaledTexture &operator=(const ScaledTexture &) = delete;
    ~ScaledTexture() { reset(); }

    // The texture holding the `w` x `h` image `argb` at config.factor times
    // that size (blended by alpha), or null when upscaling is off or the
    // texture could not be made.
    SDL_Texture *update(SDL_Renderer *renderer, const UpscaleConfig &config, const uint32_t *argb, int w, int h);

    // Drops the texture; the next update scales again.
    void reset();

  private:
    SDL_Texture *texture_ = nullptr;
    UpscaleConfig config_;
    int w_ = 0;
    int h_ = 0;
    std::vector<uint32_t> last_;
    std::vector<uint32_t> scaled_;
};

// Sets how `texture` is filtered when drawn `dst_w` pixels wide: nearest when
// that is a whole multiple of its own width (sharp pixels), linear otherwise
// (a reduction, or a ratio the pixels cannot land on).
void set_scale_mode_for(SDL_Texture *texture, float dst_w);

// Scales `argb` (w x h, alpha by pixel) once, for a texture that is made
// once: the pointers.
std::vector<uint32_t> upscale_argb(const UpscaleConfig &config, const uint32_t *argb, int w, int h);

} // namespace th
