#include "core/render/surface.h"

#include <algorithm>

namespace th
{

void Surface::resize(int width, int height, int scale)
{
    width_ = std::max(width, 0);
    height_ = std::max(height, 0);
    scale_ = std::max(scale, 1);
    pixels_.assign(size_t(pixel_width()) * size_t(pixel_height()), 0);
    reset_clip();
}

void Surface::put(int x, int y, uint8_t colour)
{
    for (int j = 0; j < scale_; j++) {
        uint8_t *out = row(y * scale_ + j) + x * scale_;
        std::fill(out, out + scale_, colour);
    }
}

void Surface::fill(uint8_t colour)
{
    std::fill(pixels_.begin(), pixels_.end(), colour);
}

void Surface::set_clip(Rect clip)
{
    int x0 = std::clamp(clip.x, 0, width_);
    int y0 = std::clamp(clip.y, 0, height_);
    int x1 = std::clamp(clip.x + clip.w, 0, width_);
    int y1 = std::clamp(clip.y + clip.h, 0, height_);
    clip_ = {x0, y0, std::max(x1 - x0, 0), std::max(y1 - y0, 0)};
}

} // namespace th
