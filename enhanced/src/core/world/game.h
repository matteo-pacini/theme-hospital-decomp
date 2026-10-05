#pragma once

#include "core/data/anims.h"
#include "core/data/balance.h"
#include "core/data/level.h"
#include "core/world/arrivals.h"
#include "core/world/complex.h"
#include "core/world/world.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace th
{

// Turns a second at each of the game's speeds (vary_speed, src/utils.cpp).
// The fastest, "and then some", runs three turns a frame at the game's 25
// frames a second.
enum class Speed : int {
    Slowest = 1,
    Slower = 5,
    Normal = 22,
    Max = 25,
    AndThenSome = 75,
};

// The game's calendar: 50 turns a day (start_turn, src/game.cpp).
constexpr int kTurnsPerDay = 50;

struct Date {
    int day = 1;   // 1 to the month's length
    int month = 0; // 0 January to 11 December
    int year = 0;  // years since the game started
};

// Days in each month (the game's calendar table, src/tables.cpp).
int days_in_month(int month);

// The state of a game in progress and its turn, as the game's start_turn
// and process_things (src/game.cpp, src/things.cpp) for what exists so far:
// the date, the opening countdown, arrivals, patients' action sequences,
// the entrance doors and the objects' animations.
class Game
{
  public:
    // Starts level `number`: its map and objects, the date at day 1 of the
    // first month, and from its settings the starting cash and the opening
    // countdown (gbv OpenCountdown turns; 0 opens the hospital at once), as
    // setup_new_level and init_player (src/start.cpp).
    void start(const Level &level, int number, const Balance &settings, const AnimData &anims,
               const PersonComplexes &complexes);

    // Runs one game turn unless paused: advances the turn count and the
    // random values the game draws every turn, the date every 50 turns,
    // the opening countdown, then the things.
    void turn();

    World world;
    Date date;
    // Turns played (the game's actual_game_turn, 1 at the start).
    uint32_t turn_count = 1;
    bool paused = false;
    // Turns until the hospital opens to patients; 0 once it is open.
    int opening_countdown = 0;
    // The level's countdown from the start (gbv OpenCountdown).
    int open_countdown = 0;
    bool open = true;
    // The player's hospital: its bank balance and reputation (0 to 1000;
    // evaluate_reputation gives 500 at the start).
    int balance = 0;
    int reputation = 500;
    // Patients arriving (src/peepmake.cpp).
    ArrivalState arrivals;
    // The patients that reached the map this turn, to be set going.
    std::vector<uint16_t> new_patients;

    // Path finding tool: puts a test patient on `cell` that only walks where
    // it is sent (it has no action sequence). Returns its id, or 0 if there
    // was no free entry. Draws from the random generator, as making a
    // patient does.
    uint16_t add_walker(int cell);
    // Path finding tool: sends person `id` to `cell`. A patient on an action
    // sequence leaves it and only walks from then on.
    void send_walker(uint16_t id, int cell);

  private:
    const AnimData *anims_ = nullptr;
    const PersonComplexes *complexes_ = nullptr;
    Balance settings_;
    std::unique_ptr<Level> level_;
    int level_number_ = 0;
    // The path finding tool's walkers, walked by walk_turn alone.
    std::vector<bool> walkers_;
};

} // namespace th
