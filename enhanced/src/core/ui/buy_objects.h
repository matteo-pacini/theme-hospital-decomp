#pragma once

#include "core/data/palette.h"
#include "core/render/iso.h"
#include "core/render/surface.h"
#include "core/ui/layer.h"
#include "core/ui/windows.h"

#include <array>
#include <cstdint>

namespace th
{

class Game;
class GameData;
struct ObjectPlacing;

// The interface's own state for buying and placing objects, kept between
// frames by the caller; the game's part is World::placing.
struct ObjectUi {
    // The palette's greys, for an object that may not go where it is
    // (bw_remap), and its red (color.red).
    std::array<uint8_t, 256> grey{};
    uint8_t red = 0;
    // The buy objects requester's entry under the pointer, -1 for none.
    int hovered_entry = -1;
    // The kind in the edit room requester's list under the pointer
    // (hilighted_object), -1 for none.
    int highlighted = -1;
    // The object being placed is drawn this frame: the pointer is on the
    // map (draw_object_cursor runs only then).
    bool cursor_shown = false;
};

// draw_a_frame_scaled into `layer`: composite frame `frame` with its origin
// at `at`, mirrored if `flip`, its elements placed and sized by `scale`
// (16.16; 0x10000 draws it as draw_a_frame), each pixel from the nearest
// sprite pixel. Ghost flags are not applied.
void draw_layer_frame(UiLayer &layer, const GameData &data, uint32_t frame, Point at, bool flip, int scale);

// Sets up `ui` for the game's palette (generate_bw_remap, setup_colors).
void init_object_ui(ObjectUi &ui, const Palette &palette);

// The panel's furnish corridor icon (panel_l_button, src/panel.cpp) once the
// caller has checked the game's refusals (PanelAction::FurnishCorridor):
// stops placing (mouse mode MM_NORMAL) and sends EV_OPEN_FURNISH_CORRIDOR.
void furnish_corridor(Game &game);

// Before Interface::frame: shows the requester World::placing asks for and
// closes the other of the two (buy objects, window 10; edit room, window
// 5). Opening one closes any other requester first, as
// cancel_top_level_requesters closes the build room and hire staff ones.
// Does nothing while the interface is modal.
void sync_object_requesters(Interface &ui, const Game &game);

// After Interface::frame, with what it did: the buy objects requester's
// clicks (req11, clicked_buy_objects: an entry, its plus or minus buys or
// sells one, five with `keys.shift`, ten with `keys.ctrl`; accept places what was
// bought; cancel empties the list; furnishing a room, both go back to
// placing, cancel with the list as it was) and the entry under the pointer
// becoming its selection (req11_overdraw); the edit room requester's cancel
// (req14) and its list, where holding the left button on an entry places
// that kind next (req14_overdraw), which also fits the window's height to
// the list.
// The commands go to Game::event.
void object_requesters_input(Interface &ui, ObjectUi &oui, Game &game, const InterfaceEvents &events,
                             const UiMouse &mouse, UiModifiers keys);

// The map's part of process_mouse for objects, every frame: `on_map` is the
// pointer over the hospital view and not over a window, the panel or the
// menu bar, the interface not modal and the game not paused; `cell` the
// map cell under it (column + row * 128, -1 off the map). While placing it
// finds where the object would go (draw_object_cursor); a left click places
// it there if it may go (EV_PLACE_OBJECT), a right click turns it to its
// next facing first. Not placing, a right click while a requester of these
// is up cancels (as Escape), but for the edit room requester of a room built
// or furnished.
void placement_mouse(ObjectUi &oui, Game &game, int cell, const UiMouse &mouse, bool on_map);

// Escape (setup_abort_all_requesters, src/utils.cpp) while a requester of
// these is up, an object is being placed or a room built: while a room is
// furnished, the buy objects requester goes back to placing
// (EV_BACK_TO_PLACE_OBJECT, restoring the list); the edit room requester
// abandons the blueprint (EV_DELETE_ROOM) or takes the room back to one
// (EV_EDIT_BLUEPRINT); otherwise EV_CANCEL_BUILD, which drops what was
// bought and not placed. Returns whether it did. EV_CANCEL_BUILD's value is
// the held staff member, 0 here.
bool object_escape(Game &game);

// After draw_interface: the buy objects requester's own drawing
// (req11_overdraw: the title, the selected kind's price and picture, the
// total of the list, each entry's name and count, the one under the pointer
// in the highlight font) and the edit room requester's (req14_overdraw: the
// disabled buttons, the room's name, the instructions, the list of what is
// left to place with the selection boxed), for those that are open. The
// advisor's flashing hints are not drawn.
void draw_object_requesters(UiLayer &layer, const Interface &ui, const ObjectUi &oui, const Game &game,
                            const RequesterArt &art, const GameData &data);

// draw_object_cursor's drawing: the object being placed on its cell, in
// `view` of `dst` with `camera` as draw_world, over everything drawn (the
// game draws it after the view), in greys where it may not go. Nothing
// unless ObjectUi::cursor_shown.
void draw_placement(Surface &dst, Rect view, Point camera, const ObjectPlacing &p, const ObjectUi &oui,
                    const GameData &data);

} // namespace th
