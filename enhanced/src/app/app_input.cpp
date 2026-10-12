// Menu bar, event, key and mouse handling (src/keyboard.cpp, src/mouse.cpp,
// src/io.cpp's key handling).

#include "app/app.h"
#include "app/app_internal.h"
#include "app/web.h"
#include "core/data/gamedata.h"

#include <imgui_impl_sdl3.h>

#include <algorithm>

namespace th
{

namespace
{

// The scroll bit an arrow or numpad key asks for, or 0 (src/keyboard.cpp's
// KC_UP/KC_NUMPAD8 and so on).
uint8_t scroll_key_bit(SDL_Keycode key)
{
    switch (key) {
    case SDLK_UP:
    case SDLK_KP_8:
        return kScrollUp;
    case SDLK_DOWN:
    case SDLK_KP_2:
        return kScrollDown;
    case SDLK_LEFT:
    case SDLK_KP_4:
        return kScrollLeft;
    case SDLK_RIGHT:
    case SDLK_KP_6:
        return kScrollRight;
    default:
        return 0;
    }
}

// The game speed a Speed menu item's value picks.
Speed menu_speed(int value)
{
    return value <= 10   ? Speed::Slowest
           : value <= 20 ? Speed::Slower
           : value <= 30 ? Speed::Normal
           : value <= 40 ? Speed::Max
                         : Speed::AndThenSome;
}

} // namespace

// Runs and draws the menu bar for the frame: it spans the window's width
// (at least the game's 640) across the top.
void App::menu_step(int out_w, int scale)
{
    int width = std::max(kUiWidth, out_w / scale);
    int ox = (out_w - width * scale) / 2;
    MenuContext context;
    context.shadows = shadows_;
    context.autosave = details_.autosave != 0;
    context.sound = sounds_.sfx_on();
    context.sound_volume = sounds_.volume_item();
    context.music = music_.music_on();
    context.music_volume = music_.volume_item();
    context.announcements = sounds_.ambients_on();
    context.announcement_volume = sounds_.ambient_volume_item();
    context.speed = speed_ == Speed::Slowest  ? 0
                    : speed_ == Speed::Slower ? 1
                    : speed_ == Speed::Normal ? 2
                    : speed_ == Speed::Max    ? 3
                                              : 4;
    context.paused = map_ && map_->game.paused;
    context.rat_level = map_ && map_->game.world.rats.fest_level;
    context.disabled = interface_.modal() || (map_ && map_->game.world.placing.menu_disabled);
    for (int i = 0; i < int(context.save_slots.size()); i++)
        context.save_slots[size_t(i)] = save_slots_[size_t(i + 1)];
    perform(menu_frame(menu_, ui_mouse(ox, 0, scale), context));

    if (menu_layer_.width() != width)
        menu_layer_.resize(width, kUiHeight);
    menu_layer_.clear();
    draw_menu_bar(menu_layer_, menu_);
    shade_ui(menu_layer_);
    std::vector<uint32_t> pixels = menu_layer_.to_argb(data_->palette());
    SDL_FRect dst = {float(ox), 0, float(width * scale), float(kUiHeight * scale)};
    if (menu_tex_ && menu_tex_->w != width) {
        SDL_DestroyTexture(menu_tex_);
        menu_tex_ = nullptr;
    }
    if (!menu_tex_) {
        menu_tex_ =
            SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, kUiHeight);
        SDL_SetTextureScaleMode(menu_tex_, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(menu_tex_, SDL_BLENDMODE_BLEND);
    }
    draw_layer(menu_scaled_, menu_tex_, pixels, width, dst);
}

// Carries out what the menu bar asked for; what does not exist yet is
// ignored.
void App::perform(const MenuEvent &event)
{
    play_menu_sound(event.sound);
    switch (event.action) {
    case MenuAction::Speed:
        speed_ = menu_speed(event.value);
        // pulldown_vary_speed: only the menu sets speed_choice.
        details_.speed_choice = uint8_t(event.value / 10 - 1);
        break;
    case MenuAction::ToggleShadows:
        shadows_ = !shadows_;
        break;
    case MenuAction::ToggleAutosave:
        details_.autosave = details_.autosave ? 0 : 1;
        break;
    case MenuAction::ToggleSound:
        sounds_.toggle_sound();
        break;
    case MenuAction::SoundVolume:
        sounds_.set_volume_item(event.value);
        break;
    case MenuAction::ToggleMusic:
        music_.toggle_music();
        break;
    case MenuAction::MusicVolume:
        music_.set_volume_item(event.value);
        break;
    case MenuAction::ToggleAnnouncements:
        sounds_.toggle_ambient();
        break;
    case MenuAction::AnnouncementVolume:
        sounds_.set_ambient_volume_item(event.value);
        break;
    case MenuAction::Quit:
        ask_quit();
        break;
    case MenuAction::Screen:
        open_menu_screen(event.value);
        break;
    case MenuAction::Jukebox:
        if (map_)
            map_->game.event = {EV_OPEN_JUKEBOX};
        break;
    case MenuAction::Restart:
        ask_restart();
        break;
    case MenuAction::Briefing:
        // Charts > Briefing: show_help_page(1, level), refused while an
        // edited room empties (menu.cpp:487).
        if (map_ && map_->game.world.building.mouse == BuildMouse::EditRoomWait)
            sounds_.play_sound(kSourceGeneral, kSfxWrong2);
        else if (map_)
            help_.set_page(-1, -1, level_briefing(data_->text(), level_));
        break;
    case MenuAction::Save:
    case MenuAction::Load:
        perform_save_load(event);
        break;
    default:
        break;
    }
}

// pulldown_save and pulldown_load: a slot already used asks before it is
// overwritten (yesno_text[3], EV_SAVE_GAME on yes); a load goes to the slot
// (EV_LOAD_GAME).
void App::perform_save_load(const MenuEvent &event)
{
    if (event.value < 1 || event.value > kMenuSlots)
        return;
    if (event.action == MenuAction::Save) {
        if (!map_)
            return;
        if (save_slots_[size_t(event.value)]) {
            interface_.open_yes_no(data_->text().at(TextSection::YesNo, 3));
            pending_save_slot_ = event.value;
            asking_quit_ = false;
        } else {
            perform_save(event.value);
        }
    } else {
        perform_load(event.value);
    }
}

void App::play_menu_sound(MenuSound sound)
{
    switch (sound) {
    case MenuSound::Highlight:
        sounds_.play_sound(kSourceGeneral, kSfxHighlight5);
        break;
    case MenuSound::Select:
        sounds_.play_sound(kSourceGeneral, kSfxSelectX);
        break;
    case MenuSound::Wrong:
        sounds_.play_sound(kSourceGeneral, kSfxWrong2);
        break;
    case MenuSound::None:
        break;
    }
}

// The Charts menu's screen `chart`, refused with a sound when it cannot open.
void App::open_menu_screen(int chart)
{
    if (!map_)
        return;
    bool refused = false;
    Event e = open_chart_command(chart, map_->game, &refused);
    if (refused)
        sounds_.play_sound(kSourceGeneral, kSfxWrong2);
    else
        map_->game.event = e;
}

// Keeps the game's input that comes during a fade, for after it. True when
// `e` was kept.
bool App::hold_during_fade(const SDL_Event &e)
{
    if (!fader_.blocking())
        return false;
    switch (e.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
    case SDL_EVENT_MOUSE_MOTION:
    case SDL_EVENT_MOUSE_WHEEL:
        held_events_.push_back(e);
        return true;
    default:
        return false;
    }
}

void App::deliver_held_events()
{
    std::vector<SDL_Event> held;
    held.swap(held_events_);
    for (const SDL_Event &e : held)
        handle_event(e);
}

// A mouse drag scrolls the view.
void App::mouse_motion(const SDL_MouseMotionEvent &motion)
{
    if (!dragging_ || !data_ || quick_.open)
        return;
    float density = SDL_GetWindowPixelDensity(window_);
    scroll_x_ -= motion.xrel * density / float(zoom());
    scroll_y_ -= motion.yrel * density / float(zoom());
}

// The wheel zooms the view, or scrolls the start-up screens' changelog.
// render() keeps the view centred when the surface changes size.
void App::mouse_wheel(const SDL_MouseWheelEvent &wheel)
{
    if (!data_ || quick_.open)
        return;
    int step = wheel.y > 0 ? 1 : wheel.y < 0 ? -1 : 0;
    if (in_startup_) {
        startup_wheel_ += step;
        return;
    }
    zoom_ = std::clamp(zoom() + step, 1, 4);
}

void App::handle_event(const SDL_Event &e)
{
    if (hold_during_fade(e))
        return;
    ImGui_ImplSDL3_ProcessEvent(&e);
    const ImGuiIO &io = ImGui::GetIO();
    switch (e.type) {
    case SDL_EVENT_QUIT:
        running_ = false;
        break;
    case SDL_EVENT_DROP_FILE:
        if (!data_ && e.drop.data)
            start_setup_job(install_kind_of(e.drop.data), e.drop.data);
        break;
    case SDL_EVENT_KEY_DOWN:
        if (!io.WantCaptureKeyboard && !quick_.open)
            key_down(e.key);
        break;
    case SDL_EVENT_KEY_UP:
        key_up(e.key);
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (!io.WantCaptureMouse && !quick_.open)
            mouse_down(e.button);
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (!quick_.open)
            mouse_up(e.button);
        break;
    case SDL_EVENT_MOUSE_MOTION:
        mouse_motion(e.motion);
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        if (!io.WantCaptureMouse)
            mouse_wheel(e.wheel);
        break;
    default:
        break;
    }
}

// The keys the game reads as held or as the last key pressed: W (turbo),
// the numpad 8/2/4/6 (the casebook and staff list), and the modifiers.
void App::note_held_keys(const SDL_KeyboardEvent &key, bool down)
{
    key_mod_ = key.mod;
    if (key.key == SDLK_W)
        turbo_held_ = down;
    if (key.key == SDLK_TAB)
        tab_held_ = down;
    uint8_t pad = 0;
    switch (key.key) {
    case SDLK_KP_8:
        pad = ChartKeys::kPadUp;
        break;
    case SDLK_KP_2:
        pad = ChartKeys::kPadDown;
        break;
    case SDLK_KP_4:
        pad = ChartKeys::kPadLeft;
        break;
    case SDLK_KP_6:
        pad = ChartKeys::kPadRight;
        break;
    default:
        break;
    }
    if (down) {
        pad_held_ |= pad;
        if (charts_.open)
            pad_pressed_ |= pad;
    } else {
        pad_held_ &= uint8_t(~pad);
    }
}

// A key pressed, outside the developer tools.
void App::key_down(const SDL_KeyboardEvent &key)
{
    note_held_keys(key, true);
    any_key_ = winlose_showing_;
    if (key.key == SDLK_F11 && web::kEnabled) {
        SDL_SetWindowFullscreen(window_, !(SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN));
        return;
    }
    if (key.key == SDLK_F10 && data_) {
        quick_report_key(SDL_Keymod(key.mod));
        return;
    }
    if (key.key == SDLK_F12 && !debug_)
        return;
    if (key.key == SDLK_F12 && data_ && in_startup_) {
        tools_.open = !tools_.open;
        return;
    }
    if (in_startup_) {
        startup_keys_.push_back(startup_key(key.key));
        return;
    }
    if (key.key == SDLK_F12) {
        tools_.open = !tools_.open;
    } else if (!data_) {
        if (key.key == SDLK_ESCAPE)
            running_ = false;
    } else if (key.key == SDLK_ESCAPE && !(key.mod & SDL_KMOD_SHIFT)) {
        // Closes the front requester (the game's Escape).
        escape_ = true;
    } else if (interface_.modal() || town_.is_open()) {
        // A modal requester takes the keys; the town map only Escape.
    } else {
        if (uint8_t bit = scroll_key_bit(key.key))
            scroll_held_ |= bit;
        game_key(key);
    }
}

// A key released: the scroll keys stop (move_windows clears its scroll.*
// every frame; the held set is what keeps them on).
void App::key_up(const SDL_KeyboardEvent &key)
{
    note_held_keys(key, false);
    scroll_held_ &= uint8_t(~scroll_key_bit(key.key));
}

// The game's speed keys 1-5 (process_game_keys, src/keyboard.cpp): key 1 is
// ignored with Shift, its cheat, and keys 1 and 2 in a rat game. True when
// `key` is one and was taken.
bool App::game_speed_key(const SDL_KeyboardEvent &key)
{
    if (key.key < SDLK_1 || key.key > SDLK_5)
        return false;
    if (key.key == SDLK_1 && (key.mod & SDL_KMOD_SHIFT))
        return false;
    if (key.key <= SDLK_2 && map_->game.world.rats.fest_level)
        return true;
    speed_ = menu_speed(10 * int(key.key - SDLK_1 + 1));
    return true;
}

// Alt+S quick saves and Alt+L quick loads debug_save_slot (keyboard.cpp's
// KC_S and KC_L with MOD_ALT).
bool App::quick_save_load_key(const SDL_KeyboardEvent &key)
{
    if (!map_ || !(key.mod & SDL_KMOD_ALT) || (key.key != SDLK_S && key.key != SDLK_L))
        return false;
    if (key.key == SDLK_S)
        perform_save(debug_save_slot_);
    else
        perform_load(debug_save_slot_);
    return true;
}

// Alt+F1-F8 stores the cell at the view's centre and Ctrl+F1-F8 moves the
// view to it (record_mappos and recall_mappos, src/utils.cpp). Neither runs
// while paused, nor in a rat game except on F4; recalling a slot nothing was
// stored in does nothing, and cell 0 cannot be stored, as in the original.
bool App::view_bookmark_key(const SDL_KeyboardEvent &key)
{
    if (!map_ || key.key < SDLK_F1 || key.key > SDLK_F8 || !(key.mod & (SDL_KMOD_ALT | SDL_KMOD_CTRL)))
        return false;
    const size_t slot = size_t(key.key - SDLK_F1);
    if (map_->game.paused || (key.key != SDLK_F4 && map_->game.world.rats.fest_level))
        return true;
    if (key.mod & SDL_KMOD_ALT) {
        sounds_.play_sound(kSourceGeneral, kSfxSnapshot);
        Point cell = world_to_cell({camera_.x + surface_.width() / 2, camera_.y + surface_.height() / 2});
        int col = std::clamp(cell.x, 0, kMapSize - 1);
        int row = std::clamp(cell.y, 0, kMapSize - 1);
        key_mappos_[slot] = uint16_t(col + row * kMapSize);
    } else if (key_mappos_[slot]) {
        Point p = cell_to_world(key_mappos_[slot] % kMapSize, key_mappos_[slot] / kMapSize);
        camera_ = {p.x - surface_.width() / 2, p.y - surface_.height() / 2};
    }
    return true;
}

// A key pressed over the hospital or a chart, with no modal requester up.
void App::game_key(const SDL_KeyboardEvent &key)
{
    if (fax_.open) {
        // The fax reads Escape and Space alone.
        chart_space_ = chart_space_ || key.key == SDLK_SPACE;
        return;
    }
    if (year_end_showing_ || winlose_showing_) {
        // The year-end screens read Escape and Space alone; the letters, any
        // key (any_key_).
        chart_space_ = chart_space_ || key.key == SDLK_SPACE;
        return;
    }
    if (map_ && game_speed_key(key))
        return;
    if (quick_save_load_key(key))
        return;
    if (view_bookmark_key(key))
        return;
    if (screen_shot_key(key))
        return;
    if (charts_.open)
        chart_space_ = chart_space_ || key.key == SDLK_SPACE;
    if (key.key >= SDLK_F1 && key.key <= SDLK_F8 && map_) {
        // The charts, from the game or another chart; not while paused, and
        // only F4 in a rat game.
        if (!map_->game.paused && (key.key == SDLK_F4 || !map_->game.world.rats.fest_level))
            open_chart_key(int(key.key - SDLK_F1));
        return;
    }
    if (quit_key(key) || !map_ || cheat_key(key))
        return;
    game_letter_key(key);
}

// The cheat keys of process_game_keys (src/keyboard.cpp), which act only
// once the fax's code has enabled cheats: Ctrl+C and Ctrl+Shift+C make
// objects available, Shift+C adds 1000 cash, Shift+1 sends EV_CHEAT_CELL,
// Shift+R makes a rat, Ctrl+Alt+D lets the view scroll over the map, Ctrl+Y
// and Ctrl+M end the year and the month. Alt+C is the screenshot's. True
// when `key` was one.
bool App::cheat_key(const SDL_KeyboardEvent &key)
{
    Game &game = map_->game;
    const bool shift = key.mod & SDL_KMOD_SHIFT;
    const bool ctrl = key.mod & SDL_KMOD_CTRL;
    const bool alt = key.mod & SDL_KMOD_ALT;
    if (key.key == SDLK_C && !alt && (ctrl || shift)) {
        if (game.cheat(ctrl ? (shift ? 4 : 2) : 8)) {
            const int announcement = kAnnounceCheat + saferand(sounds_.safeseed) % 2;
            sounds_.play_ambient(announcement, has_receptionist(game.world), game.paused);
        }
    } else if (key.key == SDLK_1 && shift) {
        if (game.cheats_enabled)
            game.event = {EV_CHEAT_CELL, uint16_t(std::max(map_cell_, 0)), 0, 0};
    } else if (key.key == SDLK_R && shift) {
        game.cheat_rat();
    } else if (key.key == SDLK_D && ctrl && alt) {
        game.cheat_scroll();
    } else if (key.key == SDLK_Y && ctrl && game.cheats_enabled) {
        game.event = {EV_CHEAT_END_YEAR};
    } else if (key.key == SDLK_M && ctrl && game.cheats_enabled) {
        game.event = {EV_CHEAT_END_MONTH};
    } else {
        return false;
    }
    return true;
}

void App::game_letter_key(const SDL_KeyboardEvent &key)
{
    if (key.key == SDLK_P) {
        map_->game.paused = !map_->game.paused;
    } else if (key.key == SDLK_T && charts_.open) {
        // From a chart, T leaves for the town map (DM_TOWN_MAP).
        charts_.open = false;
        open_town_map();
    } else if (key.key == SDLK_T) {
        open_town_ = true;
    } else if (key.key == SDLK_A && (key.mod & SDL_KMOD_SHIFT)) {
        toggle_advisor(map_->game.advisor, map_->game.world);
    } else if (key.key == SDLK_J && !(key.mod & SDL_KMOD_SHIFT) && !map_->game.paused) {
        // Over a chart too (process_keys runs in DM_CHARTS): the requester
        // opens behind it and shows when the chart is left.
        map_->game.event = {EV_OPEN_JUKEBOX};
    }
}

// The game's quit keys, Escape with Shift and Q or X with a modifier: they
// ask first, apart from Shift+Ctrl+Escape and Shift+Escape with Tab held,
// which quit at once. True when `key` is one.
bool App::quit_key(const SDL_KeyboardEvent &key)
{
    const bool shift_escape = key.key == SDLK_ESCAPE && (key.mod & SDL_KMOD_SHIFT);
    if (shift_escape && ((key.mod & SDL_KMOD_CTRL) || tab_held_)) {
        quit_now_ = true;
        return true;
    }
    if (shift_escape ||
        ((key.key == SDLK_Q || key.key == SDLK_X) && (key.mod & (SDL_KMOD_CTRL | SDL_KMOD_ALT | SDL_KMOD_SHIFT)))) {
        ask_quit();
        return true;
    }
    return false;
}

// F1-F8: the chart `chart`, refused with a sound when it cannot open.
void App::open_chart_key(int chart)
{
    bool refused = false;
    Event ev = open_chart_command(chart, map_->game, &refused);
    if (refused)
        sounds_.play_sound(kSourceGeneral, kSfxWrong2);
    else
        map_->game.event = ev;
}

// The inspector's pick: a left click on a person or object inspects it, a
// right click gives up; the game sees neither.
void App::inspect_click(uint8_t button)
{
    InspectTool &inspect = tools_.inspect;
    if (button == SDL_BUTTON_RIGHT)
        inspect.picking = false;
    if (button != SDL_BUTTON_LEFT || !pointed_.thing)
        return;
    inspect.thing = pointed_.thing;
    inspect.picking = false;
}

void App::mouse_down(const SDL_MouseButtonEvent &button)
{
    if (tools_.inspect.picking && map_) {
        inspect_click(button.button);
        return;
    }
    if (button.button == SDL_BUTTON_LEFT)
        buttons_.left = buttons_.left_pressed = true;
    if (button.button == SDL_BUTTON_RIGHT)
        buttons_.right = buttons_.right_pressed = true;
    // An open menu, a modal requester or the town map takes the clicks.
    if (menu_.active || interface_.modal() || town_.is_open() || charts_.open || fax_.open || year_end_showing_ ||
        winlose_showing_)
        return;
    // While something is being placed, its clicks are the interface's.
    if (map_ && (map_->game.world.placing.placing || hire_.placing()))
        return;
    if (button.button == SDL_BUTTON_MIDDLE || button.button == SDL_BUTTON_RIGHT)
        dragging_ = true;
}

void App::mouse_up(const SDL_MouseButtonEvent &button)
{
    if (button.button == SDL_BUTTON_LEFT && buttons_.left) {
        buttons_.left = false;
        buttons_.left_released = true;
    }
    if (button.button == SDL_BUTTON_RIGHT && buttons_.right) {
        buttons_.right = false;
        buttons_.right_released = true;
    }
    if (button.button == SDL_BUTTON_MIDDLE || button.button == SDL_BUTTON_RIGHT)
        dragging_ = false;
}

} // namespace th
