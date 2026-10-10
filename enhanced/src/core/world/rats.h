#pragma once

#include "core/data/anims.h"
#include "core/data/balance.h"
#include "core/world/events.h"
#include "core/world/world.h"

#include <cstdint>

namespace th
{

struct Game;
struct StaffMarket;

// Rats, as the game's rats.cpp (manage_rats and the functions it reaches)
// and handyman.cpp's place_litter/remove_litter for their last litter cell.
//
// Litter kinds 7 and 8 are rat holes drawn on the floor; the holes
// themselves are hospital 0's (allhosps[0].ratholes), which try_create_rathole
// adds and remove_rathole takes away. manage_rats runs in daily_admin and
// may add a hole, remove one or send a rat between two. A rat is a thing
// (kThingRat) that walks its cells from one hole to the next and then
// vanishes into the hole. The player shoots it (EV_SHOOT_RAT), counting the
// hospital's rat_shots and rats_killed.

// LbSqrL (0xc9abc): the integer square root, rounded down, of `n`.
uint32_t lb_sqr_l(uint32_t n);
// LbArcTan (0xcd3f4): the game's arctangent of (dx, dy), the table indexed
// by the smaller over the larger of dx and -dy, as an angle in 1/2048 of a
// turn (src/asm/arctan.s). Only a rat's facing frame reads it.
uint16_t lb_arc_tan(int32_t dx, int32_t dy);

// manage_rats (0x7d364): once a day, for hospital 0 (single player), may
// make a rat hole (more likely the more litter), rarely remove one, and may
// send out a rat. Draws from world.seed as the game does.
void manage_rats(World &world, const AnimData &anims);

// make_a_rat (0x7d448): sends a rat from one of hospital `owner`'s rat holes
// to another, if there are two or more. In the rat game the next rat takes
// the look in World::Rats::next_rat_layer.
void make_a_rat(World &world, const AnimData &anims, uint8_t owner);

// rat_fest_control (0x7dd18): the rat game's per-turn control when
// World::Rats::fest_level is set: a new weapon after every ten kills, more
// rats, extra handymen, and the end of the game (Progress::rat_game_over,
// and no things run that turn) when the litter falls to 4 or by turn 9000.
// Outside it, resets the rat hit radii. Draws from world.seed as the game
// does.
void rat_fest_control(Game &game);

// rat_fest_builder (0x7df1c): sets up rat game level `level` from the
// game's rat_fest_level table: seeds world.seed and Game::safeseed and
// scatters litter, rat holes and handymen over the player's land, then
// starts the game (World::Rats::fest_level).
void rat_fest_builder(Game &game, StaffMarket &market, uint8_t level);

// training_builder (0x2cccc, src/staff.cpp): sets up the training level
// (towns[].special_kind 2) from the rat_fest_level table's entry `level`:
// seeds world.seed and Game::safeseed and, on the first tile of the player's
// land that passes the level's staff_chance roll, places three consultant
// doctors: a psychiatrist, a surgeon and a researcher. Leaves the job
// market's skill odds as it sets them for the three.
void training_builder(Game &game, StaffMarket &market, uint8_t level);

// setup_new_level's special levels (src/start.cpp): a level whose towns[]
// entry has special_kind 1 runs rat_fest_builder and one with special_kind 2
// training_builder, both with its special_arg; any other level nothing. Run
// after the map is loaded and before the job market is made.
void start_special_level(Game &game, StaffMarket &market, int level_number);

// cleanse_area (0x7dab4): removes hospital `owner`'s rat holes and the litter
// in the box (x, y, width, height) of cells.
void cleanse_area(World &world, uint8_t owner, int x, int y, int width, int height);

// clear_cell_of_ratholes (0x7d2a8): removes hospital `owner`'s rat hole on
// `tile`, if any. The last hole in the list is never checked.
void clear_cell_of_ratholes(World &world, uint8_t owner, uint16_t tile);

// remove_rathole (0x7d184): removes hospital `owner`'s rat hole `idx`,
// moving the last one into its place, and its floor sprite.
void remove_rathole(World &world, uint8_t owner, uint8_t idx);

// rat_turn (src/things.cpp's THING_RAT case): one turn of rat `id` in its
// state.
void rat_turn(World &world, const AnimData &anims, uint16_t id);

// The player shoots at `rat_id` (a rat, or 0 for a miss): the shot is
// counted (Game::progress.rat_shots) and its powerup spent, and a hit kills
// the rat, counts it, floats its number over it, leaves its mess and plays
// the sounds. `powerup` and `hit` are the event's argb[0] and argb[1].
void shoot_rat(Game &game, uint16_t rat_id, uint8_t powerup, uint8_t hit);

// process_left_button's rat shot (src/mouse.cpp, hover actions 5 and 6): the
// weapon in hand (World::Rats::fest_powerup) goes with the EV_SHOOT_RAT
// `shot` as its argb[0], and its shots run down; the last one puts the
// weapon away and narrows the hit box. Does nothing for another event.
void arm_rat_shot(World &world, Event &shot);

} // namespace th
