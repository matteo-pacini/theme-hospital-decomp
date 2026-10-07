#pragma once

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/data/text.h"
#include "core/render/surface.h"
#include "core/ui/layer.h"
#include "core/world/events.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace th
{

// The end-of-level screens (src/winlose.cpp): the losing animation and line,
// and the win letter typed out over the next level's offer. The letters are
// drawn in the game's 640 x 480 screen; the caller starts the next level when
// a win is accepted.

// The keys the win/lose screens read (lbInkey: Escape and Space).
struct WinLoseKeys {
    bool escape = false;
    bool space = false;
};

// The data the letters draw (othervar.winlose_result and the next level's
// offer).
struct WinLoseView {
    bool win = false;
    int level = 0; // myhosp->level
    int32_t score = 0;
    int32_t next_level_offer = 0;
    // The letter variant: the win letter's page of the Win section
    // (winlose_text_start[pick], 0-11), or the losing line's index in the
    // WinLose section. The original picks the win letter with saferand and the
    // losing line with saferand, as the caller does.
    int pick = 0;
    std::string hospital_name;
    // The losing line is drawn in this font slot (0x13, 0x14, 0x15 or 0x16;
    // the original uses 0x15/0x16 for ANIM_LOSE3).
    int lose_font = 0x13;
};

// What a frame of the win/lose screens asks of the caller.
struct WinLoseResult {
    Event event;                 // a command for Game::event (none if EV_NONE)
    std::vector<uint8_t> sounds; // play_sound SFX_*
    // A win was accepted: the level to start next (0 when none).
    int next_level = 0;
    // The screen closed (a win declined, or the losing line dismissed).
    bool closed = false;
};

// The win/lose letters' art and state.
class WinLoseScreen
{
  public:
    // Loads the letter sprites (lettr02v) and the fonts the letters draw with
    // (cache slots 0x13-0x17). `text` is kept by reference.
    bool load(const std::filesystem::path &dir, const LanguageText &text, const Palette &game_palette,
              std::string *error = nullptr);

    // Opens the win (`win` true) or lose letter for level `level`.
    void open(bool win, int level);
    void close() { open_ = false; }
    bool is_open() const { return open_; }

    Surface &surface() { return surface_; }
    const Palette &palette() const { return palette_; }

    // One frame of the letter, `seconds` since the last frame: the win letter
    // types out and waits for the accept or decline button; the losing line
    // waits for a click or key.
    WinLoseResult frame(const UiMouse &mouse, const WinLoseKeys &keys, double seconds, const WinLoseView &view);

    // concat_composite + sprintf + letter_format_text for `view`: the letter
    // as the original draws it. Exposed for the test.
    std::string letter_text(const WinLoseView &view) const;
    // The original's sprintf: `text` with its %s/%d specifiers replaced, in
    // order, by the hospital name, the offer and the town name (the last
    // unused by the last level's letters). Exposed for the test.
    static std::string format_args(std::string text, std::string_view hospital, std::string_view offer,
                                   std::string_view town);

  private:
    // The "."-ended page `page` of `section` joined into one line (the
    // original's concat_composite).
    std::string composite_page(TextSection section, int page) const;
    // The letter wrapped to `width` pixels in `font` (letter_format_text).
    std::string wrap(const std::string &text, int width, const Font &font) const;
    // The letter for `view`, formatted and wrapped.
    std::string make_letter(const WinLoseView &view) const;
    void draw_letter();
    // The win letter (typed out, then its accept and decline buttons) and the
    // losing line (its accept button).
    void win_frame(const UiMouse &mouse, WinLoseResult &result, const WinLoseView &view);
    void lose_frame(const UiMouse &mouse, WinLoseResult &result, const WinLoseView &view);
    bool over(int x, int y, int w, int h, const UiMouse &mouse) const;

    bool open_ = false;
    bool win_ = false;
    int level_ = 0;
    Surface surface_;
    Palette palette_{};
    Palette game_palette_{};
    const LanguageText *text_ = nullptr;
    SpriteSheet sprites_;
    std::array<Font, 5> fonts_{}; // cache slots 0x13-0x17
    std::string letter_;
    int typed_ = 0;
    int type_delay_ = 0;
    double frame_time_ = 0;
    UiMouse pending_mouse_;
    WinLoseKeys pending_keys_;
};

} // namespace th