#pragma once

#include "core/render/view.h"
#include "core/ui/layer.h"
#include "core/ui/windows.h"

namespace th
{

class Game;
class GameData;
struct ObjectUi;

// The interface's own state for the build tool, kept between frames by the
// caller; the game's part is World::building and World::placing.
struct BuildUi {
    // The build room requester's entry under the pointer, -1 for none.
    int hovered_entry = -1;
    // The map cell under the pointer the frame before (othervar.prev_mouse_cell),
    // -1 off the map, which EV_TEMP_PLACE_DOOR looks from.
    int prev_cell = -1;
    // The map cell under the pointer this frame, -1 when the pointer is not
    // on the map.
    int cell = -1;
};

// Whether the panel refuses the build room, furnish corridor and hire staff
// icons for the build tool (panel_l_button): while the blueprint is laid
// out (MM_BLUEPRINT), the door placed (MM_PLACE_DOOR) or the room waits to
// empty (mouse mode 0x10).
bool build_tool_busy(const Game &game);

// The panel's build room icon (panel_l_button, src/panel.cpp) once the
// caller has checked the game's refusals (PanelAction::BuildRoom): stops
// placing (mouse mode MM_NORMAL) and sends EV_OPEN_BUILD_ROOM.
void open_build_room(Game &game);

// Before Interface::frame: shows the build room requester (window 9) while
// World::placing asks for it, closing any other requester first, and closes
// it otherwise. Does nothing while the interface is modal.
void sync_build_requester(Interface &ui, const Game &game);

// After Interface::frame, with what it did: the build room requester's
// clicks (req10: a department lists its rooms, EV_REQ1_RESET_ROOMS; a room
// starts its blueprint, EV_INIT_BLUEPRINT, if the balance is above its
// price and its compulsory objects'; close, EV_CLOSE_REQUESTERS) and the
// room under the pointer becoming its selection (req10_overdraw); the edit
// room requester's buy more (EV_FURNISH_EDITED_ROOM) and confirm (req14: by
// the step, EV_COMMIT_BLUEPRINT, EV_COMMIT_DOORS or EV_OPEN_ROOM) while
// enabled. Its pick up button is not ported. The commands go to
// Game::event, the sounds to Building::sounds.
void build_requesters_input(Interface &ui, BuildUi &bui, Game &game, const InterfaceEvents &events,
                            const UiMouse &mouse);

// The map's part of process_mouse for the build tool, every frame, with
// `on_map` and `cell` as for placement_mouse (core/ui/buy_objects.h): laying
// the blueprint out, a left press or drag reshapes it (EV_MODIFY_BLUEPRINT)
// and no button updates its pointer and ends a drag (EV_HOVER_BLUEPRINT);
// placing the door, a left click puts the door or a window where it is
// shown and a right click takes a window out (EV_PLACE_BP_DOOR), no button
// shows it nearest the pointer (EV_TEMP_PLACE_DOOR).
void build_mouse(BuildUi &bui, Game &game, int cell, const UiMouse &mouse, bool on_map);

// After draw_interface, while the build room requester is open, its own
// drawing (req10_overdraw): the instructions, the departments (the one
// shown in the highlight font), and once one is chosen its rooms (the one
// under the pointer highlighted), the selected room's main object and its
// price with its compulsory objects'. The advisor's flashing hints are not
// drawn.
void draw_build_requester(UiLayer &layer, const Interface &ui, const BuildUi &bui, const Game &game,
                          const RequesterArt &art, const GameData &data);

// The build tool's drawing over the hospital view, into `target` as
// draw_world: the blueprint being laid out (draw_blueprint_area:
// its floor, block 66 where the room may not go, and its outline, in red
// where invalid) with the cell under the pointer outlined in red
// (draw_cell_no), and while the door is placed the door or window shown
// (temp_place_door: its floor and its picture, red where it may not go).
// `oui` gives the red.
void draw_build_overlay(const ViewTarget &target, const Game &game, const BuildUi &bui, const ObjectUi &oui,
                        const GameData &data);

} // namespace th
