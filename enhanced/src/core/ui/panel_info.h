#pragma once

#include "core/data/text.h"
#include "core/ui/layer.h"

#include <cstdint>

namespace th
{

class Game;
class GameData;

// Where the info bar is drawn: the interface layer, the panel's top in 640 x
// 480 coordinates and the requesters' text font (the game's cache slot 1).
struct PanelInfoView {
    UiLayer &layer;
    int panel_y = 0;
    const Font &font;
};

// The panel's information bar (display_panel_*_info, src/panel.cpp): the
// message over the panel for the thing under the pointer (`info`, the
// panel_info kind from MapHover: 1 staff, 2 patient, 3 machine, 4 a queue, 6
// a special object).
void draw_panel_info(const PanelInfoView &view, uint8_t info, uint16_t thing, const Game &game, const GameData &data);

} // namespace th
