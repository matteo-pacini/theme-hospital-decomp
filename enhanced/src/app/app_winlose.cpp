// The win/lose sequence: the movie, the letter and what follows (the next
// level or the main menu).

#include "app/app.h"
#include "app/app_internal.h"
#include "app/web.h"
#include "core/data/fs.h"
#include "core/data/gamedata.h"
#include "core/render/shadows.h"

#include <utility>

namespace th
{

inline namespace app_detail
{

// saferand (src/rand.cpp), for the year-end presentation's letter variants.
uint16_t saferand(uint32_t &seed)
{
    seed = seed * 0x24a1 + 0x24df;
    seed = (seed >> 13) | (seed << 19);
    return uint16_t(seed);
}

} // namespace app_detail

// The losing-line index `anim`'s group carries (the original's nested scans
// of win_lose), drawn from the presentation's safeseed.
int App::lose_line_index(int anim)
{
    const LanguageText &t = data_->text();
    int count = int(t.size(TextSection::WinLose));
    int tc = 0;
    for (int n = anim; n != 0; n--)
        while (tc < count && t.at(TextSection::WinLose, tc++) != ".") {
        }
    int len = 0;
    while (tc + len < count && t.at(TextSection::WinLose, tc + len) != ".")
        len++;
    if (len <= 0)
        return std::min(tc, std::max(count - 1, 0));
    return tc + saferand(sounds_.safeseed) % len;
}

// Opens the win or lose sequence: the movie, then the letter.
void App::open_winlose()
{
    Game &game = map_->game;
    bool win = game.progress.winlose_result == 1;
    int movie = kMovieWinLevel;
    if (win) {
        if (game.level_number() >= 12)
            winlose_pick_ = saferand(sounds_.safeseed) % 4 + 8;
        else {
            winlose_pick_ = saferand(sounds_.safeseed) % 4;
            if (game.progress.win_declined_count % 2 == 1)
                winlose_pick_ += 4;
        }
    } else {
        int anim = saferand(sounds_.safeseed) % 6;
        if (anim == 2)
            anim = 1; // ANIM_LOSE3 plays ANIM_LOSE2
        movie = anim;
        winlose_pick_ = lose_line_index(anim);
    }
    // insert_hall_details: the score enters the hall of fame or of shame.
    hosp_scores_ = default_hosp_scores();
    load_highscores(hiscore_path_, hosp_scores_);
    insert_hall_details(hosp_scores_, startup_.player_name(), game.progress.score);
    save_highscores(hiscore_path_, hosp_scores_);
    web::flush_storage();
    winlose_.open(win, game.level_number());
    winlose_showing_ = true;
    winlose_to_menu_ = !win;
    game.progress.winlose_result = 0;
    open_winlose_movie(movie);
}

// Starts the win/lose movie `index` (-1: none), as play_movie does for the
// start-up screens.
void App::open_winlose_movie(int index)
{
    winlose_movie_index_ = -1;
    if (movie_stream_) {
        SDL_DestroyAudioStream(movie_stream_);
        movie_stream_ = nullptr;
    }
    movie_playing_ = -1;
    if (index < 0)
        return;
    const MovieInfo &info = movie_info(index);
    auto path = resolve_ci(data_->dir(), info.file);
    std::string error;
    if (!path)
        error = "not found in " + data_->dir().string();
    else if (!winlose_movie_.open(*path, &error))
        error = error.empty() ? "cannot be opened" : error;
    else if (!winlose_movie_.next_frame())
        error = "the first frame cannot be decoded";
    if (!error.empty()) {
        std::fprintf(stderr, "win/lose movie %.*s: %s\n", int(info.file.size()), info.file.data(), error.c_str());
        return;
    }
    winlose_movie_index_ = index;
    movie_playing_ = index;
    winlose_movie_flags_ = info.flags;
    winlose_movie_time_ = 0;
    winlose_movie_surface_.resize(kUiWidth, kUiHeight);
    winlose_movie_surface_.fill(0);
    winlose_movie_frame_ = 0;
    show_winlose_movie_frame();
    winlose_movie_audio_.clear();
    SmackerAudioFormat format = winlose_movie_.audio_format(0);
    if (!(winlose_movie_flags_ & kSmkNoSound) && format.present()) {
        SDL_AudioSpec spec{SDL_AUDIO_S16, format.channels, format.rate};
        movie_stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
        if (movie_stream_)
            SDL_ResumeAudioStreamDevice(movie_stream_);
        winlose_movie_audio_.insert(winlose_movie_audio_.end(), winlose_movie_.audio(0).begin(),
                                    winlose_movie_.audio(0).end());
    }
}

void App::show_winlose_movie_frame()
{
    copy_frame_to_screen(winlose_movie_, winlose_movie_surface_, winlose_movie_flags_);
    winlose_movie_palette_ = winlose_movie_.palette();
    winlose_movie_frame_++;
    if (winlose_movie_index_ < kMovieWinLevel &&
        unsigned(winlose_movie_frame_) >= movie_info(winlose_movie_index_).text_frame) {
        WinLoseView view;
        view.pick = winlose_pick_;
        winlose_.draw_lose_line(winlose_movie_surface_, view);
    }
}

void App::advance_winlose_movie()
{
    if (winlose_movie_index_ < 0)
        return;
    bool ended = false;
    winlose_movie_time_ += frame_seconds_ * 1e6;
    while (!ended && winlose_movie_time_ >= double(winlose_movie_.frame_duration_us())) {
        winlose_movie_time_ -= double(winlose_movie_.frame_duration_us());
        if (!winlose_movie_.next_frame()) {
            ended = true;
            break;
        }
        show_winlose_movie_frame();
        if (!(winlose_movie_flags_ & kSmkNoSound))
            winlose_movie_audio_.insert(winlose_movie_audio_.end(), winlose_movie_.audio(0).begin(),
                                        winlose_movie_.audio(0).end());
    }
    if (ended) {
        winlose_.set_backdrop(winlose_movie_surface_, winlose_movie_palette_);
        // play_winlose clears lbInkey and the buttons after the movie: what
        // was pressed during it does not reach the letter.
        any_key_ = escape_ = chart_space_ = false;
        buttons_.left_pressed = buttons_.right_pressed = false;
        buttons_.left_released = buttons_.right_released = false;
        winlose_movie_index_ = -1;
        movie_playing_ = -1;
        winlose_movie_audio_.clear();
        if (movie_stream_) {
            SDL_DestroyAudioStream(movie_stream_);
            movie_stream_ = nullptr;
        }
    }
}

// The game ends: the game is unloaded for the WIN_GAME movie, the credits and
// the main menu. A run started with --level has not loaded the start-up
// screens yet; it does so here and leaves --level behind. False if they
// cannot load.
bool App::end_game()
{
    if (level_from_cli_) {
        std::string error;
        if (!startup_.load(data_dir_, data_->text(), data_->language(), &error)) {
            std::fprintf(stderr, "%s\n", error.c_str());
            return false;
        }
        level_from_cli_ = false;
    }
    map_.reset();
    in_startup_ = true;
    startup_.start_credits();
    return true;
}

// Back to the main menu (MMS_UNLOAD_GAME): the score (init_player) and the
// rivals' monthly banks are kept for the next level. The Continue slot is not
// written here; only quit_game 1 does that.
void App::return_to_main_menu()
{
    remember_level_end();
    map_.reset();
    in_startup_ = true;
    startup_.return_to_main_menu();
}

// The level an accepted letter goes to: the letter's EV_CANCEL_BUILD is the
// new level's first command (src/winlose.cpp), then comes its briefing page.
void App::start_next_level(int next)
{
    fader_.queue_level_start();
    load_level(next);
    map_->game.event = {EV_CANCEL_BUILD, 0, 0, 0};
    run_command(map_->game);
    help_.set_page(-1, -1, level_briefing(data_->text(), next));
}

// A win letter declined: the game's files reload behind the loading screen
// (src/winlose.cpp, state 5), and the game runs on.
void App::decline_win(bool win)
{
    if (!win)
        return;
    fader_.queue_level_start();
    map_->game.event = {EV_DECLINE_WIN, 0, 0, 0};
    run_command(map_->game);
}

// One frame of the win or lose sequence: its movie, then its letter. Accepting
// a win starts the next level, or ends the game after the last; dismissing the losing letter returns to the
// main menu, as the original does.
void App::winlose_step(int out_w, int out_h)
{
    Game &game = map_->game;
    if (winlose_movie_index_ >= 0) {
        advance_winlose_movie();
        if (movie_stream_ && !winlose_movie_audio_.empty()) {
            SDL_PutAudioStreamData(movie_stream_, winlose_movie_audio_.data(),
                                   int(winlose_movie_audio_.size() * sizeof(int16_t)));
            winlose_movie_audio_.clear();
        }
        SDL_FRect mat = fit_rect(winlose_movie_surface_, out_w, out_h);
        draw_fitted(winlose_movie_surface_, winlose_movie_palette_, winlose_tex_, mat, false);
        return;
    }
    const Surface &src = winlose_.surface();
    SDL_FRect at = fit_rect(src, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, src);
    WinLoseKeys keys;
    keys.escape = std::exchange(escape_, false);
    keys.space = std::exchange(chart_space_, false);
    keys.any = std::exchange(any_key_, false);
    WinLoseView view;
    view.win = game.progress.level_won != 0;
    view.level = game.level_number();
    view.score = game.progress.score;
    view.next_level_offer = game.progress.next_level_offer;
    view.pick = winlose_pick_;
    view.lose_font = 0x13;
    view.hospital_name = startup_.player_name();
    WinLoseResult r = winlose_.frame(mouse, keys, frame_seconds_, view);
    for (uint8_t sfx : r.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    if (r.closed) {
        winlose_.close();
        winlose_showing_ = false;
        if (r.next_level > 0) {
            // winlose.cpp: myhosp->score = othervar.next_level_offer_s.
            game.progress.score = game.progress.next_level_offer;
            int next = level_after_win(game, r.next_level);
            start_next_level(next);
        } else if (r.game_won && end_game()) {
            // The last level won: the movie, the credits, the main menu.
            // quit_game stays 0, so the Continue slot is not written.
        } else if (winlose_to_menu_ && !level_from_cli_) {
            // DM_MAIN_MENU: back to the main menu (the game is unloaded).
            return_to_main_menu();
        } else {
            decline_win(view.win);
        }
        winlose_to_menu_ = false;
    }
    draw_fitted(src, winlose_.palette(), winlose_tex_, at);
}

} // namespace th
