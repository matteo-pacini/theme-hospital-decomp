#pragma once

#include "app/datadir.h"

#include <string>
#include <vector>

struct SDL_Renderer;
struct SDL_Texture;

namespace th
{

class GameData;

enum class MissingDataAction { None, Choose, Quit };

// Full-window screen shown when no Theme Hospital data was found: what is
// needed, where it looked, and a way to pick the folder. `error` is the
// problem with the last folder chosen, if any.
MissingDataAction draw_missing_data_screen(const DataSearch &search, const std::string &error);

// State of the developer tools window (F1).
struct ToolsState {
    bool open = true;
    int level = 1; // selected level number
    bool level_requested = false;
    int zoom = 0;  // 0 automatic, else the integer scale
    int sheet = 0; // 0 blocks, 1 panel
    int sprite = 0;
    SDL_Texture *preview = nullptr; // sprite preview, created on first use
};

// What the tools window shows about the current view.
struct ViewInfo {
    int window_w, window_h; // output size in pixels
    int view_w, view_h;     // indexed surface size
    int zoom;
    int camera_x, camera_y;
    int cell_col, cell_row; // cell under the mouse
    float frame_ms;
    std::vector<int> levels; // level numbers present in the data
    std::string level_error;
};

void draw_tools(ToolsState &state, const GameData &data, const ViewInfo &info, SDL_Renderer *renderer);

} // namespace th
