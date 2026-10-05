#include "app/datadir.h"

#include "app/settings.h"
#include "core/data/gamedata.h"

#include <SDL3/SDL.h>

#include <system_error>

namespace th
{

DataSearch find_data_dir(const std::optional<std::filesystem::path> &argument, const Settings &settings)
{
    std::vector<std::pair<std::string, std::filesystem::path>> candidates;
    if (argument)
        candidates.emplace_back("--data", *argument);
#ifdef TH_DEV_DATA_DIR
    candidates.emplace_back("development folder", TH_DEV_DATA_DIR);
#endif
    if (std::string saved = settings.get("data_dir"); !saved.empty())
        candidates.emplace_back("saved setting", saved);
    if (const char *base = SDL_GetBasePath())
        candidates.emplace_back("next to the program", base);
    std::error_code ec;
    if (auto cwd = std::filesystem::current_path(ec); !ec)
        candidates.emplace_back("working directory", cwd);

    DataSearch search;
    for (auto &[source, path] : candidates) {
        DataCandidate c{source, path, missing_data_files(path)};
        bool usable = c.missing.empty();
        search.tried.push_back(std::move(c));
        if (usable) {
            search.found = path;
            break;
        }
    }
    return search;
}

} // namespace th
