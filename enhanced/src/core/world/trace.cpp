#include "core/world/trace.h"

#include "core/world/game.h"

#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <string_view>

namespace th
{

namespace
{

thread_local EventLog *current_log = nullptr;
thread_local uint32_t current_turn = 0;

void record(std::string text)
{
    current_log->events.push_back({current_turn, std::move(text)});
    while (current_log->limit && current_log->events.size() > current_log->limit)
        current_log->events.pop_front();
}

} // namespace

EventScope::EventScope(EventLog *log, uint32_t turn) : prev_log_(current_log), prev_turn_(current_turn)
{
    current_log = log && log->enabled ? log : nullptr;
    current_turn = turn;
}

EventScope::~EventScope()
{
    current_log = prev_log_;
    current_turn = prev_turn_;
}

void trace_event(const char *format, ...)
{
    if (!current_log)
        return;
    char text[256];
    va_list args;
    va_start(args, format);
    std::vsnprintf(text, sizeof text, format, args);
    va_end(args);
    record(text);
}

bool tracing_rand()
{
    return current_log && current_log->rand;
}

void trace_rand(const char *function, uint32_t seed)
{
    if (!tracing_rand())
        return;
    // "uint16_t th::{anonymous}::process(const Run&, uint16_t)" and, in a
    // lambda, "th::f(int)::<lambda()>": the name before the first '('.
    std::string_view name = function;
    name = name.substr(0, name.find('('));
    size_t start = name.size();
    while (start > 0 && (std::isalnum(static_cast<unsigned char>(name[start - 1])) || name[start - 1] == '_'))
        start--;
    char text[160];
    std::snprintf(text, sizeof text, "rand %.*s %08x", int(name.size() - start), name.data() + start, unsigned(seed));
    record(text);
}

std::string trace_turn(const Game &game, const EventLog *events)
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
    if (events)
        for (const TraceEvent &e : events->events)
            out += "E " + e.text + "\n";
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
