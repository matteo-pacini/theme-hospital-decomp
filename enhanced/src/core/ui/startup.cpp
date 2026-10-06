#include "core/ui/startup.h"

#include "core/data/balance.h"
#include "core/data/fs.h"
#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/text.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <initializer_list>

namespace th
{

enum class Startup::State {
    Intro,
    TitleEnter,
    Title,
    NameEnter,
    NameDelay,
    NameZoom,
    NameEntry,
    NameDone,
    MenuEnter,
    Menu,
    PrefsEnter,
    PrefsDelay,
    PrefsZoom,
    PrefsEntry,
    AreaEnter,
    Briefing,
    Quit,
    Done,
};

namespace
{

constexpr double kGameFrame = 0.04; // limit_framerate: 40 timer ticks
constexpr int kDelay800ms = 20;     // draw_nickname's delay(800), in frames
constexpr int kAttractFrames = 0x3e8;

// play_sound's sfx numbers (include/enums.h).
constexpr uint8_t kSfxWrong2 = 0x08;
constexpr uint8_t kSfxEnterName = 0x0c;  // SFX_ENTANAME
constexpr uint8_t kSfxDeleteName = 0x10; // SFX_DELANAME
constexpr uint8_t kSfxYes = 0x41;        // SFX_YESX
constexpr uint8_t kSfxNo = 0x42;         // SFX_NO4
constexpr uint8_t kSfxSelect = 0x43;     // SFX_SELECTX
constexpr uint8_t kSfxHighlight = 0x44;  // SFX_HLIGHT5
constexpr uint8_t kSfxDice = 0xd2;       // SFX_DICEYFIN
constexpr uint8_t kSfxDiceLate = 0xdc;   // SFX_DICE122M, for levels 12 and up

// mainmenu.state (MMS_* in include/enums.h); 0x11 to 0x16 show the menu's
// diamond turning from one entry to another.
enum : int {
    kHoverContinue = 3,
    kHoverLoad = 4,
    kHoverNewGame = 5,
    kHoverNetwork = 6,
    kHoverQuit = 7,
    kNewGame = 8,
    kContinue = 9,
    kNetwork = 0xa,
    kQuit = 0xc,
    kLoadGame = 0xf,
};

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

// A glass box of hiresglass (src/scoords.cpp): a rectangle with corners
// (x, y) and (x1, y1), how it is drawn (type: -1 or 0 a background panel,
// 0 see-through; 1 or 2 a foreground one; button: a button face), the
// length of its zoom in frames, its delays and where it grows from (0 its
// centre, 1 its top centre, 2 the screen's centre).
struct GlassBox {
    int x, y, x1, y1;
    int type;
    bool button;
    int steps, delay, origin;
};

// Glass screen 10 (display bit 1, the preferences) and 11 (bit 2, the name
// box): hiresglass[310..321] and [341], less the screen-resolution row
// (hiresglass[314], [317], [318]), which has no use here.
constexpr std::array<GlassBox, 9> kPrefsGlass = {{
    {124, 30, 515, 449, 0, false, 7, 0, 0},
    {149, 55, 490, 424, 1, false, 7, 0, 0},
    {161, 67, 479, 176, 2, false, 7, 0, 0},
    {161, 303, 334, 336, 2, false, 7, 0, 0},
    {339, 303, 406, 336, 2, true, 7, 0, 0},
    {411, 303, 478, 336, 2, true, 7, 0, 0},
    {339, 379, 406, 412, 2, true, 7, 0, 0},
    {441, 72, 474, 105, 3, true, 7, 0, 0},
    {441, 138, 474, 171, 3, true, 7, 0, 0},
}};
constexpr std::array<GlassBox, 1> kNameGlass = {{{90, 422, 549, 466, -1, false, 7, 0, 1}}};

// The preferences' buttons, screenbuttons 0xa0 to 0xa7 (src/scoords.cpp),
// corners (x, y) and (x1, y1): tutorial on and off, accept, cancel (drawn
// with PREF02V's sprites), previous and next difficulty. The game's low and
// high resolution buttons (0xa6, 0xa7) are left out. process_nickname_button(i)
// acts on button i.
struct ScreenButton {
    int x, y, x1, y1;
    bool sprite;
};
constexpr std::array<ScreenButton, 6> kPrefsButtons = {{
    {339, 303, 406, 336, false},
    {411, 303, 478, 336, false},
    {339, 379, 406, 412, false},
    {449, 383, 474, 408, true},
    {441, 72, 474, 105, false},
    {441, 138, 474, 171, false},
}};
enum : int {
    kTutorialOn,
    kTutorialOff,
    kAccept,
    kCancel,
    kPrevBalance,
    kNextBalance,
};

// ScreenBoxes[122]: where the difficulty's title and name are centred.
constexpr Rect kBalanceBox = {179, 67, 245, 108};

// calculate_grey's bevel greys (Gray[2..7]).
constexpr std::array<uint8_t, 8> kGray = {0, 0xff, 0xf5, 0xf5, 0xf5, 0xf8, 0xf8, 0xf6};

// Font cache slots in Startup::fonts_.
enum : int { kFontGlass, kFontGlassHi, kFontName, kFontSlotCount };
constexpr std::array<int, kFontSlotCount> kStartupFonts = {0x21, 0x22, 0x23};

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

bool over(int mx, int my, int x, int y, int w, int h)
{
    return mx >= x && mx < x + w && my >= y && my < y + h;
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
    m = mouse;
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
    case State::NameEnter:
        enter_name();
        break;
    case State::NameDelay:
        zoom_after_delay(title_, 2, State::NameZoom);
        break;
    case State::NameZoom:
    case State::NameEntry:
        name_screen(keys);
        break;
    case State::NameDone:
        // draw_nickname state 1.
        if (name_.empty())
            name_ = std::string(text_->misc(1));
        state_ = State::MenuEnter;
        break;
    case State::MenuEnter:
        enter_menu();
        break;
    case State::Menu:
        screen_ = StartupScreen::MainMenu;
        main_menu(mouse, keys);
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
    case State::Quit:
        finish_quit();
        break;
    case State::Done:
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
    // MMS_ATTRACT: any click or key goes on to the name screen; after 1000
    // frames the attract and intro movies take turns.
    if (mouse.left_pressed || mouse.right_pressed || !keys.empty()) {
        state_ = State::NameEnter;
        return;
    }
    show(title_);
    if (resolve_ci(dir_, movie_info(kMovieAttract).file) && --timer_ == 0) {
        attract_count_++;
        play_movie(attract_count_ % 2 == 1 ? kMovieAttract : kMovieIntroSm4, State::TitleEnter);
    }
}

void Startup::enter_name()
{
    // draw_nickname state 0.
    name_.clear();
    show(title_);
    calculate_grey(title_);
    screen_ = StartupScreen::Name;
    timer_ = kDelay800ms;
    state_ = State::NameDelay;
}

void Startup::enter_menu()
{
    // MMS_ENTER, MMS_UNLOAD_GAME and MMS_SHOW_MENU: the menu's picture, then
    // its opening movie.
    show(main_menu_);
    screen_ = StartupScreen::MainMenu;
    menu_state_ = kHoverContinue;
    play_movie(kMovieMainMenu1, State::Menu);
}

void Startup::enter_area()
{
    // draw_area_select state 0: a new game's level_number is 0, whose movie
    // is area01v.
    sound(level_ >= 12 ? kSfxDiceLate : kSfxDice);
    idle_ = 600;
    play_movie(kMovieArea + level_, State::Briefing);
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

void Startup::name_screen(std::span<const StartupKey> keys)
{
    show(title_);
    if (state_ == State::NameZoom) {
        // draw_nickname state 7.
        if (current_display_ == desired_display_)
            state_ = State::NameEntry;
        draw_zoom_boxes();
        return;
    }
    // State 2, display_nick_name_screen.
    draw_zoom_boxes();
    const Font &font = fonts_[kFontName];
    std::string_view prompt = text_->at(TextSection::Nickname, 0);
    std::string line = std::string(prompt) + " : " + name_ + "*";
    int ch = font.glyph('A').height;
    int y = ((0x2c - ch) >> 1) + 0x1a6;
    int x = ((0x1cb - text_width(font, prompt) - 0x70) >> 1) + 0x5a;
    draw_text(surface_, font, x, y, line, {{0, 0, surface_.width()}, {0, 0, surface_.width(), surface_.height()}});

    // process_keys_nickname state 2.
    for (const StartupKey &k : keys) {
        switch (k.code) {
        case StartupKey::Escape:
            sound(kSfxNo);
            state_ = State::Quit;
            return;
        case StartupKey::Return:
            sound(kSfxYes);
            state_ = State::NameDone;
            return;
        case StartupKey::Backspace:
            if (!name_.empty()) {
                sound(kSfxDeleteName);
                name_.pop_back();
            }
            break;
        case StartupKey::Letter:
            if (name_.size() < 8 && std::isalpha(uint8_t(k.letter))) {
                name_ += char(std::toupper(uint8_t(k.letter)));
                sound(kSfxEnterName);
            }
            break;
        default:
            break;
        }
    }
}

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
    case kContinue: // save_game_valid(99) fails: there are no saves yet
    case kLoadGame: // not ported
    case kNetwork:  // not ported
        menu_state_ = kHoverContinue;
        sound(kSfxWrong2);
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

void Startup::preferences(const UiMouse &mouse, std::span<const StartupKey> keys)
{
    bool several = balance_files_.size() > 1;
    show(prefs_);
    if (state_ == State::PrefsZoom) {
        // draw_nickname state 6.
        if (current_display_ == desired_display_)
            state_ = State::PrefsEntry;
        draw_zoom_boxes();
    } else {
        // State 5.
        draw_new_prefs_screen(mouse, several);
        // process_keys_nickname state 5.
        for (const StartupKey &k : keys) {
            if (k.code == StartupKey::Escape) {
                sound(kSfxNo);
                state_ = State::MenuEnter;
                return;
            }
        }
    }

    // process_mouse_nickname.
    if (current_display_ & desired_display_ & 1)
        draw_sprite(surface_, kPrefsButtons[kCancel].x, kPrefsButtons[kCancel].y, prefs_sprites_.at(2));
    if (state_ != State::PrefsEntry || !(current_display_ & 1))
        return;
    State next = state_;
    for (int i = 0; i < int(kPrefsButtons.size()); i++) {
        if (!several && (i == kPrevBalance || i == kNextBalance))
            continue;
        prefs_button(mouse, i, next);
    }
    state_ = next;
}

void Startup::draw_new_prefs_screen(const UiMouse &mouse, bool several)
{
    draw_zoom_boxes();
    if (several) {
        draw_glass_text(mouse, kPrevBalance, "{", false);
        draw_glass_text(mouse, kNextBalance, "|", false);
    }
    auto nick = [&](int i) { return text_->at(TextSection::Nickname, i); };
    draw_glass_text(mouse, kAccept, nick(0x12), false);
    draw_glass_text(mouse, kTutorialOn, nick(0x10), tutorial_);
    draw_glass_text(mouse, kTutorialOff, nick(0x11), !tutorial_);
    const Font &font = fonts_[kFontGlass];
    TextWindow window{0, 0, surface_.width()};
    Rect clip{0, 0, surface_.width(), surface_.height()};
    int ch = font.glyph('A').height;
    int y = 0x12f;
    y += (0x150 - y - ch) >> 1;
    draw_text(surface_, font, 0xaf, y, nick(6), {window, clip});
    int bh = font.glyph('B').height;
    std::string_view title = text_->misc(0x1e);
    int ty = kBalanceBox.y + (kBalanceBox.h - bh * 2) / 2;
    draw_text(surface_, font, kBalanceBox.x + (kBalanceBox.w - text_width(font, title)) / 2, ty, title, {window, clip});
    draw_text(surface_, font, kBalanceBox.x + (kBalanceBox.w - text_width(font, balance_name_)) / 2, ty + bh,
              balance_name_, {window, clip});
}

void Startup::prefs_button(const UiMouse &mouse, int i, State &next)
{
    const ScreenButton &b = kPrefsButtons[size_t(i)];
    int w = b.x1 - b.x + 1, h = b.y1 - b.y + 1;
    auto press = [&] {
        if (b.sprite)
            draw_sprite(surface_, b.x, b.y, prefs_sprites_.at(1));
        else
            draw_button_edges(b.x, b.y, b.x1 - b.x, b.y1 - b.y, true);
    };
    if (mouse.left_released && over(mouse.x, mouse.y, b.x, b.y, w, h)) {
        press();
        process_nickname_button(i, next);
        return;
    }
    if (mouse.left_held && over(mouse.x, mouse.y, b.x, b.y, w, h))
        press();
    bool chosen = (i == kTutorialOn && tutorial_) || (i == kTutorialOff && !tutorial_);
    if (chosen)
        draw_button_edges(b.x, b.y, b.x1 - b.x, b.y1 - b.y, true);
}

void Startup::process_nickname_button(int i, State &next)
{
    switch (i) {
    case kPrevBalance:
    case kNextBalance:
        sound(kSfxSelect);
        balance_ = (balance_ + (i == kNextBalance ? 1 : balance_files_.size() - 1)) % balance_files_.size();
        balance_name_ = std::string(text_->at(TextSection::Nickname, 0x15));
        if (auto d = balance_description(dir_, balance_files_[balance_], language_))
            balance_name_ = *d;
        break;
    case kTutorialOn:
    case kTutorialOff:
        tutorial_ = i == kTutorialOn;
        sound(kSfxSelect);
        break;
    case kAccept:
        sound(kSfxYes);
        next = State::AreaEnter;
        break;
    case kCancel:
        sound(kSfxNo);
        next = State::MenuEnter;
        break;
    }
}

void Startup::briefing(const UiMouse &mouse, std::span<const StartupKey> keys)
{
    // process_keys_area_select: Escape goes back to the main menu.
    for (const StartupKey &k : keys) {
        if (k.code == StartupKey::Escape) {
            state_ = State::MenuEnter;
            return;
        }
    }
    // process_mouse_area_select: a click, or 600 frames without the mouse
    // moving, starts the level (level 1 for a new game).
    bool start = mouse.left_pressed;
    if (last_mouse_sum_ != mouse.x + mouse.y) {
        idle_ = 600;
        last_mouse_sum_ = mouse.x + mouse.y;
    }
    if (int16_t(idle_--) == 0)
        start = true;
    if (!start)
        return;
    if (level_ == 0)
        level_ = 1;
    result_ = {};
    result_.kind = StartupResult::Kind::StartLevel;
    result_.level = level_;
    result_.balance_id = balance_files_.empty() ? std::string(kDefaultBalanceId) : balance_files_[balance_];
    result_.tutorial = tutorial_;
    screen_ = StartupScreen::Done;
    state_ = State::Done;
}

void Startup::init_zoom_boxes(int mask)
{
    current_display_ = desired_display_ = 0;
    zoom_ = {};
    for (int bit = 1; bit <= 2; bit <<= 1) {
        if (!(mask & bit))
            continue;
        // add_glass_frame and add_glass_box.
        std::span<const GlassBox> frames = bit == 1 ? std::span<const GlassBox>(kPrefsGlass) : kNameGlass;
        for (const GlassBox &g : frames) {
            auto slot = std::find_if(zoom_.begin(), zoom_.end(), [](const ZoomBox &z) { return !z.active; });
            if (slot == zoom_.end())
                break;
            ZoomBox &z = *slot;
            z.active = true;
            if (g.x == z.to[0] && g.y == z.to[1] && g.x1 - g.x + 1 == z.to[2] && g.y1 - g.y + 1 == z.to[3])
                continue;
            z.step = 0;
            z.steps = g.steps;
            z.id = bit;
            z.type = g.type;
            z.button = g.button;
            z.open_delay = z.close_delay = g.delay;
            switch (g.origin) {
            case 1:
                z.from = {g.x + (g.x1 - g.x) / 2, g.y, 0, g.y1 - g.y + 1};
                break;
            case 0:
                z.from = {g.x + (g.x1 - g.x) / 2, g.y + (g.y1 - g.y) / 2, 0, 0};
                break;
            case 2:
                z.from = {kUiWidth / 2, kUiHeight / 2, 0, 0};
                break;
            }
            z.to = {g.x, g.y, g.x1 - g.x + 1, g.y1 - g.y + 1};
        }
        desired_display_ |= bit;
    }
}

void Startup::zoom_levels(int &lo, int &hi, bool &incomplete) const
{
    lo = -128, hi = -128;
    incomplete = false;
    bool found = false;
    for (const ZoomBox &z : zoom_) {
        if (!z.active)
            continue;
        if ((z.step > 0 && z.step < z.steps - 1) || (z.step && !(desired_display_ & z.id))) {
            found = true;
            hi = std::max(hi, z.type);
        }
        if (z.step == 0 && !found) {
            hi = z.type;
            found = true;
        }
        if (z.step > 0 && z.step <= z.steps - 1)
            lo = std::max(lo, z.type);
        if (!(desired_display_ & z.id))
            incomplete = true;
    }
    if (hi == -128)
        hi = lo;
    else if (hi > lo)
        lo = hi;
}

bool Startup::open_zoom_box(ZoomBox &z, int hi, bool incomplete)
{
    if (z.id & current_display_) {
        draw_box(z);
        return false;
    }
    if (incomplete || z.type > hi)
        return false;
    bool opening = false;
    if (z.type == hi) {
        if (z.open_delay) {
            z.open_delay--;
        } else if (++z.step < z.steps - 1) {
            opening = true;
        } else {
            bool all = std::none_of(zoom_.begin(), zoom_.end(), [&](const ZoomBox &o) {
                return o.active && &o != &z && o.id == z.id && o.step != o.steps - 1;
            });
            if (all)
                current_display_ |= z.id;
            if (z.step > z.steps - 1)
                z.step--;
        }
    }
    draw_box(z);
    return opening;
}

bool Startup::close_zoom_box(ZoomBox &z, int hi)
{
    bool closing = false;
    if (z.type == hi) {
        if (!z.close_delay) {
            current_display_ &= ~z.id;
            closing = true;
            if (--z.step <= 0)
                z.active = false;
        } else {
            z.close_delay--;
        }
    }
    draw_box(z);
    return closing;
}

bool Startup::draw_zoom_boxes()
{
    int lo = 0, hi = 0;
    bool incomplete = false;
    zoom_levels(lo, hi, incomplete);
    bool result = false;
    for (ZoomBox &z : zoom_) {
        if (!z.active || z.type > lo)
            continue;
        if (z.id & desired_display_)
            result |= open_zoom_box(z, hi, incomplete);
        else
            result |= close_zoom_box(z, hi);
    }
    return result;
}

void Startup::draw_box(const ZoomBox &z)
{
    // draw_a_box and the box drawers of src/glass.cpp.
    int total = z.steps - 1;
    int s = z.step;
    if (s == 0 || total <= 0)
        return;
    int x = int16_t((z.to[0] * s + (total - s) * z.from[0]) / total);
    int y = int16_t(((total - s) * z.from[1] + z.to[1] * s) / total);
    int w = int16_t(((total - s) * z.from[2] + z.to[2] * s) / total);
    int h = int16_t(((total - s) * z.from[3] + z.to[3] * s) / total);

    auto fill = [&](int bx, int by, int bw, int bh, uint8_t colour, bool ghost) {
        fill_box({bx, by, bw, bh}, colour, ghost);
    };
    auto outline = [&](int bx, int by, int bw, int bh, uint8_t colour) { outline_box({bx, by, bw, bh}, colour); };
    const BoxColours &c = colours_;
    // A single difficulty hides the arrows (and every button but type 2's).
    if (z.button) {
        if (!(z.type != 2 && balance_files_.size() == 1)) {
            fill(x, y, w, h, c.white, false);
            outline(x, y, w, h, 0xf8);
            draw_button_edges(x, y, w - 1, h - 1, false);
        }
        return;
    }
    switch (z.type) {
    case -1:
    case 0:
        fill(x, y, w, h, c.white, z.type == 0);
        outline(x, y, w, h, c.black);
        for (int i = 1; i <= 3; i++)
            outline(x + i, y + i, w - 2 * i, h - 2 * i, z.type == 0 ? c.blue_grey : 0xca);
        break;
    case 1:
        fill(x, y, w, h, c.white, false);
        outline(x, y, w, h, 0xf8);
        for (int i = 1; i <= 3; i++)
            outline(x + i, y + i, w - 2 * i, h - 2 * i, c.blue_grey);
        break;
    default:
        fill(x, y, w, h, c.white, false);
        outline(x, y, w, h, c.blue_grey);
        outline(x + 1, y + 1, w - 2, h - 2, c.blue_grey);
        break;
    }
}

void Startup::fill_box(Rect box, uint8_t colour, bool ghost)
{
    // LbDrawBox filled: see-through through the ghost table (the colour as
    // the row, as sprites are) or solid.
    for (int yy = std::max(box.y, 0); yy < std::min(box.y + box.h, surface_.height()); yy++) {
        uint8_t *row = surface_.row(yy);
        for (int xx = std::max(box.x, 0); xx < std::min(box.x + box.w, surface_.width()); xx++)
            row[xx] = ghost ? prefs_ghost_[size_t(colour) * 256 + row[xx]] : colour;
    }
}

void Startup::outline_box(Rect box, uint8_t colour)
{
    // LbDrawBox outlined.
    const int bx = box.x, by = box.y, bw = box.w, bh = box.h;
    if (bw < 1 || bh < 1)
        return;
    fill_box({bx, by, bw, 1}, colour, false);
    fill_box({bx, by + bh - 1, bw, 1}, colour, false);
    if (bh > 2) {
        fill_box({bx, by + 1, 1, bh - 2}, colour, false);
        fill_box({bx + bw - 1, by + 1, 1, bh - 2}, colour, false);
    }
}

void Startup::draw_glass_text(const UiMouse &mouse, int button, std::string_view text, bool highlight)
{
    // draw_glass_text and draw_glass_text_hilight: centred on the button,
    // a pixel down and right in the highlighted font while held or chosen.
    const ScreenButton &b = kPrefsButtons[size_t(button)];
    int w = b.x1 - b.x + 1, h = b.y1 - b.y + 1;
    bool held = (mouse.left_held || mouse.left_released) && over(mouse.x, mouse.y, b.x, b.y, w, h);
    bool down = highlight || held;
    const Font &font = fonts_[down ? kFontGlassHi : kFontGlass];
    int x = (w - text_width(font, text)) >> 1;
    int y = (b.y1 + b.y + 1 - font.glyph('A').height) >> 1;
    if (down) {
        x++;
        y++;
    }
    draw_text(surface_, font, x, y, text, {{b.x, 0, w}, {0, 0, surface_.width(), surface_.height()}});
}

void Startup::draw_button_edges(int x, int y, int w, int h, bool sunken)
{
    // draw_button_edges: three lines a side, light above and left when
    // raised, below and right when sunken.
    auto line = [&](int x1, int y1, int x2, int y2, uint8_t colour) {
        for (int yy = std::min(y1, y2); yy <= std::max(y1, y2); yy++) {
            for (int xx = std::min(x1, x2); xx <= std::max(x1, x2); xx++) {
                if (xx >= 0 && yy >= 0 && xx < surface_.width() && yy < surface_.height())
                    surface_.row(yy)[xx] = colour;
            }
        }
    };
    int a = sunken ? 5 : 2, b = sunken ? 2 : 5;
    for (int i = 0; i < 3; i++) {
        uint8_t top = kGray[size_t(a + i)], bottom = kGray[size_t(b + i)];
        line(x + 1 + i, y + 1 + i, x + w - 1 - i, y + 1 + i, top);
        line(x + 1 + i, y + 2 + i, x + 1 + i, y + h - 1 - i, top);
        line(x + 2 + i, y + h - 1 - i, x + w - 1 - i, y + h - 1 - i, bottom);
        line(x + w - 1 - i, y + 2 + i, x + w - 1 - i, y + h - 2 - i, bottom);
    }
}

std::vector<std::string_view> level_briefing(const LanguageText &text, int level)
{
    std::vector<std::string_view> page;
    int n = int(text.size(TextSection::Level));
    int i = 0;
    for (int p = 0; p < level; p++) {
        while (i < n && text.at(TextSection::Level, i) != ".") {
            if (text.at(TextSection::Level, i) == "..")
                return page;
            i++;
        }
        i++;
    }
    for (; i < n; i++) {
        std::string_view s = text.at(TextSection::Level, i);
        if (s == "." || s == "..")
            break;
        page.push_back(s);
    }
    return page;
}

} // namespace th
