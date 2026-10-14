// One frame: the game's update, the scroll, and drawing and presenting the
// hospital view, the fades and screen shots (src/cakewin.cpp move_windows).

#include "app/app.h"
#include "app/app_internal.h"
#include "app/web.h"
#include "core/data/gamedata.h"

#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <format>
#include <utility>

namespace th
{

namespace
{

constexpr int kEdgeScrollMargin = 8; // pixels from the window edge

// The velocity the scroll keys add each frame (x moves twice as fast as y, as the game's
// 2:1 tiles do).
constexpr int32_t kScrollAccelX = 0x80000, kScrollAccelY = 0x40000;
constexpr int32_t kScrollDecay = 0xcd; // the velocity's per-frame decay, * 0xcd >> 8
// move_windows runs once a game frame, which limit_framerate caps at 40 ms.
constexpr double kGameFrame = 0.04;

} // namespace

inline namespace app_detail
{

// Draws the hospital view into `surface`.
void draw_view(Surface &surface, const GameData &data, LevelView *level, Point camera, bool shadows,
               const DrawnThings &drawn)
{
    surface.fill(0);
    if (level) {
        if (level->scratch.width() != surface.width() || level->scratch.height() != surface.height() ||
            level->scratch.scale() != surface.scale()) {
            level->scratch.resize(surface.width(), surface.height(), surface.scale());
            level->scratch.fill(kMaskColour);
        }
        draw_world(
            {surface, {0, 0, surface.width(), surface.height()}, camera, level->game.turn_count, level->overlays},
            level->game.world, data, shadows ? level->shadows.get() : nullptr, level->scratch, drawn);
    }
}

} // namespace app_detail

int App::zoom() const
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window_, &w, &h);
    if (zoom_ > 0)
        return zoom_;
    return std::max(1, h / kMinViewHeight);
}

UiMouse App::ui_mouse(int ox, int oy, int scale) const
{
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    float density = SDL_GetWindowPixelDensity(window_);
    UiMouse m;
    m.x = (int(mx * density) - ox) / scale;
    m.y = (int(my * density) - oy) / scale;
    m.left_held = buttons_.left;
    m.right_held = buttons_.right;
    m.left_pressed = buttons_.left_pressed;
    m.right_pressed = buttons_.right_pressed;
    m.left_released = buttons_.left_released;
    m.right_released = buttons_.right_released;
    return m;
}

void App::update(float dt)
{
    if (!data_ || in_startup_)
        return;
    if (quick_.open)
        return;
    if (std::exchange(quit_now_, false)) {
        quit_game();
        return;
    }
    if (restart_level_) {
        restart_level_ = false;
        load_level(level_);
        return;
    }
    if (map_ && !map_->game.paused && !winlose_showing_)
        run_turns(dt);
    if (map_ && map_->game.progress.rat_game_over)
        finish_rat_level();
    if (map_ && !year_end_showing_ && !winlose_showing_)
        check_year_end();
    if (year_end_showing_ || winlose_showing_ || town_.is_open() || charts_.open || fax_.open)
        return;
    // move_windows runs once a game frame, not once a rendered frame, so the
    // scroll speed follows the game's 40 ms frame, not the display's refresh.
    scroll_frame_time_ += double(dt);
    if (scroll_frame_time_ < kGameFrame)
        return;
    scroll_frame_time_ = std::min(scroll_frame_time_ - kGameFrame, kGameFrame);
    scroll_step(scroll_direction());
}

// The scroll direction this frame: the held keys, the pointer's window edge
// replacing them, an open pulldown ignoring both, and opposite keys
// cancelling, as keyboard.cpp, mouse.cpp and io.cpp set move_windows' flags.
uint8_t App::scroll_direction()
{
    const ImGuiIO &io = ImGui::GetIO();
    uint8_t dir = io.WantCaptureKeyboard ? 0 : scroll_held_;
    if ((dir & kScrollUp) && (dir & kScrollDown))
        dir &= uint8_t(~(kScrollUp | kScrollDown));
    if ((dir & kScrollLeft) && (dir & kScrollRight))
        dir &= uint8_t(~(kScrollLeft | kScrollRight));
    if (!io.WantCaptureMouse && !dragging_ && SDL_GetMouseFocus() == window_) {
        uint8_t edge = 0;
        float mx, my;
        SDL_GetMouseState(&mx, &my);
        int w, h;
        SDL_GetWindowSize(window_, &w, &h);
        if (mx < kEdgeScrollMargin)
            edge |= kScrollLeft;
        else if (mx >= w - kEdgeScrollMargin)
            edge |= kScrollRight;
        if (my < kEdgeScrollMargin)
            edge |= kScrollUp;
        else if (my >= h - kEdgeScrollMargin)
            edge |= kScrollDown;
        if (edge)
            dir = edge;
    }
    return menu_.open ? 0 : dir;
}

