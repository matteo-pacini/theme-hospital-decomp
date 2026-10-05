#pragma once

#include "core/data/sprites.h"
#include "core/render/surface.h"

namespace th
{

// Draws the bottom panel's normal icon set (PANEL02V) along the bottom of
// `dst`, centred horizontally, at its native size. Returns its rectangle.
Rect draw_panel(Surface &dst, const SpriteSheet &panel);

} // namespace th
