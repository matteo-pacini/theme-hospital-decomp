#pragma once

#include "core/render/iso.h"
#include "core/render/surface.h"

namespace th
{

class GameData;
class ShadowSprites;
struct World;

// Draws the hospital view into `view` of `dst`, with `camera` the map pixel
// position at the view's top-left, as the game's
// draw_all_mk7_selective_sprites_wscreen (src/engine.cpp): every floor
// first (cells off the map as grass), then row by row from the back each
// row's walls followed by what stands on its cells (do_mapwho). Things on a
// cell behind a wall (map flag 0x10) are drawn through `scratch` with the
// walls of the cells in front of them masked out. With `shadows`, floors
// and north walls carry the shadows their cells' bits ask for.
//
// `scratch` must be the size of `dst`; its contents are kept filled with
// kMaskColour between calls.
void draw_world(Surface &dst, Rect view, Point camera, const World &world, const GameData &data,
                const ShadowSprites *shadows, Surface &scratch);

// The colour the masked drawing treats as empty (the game's 0xfe); sprite
// pixels of this colour on masked cells are not drawn, as in the game.
constexpr uint8_t kMaskColour = 0xfe;

} // namespace th
