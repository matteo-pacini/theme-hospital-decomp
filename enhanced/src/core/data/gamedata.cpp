#include "core/data/gamedata.h"

#include "core/data/fs.h"
#include "core/data/rnc.h"

namespace th
{

const std::vector<std::string_view> &required_data_files()
{
    static const std::vector<std::string_view> files = {
        "data/mpalette.dat", "data/vblk-0.tab",   "data/vblk-0.dat",
        "data/panel02v.tab", "data/panel02v.dat", "levels/level.l1",
    };
    return files;
}

std::vector<std::string> missing_data_files(const std::filesystem::path &dir)
{
    std::vector<std::string> missing;
    for (std::string_view rel : required_data_files())
        if (!resolve_ci(dir, rel))
            missing.emplace_back(rel);
    return missing;
}

std::optional<std::vector<uint8_t>> load_data_file(const std::filesystem::path &dir, std::string_view rel,
                                                   std::string *error)
{
    auto path = resolve_ci(dir, rel);
    if (!path) {
        if (error)
            *error = std::string(rel) + ": not found";
        return std::nullopt;
    }
    auto bytes = read_file(*path);
    if (!bytes) {
        if (error)
            *error = path->string() + ": cannot be read";
        return std::nullopt;
    }
    if (!rnc_is_packed(*bytes))
        return bytes;
    std::string why;
    auto unpacked = rnc_unpack(*bytes, &why);
    if (!unpacked && error)
        *error = path->string() + ": " + why;
    return unpacked;
}

namespace
{

bool load_sheet(const std::filesystem::path &dir, std::string_view base, SpriteFormat format, SpriteSheet &sheet,
                std::string *error)
{
    std::string tab_name = std::string(base) + ".tab";
    std::string dat_name = std::string(base) + ".dat";
    auto tab = load_data_file(dir, tab_name, error);
    if (!tab)
        return false;
    auto dat = load_data_file(dir, dat_name, error);
    if (!dat)
        return false;
    if (!sheet.load(std::move(*tab), std::move(*dat), format)) {
        if (error)
            *error = tab_name + ": not a sprite table";
        return false;
    }
    return true;
}

} // namespace

bool GameData::load(const std::filesystem::path &dir, std::string *error)
{
    dir_ = dir;
    auto pal = load_data_file(dir, "data/mpalette.dat", error);
    if (!pal)
        return false;
    if (!palette_from_vga(*pal, palette_)) {
        if (error)
            *error = "data/mpalette.dat: too short for a palette";
        return false;
    }
    return load_sheet(dir, "data/vblk-0", SpriteFormat::Rle, blocks_, error) &&
           load_sheet(dir, "data/panel02v", SpriteFormat::Req, panel_, error);
}

std::optional<Level> GameData::load_level(int number, std::string *error) const
{
    std::string rel = "levels/level.l" + std::to_string(number);
    auto bytes = load_data_file(dir_, rel, error);
    if (!bytes)
        return std::nullopt;
    Level level;
    if (!parse_level(*bytes, level)) {
        if (error)
            *error = rel + ": too short for a level";
        return std::nullopt;
    }
    return level;
}

} // namespace th
