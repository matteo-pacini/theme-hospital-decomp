#pragma once

#include <filesystem>
#include <map>
#include <string>

namespace th
{

// Key=value settings kept in settings.ini in the user's settings folder.
class Settings
{
  public:
    // Reads settings.ini in `dir`; a missing or unreadable file leaves them
    // empty.
    void load(const std::filesystem::path &dir);
    // Writes the settings file, making its folder. Returns false if it could
    // not be written.
    bool save() const;

    std::string get(const std::string &key) const;
    void set(const std::string &key, std::string value) { values_[key] = std::move(value); }

    const std::filesystem::path &path() const { return path_; }

  private:
    std::filesystem::path path_;
    std::map<std::string, std::string> values_;
};

} // namespace th
