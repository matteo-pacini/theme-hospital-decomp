#pragma once

#include "app/datadir.h"
#include "app/ui/nav_tool.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace th
{

enum class SetupAction { None, InstallFromInstaller, InstallFromFolder, Verify, Cancel, Quit };

// What the setup screen offers for the upscaling built after an install: on
// or off, the algorithm (an index into scalers()) and the factor.
struct UpscaleChoice {
    bool on = false;
    int algorithm = 0;
    int factor = 4;
};

// What the setup screen shows.
struct SetupView {
    const DataSearch &search;
    // The last failure (an install's error text) and the last verification's
    // result; either may be empty.
    const std::string &error;
    const std::string &notice;
    // The game folder, whether it exists, and whether it holds an install
    // that is not complete.
    std::string game_dir;
    bool game_dir_exists = false;
    bool game_dir_incomplete = false;
    // A job on the worker thread: whether it verifies, and its progress.
    bool working = false;
    bool verifying = false;
    uint64_t done = 0;
    uint64_t total = 0;
    // The upscaling choice the screen edits, and whether the job is past
    // the install and building the upscaled sprites.
    UpscaleChoice *upscale = nullptr;
    bool upscaling = false;
};

// Full-window setup screen shown when no Theme Hospital data was found: what
// is needed, where it looked, and the ways to install it from GOG's installer
// or a game folder. While a job runs it shows the progress and Cancel.
SetupAction draw_setup_screen(const SetupView &view);

// The debug report tool: saves a screenshot of the game, its
// state and a note, for agents to review.
struct DebugReport {
    std::array<char, 4096> note{};
    // Save was pressed this frame; the caller saves and clears it.
    bool save = false;
    // Where the last report went, or why it failed.
    std::string last;
};

// The inspector: a person or object picked by clicking it in the hospital
// view, its fields read live each frame.
struct InspectTool {
    uint16_t thing = 0;   // the thing inspected, 0 for none
    bool picking = false; // the next click on the view picks the thing
};

// What the F10 quick report's note box asked for this frame.
enum class QuickNoteAction { None, Save, Discard };

// The F10 quick report's note box: a text field, focused on the first frame
// (`focus` is cleared once set). Enter saves, Shift+Enter adds a new line and
// Escape discards.
QuickNoteAction draw_quick_report_note(std::array<char, 4096> &note, bool &focus);

// A short confirmation message, drawn over everything else.
void draw_toast(const std::string &text);

// State of the tools window (F12).
struct ToolsState {
    bool open = false;
    int ui_scale = 0; // panel and pointer scale: 0 automatic, else the integer scale
    InspectTool inspect;
    NavTool nav;
    DebugReport report;
};

// The tools window; with `world` (a level in play), also the inspector and
// the path finding tool.
void draw_tools(ToolsState &state, const World *world);

} // namespace th
