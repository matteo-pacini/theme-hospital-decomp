#pragma once

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/render/surface.h"

#include <cstdint>
#include <vector>

namespace th
{

// Draws the bottom panel's normal icon set (PANEL02V) along the bottom of
// `dst`, centred horizontally, at its native size. Returns its rectangle.
Rect draw_panel(Surface &dst, const SpriteSheet &panel);

// The same panel as 0xAARRGGBB pixels, transparent where no sprite covers
// it, for drawing apart from the hospital view. Sets `width` and `height`.
std::vector<uint32_t> panel_to_argb(const SpriteSheet &panel, const Palette &palette, int &width, int &height);

} // namespace th
