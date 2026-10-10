#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace th
{

// A folder looked at for the game data, and why it was rejected.
struct DataCandidate {
    std::string source; // where the candidate came from
    std::filesystem::path path;
    std::vector<std::string> missing; // required files not found; empty if usable
};

struct DataSearch {
    std::optional<std::filesystem::path> found;
    std::vector<DataCandidate> tried;
    bool from_dev_folder = false; // `found` is the development folder
};

// Looks for the Theme Hospital data: the --data argument first, then the
// development folder (TH_DEV_DATA_DIR, orig/ in the repository unless the build
// sets it empty) unless `skip_dev_folder`, then `game_dir`, which counts only
// with the install.ok marker an install leaves. The first folder holding
// every required file wins.
DataSearch find_data_dir(const std::optional<std::filesystem::path> &argument, bool skip_dev_folder,
                         const std::filesystem::path &game_dir);

} // namespace th
