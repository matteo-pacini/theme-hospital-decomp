#pragma once

#include "core/data/palette.h"
#include "core/data/text.h"
#include "core/render/iso.h"
#include "core/render/surface.h"
#include "core/ui/layer.h"
#include "core/ui/windows.h"
#include "core/world/events.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace th
{

class Game;
class GameData;
struct World;

// What the hire staff requester draws with besides its sheet (REQ11V): the
// faces file and the font of its applicant counts.
class HireStaffArt
{
  public:
    // Loads QDATA/FACE01V.DAT (hires_faces, src/files.cpp) and font cache
    // slot 13. On failure says why in `error`.
    bool load(const std::filesystem::path &dir, std::string *error = nullptr);

    // The faces file: 18 heads, then 18 bodies, then 18 legs, each 65 pixels
    // wide (HiresCoords[162-165]: heads 29 rows, bodies 24, legs 22),
    // uncompressed palette indices.
    const std::vector<uint8_t> &faces() const { return faces_; }
    // Font cache slot 13 (QDATA/FONT24V).
    const Font &count_font() const { return count_font_; }

  private:
    std::vector<uint8_t> faces_;
    Font count_font_;
};

// The hire staff requester's state (the game's crequester while it is
// REQ_HIRE_STAFF) and the applicant or picked-up member of staff being
// placed (mouse mode MM_PLACE_STAFF: gamevar.held_staff and the cursor's
// cache).
struct HireStaff {
    // crequester.mode: 1 until a kind is chosen, then 2.
    uint8_t mode = 1;
    // crequester.arg: the staff kind shown (kStaff*).
    uint16_t kind = 0;
    // crequester.arg2: the applicant shown, 0 for none. Kept when the
    // requester closes, as in the game.
    uint16_t shown = 0;
    // The applicant or member of staff being placed and their kind; 0 when
    // not placing.
    uint16_t held = 0;
    uint16_t held_kind = 0;
    // draw_staff_cursor's last cell, and whether the held applicant may be
    // put down there (gamevar.staff_place_ok).
    int cursor_cell = -1;
    bool place_ok = false;

    // Whether the mouse mode is MM_PLACE_STAFF.
    bool placing() const { return held != 0; }
};

// req2_alter_staff (0x6aae0): the next (`forward`) or previous applicant of
// staff kind `kind` after thing `cur` in thing order, or the first one if
// `cur` is 0; `cur` if there is none further; 0 if there are none.
uint16_t alter_applicant(const World &world, bool forward, uint16_t cur, uint16_t kind);

// req2 (0x697e0): the command a click on the hire staff requester gives,
// from what the interface's frame did (its triggered button in window
// Requester::HireStaff): a kind button EV_HIRE_SELECT_KIND; the arrows
// EV_HIRE_NEXT_APPLICANT when there is another applicant; hire
// EV_MARK_EMPLOYING when a kind is chosen and the balance is positive and
// covers the wage; close the command of hire_staff_cancel. Otherwise (and
// for clicks the game refuses with a sound) EV_NONE.
Event hire_staff_click(const HireStaff &hire, const InterfaceEvents &events, const Game &game);

// setup_abort_all_requesters (0x1b1f8) while hiring or placing: Escape, a
// right click on the map while placing, or the requester's close button.
// EV_CANCEL_BUILD with the held applicant (0 for none) and the mouse mode.
Event hire_staff_cancel(const HireStaff &hire);

// The interface's part of command `event` (the player's half of
// process_turn, src/io.cpp): opening the requester (EV_OPEN_HIRE_STAFF:
// closes the build room requester and any open hire staff one, then, if no
// requester is open, opens it with no kind chosen and closes the patient,
// staff, handyman and machine windows), choosing a kind and paging
// (EV_HIRE_SELECT_KIND, EV_HIRE_NEXT_APPLICANT), taking the applicant to
// place (EV_MARK_EMPLOYING: closes the requester), and dropping everything
// (EV_CANCEL_BUILD: closes the requesters and stops placing). Call it with
// Game::event before Game::process_turn.
void hire_staff_event(HireStaff &hire, Interface &ui, const World &world, const Event &event);

// After Game::process_turn: stops placing once the held applicant was put
// down (place_staff_on_map ends MM_PLACE_STAFF), and starts placing a
// member of staff picked up (World::held_staff taken off the map,
// staff_picked_up's MM_PLACE_STAFF).
void hire_staff_after_turn(HireStaff &hire, const World &world);

// draw_staff_cursor's test (0x2b690), redone when `cell` (the map cell
// under the mouse, clamped to the map) changes: sets and returns
// hire.place_ok.
bool update_staff_cursor(HireStaff &hire, const World &world, int cell);

// A right click on the map in the normal mouse mode with no requester open
// and no room being edited (process_right_button, 0x5b95c), on `pointed`
// (PointedThing, core/render/view.h): EV_PICK_UP_STAFF if it is a member of
// player 0's staff, else EV_NONE (picking up objects is not ported). The
// game plays SFX_PICKUP with it.
Event pick_up_staff_click(const World &world, uint16_t pointed);

// A left click on the map at `cell` while placing (process_mouse,
// MM_PLACE_STAFF): EV_PLACE_STAFF if the held applicant may go there, else
// EV_NONE.
Event place_staff_click(HireStaff &hire, const World &world, int cell);

// req2_overdraw (0x69d34), after draw_interface while the requester is
// open: the chosen kind's button lit, the applicant count of each kind and,
// with a kind chosen, the shown applicant's name, wage, face, doctor
// qualifications (covered for other kinds), ability gauge and reference;
// otherwise a blank panel. Moves on to the next applicant if the shown one
// was taken. The advisor's flashing hints are not drawn.
void draw_hire_staff(UiLayer &layer, HireStaff &hire, const Interface &ui, const RequesterArt &art,
                     const HireStaffArt &hire_art, const World &world, const LanguageText &text);

// draw_staff_cursor's drawing: the held applicant standing on `cell`, facing
// +x, drawn into the hospital view as draw_world draws it (`view`,
// `camera`), in greys where they may not be put down. `scratch` is
// draw_world's: the size of `dst` and kept filled with kMaskColour.
void draw_held_staff(Surface &dst, Rect view, Point camera, const HireStaff &hire, const World &world, int cell,
                     const GameData &data, Surface &scratch);

// generate_bw_remap (0x1a5f4): for each of the first 255 colours of
// `palette`, the nearest grey in it (LbPaletteFindColour); entry 255 is 0.
std::array<uint8_t, 256> grey_remap(const Palette &palette);

} // namespace th
