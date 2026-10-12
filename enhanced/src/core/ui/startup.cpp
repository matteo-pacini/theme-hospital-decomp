#include "core/ui/startup.h"

#include "core/ui/startup_internal.h"

#include "core/data/balance.h"
#include "core/data/fs.h"
#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/text.h"
#include "core/ui/helpmsg.h"
#include "core/world/savegame.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <format>
#include <initializer_list>

namespace th
{

namespace
{

constexpr double kGameFrame = 0.04; // limit_framerate: 40 timer ticks
constexpr int kAttractFrames = 0x3e8;

// play_sound's sfx numbers (include/enums.h).
constexpr uint8_t kSfxDice = 0xd2;     // SFX_DICEYFIN
constexpr uint8_t kSfxDiceLate = 0xdc; // SFX_DICE122M, for levels 12 and up

int sqr(int v)
{
    return v * v;
}

// The candidates of `list` whose `key` is the smallest, in `list`'s order.
template <typename Key> std::vector<int> smallest(const std::vector<int> &list, Key &&key)
{
    int best = 999999;
    for (int i : list)
        best = std::min(best, key(i));
    std::vector<int> out;
    for (int i : list)
        if (key(i) == best)
            out.push_back(i);
    return out;
}

// LbPaletteFindColour (src/lib/bf1/gpalette.cpp): the nearest colour of a
// 6-bit palette by squared distance, ties broken by the sum of the
// differences, then by the darkest (green and red weighted double).
uint8_t find_colour(const std::array<uint8_t, 768> &pal, int r, int g, int b)
{
    auto at = [&](int i, int k) { return int(pal[size_t(i) * 3 + size_t(k)]); };
    auto dist = [&](int i) { return sqr(r - at(i, 0)) + sqr(g - at(i, 1)) + sqr(b - at(i, 2)); };
    auto manhattan = [&](int i) { return std::abs(r - at(i, 0)) + std::abs(g - at(i, 1)) + std::abs(b - at(i, 2)); };
    auto darkness = [&](int i) { return at(i, 1) * 2 * at(i, 1) + at(i, 0) * 2 * at(i, 0) + at(i, 2) * at(i, 2); };
    std::vector<int> list(256);
    for (int i = 0; i < 256; i++)
        list[size_t(i)] = i;
    list = smallest(list, dist);
    // An exact match is the first one, whatever the ties.
    if (list.size() == 1 || dist(list[0]) == 0)
        return uint8_t(list[0]);
    list = smallest(list, manhattan);
    if (list.size() == 1)
        return uint8_t(list[0]);
    return uint8_t(smallest(list, darkness)[0]);
}

bool load_sheet(const std::filesystem::path &dir, std::string_view name, SpriteSheet &sheet, std::string *error)
{
    auto tab = load_data_file(dir, "qdata/" + std::string(name) + ".tab", error);
    auto dat = tab ? load_data_file(dir, "qdata/" + std::string(name) + ".dat", error) : std::nullopt;
    if (!dat)
        return false;
    if (!sheet.load(std::move(*tab), std::move(*dat), SpriteFormat::Req)) {
        if (error)
            *error = std::string(name) + ": not a sprite table";
        return false;
    }
    return true;
}

// setup_balance_files (src/name.cpp): the balance files levels/<id>00.sam,
// but net00.sam. The game takes them in the order the directory lists
// them; here they are sorted by name.
std::vector<std::string> balance_files(const std::filesystem::path &dir)
{
    std::vector<std::string> ids;
    auto levels = resolve_ci(dir, "levels");
    std::error_code ec;
    if (!levels)
        return ids;
    for (const auto &entry : std::filesystem::directory_iterator(*levels, ec)) {
        std::string name = entry.path().filename().string();
        std::string lower = name;
        for (char &c : lower)
            c = char(std::tolower(uint8_t(c)));
        if (lower.size() < 7 || lower.substr(lower.size() - 6) != "00.sam" || lower.rfind("net", 0) == 0)
            continue;
        ids.push_back(name.substr(0, name.size() - 6));
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

bool ieq(std::string_view a, std::string_view b)
{
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) { return std::tolower(x) == std::tolower(y); });
}

} // namespace

inline namespace startup_detail
{

bool over(int mx, int my, int x, int y, int w, int h)
{
    return mx >= x && mx < x + w && my >= y && my < y + h;
}

} // namespace startup_detail

bool Startup::load_picture(const std::filesystem::path &dir, std::string_view name, int width, int height, Picture &out,
                           std::string *error)
{
    Surface &pixels = out.pixels;
    std::array<uint8_t, 768> &vga = out.vga;
    auto dat = load_data_file(dir, "qdata/" + std::string(name) + ".dat", error);
    if (!dat)
        return false;
    auto pal = load_data_file(dir, "qdata/" + std::string(name) + ".pal", error);
    if (!pal)
        return false;
    if (dat->size() < size_t(width) * size_t(height) || pal->size() < vga.size()) {
        if (error)
            *error = std::string(name) + ": not a full-screen picture";
        return false;
    }
    pixels.resize(width, height);
    for (int y = 0; y < height; y++)
        std::memcpy(pixels.row(y), dat->data() + size_t(y) * size_t(width), size_t(width));
    std::memcpy(vga.data(), pal->data(), vga.size());
    palette_from_vga(vga, out.palette);
    return true;
}

bool Startup::load(const std::filesystem::path &dir, const LanguageText &text, int language, std::string *error)
{
    dir_ = dir;
    text_ = &text;
    name_ = std::string(text.misc(1));
    language_ = language;
    if (!load_picture(dir, "title01v", kUiWidth, kUiHeight, title_, error) ||
        !load_picture(dir, "main01m", 320, 200, main_menu_, error) ||
        !load_picture(dir, "pref01v", kUiWidth, kUiHeight, prefs_, error) ||
        !load_sheet(dir, "main02m", main_sprites_, error) || !load_sheet(dir, "pref02v", prefs_sprites_, error))
        return false;
    // LbGhostTableGenerate(ppalette, 0x1e, "qdata/ghost1.dat") on the
    // preferences screen: the shipped file is that table.
    auto ghost = load_data_file(dir, "qdata/ghost1.dat", error);
    if (!ghost)
        return false;
    if (ghost->size() < 0x10000) {
        if (error)
            *error = "qdata/ghost1.dat: too short";
        return false;
    }
    prefs_ghost_ = std::move(*ghost);
    for (size_t i = 0; i < fonts_.size(); i++) {
        if (!load_font(dir, kStartupFonts[i], fonts_[i], error))
            return false;
    }
    balance_files_ = balance_files(dir);
    state_ = State::Intro;
    return true;
}

SmackerAudioFormat Startup::movie_audio_format() const
{
    if (movie_index_ < 0 || (movie_flags_ & kSmkNoSound))
        return {};
    return video_.audio_format(0);
}

void Startup::take_movie_audio(std::vector<int16_t> &out)
{
    out.insert(out.end(), movie_audio_.begin(), movie_audio_.end());
    movie_audio_.clear();
}

std::vector<uint8_t> Startup::take_sounds()
{
    std::vector<uint8_t> out;
    out.swap(sounds_);
    return out;
}

StartupResult Startup::frame(const UiMouse &mouse, std::span<const StartupKey> keys, double seconds)
{
    UiMouse &m = pending_mouse_;
    bool lp = m.left_pressed, rp = m.right_pressed, lr = m.left_released, rr = m.right_released;
    const int wheel = m.wheel;
    m = mouse;
    m.wheel += wheel;
    m.left_pressed |= lp;
    m.right_pressed |= rp;
    m.left_released |= lr;
    m.right_released |= rr;
    pending_keys_.insert(pending_keys_.end(), keys.begin(), keys.end());

    if (movie_index_ >= 0) {
        bool skip = false;
        if (!(movie_flags_ & kSmkNoSkip)) {
            skip = m.left_pressed;
            for (const StartupKey &k : pending_keys_)
                skip |= k.code == StartupKey::Escape || k.code == StartupKey::Return || k.code == StartupKey::Space;
        }
        // hosp_play_smk clears the buttons after a movie; the keys that
        // skipped it are spent too.
        m.left_pressed = m.right_pressed = m.left_released = m.right_released = false;
        m.wheel = 0;
        pending_keys_.clear();
        advance_movie(seconds, skip);
        if (movie_index_ >= 0)
            return result_;
        frame_time_ = kGameFrame;
    } else {
        frame_time_ += seconds;
    }
    for (int n = 0; frame_time_ >= kGameFrame && n < 5 && movie_index_ < 0; n++) {
        frame_time_ -= kGameFrame;
        tick(m, pending_keys_);
        m.left_pressed = m.right_pressed = m.left_released = m.right_released = false;
        m.wheel = 0;
        pending_keys_.clear();
    }
    frame_time_ = std::min(frame_time_, kGameFrame);
    return result_;
}

void Startup::play_movie(int index, State next)
{
    after_movie_ = next;
    state_ = next;
    const MovieInfo &info = movie_info(index);
    auto path = resolve_ci(dir_, info.file);
    if (!path || !video_.open(*path) || !video_.next_frame())
        return;
    movie_index_ = index;
    movie_flags_ = info.flags;
    movie_time_ = 0;
    screen_ = StartupScreen::Movie;
    // play_intro and draw_main_menu play these in the 320 x 200 mode.
    bool low =
        index == kMovieIntroSm2 || index == kMovieIntroSm4 || index == kMovieMainMenu1 || index == kMovieMainMenu2;
    surface_.resize(low ? 320 : kUiWidth, low ? 200 : kUiHeight);
    surface_.fill(0);
    copy_frame_to_screen(video_, surface_, movie_flags_);
    palette_ = video_.palette();
    if (!(movie_flags_ & kSmkNoSound))
        movie_audio_.insert(movie_audio_.end(), video_.audio(0).begin(), video_.audio(0).end());
}

void Startup::advance_movie(double seconds, bool skip)
{
    bool ended = skip;
    movie_time_ += seconds * 1e6;
    while (!ended && movie_time_ >= double(video_.frame_duration_us())) {
        movie_time_ -= double(video_.frame_duration_us());
        if (!video_.next_frame()) {
            ended = true;
            break;
        }
        copy_frame_to_screen(video_, surface_, movie_flags_);
        palette_ = video_.palette();
        if (!(movie_flags_ & kSmkNoSound))
            movie_audio_.insert(movie_audio_.end(), video_.audio(0).begin(), video_.audio(0).end());
    }
    if (ended) {
        movie_index_ = -1;
        movie_audio_.clear();
        state_ = after_movie_;
    }
}

// calculate_grey: the boxes' colours in the screen's palette.
void Startup::calculate_grey(const Picture &picture)
{
    colours_.blue_grey = find_colour(picture.vga, 3, 0x2f, 0x3b);
    colours_.white = find_colour(picture.vga, 0x3f, 0x3f, 0x3f);
    colours_.black = find_colour(picture.vga, 0, 0, 0);
}

void Startup::show(const Picture &picture)
{
    surface_.resize(picture.pixels.width(), picture.pixels.height());
    for (int y = 0; y < surface_.height(); y++)
        std::memcpy(surface_.row(y), picture.pixels.data() + size_t(y) * size_t(surface_.width()),
                    size_t(surface_.width()));
    palette_ = picture.palette;
}

void Startup::tick(const UiMouse &mouse, std::span<const StartupKey> keys)
{
    switch (state_) {
    case State::Intro:
        // MMS_INTRO: play_intro picks INTRO.SM4 for a CD drive faster than
        // 4x, which any computer now is.
        play_movie(kMovieIntroSm4, State::TitleEnter);
        break;
    case State::TitleEnter:
        enter_title();
        break;
    case State::Title:
        attract(mouse, keys);
        break;
    case State::AboutEnter:
        enter_about();
        break;
    case State::AboutDelay:
        about_wait(mouse, keys);
        break;
    case State::AboutZoom:
    case State::AboutEntry:
        about_screen(mouse, keys);
        break;
    case State::MenuEnter:
        enter_menu();
        break;
    default:
        // The main menu on, after the about screen.
        tick_menu(mouse, keys);
        break;
    }
}

// The start-up screens from the main menu on: the menu, the load game
// screen, the preferences, the level's area movie and its briefing, and the
// quit. tick's, apart for its length.
void Startup::tick_menu(const UiMouse &mouse, std::span<const StartupKey> keys)
{
    switch (state_) {
    case State::Menu:
        screen_ = StartupScreen::MainMenu;
        main_menu(mouse, keys);
        break;
    case State::LoadEnter:
    case State::LoadDelay:
    case State::LoadZoom:
    case State::LoadEntry:
        load_tick(mouse, keys);
        break;
    case State::PrefsEnter:
        enter_preferences();
        break;
    case State::PrefsDelay:
        zoom_after_delay(prefs_, 1, State::PrefsZoom);
        break;
    case State::PrefsZoom:
    case State::PrefsEntry:
        preferences(mouse, keys);
        break;
    case State::AreaEnter:
        enter_area();
        break;
    case State::Briefing:
        screen_ = StartupScreen::Briefing;
        briefing(mouse, keys);
        break;
    case State::AreaStart:
        start_level();
        break;
    case State::CreditsEnter:
        enter_credits();
        break;
    case State::Credits:
        screen_ = StartupScreen::Credits;
        credits(mouse);
        break;
    case State::Quit:
        finish_quit();
        break;
    case State::Done:
        break;
    default:
        break;
    }
}

void Startup::enter_title()
{
    show(title_);
    screen_ = StartupScreen::Title;
    timer_ = kAttractFrames;
    state_ = State::Title;
}

void Startup::attract(const UiMouse &mouse, std::span<const StartupKey> keys)
{
    // MMS_ATTRACT: any click or key goes on to the about screen (the
    // original's name screen); after 1000 frames the attract and intro
    // movies take turns.
    if (mouse.left_pressed || mouse.right_pressed || !keys.empty()) {
        state_ = State::AboutEnter;
        return;
    }
    show(title_);
    if (resolve_ci(dir_, movie_info(kMovieAttract).file) && --timer_ == 0) {
        attract_count_++;
        play_movie(attract_count_ % 2 == 1 ? kMovieAttract : kMovieIntroSm4, State::TitleEnter);
    }
}

void Startup::enter_menu()
{
    // MMS_ENTER, MMS_UNLOAD_GAME and MMS_SHOW_MENU: the menu's picture, then
    // its opening movie. A result the caller was given (a started level or a
    // load) is spent; without this the frame would hand it out again.
    result_ = {};
    show(main_menu_);
    screen_ = StartupScreen::MainMenu;
    menu_state_ = kHoverContinue;
    play_movie(kMovieMainMenu1, State::Menu);
}

void Startup::load_failed(bool from_load_screen)
{
    result_ = {};
    if (from_load_screen) {
        screen_ = StartupScreen::LoadGame;
        state_ = State::LoadEntry;
    } else {
        enter_menu();
    }
}

void Startup::enter_area()
{
    // draw_area_select state 0: a new game's level_number is 0, whose movie
    // is area01v.
    sound(level_ >= 12 ? kSfxDiceLate : kSfxDice);
    idle_ = 600;
    pick_any_level_ = false;
    play_movie(kMovieArea + level_, State::Briefing);
}

void Startup::start_credits()
{
    result_ = {};
    play_movie(kMovieWinGame, State::CreditsEnter);
}

// MMS_CREDITS: the picture and font load, then MMS_CREDITS_SCROLL's first
// play_credits call, whose result the original ignores.
void Startup::enter_credits()
{
    std::string error;
    if (!load_picture(dir_, "cred01v", kUiWidth, kUiHeight, credits_, &error) ||
        !load_font(dir_, 0x2a, credit_font_, &error)) {
        enter_menu();
        return;
    }
    result_ = {};
    show(credits_);
    screen_ = StartupScreen::Credits;
    credit_scroll_ = 0;
    credit_line_ = 0;
    credit_pressed_ = false;
    play_credits(UiMouse{});
    state_ = State::Credits;
}

// MMS_CREDITS_SCROLL's loop body; MMS_CREDITS_DONE returns to the main menu.
void Startup::credits(const UiMouse &mouse)
{
    show(credits_);
    credit_pressed_ |= mouse.left_pressed || mouse.right_pressed;
    if (play_credits(mouse))
        enter_menu();
}

bool Startup::play_credits(const UiMouse &mouse)
{
    // The line spacing: the height of glyph 'A' (lbFontPtr[34]), up to a
    // multiple of 3, the scroll's step.
    int height = credit_font_.glyph('A').height;
    if (height % 3)
        height = (height / 3 + 1) * 3;
    const TextSettings settings{
        {0, 0x72, surface_.width()}, {0, 0, surface_.width(), surface_.height()}, kTextCentre, 0, nullptr};
    const size_t count = text_->size(TextSection::Credit);
    bool done = false;
    int y = -credit_scroll_;
    for (size_t i = 0; y < surface_.height(); y += height, i++) {
        size_t index = i + size_t(credit_line_);
        std::string line(index < count ? text_->at(TextSection::Credit, int(index)) : ".");
        char marker = line.empty() ? '\0' : line[0];
        // '!' draws nothing, '.' ends the credits, ':' and ',' draw the line
        // with a space for the marker.
        if (marker == '!')
            continue;
        if (marker == '.') {
            done = true;
            continue;
        }
        if (marker == ':' || marker == ',')
            line[0] = ' ';
        draw_text(surface_, credit_font_, 0, y, line, settings);
    }
    if (credit_pressed_ && (mouse.left_released || mouse.right_released))
        done = true;
    credit_scroll_ += 3;
    if (credit_scroll_ % height == 0) {
        credit_scroll_ = 0;
        credit_line_++;
    }
    return done;
}

void Startup::finish_quit()
{
    screen_ = StartupScreen::Done;
    result_ = {};
    result_.kind = StartupResult::Kind::Quit;
    state_ = State::Done;
}

void Startup::enter_preferences()
{
    // draw_nickname state 3.
    show(prefs_);
    screen_ = StartupScreen::Preferences;
    calculate_grey(prefs_);
    balance_ = 0;
    for (size_t i = 0; i < balance_files_.size(); i++) {
        if (ieq(balance_files_[i], kDefaultBalanceId))
            balance_ = i;
    }
    balance_name_ = std::string(text_->at(TextSection::Nickname, 0x15));
    if (!balance_files_.empty()) {
        if (auto d = balance_description(dir_, balance_files_[balance_], language_))
            balance_name_ = *d;
    }
    timer_ = kDelay800ms;
    state_ = State::PrefsDelay;
}

void Startup::zoom_after_delay(const Picture &picture, int mask, State next)
{
    show(picture);
    if (--timer_ <= 0) {
        init_zoom_boxes(mask);
        state_ = next;
    }
}

} // namespace th
