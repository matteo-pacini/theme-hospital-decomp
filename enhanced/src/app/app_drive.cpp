// Driven runs (drive.h): the --drive and --control arguments, the driven
// clock and what a script's commands do to the app.

#include "app/app.h"
#include "core/data/gamedata.h"
#include "core/render/shadows.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <thread>

namespace th
{

void App::parse_drive_args(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--drive" && i + 1 < argc)
            drive_script_ = argv[++i];
        else if (arg == "--out" && i + 1 < argc)
            drive_out_ = argv[++i];
        else if (arg == "--size" && i + 1 < argc)
            std::sscanf(argv[++i], "%dx%d", &drive_w_, &drive_h_);
        else if (arg == "--control")
            drive_control_ = true;
        else if (arg == "--visible")
            drive_visible_ = true;
        else if (arg == "--default-speed")
            drive_default_speed_ = true;
        else if (arg == "--real-time")
            drive_real_time_ = true;
        else if (arg == "--fades")
            drive_fades_ = true;
    }
}

void App::hold_drive_slot()
{
    const char *env = std::getenv("TH_DRIVE_SLOTS");
    int max = env ? std::max(1, std::atoi(env)) : 6;
    // The executable sits in build/<tree>/ of the repository.
    const char *base = SDL_GetBasePath();
    std::filesystem::path dir = std::filesystem::path(base ? base : "").parent_path().parent_path() / "slots" / "drive";
    std::string info =
        drive_control_ ? std::string("th_enhanced --control") : "th_enhanced --drive " + drive_script_.string();
    if (!hold_slot(dir, max, info))
        std::fprintf(stderr, "drive: no slot files in %s; running without a slot\n", dir.string().c_str());
}

void App::start_driver()
{
    // English unless --language says otherwise, whatever HOSPITAL.CFG holds,
    // so that expectations on text hold everywhere.
    if (!language_)
        language_ = kEnglish;
    driver_ = std::make_unique<Driver>(drive_out_, drive_control_);
    fader_.enable(drive_fades_);
    if (!drive_control_) {
        std::ifstream in(drive_script_);
        if (!in)
            std::fprintf(stderr, "drive: cannot read %s\n", drive_script_.string().c_str());
        for (std::string line; std::getline(in, line);)
            driver_->add_line(line);
        driver_->end_input();
        return;
    }
    // Control commands come one per line on stdin, as the controller sends
    // them; the app waits between them.
    std::thread([driver = driver_.get()] {
        for (std::string line; std::getline(std::cin, line);)
            driver->add_line(line);
        driver->end_input();
    }).detach();
}

uint32_t App::ticks_ms() const
{
    return driver_ ? uint32_t(drive_ns_ / 1000000) : uint32_t(SDL_GetTicks());
}

bool App::drive_warp_world(Point p)
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window_, &w, &h);
    int z = zoom();
    float px = float((p.x - camera_.x) * z), py = float((p.y - camera_.y) * z);
    if (px < 0 || py < 0 || px >= float(w) || py >= float(h))
        return false;
    float density = SDL_GetWindowPixelDensity(window_);
    SDL_WarpMouseInWindow(window_, px / density, py / density);
    return true;
}

bool App::drive_move_screen(int x, int y)
{
    if (x < 0 || y < 0 || x >= kUiWidth || y >= kUiHeight)
        return false;
    float px = 0, py = 0;
    if (fit_.frame == frame_ && fit_.w > 0 && fit_.h > 0) {
        px = fit_.at.x + (float(x) + 0.5f) * fit_.at.w / float(fit_.w);
        py = fit_.at.y + (float(y) + 0.5f) * fit_.at.h / float(fit_.h);
    } else {
        int w = 0, h = 0;
        SDL_GetWindowSizeInPixels(window_, &w, &h);
        int scale = ui_scale();
        px = float((w - kUiWidth * scale) / 2 + x * scale + scale / 2);
        py = float(h - kUiHeight * scale + y * scale + scale / 2);
    }
    float density = SDL_GetWindowPixelDensity(window_);
    SDL_WarpMouseInWindow(window_, px / density, py / density);
    return true;
}

