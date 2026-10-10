#pragma once

#include "core/data/anims.h"
#include "core/data/level.h"
#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/data/text.h"
#include "core/world/complex.h"
#include "core/world/staff_complex.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace th
{

// Files a folder must hold to be used as the game data, relative to it
// (matched case-insensitively).
const std::vector<std::string_view> &required_data_files();

// The required files missing from `dir`; empty if it holds the game data.
std::vector<std::string> missing_data_files(const std::filesystem::path &dir);

// Reads a game file relative to `dir`, unpacking it if RNC-compressed.
std::optional<std::vector<uint8_t>> load_data_file(const std::filesystem::path &dir, std::string_view rel,
                                                   std::string *error = nullptr);

// The game data the engine uses, loaded from an installed copy of the game.
class GameData
{
  public:
    // Loads everything from `dir`, the text in `language` (LANG-n.DAT; by
    // default HOSPITAL.CFG's, else English). On failure says why in `error`.
    bool load(const std::filesystem::path &dir, std::string *error, std::optional<int> language = std::nullopt);

    // Loads levels/level.l<number>.
    std::optional<Level> load_level(int number, std::string *error = nullptr) const;

    const std::filesystem::path &dir() const { return dir_; }
    const Palette &palette() const { return palette_; }
    const SpriteSheet &blocks() const { return blocks_; }
    const SpriteSheet &panel() const { return panel_; }
    // DATA/PANEL04V: the rat game's panel.
    const SpriteSheet &rat_panel() const { return rat_panel_; }
    const SpriteSheet &sprites() const { return sprites_; }
    const SpriteSheet &pointers() const { return pointers_; }
    const SpriteSheet &watch() const { return watch_; }
    // DATA/MONEY01V: the money bag and its digit sprites
    // (mapwho_numfx's draw_money).
    const SpriteSheet &money() const { return money_; }
    const AnimData &anims() const { return anims_; }
    // The patients' action sequences (DATA/P_COMP.LEX).
    const PersonComplexes &person_complexes() const { return person_complexes_; }
    // The staff's action sequences (DATA/S_COMP.LEX).
    const StaffComplexes &staff_complexes() const { return staff_complexes_; }
    // 256 x 256 blend table for see-through drawing (QDATA/GHOST66.DAT, the in-game table):
    // entry [a * 256 + b] mixes colour b into a.
    const std::vector<uint8_t> &ghost() const { return ghost_; }
    const LanguageText &text() const { return text_; }
    int language() const { return language_; }
    // The font in cache slot `slot` (set_font_from_cache); only the slots the
    // engine draws with are loaded: 0 (map text), 5 (panel balance), 6 (panel
    // date), 0x1d (advisor) and 0x29 (the rat game's score).
    const Font &font(int slot) const { return fonts_.at(slot); }

  private:
    std::filesystem::path dir_;
    Palette palette_{};
    SpriteSheet blocks_;
    SpriteSheet panel_;
    SpriteSheet rat_panel_;
    SpriteSheet sprites_;
    SpriteSheet pointers_;
    SpriteSheet watch_;
    SpriteSheet money_;
    AnimData anims_;
    PersonComplexes person_complexes_;
    StaffComplexes staff_complexes_;
    std::vector<uint8_t> ghost_;
    LanguageText text_;
    int language_ = 0;
    std::map<int, Font> fonts_;
};

} // namespace th
