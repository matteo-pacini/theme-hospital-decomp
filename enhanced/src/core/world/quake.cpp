#include "core/world/quake.h"

#include "core/world/complex_tables.h"
#include "core/world/disaster.h"
#include "core/world/emergency.h"
#include "core/world/game.h"
#include "core/world/litter.h"
#include "core/world/trace.h"

#include <bit>

namespace th
{

namespace
{

// The game's earthquake_table (src/earthqua.cpp): the view's shake per step.
constexpr std::array<std::array<int8_t, 2>, 16> kEarthquakeTable = {{
    {4, 4},
    {1, 2},
    {3, 1},
    {2, 1},
    {0, -2},
    {-1, -2},
    {4, -4},
    {1, -2},
    {-4, 4},
    {-3, -1},
    {-2, 0},
    {0, 2},
    {-2, -1},
    {-4, -4},
    {2, 0},
    {-1, 2},
}};

// Object states (OS_*): a wreck taken off the map.
constexpr uint8_t kObjectRemoved = 0x12;

// shake_screen (0xa30f0): moves the view by dx, dy unless that would pass
// the scroll limit. The port keeps the displacement in World::shake_x/y,
// which the interface adds to its camera; its scroll limit is not applied.
void shake_screen(World &world, int8_t dx, int8_t dy)
{
    world.shake_x = int16_t(world.shake_x + dx);
    world.shake_y = int16_t(world.shake_y + dy);
}

// saferand (src/rand.cpp) on the presentation generator, for the shake index
// the game draws from it.
uint16_t quake_saferand(Disasters &d)
{
    d.shake_seed = std::rotr(d.shake_seed * 0x24a1u + 0x24dfu, 13);
    return uint16_t(d.shake_seed);
}

// do_earthquake (0x43eac): wears every machine by the quake's severity
// (Disasters::quake_severity uses), updating those past 75% of their
// strength (modify_machine). Machines in a room being edited or rebuilt are
// spared.
void do_earthquake(Game &game)
{
    World &world = game.world;
    const AnimData &anims = game.anims();
    for (size_t i = 1; i < world.things.size(); i++) {
        Thing &obj = world.things[i];
        if (obj.type != kThingObject)
            continue;
        if (!kObjectUse[obj.subtype % kObjectUse.size()].overlay_anim)
            continue;
        if (obj.room_id && obj.room_id == world.building.build.room)
            continue;
        if (obj.state != kObjectRemoved) {
            obj.times_used += game.disasters.quake_severity;
            if (obj.strength * 75 / 100 <= obj.times_used)
                modify_machine(world, anims, uint16_t(i), game.turn_count);
        }
    }
}

} // namespace

uint8_t earthquake(Game &game, int8_t magnitude, uint8_t chance)
{
    World &world = game.world;
    Disasters &d = game.disasters;
    if (d.quake_ramp == 1) {
        while (d.quake_amplitude <= magnitude) {
            if (old_newrand(world.seed) % chance == 0) {
                d.quake_amplitude++;
            } else {
                d.quake_shake_index = uint8_t((d.quake_shake_index + quake_saferand(d)) % 16);
                shake_screen(world, int8_t(d.quake_amplitude * kEarthquakeTable[d.quake_shake_index][0]),
                             int8_t(d.quake_amplitude * kEarthquakeTable[d.quake_shake_index][1]));
                return 1;
            }
        }
        d.quake_ramp = -1;
    }
    if (d.quake_ramp == -1) {
        while (d.quake_amplitude >= 0) {
            if (old_newrand(world.seed) % chance == 0) {
                d.quake_amplitude--;
            } else {
                d.quake_shake_index = uint8_t((d.quake_shake_index + quake_saferand(d)) % 16);
                shake_screen(world, int8_t(d.quake_amplitude * kEarthquakeTable[d.quake_shake_index][0]),
                             int8_t(d.quake_amplitude * kEarthquakeTable[d.quake_shake_index][1]));
                return 1;
            }
        }
    }
    world.counts.last_quake_size = d.quake_strength;
    d.quake_strength = 0;
    d.quake_shake_index = 0;
    d.quake_ramp = 1;
    // ReleaseLoopedSample (0xc, 0x48) stops the game's looped quake roar; the
    // port plays no looped sample, so there is nothing to stop.
    // The game leaves the view where the quake pushed it; the port returns it.
    world.shake_x = 0;
    world.shake_y = 0;
    return 1;
}

void generate_quake(Game &game)
{
    World &world = game.world;
    Disasters &d = game.disasters;
    switch (d.quake_phase) {
    case 0:
        if (game.turn_count >= d.next_quake_turn && d.quake_pending)
            d.quake_phase = 1;
        break;
    case 1:
        d.quake_strength = 4;
        d.quake_step_chance = 4;
        d.quake_amplitude = 0;
        d.quake_ramp = 1;
        d.quake_phase = 2;
        d.quake_timer = game.turn_count + old_normal_rand(world.seed, 5, 10) * 50;
        // play_ambient: the tremor's rumble.
        world.machine_announcements.push_back(uint8_t(quake_saferand(d) % 3 + 0xab));
        break;
    case 2:
        if (game.turn_count > d.quake_timer) {
            d.quake_strength = 0x10;
            d.quake_step_chance = 4;
            d.quake_amplitude = 0;
            d.quake_ramp = 1;
            d.quake_phase = 3;
            d.quake_timer = game.turn_count + 200;
            disaster_peeps(world, game.anims(), game.settings(), 1, -1, game.turn_count);
            world.machine_announcements.push_back(uint8_t(quake_saferand(d) % 3 + 0xab));
        }
        break;
    case 3:
        if (d.quake_amplitude >= d.quake_strength * 3 / 4 || game.turn_count > d.quake_timer) {
            d.quake_phase = 4;
            d.quake_timer = game.turn_count + 100;
            do_earthquake(game);
        }
        break;
    case 4:
        if ((d.quake_ramp == -1 && d.quake_amplitude < 2) || game.turn_count > d.quake_timer) {
            disaster_peeps(world, game.anims(), game.settings(), 2, -1, game.turn_count);
            d.quake_phase = 0;
            setup_next_quake(d, world, game.settings(), game.date);
        }
        break;
    default:
        break;
    }
}

} // namespace th