// The turns due after `dt` seconds at the game's speed.
void App::run_turns(float dt)
{
    // The staff demand requester is modal (display_mode DM_MODAL): the game's
    // turns pause while it is open.
    if (map_ && interface_.requester() == Requester::StaffDemand)
        return;
    double rate = double(int(speed_));
    if (turbo_held_ && !(key_mod_ & SDL_KMOD_CTRL) && !interface_.modal() && !town_.is_open() && !fax_.open &&
        !year_end_showing_ && !winlose_showing_) {
        // process_game_keys sets othervar.turbo_key on every frame W is held
        // without Ctrl: start_turn then runs unpaced, at least every second
        // frame, and the game loop adds two more turns to each one it ran.
        rate = 3 * std::clamp(rate, 0.5 / kGameFrame, 1.0 / kGameFrame);
    }
    // At most a few frames' worth of turns after a stall.
    turn_clock_ = std::min(turn_clock_ + dt * rate, 8.0);
    for (; turn_clock_ >= 1; turn_clock_ -= 1)
        map_->game.turn();
}

// move_windows (src/cakewin.cpp): the scroll direction `dir` accelerates the
// view's velocity while held (and stops it dead when none is), the velocity
// moves the view by its whole pixels with the fraction carried, clamped to
// one tile a frame, stopped at the scroll limit, and then decays.
void App::scroll_step(uint8_t dir)
{
    dir &= 0xf;
    if (!dir) {
        scroll_vel_x_ = 0;
        scroll_vel_y_ = 0;
    } else {
        if (dir & kScrollLeft)
            scroll_vel_x_ -= kScrollAccelX;
        if (dir & kScrollRight)
            scroll_vel_x_ += kScrollAccelX;
        if (dir & kScrollUp)
            scroll_vel_y_ -= kScrollAccelY;
        if (dir & kScrollDown)
            scroll_vel_y_ += kScrollAccelY;
    }
    int32_t dx = scroll_vel_x_ >> 16;
    int32_t dy = scroll_vel_y_ >> 16;
    scroll_frac_x_ += scroll_vel_x_ - (dx << 16);
    scroll_frac_y_ += scroll_vel_y_ - (dy << 16);
    int32_t carry_x = scroll_frac_x_ >> 16;
    int32_t carry_y = scroll_frac_y_ >> 16;
    scroll_frac_x_ -= carry_x << 16;
    scroll_frac_y_ -= carry_y << 16;
    dx += carry_x;
    dy += carry_y;
    // The mouse drag's own pixels ride along with the velocity's.
    int32_t drag_x = int32_t(scroll_x_), drag_y = int32_t(scroll_y_);
    scroll_x_ -= float(drag_x);
    scroll_y_ -= float(drag_y);
    dx += drag_x;
    dy += drag_y;
    // The game refuses, dead, a scroll that would take the view's centre out
    // of the level's scroll area: both axes stop and the velocity is lost.
    Point centre = {camera_.x + surface_.width() / 2, camera_.y + surface_.height() / 2};
    if (beyond_scroll_limit({centre.x + dx, centre.y + dy}, scroll_min_, scroll_max_)) {
        scroll_vel_x_ = 0;
        scroll_vel_y_ = 0;
        scroll_frac_x_ = 0;
        scroll_frac_y_ = 0;
        dx = 0;
        dy = 0;
    }
    dx = std::clamp(dx, -kTileWidth, kTileWidth);
    dy = std::clamp(dy, -kTileHeight, kTileHeight);
    camera_.x += dx;
    camera_.y += dy;
    scroll_vel_x_ = (scroll_vel_x_ * kScrollDecay) >> 8;
    scroll_vel_y_ = (scroll_vel_y_ * kScrollDecay) >> 8;
}

