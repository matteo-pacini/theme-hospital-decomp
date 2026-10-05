#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace th
{

class Settings;

// A folder looked at for the game data, and why it was rejected.
struct DataCandidate {
    std::string source; // where the candidate came from
    std::filesystem::path path;
    std::vector<std::string> missing; // required files not found; empty if usable
};

struct DataSearch {
    std::optional<std::filesystem::path> found;
    std::vector<DataCandidate> tried;
};

// Looks for the Theme Hospital data, in order: the --data argument, the
// development folder (TH_DEV_DATA_DIR, orig/ in the repository unless the
// build sets it empty), the folder saved in the settings, then next to the
// executable and the working directory. The first folder holding every
// required file wins.
DataSearch find_data_dir(const std::optional<std::filesystem::path> &argument, const Settings &settings);

} // namespace th
