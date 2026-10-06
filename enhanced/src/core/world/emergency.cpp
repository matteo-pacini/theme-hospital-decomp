#include "core/world/emergency.h"

#include "core/ui/charts.h"
#include "core/world/game.h"
#include "core/world/rooms.h"

namespace th
{

namespace
{

// The player call offering an emergency, and the one about an epidemic.
constexpr uint16_t kCallEmergency = 2;
constexpr uint16_t kCallEpidemic = 3;

// Turns in the months emergencies and earthquakes are scheduled by.
constexpr uint32_t kTurnsPerMonth = 1500;

uint32_t month_of(const Date &date)
{
    return uint32_t(date.year * 12 + date.month);
}

// check_emergency_criteria (0xaf118) for a one-player game.
bool check_emergency_criteria(const Disasters &d, const World &world, bool open, uint32_t game_turn)
{
    if (game_turn < d.next_emergency_turn || !d.emergency_pending)
        return false;
    if (check_playercall(world, kCallEpidemic, 0, 0) || !open)
        return false;
    return d.emergencies_active < 4;
}

// setup_emergency_stats (0xaf448) for the player's offer, in a free slot
// (one is, with fewer than four offers outstanding).
void setup_emergency_stats(Disasters &d, uint16_t call)
{
    for (Disasters::Offer &offer : d.offers) {
        if (!offer.used) {
            offer = {call, d.emergency_bonus, 1, d.emergency_illness, d.emergency_patients, d.emergency_win_percent};
            break;
        }
    }
    d.emergencies_active++;
}

} // namespace

void setup_next_emergency(Disasters &d, World &world, const Balance &settings, const Date &date)
{
    uint32_t month = month_of(date);
    d.emergency_pending = false;
    for (size_t i = 0; i < d.emergency_used.size(); i++) {
        if (d.emergency_used[i])
            continue;
        d.emergency_used[i] = 1;
        const Balance::Emergency &e = settings.emergency_control[i];
        if (e.end_month > month && e.end_month > e.start_month) {
            uint32_t r = old_newrand(world.seed) % ((e.end_month + 1u - e.start_month) * kTurnsPerMonth);
            d.next_emergency_turn = r + e.start_month * kTurnsPerMonth;
            d.emergency_illness = e.illness;
            r = old_newrand(world.seed) % uint32_t(e.max + 1 - e.min);
            d.emergency_patients = uint8_t(r + e.min);
            d.emergency_win_percent = e.perc_win;
            d.emergency_bonus = e.bonus;
            d.emergency_pending = true;
            break;
        }
    }
}

void setup_next_quake(Disasters &d, World &world, const Balance &settings, const Date &date)
{
    uint32_t month = month_of(date);
    d.quake_pending = false;
    for (size_t i = 0; i < d.quake_used.size(); i++) {
        if (d.quake_used[i])
            continue;
        d.quake_used[i] = 1;
        const Balance::Quake &q = settings.quake_control[i];
        if (q.end_month > month && q.end_month > q.start_month) {
            uint32_t offset = old_newrand(world.seed) % ((q.end_month + 1u - q.start_month) * kTurnsPerMonth);
            d.next_quake_turn = offset + q.start_month * kTurnsPerMonth;
            d.quake_severity = uint8_t(q.severity);
            d.quake_pending = true;
            break;
        }
    }
}

void try_to_generate_emergency(Disasters &d, World &world, const Balance &settings, const Date &date, bool open,
                               uint32_t game_turn)
{
    if (!check_emergency_criteria(d, world, open, game_turn))
        return;
    if (world.hospital && world.hospital->illnesses[d.emergency_illness % kIllnessCount].discovered == 1)
        setup_emergency_stats(d, make_a_playercall(world, kCallEmergency, 0, 0x14, 0));
    setup_next_emergency(d, world, settings, date);
}

} // namespace th
