#pragma once

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/render/surface.h"

namespace th
{

// Sprite draw flags, as the game's lbDisplay.draw_flags.
enum DrawFlags : unsigned {
    kDrawFlipX = 1,
    kDrawFlipY = 2,
};

// Draws `sprite` with its top-left corner at (x, y), clipped to the
// surface's clip rectangle. Transparent pixels leave the surface unchanged.
void draw_sprite(Surface &dst, int x, int y, const Sprite &sprite, unsigned flags = 0);

// Decodes `sprite` into 0xAARRGGBB pixels at `out` (rows `pitch` bytes
// apart, at least width x height), transparent pixels as 0.
void sprite_to_argb(const Sprite &sprite, const Palette &palette, uint32_t *out, int pitch);

} // namespace th
