#pragma once

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/render/surface.h"
#include "core/ui/layer.h"

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
// it, for drawing apart from the hospital view: draw_panel_ui's drawing of
// the normal icon set, nothing pressed. The image is bottom-aligned with
// the panel and tall enough for the watch. Sets `width` and `height`.
std::vector<uint32_t> panel_to_argb(const GameData &data, const PanelFigures *figures, int &width, int &height);

// No panel icon (which_icon_is_mouse_over's 0x63).
inline constexpr int kNoPanelIcon = 0x63;

// The panel's icons of the raised set (panel_icons_over, src/array.cpp),
// left to right. In the normal set the first six are the same and the rest
// is the info bar.
enum PanelIcon : int {
    kIconBank = 0,
    kIconFigures = 1, // balance, reputation and date
    kIconBuildRoom = 2,
    kIconFurnishCorridor = 3,
    kIconEditRoom = 4,
    kIconHireStaff = 5,
    kIconStaffManagement = 6,
    kIconTownMap = 7,
    kIconCasebook = 8,
    kIconResearch = 9,
    kIconStatus = 10,
    kIconCharts = 11,
    kIconPolicy = 12,
};

// What a click on the panel asks for (panel_l_button, panel_r_button in
// src/panel.cpp), for the caller to do. The game first refuses every click
// with SFX_WRONG2 while an edited room empties (MM_EDIT_ROOM_WAIT), and
// plays SFX_SELECTX for what it does. The EV_OPEN_SCREEN numbers are
// chart_type's.
enum class PanelAction {
    None,
    // SFX_WRONG2 only: the figures, or a right click on an icon other than
    // the bank and hire staff.
    Refused,
    BankManager,   // left on the bank: EV_OPEN_SCREEN 6
    BankStatement, // right on the bank: EV_OPEN_SCREEN 0
    // Left on build room, furnish corridor and hire staff: refused while
    // placing a blueprint, door or staff member (MM_BLUEPRINT,
    // MM_PLACE_DOOR, MM_PLACE_STAFF, mouse mode 0x10); otherwise mouse mode
    // MM_NORMAL and EV_OPEN_BUILD_ROOM, EV_OPEN_FURNISH_CORRIDOR or
    // EV_OPEN_HIRE_STAFF.
    BuildRoom,
    FurnishCorridor,
    HireStaff,
    // Left on edit room, with no requester open: MM_NORMAL to MM_EDIT_ROOM
    // or back (clearing the hospital's edited_room); refused otherwise.
    EditRoom,
    StaffManagement, // left on staff management, right on hire staff: EV_OPEN_SCREEN 5
    // Left on town map: display_mode DM_TOWN_MAP, town.state 0; refused
    // while the edit room requester, or the buy objects one other than for
    // the corridor (crequester.arg 1), is open.
    TownMap,
    Casebook, // EV_OPEN_SCREEN 1
    Research, // EV_OPEN_SCREEN 3; refused without a research room
    Status,   // EV_OPEN_SCREEN 7
    Charts,   // EV_OPEN_SCREEN 4
    Policy,   // EV_OPEN_SCREEN 2
};

// The panel's interface state between frames.
struct PanelState {
    // The raised icon set is shown (othervar.panel_icon_set 2).
    bool raised = false;
    // The icon drawn pressed this frame (panel_ml_button), or kNoPanelIcon.
    int pressed = kNoPanelIcon;
    // A left press over the panel not yet acted on (lbDisplay.left_button):
    // the game acts on it once the button is up.
    bool left_click = false;
};

// The top of the panel in 640 x 480 coordinates (statvar.panel_top).
int panel_top(const GameData &data);

// which_icon_is_mouse_over: the icon of the raised or normal set at (x, y),
// or kNoPanelIcon above the panel or past its icons. An icon also covers
// the column of its right edge, which is the next icon's left: the left
// icon has it.
int panel_icon_at(const GameData &data, bool raised, int x, int y);

// One frame of the panel for `mouse` (640 x 480 coordinates), as
// choose_panel_icons and process_mouse's panel part: raises the icons while
// the mouse is over the panel and the game is not `paused`, draws the icon
// under a held button pressed, and returns what a left click (on release)
// or right click (on press) asks for. Without `input` (the game paused, or
// process_mouse returning earlier: the menu bar open, the mouse over a
// window or the interface modal) nothing is pressed or clicked. A left
// press elsewhere, or one dragged off the panel, is the map's.
PanelAction panel_frame(PanelState &state, const UiMouse &mouse, const GameData &data, bool paused, bool input = true);

// Draws the panel into `layer` (640 x 480 coordinates) as draw_panel and
// panel_ml_button (src/panel.cpp) do: the icon set `state` chooses, its
// pressed icon, and with `figures` the balance, date and reputation marker
// as draw_money_on_panel, draw_date_on_panel and draw_rep_on_panel draw
// them (the balance not rolling), and the opening countdown's watch above
// the panel's left end.
void draw_panel_ui(UiLayer &layer, const PanelState &state, const PanelFigures *figures, const GameData &data);

} // namespace th
