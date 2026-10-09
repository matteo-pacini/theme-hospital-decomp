#pragma once

#include "app/datadir.h"
#include "app/ui/nav_tool.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace th
{

enum class MissingDataAction { None, Choose, Quit };

// Full-window screen shown when no Theme Hospital data was found: what is
// needed, where it looked, and a way to pick the folder. `error` is the
// problem with the last folder chosen, if any.
MissingDataAction draw_missing_data_screen(const DataSearch &search, const std::string &error);

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
