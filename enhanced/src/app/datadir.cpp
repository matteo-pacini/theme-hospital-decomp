#include "app/datadir.h"

#include "core/data/gamedata.h"

#include <system_error>

namespace th
{

DataSearch find_data_dir(const std::optional<std::filesystem::path> &argument, [[maybe_unused]] bool skip_dev_folder,
                         const std::filesystem::path &game_dir)
{
    DataSearch search;
    auto consider = [&](const char *source, const std::filesystem::path &path, bool installed, bool dev = false) {
        if (search.found)
            return;
        DataCandidate c{source, path, missing_data_files(path)};
        if (!installed)
            c.missing.insert(c.missing.begin(), "install.ok");
        if (c.missing.empty())
            search.found = path;
        search.from_dev_folder = search.found && dev;
        search.tried.push_back(std::move(c));
    };
    if (argument)
        consider("--data", *argument, true);
#ifdef TH_DEV_DATA_DIR
    if (!skip_dev_folder)
        consider("development folder", TH_DEV_DATA_DIR, true, true);
#endif
    if (!game_dir.empty()) {
        std::error_code ec;
        consider("game folder", game_dir, std::filesystem::exists(game_dir / "install.ok", ec));
    }
    return search;
}

} // namespace th
