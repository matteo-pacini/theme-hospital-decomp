#pragma once

#include <filesystem>
#include <optional>

namespace th
{

// The per-user folders (none is created by the lookup).
struct UserPaths {
    std::filesystem::path settings_dir; // settings.ini
    std::filesystem::path save_dir;     // saved games and hiscore.dat
    std::filesystem::path game_dir;     // the installed game data
    std::filesystem::path reports_dir;  // debug reports
    std::filesystem::path upscale_dir;  // the upscaled sprites (a cache)
};

// The folders for the running user, from the platform's conventions: XDG on
// Linux (an unset or relative XDG_*_HOME falls back to its default under
// $HOME), Application Support on macOS, the known folders on Windows. All
// are empty when the user's home cannot be found.
UserPaths user_paths();

// The folder the settings and saved games were kept in before the per-user
// folders (SDL_GetPrefPath("ThemeHospitalDecomp", "Enhanced")).
std::filesystem::path old_prefs_dir();

// Copies settings.ini and the files of save/ from old_prefs_dir() to the new
// folders when missing there. Never overwrites or deletes. Returns how many
// files it copied.
int migrate_old_prefs(const UserPaths &paths);

// Copies save/hiscore.dat of `data_dir` to the save folder when missing
// there. Returns how many files it copied (0 or 1).
int migrate_hiscore(const UserPaths &paths, const std::filesystem::path &data_dir);

} // namespace th
