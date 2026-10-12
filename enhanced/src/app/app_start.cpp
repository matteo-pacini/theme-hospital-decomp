// App's start-up and shutdown: init, the window, the game data, the UI art
// and the textures it owns, and the main loop's pass (begin, iterate).

#include "app/app.h"
#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/upscale/cache.h"
#include "th_icon.h"
#include "th_version.h"

#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

namespace th
{

namespace
{

// The in-game pointers drawn: the default one and those hovering on the map
// shows (core/ui/info_windows.h), all with their hotspot at the top-left
// corner.
constexpr uint8_t kPointersDrawn[] = {kPointerDefault, kPointerStaff,     kPointerMachine,        kPointerPatient,
                                      kPointerQueue,   kPointerVaccinate, kPointerVaccinateTarget};

// The window's icon, decoded from the PNG built into the program. A platform
// with no window icons (the browser) refuses it; the window works without.
void set_window_icon(SDL_Window *window)
{
    SDL_IOStream *io = SDL_IOFromConstMem(kIconPng, sizeof kIconPng);
    SDL_Surface *icon = io ? SDL_LoadPNG_IO(io, true) : nullptr;
    if (!icon) {
        std::fprintf(stderr, "window icon: %s\n", SDL_GetError());
        return;
    }
    SDL_SetWindowIcon(window, icon);
    SDL_DestroySurface(icon);
}

// A static texture of `width` x `height` 0xAARRGGBB pixels, scaled without
// smoothing.
SDL_Texture *argb_texture(SDL_Renderer *renderer, const std::vector<uint32_t> &pixels, int width, int height)
{
    SDL_Texture *t = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);
    if (!t)
        return nullptr;
    SDL_UpdateTexture(t, nullptr, pixels.data(), width * 4);
    SDL_SetTextureScaleMode(t, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    return t;
}

} // namespace

bool App::init(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--data" && i + 1 < argc)
            data_arg_ = argv[++i];
        else if (arg == "--debug")
            debug_ = true;
        else if (arg == "--level" && i + 1 < argc) {
            level_ = std::atoi(argv[++i]);
            level_from_cli_ = true;
        } else if (arg == "--language" && i + 1 < argc)
            language_ = std::atoi(argv[++i]);
    }
    parse_drive_args(argc, argv);
    bool driving = !drive_script_.empty() || drive_control_;
    debug_ = debug_ || driving;

    user_paths_ = user_paths();
    std::optional<std::filesystem::path> install_from = parse_setup_args(argc, argv);
    if (install_from && !install_from_command_line(*install_from)) {
        install_failed_ = true;
        return false;
    }
#ifndef TH_NO_DRIVE_SLOTS
    if (driving)
        hold_drive_slot();
#endif
    if (driving && !drive_visible_) {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
        SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
    }

    if (!create_window(driving))
        return false;

    // A driven run does not depend on, or change, the player's settings.
    if (driving) {
        start_driver();
    } else {
        migrate_old_prefs(user_paths_);
        settings_.load(user_paths_.settings_dir);
    }
    search_ = find_data_dir(data_arg_, user_data_ || install_from.has_value(), user_paths_.game_dir);
    if (search_.found)
        try_load_data(*search_.found);
    refresh_game_dir_state();
    queue_drop_arg();
    return true;
}

// SDL, the window and its renderer, and ImGui; a driven run's window is the
// --size it asked for, at a scale of 1.
bool App::create_window(bool driving)
{
    // The identifier names the app to the desktop (Wayland app id, X11 class)
    // and matches the macOS bundle's.
    SDL_SetAppMetadata("Theme Hospital Enhanced", kVersion, "me.matteopacini.theme-hospital-enhanced");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return false;
    }
    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    if (scale <= 0 || driving)
        scale = 1.0f;
    int win_w = driving ? drive_w_ : 1280, win_h = driving ? drive_h_ : 720;
    window_ = SDL_CreateWindow("Theme Hospital Enhanced", int(float(win_w) * scale), int(float(win_h) * scale),
                               driving ? 0 : SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window_) {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return false;
    }
    set_window_icon(window_);
    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        std::fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return false;
    }
    // A driven run's frames go as fast as they can: its clock is its own.
    if (!driving)
        SDL_SetRenderVSync(renderer_, 1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui::GetStyle().ScaleAllSizes(scale);
    ImGui::GetStyle().FontScaleDpi = scale;
    ImGui_ImplSDL3_InitForSDLRenderer(window_, renderer_);
    ImGui_ImplSDLRenderer3_Init(renderer_);
    imgui_ready_ = true;
    return true;
}

