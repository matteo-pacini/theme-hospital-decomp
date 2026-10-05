#pragma once

#include "core/data/sprites.h"
#include "core/data/text.h"
#include "core/ui/layer.h"
#include "core/ui/menu_tables.h"

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace th
{

// What the menus' check marks show and what limits them, set by the caller
// each frame (the game's settings that the items' check bytes point at, as
// setup_pulldown_menu_options sets them).
struct MenuContext {
    bool sound = true;
    bool announcements = true;
    bool music = true;
    bool autosave = false;
    bool shadows = true;
    bool high_res = true;
    bool low_res = false;
    // Save slots 1-8 in use ([0] is slot 1): ticked in Load and Save.
    std::array<bool, 8> save_slots{};
    // The ticked volume items, 0 (100%) to 9 (10%): 9 - volume * 10 / 128
    // for a volume of 0-127.
    int sound_volume = 0;
    int announcement_volume = 0;
    int music_volume = 0;
    // The ticked speed item, 0 slowest to 4 (myplayer.speed_choice).
    int speed = 2;
    // Items not usable while paused or on the rat level are refused.
    bool paused = false;
    bool rat_level = false;
    // A modal requester is open (pmenu.disabled): the bar is hidden and
    // ignores the mouse.
    bool disabled = false;
};

// A sound the menus play (play_sound with SNDSRC_GENERAL).
enum class MenuSound : uint8_t {
    None,
    Highlight, // SFX_HLIGHT5: a menu opened or another item under the mouse
    Select,    // SFX_SELECTX: an item chosen
    Wrong,     // SFX_WRONG2: an item refused
};

// The result of a frame of the menu bar. When the left button is released
// over an item, `action` is what it asks for (MenuAction says what `value`
// holds); the caller performs it.
struct MenuEvent {
    MenuAction action = MenuAction::None;
    int value = 0;
    // The last sound of the frame.
    MenuSound sound = MenuSound::None;
};

// The pull-down menu bar (the game's pmenu, src/menu.cpp) with its art: the
// PULLDV sprites and the menu fonts, cache slots 1 (items) and 2 (the item
// under the mouse).
struct MenuBar {
    struct Pulldown {
        bool active = false;
        int text = 0;
        // Title length in characters and longest item, which set the width
        // on the bar and of the box at char_width a character.
        int title_length = 0;
        int width = 0;
        int count = 0;
        // Submenus: the pulldown holding the item that opens it and the y of
        // that item.
        int parent = 0;
        int y = 0;
        // Item `i` is kMenuItems[first + i]; submenu[i] the pulldown it
        // opens, 0 for none.
        int first = 0;
        std::array<int, 30> submenu{};
    };

    SpriteSheet sprites;
    Font font;
    Font font_hi;
    std::vector<std::string> text; // pulldown_text

    int line_height = 0;
    int char_width = 0;
    // Added to each text's y and to the line height (HiresCoords' pulldown
    // text offset, 2).
    int text_dy = 0;
    // pulldowns[1 .. count - 1] are the bar's menus, the submenus follow up
    // to subcount.
    std::array<Pulldown, 13> pulldowns{};
    int count = 0;
    int subcount = 0;

    // The open pulldown, 0 for none.
    int open = 0;
    // Shown this frame (othervar.menu_bar_active): the mouse is over the top
    // of the screen or a menu is open. While it is, the mouse belongs to the
    // menu bar: the caller passes no clicks to the view.
    bool active = false;

    // State for draw_menu_bar, as the game draws before handling the mouse.
    int drawn_open = 0;
    bool held = false;
    int mouse_x = -1;
    int mouse_y = -1;
    MenuContext context;
    // The item under the mouse when last drawn (selected_option, 1-based, and
    // last_p), to sound a change.
    int selected_option = 0;
    int last_p = 0;

    // Loads the art from `dir` and builds the menus from `lang`'s
    // pulldown_text (setup_menus). On failure says why in `error`.
    bool load(const std::filesystem::path &dir, const LanguageText &lang, std::string *error = nullptr);
};

// One frame of the menu bar with the mouse in the bar's layer coordinates
// (draw_menus' state, then process_menus): shows the bar while the mouse is
// in its top 22 lines or a menu is open; holding the left button over a
// title opens its menu, over an item with a submenu opens that; releasing it
// over an item of the open menu chooses it. Menus close when the button is
// up.
MenuEvent menu_frame(MenuBar &bar, const UiMouse &mouse, const MenuContext &context);

// Draws the bar as its last menu_frame left it, across the layer's width
// from its top-left corner, with the open menu while the button is held.
// The layer must be at least kUiHeight high to hold the longest menu.
void draw_menu_bar(UiLayer &layer, const MenuBar &bar);

} // namespace th
