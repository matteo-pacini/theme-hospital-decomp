#include "core/world/trace.h"

#include "core/world/game.h"

#include <cstdio>

namespace th
{

std::string trace_turn(const Game &game)
{
    const World &w = game.world;
    int patients = 0;
    for (size_t i = 1; i < w.things.size(); i++)
        patients += w.things[i].type == kThingPatient;

    std::string out;
    char line[160];
    std::snprintf(line, sizeof line, "T %u seed=%08x date=%d/%d/%d balance=%d patients=%d\n", unsigned(game.turn_count),
                  unsigned(w.seed), game.date.year, game.date.month, game.date.day, game.balance, patients);
    out += line;
    for (size_t h = 0; h < game.arrivals.hospitals.size(); h++) {
        const HospitalMarket &m = game.arrivals.hospitals[h];
        // Entries 1-45: entry 0 is no illness (the game leaves its reputation 0).
        long rep = 0, price = 0;
        for (size_t i = 1; i < 46; i++) {
            rep += m.reputation[i];
            price += m.price[i];
        }
        bool open = h == 0 ? game.open : m.open;
        std::snprintf(line, sizeof line, "H %zu open=%d rep=%ld price=%ld\n", h, int(open), rep, price);
        out += line;
    }
    for (size_t i = 1; i < w.things.size(); i++) {
        const Thing &t = w.things[i];
        if (t.type == kThingFree)
            continue;
        std::snprintf(line, sizeof line, " %zu type=%u sub=%u state=%u x=%04x y=%04x\n", i, unsigned(t.type),
                      unsigned(t.subtype), unsigned(t.state), unsigned(t.x.tile << 8 | t.x.frac),
                      unsigned(t.y.tile << 8 | t.y.frac));
        out += line;
    }
    return out;
}

} // namespace th
