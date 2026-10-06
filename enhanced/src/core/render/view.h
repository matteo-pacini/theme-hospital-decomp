#pragma once

#include "core/render/iso.h"
#include "core/render/surface.h"

#include <vector>

namespace th
{

class GameData;
class ShadowSprites;
struct FrameSound;
struct World;

// The thing under the pointer (othervar.pointed_thing), as draw_world finds
// it: of the patients, staff, and objects and doors in the states
// mapwho_object tests, the last drawn whose frame's box holds `at` (in the
// surface's pixels) strictly inside; 0 for none.
struct PointedThing {
    Point at;
    uint16_t thing = 0;
};

// Where the hospital view is drawn: the rectangle `view` of `dst`, with
// `camera` the map pixel position at its top-left.
struct ViewTarget {
    Surface &dst;
    Rect view;
    Point camera;
};

// What draw_world reports of the things it draws, each if given: in
// `sounds`, every thing's frame drawn with its sound (0 for none), for
// SoundPlayer::frames_drawn; in `pointed`, the thing under its `at`
// (PointedThing).
struct DrawnThings {
    std::vector<FrameSound> *sounds = nullptr;
    PointedThing *pointed = nullptr;
};

// Draws the hospital view into `target`, as the game's
// draw_all_mk7_selective_sprites_wscreen (src/engine.cpp): every floor
// first (cells off the map as grass), then row by row from the back each
// row's walls followed by what stands on its cells (do_mapwho). Things on a
// cell behind a wall (map flag 0x10) are drawn through `scratch` with the
// walls of the cells in front of them masked out. With `shadows`, floors
// and north walls carry the shadows their cells' bits ask for.
//
// `scratch` must be the size of `target.dst`; its contents are kept filled
// with kMaskColour between calls.
void draw_world(const ViewTarget &target, const World &world, const GameData &data, const ShadowSprites *shadows,
                Surface &scratch, const DrawnThings &drawn = {});

// The colour the masked drawing treats as empty (the game's 0xfe); sprite
// pixels of this colour on masked cells are not drawn, as in the game.
constexpr uint8_t kMaskColour = 0xfe;

} // namespace th
