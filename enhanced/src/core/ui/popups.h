#pragma once

#include "core/render/iso.h"
#include "core/render/panel.h"
#include "core/ui/layer.h"
#include "core/world/events.h"

#include <array>
#include <cstdint>

namespace th
{

class GameData;
struct World;

// The fax icons above the panel's left end, one per kind of call the player
// has (src/panel.cpp's draw_popup_icons; the calls' slots are kept by
// core/world/popups.h): a new one rises from behind the panel and slides
// left into its slot; the emergency's flashes. A click on one answers its
// kind's call (process_left_button's hover action 7).
struct PopupIcons {
    // Game frames run every 40 ms (limit_framerate); what is left over.
    double frame_time = 0;
    // othervar.emergency_anim_frame: the emergency icon's flash, a
    // kPanelEmergencySprites index.
    uint8_t emergency_frame = 0;
    // What the last game frame drew, in order: the icons still rising
    // (drawn before the panel's left end is redrawn over them) and then
    // those level with the slots, one step on; and every icon as it stood
    // before moving on, where the game tests the mouse. y is relative to
    // the icons' window; `hover_y` is the top of the box the game tests the
    // mouse in, 4 pixels above where it draws the icon.
    struct Icon {
        uint8_t kind = 0;
        int x = 0;
        int y = 0;
        int hover_y = 0;
    };
    std::array<Icon, 9> rising{};
    std::array<Icon, 9> level{};
    std::array<Icon, 9> hover{};
    uint8_t rising_count = 0;
    uint8_t level_count = 0;
    uint8_t hover_count = 0;
    // The panel's left end is redrawn between them.
    bool redraw = false;
    // The call kind (1-8) whose icon is under the mouse (the game's
    // othervar.hovered_popup_icon), 0 for none.
    uint8_t hovered = 0;
};

// The panel's fax icons layout (HiresCoords 0x89-0x95 and 0xb3, through
// PanelLayout, src/panel.cpp): the slots' x, the icons' window 4 pixels
// below the top of where they would stand on the panel, and where the
// emergency's flash goes on its icon.
inline constexpr std::array<int, 9> kPopupSlotX = {1, 30, 59, 88, 117, 146, 175, 531, 619};
inline constexpr int kPopupWindowDy = 4;
inline constexpr Point kPopupEmergencyFlash = {10, 9};

// One frame of the fax icons for the player (draw_popup_icons without its
// drawing): every 40 ms of `seconds`, a game frame: maintain_panel_popup,
// the emergency's flash, each head call's icon moving along its slide
// (panel_popup_coord) and the message bar's shutter, whose closing holds a
// new icon's slide back; then, every call, the icon under `mouse` (640 x
// 480 coordinates) as the game tests it in its last frame. Nothing on the
// rat level (World::Rats::fest_level).
void popup_icons_frame(PopupIcons &icons, World &world, const UiMouse &mouse, const GameData &data, double seconds);

// Draws the last game frame's icons into `layer` as draw_popup_icons does:
// the rising icons clipped to their window under the panel's top, the
// panel's left end redrawn over them (redraw_panel_left, with `state` and
// `figures`), then the icons level with their slots; the emergency's with
// its flash.
void draw_popup_icons(UiLayer &layer, const PopupIcons &icons, const GameData &data, const PanelState &state,
                      const PanelFigures *figures);

// process_left_button's click on a fax icon, with no requester open: the
// command answering the hovered kind's call (EV_ANSWER_PLAYERCALL, value
// the kind, arg 0: its first call); EV_NONE with none hovered. A right
// click on one does nothing.
Event popup_icon_click(const PopupIcons &icons);

} // namespace th
