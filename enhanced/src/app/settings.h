#pragma once

#include <filesystem>
#include <map>
#include <string>

namespace th
{

// Key=value settings kept in the user's preferences folder (SDL_GetPrefPath).
class Settings
{
  public:
    // Reads the settings file; a missing or unreadable file leaves them empty.
    void load();
    // Writes the settings file. Returns false if it could not be written.
    bool save() const;

    std::string get(const std::string &key) const;
    void set(const std::string &key, std::string value) { values_[key] = std::move(value); }

    const std::filesystem::path &path() const { return path_; }

  private:
    std::filesystem::path path_;
    std::map<std::string, std::string> values_;
};

} // namespace th
