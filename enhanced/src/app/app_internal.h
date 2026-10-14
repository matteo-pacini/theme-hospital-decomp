#pragma once

#include "app/app.h"

// What the parts of App (app_*.cpp) and main.cpp share besides the class.

namespace th
{

inline namespace app_detail
{

// The view never shows less than the original's 640x480; automatic zoom
// picks the largest integer scale that keeps at least this many lines.
constexpr int kMinViewHeight = 480;

// The scroll keys' bits (move_windows' scroll.up/down/left/right).
constexpr uint8_t kScrollUp = 1, kScrollDown = 2, kScrollLeft = 4, kScrollRight = 8;

// What a level leaves for the next: the hospital's score (0: none), the
// rivals' monthly banks (null: none) and whether a bonus level is forced.
struct Carried {
    int32_t score = 0;
    const RivalBuckets *buckets = nullptr;
    bool force_bonus = false;
};

// Starts level `number` with its settings (the balance set `balance_id`:
// "EASY", "FULL" or "HARD"), or returns nothing and says why. `carried` is
// what the last level left.
std::unique_ptr<LevelView> begin_level(const GameData &data, const Level &level, int number,
                                       std::string_view balance_id, std::string *error, Carried carried = {});

// Draws the hospital view into `surface`.
void draw_view(Surface &surface, const GameData &data, LevelView *level, Point camera, bool shadows,
               const DrawnThings &drawn = {});

// Whether the hospital has a receptionist (the announcer's pages wait for
// one).
bool has_receptionist(const World &world);

// Where `src` goes in an `out_w` x `out_h` window, as large as fits whole and
// centred: at a whole-number scale where it can be (a window smaller than
// `src` shows it reduced to fit).
SDL_FRect fit_rect(const Surface &src, int out_w, int out_h);

// saferand (src/rand.cpp), for the year-end presentation's letter variants.
uint16_t saferand(uint32_t &seed);

// Plays one of the game's turns (Game::turn or Game::process_turn) on the
// shell's presentation seed: the original has one safeseed, which the turn
// draws from as the shell does, so the seed goes in and comes back.
inline void play_turn_on_seed(Game &game, uint32_t &seed, void (Game::*turn)())
{
    game.safeseed = seed;
    (game.*turn)();
    seed = game.safeseed;
}

// A key pressed in the start-up screens, which read keys as lbInkey.
StartupKey startup_key(SDL_Keycode key);

} // namespace app_detail

} // namespace th
