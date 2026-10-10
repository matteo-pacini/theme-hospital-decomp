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
    // The size drawn at, 16.16 fixed point, as draw_a_frame_scaled: each
    // element's offset and size scaled (LbSpriteDrawScaled). The box
    // returned is unscaled, as the game's.
    int32_t scale = 0x10000;
    // A thing's status icon: the frame to draw above it (0 for none). A
    // person's icon hangs over the first head element (flags 0x200/0x250);
    // with `icon_over_frame` (a machine) it hangs over the frame's box
    // instead.
    uint32_t icon = 0;
    bool icon_over_frame = false;
    // draw_glow_frame: 0-255 draws the elements tagged with a layer blended
    // with this colour (kDrawGhost, as sprite_draw_glow) and the others
    // with colour 0 in their own flags; -1 draws as usual.
    int glow = -1;
    // Draws nothing but still returns the box and extent, as
    // draw_hilight_frame does on its hidden turn.
    bool hidden = false;
};

// icon_gap (HiresCoords' IconScreen, src/coords.cpp): the lift of a status
// icon above the thing's frame.
constexpr int kIconGap = 8;

// Draws composite frame `frame_id` with its origin at `at`, mirrored if
// `flip`, as the game's draw_a_frame. Returns the box the drawn elements
// start in (top-left) and the frame's own width and height; an empty rect if
// nothing was drawn.
Rect draw_frame(Surface &dst, Point at, uint32_t frame_id, bool flip, const GameData &data,
                const FrameOptions &options = {});

constexpr int kLayerCount = 13;

// draw_glow_frame's colour (glow_path, src/draw.cpp) for game turn `turn` and
// the thing's `layers`: the glow's colour cycles over eight steps of two
// turns, offset by two of its layer choices.
uint8_t glow_colour(uint32_t turn, const uint8_t *layers);

} // namespace th
