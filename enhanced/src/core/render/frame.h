#pragma once

#include "core/render/iso.h"
#include "core/render/surface.h"

#include <cstdint>

namespace th
{

class GameData;

// draw_frame's optional settings.
struct FrameOptions {
    // DrawFlags added to each element's own.
    unsigned flags = 0;
    // A thing's 13 layer choices: elements tagged with a layer are drawn
    // only if their id is the one chosen for that layer, as
    // draw_special_id_frame does; without it every element is drawn.
    const uint8_t *layers = nullptr;
    // If given, set to the rectangle covering every sprite drawn, which may
    // reach outside the returned box.
    Rect *extent = nullptr;
};

// Draws composite frame `frame_id` with its origin at `at`, mirrored if
// `flip`, as the game's draw_a_frame. Returns the box the drawn elements
// start in (top-left) and the frame's own width and height; an empty rect if
// nothing was drawn.
Rect draw_frame(Surface &dst, Point at, uint32_t frame_id, bool flip, const GameData &data,
                const FrameOptions &options = {});

constexpr int kLayerCount = 13;

} // namespace th
