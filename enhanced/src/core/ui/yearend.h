#pragma once

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/data/text.h"
#include "core/render/surface.h"
#include "core/ui/layer.h"
#include "core/world/events.h"
#include "core/world/year_end.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace th
{

// The year-end screens: the hall of fame (src/score.cpp) then the league
// tables, awards, trophies and plaque (src/annualch.cpp), shown in place of
// everything else while the game is paused behind them. The sequence is
// driven by the caller from Game::progress's year_end_type (0 sets it up, 1
// the hall, 2 the charts, 3/4 finish); this class owns the screens' own state
// (hall.state, yearend.state, current_display, current_award). Player input is
// Escape/Space and the buttons, all as the original; the screens also close
// themselves after the original's idle countdown.

// The keys the year-end screens read (lbInkey: Escape and Space).
struct YearEndKeys {
    bool escape = false;
    bool space = false;
};

// The hall of fame table (src/score.cpp's HospScores, save/hiscore.dat).
struct HighScore {
    std::string name;
    int32_t score = 0;
};
struct HospScores {
    std::array<HighScore, 10> fame{};
    std::array<HighScore, 10> shame{};
};

// The table as src/score.cpp initialises it (hosp_scores).
HospScores default_hosp_scores();

// load_highscores (0x95790) / save_highscores (0x95710): reads or writes the
// table at `path` (the original's save/hiscore.dat). Loading a missing file
// leaves `scores` as it was; both return false on an I/O failure.
bool load_highscores(const std::filesystem::path &path, HospScores &scores);
bool save_highscores(const std::filesystem::path &path, const HospScores &scores);

// insert_hall_details (0x951c8): enters `name` and `score` in the hall of fame
// if they beat its lowest entry, else in the hall of shame if they beat its
// lowest, replacing their own entry if they have one with a lower score.
// Returns othervar.showing_fame (which hall the screen shows).
bool insert_hall_details(HospScores &scores, std::string_view name, int32_t score);

// The data the league tables and awards draw, from Game::progress and the
// game (Game::date.year, Game::level_number() and the hospitals' names).
struct YearEndView {
    int year = 0;                // mycontrol->year, drawn as year + 1999
    int level = 0;               // myhosp->level
    int plaque_rat_kills = 0;    // othervar.plaque_rat_kills
    int plaque_rat_kill_pct = 0; // othervar.plaque_rat_kill_pct
    const std::array<Award, 13> *awards = nullptr;
    const std::array<Trophy, 9> *trophies = nullptr;
    const std::array<std::array<ChartEntry, 4>, 11> *chart = nullptr;
    const std::array<std::string, 4> *hospital_names = nullptr;
};

// What a frame of the year-end screens asks of the caller.
struct YearEndResult {
    // The player's command, for Game::event (none if type is EV_NONE).
    Event event;
    // The sound effects played (SFX_* of include/enums.h), in order.
    std::vector<uint8_t> sounds;
    // The hall (state 2) or the charts (state 4) finished this frame: the
    // caller advances year_end_type (the original's year_end_type++).
    bool hall_done = false;
    bool charts_done = false;
};

// The year-end screens' art and state.
class YearEndScreen
{
  public:
    // Loads the hall of fame's picture and sprites (fame01v/fame02v) and the
    // awards' (award01v/award02v and award03v), their palettes, the fonts the
    // screens draw with (cache slots 0x10-0x17) and the help text section.
    // `text` is kept by reference. On failure says why in `error`.
    bool load(const std::filesystem::path &dir, const LanguageText &text, const Palette &game_palette,
              std::string *error = nullptr);

    // Starts the sequence: the hall's and the charts' state reset, the game's
    // pause set as do_year_end case 0 does.
    void open(const HospScores &scores, bool showing_fame);
    void close() { open_ = false; }
    bool is_open() const { return open_; }

    // The phase being shown: 0 the hall, 1 the charts.
    int phase() const { return phase_; }
    void set_phase(int phase) { phase_ = phase; }

    Surface &surface() { return surface_; }
    const Palette &palette() const { return palette_; }

    // One frame of the hall of fame (display_hall), `seconds` since the last
    // frame; `player_name`/`player_score` are allhosps[player]. The picture is
    // drawn into surface(). Nothing while the phase is not the hall.
    YearEndResult hall_frame(const UiMouse &mouse, const YearEndKeys &keys, double seconds,
                             std::string_view player_name, int32_t player_score);
    // One frame of the league tables, awards and plaque (display_year_charts);
    // `view` is the year's figures. Nothing while the phase is not the charts.
    YearEndResult charts_frame(const UiMouse &mouse, const YearEndKeys &keys, double seconds, const YearEndView &view);

    // The hall of fame table (insert_hall_details' othervar.showing_fame).
    const HospScores &scores() const { return scores_; }
    bool showing_fame() const { return showing_fame_; }

  private:
    struct Picture {
        Surface pixels;
        Palette palette{};
        std::array<uint8_t, 768> vga{};
        SpriteSheet sprites;
    };

    // display_hall's states.
    void hall_open();
    void hall_draw();
    // draw_fame_text (0x94d34): the sliding title and the ten rows.
    void draw_fame_text();
    // move_text (0x94500): slides the rows and title towards their targets.
    void move_text();
    // set_fame_pos (0x947f0) / set_shame_pos (0x94914).
    void set_hall_pos(bool fame);
    // modify_background (0x94a18): the fixed labels into the picture.
    void draw_hall_labels();
    // check_hall_buttons (0x9542c) / process_hall_buttons (0x9536c).
    void hall_buttons(const UiMouse &mouse, YearEndResult &result);
    void process_hall_button(int button, YearEndResult &result);

    // display_year_charts' states.
    void charts_open();
    void league_draw();
    void awards_open();
    void awards_draw();
    // display_awards (0xb647c) / display_plaque (0xb5460).
    void collect_awards_won();
    void draw_plaque();
    // process_yearend_buttons (0xb5b38) and the page buttons' checks.
    void process_yearend_button(int button, YearEndResult &result);
    void league_buttons(const UiMouse &mouse, YearEndResult &result);
    void awards_buttons(const UiMouse &mouse, YearEndResult &result);
    bool button_available(int button) const;
    // The view the charts page draws (charts_frame's data).
    void set_view(const YearEndView &view);
    // The escape/space and idle close, and the mouse-movement idle reset.
    void charts_input(UiMouse &mouse, YearEndResult &result);

    // The picture and palette of the page now drawn.
    void use_picture(int index);
    // Whether `sprite` is in the loaded sheet.
    Sprite sprite(int index) const;
    // Draws `sprite` at (x, y) in the current palette.
    void draw_sprite_at(int x, int y, int sprite);
    // LbTextDraw of `text` at (x, y) in `font`.
    void draw_text_at(const Font &font, int x, int y, std::string_view text, unsigned flags = 0, Rect clip = {});
    // The screen's button table entries and hotspot test.
    bool over(int x, int y, int w, int h, const UiMouse &mouse) const;

    bool open_ = false;
    int phase_ = 0; // 0 hall, 1 charts
    Surface surface_;
    Palette palette_{};
    Palette game_palette_{};
    const LanguageText *text_ = nullptr;

    Picture fame_;                // SCR_HALL_OF_FAME
    Picture awards1_;             // SCR_AWARDS1 (the awards page)
    Picture awards2_;             // SCR_AWARDS2 (the league tables)
    Picture *active_ = nullptr;   // the page now drawn
    std::array<Font, 8> fonts_{}; // cache slots 0x10-0x17

    // The hall of fame (src/score.cpp).
    HospScores scores_{};
    bool showing_fame_ = true;
    int hall_state_ = 0;
    int hall_idle_ = 0;
    // The rows' and title's positions, 16.16 fixed point.
    std::array<int32_t, 10> text_pos_{};
    std::array<int32_t, 10> desired_text_pos_{};
    std::array<int32_t, 20> title_pos_{};
    std::array<int32_t, 20> desired_title_pos_{};
    std::string title_;
    int title_len_ = 0;

    // The year-end charts (src/annualch.cpp).
    int charts_state_ = 0;
    int charts_idle_ = 0;
    int current_display_ = 0;
    int current_award_ = 0;
    bool exit_screen_ = false;
    int last_mouse_sum_ = 0;
    std::array<Award, 6> awards_won_{};
    std::array<Trophy, 3> trophies_won_{};

    // The year's data the charts page draws (charts_frame's view).
    int view_year_ = 0;
    int view_rat_kills_ = 0;
    int view_rat_pct_ = 0;
    const std::array<Award, 13> *view_awards_ = nullptr;
    const std::array<Trophy, 9> *view_trophies_ = nullptr;
    const std::array<std::array<ChartEntry, 4>, 11> *view_chart_ = nullptr;
    const std::array<std::string, 4> *view_names_ = nullptr;

    // Frame pacing: one game frame per 40 ms, as the charts.
    double frame_time_ = 0;
    UiMouse pending_mouse_;
    YearEndKeys pending_keys_;
};

} // namespace th