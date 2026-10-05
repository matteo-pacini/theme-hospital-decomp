#include "core/data/text.h"

#include "core/data/gamedata.h"

#include <array>
#include <cctype>

namespace th
{

std::optional<int> config_language(const std::filesystem::path &dir)
{
    auto bytes = load_data_file(dir, "hospital.cfg");
    if (!bytes)
        return std::nullopt;
    std::string_view data(reinterpret_cast<const char *>(bytes->data()), bytes->size());
    size_t pos = 0;
    for (int i = 0; i < 4 && pos + 1 < data.size(); i++) {
        size_t nl = data.find('\n', pos);
        if (nl == std::string_view::npos)
            nl = data.size();
        // The game blanks the byte before each '\n', expecting CRLF.
        std::string_view line = data.substr(pos, nl > pos ? nl - pos - 1 : 0);
        pos = nl + 1;
        size_t eq = line.find('=');
        if (eq == std::string_view::npos)
            break;
        std::string_view key = line.substr(0, eq);
        std::string_view value = line.substr(eq + 1);
        if (key == "INSTALL_PATH" || key == "INSTALL_TYPE" || key == "SAVE_OVERRIDE")
            continue;
        if (key != "LANGUAGE")
            break;
        auto letter = [&](size_t n) {
            return n < value.size() ? std::tolower(static_cast<unsigned char>(value[n])) : 0;
        };
        switch (letter(0)) {
        case 'f':
            return kFrench;
        case 'g':
            return kGerman;
        case 'i':
            return kItalian;
        case 's':
            return letter(1) == 'p' ? kSpanish : kSwedish;
        default:
            return kEnglish;
        }
    }
    return std::nullopt;
}

bool LanguageText::load(std::vector<uint8_t> bytes, std::string *error)
{
    auto fail = [&](const char *why) {
        if (error)
            *error = why;
        sections_.clear();
        data_.clear();
        return false;
    };
    auto s16 = [&](size_t at) { return int16_t(bytes[at] | bytes[at + 1] << 8); };
    if (bytes.size() < 2)
        return fail("language file too short");
    int count = s16(0);
    if (count < 0 || bytes.size() < 2 + size_t(count) * 2)
        return fail("language file too short for its section count");
    std::vector<std::vector<uint32_t>> sections(static_cast<size_t>(count));
    size_t text = 2 + size_t(count) * 2;
    for (int i = 0; i < count; i++) {
        int strings = s16(2 + size_t(i) * 2);
        for (int j = 0; j < strings; j++) {
            size_t end = text;
            while (end < bytes.size() && bytes[end])
                end++;
            if (end == bytes.size())
                return fail("language file ends inside its strings");
            sections[size_t(i)].push_back(uint32_t(text));
            text = end + 1;
        }
    }
    data_ = std::move(bytes);
    sections_ = std::move(sections);
    return true;
}

size_t LanguageText::size(int section) const
{
    return section >= 0 && size_t(section) < sections_.size() ? sections_[size_t(section)].size() : 0;
}

std::string_view LanguageText::at(int section, int index) const
{
    if (index < 0 || size_t(index) >= size(section))
        return {};
    return reinterpret_cast<const char *>(data_.data() + sections_[size_t(section)][size_t(index)]);
}

bool load_language(const std::filesystem::path &dir, int language, LanguageText &text, std::string *error)
{
    std::string name = "data/lang-" + std::to_string(language) + ".dat";
    auto bytes = load_data_file(dir, name, error);
    if (!bytes)
        return false;
    if (!text.load(std::move(*bytes), error)) {
        if (error)
            *error = name + ": " + *error;
        return false;
    }
    return true;
}

namespace
{

// hires_font_filenames (src/files.cpp). Slot 45 is a low-resolution font in
// both tables.
constexpr std::array<std::string_view, kFontSlots> kHiresFonts = {
    "qdata/font00v",  "qdata/font01v",  "qdata/font02v",  "qdata/font74v",   "qdata/font04v",  "qdata/font05v",
    "qdata/font16v",  "qdata/font105v", "qdata/font106v", "qdata/font43v",   "qdata/font09v",  "qdata/font44v",
    "qdata/font60v",  "qdata/font24v",  "qdata/font25v",  "qdata/font26v",   "qdata/font45v",  "qdata/font46v",
    "qdata/font47v",  "qdata/font39v",  "qdata/font40v",  "qdata/font500v",  "qdata/font501v", "qdata/font35v",
    "qdata/font31v",  "qdata/font34v",  "qdata/font38v",  "qdata/font36v",   "qdata/font37v",  "qdata/font50v",
    "qdata/font51v",  "qdata/font19v",  "qdata/font18v",  "qdata/font112v",  "qdata/font113v", "qdata/font115v",
    "qdata/font100v", "qdata/font101v", "qdata/font102v", "qdata/font120v",  "qdata/font111v", "qdata/font110v",
    "qdata/font121v", "qdata/font122v", "qdata/font124v", "qdatam/font130m", "qdata/font502v", "qdata/font250v",
};

} // namespace

std::string_view font_file(int slot)
{
    return slot >= 0 && slot < kFontSlots ? kHiresFonts[size_t(slot)] : std::string_view{};
}

bool load_font(const std::filesystem::path &dir, int slot, Font &font, std::string *error)
{
    std::string_view base = font_file(slot);
    if (base.empty()) {
        if (error)
            *error = "no font slot " + std::to_string(slot);
        return false;
    }
    std::string tab_name = std::string(base) + ".tab";
    auto tab = load_data_file(dir, tab_name, error);
    if (!tab)
        return false;
    auto dat = load_data_file(dir, std::string(base) + ".dat", error);
    if (!dat)
        return false;
    SpriteSheet sheet;
    if (!sheet.load(std::move(*tab), std::move(*dat), SpriteFormat::Rle)) {
        if (error)
            *error = tab_name + ": not a sprite table";
        return false;
    }
    font = Font(std::move(sheet));
    return true;
}

} // namespace th
