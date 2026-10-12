// The main menu and the load game screen of Startup: the hover and click
// states and the save slots (src/mainmenu.cpp, src/loadscr.cpp).

#include "core/ui/startup_internal.h"

#include "core/render/blit.h"
#include "core/render/text.h"
#include "core/ui/helpmsg.h"
#include "core/world/savegame.h"

#include <algorithm>
#include <format>
#include <initializer_list>

namespace th
{

namespace
{

// play_sound's sfx numbers (include/enums.h).
constexpr uint8_t kSfxWrong2 = 0x08;
constexpr uint8_t kSfxHighlight = 0x44; // SFX_HLIGHT5

// The sprite of MAIN02M each hover state draws and where (draw_main_menu,
// with ScreenCoords 0xb3 to 0xbd of src/scoords.cpp), for states 3 to 7
// and 0x11 to 0x16.
struct MenuSprite {
    int state, x, y, sprite;
};
constexpr std::array<MenuSprite, 11> kMenuSprites = {{
    {kHoverContinue, 18, 23, 2},
    {kHoverLoad, 18, 22, 3},
    {kHoverNewGame, 18, 23, 1},
    {kHoverNetwork, 16, 23, 4},
    {kHoverQuit, 18, 23, 5},
    {0x11, 19, 23, 6},
    {0x12, 18, 23, 7},
    {0x13, 18, 23, 8},
    {0x14, 18, 23, 9},
    {0x15, 18, 23, 10},
    {0x16, 18, 23, 11},
}};

// The load screen's slot buttons and its Back button: slots 0 to 8 (the
// ninth button calls process_loadgame_buttons(9), which goes back).
constexpr int kLoadScreenSlots = 9;
constexpr Point kLoadBackButton = {595, 435};

// check_tri (src/mainmenu.cpp): (x, y) inside or on the triangle, its
// corners counterclockwise on screen. Coordinates are 16-bit unsigned, as
// the game passes them.
bool check_tri(Point p1, Point p2, Point p3, int mx, int my)
{
    const int x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y, x3 = p3.x, y3 = p3.y;
    int x = uint16_t(mx), y = uint16_t(my);
    auto side = [](int64_t a, int64_t b, int64_t c, int64_t d) { return a * d - b * c > 0; };
    return !(side(x2 - x1, y2 - y1, x - x1, y - y1) || side(x3 - x2, y3 - y2, x - x2, y - y2) ||
             side(x1 - x3, y1 - y3, x - x3, y - y3));
}

} // namespace

namespace
{

// The main menu's entries, as display_main_menu and tri_test number them.
enum MenuEntry : int { kEntryNewGame, kEntryLoad, kEntryContinue, kEntryNetwork, kEntryQuit };

// tri_test's test of entry `entry`'s triangles.
bool over_menu_entry(int entry, int mx, int my)
{
    switch (entry) {
    case kEntryNewGame:
        return check_tri({0x2c, 0x13}, {0x55, 0x3a}, {0x7f, 0x13}, mx, my);
    case kEntryLoad:
        return check_tri({0x55, 0x3a}, {0x7f, 0x68}, {0x7f, 0x13}, mx, my);
    case kEntryContinue:
        return check_tri({0x2c, 0x13}, {0x2c, 0x68}, {0x55, 0x3a}, mx, my);
    case kEntryNetwork:
        return check_tri({0x2c, 0x68}, {0x7f, 0x68}, {0x55, 0x3a}, mx, my);
    default:
        return check_tri({0x30, 0x77}, {0x30, 0x96}, {0x74, 0x79}, mx, my) ||
               check_tri({0x74, 0x79}, {0x30, 0x96}, {0x74, 0x96}, mx, my);
    }
}

// The menu state for hovering `target` from `state`: a turning state when
// another entry was hovered (`from`: hovered state, turning state).
int turn_menu(int state, int target, std::initializer_list<std::pair<int, int>> from)
{
    if (state == target)
        return state;
    for (auto [s, to] : from) {
        if (state == s)
            return to;
    }
    return target;
}

// display_main_menu and tri_test: the entry under the mouse becomes the
// hovered one, through a turning state when another was.
int hover_main_menu(int state, int mx, int my)
{
    if (over_menu_entry(kEntryNewGame, mx, my))
        return turn_menu(state, kHoverNewGame,
                         {{kHoverContinue, 0x11}, {kHoverLoad, 0x13}, {kHoverNetwork, 0x12}, {kHoverQuit, 0x12}});
    if (over_menu_entry(kEntryLoad, mx, my))
        return turn_menu(state, kHoverLoad,
                         {{kHoverNewGame, 0x13}, {kHoverContinue, 0x12}, {kHoverNetwork, 0x14}, {kHoverQuit, 0x14}});
    if (over_menu_entry(kEntryContinue, mx, my))
        return turn_menu(state, kHoverContinue,
                         {{kHoverNewGame, 0x11}, {kHoverLoad, 0x12}, {kHoverNetwork, 0x16}, {kHoverQuit, 0x16}});
    if (over_menu_entry(kEntryNetwork, mx, my))
        return turn_menu(state, kHoverNetwork,
                         {{kHoverNewGame, 0x12}, {kHoverLoad, 0x14}, {kHoverContinue, 0x16}, {kHoverQuit, 0x15}});
    if (over_menu_entry(kEntryQuit, mx, my))
        return turn_menu(state, kHoverQuit,
                         {{kHoverNewGame, 0x12}, {kHoverLoad, 0x14}, {kHoverContinue, 0x16}, {kHoverNetwork, 0x15}});
    if (state >= 0x11 && state <= 0x16)
        return kHoverContinue;
    return state;
}

// process_mouse_main_menu: a release over an entry chooses it, the last
// one tested winning.
int release_main_menu(int state, int mx, int my)
{
    constexpr std::pair<int, int> kChoices[] = {
        {kEntryNewGame, kNewGame}, {kEntryLoad, kLoadGame}, {kEntryContinue, kContinue},
        {kEntryNetwork, kNetwork}, {kEntryQuit, kQuit},
    };
    for (auto [entry, chosen] : kChoices)
        if (over_menu_entry(entry, mx, my))
            state = chosen;
    return state;
}

} // namespace

void Startup::main_menu(const UiMouse &mouse, std::span<const StartupKey> keys)
{
    // draw_main_menu.
    int shown = menu_state_;
    switch (menu_state_) {
    case kNewGame:
        play_movie(kMovieMainMenu2, State::PrefsEnter);
        level_ = 0;
        return;
    case kQuit:
        play_movie(kMovieMainMenu2, State::Quit);
        return;
    case kLoadGame:
        // MMS_LOAD_GAME: the closing movie, then the load game screen.
        play_movie(kMovieMainMenu2, State::LoadEnter);
        return;
    case kContinue:
        // MMS_CONTINUE: the Continue slot (0x63), or the wrong sound without
        // one (save_game_valid).
        if (load_slot_used(kContinueSlot)) {
            result_ = {};
            result_.kind = StartupResult::Kind::LoadGame;
            result_.slot = kContinueSlot;
            screen_ = StartupScreen::Done;
            state_ = State::Done;
        } else {
            menu_state_ = kHoverContinue;
            sound(kSfxWrong2);
        }
        return;
    case kNetwork: // not ported: the original opens network select
        // The original plays no sound for it.
        menu_state_ = kHoverContinue;
        return;
    default:
        break;
    }

    show(main_menu_);
    menu_state_ = hover_main_menu(menu_state_, mouse.x, mouse.y);
    for (const MenuSprite &s : kMenuSprites) {
        if (s.state == shown)
            draw_sprite(surface_, s.x, s.y, main_sprites_.at(size_t(s.sprite)));
    }
    if (shown >= 0x11)
        sound(kSfxHighlight);

    // tri_test's help (src/mainmenu.cpp): the hovered entry's tooltip, by its
    // absolute hotspot index in help group 1, drawn over the menu.
    main_menu_help(mouse);

    // process_keys_main_menu: Escape or Q quits at once.
    for (const StartupKey &k : keys) {
        if (k.code == StartupKey::Escape || (k.code == StartupKey::Letter && std::toupper(k.letter) == 'Q')) {
            state_ = State::Quit;
            return;
        }
    }
    if (mouse.left_released)
        menu_state_ = release_main_menu(menu_state_, mouse.x, mouse.y);
}

void Startup::main_menu_help(const UiMouse &mouse)
{
    if (!help_ || !help_ctx_)
        return;
    int hotspot = 0;
    if (over_menu_entry(kEntryNewGame, mouse.x, mouse.y))
        hotspot = 1;
    else if (over_menu_entry(kEntryLoad, mouse.x, mouse.y))
        hotspot = 2;
    else if (over_menu_entry(kEntryContinue, mouse.x, mouse.y))
        hotspot = 3;
    else if (over_menu_entry(kEntryNetwork, mouse.x, mouse.y))
        hotspot = 4;
    else if (over_menu_entry(kEntryQuit, mouse.x, mouse.y))
        hotspot = 5;
    if (!hotspot)
        return;
    help_ctx_->mouse = &mouse;
    help_->box().screen_pic_loaded = true;
    // The main menu measures and draws with font cache 0x2d
    // (display_mode == DM_MAIN_MENU in set_help_text/draw_help_text).
    help_->set_main_menu(true);
    help_->help_hotspot_detected(*help_ctx_, uint16_t(hotspot), 1, 0, 0);
    help_->draw(surface_, palette_);
    help_->set_main_menu(false);
    help_->box().screen_pic_loaded = false;
}

// The load game screen's states, one per tick.
void Startup::load_tick(const UiMouse &mouse, std::span<const StartupKey> keys)
{
    switch (state_) {
    case State::LoadEnter:
        enter_load();
        break;
    case State::LoadDelay:
        zoom_after_delay(prefs_, 0x2000, State::LoadZoom);
        break;
    default:
        load_screen(mouse, keys);
        break;
    }
}

// loadscr's state 0: the load game screen over PREF01V, then its panels
// open (init_zoom_boxes(0x2000)).
void Startup::enter_load()
{
    screen_ = StartupScreen::LoadGame;
    calculate_grey(prefs_);
    load_selected_ = 0xfa;
    timer_ = kDelay800ms;
    state_ = State::LoadDelay;
}

// Whether slot `slot` holds a save readable by this build: its file exists
// and starts with this build's snapshot header (loadscr's LbFileLengthRnc
// check and save_game_valid).
bool Startup::load_slot_used(int slot) const
{
    if (save_dir_.empty())
        return false;
    return save_slot_level(save_slot_path(save_dir_, name_, slot)) >= 0;
}

// loadscr's states 3 and 4: the zooming panel, then its slot buttons.
void Startup::load_screen(const UiMouse &mouse, std::span<const StartupKey> keys)
{
    if (state_ == State::LoadZoom) {
        if (current_display_ == desired_display_)
            state_ = State::LoadEntry;
        draw_zoom_boxes();
        return;
    }
    // State 4: process_keys_load_game (Escape goes back) and the buttons.
    for (const StartupKey &k : keys) {
        if (k.code == StartupKey::Escape) {
            sound(kSfxNo);
            state_ = State::MenuEnter;
            return;
        }
    }
    show(prefs_);
    draw_zoom_boxes();
    draw_load_screen(mouse);
    if (current_display_ & 0x2000)
        draw_load_buttons(mouse);
}

// loadscr's display_loadgame_screen: each slot's label on its button, the
// slot's number when it holds a save and the empty text otherwise.
void Startup::draw_load_screen(const UiMouse &mouse)
{
    std::string_view used = text_->at(TextSection::Load, 0);
    std::string_view empty = text_->at(TextSection::Load, 1);
    for (int slot = 0; slot < kLoadScreenSlots; slot++) {
        const GlassBox &b = kLoadGlass[size_t(2 + slot)];
        std::string label = load_slot_used(slot) ? std::format("{} {}", used, slot) : std::string(empty);
        bool held = (mouse.left_held || mouse.left_released) &&
                    over(mouse.x, mouse.y, b.x, b.y, b.x1 - b.x + 1, b.y1 - b.y + 1);
        bool down = held;
        const Font &font = fonts_[down ? kFontGlassHi : kFontGlass];
        int w = b.x1 - b.x + 1;
        int x = ((w - text_width(font, label)) >> 1) + (down ? 1 : 0);
        int y = ((b.y1 + b.y + 1 - font.glyph('A').height) >> 1) + (down ? 1 : 0);
        draw_text(surface_, font, x, y, label, {{b.x, 0, w}, {0, 0, surface_.width(), surface_.height()}});
    }
}

// process_mouse_load_game: the Back button and the slot buttons, highlighted
// under the mouse, taking a release.
void Startup::draw_load_buttons(const UiMouse &mouse)
{
    const Sprite &back = prefs_sprites_.at(2);
    draw_sprite(surface_, kLoadBackButton.x, kLoadBackButton.y, back);
    State next = state_;
    load_button(mouse, kLoadScreenSlots, true, next);
    for (int slot = 0; slot < kLoadScreenSlots; slot++)
        load_button(mouse, slot, false, next);
    state_ = next;
}

// loadscr's process_loadgame_buttons: slot 9 goes back, another loads the
// save if it is valid. `next` receives the state to go on to.
void Startup::load_button(const UiMouse &mouse, int slot, bool back, State &next)
{
    Rect r;
    if (back) {
        const Sprite &s = prefs_sprites_.at(2);
        r = {kLoadBackButton.x, kLoadBackButton.y, s.width, s.height};
    } else {
        const GlassBox &b = kLoadGlass[size_t(2 + slot)];
        r = {b.x, b.y, b.x1 - b.x + 1, b.y1 - b.y + 1};
    }
    bool full = current_display_ & 0x2000;
    if (mouse.left_released && full && over(mouse.x, mouse.y, r.x, r.y, r.w, r.h)) {
        sound(kSfxYes);
        if (back) {
            next = State::MenuEnter;
        } else if (load_slot_used(slot)) {
            result_ = {};
            result_.kind = StartupResult::Kind::LoadGame;
            result_.slot = slot;
            screen_ = StartupScreen::Done;
            next = State::Done;
        }
        return;
    }
    if (full && over(mouse.x, mouse.y, r.x, r.y, r.w, r.h) && load_selected_ != slot) {
        sound(kSfxHighlight);
        load_selected_ = slot;
    }
}

} // namespace th
