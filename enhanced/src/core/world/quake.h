#pragma once

#include "core/world/world.h"

#include <cstdint>

namespace th
{

class Game;
struct Date;

// Earthquakes (src/quake.cpp and src/earthqua.cpp): the state machine a
// scheduled quake runs through, its screen shake and the wear it leaves on
// the hospitals' machines. The schedule (Disasters::next_quake_turn) is set
// by setup_next_quake (emergency.cpp).

// generate_quake (0x43b90), daily (daily_admin): runs the quake's phase. At
// the scheduled turn a warning tremor is scheduled (phase 1), which after a
// few days becomes the main quake (phase 2) that shakes the screen and
// scares people; its peak (phase 3) wears the machines (do_earthquake), and
// once it dies down (phase 4) another quake is scheduled.
void generate_quake(Game &game);

// earthquake (0xa2f00), every turn while one runs (start_turn): the quake's
// amplitude builds up to `magnitude`, then dies down, each step taken with
// probability 1/chance per call; otherwise the view is shaken by the
// amplitude. When it has died out it records the strength (AdCounts::
// last_quake_size) and resets for the next quake. Always returns 1.
uint8_t earthquake(Game &game, int8_t magnitude, uint8_t chance);

} // namespace th
