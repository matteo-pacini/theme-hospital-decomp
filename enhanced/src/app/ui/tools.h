#pragma once

#include "app/datadir.h"

#include <string>
#include <vector>

namespace th
{

enum class MissingDataAction { None, Choose, Quit };

// Full-window screen shown when no Theme Hospital data was found: what is
// needed, where it looked, and a way to pick the folder. `error` is the
// problem with the last folder chosen, if any.
MissingDataAction draw_missing_data_screen(const DataSearch &search, const std::string &error);

// State of the tools window (F1).
struct ToolsState {
    bool open = false;
    int ui_scale = 0; // panel and pointer scale: 0 automatic, else the integer scale
};

void draw_tools(ToolsState &state);

} // namespace th
