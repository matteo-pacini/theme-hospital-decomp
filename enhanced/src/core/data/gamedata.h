#pragma once

#include "core/data/level.h"
#include "core/data/palette.h"
#include "core/data/sprites.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace th
{

// Files a folder must hold to be used as the game data, relative to it
// (matched case-insensitively).
const std::vector<std::string_view> &required_data_files();

// The required files missing from `dir`; empty if it holds the game data.
std::vector<std::string> missing_data_files(const std::filesystem::path &dir);

// Reads a game file relative to `dir`, unpacking it if RNC-compressed.
std::optional<std::vector<uint8_t>> load_data_file(const std::filesystem::path &dir, std::string_view rel,
                                                   std::string *error = nullptr);

// The game data the engine uses, loaded from an installed copy of the game.
class GameData
{
  public:
    // Loads everything from `dir`. On failure says why in `error`.
    bool load(const std::filesystem::path &dir, std::string *error);

    // Loads levels/level.l<number>.
    std::optional<Level> load_level(int number, std::string *error = nullptr) const;

    const std::filesystem::path &dir() const { return dir_; }
    const Palette &palette() const { return palette_; }
    const SpriteSheet &blocks() const { return blocks_; }
    const SpriteSheet &panel() const { return panel_; }

  private:
    std::filesystem::path dir_;
    Palette palette_{};
    SpriteSheet blocks_;
    SpriteSheet panel_;
};

} // namespace th
