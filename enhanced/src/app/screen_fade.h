#pragma once

#include "core/ui/fade.h"

#include <SDL3/SDL.h>

#include <deque>
#include <functional>
#include <string>

namespace th
{

// The palette fades between the game's screens and its loading screen, shown
// by dimming whole frames: the game does nothing else while it fades
// (LbPaletteFade, src/lib/bf1/gpalette.cpp:24, blocks), so the fade takes
// the window from the frame loop. The frame being left stays as it was drawn
// and fades out, the new screen's first frame fades in.
//
// The fader owns the two frame targets the game draws into. The frame loop
// asks it for a target, draws, and hands it back; in between frames it
// presents the fades' pictures itself.
class ScreenFader
{
  public:
    ~ScreenFader();
    // Frees the targets; before the renderer goes.
    void release();

    // Fades on or off (a run that wants screens to change at once).
    void enable(bool on) { enabled_ = on; }

    // Which picture a fade step dims.
    enum class Source {
        Shown,   // the frame last shown
        Loading, // the loading screen (drawn by the caller)
        Next,    // the next frame the game draws
    };

    // Queues the fades of the change from the screen shown last frame to
    // `now`, unless fades are queued already (a load queued its own).
    void screen(const ScreenId &now);

    // The game's loading screen (display_loading_screen, clear_loading_
    // screen: src/screens.cpp:251, :284) around a level's start: the old
    // screen fades out, the loading picture in and out, the new screen in.
    void queue_level_start();
    // The same at the program's start (setup_host, src/pchost.cpp:90), with
    // the first screen fading in unless it is a movie.
    void queue_boot(bool fade_in_first_screen);

    // Whether a fade is running or queued before the next frame is drawn:
    // the game's frame (turns, input) waits.
    bool blocking() const;
    bool running() const { return fade_.running(); }
    // "none", or what the fade is doing (the debug report).
    std::string state() const;

    // Presents a fade's frame when one is due and returns true; `draw_loading`
    // draws the loading picture into the current target. False means the
    // caller draws the frame, between begin_frame and end_frame.
    bool present(SDL_Renderer *renderer, int width, int height, double seconds,
                 const std::function<void()> &draw_loading);
    // Makes the frame target current and clears it to black.
    void begin_frame(SDL_Renderer *renderer, int width, int height);
    // Presents the frame drawn since begin_frame, dimmed by a fade starting
    // on it.
    void end_frame(SDL_Renderer *renderer);

  private:
    struct Step {
        PaletteFade::Kind kind;
        int steps;
        Source source;
    };

    void queue(PaletteFade::Kind kind, int steps, Source source);
    void make_targets(SDL_Renderer *renderer, int width, int height);
    // Shows `target` at the running fade's brightness.
    void show(SDL_Renderer *renderer, SDL_Texture *target) const;

    bool enabled_ = true;
    std::deque<Step> steps_;
    PaletteFade fade_;
    // Two targets: the frame being drawn and the one shown last.
    SDL_Texture *targets_[2] = {};
    int width_ = 0, height_ = 0;
    int current_ = 0, shown_ = -1;
    ScreenId last_;
    bool have_last_ = false;
};

} // namespace th