bool App::screen_step(int out_w, int out_h)
{
    if (in_startup_) {
        startup_step(out_w, out_h);
    } else if (map_ && year_end_showing_) {
        year_end_step(out_w, out_h);
    } else if (map_ && winlose_showing_) {
        winlose_step(out_w, out_h);
    } else if (map_ && fax_.open) {
        fax_step(out_w, out_h);
    } else if (map_ && charts_.open) {
        charts_step(out_w, out_h);
    } else if (map_ && (town_.is_open() || std::exchange(open_town_, false))) {
        if (!town_.is_open())
            open_town_map();
        town_step(out_w, out_h);
    } else {
        return false;
    }
    mixer_.update(sounds_, &music_);
    draw_pointer(ui_scale());
    return true;
}

void App::render()
{
    frame_++;
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    int out_w = 0, out_h = 0;
    SDL_GetCurrentRenderOutputSize(renderer_, &out_w, &out_h);
    SDL_SetRenderScale(renderer_, 1.0f, 1.0f);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    if (!data_) {
        setup_step();
    } else if (!fade_frame(out_w, out_h)) {
        fader_.begin_frame(renderer_, out_w, out_h);
        if (!screen_step(out_w, out_h))
            hospital_step(out_w, out_h);
        if (std::exchange(screen_shot_requested_, false))
            take_screen_shot();
        fader_.end_frame(renderer_);
    }

    if (data_)
        draw_tools(tools_, map_ && !in_startup_ ? &map_->game.world : nullptr);
    // Read back before ImGui draws, so the screenshot shows the game alone.
    if (std::exchange(tools_.report.save, false))
        save_debug_report();
    draw_quick_report();
    if (!drive_shot_.empty())
        save_drive_screenshot(std::exchange(drive_shot_, {}));

    ImGui::Render();
    const ImGuiIO &io = ImGui::GetIO();
    SDL_SetRenderScale(renderer_, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
    if (!drive_shot_ui_.empty())
        save_drive_screenshot(std::exchange(drive_shot_ui_, {}));
    SDL_RenderPresent(renderer_);
    buttons_.left_pressed = buttons_.right_pressed = false;
    buttons_.left_released = buttons_.right_released = false;
}

void App::save_drive_screenshot(const std::filesystem::path &path)
{
    SDL_Surface *shot = SDL_RenderReadPixels(renderer_, nullptr);
    if (!shot || !SDL_SavePNG(shot, path.string().c_str()))
        std::fprintf(stderr, "drive: screenshot %s: %s\n", path.string().c_str(), SDL_GetError());
    if (shot)
        SDL_DestroySurface(shot);
}

void App::take_screen_shot()
{
    // scrshots/hospNNNN.png beside the saves; the first shot of a run takes
    // the first number not used yet.
    std::filesystem::path dir = save_dir_.parent_path() / "scrshots";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    auto name = [](int n) { return std::format("hosp{:04}.png", n); };
    if (screen_shot_number_ == -1) {
        int i = 0;
        while (i < 10000 && std::filesystem::exists(dir / name(i), ec))
            i++;
        screen_shot_number_ = i - 1;
    }
    int number = ++screen_shot_number_;
    if (number >= 10000)
        return;
    // The frame just drawn, with its interface and pointer.
    SDL_Surface *shot = SDL_RenderReadPixels(renderer_, nullptr);
    if (!shot)
        return;
    const std::filesystem::path path = dir / name(number);
    const bool saved = SDL_SavePNG(shot, path.string().c_str());
    SDL_DestroySurface(shot);
    if (!saved)
        return;
    screen_shots_++;
    // The browser's storage is no place to keep pictures: the shot is
    // downloaded and removed.
    if (web::kEnabled) {
        web::download_file(path);
        std::filesystem::remove(path, ec);
    }
}

bool App::screen_shot_key(const SDL_KeyboardEvent &key)
{
    if (key.key != SDLK_C || !(key.mod & SDL_KMOD_ALT))
        return false;
    screen_shot_requested_ = true;
    return true;
}

bool App::fade_frame(int out_w, int out_h)
{
    if (!std::exchange(boot_faded_, true)) {
        // setup_host shows the loading screen once, before the intro; a level
        // started at once (--level) fades in from it.
        std::string error;
        if (load_loading_screen(data_->dir(), loading_, &error))
            fader_.queue_boot(!in_startup_);
        else
            std::fprintf(stderr, "%s\n", error.c_str());
    }
    fader_.screen(screen_id());
    bool shown = fader_.present(renderer_, out_w, out_h, frame_seconds_, [&] { draw_loading_picture(out_w, out_h); });
    if (shown)
        mixer_.update(sounds_, &music_);
    return shown;
}

ScreenId App::screen_id() const
{
    if (in_startup_) {
        FadeScreen kind = startup_.screen() == StartupScreen::Movie ? FadeScreen::Movie : FadeScreen::Startup;
        int sub = kind == FadeScreen::Movie ? startup_.movie() : int(startup_.screen());
        return {kind, sub};
    }
    if (!map_)
        return {};
    if (year_end_showing_)
        return {FadeScreen::YearEnd, int(year_end_.phase()) * 8 + year_end_phase_};
    if (winlose_showing_)
        return {winlose_movie_index_ >= 0 ? FadeScreen::WinLoseMovie : FadeScreen::WinLoseLetter, 0};
    if (fax_.open)
        return {FadeScreen::Fax, 0};
    if (charts_.open)
        return {FadeScreen::Chart, charts_.chart};
    if (town_.is_open())
        return {FadeScreen::TownMap, 0};
    return {FadeScreen::Hospital, 0};
}

void App::draw_loading_picture(int out_w, int out_h)
{
    SDL_FRect at = fit_rect(loading_.pixels, out_w, out_h);
    draw_fitted(loading_.pixels, loading_.palette, loading_tex_, at);
}

// One frame of the hospital: the view at its zoom with what is being placed
// over it, its sounds, then the interface, the menu bar and the pointer at
// the UI scale.
void App::hospital_step(int out_w, int out_h)
{
    int z = zoom();
    int vw = std::max(out_w / z, 1), vh = std::max(out_h / z, 1);
    resize_view(vw, vh);

    float cmx, cmy;
    SDL_GetMouseState(&cmx, &cmy);
    float cdensity = SDL_GetWindowPixelDensity(window_);
    pointed_.at = {int(cmx * cdensity) / z, int(cmy * cdensity) / z};
    near_rat_ = false;
    frame_sounds_.clear();
    // The earthquake's shake offset (shake_screen) moved the original view.
    Point shaken = {camera_.x + map_->game.world.shake_x, camera_.y + map_->game.world.shake_y};
    // What the interface of the frame before left on the map (the game's
    // interface runs after the view): the cells the carried object covers
    // (draw_object_cursor) and what flashes (othervar.pickup_thing and
    // queue_hilight_thing).
    std::vector<CellMark> marks;
    if (object_ui_.cursor_shown)
        marks = placement_marks(map_->game.world, map_->game.world.placing);
    map_->overlays = {&marks, build_ui_.pickup_thing, queue_.hilight};
    draw_view(surface_, *data_, map_.get(), shaken, shadows_, {&frame_sounds_, &pointed_, &near_rat_});
    if (map_)
        view_sounds(vw, vh);
    mixer_.update(sounds_, &music_);

    // What is being placed, drawn over the view as the game does.
    Point under = world_to_cell({camera_.x + pointed_.at.x, camera_.y + pointed_.at.y});
    bool inside = under.x >= 0 && under.x < kMapSize && under.y >= 0 && under.y < kMapSize;
    map_cell_ = inside ? under.x + under.y * kMapSize : -1;
    if (map_)
        draw_view_overlays(vw, vh);
    shade_view();
    present_view(vw, vh, z);

    // The interface and the pointer keep their size whatever the view's
    // zoom.
    int s = ui_scale();
    interface_step(out_w, out_h, s);
    menu_step(out_w, s);

    draw_pointer(s, menu_.active || interface_.modal() ? kPointerDefault : pointer_sprite_);

    if (tools_.open && map_)
        draw_nav_overlay(tools_.nav, map_->game.world, camera_, float(z) / ImGui::GetIO().DisplayFramebufferScale.x);
}

// Makes the view's surface and texture `vw` x `vh`.
void App::resize_view(int vw, int vh)
{
    const int n = upscale_.on() ? upscale_.factor : 1;
    if (vw != surface_.width() || vh != surface_.height() || n != surface_.scale()) {
        // Keep the same map point at the view's centre across resizes.
        Point centre = {camera_.x + surface_.width() / 2, camera_.y + surface_.height() / 2};
        surface_.resize(vw, vh, n);
        camera_ = {centre.x - vw / 2, centre.y - vh / 2};
    }
    if (!screen_ || screen_w_ != vw * n || screen_h_ != vh * n) {
        if (screen_)
            SDL_DestroyTexture(screen_);
        screen_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, vw * n, vh * n);
        SDL_SetTextureScaleMode(screen_, SDL_SCALEMODE_NEAREST);
        screen_w_ = vw * n;
        screen_h_ = vh * n;
    }
}