bool App::drive_move_cell(int col, int row)
{
    if (!map_ || in_startup_ || col < 0 || row < 0 || col >= kMapSize || row >= kMapSize)
        return false;
    Point p = cell_to_world(col, row);
    return drive_warp_world({p.x, p.y + kTileHeight / 2});
}

bool App::drive_move_thing(uint16_t id)
{
    if (!map_ || in_startup_ || id == 0 || id >= map_->game.world.things.size())
        return false;
    const Thing &t = map_->game.world.things[id];
    if (t.type == kThingFree)
        return false;
    // Above the thing's feet, onto its sprite.
    Point p = thing_to_world(t);
    return drive_warp_world({p.x, p.y - kTileHeight / 2});
}

void App::drive_button(int button, bool down)
{
    SDL_Event e{};
    e.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
    e.button.windowID = SDL_GetWindowID(window_);
    e.button.button = uint8_t(button);
    e.button.down = down;
    e.button.clicks = 1;
    SDL_GetMouseState(&e.button.x, &e.button.y);
    SDL_PushEvent(&e);
}

bool App::drive_key(std::string_view name, bool down, uint16_t mods)
{
    // A script word has no spaces: Keypad_8 is "Keypad 8".
    std::string key_name(name);
    std::ranges::replace(key_name, '_', ' ');
    SDL_Keycode key = SDL_GetKeyFromName(key_name.c_str());
    if (key == SDLK_UNKNOWN)
        return false;
    SDL_Keymod sdl_mods = SDL_KMOD_NONE;
    if (mods & kDriveModShift)
        sdl_mods |= SDL_KMOD_SHIFT;
    if (mods & kDriveModCtrl)
        sdl_mods |= SDL_KMOD_CTRL;
    if (mods & kDriveModAlt)
        sdl_mods |= SDL_KMOD_ALT;
    SDL_Event e{};
    e.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    e.key.windowID = SDL_GetWindowID(window_);
    e.key.key = key;
    e.key.scancode = SDL_GetScancodeFromKey(key, nullptr);
    e.key.mod = sdl_mods;
    e.key.down = down;
    SDL_PushEvent(&e);
    return true;
}

bool App::drive_text(std::string_view text)
{
    drive_texts_.emplace_back(text);
    SDL_Event e{};
    e.type = SDL_EVENT_TEXT_INPUT;
    e.text.windowID = SDL_GetWindowID(window_);
    e.text.text = drive_texts_.back().c_str();
    SDL_PushEvent(&e);
    return true;
}

void App::drive_screenshot(const std::filesystem::path &path)
{
    drive_shot_ = path;
}

void App::drive_screenshot_ui(const std::filesystem::path &path)
{
    drive_shot_ui_ = path;
}

std::string App::drive_state() const
{
    return debug_state();
}

std::string App::drive_report_dir() const
{
    return quick_.last_report;
}

int64_t App::drive_turn() const
{
    return map_ && !in_startup_ ? int64_t(map_->game.turn_count) : -1;
}

bool App::drive_staff_demand(uint16_t id)
{
    if (!map_ || in_startup_ || id == 0 || id >= map_->game.world.things.size())
        return false;
    open_staff_resign_requester(demand_, interface_, map_->game.world, id, sounds_.safeseed);
    return true;
}

bool App::drive_win_level()
{
    if (!map_ || in_startup_)
        return false;
    YearEnd &s = map_->game.progress;
    s.level_won = 1;
    s.next_level_offer = 1000000;
    s.year_end_type = 5;
    s.winlose_result = 1;
    s.win_declined = 0;
    return true;
}

bool App::drive_lose_level()
{
    if (!map_ || in_startup_)
        return false;
    YearEnd &s = map_->game.progress;
    s.level_won = 0;
    s.year_end_type = 7;
    s.winlose_result = 2;
    return true;
}

} // namespace th
