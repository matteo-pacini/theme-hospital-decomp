#pragma once

#include "core/world/events.h"
#include "core/world/game.h"

namespace th
{

// The fax screen's hold on the game's colours (src/choice.cpp, src/screens.cpp
// and the palette fade of the frame loop in src/game.cpp). Showing a fax loads
// its picture and palette, and the frame's setup_colors then finds color.red
// and color.dark_blue in that palette (World::fax_palette) until the screen
// the fax replaced is back. The interface's fax draws the screen; this keeps
// the state the colours follow, a turn at a time.

// display_choice_screen (0xa410c): a fax of kind `kind` (kFax*) and of
// `timeout` turns without the pointer moving (0: it must be answered) is
// called up over what is shown; the frame after draws it.
void fax_open(Game &game, uint8_t kind, int16_t timeout);

// restore_display_screen (0xa4208), after an answer: the fax closes, and the
// screen under it, another fax or the game, is back.
void fax_restore(Game &game);

// return_from_screen (0xb73a0) for a fax: the game is back, whatever was
// under it.
void fax_return_to_game(Game &game);

// process_turn's command `event` as it affects the fax: an answer closes it
// (EV_CHOICE_RESULT, unless the epidemic cover-up it is given quietly), and so
// does starting an emergency.
void fax_event(Game &game, const Event &event);

// display_choice (0xa3158), the frame's step for a fax: showing it, taking
// the pointer and the keys for the time to close it, closing it.
void fax_frame(Game &game);

// The palette the fax or its closing chose (othervar.fade_palette) takes
// effect, as it does at the end of the game's frame; a change of colours is
// traced. Run it at the start of a turn, after its commands.
void fax_fade(Game &game);

} // namespace th