void App::choose_upscale()
{
    const std::string text = upscale_arg_.empty() ? settings_.get("upscale") : upscale_arg_;
    upscale_ = {};
    if (!text.empty() && !parse_upscale(text, upscale_))
        std::fprintf(stderr, "upscale: expected off or ALGORITHM:FACTOR (xbrz:2 to xbrz:6), not \"%s\"\n",
                     text.c_str());
}

std::filesystem::path App::upscale_cache_dir() const
{
    return driver_ ? drive_out_ / "upscale" : user_paths_.upscale_dir;
}

void App::init_upscale_choice()
{
    if (upscale_choice_ready_)
        return;
    choose_upscale();
    upscale_choice_.on = upscale_.on();
    if (upscale_.on()) {
        upscale_choice_.factor = upscale_.factor;
        for (size_t i = 0; i < scalers().size(); i++)
            if (&scalers()[i] == upscale_.scaler)
                upscale_choice_.algorithm = int(i);
    }
    upscale_choice_ready_ = true;
}

void App::attach_upscaled(GameData &data, const std::filesystem::path &dir)
{
    if (!upscale_.on())
        return;
    MapHires hires;
    std::string error;
    const std::filesystem::path cache = upscale_cache_dir();
    if (!upscaled_map_sheets(dir, cache, upscale_, {}, hires, &error)) {
        std::fprintf(stderr, "upscale: %s\n", error.c_str());
        upscale_ = {};
        return;
    }
    data.attach_hires(std::move(hires.blocks), std::move(hires.sprites), std::move(hires.money));
}

bool App::try_load_data(const std::filesystem::path &dir)
{
    auto data = std::make_unique<GameData>();
    std::string error;
    if (!data->load(dir, &error, language_)) {
        data_error_ = error;
        return false;
    }
    choose_upscale();
    attach_upscaled(*data, dir);
    data_ = std::move(data);
    load_ui_art(dir);
    data_dir_ = dir;
    data_error_.clear();
    // The saved games and the high scores: in the user's save folder, or
    // under a driven run's output folder, which is removed between runs, so
    // tests start with none.
    if (driver_) {
        save_dir_ = drive_out_ / "save";
    } else {
        save_dir_ = user_paths_.save_dir.empty() ? std::filesystem::path("save") : user_paths_.save_dir;
        migrate_hiscore(user_paths_, dir);
    }
    std::error_code save_ec;
    std::filesystem::create_directories(save_dir_, save_ec);
    hiscore_path_ = save_dir_ / "hiscore.dat";
    startup_.set_save_dir(save_dir_);
    // The start-up screens choose the level, unless --level did.
    if (!level_from_cli_ && startup_.load(dir, data_->text(), data_->language(), &error))
        in_startup_ = true;
    else {
        load_level(level_);
        // debug.direct_start: setup_new_level, then load_player_details.
        load_player_details();
    }
    return true;
}

void App::load_ui_art(const std::filesystem::path &dir)
{
    std::string error;
    auto report = [&](bool ok) {
        if (!ok)
            std::fprintf(stderr, "%s\n", error.c_str());
    };
    report(menu_.load(dir, data_->text(), &error));
    report(requester_art_.load(dir, &error));
    report(hire_art_.load(dir, &error));
    report(town_.load(dir, data_->text(), &error));
    report(load_charts(charts_, dir, data_->text(), data_->palette(), &error));
    report(load_fax(fax_, dir, data_->text(), &error));
    report(year_end_.load(dir, data_->text(), data_->palette(), &error));
    report(winlose_.load(dir, data_->text(), data_->palette(), &error));
    init_object_ui(object_ui_, data_->palette());
    report(bank_.load(dir, data_->language(), &error));
    sounds_.set_bank(&bank_);
    if (!music_.load(dir, &error))
        std::fprintf(stderr, "no music: %s\n", error.c_str());
    if (!mixer_.is_open() && !mixer_.open(&error))
        std::fprintf(stderr, "no sound: %s\n", error.c_str());
    help_.setup_page_indices(data_->text());
    load_help_art(dir);
    wire_screen_help();
    make_ui_textures();
}

