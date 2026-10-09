#pragma once

#include "core/render/frame.h"
#include "core/render/surface.h"

#include <cstdint>

namespace th
{

class GameData;

// The game's pixBuf as draw_wibble_frame uses it: 180 x 180 palette indices
// (kMaskColour for nothing), a frame's elements drawn into it before they
// wobble onto the screen.
Surface make_wibble_buffer();

// What draw_wibble_frame draws: frame `frame` with its origin at `at`, mirrored if
// `flip`, wobbling with the phase of game turn `turn`.
struct WibbleFrame {
    Point at;
    uint32_t frame;
    bool flip;
    uint32_t turn;
};

// draw_wibble_frame (src/draw.cpp): a jellyitis patient's frame onto `dst`. The
// elements tagged with a layer (`options.layers` picks which are drawn) go
// through `buffer` and wobble with the phase turn % 17 (draw_wibble); the
// others are drawn plain. No status icon is drawn. Returns the box drawn plus
// 4 pixels on each side, or an empty rect if nothing was drawn. `buffer` is
// left clear.
Rect draw_wibble_frame(Surface &dst, Surface &buffer, const WibbleFrame &f, const GameData &data,
                       const FrameOptions &options);

} // namespace th
