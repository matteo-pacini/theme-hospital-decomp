#pragma once

#include <string>

namespace th
{

class Game;

// A plain-text dump of `game` for bug reports, as much of its state as is
// ported, each line `name=value` pairs:
//   - the date, money, finance, the hospitals' figures and the turn's trace
//     (core/world/trace.h);
//   - the recent events Game::events holds, oldest first, with their turns;

//   - the building and object placing states, pagers and player calls;
//   - every room in use;
//   - every thing in use, people with all their fields (patient, action
//     sequence, staff, needs, queue and diagnosis groups), objects with
//     theirs;
//   - the map cells within `radius` of cell `focus` (column + row * 128; none
//     if negative): floor, walls, flags, shadows and litter, owner, first
//     thing and room.
// Fields added to Thing, RoomInfo or World are not in it until added here.
std::string dump_game(const Game &game, int focus, int radius = 10);

} // namespace th
