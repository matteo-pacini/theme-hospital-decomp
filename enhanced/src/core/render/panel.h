#pragma once

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/render/surface.h"

#include <cstdint>
#include <vector>

namespace th
{

class GameData;

// Draws the bottom panel's normal icon set (PANEL02V) along the bottom of
// `dst`, centred horizontally, at its native size. Returns its rectangle.
Rect draw_panel(Surface &dst, const SpriteSheet &panel);

// What the panel shows of the hospital.
struct PanelFigures {
    int balance = 0;
    int day = 1;
    int month = 0;        // 0 January
    int reputation = 500; // 0 to 1000
    // Turns until the hospital opens (0: open, no watch) out of the level's
    // OpenCountdown.
    int opening_countdown = 0;
    int open_countdown = 0;
};

// The same panel as 0xAARRGGBB pixels, transparent where no sprite covers
// it, for drawing apart from the hospital view; with `figures`, also the
// balance, date and reputation marker as draw_money_on_panel,
// draw_date_on_panel and draw_rep_on_panel (src/panel.cpp) draw them
// (the balance not rolling), and the opening countdown's watch above the
// panel's left end. The image is bottom-aligned with the panel and tall
// enough for the watch. Sets `width` and `height`.
std::vector<uint32_t> panel_to_argb(const GameData &data, const PanelFigures *figures, int &width, int &height);

} // namespace th
