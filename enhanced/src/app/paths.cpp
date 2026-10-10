#include "app/paths.h"

#include <cstdlib>
#include <system_error>

#ifdef _WIN32
#include <knownfolders.h>
#include <objbase.h>
#include <shlobj.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

namespace th
{

namespace
{

namespace fs = std::filesystem;

constexpr const char *kAppFolder = "ThemeHospitalEnhanced";

#ifdef _WIN32

fs::path known_folder(const KNOWNFOLDERID &id)
{
    PWSTR raw = nullptr;
    fs::path result;
    if (SUCCEEDED(SHGetKnownFolderPath(id, KF_FLAG_CREATE, nullptr, &raw)))
        result = raw;
    CoTaskMemFree(raw);
    return result;
}

UserPaths windows_paths(const fs::path &roaming, const fs::path &saved, const fs::path &local)
{
    if (roaming.empty() || saved.empty() || local.empty())
        return {};
    return {roaming / kAppFolder, saved / kAppFolder, local / kAppFolder / "game",
            local / kAppFolder / "debug-reports"};
}

#else

fs::path home_dir()
{
    if (const char *home = std::getenv("HOME"); home && *home)
        return home;
    if (const passwd *pw = getpwuid(getuid()); pw && pw->pw_dir)
        return pw->pw_dir;
    return {};
}

// An absolute $name, else `fallback` under the home folder.
fs::path xdg_dir(const char *name, const char *fallback)
{
    if (const char *value = std::getenv(name); value && *value) {
        fs::path path = value;
        if (path.is_absolute())
            return path;
    }
    fs::path home = home_dir();
    return home.empty() ? fs::path() : home / fallback;
}

#endif

fs::path old_prefs_under(const fs::path &base)
{
    return base.empty() ? fs::path() : base / "ThemeHospitalDecomp" / "Enhanced";
}

int copy_if_missing(const fs::path &from, const fs::path &to)
{
    std::error_code ec;
    if (!fs::is_regular_file(from, ec) || fs::exists(to, ec))
        return 0;
    fs::create_directories(to.parent_path(), ec);
    return fs::copy_file(from, to, fs::copy_options::none, ec) ? 1 : 0;
}

} // namespace

UserPaths user_paths()
{
#if defined(_WIN32)
    return windows_paths(known_folder(FOLDERID_RoamingAppData), known_folder(FOLDERID_SavedGames),
                         known_folder(FOLDERID_LocalAppData));
#elif defined(__APPLE__)
    // The folder SDL_GetPrefPath("", name) gives: SDL3 accepts an empty
    // organisation and leaves it out of the path.
    fs::path home = home_dir();
    if (home.empty())
        return {};
    fs::path base = home / "Library" / "Application Support" / kAppFolder;
    return {base, base / "save", base / "game", base / "debug-reports"};
#else
    fs::path config = xdg_dir("XDG_CONFIG_HOME", ".config");
    fs::path data = xdg_dir("XDG_DATA_HOME", ".local/share");
    fs::path state = xdg_dir("XDG_STATE_HOME", ".local/state");
    if (config.empty() || data.empty() || state.empty())
        return {};
    return {config / kAppFolder, data / kAppFolder / "save", data / kAppFolder / "game",
            state / kAppFolder / "debug-reports"};
#endif
}

fs::path old_prefs_dir()
{
#if defined(_WIN32)
    return old_prefs_under(known_folder(FOLDERID_RoamingAppData));
#elif defined(__APPLE__)
    fs::path home = home_dir();
    return old_prefs_under(home.empty() ? fs::path() : home / "Library" / "Application Support");
#else
    return old_prefs_under(xdg_dir("XDG_DATA_HOME", ".local/share"));
#endif
}

int migrate_old_prefs(const UserPaths &paths)
{
    fs::path old = old_prefs_dir();
    if (old.empty() || paths.settings_dir.empty())
        return 0;
    int copied = copy_if_missing(old / "settings.ini", paths.settings_dir / "settings.ini");
    std::error_code ec;
    for (fs::recursive_directory_iterator it(old / "save", ec), end; !ec && it != end; it.increment(ec))
        copied += copy_if_missing(it->path(), paths.save_dir / fs::relative(it->path(), old / "save"));
    return copied;
}

int migrate_hiscore(const UserPaths &paths, const fs::path &data_dir)
{
    if (paths.save_dir.empty())
        return 0;
    return copy_if_missing(data_dir / "save" / "hiscore.dat", paths.save_dir / "hiscore.dat");
}

} // namespace th
