// The full-screen screens: the town map, the fax, the charts, the year end and
// the start-up screens, with the frame's command and sounds.

#include "app/app.h"
#include "app/app_internal.h"
#include "app/web.h"
#include "core/data/gamedata.h"
#include "core/world/rooms.h"

namespace th
{

inline namespace app_detail
{

// A key pressed in the start-up screens, which read keys as lbInkey.
StartupKey startup_key(SDL_Keycode key)
{
    StartupKey k;
    if (key >= SDLK_A && key <= SDLK_Z) {
        k.code = StartupKey::Letter;
        k.letter = char('A' + (key - SDLK_A));
    } else if (key == SDLK_ESCAPE) {
        k.code = StartupKey::Escape;
    } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        k.code = StartupKey::Return;
    } else if (key == SDLK_SPACE) {
        k.code = StartupKey::Space;
    } else if (key == SDLK_UP) {
        k.code = StartupKey::Up;
    } else if (key == SDLK_DOWN) {
        k.code = StartupKey::Down;
    } else if (key == SDLK_PAGEUP) {
        k.code = StartupKey::PageUp;
    } else if (key == SDLK_PAGEDOWN) {
        k.code = StartupKey::PageDown;
    }
    return k;
}

} // namespace app_detail

// Opens the town map, first dropping what is being hired, bought or placed
// (setup_abort_all_requesters), as the game does.
void App::open_town_map()
{
    if (!map_ || town_.is_open())
        return;
    Game &game = map_->game;
    if (hire_.placing() || interface_.requester() == Requester::HireStaff ||
        interface_.requester() == Requester::Jukebox)
        game.event = hire_staff_cancel(hire_);
    else
        object_escape(game);
    town_.open();
}