// The sounds of what was drawn, positioned on the `vw` x `vh` view.
void App::view_sounds(int vw, int vh)
{
    sounds_.frames_drawn(frame_sounds_, vw);
    sounds_.manage_sound(map_->game.world, camera_, vw, vh);
    const World::Emergency &emergency = map_->game.world.emergency;
    sounds_.manage_background_sounds(map_->game.paused,
                                     emergency.timer > 0 ? std::optional<int>(emergency.timer) : std::nullopt);
    // twice_daily_admin stops the clock as the emergency's time runs out.
    if (emergency.timer <= 0 && sounds_.playing(kSourceClock, 0xa9))
        sounds_.stop_sample(kSourceClock, 0xa9);
    // The worn machines' warnings and announcements, and explosions.
    World &world = map_->game.world;
    for (uint8_t sfx : world.machine_sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    for (uint8_t announcement : world.machine_announcements)
        sounds_.play_ambient(announcement, has_receptionist(world), map_->game.paused);
    world.machine_sounds.clear();
    world.machine_announcements.clear();
    for (uint8_t sfx : world.epidemic_sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    world.epidemic_sounds.clear();
    for (uint8_t sfx : world.treatment_sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    world.treatment_sounds.clear();
    for (uint8_t sfx : world.rat_sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    world.rat_sounds.clear();
    // The music moves on to the next song, and the announcer speaks, as
    // the game does each frame; only the announcer while a modal
    // requester is up.
    if (!interface_.modal())
        music_.play_next(map_->game.paused);
    sounds_.do_ambients(map_->game.paused, map_->game.world.rats.fest_level != 0, has_receptionist(map_->game.world));
}

// What is being placed (an object, a room's blueprint, a member of staff),
// over the `vw` x `vh` view.
void App::draw_view_overlays(int vw, int vh)
{
    Rect view{0, 0, vw, vh};
    draw_placement(surface_, view, camera_, map_->game.world, object_ui_, *data_);
    draw_build_overlay({surface_, view, camera_}, map_->game, build_ui_, object_ui_, *data_);
    if (hire_.placing() && map_cell_ >= 0)
        draw_held_staff({surface_, view, camera_}, hire_, map_->game.world, map_cell_, *data_, map_->scratch);
}

// Whether the paused game is shaded: the player's pause, not the one the F10
// report takes for its capture.
bool App::pause_shaded() const
{
    return map_ && map_->game.paused && !(quick_.have_pause && !quick_.restore_paused);
}

// draw_pause_mode (src/game.cpp): the paused view is blended with dark blue
// through the ghost table (draw flag 4).
void App::shade_view()
{
    if (!pause_shaded())
        return;
    const uint8_t *ghost = data_->ghost().data();
    const size_t blend = size_t(colour_dark_blue(map_->game.world)) << 8;
    for (int y = 0; y < surface_.pixel_height(); y++) {
        uint8_t *row = surface_.row(y);
        for (int x = 0; x < surface_.pixel_width(); x++)
            row[x] = ghost[blend | row[x]];
    }
}

// The same shade over what the interface layer drew, which the game also
// shaded; the help text it draws afterwards is not.
void App::shade_ui(UiLayer &layer)
{
    if (pause_shaded())
        layer.shade(colour_dark_blue(map_->game.world), data_->ghost().data());
}

// The `vw` x `vh` view in the window at zoom `z`.
void App::present_view(int vw, int vh, int z)
{
    void *pixels;
    int pitch;
    if (!screen_ || !SDL_LockTexture(screen_, nullptr, &pixels, &pitch))
        return;
    const Palette &pal = data_->palette();
    const uint8_t *src = surface_.data();
    const int pw = surface_.pixel_width(), ph = surface_.pixel_height();
    for (int y = 0; y < ph; y++) {
        auto *row = reinterpret_cast<uint32_t *>(static_cast<uint8_t *>(pixels) + y * pitch);
        for (int x = 0; x < pw; x++)
            row[x] = pal[src[x]];
        src += pw;
    }
    SDL_UnlockTexture(screen_);
    SDL_FRect dst = {0, 0, float(vw * z), float(vh * z)};
    set_scale_mode_for(screen_, dst.w);
    SDL_RenderTexture(renderer_, screen_, nullptr, &dst);
}

} // namespace th
