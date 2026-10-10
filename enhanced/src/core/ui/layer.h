#pragma once

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/render/surface.h"
#include "core/render/text.h"

#include <cstdint>
#include <string_view>
#include <vector>

namespace th
{

// The game's interface is laid out in its 640 x 480 screen coordinates.
constexpr int kUiWidth = 640;
constexpr int kUiHeight = 480;

// A transparent drawing layer for the game's interface (panel, requesters,
// menus) in those coordinates: an indexed surface plus which of its pixels
// were drawn, turned into an ARGB image drawn over the hospital view.
class UiLayer
{
  public:
    // Sizes the layer and clears it to transparent.
    void resize(int width, int height);
    void clear();

    int width() const { return pixels_.width(); }
    int height() const { return pixels_.height(); }

    // As th::draw_sprite, marking the sprite's opaque pixels drawn.
    void draw_sprite(int x, int y, const Sprite &sprite, unsigned flags = 0, const uint8_t *ghost = nullptr);
    // As th::draw_text, marking the glyphs' pixels drawn.
    void draw_text(const Font &font, int x, int y, std::string_view text, const TextSettings &settings);
    // Fills `r` with `colour` and marks it drawn.
    void fill(Rect r, uint8_t colour);

    // Fills `r` with black drawn at opacity `alpha` (0-255): the layer's
    // pixels stay transparent where nothing is drawn, so a ghost blend has no
    // backdrop to mix into; a translucent black over what is behind the layer
    // gives the same result.
    void shadow(Rect r, uint8_t alpha);

    // Blends `colour` into every drawn pixel through `ghost` (the game's
    // draw flag 4: ghost[colour << 8 | pixel]); undrawn pixels stay
    // transparent.
    void shade(uint8_t colour, const uint8_t *ghost);

    // The indexed pixels, for drawing that needs more than the above; mark
    // what it draws with mark().
    Surface &pixels() { return pixels_; }
    void mark(Rect r);

    // The layer as 0xAARRGGBB pixels, transparent where nothing was drawn.
    std::vector<uint32_t> to_argb(const Palette &palette) const;

  private:
    Surface pixels_;
    Surface drawn_; // the opacity drawn at each pixel (0 transparent, 255 opaque)
};

// The mouse as the interface sees it, in a layer's coordinates (the game's
// lbDisplay.mmouse_x/y and button state).
struct UiMouse {
    int x = -1;
    int y = -1;
    bool left_held = false;
    bool right_held = false;
    // Pressed or released since the previous frame.
    bool left_pressed = false;
    bool right_pressed = false;
    bool left_released = false;
    bool right_released = false;
};

// The modifier keys held, as the interface sees them.
struct UiModifiers {
    bool shift = false;
    bool ctrl = false;
};

} // namespace th
