#pragma once

#include <cstdint>

namespace th
{

// The game's palette fade as the screen shows it. LbPaletteFade
// (src/lib/bf1/gpalette.cpp:24) with Lb_PALETTE_FADE_CLOSED sets palette n of
// `steps` (n = 0 to steps), one per vertical blank, and returns when n has
// reached steps: the fade lasts `steps` blanks, and holds nothing the player
// can touch (the game does nothing else meanwhile).
class PaletteFade
{
  public:
    // Brightness goes from the picture's down to black (Out) or from black up
    // to the picture's (In).
    enum class Kind { Out, In };

    // The vertical blank's period. The game's fades ran at 70 Hz in both its
    // modes in DOSBox-X: twenty fades of 10 steps took 3.07 s (the 320 x 200
    // mode's rate; a real card's 640 x 480 modes blank at 60 Hz).
    static constexpr double kPeriod = 1.0 / 70.09;

    // Starts a fade of `steps` blanks (at least one) of `period` seconds.
    void start(Kind kind, int steps, double period);
    void stop() { running_ = false; }

    // Time passes: `seconds` of blanks.
    void advance(double seconds);

    bool running() const { return running_; }
    Kind kind() const { return kind_; }
    // The palette number n of the fade now showing (0 to steps).
    int step() const;
    // Whether the last palette (n == steps) is showing.
    bool done() const { return step() >= steps_; }
    // The share of the picture's brightness showing, 0 to 1.
    float brightness() const;

  private:
    Kind kind_ = Kind::Out;
    int steps_ = 1;
    double period_ = kPeriod;
    double elapsed_ = 0;
    bool running_ = false;
};

// The screens a fade lies between, as far as the game fades them.
enum class FadeScreen {
    Setup,    // the game data setup screen (not the game's)
    Startup,  // a start-up screen: title, about, main menu, load game, ...
    Movie,    // a start-up movie
    Hospital, // the hospital view
    Chart,    // a full-screen chart (`sub` says which)
    TownMap,
    Fax,
    YearEnd,      // a year-end screen (`sub` says which)
    WinLoseMovie, // the level-end movie
    WinLoseLetter,
};

struct ScreenId {
    FadeScreen kind = FadeScreen::Setup;
    int sub = 0;
    bool operator==(const ScreenId &) const = default;
};

// How a change of screen fades: `out_steps` blanks of the old screen going
// to black, then `in_steps` of the new one coming up from black (0: none).
struct FadePlan {
    int out_steps = 0;
    int in_steps = 0;
};

// What the game does between screens. A picture loaded with load_ingame_pic
// (src/screens.cpp:136) fades the old screen out in 10 steps (fade_out_menu,
// :243) and, at the end of the frame (src/game.cpp:254), the new one in over
// 10; leaving a screen for the hospital (return_from_screen, :49) fades out
// in 5 and back in over 10. A movie sets its own palette, so nothing fades
// into it; the letter after the level-end movie reuses the movie's picture
// and palette (src/winlose.cpp:203, fade 0), and the name box is drawn over
// the title.
FadePlan fade_between(const ScreenId &from, const ScreenId &to);

// One colour component (0 to 63) of palette `count` of a fade from `from` to
// `to` in `steps` steps, as LbPaletteFade computes it: the difference scaled
// by count / steps, truncated towards zero, added to `from`.
int fade_component(int from, int to, int count, int steps);

} // namespace th
