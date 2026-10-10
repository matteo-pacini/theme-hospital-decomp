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

class HelpSystem;
struct HelpContext;

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
    LoadGame,    // PREF01V with the save slots' list (src/loadscr.cpp)
    Preferences, // PREF01V: difficulty and tutorial
    Briefing,    // the level's area movie's last frame, waiting for a click
    Credits,     // CRED01V with the credit text scrolling up over it
    Done,        // a level was started or the game quit
};

// What a frame of the start-up screens asks the caller to do.
struct StartupResult {
    enum class Kind { Running, StartLevel, LoadGame, Quit };
    Kind kind = Kind::Running;
    // StartLevel: the level, its balance id (the balance file prefix chosen
    // in the preferences: "EASY", "FULL" or "HARD", as the files are named)
    // and whether the advisor's tutorial is on (advisor.flags bit 0).
    int level = 0;
    std::string balance_id;
    bool tutorial = false;
    // LoadGame: the save slot to load (loadscr's choice, or the Continue slot
    // 0x63 when the main menu's Continue was chosen).
    int slot = 0;
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

    // mainmenu.state MMS_UNLOAD_GAME: returns to the main menu from a level
    // that ended (the losing letter), showing its opening movie.
    void return_to_main_menu() { enter_menu(); }

    // The game won (level 12's letter accepted): the WIN_GAME movie, the
    // scrolling credits, then the main menu with its opening movie.
    void start_credits();

    // A load the caller refused: the load game screen stays up
    // (from_load_screen, loadscr state 1's failure), or the main menu returns
    // (mainmenu MMS_CONTINUE_FAILED).
    void load_failed(bool from_load_screen);

    // The folder the saved games live in; the load screen lists its
    // "<player name>.h<slot>" files. Empty lists none.
    void set_save_dir(const std::filesystem::path &dir) { save_dir_ = dir; }

    // The help system and context the main menu's tooltip is drawn from (the
    // caller owns both; the context's mouse is the one frame gets).
    void set_help(HelpSystem &system, HelpContext &context)
    {
        help_ = &system;
        help_ctx_ = &context;
    }

    StartupScreen screen() const { return screen_; }
    // Whether the load screen's panels are fully open (its buttons work).
    bool load_ready() const { return (current_display_ & 0x2000) != 0; }
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
    // Whether the name screen finished since the last call (draw_nickname
    // state 1, where the game loads the player's details).
    bool take_name_confirmed()
    {
        bool confirmed = name_confirmed_;
        name_confirmed_ = false;
        return confirmed;
    }

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

    // Loads the `width` x `height` picture qdata/<name>.dat with its palette
    // qdata/<name>.pal into `out`, or says why not.
    static bool load_picture(const std::filesystem::path &dir, std::string_view name, int width, int height,
                             Picture &out, std::string *error);

