#pragma once

#include "core/render/iso.h"
#include "core/ui/layer.h"
#include "core/ui/windows.h"
#include "core/world/events.h"

#include <cstdint>

namespace th
{

class Game;
class GameData;
struct RequesterArt;
struct World;

// The queue window (req7): World::rooms index `room` shown, the queue
// position under or held by the pointer, the drag state and the patient being
// moved, and the room's own queue. The game's fields are othervar's
// queue_win_room, queue_win_cell, queue_pos, queue_drag, queue_hilight_thing
// and gamevar.queue_selected_person.
struct QueueWindow {
    uint16_t room = 0;
    uint16_t cell = 0;
    uint8_t pos = 0;
    uint16_t hilight = 0;
    uint8_t drag = 0;
    uint16_t selected = 0;
    bool left_was_down = false;
    // The pointer, kept for the overdraw's highlights.
    int mouse_x = 0, mouse_y = 0;
    // The person the pointer is over, for the panel's information bar
    // (othervar.panel_thing) and the kind of bar its window is (2).
    uint16_t panel_thing = 0;
    uint8_t panel_info = 0;
    // A request to scroll the view to this cell (goto_column_row_in_window
    // from the left click on r25/r26), -1 for none.
    int goto_cell = -1;
    // The tooltip the overdraw asks the help system for (group 0x12) and
    // where, 0 for none.
    int help_group = 0;
    int help_x = 0, help_y = 0;
};

// The input a frame of the queue window reads besides the window itself.
struct QueueInputs {
    const GameData &data;
    const RequesterArt &art;
    int map_cell;
    const InterfaceEvents &events;
    const UiMouse &mouse;
};

// The queue window's part of command `event`, before Game::process_turn:
// EV_OPEN_QUEUE_WINDOW opens it on room `value` (cell `arg`) when no
// requester is up; EV_CLOSE_QUEUE_WINDOW closes it.
void queue_window_event(QueueWindow &q, Interface &ui, const World &world, const Event &event);

// The queue window's buttons and req7_overdraw's clicks after the Interface's
// frame: the capacity and close buttons (req7), the patient under the
// pointer, dragging one to the front, back or another room's queue
// (MM_MOVE_QUEUE_PERSON, EV_FRONT_QUEUE, EV_BACK_QUEUE,
// EV_QUEUE_PERSON_TO_NEW_QUEUE) and the right-click bar
// (EV_QUEUE_PERSON_GO_HOME, EV_QUEUE_PERSON_GO_RECEPTION). The commands go
// to Game::event, the sounds to World::building.
void queue_window_input(QueueWindow &q, Interface &ui, Game &game, const QueueInputs &in);

// display_queue_people's patient `id` standing at `at` (its feet): glowing
// with radiation (draw_glow_frame), wobbling with the phase of game turn
// `turn` with jellyitis (draw_wibble_frame), else drawn plain.
void draw_queue_person(UiLayer &layer, const World &world, const GameData &data, uint16_t id, Point at, uint32_t turn);

// req7_overdraw: the queue's labels (queue_text), its length, those on their
// way, admitted and limit, the patients (with a gap around the selected one)
// and, while the right button is held, the bar of what to do with one. A
// radiation patient glows and a jellyitis patient wobbles with the phase of
// the game turn.
void draw_queue_window(UiLayer &layer, const QueueWindow &q, const Interface &ui, const Game &game,
                       const RequesterArt &art, const GameData &data);

} // namespace th
