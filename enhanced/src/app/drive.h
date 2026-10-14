#pragma once

#include <cstdint>
#include <deque>
#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace th
{

// The modifiers a `key` command may prefix (Shift+Return, Shift+F10). The
// host maps them to its own key-event modifiers; the tests need no SDL.
enum DriveMod : uint16_t {
    kDriveModShift = 1,
    kDriveModCtrl = 2,
    kDriveModAlt = 4,
};

// What the driver asks of the app. Screen coordinates are the game's 640 x 480
// ones of whatever is shown: the interface and panel in the hospital, or a
// full-screen screen (charts, town map, fax, start-up). Each move returns
// false when the target is not on the screen.
class DriveHost
{
  public:
    virtual ~DriveHost() = default;
    virtual bool drive_move_screen(int x, int y) = 0;
    virtual bool drive_move_cell(int col, int row) = 0;
    virtual bool drive_move_thing(uint16_t id) = 0;
    // `button` 1 left, 3 right (SDL's numbering).
    virtual void drive_button(int button, bool down) = 0;
    // `name` as SDL_GetKeyFromName reads it (Escape, Return, F1, a, ...),
    // with the DriveMod modifiers of a `key` command.
    virtual bool drive_key(std::string_view name, bool down, uint16_t mods = 0) = 0;
    // Text as a keyboard would type it, for the quick report's note box.
    virtual bool drive_text(std::string_view text) = 0;
    // Saves the frame drawn next as a PNG at `path`.
    virtual void drive_screenshot(const std::filesystem::path &path) = 0;
    // The same, after the ImGui layer, so the tools and the note box show.
    virtual void drive_screenshot_ui(const std::filesystem::path &path) = 0;
    // The debug report's state text (debug_state).
    virtual std::string drive_state() const = 0;
    // The last debug report written this run (its folder), empty if none.
    virtual std::string drive_report_dir() const = 0;
    // The game's turn count, or -1 with no level loaded.
    virtual int64_t drive_turn() const = 0;
    // Opens the staff pay-raise requester (req12) for thing `id`, as
    // answering a resignation call does. A test hook: the window otherwise
    // needs a member of staff to resign (about 9000 turns and discontent).
    virtual bool drive_staff_demand(uint16_t id) = 0;
    // Makes the level won, as the year end does: the win letter follows. A
    // test hook: a win needs the level's criteria met at a year end.
    virtual bool drive_win_level() = 0;
    // Loses the level now, as a failed criterion does: the losing letter
    // follows. A test hook.
    virtual bool drive_lose_level() = 0;
    // Starts an emergency (`epidemic` false; its patients come from the
    // helicopter), or covers up an epidemic with every patient contagious,
    // so the panel's watch has patients to cycle through. A test hook: both
    // need a long, built-up game.
    virtual bool drive_watch(bool epidemic) = 0;
    // Turns the mouse wheel `notches` (positive away from the user).
    virtual bool drive_wheel(int notches) = 0;
};

// Waits for one of `max` slots in `dir` (enhanced/tools/slot.sh's files,
// build/slots/POOL) and holds it until the process exits, so that driven
// runs started by several agents stay within a limit. `info` is shown by
// `slot.sh --status`. Returns false, without waiting, if the slot files
// cannot be made.
bool hold_slot(const std::filesystem::path &dir, int max, const std::string &info);

// Runs the commands of a drive script (--drive) or of a controlling process
// (--control) against the app, one step per frame, so that agents and tests
// can click through the real interface. Lines are commands, '#' starts a
// comment:
//
//   wait N              let N frames pass
//   turns N             let frames pass until N more game turns have run
//   move X Y            the pointer to screen point (X, Y)
//   cell C R            the pointer to the middle of map cell (C, R)
//   thing ID            the pointer onto thing ID
//   down [left|right]   press a mouse button where the pointer is
//   up [left|right]     release it
//   wheel N             turn the mouse wheel N notches (negative toward the
//                       user)
//   click X Y [button]  move, a frame, press, 4 frames, release, 2 frames
//   click-cell C R [b]  the same on a map cell
//   click-thing ID [b]  the same on a thing
//   key [MOD+]NAME      press a key, 4 frames, release, 2 frames; MOD is
//                       Shift, Ctrl or Alt (Shift+Return, Shift+F10)
//                       (NAME as SDL names it, spaces written as _: Keypad_8)
//   keydown [MOD+]NAME  press a key and keep it down (W, Tab), 2 frames
//   keyup NAME          release it, 2 frames
//   type TEXT           the characters TEXT, as a keyboard types them
//   demand ID           open the staff pay-raise requester for thing ID (a
//                       test hook; the window normally needs a resignation)
//   win                 win the level now (a test hook; the letter follows)
//   lose                lose the level now (a test hook; the letter follows)
//   watch emergency|epidemic
//                       start an emergency, or an epidemic's cover-up with
//                       every patient contagious (a test hook)
//   screenshot NAME     save the next frame as OUT/NAME.png (the game, no
//                       tools)
//   screenshot-ui NAME  the same after the tools and note box are drawn
//   dump NAME           save the state text as OUT/NAME.txt
//   expect TEXT         fail unless the state text contains TEXT
//   expect-not TEXT     fail if it does
//   expect-report TEXT  fail unless the last debug report's note.txt contains
//                       TEXT (the note alone when TEXT is empty)
//   expect-report-file FILE TEXT
//                       fail unless the last debug report's FILE (state.txt,
//                       system.txt, ...) contains TEXT
//   expect-report-none  fail unless no debug report was written
//   expect-report-shot FILE
//                       fail unless the last report's screenshot.png equals
//                       OUT/FILE.png byte for byte
//   echo TEXT           print TEXT
//   quit                stop
//
// Each command answers one line, "ok ...", "fail ..." or a path, once its
// frames have run: on stdout with --control, and failures on stderr with
// --drive.
class Driver
{
  public:
    Driver(std::filesystem::path out, bool control);

    // Adds a command line; safe to call from another thread.
    void add_line(std::string line);
    // Marks the end of the input: the driver stops once it has run all of it.
    void end_input();

    // Runs the commands due at the start of a frame; returns whether the
    // app should run the frame (a command is waiting for it).
    bool frame(DriveHost &host);

    // Stopped by quit or by the end of its input.
    bool finished() const;
    // Prints the answer still held for the last command (see defer).
    void flush();
    int failures() const { return failures_; }

  private:
    // A command and the line it came from.
    struct Command {
        std::string line;
        std::vector<std::string> words;
    };

    bool next_command(Command &out);
    // Runs one command; returns the frames it needs before the next.
    int run(DriveHost &host, const Command &c);
    int run_pointer(DriveHost &host, const Command &c);
    int run_hook(DriveHost &host, const Command &c);
    int run_click(DriveHost &host, const Command &c);
    int run_key_hold(DriveHost &host, const Command &c);
    int run_output(DriveHost &host, const Command &c);
    // The expect-report commands; true when `c` is one.
    bool run_report(DriveHost &host, const Command &c, const std::string &text);
    void reply(std::string text, bool failed = false);
    // Holds a command's answer until its frames have run, so a controller
    // reading it finds the screenshot written and the state current.
    void defer(std::string text, bool failed = false);

    std::filesystem::path out_;
    bool control_ = false;
    mutable std::mutex mutex_;
    std::deque<std::string> input_;
    bool input_ended_ = false;
    // Steps the running command still has, before the queued input.
    std::deque<Command> pending_;
    int wait_frames_ = 0;
    int64_t until_turn_ = -1;
    int turn_wait_frames_ = 0;
    bool quit_ = false;
    int failures_ = 0;
    std::string deferred_;
    bool deferred_failed_ = false;
};

} // namespace th
