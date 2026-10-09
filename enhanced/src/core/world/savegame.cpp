#include "core/world/savegame.h"

#include "core/data/gamedata.h"
#include "core/world/game.h"
#include "core/world/snapshot.h"

#include <array>
#include <cstring>
#include <format>
#include <fstream>
#include <iterator>

namespace th
{

namespace
{

// The save header's magic (the snapshot's own magic follows in the payload).
constexpr std::array<uint8_t, 8> kSaveMagic = {'T', 'H', 'S', 'V', 'G', 'M', 0, 1};

void put_u32(std::vector<uint8_t> &out, uint32_t v)
{
    for (int s = 0; s < 32; s += 8)
        out.push_back(uint8_t(v >> s));
}

void put_u64(std::vector<uint8_t> &out, uint64_t v)
{
    for (int s = 0; s < 64; s += 8)
        out.push_back(uint8_t(v >> s));
}

uint32_t get_u32(std::span<const uint8_t> in, size_t &at)
{
    uint32_t v = 0;
    if (at + 4 > in.size()) {
        at = in.size();
        return 0;
    }
    for (int s = 0; s < 32; s += 8)
        v |= uint32_t(in[at++]) << s;
    return v;
}

uint64_t get_u64(std::span<const uint8_t> in, size_t &at)
{
    uint64_t v = 0;
    if (at + 8 > in.size()) {
        at = in.size();
        return 0;
    }
    for (int s = 0; s < 64; s += 8)
        v |= uint64_t(in[at++]) << s;
    return v;
}

// A parsed save header and where its payload starts. `ok` is false for a
// header this build cannot load, with `why` set.
struct Header {
    bool ok = false;
    std::string why;
    int level = 0;
    std::string balance_id;
    bool tutorial = false;
    int view_x = 0;
    int view_y = 0;
    size_t payload_at = 0;
    uint32_t payload_len = 0;
};

Header parse_header(std::span<const uint8_t> data)
{
    Header h;
    size_t at = 0;
    if (data.size() < kSaveMagic.size()) {
        h.why = "not a save";
        return h;
    }
    if (std::memcmp(data.data(), kSaveMagic.data(), kSaveMagic.size()) != 0) {
        h.why = "not a save";
        return h;
    }
    at = kSaveMagic.size();
    uint32_t format = get_u32(data, at);
    if (at + 8 > data.size() || format != kSaveFormat) {
        h.why = std::format("save format {}, this build reads {}", format, kSaveFormat);
        return h;
    }
    uint64_t layout = get_u64(data, at);
    if (layout != snapshot_layout_hash()) {
        h.why = "a save of another layout: load it with the build that wrote it";
        return h;
    }
    h.level = int32_t(get_u32(data, at));
    uint32_t name_len = get_u32(data, at);
    if (at + name_len + 1 + 8 + 4 > data.size() || name_len > 64) {
        h.why = "a save cut short";
        return h;
    }
    h.balance_id.assign(reinterpret_cast<const char *>(data.data() + at), name_len);
    at += name_len;
    h.tutorial = data[at++] != 0;
    h.view_x = int32_t(get_u32(data, at));
    h.view_y = int32_t(get_u32(data, at));
    h.payload_len = get_u32(data, at);
    h.payload_at = at;
    if (at + h.payload_len != data.size()) {
        h.why = "a save cut short or damaged";
        return h;
    }
    h.ok = true;
    return h;
}

std::vector<uint8_t> read_file(const std::filesystem::path &path)
{
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

// Writes `out` to `path` through `path.tmp` and renames it over, so a failed
// or cut short write never leaves a half save behind.
void set_error(std::string *error, std::string text)
{
    if (error)
        *error = std::move(text);
}

// Writes `out` to `path` through `path.tmp` and renames it over, so a failed
// or cut short write never leaves a half save behind.
bool atomic_write(const std::filesystem::path &path, const std::vector<uint8_t> &out, std::string *error)
{
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) {
        set_error(error, std::format("cannot make {}: {}", path.parent_path().string(), ec.message()));
        return false;
    }
    std::filesystem::path tmp = path;
    tmp += ".tmp";
    {
        std::ofstream file(tmp, std::ios::binary | std::ios::trunc);
        if (!file) {
            set_error(error, std::format("cannot write {}", tmp.string()));
            return false;
        }
        file.write(reinterpret_cast<const char *>(out.data()), std::streamsize(out.size()));
        file.close();
        if (!file) {
            set_error(error, std::format("cannot write {}", tmp.string()));
            return false;
        }
    }
    std::filesystem::rename(tmp, path, ec);
    if (ec) {
        std::filesystem::remove(tmp);
        set_error(error, std::format("cannot rename {}: {}", tmp.string(), ec.message()));
        return false;
    }
    return true;
}

// The save file's bytes: the header, then the snapshot `payload`.
std::vector<uint8_t> save_bytes(const Game &game, std::string_view balance_id, bool tutorial, int view_x, int view_y,
                                const std::vector<uint8_t> &payload)
{
    std::vector<uint8_t> out(kSaveMagic.begin(), kSaveMagic.end());
    put_u32(out, kSaveFormat);
    put_u64(out, snapshot_layout_hash());
    put_u32(out, uint32_t(int32_t(game.level_number())));
    put_u32(out, uint32_t(balance_id.size()));
    out.insert(out.end(), balance_id.begin(), balance_id.end());
    out.push_back(tutorial ? 1 : 0);
    put_u32(out, uint32_t(int32_t(view_x)));
    put_u32(out, uint32_t(int32_t(view_y)));
    put_u32(out, uint32_t(payload.size()));
    out.insert(out.end(), payload.begin(), payload.end());
    return out;
}

} // namespace

std::filesystem::path save_slot_path(const std::filesystem::path &save_dir, std::string_view player_name, int slot)
{
    return save_dir / std::format("{}.h{}", player_name, slot);
}

bool save_slot_exists(const std::filesystem::path &path)
{
    std::error_code ec;
    return std::filesystem::is_regular_file(path, ec);
}

bool save_slot_compatible(const std::filesystem::path &path)
{
    if (!save_slot_exists(path))
        return false;
    std::vector<uint8_t> bytes = read_file(path);
    return parse_header(bytes).ok;
}

int save_slot_level(const std::filesystem::path &path)
{
    if (!save_slot_exists(path))
        return -1;
    std::vector<uint8_t> bytes = read_file(path);
    Header h = parse_header(bytes);
    return h.ok ? h.level : -1;
}

bool write_save_slot(const std::filesystem::path &path, const Game &game, const SaveInfo &info, std::string *error)
{
    std::vector<uint8_t> payload;
    if (!save_snapshot(game, payload)) {
        if (error)
            *error = "the game was never started";
        return false;
    }
    return atomic_write(
        path, save_bytes(game, info.balance_id, info.tutorial, info.view_centre_x, info.view_centre_y, payload), error);
}

bool read_save_slot(const std::filesystem::path &path, Game &game, const GameData &data, SaveInfo *info,
                    std::string *error)
{
    auto fail = [&](std::string why) {
        if (error)
            *error = std::move(why);
        return false;
    };
    if (!save_slot_exists(path))
        return fail(std::format("no save file {}", path.string()));
    std::vector<uint8_t> bytes = read_file(path);
    Header h = parse_header(bytes);
    if (!h.ok)
        return fail(h.why);
    if (!load_snapshot_payload(game, std::span<const uint8_t>(bytes).subspan(h.payload_at, h.payload_len), data, error))
        return false;
    if (info) {
        info->level = h.level;
        info->balance_id = std::move(h.balance_id);
        info->tutorial = h.tutorial;
        info->view_centre_x = h.view_x;
        info->view_centre_y = h.view_y;
    }
    return true;
}

} // namespace th
