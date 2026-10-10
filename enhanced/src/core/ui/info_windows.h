#pragma once

#include "core/render/iso.h"
#include "core/render/surface.h"
#include "core/ui/layer.h"
#include "core/ui/windows.h"
#include "core/world/events.h"

#include <cstdint>
#include <string>
#include <vector>

namespace th
{

struct WindowArt;
struct World;

// The patient, staff, handyman and machine windows (src/req3.cpp,
// src/req4.cpp, src/req5.cpp): the game's query windows over the hospital
// (irequester_slots[1] to [3]), opened by clicking a person or a machine on
// the map. Which of them is open is the Interface's (Requester::Patient,
// Staff, Handyman or Machine); this is what they show.
struct InfoWindows {
    // irequester_slots[1].value to [3].value: the patient, member of staff
    // and machine shown.
    uint16_t patient = 0;
    uint16_t staff = 0;
    uint16_t machine = 0;
    // irequester_slots[1].show_graph: the patient window shows the health
    // graph, else the treatments; toggle_delay: frames before clicking it
    // toggles again.
    bool show_graph = true;
    uint8_t toggle_delay = 0;
    // othervar.right_button_was_down, as the windows' drawing keeps it.
    bool right_was_down = false;
};

// The in-game pointers (PTR_*, include/enums.h) hovering shows; the
// pointer sheet's sprites, each with its hotspot at its top-left corner
// (ingame_pointer_hotspots).
constexpr uint8_t kPointerKeep = 0; // hovering leaves the pointer as it was
constexpr uint8_t kPointerDefault = 0x01;
constexpr uint8_t kPointerStaff = 0x10;
constexpr uint8_t kPointerMachine = 0x11;
constexpr uint8_t kPointerPatient = 0x12;
constexpr uint8_t kPointerQueue = 0x13;
// The rat pointers (PTR_RAT_NEAR, PTR_RAT_AIM): near a rat and aimed at one.
constexpr uint8_t kPointerRatNear = 0x0a;
constexpr uint8_t kPointerRatAim = 0x0b;
// MM_MOVE_QUEUE_PERSON's pointer, PTR_MOVE_QUEUE_PERSON.
constexpr uint8_t kPointerMoveQueuePerson = 0x0f;
// The epidemic's vaccination pointer: the tool, over a patient it can
// vaccinate.
constexpr uint8_t kPointerVaccinate = 0x0c;
constexpr uint8_t kPointerVaccinateTarget = 0x0d;

// What the pointer is over on the map in the normal mouse mode
// (process_mouse's MM_NORMAL, src/mouse.cpp): with `hit`, `thing` is
// othervar.panel_thing (gamevar.hover_action 3) and `pointer` the pointer
// the game sets.
struct MapHover {
    bool hit = false;
    uint16_t thing = 0;
    uint8_t pointer = kPointerDefault;
    // The panel's information kind (othervar.panel_info): 1 staff, 2 patient,
    // 3 machine, 4 a queue, 5 a VIP or the inspector, 6 a special object, 0
    // none.
    uint8_t info = 0;
};

// process_mouse's MM_NORMAL test of `pointed` (othervar.pointed_thing,
// PointedThing in core/render/view.h): a patient or member of staff using
// the reception desk (the queue pointer, the person kept); using a machine,
// the machine (the machine pointer); of objects, a machine (by its first
// part) or one whose objects[].flags bit 1 is set (the pointer kept);
// otherwise a patient or member of staff (their pointers), a VIP or the
// inspector (the pointer kept), a door of a room with a queue (the queue
// pointer). Rats are not tested. The epidemic's vaccination pointer is set by the interface
// (app/main.cpp), not here.
MapHover map_hover(const World &world, uint16_t pointed);

// process_left_button's left click on the map in the normal mouse mode with
// no requester open (src/mouse.cpp), on `hover`: for the player's patient or
// member of staff, closes the query windows (close_irequesters) and gives
// EV_OPEN_THING_WINDOW, or with a pending call about them EV_ANSWER_PLAYERCALL
// (a resigning member of staff: 1; a patient waiting for a diagnosis room:
// 8, for a decision: 4). For the player's machine, EV_OPEN_THING_WINDOW (the
// machine window). A receptionist at her desk, a reception desk and the door of a queued room give EV_OPEN_QUEUE_WINDOW
// (the queue window). EV_NONE for the rest; poaching another player's staff needs network games. A rat under the
// pointer (kThingRat) gives EV_SHOOT_RAT, argb[1] 1: a hit. With `near_rat` (othervar.near_rat) and nothing under the
// pointer, a shot at nothing (argb[1] 0: a miss).
Event map_left_click(World &world, const MapHover &hover, Interface &ui, int cell = -1, bool near_rat = false);

// The windows' part of command `event` (process_turn, src/io.cpp), before
// Game::process_turn: EV_OPEN_THING_WINDOW opens the window for the player's
// patient (the health graph shown), member of staff (the handyman window
// for a handyman) or machine `value`, closing the others; EV_CLOSE_QUERY
// closes them; EV_PICK_UP_STAFF closes them; EV_CANCEL_SACK (after the
// yes/no requester closed) and EV_RETURN_TO_THING_WINDOW (the staff list
// leaving to `arg`, when no requester is open) open the window again.
void info_window_event(InfoWindows &info, Interface &ui, const World &world, const Event &event);

// What a frame of the windows asks of the caller.
struct InfoWindowResult {
    // The command to send (Game::event), EV_NONE for none.
    Event event;
    // With event EV_OPEN_YES_NO: the yes/no requester's question (language
    // section YesNo) and the commands for its answers.
    int question = 0;
    // The question itself when it needs arguments formatted into it (the
    // machine window's replacement question); empty for `question`.
    std::string question_text;
    Event yes;
    Event no;
    // Sound effects to play.
    std::vector<uint8_t> sounds;
    // othervar.center_on_slot: centre the hospital view on this thing
    // (info_centre), 0 for none.
    uint16_t centre_on = 0;
    // The casebook to open on this illness (req3 opens it directly, with no
    // command), 0 for none.
    uint8_t casebook = 0;
    // othervar.slist_staff, set with event EV_OPEN_SCREEN 5: the staff list
    // opens on this member of staff.
    uint16_t slist_staff = 0;
};

// What the windows' frame reads of the game and the interface: the hospital
// world, the level's language text, the level's scroll area (cells) and the
// player's balance.
struct InfoWindowContext {
    const World &world;
    const LanguageText &text;
    ScrollArea scroll;
    int balance;
};

// One frame of the open patient, staff, handyman or machine window, after
// the Interface's frame (`events`, `mouse` in its coordinates): req3, req4
// and req5 for the button pressed, then req3_overdraw's, req4_overdraw's and
// req5_overdraw's clicks on the window (left on the person's or machine's
// view: centre on it; right on it: the next of their kind, find_next_thing;
// left on the treatments: toggle the graph; left on a member of staff's face:
// the staff list), then process_tracking_windows: the windows close once
// their thing is beyond the level's scroll area widened by 10 cells, or gone.
// The patient window's queue button gives EV_OPEN_QUEUE_WINDOW for the patient's queue room.
InfoWindowResult info_window_frame(InfoWindows &info, Interface &ui, const InterfaceEvents &events,
                                   const UiMouse &mouse, const InfoWindowContext &context);

// find_next_thing (0x73bb0) for people: the next thing after `from` in thing
// order, wrapping round, of type `type` and the player's, of subtype `sub1`
// or `sub2` if `sub1` is not negative, which passes the game's scroll limit
// test (`scroll`) on its tile column and row taken as a pixel position (as the game
// passes them: in practice every one). The things table's size if none.
uint16_t find_next_thing(const World &world, uint16_t from, uint8_t type, int sub1, int sub2, ScrollArea scroll);

// Where thing `id` is drawn: the map pixel position of its feet (as
// draw_world places its frame).
Point thing_position(const World &world, uint16_t id);

// process_tracking_windows' centring of the hospital view on person `id`:
// the map pixel position for the view's centre (their body, as in
// info_view); the caller moves the view there unless it is beyond the
// level's scroll area.
Point info_centre(const World &world, uint16_t id);

// The open window's view of its person (the view window open_new_window
// opens over it, kept on them by process_tracking_windows): its box in the
// interface's coordinates and the map pixel position of its top-left, the
// person's body at its centre. False if no such window is open.
bool info_view(const InfoWindows &info, const Interface &ui, const World &world, Rect &box, Point &camera);

// req3_overdraw's and req4_overdraw's drawing, after draw_interface: the
// patient window's unavailable buttons covered, its happiness, thirst and
// warmth gauges and the health graph (in `red`, color.red) or treatments;
// the staff window's wage, qualifications (covered for all but doctors),
// face, happiness, tiredness and ability gauges and name; the handyman's
// wage and the three priorities' gauges.
void draw_info_windows(UiLayer &layer, const InfoWindows &info, const Interface &ui, const WindowArt &window_art,
                       const World &world, uint8_t red);

} // namespace th
