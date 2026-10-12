// Drawing the interface layer, the help boxes and the pointer, and putting a
// picture or layer in the window.

#include "app/app.h"
#include "app/app_internal.h"
#include "core/data/gamedata.h"
#include "core/ui/panel_info.h"

#include <imgui.h>

#include <cmath>

namespace th
{

inline namespace app_detail
{

// Where `src` goes in an `out_w` x `out_h` window, as large as fits whole and
// centred: at a whole-number scale where it can be (a window smaller than
// `src` shows it reduced to fit).
SDL_FRect fit_rect(const Surface &src, int out_w, int out_h)
{
    int sw = std::max(src.width(), 1), sh = std::max(src.height(), 1);
    float fit = std::min(float(out_w) / float(sw), float(out_h) / float(sh));
    if (fit >= 1.0f)
        fit = std::floor(fit);
    float dw = sw * fit, dh = sh * fit;
    return {(out_w - dw) / 2, (out_h - dh) / 2, dw, dh};
}

} // namespace app_detail

// Draws the panel, the advisor and the requesters into the interface's
// canvas, and the canvas at (ox, oy) of the window at `scale`.
// The tutorial's flashing hint, as panel_flash and the requesters' drawers
// have it (the panel's is for highlights 0x10 to 0x1b whether or not the
// tutorial is on, and comes first in a frame).
void App::flash_tutorial(const Game *game)
{
    TutorialFlash &flash = interface_.flash();
    flash.tutorial = game && (game->advisor.flags & kAdvisorTutorial);
    flash.highlight = game ? game->advisor.tutorial_highlight : 0;
    if (game && flash.highlight >= 0x10 && flash.highlight <= 0x1b && flash.tick()) {
        draw_panel_icon_pressed(ui_, panel_state_, flash.highlight - 0x10, *data_);
        flash.drawn++;
    }
}

void App::interface_draw(Game *game, int ox, int oy, int scale)
{
    ui_.clear();
    PanelFigures figures;
    if (map_) {
        const Game &g = map_->game;
        figures = {g.balance, g.date.day, g.date.month, g.reputation(), g.opening_countdown, g.open_countdown};
        figures.emergency_timer = g.world.emergency.timer;
        figures.emergency_state = g.world.emergency.state;
        figures.epidemic_timer = g.world.epidemic.timer;
        figures.epidemic_illness = g.world.epidemic.illness != 0;
        // The watch is not shown while the tutorial holds the countdown.
        if (g.advisor.flags & kAdvisorTutorial)
            figures.opening_countdown = 0;
    }
    draw_panel(game, figures);
    if (game) {
        draw_advisor(ui_, game->advisor, *data_, data_->font(0x1d));
        advance_balance_roll(panel_state_, game->balance);
        draw_info_view(*game);
    }
    draw_interface(ui_, interface_, requester_art_);
    if (game)
        draw_info_windows(ui_, info_, interface_, {requester_art_, hire_art_, data_->text()}, game->world,
                          object_ui_.red);
    if (game)
        draw_queue_window(ui_, queue_, interface_, *game, requester_art_, *data_);
    if (interface_.requester() == Requester::Jukebox)
        draw_jukebox(ui_, interface_, requester_art_, music_);
    if (game) {
        draw_object_requesters(ui_, interface_, object_ui_, *game, requester_art_, *data_);
        draw_build_requester(ui_, interface_, build_ui_, *game, requester_art_, *data_);
        draw_hire_staff(ui_, hire_, interface_, {requester_art_, hire_art_, data_->text()}, game->world);
        draw_staff_demand(ui_, demand_, interface_, {requester_art_, hire_art_, data_->text()}, *data_, game->world);
    }
    shade_ui(ui_);
    help_draw();
    SDL_FRect dst = {float(ox), float(oy), float(kUiWidth * scale), float(kUiHeight * scale)};
    draw_layer(ui_scaled_, ui_tex_, ui_.to_argb(data_->palette()), kUiWidth, dst);
}

// The panel with what is drawn over it: the rat game's (score, weapons, litter
// bar), or the hospital's icons with its figures, information bar, fax icons
// and watch.
void App::draw_panel(const Game *game, const PanelFigures &figures)
{
    if (map_ && map_->game.world.rats.fest_level) {
        const Game &g = map_->game;
        draw_rat_panel(ui_, {g.progress.rats_killed, g.world.litter_count, g.world.rats.fest_powerup, g.turn_count},
                       *data_);
        flash_tutorial(game);
        return;
    }
    draw_panel_ui(ui_, panel_state_, map_ ? &figures : nullptr, *data_);
    flash_tutorial(game);
    if (map_ && game)
        draw_panel_bar(*game);
    if (map_) {
        draw_popup_icons(ui_, popups_, *data_, panel_state_, &figures);
        draw_emergency(ui_, figures, *data_);
    }
}

// The panel's information bar: the special message while one is up, else
// the thing under the pointer.
void App::draw_panel_bar(const Game &game)
{
    if (panel_message_.empty()) {
        panel_info_drawn_ = draw_panel_info({ui_, panel_top(*data_), requester_art_.font(), panel_state_.raised},
                                            panel_info_, panel_thing_, game, *data_);
    } else {
        panel_info_drawn_ = 0;
        draw_panel_special_info();
    }
}

// display_panel_special_info: the saved/cannot-save/load-failed message on
// the panel's information bar, for its remaining frames. The pointer over the
// panel clears it at once, unless the game is paused (panel.cpp:1558-1560).
void App::draw_panel_special_info()
{
    if (map_ && !map_->game.paused && help_mouse_.y >= panel_top(*data_)) {
        panel_message_.clear();
        panel_message_time_ = 0;
        return;
    }
    draw_panel_message({ui_, panel_top(*data_), requester_art_.font()}, panel_message_, *data_);
    if (panel_message_time_ > 0 && --panel_message_time_ == 0)
        panel_message_.clear();
}

// The help tooltip of the screen open this frame: the original computes the
// front requester's group in its overdraw (draw_main_window), then the
// panel's group 3 in draw_all_windows, every hospital frame.
void App::interface_help()
{
    if (interface_.modal()) {
        help_screen(8, 0, 0); // req13, the yes/no requester
    } else if (uint8_t group = requester_help_group(interface_.requester())) {
        const Window *w = nullptr;
        Requester r = interface_.requester();
        if (r == Requester::BuildRoom || r == Requester::HireStaff || r == Requester::EditRoom || r == Requester::Queue)
            w = &interface_.window(int(r));
        help_screen(group, w ? w->x : 0, w ? w->y : 0);
    }
    if (queue_.help_group)
        help_screen(queue_.help_group, queue_.help_x, queue_.help_y);
    help_screen(3, 0, 0); // the panel, after the requester
}

void App::interface_room_help(Game &game)
{
    uint8_t built = game.world.building.last_built_room;
    if (built == last_built_room_)
        return;
    last_built_room_ = built;
    if (built == 0 || built >= kRoomTypes)
        return;
    // The original's room_help_seen has 30 entries and is indexed by room
    // type up to 30: the decontamination room's flag is advisor_flags[0],
    // the advice given once a research room is built.
    uint8_t &seen =
        built < details_.room_help_seen.size() ? details_.room_help_seen[built] : game.advisor.given[built - 30];
    if (seen)
        return;
    seen = 1;
    // open_room closes the edit room requester before the page; the page
    // then holds the screen, so the next frame's sync would come too late.
    sync_object_requesters(interface_, game);
    help_.show_help_page(data_->text(), built);
}

void App::help_screen(int group, int x, int y)
{
    if (!data_)
        return;
    help_context();
    help_.compute_help_text(help_ctx_, x, y, uint8_t(group));
}

void App::load_help_art(const std::filesystem::path &dir)
{
    std::string error;
    if (!load_font(dir, 0, help_font_, &error))
        std::fprintf(stderr, "%s\n", error.c_str());
    if (load_font(dir, 0x2d, help_menu_font_, &error))
        help_.set_main_menu_font(help_menu_font_);
    else
        std::fprintf(stderr, "%s\n", error.c_str());
    help_.set_art(help_font_, menu_.sprites);
    help_.set_ghost(data_->ghost().data());
}

void App::help_context()
{
    help_ctx_ = HelpContext{};
    if (map_) {
        help_ctx_.game = &map_->game;
        help_ctx_.world = &map_->game.world;
        help_ctx_.placing = &map_->game.world.placing;
    }
    help_ctx_.data = data_.get();
    help_ctx_.text = &data_->text();
    help_ctx_.ui = &interface_;
    help_ctx_.hire = &hire_;
    help_ctx_.info = &info_;
    help_ctx_.charts = &charts_;
    help_ctx_.object_ui = &object_ui_;
    help_ctx_.build = &build_ui_;
    help_ctx_.mouse = &help_mouse_;
    help_ctx_.pulldv = &menu_.sprites;
    help_ctx_.help = &help_.box();
    help_.set_clock(ticks_ms());
}

void App::wire_screen_help()
{
    town_.set_help(help_, help_ctx_);
    charts_.set_help(help_, help_ctx_);
    startup_.set_help(help_, help_ctx_);
}

void App::help_draw()
{
    if (!data_ || help_.box().msg.empty())
        return;
    // mouse_mode_help_allowed: a tooltip only in the normal mode, not while
    // something is placed or built; the advisor's tutorial overrides. A
    // framed page is MM_HELP, which is allowed.
    bool page = help_.box().style == 2;
    bool allowed = page;
    bool tutorial = false;
    if (map_) {
        const Game &g = map_->game;
        tutorial = (g.advisor.flags & kAdvisorTutorial) != 0;
        allowed = allowed || (!g.world.placing.placing && !build_tool_busy(g) && !hire_.placing() &&
                              interface_.requester() != Requester::EditRoom);
    }
    if (!(allowed || tutorial)) {
        // The tooltip was not computed in a mode that may show it; drop it so
        // it cannot reappear once the mode is allowed again.
        help_.box().msg.clear();
        return;
    }
    help_.draw(ui_.pixels(), data_->palette(), allowed, tutorial);
    const HelpBox &b = help_.box();
    ui_.mark({b.x, b.y, b.w, b.h});
    // The framed page's shadow is a ghost blend in the original, but the
    // interface layer is transparent, so it is drawn translucent instead.
    for (Rect r : help_.page_shadow_rects())
        ui_.shadow(r, HelpSystem::kPageShadowAlpha);
}

// The view window over the query window showing a person, under its art.
void App::draw_info_view(Game &game)
{
    Rect view_box;
    Point view_camera;
    if (!info_view(info_, interface_, game.world, view_box, view_camera))
        return;
    draw_map_view(ui_.pixels(), game, view_box, view_camera);
    ui_.mark(view_box);
}

// The world seen from `camera`, drawn into `box` of `dst`.
void App::draw_map_view(Surface &dst, const Game &game, Rect box, Point camera)
{
    if (info_view_.width() != box.w || info_view_.height() != box.h) {
        info_view_.resize(box.w, box.h);
        info_scratch_.resize(box.w, box.h);
        info_scratch_.fill(kMaskColour);
    }
    info_view_.fill(0);
    draw_world({info_view_, {0, 0, box.w, box.h}, camera, game.turn_count}, game.world, *data_,
               shadows_ ? map_->shadows.get() : nullptr, info_scratch_);
    for (int y = 0; y < box.h; y++)
        for (int x = 0; x < box.w; x++)
            if (box.x + x >= 0 && box.x + x < dst.width() && box.y + y >= 0 && box.y + y < dst.height())
                dst.row(box.y + y)[box.x + x] = info_view_.row(y)[x];
}

// Asks "really quit?" with the yes/no requester, as the game does.
void App::ask_quit()
{
    if (interface_.modal() || !data_)
        return;
    interface_.open_yes_no(data_->text().at(TextSection::YesNo, 0));
    asking_quit_ = true;
}

// File > Restart: ask_restart_level's yes/no requester.
void App::ask_restart()
{
    if (!map_ || interface_.modal())
        return;
    interface_.open_yes_no(data_->text().at(TextSection::YesNo, 6));
    yes_event_ = {EV_RESTART_LEVEL};
    no_event_ = {};
    asking_quit_ = false;
}

// The game's own pointer `sprite`, in place of the system one over the
// game; its hotspot is its top-left corner.
void App::draw_pointer(int scale, uint8_t sprite)
{
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    float density = SDL_GetWindowPixelDensity(window_);
    const PointerTexture &pt =
        pointers_[sprite < pointers_.size() && pointers_[sprite].texture ? sprite : kPointerDefault];
    bool over_game = SDL_GetMouseFocus() == window_ && !ImGui::GetIO().WantCaptureMouse;
    bool crosshair = tools_.inspect.picking && over_game;
    if (crosshair && !crosshair_)
        crosshair_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
    if (crosshair != crosshair_shown_) {
        SDL_SetCursor(crosshair ? crosshair_ : SDL_GetDefaultCursor());
        crosshair_shown_ = crosshair;
    }
    if (crosshair) {
        SDL_ShowCursor();
    } else if (pt.texture && over_game) {
        SDL_HideCursor();
        SDL_FRect p = {mx * density, my * density, float(pt.w * scale), float(pt.h * scale)};
        set_scale_mode_for(pt.texture, p.w);
        SDL_RenderTexture(renderer_, pt.texture, nullptr, &p);
    } else {
        SDL_ShowCursor();
    }
}

UiMouse App::fitted_mouse(const SDL_FRect &at, const Surface &src) const
{
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    float density = SDL_GetWindowPixelDensity(window_);
    float fit = at.w / float(std::max(src.width(), 1));
    UiMouse mouse = ui_mouse(0, 0, 1);
    mouse.x = int((mx * density - at.x) / fit);
    mouse.y = int((my * density - at.y) / fit);
    return mouse;
}

void App::draw_layer(ScaledTexture &scaled, SDL_Texture *plain, const std::vector<uint32_t> &pixels, int w,
                     const SDL_FRect &dst)
{
    const int h = int(pixels.size()) / w;
    SDL_Texture *tex = scaled.update(renderer_, upscale_, pixels.data(), w, h);
    if (tex) {
        set_scale_mode_for(tex, dst.w);
    } else {
        SDL_UpdateTexture(plain, nullptr, pixels.data(), w * 4);
        tex = plain;
    }
    SDL_RenderTexture(renderer_, tex, nullptr, &dst);
}

void App::draw_fitted(const Surface &src, const Palette &palette, SDL_Texture *&tex, const SDL_FRect &dst,
                      bool upscalable)
{
    int sw = std::max(src.width(), 1), sh = std::max(src.height(), 1);
    if (upscalable && upscale_.on() && src.width() > 0) {
        std::vector<uint32_t> argb(size_t(sw) * size_t(sh));
        for (size_t i = 0; i < argb.size(); i++)
            argb[i] = palette[src.data()[i]];
        if (SDL_Texture *scaled = fit_scaled_.update(renderer_, upscale_, argb.data(), sw, sh)) {
            set_scale_mode_for(scaled, dst.w);
            SDL_RenderTexture(renderer_, scaled, nullptr, &dst);
            fit_.at = dst;
            fit_.w = sw;
            fit_.h = sh;
            fit_.frame = frame_;
            return;
        }
    }
    if (!tex || tex->w != sw || tex->h != sh) {
        if (tex)
            SDL_DestroyTexture(tex);
        tex = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, sw, sh);
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    }
    void *pixels;
    int pitch;
    if (tex && src.width() > 0 && SDL_LockTexture(tex, nullptr, &pixels, &pitch)) {
        const uint8_t *p = src.data();
        for (int y = 0; y < sh; y++) {
            auto *row = reinterpret_cast<uint32_t *>(static_cast<uint8_t *>(pixels) + y * pitch);
            for (int x = 0; x < sw; x++)
                row[x] = palette[*p++];
        }
        SDL_UnlockTexture(tex);
        SDL_RenderTexture(renderer_, tex, nullptr, &dst);
        fit_.at = dst;
        fit_.w = sw;
        fit_.h = sh;
        fit_.frame = frame_;
    }
}

} // namespace th
