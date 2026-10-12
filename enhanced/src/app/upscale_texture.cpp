#include "app/upscale_texture.h"

#include <cmath>
#include <cstring>

namespace th
{

SDL_Texture *ScaledTexture::update(SDL_Renderer *renderer, const UpscaleConfig &config, const uint32_t *argb, int w,
                                   int h)
{
    if (!config.on() || w <= 0 || h <= 0) {
        reset();
        return nullptr;
    }
    const size_t count = size_t(w) * size_t(h);
    const bool same = texture_ && config == config_ && w == w_ && h == h_ &&
                      std::memcmp(last_.data(), argb, count * sizeof(uint32_t)) == 0;
    if (same)
        return texture_;

    if (!texture_ || config != config_ || w != w_ || h != h_) {
        reset();
        texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w * config.factor,
                                     h * config.factor);
        if (!texture_)
            return nullptr;
        SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
        config_ = config;
        w_ = w;
        h_ = h;
    }
    last_.assign(argb, argb + count);
    scaled_.resize(count * size_t(config.factor) * size_t(config.factor));
    config.scaler->scale(argb, w, h, config.factor, scaled_.data());
    SDL_UpdateTexture(texture_, nullptr, scaled_.data(), w * config.factor * int(sizeof(uint32_t)));
    return texture_;
}

void ScaledTexture::reset()
{
    if (texture_)
        SDL_DestroyTexture(texture_);
    texture_ = nullptr;
    config_ = {};
    w_ = h_ = 0;
    last_.clear();
}

void set_scale_mode_for(SDL_Texture *texture, float dst_w)
{
    const float ratio = dst_w / float(texture->w);
    const bool whole = std::fabs(ratio - std::round(ratio)) < 0.001f;
    SDL_SetTextureScaleMode(texture, whole ? SDL_SCALEMODE_NEAREST : SDL_SCALEMODE_LINEAR);
}

std::vector<uint32_t> upscale_argb(const UpscaleConfig &config, const uint32_t *argb, int w, int h)
{
    std::vector<uint32_t> out(size_t(w) * size_t(h) * size_t(config.factor) * size_t(config.factor));
    config.scaler->scale(argb, w, h, config.factor, out.data());
    return out;
}

} // namespace th
