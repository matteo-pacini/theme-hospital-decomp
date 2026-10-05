#include "core/render/panel.h"

#include "core/render/blit.h"

#include <algorithm>
#include <array>

namespace th
{

namespace
{

// Sprites of panel_icons_normal (src/array.cpp), laid left to right and
// bottom-aligned as setup_panel_icons does.
constexpr std::array<int, 33> kPanelSprites = {
    1,  3,  4,  6,  8,  10, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14,
};

void panel_size(const SpriteSheet &panel, int &width, int &height)
{
    width = 0;
    height = 0;
    for (int index : kPanelSprites) {
        Sprite s = panel.at(size_t(index));
        width += s.width;
        height = std::max(height, s.height);
    }
}

} // namespace

Rect draw_panel(Surface &dst, const SpriteSheet &panel)
{
    int width, height;
    panel_size(panel, width, height);
    int x = (dst.width() - width) / 2;
    for (int index : kPanelSprites) {
        Sprite s = panel.at(size_t(index));
        draw_sprite(dst, x, dst.height() - s.height, s);
        x += s.width;
    }
    return {(dst.width() - width) / 2, dst.height() - height, width, height};
}

std::vector<uint32_t> panel_to_argb(const SpriteSheet &panel, const Palette &palette, int &width, int &height)
{
    panel_size(panel, width, height);
    std::vector<uint32_t> pixels(size_t(width) * size_t(height), 0);
    int x = 0;
    for (int index : kPanelSprites) {
        Sprite s = panel.at(size_t(index));
        sprite_to_argb(s, palette, pixels.data() + size_t(height - s.height) * size_t(width) + size_t(x), width * 4);
        x += s.width;
    }
    return pixels;
}

} // namespace th
