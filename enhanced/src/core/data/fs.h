#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string_view>
#include <vector>

namespace th
{

// Finds `rel` (components separated by '/') under `root`, matching each
// component case-insensitively: the game refers to "data/vblk-0.dat" while
// GOG's files are upper case. Returns the path as it exists on disk.
std::optional<std::filesystem::path> resolve_ci(const std::filesystem::path &root, std::string_view rel);

// Reads a whole file, or nothing if it cannot be opened.
std::optional<std::vector<uint8_t>> read_file(const std::filesystem::path &path);

} // namespace th
