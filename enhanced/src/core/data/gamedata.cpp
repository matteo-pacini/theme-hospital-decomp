#include "core/data/gamedata.h"

#include "core/data/fs.h"
#include "core/data/rnc.h"

namespace th
{

const std::vector<std::string_view> &required_data_files()
{
    static const std::vector<std::string_view> files = {
        "data/mpalette.dat", "data/vblk-0.tab",   "data/vblk-0.dat",   "data/panel02v.tab", "data/panel02v.dat",
        "data/panel04v.tab", "data/panel04v.dat", "data/vspr-0.tab",   "data/vspr-0.dat",   "data/vstart-1.ani",
        "data/vfra-1.ani",   "data/vele-1.ani",   "data/vlist-1.ani",  "qdata/ghost66.dat", "data/mpointer.tab",
        "data/mpointer.dat", "qdata/font05v.tab", "qdata/font05v.dat", "qdata/font16v.tab", "qdata/font16v.dat",
        "data/watch01v.tab", "data/watch01v.dat", "data/p_comp.lex",   "data/s_comp.lex",   "levels/level.l1",
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

bool GameData::load(const std::filesystem::path &dir, std::string *error, std::optional<int> language)
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
    if (!load_sheet(dir, "data/vblk-0", SpriteFormat::Rle, blocks_, error) ||
        !load_sheet(dir, "data/panel02v", SpriteFormat::Req, panel_, error) ||
        !load_sheet(dir, "data/panel04v", SpriteFormat::Req, rat_panel_, error) ||
        !load_sheet(dir, "data/vspr-0", SpriteFormat::Rle, sprites_, error) ||
        !load_sheet(dir, "data/mpointer", SpriteFormat::Rle, pointers_, error) ||
        !load_sheet(dir, "data/watch01v", SpriteFormat::Req, watch_, error) ||
        !load_sheet(dir, "data/money01v", SpriteFormat::Rle, money_, error) || !anims_.load(dir, error))
        return false;
    auto ghost = load_data_file(dir, "qdata/ghost66.dat", error);
    if (!ghost)
        return false;
    if (ghost->size() < 0x10000) {
        if (error)
            *error = "qdata/ghost66.dat: too short for a blend table";
        return false;
    }
    ghost_ = std::move(*ghost);
    anims_.sprite_heights.clear();
    for (size_t i = 0; i < sprites_.size(); i++)
        anims_.sprite_heights.push_back(uint8_t(sprites_.at(i).height));
    if (!load_person_complexes(dir, person_complexes_, error))
        return false;
    if (!load_staff_complexes(dir, staff_complexes_, error))
        return false;
    language_ = language ? *language : config_language(dir).value_or(0);
    if (!load_language(dir, language_, text_, error))
        return false;
    fonts_.clear();
    for (int slot : {0, 5, 6, 0x1d, 0x29})
        if (!load_font(dir, slot, fonts_[slot], error))
            return false;
    return true;
}

void GameData::attach_hires(HiresSheet blocks, HiresSheet sprites, HiresSheet money)
{
    blocks_.attach_hires(std::move(blocks));
    sprites_.attach_hires(std::move(sprites));
    money_.attach_hires(std::move(money));
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
