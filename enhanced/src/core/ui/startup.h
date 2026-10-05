#pragma once

#include "core/data/palette.h"
#include "core/data/smacker.h"
#include "core/data/sprites.h"
#include "core/data/text.h"
#include "core/render/surface.h"
#include "core/ui/layer.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace th
{

// A key press as the start-up screens read it (lbInkey).
struct StartupKey {
    enum Code : uint8_t {
        Other,
        Letter, // `letter` is 'A' to 'Z'
        Escape,
        Return,    // Return or keypad Enter
        Backspace, // Backspace or keypad '.'
        Space,
    };
    Code code = Other;
    char letter = 0;
};

// What the start-up screens are showing.
enum class StartupScreen {
    Movie,       // a movie (movie() says which)
    Title,       // TITLE01V, waiting for a click or key; the attract movies play after a while
    Name,        // the player's name typed over the title
    MainMenu,    // MAIN01M, 320 x 200
    Preferences, // PREF01V: difficulty and tutorial
    Briefing,    // the level's area movie's last frame, waiting for a click
    Done,        // a level was started or the game quit
};

// What a frame of the start-up screens asks the caller to do.
struct StartupResult {
    enum class Kind { Running, StartLevel, Quit };
    Kind kind = Kind::Running;
    // StartLevel: the level, its balance id (the balance file prefix chosen
    // in the preferences: "EASY", "FULL" or "HARD", as the files are named)
    // and whether the advisor's tutorial is on (advisor.flags bit 0).
    int level = 0;
    std::string balance_id;
    bool tutorial = false;
};

// The game's start-up flow, from the intro to a level's start, as game()
// runs it without debug.direct_start (display_mode DM_MAIN_MENU, mainmenu
// state MMS_INTRO): the intro movie (draw_main_menu, play_intro: INTRO.SM4,
// the better of the two), the title screen with the attract movies, the
// name screen (draw_nickname, src/name.cpp), the main menu with its
// opening and closing movies (src/mainmenu.cpp), the preferences screen
// where the difficulty and tutorial are chosen, and the level's area movie
// before it starts (draw_area_select, src/area.cpp). Load, Continue (there
// are no saves) and Network are refused as the game refuses Continue
// without a save. Palette fades, the help tooltips and the screen pointers
// are not drawn.
//
// It draws into an indexed surface with its own palette: 640 x 480, or
// 320 x 200 for the intro, the main menu and its movies (the game's
// low-resolution mode). The game runs a frame every 40 ms; movies run at
// their own rate.
class Startup
{
  public:
    // Loads the screens' art and fonts from `dir`. `text` is the language's
    // text, kept by reference; `language` picks the difficulty names. On
    // failure says why in `error`.
    bool load(const std::filesystem::path &dir, const LanguageText &text, int language, std::string *error = nullptr);

    // Runs the screens for `seconds` of time with this mouse (in the
    // surface's coordinates) and the keys pressed since the last call:
    // movies advance by their frame time, the screens by a game frame per
    // 40 ms (at most five per call). Mouse presses, releases and keys are
    // kept for the next game frame when none runs.
    StartupResult frame(const UiMouse &mouse, std::span<const StartupKey> keys, double seconds);

    StartupScreen screen() const { return screen_; }
    // The movie playing (a MovieIndex), -1 if none.
    int movie() const { return movie_index_; }

    const Surface &surface() const { return surface_; }
    const Palette &palette() const { return palette_; }

    // The audio of the movie playing: its format (rate 0 if none or played
    // silent), and the samples decoded since the last call, interleaved
    // signed 16-bit, appended to `out`. When the movie ends or is skipped
    // the caller should drop what it still has queued.
    SmackerAudioFormat movie_audio_format() const;
    void take_movie_audio(std::vector<int16_t> &out);

    // The sound effects played since the last call (play_sound with
    // SNDSRC_GENERAL; SFX_* numbers in include/enums.h), in order.
    std::vector<uint8_t> take_sounds();

    // The name typed on the name screen (myplayer.name; misc_text[1] if
    // none was typed).
    const std::string &player_name() const { return name_; }

  private:
    enum class State;
    struct Picture {
        Surface pixels;
        Palette palette{};
        std::array<uint8_t, 768> vga{};
    };
    struct ZoomBox {
        bool active = false;
        int step = 0, steps = 0;
        int open_delay = 0, close_delay = 0;
        int id = 0;
        int type = 0;
        bool button = false;
        std::array<int, 4> from{}, to{};
    };
    struct BoxColours {
        uint8_t blue_grey = 0, white = 0, black = 0;
    };

    void tick(const UiMouse &mouse, std::span<const StartupKey> keys);
    void advance_movie(double seconds, bool skip);
    void play_movie(int index, State next);
    void show(const Picture &picture);
    void calculate_grey(const Picture &picture);
    void main_menu(const UiMouse &mouse, std::span<const StartupKey> keys);
    void name_screen(std::span<const StartupKey> keys);
    void preferences(const UiMouse &mouse, std::span<const StartupKey> keys);
    void briefing(const UiMouse &mouse, std::span<const StartupKey> keys);
    void init_zoom_boxes(int mask);
    bool draw_zoom_boxes();
    void draw_box(const ZoomBox &box);
    void draw_glass_text(const UiMouse &mouse, int button, std::string_view text, bool highlight);
    void draw_button_edges(int x, int y, int w, int h, bool sunken);
    void sound(uint8_t sfx) { sounds_.push_back(sfx); }

    std::filesystem::path dir_;
    const LanguageText *text_ = nullptr;
    int language_ = 0;
    Picture title_, main_menu_, prefs_;
    SpriteSheet main_sprites_, prefs_sprites_;
    std::vector<uint8_t> prefs_ghost_;
    std::array<Font, 3> fonts_; // cache slots 0x21, 0x22, 0x23

    State state_{};
    StartupScreen screen_ = StartupScreen::Movie;
    Surface surface_;
    Palette palette_{};
    double frame_time_ = 0;
    int timer_ = 0;
    int attract_count_ = 0;

    SmackerVideo video_;
    int movie_index_ = -1;
    unsigned movie_flags_ = 0;
    double movie_time_ = 0;
    State after_movie_{};
    std::vector<int16_t> movie_audio_;
    std::vector<uint8_t> sounds_;

    UiMouse pending_mouse_;
    std::vector<StartupKey> pending_keys_;

    int menu_state_ = 0; // mainmenu.state
    std::string name_;
    std::vector<std::string> balance_files_;
    size_t balance_ = 0;
    std::string balance_name_;
    bool tutorial_ = false;
    bool low_res_ = false;
    int level_ = 0;
    int idle_ = 600;
    int last_mouse_sum_ = 0;

    std::array<ZoomBox, 46> zoom_{};
    int current_display_ = 0, desired_display_ = 0;
    BoxColours colours_;
    StartupResult result_;
};

// The level's briefing, the help page the game opens once level `level`
// starts (show_help_page(1, level), src/helpmsg.cpp): the strings of
// level_text's page `level`, pages being separated by "." and the list
// ended by ".." (setup_indices_for_composite_help). Empty if there is no
// such page.
std::vector<std::string_view> level_briefing(const LanguageText &text, int level);

} // namespace th
