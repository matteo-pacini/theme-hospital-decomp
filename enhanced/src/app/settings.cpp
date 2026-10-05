#include "app/settings.h"

#include <SDL3/SDL.h>

#include <fstream>

namespace th
{

void Settings::load()
{
    values_.clear();
    if (char *pref = SDL_GetPrefPath("ThemeHospitalDecomp", "Enhanced")) {
        path_ = std::filesystem::path(pref) / "settings.ini";
        SDL_free(pref);
    }
    if (path_.empty())
        return;
    std::ifstream in(path_);
    std::string line;
    while (std::getline(in, line)) {
        size_t eq = line.find('=');
        if (eq != std::string::npos && line[0] != '#')
            values_[line.substr(0, eq)] = line.substr(eq + 1);
    }
}

bool Settings::save() const
{
    if (path_.empty())
        return false;
    std::ofstream out(path_, std::ios::trunc);
    for (const auto &[key, value] : values_)
        out << key << '=' << value << '\n';
    return bool(out);
}

std::string Settings::get(const std::string &key) const
{
    auto it = values_.find(key);
    return it == values_.end() ? std::string{} : it->second;
}

} // namespace th
