#include "app/player_details.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>
#include <vector>

namespace th
{

namespace
{

// HospPlayerDetails' offsets (include/hosp.h).
constexpr size_t kOffNameMax = 8;
constexpr size_t kOffYearsPlayed = 0x9;
constexpr size_t kOffUnused0xb = 0xb;
constexpr size_t kOffMusicVolume = 0xc;
constexpr size_t kOffSfxVolume = 0xd;
constexpr size_t kOffAmbientVolume = 0xe;
constexpr size_t kOffSfxOn = 0xf;
constexpr size_t kOffAmbientsOn = 0x10;
constexpr size_t kOffMusicOn = 0x11;
constexpr size_t kOffTrackEnabled = 0x12;
constexpr size_t kOffUnused0x1c = 0x1c;
constexpr size_t kOffShadowsOn = 0x1d;
constexpr size_t kOffGameSpeed = 0x1e;
constexpr size_t kOffSpeedChoice = 0x20;
constexpr size_t kOffLanguage = 0x21;
constexpr size_t kOffAutosave = 0x22;
constexpr size_t kOffRoomHelpSeen = 0x23;
constexpr size_t kOffAdvisorFlags = 0x41;
constexpr size_t kOffSerialConfig = 0x56;

} // namespace

void set_default_player_details(PlayerDetails &d)
{
    d.game_speed = kDefaultGameSpeed;
    d.speed_choice = 2;
    d.years_played = 0;
    d.unused_0xb = 0;
    d.shadows_on = 1;
    d.autosave = 1;
    d.advisor_flags[20] = 1;
    d.sfx_on = 1;
    d.sfx_volume = 0x7f;
    d.music_on = 1;
    d.music_volume = 0x7f;
    d.ambients_on = 1;
    d.ambient_volume = 0x7f;
}

std::array<uint8_t, kPlayerDetailsSize> encode_player_details(const PlayerDetails &d, std::string_view name)
{
    std::array<uint8_t, kPlayerDetailsSize> b{};
    // The fields below share bytes with the name's room after its eight
    // characters and terminator, so the name goes first.
    std::copy_n(name.begin(), std::min(name.size(), kOffNameMax), b.begin());
    b[kOffYearsPlayed] = uint8_t(d.years_played);
    b[kOffYearsPlayed + 1] = uint8_t(d.years_played >> 8);
    b[kOffUnused0xb] = d.unused_0xb;
    b[kOffMusicVolume] = d.music_volume;
    b[kOffSfxVolume] = d.sfx_volume;
    b[kOffAmbientVolume] = d.ambient_volume;
    b[kOffSfxOn] = d.sfx_on;
    b[kOffAmbientsOn] = d.ambients_on;
    b[kOffMusicOn] = d.music_on;
    std::ranges::copy(d.track_enabled, b.begin() + kOffTrackEnabled);
    b[kOffUnused0x1c] = d.unused_0x1c;
    b[kOffShadowsOn] = d.shadows_on;
    b[kOffGameSpeed] = uint8_t(d.game_speed);
    b[kOffGameSpeed + 1] = uint8_t(d.game_speed >> 8);
    b[kOffSpeedChoice] = d.speed_choice;
    b[kOffLanguage] = d.language;
    b[kOffAutosave] = d.autosave;
    std::ranges::copy(d.room_help_seen, b.begin() + kOffRoomHelpSeen);
    std::ranges::copy(d.advisor_flags, b.begin() + kOffAdvisorFlags);
    std::ranges::copy(d.serial_config, b.begin() + kOffSerialConfig);
    return b;
}

void decode_player_details(PlayerDetails &d, std::span<const uint8_t> bytes)
{
    // A short file fills the front of the original's buffer: take the bytes
    // read over a copy of the current ones.
    std::array<uint8_t, kPlayerDetailsSize> b = encode_player_details(d, {});
    std::copy_n(bytes.begin(), std::min(bytes.size(), b.size()), b.begin());
    d.years_played = uint16_t(b[kOffYearsPlayed] | b[kOffYearsPlayed + 1] << 8);
    d.unused_0xb = b[kOffUnused0xb];
    d.music_volume = b[kOffMusicVolume];
    d.sfx_volume = b[kOffSfxVolume];
    d.ambient_volume = b[kOffAmbientVolume];
    d.sfx_on = b[kOffSfxOn];
    d.ambients_on = b[kOffAmbientsOn];
    d.music_on = b[kOffMusicOn];
    std::copy_n(b.begin() + kOffTrackEnabled, d.track_enabled.size(), d.track_enabled.begin());
    d.unused_0x1c = b[kOffUnused0x1c];
    d.shadows_on = b[kOffShadowsOn];
    d.game_speed = uint16_t(b[kOffGameSpeed] | b[kOffGameSpeed + 1] << 8);
    d.speed_choice = b[kOffSpeedChoice];
    d.language = b[kOffLanguage];
    d.autosave = b[kOffAutosave];
    std::copy_n(b.begin() + kOffRoomHelpSeen, d.room_help_seen.size(), d.room_help_seen.begin());
    std::copy_n(b.begin() + kOffAdvisorFlags, d.advisor_flags.size(), d.advisor_flags.begin());
    std::copy_n(b.begin() + kOffSerialConfig, d.serial_config.size(), d.serial_config.begin());
}

std::filesystem::path player_details_path(const std::filesystem::path &save_dir, std::string_view name)
{
    return save_dir / (std::string(name) + ".hdt");
}

DetailsRead read_player_details(const std::filesystem::path &path, PlayerDetails &d)
{
    std::error_code ec;
    if (!std::filesystem::exists(path, ec))
        return DetailsRead::Missing;
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return DetailsRead::Failed;
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    decode_player_details(d, bytes);
    return DetailsRead::Read;
}

bool write_player_details(const std::filesystem::path &path, const PlayerDetails &d, std::string_view name)
{
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    auto bytes = encode_player_details(d, name);
    out.write(reinterpret_cast<const char *>(bytes.data()), std::streamsize(bytes.size()));
    return bool(out);
}

} // namespace th
