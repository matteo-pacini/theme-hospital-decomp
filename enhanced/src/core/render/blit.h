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
    // See-through: each pixel mixed with what is under it through the ghost
    // table, the sprite's colour as the row (4) or the column (8).
    kDrawGhost = 4,
    kDrawGhostUnder = 8,
};

// Draws `sprite` with its top-left corner at (x, y), clipped to the
// surface's clip rectangle. Transparent pixels leave the surface unchanged.
// The ghost flags need `ghost` (GameData::ghost()); without it they draw
// solid.
void draw_sprite(Surface &dst, int x, int y, const Sprite &sprite, unsigned flags = 0, const uint8_t *ghost = nullptr);

// Draws `sprite`'s shape at (x, y) in the single colour `colour` (as
// LbSpriteDrawOneColour), mirrored by kDrawFlipX and kDrawFlipY in `flags`,
// clipped like draw_sprite.
void draw_sprite_one_colour(Surface &dst, int x, int y, const Sprite &sprite, uint8_t colour, unsigned flags = 0);

// Decodes `sprite` into 0xAARRGGBB pixels at `out` (rows `pitch` bytes
// apart, at least width x height), transparent pixels as 0.
void sprite_to_argb(const Sprite &sprite, const Palette &palette, uint32_t *out, int pitch);

} // namespace th