// One frame of the town map, in place of the view and the interface, drawn
// aspect-fit in the window; then the turn's command, as interface_step
// carries it out.
void App::town_step(int out_w, int out_h)
{
    Game &game = map_->game;
    const Surface &src = town_.surface();
    SDL_FRect at = fit_rect(src, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, src);
    help_mouse_ = mouse;
    help_context();
    bool escape = escape_;
    escape_ = false;
    auto go_to = [this](int column, int row) {
        Point centre = cell_to_world(column, row);
        if (beyond_scroll_limit(centre, scroll_min_, scroll_max_))
            return false;
        camera_ = {centre.x - surface_.width() / 2, centre.y - surface_.height() / 2};
        return true;
    };
    TownMapResult r = town_.frame({mouse, escape}, game, game.land, panel_state_, startup_.player_name(), go_to);
    for (uint8_t sfx : r.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    if (r.event.type)
        game.event = r.event;
    if (r.closed || r.event.type == EV_OPEN_SCREEN)
        town_.close();
    last_event_ = game.event;
    run_command(game);
    draw_fitted(src, town_.palette(), town_tex_, at);
}

// setup_abort_all_requesters' question over an edited room: yes deletes it
// (the requester closes with the room), no goes back to the blueprint.
void App::edited_room_question(Game &game)
{
    if (game.event.type == EV_OPEN_YES_NO) {
        interface_.open_yes_no(data_->text().at(TextSection::YesNo, game.event.value));
        yes_event_ = {EV_DELETE_ROOM, 0, 0, 0};
        no_event_ = {EV_CLOSE_YES_NO, 1, 0, uint16_t(BuildMouse::Blueprint)};
        game.event = {};
    }
    if (game.event.type == EV_DELETE_ROOM)
        interface_.close_yes_no(false);
}

void App::run_command(Game &game)
{
    // EV_OPEN_SCREEN is refused while an edited room empties (io.cpp:824).
    if (game.event.type == EV_OPEN_SCREEN && game.world.building.mouse == BuildMouse::EditRoomWait) {
        sounds_.play_sound(kSourceGeneral, kSfxWrong2);
        game.event = {};
    }
    edited_room_question(game);
    bool restart = game.event.type == EV_RESTART_LEVEL;
    // Answering a member of staff's resignation call opens the staff demand
    // requester once the command has removed the call (io.cpp's
    // EV_ANSWER_PLAYERCALL).
    uint16_t demand_staff = 0;
    if (game.event.type == EV_ANSWER_PLAYERCALL && game.event.value == 1)
        demand_staff = playercall_subject(game.world, 1, game.event.arg, 0);
    // The demand's answers close its requester (process_turn's
    // close_crequester and close_irequesters).
    bool demand_answer = game.event.type == EV_REFUSE_STAFF_DEMAND || game.event.type == EV_RAISE_STAFF_SALARY ||
                         game.event.type == EV_GIVE_STAFF_BONUS;
    hire_staff_event(hire_, interface_, game.world, game.event);
    jukebox_event(jukebox_, interface_, game.event);
    chart_command(charts_, game.event);
    // EV_SACK_STAFF and EV_CANCEL_SACK close the yes/no requester that asked.
    if (game.event.type == EV_SACK_STAFF || game.event.type == EV_CANCEL_SACK)
        interface_.close_yes_no(true);
    if (game.event.type == EV_SACK_STAFF)
        sounds_.play_sound(kSourceGeneral, kSfxSack);
    // EV_CANCEL_SACK plays SFX_NO4 (io.cpp:1841), on top of the No button's.
    else if (game.event.type == EV_CANCEL_SACK)
        sounds_.play_sound(kSourceGeneral, kSfxNo4);
    info_window_event(info_, interface_, game.world, game.event);
    queue_window_event(queue_, interface_, game.world, game.event);
    fax_command(fax_, game.event);
    game.process_turn();
    if (demand_answer) {
        interface_.close_requester(Requester::StaffDemand);
        close_irequesters(interface_);
    }
    // compute_whether_user_not_busy (user_idle): the demand opens only with
    // no window open and nothing being placed.
    if (demand_staff && !interface_.modal() && !game.world.placing.placing && !build_tool_busy(game) &&
        !hire_.placing()) {
        sounds_.play_sound(kSourceGeneral, kSfxStaffDemand);
        open_staff_resign_requester(demand_, interface_, game.world, demand_staff, sounds_.safeseed);
    }
    // display_choice_screen, from the command's answer to a call.
    if (game.choice_screen.type) {
        close_irequesters(interface_);
        display_choice_screen(fax_, game.choice_screen);
    }
    hire_staff_after_turn(hire_, game.world);
    for (uint8_t sfx : game.world.building.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    game.world.building.sounds.clear();
    game_sounds(game);
    check_autosave(game);
    if (restart)
        restart_level_ = true;
}

void App::game_sounds(Game &game)
{
    // add_panel_popup's SFX_NEWFAX.
    constexpr uint8_t kSfxNewFax = 0xdd;
    // research_category_exhausted's SFX_RESEARCH_EXHAUSTED.
    constexpr uint8_t kSfxResearchExhausted = 0xdb;
    for (; game.world.popups.new_faxes; game.world.popups.new_faxes--)
        sounds_.play_sound(kSourceGeneral, kSfxNewFax);
    for (; game.world.research.exhausted; game.world.research.exhausted--)
        sounds_.play_sound(kSourceGeneral, kSfxResearchExhausted);
    // summon_vehicle's announcement of the emergency's illness, as its
    // helicopter is sent.
    const World::Emergency &e = game.world.emergency;
    if (e.heli && e.heli != heli_seen_ && e.illness < 0x24)
        sounds_.play_ambient(kAnnounceEmergency + e.illness, has_receptionist(game.world), game.paused);
    heli_seen_ = e.heli;
}

// One frame of the fax, in place of the view and the interface, drawn
// aspect-fit in the window; then the turn's command.
void App::fax_step(int out_w, int out_h)
{
    Game &game = map_->game;
    SDL_FRect at = fit_rect(fax_.surface, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, fax_.surface);
    FaxKeys keys;
    keys.escape = std::exchange(escape_, false) || std::exchange(chart_space_, false);
    FaxResult r = fax_frame(fax_, mouse, keys, frame_seconds_, game, sounds_.safeseed);
    for (uint8_t sfx : r.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    if (r.event.type)
        game.event = r.event;
    last_event_ = game.event;
    run_command(game);
    draw_fitted(fax_.surface, fax_.palette, fax_tex_, at);
}

// One frame of the open chart, in place of the view and the interface,
// drawn aspect-fit in the window; then the turn's command.
// Escape over a chart: process_keys runs there as over the hospital, so it
// also aborts what is open behind the chart (setup_abort_all_requesters), the
// jukebox among it. The abort's command runs now; the event the frame had
// pending stays. Returns whether Escape was pressed, for the chart.
bool App::chart_escape(Game &game)
{
    const bool escape = escape_;
    Event pending = std::exchange(game.event, {});
    interface_escape(&game);
    if (game.event.type)
        run_command(game);
    game.event = pending;
    return escape;
}

void App::charts_step(int out_w, int out_h)
{
    Game &game = map_->game;
    SDL_FRect at = fit_rect(charts_.surface, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, charts_.surface);
    help_mouse_ = mouse;
    help_context();
    const bool *held = SDL_GetKeyboardState(nullptr);
    ChartKeys keys;
    keys.escape = chart_escape(game);
    keys.space = std::exchange(chart_space_, false);
    keys.up = held[SDL_SCANCODE_UP];
    keys.down = held[SDL_SCANCODE_DOWN];
    keys.left = held[SDL_SCANCODE_LEFT];
    keys.right = held[SDL_SCANCODE_RIGHT];
    keys.pad_held = pad_held_;
    keys.pad_pressed = std::exchange(pad_pressed_, uint8_t(0));
    ChartsResult r = chart_frame(charts_, mouse, keys, frame_seconds_, game, game.hospital);
    for (uint8_t sfx : r.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    // The research screen's bubbling loops on the crowd source until the
    // screen closes (rschscrn.cpp:39/50).
    if (r.research_sound)
        sounds_.play_sound(kSourceCrowd, kSfxResearch);
    if (r.research_stop)
        sounds_.stop_sample(kSourceCrowd, kSfxResearch);
    if (r.overview_hospital >= 0)
        game.overview_frame(uint8_t(r.overview_hospital));
    if (r.event.type)
        game.event = r.event;
    last_event_ = game.event;
    run_command(game);
    if (r.event.type == EV_OPEN_YES_NO) {
        // EV_OPEN_YES_NO: the chart closed, the requester over the hospital.
        interface_.open_yes_no(data_->text().at(TextSection::YesNo, r.question));
        yes_event_ = r.yes;
        no_event_ = r.no;
    }
    Rect slist_box;
    Point slist_camera;
    if (slist_view(charts_, game.world, slist_box, slist_camera))
        draw_map_view(charts_.surface, game, slist_box, slist_camera);
    draw_fitted(charts_.surface, charts_.palette, charts_tex_, at);
    if (r.town_map) {
        charts_.open = false;
        open_town_map();
    }
}

// Opens the year-end screens when the year moves on (year_end), or the
// win/lose sequence when a level ends (check_game_over /
// check_quarterly_game_over). The original's start_year_end pauses the game
// for the screens.
void App::check_year_end()
{
    if (!map_ || year_end_showing_ || winlose_showing_)
        return;
    Game &game = map_->game;
    bool year_moved = game.date.year != last_year_;
    last_year_ = game.date.year;
    if (year_moved)
        open_year_end();
    else if (game.progress.winlose_result != 0)
        open_winlose();
}

void App::open_year_end()
{
    Game &game = map_->game;
    hosp_scores_ = default_hosp_scores();
    load_highscores(hiscore_path_, hosp_scores_);
    bool showing_fame = insert_hall_details(hosp_scores_, startup_.player_name(), game.progress.score);
    save_highscores(hiscore_path_, hosp_scores_);
    web::flush_storage();
    year_end_.open(hosp_scores_, showing_fame);
    year_end_phase_ = 0;
    year_end_showing_ = true;
    game.paused = true;
}

// One frame of the year-end screens, drawn aspect-fit in the window; the
// game is paused behind them.
void App::year_end_step(int out_w, int out_h)
{
    Game &game = map_->game;
    const Surface &src = year_end_.surface();
    SDL_FRect at = fit_rect(src, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, src);
    YearEndKeys keys;
    keys.escape = std::exchange(escape_, false);
    keys.space = std::exchange(chart_space_, false);
    YearEndResult r;
    if (year_end_phase_ == 0) {
        r = year_end_.hall_frame(mouse, keys, frame_seconds_, startup_.player_name(), game.progress.score);
        if (r.hall_done) {
            year_end_phase_ = 1;
            year_end_.set_phase(1);
        }
    } else {
        YearEndView view;
        view.year = game.date.year;
        view.level = game.level_number();
        view.awards = &game.progress.awards;
        view.trophies = &game.progress.trophies;
        view.chart = &game.progress.chart;
        view.hospital_names = &charts_.hospital_names;
        view.plaque_rat_kills = game.progress.plaque_rat_kills;
        r = year_end_.charts_frame(mouse, keys, frame_seconds_, view);
        if (r.charts_done) {
            year_end_.close();
            year_end_showing_ = false;
            game.paused = false;
        }
    }
    for (uint8_t sfx : r.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    draw_fitted(src, year_end_.palette(), year_end_tex_, at);
}

// One frame of the start-up screens: drawn aspect-fit in the window, with
// the movies' sound on a stream of its own. Starts the level they choose.
void App::startup_step(int out_w, int out_h)
{
    const Surface &src = startup_.surface();
    SDL_FRect at = fit_rect(src, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, src);
    mouse.wheel = std::exchange(startup_wheel_, 0);
    help_mouse_ = mouse;
    help_context();
    StartupResult r = startup_.frame(mouse, startup_keys_, frame_seconds_);
    startup_keys_.clear();
    for (uint8_t sfx : startup_.take_sounds())
        sounds_.play_sound(kSourceGeneral, sfx);
    if (startup_.take_name_confirmed())
        load_player_details();

    // The movie's sound: a stream of the movie's format, emptied when the
    // movie changes.
    SmackerAudioFormat format = startup_.movie_audio_format();
    if (startup_.movie() != movie_playing_ || !format.present()) {
        if (movie_stream_)
            SDL_DestroyAudioStream(movie_stream_);
        movie_stream_ = nullptr;
        movie_playing_ = startup_.movie();
        if (format.present()) {
            SDL_AudioSpec spec{SDL_AUDIO_S16, format.channels, format.rate};
            movie_stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
            if (movie_stream_)
                SDL_ResumeAudioStreamDevice(movie_stream_);
        }
    }
    std::vector<int16_t> pcm;
    startup_.take_movie_audio(pcm);
    if (movie_stream_ && !pcm.empty())
        SDL_PutAudioStreamData(movie_stream_, pcm.data(), int(pcm.size() * sizeof(int16_t)));

    draw_fitted(src, startup_.palette(), startup_tex_, at, startup_.movie() < 0);

    startup_result(r);
}

// What the start-up screens chose: quit, or start a level, or load a save
// (the main menu's Continue, or the load game screen's chosen slot).
void App::startup_result(const StartupResult &r)
{
    if (r.kind == StartupResult::Kind::Running)
        return;
    if (movie_stream_)
        SDL_DestroyAudioStream(movie_stream_);
    movie_stream_ = nullptr;
    if (r.kind == StartupResult::Kind::Quit) {
        running_ = false;
    } else if (r.kind == StartupResult::Kind::StartLevel) {
        fader_.queue_level_start();
        in_startup_ = false;
        balance_id_ = r.balance_id;
        tutorial_ = r.tutorial;
        load_level(r.level);
    } else if (r.kind == StartupResult::Kind::LoadGame) {
        if (perform_load(r.slot)) {
            fader_.queue_level_start();
            in_startup_ = false;
        } else {
            // loadscr state 1 on failure returns to the load game screen
            // (state 4); the main menu's Continue (mainmenu.cpp
            // MMS_CONTINUE_FAILED) returns to the main menu. The screens must
            // stay up: no level was loaded.
            in_startup_ = true;
            startup_.load_failed(r.slot != kContinueSlot);
        }
    }
}

} // namespace th
