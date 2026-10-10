#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string_view>

namespace th
{

// The size of the player details file: the original's HospPlayerDetails
// (myplayer), which it writes whole as save/<name>.hdt.
inline constexpr size_t kPlayerDetailsSize = 0x5c;

// The speed a player without a file starts at: set_default_player_details's
// 0x19, Max (apply_player_details calls vary_speed(40) for it, which ticks the
// Max item although speed_choice stays 2).
inline constexpr uint16_t kDefaultGameSpeed = 0x19;
// What a driven run (--drive, --control) starts at instead: Normal, so that
// the scripts' frame counts stay as they were.
inline constexpr uint16_t kDrivenGameSpeed = 0x16;

// The settings the original keeps per player and carries between runs
// (src/details.cpp). The values a new process starts with are the member
// initialisers (start_game and init_midi's).
struct PlayerDetails {
    uint8_t sfx_on = 1;
    uint8_t sfx_volume = 0x7f;
    uint8_t music_on = 1;
    uint8_t music_volume = 0x7f;
    uint8_t ambients_on = 1;
    uint8_t ambient_volume = 0x7f;
    // The jukebox's playlist, by track.
    std::array<uint8_t, 10> track_enabled{1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    uint8_t shadows_on = 1;
    // Turns a second (1, 5, 0x16, 0x19 or 0x7d00) and the ticked item of
    // Options > Game speed.
    uint16_t game_speed = kDefaultGameSpeed;
    uint8_t speed_choice = 2;
    uint8_t language = 0;
    uint8_t autosave = 1;
    // By room type below 30. The decontamination room (type 30) is indexed
    // past the end of the original's array, onto advisor_flags[0].
    std::array<uint8_t, 30> room_help_seen{};
    // One-off advice given, by advice slot; [20] is the advisor on (Shift+A).
    std::array<uint8_t, 21> advisor_flags{};
    // Fields only the original uses: kept as read so that a file written by
    // the original is written back whole.
    uint16_t years_played = 0;
    uint8_t unused_0xb = 0;
    uint8_t unused_0x1c = 0;
    std::array<uint8_t, 6> serial_config{};
};

// set_default_player_details: the values a player without a file starts
// with. It leaves room_help_seen, advisor_flags[0..19] and track_enabled as
// they are (the original's three clearing calls pass their size and value
// swapped, so they clear nothing).
void set_default_player_details(PlayerDetails &d);

// The file's bytes: HospPlayerDetails' layout, with `name` (at most eight
// characters) in front.
std::array<uint8_t, kPlayerDetailsSize> encode_player_details(const PlayerDetails &d, std::string_view name);

// Reads the fields from `bytes` (kPlayerDetailsSize of them; fewer leave the
// rest as they are, as a short read does). The name is not read.
void decode_player_details(PlayerDetails &d, std::span<const uint8_t> bytes);

// save/<name>.hdt: the original's save prefix "save/<name>.h" and "dt".
std::filesystem::path player_details_path(const std::filesystem::path &save_dir, std::string_view name);

enum class DetailsRead {
    Missing, // no file: the caller applies the defaults
    Read,
    Failed, // the file exists but could not be read
};

// load_player_details' file part.
DetailsRead read_player_details(const std::filesystem::path &path, PlayerDetails &d);

// save_player_details: returns false when the file cannot be written whole.
bool write_player_details(const std::filesystem::path &path, const PlayerDetails &d, std::string_view name);

} // namespace th
