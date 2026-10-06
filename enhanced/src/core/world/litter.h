#pragma once

#include "core/data/anims.h"
#include "core/data/balance.h"
#include "core/world/world.h"

#include <cstdint>

namespace th
{

// Litter and the patients' needs, as the game's environ.cpp (modify_person,
// do_reaction) and handyman.cpp (place_litter, remove_litter).
//
// A cell's litter is the low nibble of MapCell.litter_shadow: 0 none, 1-3
// rubbish, 4 the rats' mess, 5 vomit, 6 a puddle, 7 and 8 rat holes, which
// handymen leave. Litter bins (OBJ_BIN) do nothing, as in the game: nothing
// there looks for them.

// Litter kinds.
constexpr uint8_t kLitterVomit = 5;
constexpr uint8_t kLitterPuddle = 6;

// Patient state playing a reaction (the game's PS_SHOWANIM).
constexpr uint8_t kPsShowanim = 0x3a;

// Thing.person_flags bits (the game's PF_*, include/enums.h): the reaction
// playing ends in vomit or a puddle.
constexpr uint32_t kPfVomits = 0x4;
constexpr uint32_t kPfWets = 0x8;

// The heating level a hospital starts with (allhosps.heating_level,
// init_player in src/start.cpp).
constexpr int kStartHeatingLevel = 5;

// place_litter (0x7a710): drops litter of `kind` (1-15) on `tile` if the
// cell is `owner`'s land and indoors (owner_flags exactly 1 << owner), on a
// clean cell; vomit and the rats' mess (4) also replace other litter, but
// not rat holes. Nothing stops litter on a cell under an object. The
// hospital's litter count and last litter cell are not kept.
void place_litter(World &world, uint16_t tile, uint8_t owner, uint8_t kind);

// remove_litter (0x7a814): cleans `tile` if it is `owner`'s land and holds
// litter other than rat holes.
void remove_litter(World &world, uint16_t tile, uint8_t owner);

// A patient's urge to drop litter grows by `amount`, as using a drinks
// machine does (+10, complex_success's case 2 in src/complex.cpp).
void add_litter_urge(Thing &patient, uint16_t amount);

// check_warmth (0x712a8): person `p`'s warmth one step towards that of
// the radiators near by at `heating_level` (outside rooms; a room's
// radiators are not ported, so its warmth is 0); a hot patient gets
// thirstier.
void check_warmth(const World &world, Thing &p, int heating_level);

// staff_check_around (0x72110): member of staff `s` is happier by the
// healthy plants near by and less happy by the wilting ones and, in the
// corridor, by the litter round them.
void staff_check_around(const World &world, Thing &s);

// modify_person (0x70f80) for patient `id` at `game_turn`: due once
// game_turn (in 16 bits) passes Thing.modify_time, which it then sets
// gbv.modify_freq turns on, and indoors: warmth from the radiators at
// `heating_level` (check_warmth), plants (check_feelgood), litter around
// (check_around), spreading contagion (spread_epidemic), then impatience,
// thirst, the toilet, heat, happiness and 1 health (change_health, which
// may kill). Draws from world.seed as the game does.
// Not ported, as what they need does not exist yet: emergencies, the
// contagious and vaccinated flags (never set), status icons, a room's
// radiators (its warmth is 0) and effect_illness on a spreading carrier.
void modify_person(World &world, const AnimData &anims, const Balance &settings, int heating_level, uint16_t id,
                   uint32_t game_turn);

// do_reaction (0x71960) for patient `id`: in the corridor indoors, may
// vomit (from gbv.vomit_limit nausea) or wet themself (from
// gbv.bowel_overflows), each costing 10 health and playing a reaction
// (PS_SHOWANIM, see person_showanim_turn) that leaves the mess; may play an
// idle animation (1 in 200); may drop litter (past gbv.litter_drop urge or
// 1 in gbv.litter_random, then 1 in 10). Earthquakes and abductions, which
// do not exist, never happen. Returns whether a reaction started, in which
// case the patient does not step this turn. Draws from world.seed as the
// game does.
bool do_reaction(World &world, const AnimData &anims, const Balance &settings, uint16_t id, uint32_t game_turn);

// modify_person then do_reaction, as a walking patient does when it decides
// its next step (person_goto_destination, 0x763d0). Returns do_reaction's
// result.
bool person_needs_turn(World &world, const AnimData &anims, const Balance &settings, int heating_level, uint16_t id,
                       uint32_t game_turn);

// play_person_showanim (0x17ae0), PS_SHOWANIM's turn: plays the reaction a
// frame every anim_speed turns; at its last frame goes back to the saved
// state, frame and mirroring, and leaves vomit or a puddle on its cell if
// the reaction was one.
void person_showanim_turn(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

} // namespace th
