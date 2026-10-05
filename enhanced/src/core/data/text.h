#pragma once

#include "core/data/sprites.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace th
{

// The game's languages, as numbered by HOSPITAL.CFG's LANGUAGE and the
// DATA/LANG-<n>.DAT files.
enum Language : int {
    kEnglish = 0,
    kFrench = 1,
    kGerman = 2,
    kItalian = 3,
    kSpanish = 4,
    kSwedish = 5,
};

// The language HOSPITAL.CFG in `dir` names, as read_config_file (src/ben.cpp)
// reads it: by the value's first letter (e, f, g, i; "sp" Spanish, any other
// s Swedish; anything else English). Nothing if the file is missing or no
// LANGUAGE line comes among its first four lines before a malformed line or
// an unknown key, where the game stops reading.
std::optional<int> config_language(const std::filesystem::path &dir);

// The string tables of a language file, numbered as set_group_address
// (src/text.cpp) assigns them to the game's variables.
enum class TextSection : int {
    StaffNames = 0,
    ObjectNames = 1,
    Misc = 2,
    Illness = 3,
    ComputerNames = 4,
    Months = 5,
    Report = 6,
    Finance = 7,
    StaffNames1 = 8,
    StaffNames2 = 9,
    Advisor = 0x0a,
    TownNames = 0x0b,
    TownDetails = 0x0c,
    RoomNames = 0x0d,
    SupplierNames = 0x0e,
    Req1Help = 0x0f,
    ResearchNames = 0x10,
    PolicyNames = 0x11,
    DepartmentNames = 0x12,
    InsuranceNames = 0x13,
    ForcedAdvice = 0x14,
    MouseAdvice = 0x15,
    Pulldown = 0x16,
    Slist = 0x17,
    Hall = 0x18,
    ChartTypes = 0x19,
    Trophy = 0x1a,
    ExtraAdvice = 0x1b,
    Cabinet = 0x1c,
    Network = 0x1d,
    Nickname = 0x1e,
    Help = 0x1f,
    RoomHelp = 0x20,
    SkillNames = 0x21,
    YesNo = 0x22,
    Bank = 0x23,
    Stat = 0x24,
    WinLose = 0x25,
    Win = 0x26,
    ObjectHelp = 0x27,
    Load = 0x28,
    MayorNames = 0x29,
    Credit = 0x2a,
    Choice = 0x2b,
    Cheat = 0x2c,
    RefGeneral = 0x2d,
    RefGood = 0x2e,
    RefBad = 0x2f,
    Queue = 0x30,
    WinLoseText = 0x31,
    Progress = 0x32,
    DebugMsgs = 0x33,
    RealRoomNames = 0x34,
    Level = 0x35,
    GoodAwards = 0x36,
    BadAwards = 0x37,
    ShowHelp0 = 0x38,
    ShowHelp1 = 0x39,
    InfoBar = 0x3a,
    Overview = 0x3b,
    StaffUnhappy = 0x3c,
    AdvisorSituations = 0x3d,
    TrophyBonus = 0x3e,
    WinCriteriaHelp = 0x3f,
    // 0x40 and 0x41 are assigned to no variable.
    NetMission = 0x42,
    BetaDemoShort = 0x43,
};

// A language file (DATA/LANG-<n>.DAT, unpacked): a little-endian s16 count
// of sections, an s16 string count per section, then every string,
// NUL-terminated, section after section (setup_lang, src/text.cpp). Strings
// are in the game's 8-bit encoding, which the fonts index directly.
class LanguageText
{
  public:
    // Parses an unpacked language file. Fails if it is shorter than its
    // counts say.
    bool load(std::vector<uint8_t> bytes, std::string *error = nullptr);

    size_t section_count() const { return sections_.size(); }
    // Number of strings in `section`, 0 if there is no such section.
    size_t size(int section) const;
    size_t size(TextSection section) const { return size(int(section)); }
    // String `index` of `section`, empty if out of range.
    std::string_view at(int section, int index) const;
    std::string_view at(TextSection section, int index) const { return at(int(section), index); }

    // month_text[month]: the panel's date is "%d %s" with the day and this.
    std::string_view month(int month) const { return at(TextSection::Months, month); }
    // misc_text[index].
    std::string_view misc(int index) const { return at(TextSection::Misc, index); }
    // advisor_text[index].
    std::string_view advisor(int index) const { return at(TextSection::Advisor, index); }

  private:
    std::vector<uint8_t> data_;
    // Per section, the offset of each string in data_.
    std::vector<std::vector<uint32_t>> sections_;
};

// Loads DATA/LANG-<language>.DAT from `dir` (langfile, src/files.cpp).
bool load_language(const std::filesystem::path &dir, int language, LanguageText &text, std::string *error = nullptr);

// A Bullfrog font: a sprite sheet whose sprite c - 31 is the glyph of
// character c (c > 32). Sprite 1, the space, gives the space width and the
// line height.
class Font
{
  public:
    Font() = default;
    explicit Font(SpriteSheet sheet) : sheet_(std::move(sheet)) {}

    bool empty() const { return sheet_.size() < 2; }
    // Glyph of character `c`, empty for c <= 32 or past the sheet's end.
    Sprite glyph(uint8_t c) const { return c > 32 ? sheet_.at(c - 31u) : Sprite{}; }
    int space_width() const { return sheet_.at(1).width; }
    int line_height() const { return sheet_.at(1).height; }

  private:
    SpriteSheet sheet_;
};

// Number of font cache slots (set_font_from_cache's argument).
inline constexpr int kFontSlots = 48;

// The file, without .TAB/.DAT, that font cache slot `slot` loads in high
// resolution (hires_font_filenames, src/files.cpp); empty if out of range.
std::string_view font_file(int slot);

// Loads the font of cache slot `slot` from `dir` in high resolution, its
// glyphs in LbSpriteDraw's coding.
bool load_font(const std::filesystem::path &dir, int slot, Font &font, std::string *error = nullptr);

} // namespace th
