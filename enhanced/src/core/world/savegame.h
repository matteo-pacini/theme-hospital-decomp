#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace th
{

class Game;
class GameData;

// The saved games. A save file holds a snapshot (core/world/snapshot.h) as
// its payload, wrapped in a small header: the format magic, kSaveFormat and
// the snapshot layout hash (snapshot_layout_hash), then the level, the
// balance id and tutorial the level continues with, the view's camera and
// the payload's length. The header, not the snapshot's source hash, is the
// save format check, so a save stays loadable across ordinary source edits;
// bump kSaveFormat by hand when the header's fields change and the
// snapshot.cpp static_asserts when the saved layout changes.

// The slot the main menu's Continue loads, written when the game returns to
// the main menu (quit_game 1: save_game(0x63)).
inline constexpr int kContinueSlot = 0x63;
// The slot check_autosave uses.
inline constexpr int kAutosaveSlot = 8;
// The slots the File > Save and File > Load menus offer (pulldown_save and
// pulldown_load: item + 1, below 10): their submenus hold eight each.
inline constexpr int kMenuSlots = 8;
// The save header's format: bump this by hand when its fields change (the
// snapshot's layout has its own hash).
inline constexpr uint32_t kSaveFormat = 2;

// What a save file carries besides the snapshot: the level, the balance id
// and tutorial the game continues with, and the view's centre in map pixels
// (so the view is restored the same whatever the window's size, and whether
// or not the view surface is sized yet).
struct SaveInfo {
    int level = 0;
    std::string balance_id;
    bool tutorial = false;
    int view_centre_x = 0;
    int view_centre_y = 0;
};

// The path of slot `slot` for the player named `player_name` in `save_dir`.
std::filesystem::path save_slot_path(const std::filesystem::path &save_dir, std::string_view player_name, int slot);

// Whether `path` holds a readable file.
bool save_slot_exists(const std::filesystem::path &path);

// Whether `path` holds a save this build can load: the right magic,
// kSaveFormat and snapshot layout hash.
bool save_slot_compatible(const std::filesystem::path &path);

// The level number of the save at `path`, or -1 when it does not exist or is
// not a save this build can load. It reads the header only
// (save_game_valid/read_level_number of src/io.cpp).
int save_slot_level(const std::filesystem::path &path);

// Writes the snapshot of `game` to `path` through `path.tmp` (renamed over
// it, so a failed or cut short write never leaves a half save behind), with
// `info`'s balance id, tutorial and camera (its level is the game's). Returns
// false, with the reason in `error`, when the game was never started or the
// file cannot be written.
bool write_save_slot(const std::filesystem::path &path, const Game &game, const SaveInfo &info,
                     std::string *error = nullptr);

// Replaces `game` with the save at `path`, its level taken from `data` and
// its level, balance id, tutorial and camera, when `info` is not null, from
// the header. Returns false, with the reason in `error`, for a missing file,
// a save of another format or layout, or a damaged payload; then `game` is
// unchanged unless the payload was cut short, which leaves it to be started
// or loaded again.
bool read_save_slot(const std::filesystem::path &path, Game &game, const GameData &data, SaveInfo *info = nullptr,
                    std::string *error = nullptr);

} // namespace th