void App::make_ui_textures()
{
    for (PointerTexture &pt : pointers_)
        if (pt.texture)
            SDL_DestroyTexture(std::exchange(pt.texture, nullptr));
    if (!ui_tex_) {
        ui_.resize(kUiWidth, kUiHeight);
        ui_tex_ =
            SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, kUiWidth, kUiHeight);
        SDL_SetTextureScaleMode(ui_tex_, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(ui_tex_, SDL_BLENDMODE_BLEND);
    }
    for (uint8_t id : kPointersDrawn) {
        Sprite s = data_->pointers().at(id);
        PointerTexture &pt = pointers_[id];
        pt.w = s.width;
        pt.h = s.height;
        std::vector<uint32_t> pixels(size_t(pt.w) * size_t(pt.h));
        sprite_to_argb(s, data_->palette(), pixels.data(), pt.w * 4);
        if (upscale_.on() && !s.empty()) {
            pt.texture = argb_texture(renderer_, upscale_argb(upscale_, pixels.data(), pt.w, pt.h),
                                      pt.w * upscale_.factor, pt.h * upscale_.factor);
        } else {
            pt.texture = argb_texture(renderer_, pixels, pt.w, pt.h);
        }
    }
}

void App::begin()
{
    // A driven run starts with the pointer at the window's centre, not at the
    // window's (0,0) edge, so the camera starts where the level puts it and
    // does not edge-scroll until a script moves the pointer there.
    if (driver_) {
        int w = 0, h = 0;
        SDL_GetWindowSize(window_, &w, &h);
        SDL_WarpMouseInWindow(window_, float(w) / 2.0f, float(h) / 2.0f);
    }
    last_ns_ = SDL_GetTicksNS();
}

bool App::iterate()
{
    // What came during a fade reaches the game once it is over.
    if (!fader_.blocking() && !held_events_.empty())
        deliver_held_events();
    // A driven run waits for an install, its frames not counted, so that
    // what follows does not depend on how long the install took.
    bool installing = driver_ && job_.busy();
    // A driven run moves on only as its commands ask for frames.
    if (driver_ && installing) {
        SDL_Delay(5);
    } else if (driver_) {
        if (driver_->finished()) {
            driver_->flush();
            return false;
        }
        if (!driver_->frame(*this)) {
            SDL_Delay(5);
            return true;
        }
    }
    // The events the driver just pushed take effect in this frame; SDL's own
    // events were handled before this call.
    if (driver_) {
        SDL_Event e;
        while (SDL_PollEvent(&e))
            handle_event(e);
        // The `type` text events are polled; SDL no longer needs the strings.
        drive_texts_.clear();
    }

    if (!data_)
        poll_setup();

    uint64_t now = SDL_GetTicksNS();
    float dt = float(now - last_ns_) / 1e9f;
    last_ns_ = now;
    if (driver_ && drive_real_time_) {
        if (!installing)
            drive_ns_ += uint64_t(dt * 1e9f);
    } else if (driver_) {
        dt = 1.0f / 60.0f;
        if (!installing)
            drive_ns_ += 16666667;
    }
    frame_seconds_ = std::min(dt, 0.1f);
    if (!fader_.blocking())
        update(frame_seconds_);
    render();
    return running_;
}

void App::destroy_textures()
{
    if (quick_.capture.shot)
        SDL_DestroySurface(quick_.capture.shot);
    if (startup_tex_)
        SDL_DestroyTexture(startup_tex_);
    if (town_tex_)
        SDL_DestroyTexture(town_tex_);
    if (charts_tex_)
        SDL_DestroyTexture(charts_tex_);
    if (fax_tex_)
        SDL_DestroyTexture(fax_tex_);
    if (year_end_tex_)
        SDL_DestroyTexture(year_end_tex_);
    if (winlose_tex_)
        SDL_DestroyTexture(winlose_tex_);
    if (ui_tex_)
        SDL_DestroyTexture(ui_tex_);
    for (PointerTexture &pt : pointers_)
        if (pt.texture)
            SDL_DestroyTexture(pt.texture);
    if (crosshair_)
        SDL_DestroyCursor(crosshair_);
    if (screen_)
        SDL_DestroyTexture(screen_);
    if (loading_tex_)
        SDL_DestroyTexture(loading_tex_);
    fader_.release();
}

void App::shutdown()
{
    mixer_.close();
    if (movie_stream_)
        SDL_DestroyAudioStream(movie_stream_);
    destroy_textures();
    if (imgui_ready_) {
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }
    if (renderer_)
        SDL_DestroyRenderer(renderer_);
    if (window_)
        SDL_DestroyWindow(window_);
    SDL_Quit();
}

} // namespace th
