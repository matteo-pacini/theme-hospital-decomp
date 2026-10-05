#pragma once

#include "core/render/surface.h"

#include <cstdint>

namespace th
{

class GameData;

// Draws composite frame `frame_id` with its origin at (x, y), mirrored if
// `flip`, with `flags` (DrawFlags) added to each element's own, as the
// game's draw_a_frame. Returns the box the drawn elements start in (top-left)
// and the frame's own width and height; an empty rect if nothing was drawn.
//
// With `layers` (a thing's 13 layer choices), elements tagged with a layer
// are drawn only if their id is the one chosen for that layer, as
// draw_special_id_frame does; without it every element is drawn.
//
// With `extent`, it is set to the rectangle covering every sprite drawn,
// which may reach outside the returned box.
Rect draw_frame(Surface &dst, int x, int y, uint32_t frame_id, bool flip, unsigned flags, const GameData &data,
                const uint8_t *layers = nullptr, Rect *extent = nullptr);

constexpr int kLayerCount = 13;

} // namespace th
