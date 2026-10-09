#pragma once

#include "core/data/anims.h"
#include "core/data/balance.h"
#include "core/data/level.h"
#include "core/ui/charts.h"
#include "core/ui/town_map.h"
#include "core/world/advisor.h"
#include "core/world/arrivals.h"
#include "core/world/complex.h"
#include "core/world/emergency.h"
#include "core/world/events.h"
#include "core/world/finance.h"
#include "core/world/staff.h"
#include "core/world/staff_complex.h"
#include "core/world/trace.h"
#include "core/world/world.h"
#include "core/world/year_end.h"

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace th
{

class GameData;

// Game::skipped_calls' bits, named as the instrumented original's TRACE_SKIP
// names the calls.
constexpr uint8_t kSkipComputerAction = 1;

// The bits for `names`, a list of such names separated by commas; false if
// one is not known.
bool parse_skipped_calls(std::string_view names, uint8_t &calls);

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

// The fax screen's kinds (the game's choice.value, display_choice_screen):
// an emergency offered, an emergency's result, a new illness, an epidemic
// found, an epidemic's end, a patient with no treatment, a VIP asking to
// visit, the VIP's verdict, a patient with no diagnosis.
constexpr uint8_t kFaxEmergency = 2;
constexpr uint8_t kFaxEmergencyOver = 3;
constexpr uint8_t kFaxNewIllness = 4;
constexpr uint8_t kFaxEpidemic = 5;
constexpr uint8_t kFaxEpidemicOver = 6;
constexpr uint8_t kFaxNoTreatment = 7;
constexpr uint8_t kFaxVip = 8;
constexpr uint8_t kFaxVipResult = 9;
constexpr uint8_t kFaxNoDiagnosis = 10;

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
               const PersonComplexes &complexes, const StaffComplexes &staff_complexes);

    // Runs one game turn unless paused: advances the turn count and the
    // random values the game draws every turn, the date every 50 turns,
    // the opening countdown, then the things.
    void turn();
    // turn() in its two halves, between which the game's frame runs (its
    // display, the advisor among it): start_turn (the game's start_turn:
    // the turn count, the random values, the twice daily and daily admin,
    // the opening countdown; false, doing nothing, while paused), then
    // run_things (process_things and the blueprints').
    bool start_turn();
    void run_things();

    World world;
    Date date;
    // Turns played (the game's actual_game_turn, 1 at the start).
    uint32_t turn_count = 1;
    bool paused = false;
    // The admin calls left out, the replay check's SKIP (kSkip*); not part of
    // a saved game.
    uint8_t skipped_calls = 0;
    // Turns until the hospital opens to patients; 0 once it is open.
    int opening_countdown = 0;
    // The level's countdown from the start (gbv OpenCountdown).
    int open_countdown = 0;
    bool open = true;
    // The player's hospital's bank balance.
    int balance = 0;
    // Its reputation, 0 to 1000 (evaluate_reputation, kept with the
    // hospitals' figures in ArrivalState).
    int reputation() const { return arrivals.hospitals[0].hospital_reputation; }
    // The bank statement, wages, interest and the hospital's value
    // (src/finance.cpp), and the advisor (src/advisor.cpp); the app starts
    // the advisor and runs its frames.
    Finance finance;
    Advisor advisor;
    // Patients arriving (src/peepmake.cpp).
    ArrivalState arrivals;
    // The land: plots for sale and their prices (src/town.cpp).
    TownLand land;
    // The player's policy, research and casebook figures, which the charts
    // show and change; the app starts it (start_hospital_settings). Prices
    // and reputations are in `arrivals`, the insurers in world.ins_control.
    HospitalSettings hospital;
    // The year-end and win/lose state (core/world/year_end.h): the score,
    // the criteria, this year's awards and trophies and the year-end league
    // tables.
    YearEnd progress;
    // The applicants for jobs (src/staff.cpp); the caller sets ref_counts
    // from the language text before start().
    StaffMarket staff_market;
    // The emergencies and earthquakes scheduled (core/world/emergency.h).
    Disasters disasters;
    // The player's command waiting to be carried out (player_events[player]).
    Event event;
    // Carries out `event` and clears it, as the game's process_turn does at
    // the start of every frame, paused or not: run it before the frame's
    // turns. Commands not ported yet are dropped.
    void process_turn();
    // The fax the last command carried out asks to show the player
    // (display_choice_screen's arguments, src/choice.cpp): its kind (kFax*,
    // 0 for none), its subject (choice_spare_data: a patient or an
    // illness) and the turns without the mouse moving after which it may
    // close itself (choice_time_out; 0: it must be answered). process_turn
    // clears it before each command; the interface opens the fax from it.
    struct ChoiceScreen {
        uint8_t type = 0;
        uint16_t subject = 0;
        int16_t timeout = 0;
    };
    ChoiceScreen choice_screen;

    // The fax screen as far as the colours go (core/world/fax_screen.h): the
    // game keeps display_mode DM_CHOICE and the fax picture's palette while a
    // fax is up, and setup_colors finds color.red and the rest in it.
    struct FaxScreen {
        // The fax is up (display_mode is DM_CHOICE), display_choice's state
        // (choice.state: 0x5a showing it, 0x5b up, 0x5c closing) and the turns
        // without the pointer moving after which it may close itself
        // (choice_time_out).
        bool open = false;
        uint8_t state = 0;
        int16_t timeout = 0;
        // The screen shown when this one closes (choice_backup): another fax,
        // with its timeout, or the game.
        bool backup_open = false;
        int16_t backup_timeout = 0;
        // othervar.fade_palette: the palette to take on at the end of the
        // frame, 0 for none, 1 the hospital's, 2 the fax picture's.
        uint8_t fade = 0;
        // The turn the pointer last moved (choice_actual_game_turn) and where
        // (choice_mouse_position, x + y).
        uint32_t since = 0;
        int32_t pointer = 0;
    };
    FaxScreen fax_screen;
    // The pointer's x + y (lbDisplay.mmouse_x + mmouse_y) and whether Escape
    // or Space was pressed since the last turn, which the interface sets while
    // a fax is up. The pointer starts off the corner, as the game's does when
    // a script plays it.
    int32_t fax_pointer = 1;
    bool fax_key = false;

    // What the player and the interface did since the level started, a line
    // each, for th_replay's --script (tools/replay/main.cpp): "<turn> <rest>",
    // the rest carried out by replay_input before turn <turn> runs, in order.
    // start() makes it a "#" line naming the level and the build
    // (snapshot_build_id, core/world/snapshot.h), then it records:
    //   <type> <value> <arg>          a command, as process_turn carries it out
    //                                 (Event::flags is not kept);
    //   @walker_add <cell>            add_walker;
    //   @walker_send <id> <cell>      send_walker;
    //   @cursor <cell>                update_placement, when it would look
    //                                 again (the pointer moved or a redraw);
    // and what the interface changes in place, outside commands, found by
    // comparing with what it was after the last turn, command or line above
    // (the first comparison after start() records all of it):
    //   @paused <0 or 1>              Game::paused;
    //   @path_finder <0 A*, 1 original>  World::path_finder;
    //   @tutorial <0 or 1>            advisor.flags' kAdvisorTutorial;
    //   @build_mouse <n>              Building::mouse;
    //   @placing <offset> <hex>       the bytes of World::placing that changed;
    //   @build <offset> <hex>         Building::build's;
    //   @hospital <offset> <hex>      Game::hospital's (start_hospital_settings).
    //   @ins_control <offset> <hex>   World::ins_control's (setup_insurances).
    // The @placing, @build, @hospital and @ins_control bytes are this build's
    // layout.
    std::vector<std::string> command_log;
    // Carries out one command_log line without its turn: sets `event` and
    // runs process_turn for a command, else as the line says. Returns false,
    // with the reason in `error`, for a line it does not know.
    bool replay_input(std::string_view line, std::string *error = nullptr);
    // update_placement (core/world/objects.h) on world.placing for the
    // pointer on map cell `cell`, recorded in command_log.
    void update_placement(int cell);

    // The events of the turns run and commands carried out
    // (core/world/trace.h): by default the last 500, for the debug report.
    // start() clears it.
    EventLog events;

    // The patients that reached the map this turn, to be set going.

    std::vector<uint16_t> new_patients;

    // Path finding tool: puts a test patient on `cell` that only walks where
    // it is sent (it has no action sequence). Returns its id, or 0 if there
    // was no free entry. Draws from the random generator, as making a
    // patient does. Recorded in command_log.
    uint16_t add_walker(int cell);
    // Path finding tool: sends person `id` to `cell`. A patient on an action
    // sequence leaves it and only walks from then on. Recorded in
    // command_log.
    void send_walker(uint16_t id, int cell);

    // The level's settings, its number and the animations it was started with.
    const Balance &settings() const { return settings_; }
    int level_number() const { return level_number_; }
    const AnimData &anims() const { return *anims_; }
    // The level's action sequences (start_rooms_in_use).
    const PersonComplexes &person_complexes() const { return *complexes_; }

  private:
    friend bool save_snapshot(const Game &game, std::vector<uint8_t> &out);
    friend bool load_snapshot(Game &game, std::span<const uint8_t> data, const GameData &gamedata, std::string *error);
    friend bool load_snapshot_impl(Game &game, std::span<const uint8_t> data, const GameData &gamedata,
                                   bool check_build, std::string *error);

    // What command_log compares to find the interface's changes.
    struct LoggedInputs {
        bool paused = false;
        PathFinder path_finder = PathFinder::AStar;
        bool tutorial = false;
        BuildMouse build_mouse = BuildMouse::None;
        ObjectPlacing placing;
        BuildState build;
        HospitalSettings hospital;
        std::array<uint16_t, 5> ins_control{};
    };

    // Points the world at what it reads and changes outside itself (World's
    // pointers): this game's settings, hospital 0's market, finances,
    // balance, date and hospital settings, and the action sequences.
    void connect_world();
    void carry_out(const Event &e);
    void log_line(std::string_view text);
    // Records the interface's changes since remember_inputs, then remembers.
    void log_inputs();
    void remember_inputs();

    const AnimData *anims_ = nullptr;
    const PersonComplexes *complexes_ = nullptr;
    const StaffComplexes *staff_complexes_ = nullptr;
    Balance settings_;
    std::unique_ptr<Level> level_;
    int level_number_ = 0;
    // The path finding tool's walkers, walked by walk_turn alone.
    std::vector<bool> walkers_;
    LoggedInputs logged_inputs_;
    // logged_inputs_ holds what was recorded; false from start() until the
    // first comparison.
    bool inputs_logged_ = false;
};

} // namespace th