    void tick(const UiMouse &mouse, std::span<const StartupKey> keys);
    // tick from the main menu on (its, apart for tick's length).
    void tick_menu(const UiMouse &mouse, std::span<const StartupKey> keys);
    void attract(const UiMouse &mouse, std::span<const StartupKey> keys);
    void enter_title();
    void enter_name();
    void enter_menu();
    void enter_preferences();
    void enter_area();
    void enter_credits();
    void credits(const UiMouse &mouse);
    // play_credits: draws one frame of the credit text and scrolls it; true
    // at the '.' line or when a click ends it.
    bool play_credits(const UiMouse &mouse);
    void finish_quit();
    // Shows `picture`; once the delay is over, zooms in boxes `mask` and
    // goes on to `next`.
    void zoom_after_delay(const Picture &picture, int mask, State next);
    void advance_movie(double seconds, bool skip);
    void play_movie(int index, State next);
    void show(const Picture &picture);
    void calculate_grey(const Picture &picture);
    void main_menu(const UiMouse &mouse, std::span<const StartupKey> keys);
    // tri_test's help: the hovered main-menu entry's tooltip, drawn over the
    // menu from help_, help_ctx_.
    void main_menu_help(const UiMouse &mouse);
    // The load game screen: draw_load_game's states 0, 3 and 4 (draw_loadscr).
    void enter_load();
    void load_tick(const UiMouse &mouse, std::span<const StartupKey> keys);
    void load_screen(const UiMouse &mouse, std::span<const StartupKey> keys);
    void draw_load_screen(const UiMouse &mouse);
    // The load screen's slot buttons and their labels (screen_buttons 0x95).
    void draw_load_buttons(const UiMouse &mouse);
    void load_button(const UiMouse &mouse, int slot, bool back, State &next);
    // Whether slot `slot` holds a save readable by this build.
    bool load_slot_used(int slot) const;
    void name_screen(std::span<const StartupKey> keys);
    void preferences(const UiMouse &mouse, std::span<const StartupKey> keys);
    void draw_new_prefs_screen(const UiMouse &mouse, bool several);
    // process_mouse_nickname's preferences button `i`, setting the state
    // that follows in `next`.
    void prefs_button(const UiMouse &mouse, int i, State &next);
    void process_nickname_button(int i, State &next);
    void briefing(const UiMouse &mouse, std::span<const StartupKey> keys);
    // process_mouse_area_select state 5: a click on one of the 21 level areas
    // plays that level's area movie and starts it. True when `mouse` was one.
    bool pick_level(const UiMouse &mouse);
    // draw_area_select state 2: hands the chosen level to the caller.
    void start_level();
    void init_zoom_boxes(int mask);
    bool draw_zoom_boxes();
    // draw_zoom_boxes' levels: boxes of a type above `lo` are skipped, those
    // of type `hi` open or close, and `incomplete` is whether an active box's
    // group is not to be shown.
    void zoom_levels(int &lo, int &hi, bool &incomplete) const;
    // A box of a shown group: opens a step, or is drawn open. True while it
    // is still opening.
    bool open_zoom_box(ZoomBox &z, int hi, bool incomplete);
    // A box of a hidden group: closes a step. True when it did.
    bool close_zoom_box(ZoomBox &z, int hi);
    void draw_box(const ZoomBox &box);
    void fill_box(Rect box, uint8_t colour, bool ghost);
    void outline_box(Rect box, uint8_t colour);
    void draw_glass_text(const UiMouse &mouse, int button, std::string_view text, bool highlight);
    void draw_button_edges(int x, int y, int w, int h, bool sunken);
    void sound(uint8_t sfx) { sounds_.push_back(sfx); }

    std::filesystem::path dir_;
    const LanguageText *text_ = nullptr;
    int language_ = 0;
    Picture title_, main_menu_, prefs_;
    // CRED01V and font slot 0x2a, loaded when the credits first play.
    Picture credits_;
    Font credit_font_;
    // play_credits' scroll (pixels into the top line), the index of that line
    // and whether a mouse button went down since the credits began (the
    // original's rleft_button is only set by a press and release).
    int credit_scroll_ = 0, credit_line_ = 0;
    bool credit_pressed_ = false;
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
    bool name_confirmed_ = false;
    std::filesystem::path save_dir_;
    // The load game screen's slot under the mouse and the one last chosen
    // (loadgame.value).
    int load_selected_ = 0xfa;
    std::vector<std::string> balance_files_;
    size_t balance_ = 0;
    std::string balance_name_;
    bool tutorial_ = false;
    int level_ = 0;
    int idle_ = 600;
    int last_mouse_sum_ = 0;
    // process_keys_area_select's C (area.state 5): any level can be picked.
    bool pick_any_level_ = false;

    std::array<ZoomBox, 46> zoom_{};
    int current_display_ = 0, desired_display_ = 0;
    // The zoom boxes' opening and closing sounds are playing
    // (playing_open_sound, playing_close_sound, src/glass.cpp).
    bool zoom_open_sound_ = false, zoom_close_sound_ = false;
    BoxColours colours_;
    StartupResult result_;
    // The help system and context the main menu's tooltip is drawn from,
    // passed to frame(); null when none.
    HelpSystem *help_ = nullptr;
    HelpContext *help_ctx_ = nullptr;
};

// The level's briefing, the help page the game opens once level `level`
// starts (show_help_page(1, level), src/helpmsg.cpp): the strings of
// level_text's page `level`, pages being separated by "." and the list
// ended by ".." (setup_indices_for_composite_help). Empty if there is no
// such page.
std::vector<std::string_view> level_briefing(const LanguageText &text, int level);

} // namespace th
