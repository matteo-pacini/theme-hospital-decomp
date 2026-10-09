#pragma once

#include "core/data/text.h"
#include "core/ui/layer.h"

#include <cstdint>
#include <string_view>

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
    // The panel's raised icon set is shown (othervar.panel_icon_set 2): the
    // bar is not drawn then, as update_panel only fills it while the set is 1.
    bool raised = false;
};

// The panel's information bar (display_panel_*_info, src/panel.cpp): the
// message over the panel for the thing under the pointer (`info`, the
// panel_info kind from MapHover: 1 staff, 2 patient, 3 machine, 4 a queue, 6
// a special object). Returns `info` when the bar was drawn and 0 when it was
// not, which is the case while the panel's raised icon set is shown.
uint8_t draw_panel_info(const PanelInfoView &view, uint8_t info, uint16_t thing, const Game &game,
                        const GameData &data);

// display_panel_special_info: `message` on the bar (a panel message, e.g. a
// save's, from the language's misc_text).
void draw_panel_message(const PanelInfoView &view, std::string_view message, const GameData &data);

} // namespace th
