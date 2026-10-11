// The per-user folders and the migration from the old preferences folder
// (the XDG layout, so Linux only).

#include "common.h"

#include "app/paths.h"

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__EMSCRIPTEN__)

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

namespace
{

namespace fs = std::filesystem;

// Sets (or, with a null value, removes) an environment variable and puts the
// old value back on destruction.
class EnvVar
{
  public:
    EnvVar(const char *name, const char *value) : name_(name)
    {
        if (const char *old = std::getenv(name))
            old_ = old;
        set(value);
    }
    ~EnvVar() { set(old_ ? old_->c_str() : nullptr); }
    EnvVar(const EnvVar &) = delete;
    EnvVar &operator=(const EnvVar &) = delete;

  private:
    void set(const char *value) const
    {
        if (value)
            setenv(name_, value, 1);
        else
            unsetenv(name_);
    }
    const char *name_;
    std::optional<std::string> old_;
};

void write_file(const fs::path &path, const std::string &text)
{
    fs::create_directories(path.parent_path());
    std::ofstream(path) << text;
}

std::string read_file(const fs::path &path)
{
    std::ifstream in(path);
    return std::string(std::istreambuf_iterator<char>(in), {});
}

} // namespace

void test_user_paths()
{
    fs::path home = fs::temp_directory_path() / "th_test_paths_home";
    fs::remove_all(home);
    EnvVar h("HOME", home.c_str());
    EnvVar config("XDG_CONFIG_HOME", nullptr), data("XDG_DATA_HOME", nullptr), state("XDG_STATE_HOME", nullptr);

    th::UserPaths p = th::user_paths();
    CHECK(p.settings_dir == home / ".config/ThemeHospitalEnhanced");
    CHECK(p.save_dir == home / ".local/share/ThemeHospitalEnhanced/save");
    CHECK(p.game_dir == home / ".local/share/ThemeHospitalEnhanced/game");
    CHECK(p.reports_dir == home / ".local/state/ThemeHospitalEnhanced/debug-reports");
    CHECK(th::old_prefs_dir() == home / ".local/share/ThemeHospitalDecomp/Enhanced");

    setenv("XDG_CONFIG_HOME", (home / "c").c_str(), 1);
    setenv("XDG_DATA_HOME", (home / "d").c_str(), 1);
    setenv("XDG_STATE_HOME", (home / "s").c_str(), 1);
    p = th::user_paths();
    CHECK(p.settings_dir == home / "c/ThemeHospitalEnhanced");
    CHECK(p.save_dir == home / "d/ThemeHospitalEnhanced/save");
    CHECK(p.game_dir == home / "d/ThemeHospitalEnhanced/game");
    CHECK(p.reports_dir == home / "s/ThemeHospitalEnhanced/debug-reports");
    CHECK(th::old_prefs_dir() == home / "d/ThemeHospitalDecomp/Enhanced");

    // A relative value is ignored, as the XDG specification says.
    setenv("XDG_CONFIG_HOME", "relative/dir", 1);
    setenv("XDG_DATA_HOME", "", 1);
    p = th::user_paths();
    CHECK(p.settings_dir == home / ".config/ThemeHospitalEnhanced");
    CHECK(p.save_dir == home / ".local/share/ThemeHospitalEnhanced/save");
    fs::remove_all(home);
}

void test_migrate_old_prefs()
{
    fs::path home = fs::temp_directory_path() / "th_test_migrate_home";
    fs::remove_all(home);
    EnvVar h("HOME", home.c_str());
    EnvVar config("XDG_CONFIG_HOME", nullptr), data("XDG_DATA_HOME", nullptr), state("XDG_STATE_HOME", nullptr);
    th::UserPaths p = th::user_paths();
    fs::path old = th::old_prefs_dir();

    write_file(old / "settings.ini", "language=3\n");
    write_file(old / "save/slot1.sav", "one");
    write_file(old / "save/sub/slot2.sav", "two");
    fs::path game = home / "game_source";
    write_file(game / "save/hiscore.dat", "scores");

    CHECK(th::migrate_old_prefs(p) == 3);
    CHECK(read_file(p.settings_dir / "settings.ini") == "language=3\n");
    CHECK(read_file(p.save_dir / "slot1.sav") == "one");
    CHECK(read_file(p.save_dir / "sub/slot2.sav") == "two");
    CHECK(th::migrate_hiscore(p, game) == 1);
    CHECK(read_file(p.save_dir / "hiscore.dat") == "scores");

    // The old files stay, and a second start copies nothing.
    CHECK(fs::exists(old / "settings.ini") && fs::exists(old / "save/slot1.sav"));
    CHECK(fs::exists(game / "save/hiscore.dat"));
    CHECK(th::migrate_old_prefs(p) == 0);
    CHECK(th::migrate_hiscore(p, game) == 0);

    // Files already in the new folders are never overwritten.
    write_file(p.settings_dir / "settings.ini", "language=1\n");
    write_file(game / "save/hiscore.dat", "newer");
    CHECK(th::migrate_old_prefs(p) == 0);
    CHECK(th::migrate_hiscore(p, game) == 0);
    CHECK(read_file(p.settings_dir / "settings.ini") == "language=1\n");
    CHECK(read_file(p.save_dir / "hiscore.dat") == "scores");
    fs::remove_all(home);
}

#else

void test_user_paths()
{
}
void test_migrate_old_prefs()
{
}

#endif
