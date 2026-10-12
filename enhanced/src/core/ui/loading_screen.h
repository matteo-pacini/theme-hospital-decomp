#pragma once

#include "core/data/palette.h"
#include "core/render/surface.h"

#include <filesystem>
#include <string>

namespace th
{

// The loading screen: LOAD01V's picture with its palette (display_loading_
// screen, src/screens.cpp:251, shows it with nothing drawn over it).
struct LoadingScreen {
    Surface pixels;
    Palette palette{};
};

// Loads qdata/load01v.dat and .pal from the game data folder `dir`, or says
// why not in `error`.
bool load_loading_screen(const std::filesystem::path &dir, LoadingScreen &out, std::string *error = nullptr);

} // namespace th
