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

} // namespace

Rect draw_panel(Surface &dst, const SpriteSheet &panel)
{
    int width = 0;
    int height = 0;
    for (int index : kPanelSprites) {
        Sprite s = panel.at(size_t(index));
        width += s.width;
        height = std::max(height, s.height);
    }
    int x = (dst.width() - width) / 2;
    for (int index : kPanelSprites) {
        Sprite s = panel.at(size_t(index));
        draw_sprite(dst, x, dst.height() - s.height, s);
        x += s.width;
    }
    return {(dst.width() - width) / 2, dst.height() - height, width, height};
}

} // namespace th
