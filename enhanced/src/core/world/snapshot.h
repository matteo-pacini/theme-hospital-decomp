#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace th
{

class Game;
class GameData;

// A snapshot: the whole state of a game in progress, for the debug report
// and th_replay --load. It is tied to the build that wrote it, not a saved
// game format: it starts with a magic and snapshot_build_id(), and a build
// with another id refuses it.
//
// It holds everything the turns, the commands and command_log read: the
// world (map, things, rooms, entrances, placing, building, pagers, calls and
// the rest of World), the date, turn, pause, opening countdown, balance,
// finance, advisor, arrivals and hospitals, land, hospital settings, staff
// market, the pending command, the event log, command_log, the level's
// settings (the Balance itself, so the balance set is not needed), the
// path finding tool's walkers and the random seeds. The level file, the
// animations and the action sequences are the data's, taken from the
// GameData it is loaded with by the level's number.

// The build's id: a hash of the snapshotted structures' headers and the
// snapshot code (without comments), and of the sizes of what is copied as
// raw bytes. A debug report's snapshot and th_replay --load are tied to the
// build that wrote them.
uint64_t snapshot_build_id();

// The hash of the sizes of what a snapshot copies as raw bytes, the part of
// the build id that a save file checks instead of the source hash: a save
// stays loadable across ordinary source edits (new methods, trace events)
// that do not change the saved layout.
uint64_t snapshot_layout_hash();

// Writes the snapshot of `game` into `out` (replacing what it held). Returns
// false if `game` was never started.
bool save_snapshot(const Game &game, std::vector<uint8_t> &out);

// Makes `game` the one `data` is the snapshot of, its level file and
// animations taken from `gamedata`, its World pointers set as Game::start
// sets them. Returns false, with the reason in `error`, for something that
// is not a snapshot, one of another build, or a level `gamedata` cannot
// load; then `game` is unchanged unless the snapshot was cut short, which
// leaves it to be started or loaded again.
bool load_snapshot(Game &game, std::span<const uint8_t> data, const GameData &gamedata, std::string *error = nullptr);

// As load_snapshot but without the build-id check: the payload of a save file
// whose own header (savegame.h) already matched the saved layout. The magic
// and the payload's length are still checked.
bool load_snapshot_payload(Game &game, std::span<const uint8_t> data, const GameData &gamedata,
                           std::string *error = nullptr);

} // namespace th
