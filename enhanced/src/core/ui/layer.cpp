#include "core/ui/layer.h"

#include "core/render/blit.h"

#include <algorithm>

namespace th
{

void UiLayer::resize(int width, int height)
{
    pixels_.resize(width, height);
    drawn_.resize(width, height);
}

void UiLayer::clear()
{
    pixels_.fill(0);
    drawn_.fill(0);
}

void UiLayer::draw_sprite(int x, int y, const Sprite &sprite, unsigned flags, const uint8_t *ghost)
{
    th::draw_sprite(pixels_, x, y, sprite, flags, ghost);
    drawn_.set_clip(pixels_.clip());
    draw_sprite_one_colour(drawn_, x, y, sprite, 1, flags);
    drawn_.reset_clip();
}

void UiLayer::draw_text(const Font &font, TextWindow window, int x, int y, std::string_view text, unsigned flags,
                        Rect clip, uint8_t colour)
{
    th::draw_text(pixels_, font, window, x, y, text, flags, clip, colour);
    th::draw_text(drawn_, font, window, x, y, text, flags | kTextOneColour, clip, 1);
}

void UiLayer::fill(Rect r, uint8_t colour)
{
    Rect c = {std::max(r.x, 0), std::max(r.y, 0), 0, 0};
    c.w = std::min(r.x + r.w, width()) - c.x;
    c.h = std::min(r.y + r.h, height()) - c.y;
    for (int y = c.y; y < c.y + c.h; y++)
        std::fill(pixels_.row(y) + c.x, pixels_.row(y) + c.x + std::max(c.w, 0), colour);
    mark(r);
}

void UiLayer::mark(Rect r)
{
    int x0 = std::max(r.x, 0), y0 = std::max(r.y, 0);
    int x1 = std::min(r.x + r.w, width()), y1 = std::min(r.y + r.h, height());
    for (int y = y0; y < y1; y++)
        std::fill(drawn_.row(y) + x0, drawn_.row(y) + std::max(x1, x0), uint8_t(1));
}

std::vector<uint32_t> UiLayer::to_argb(const Palette &palette) const
{
    std::vector<uint32_t> out(size_t(width()) * size_t(height()), 0);
    const uint8_t *p = pixels_.data();
    const uint8_t *d = drawn_.data();
    for (size_t i = 0; i < out.size(); i++)
        if (d[i])
            out[i] = palette[p[i]];
    return out;
}

} // namespace th
