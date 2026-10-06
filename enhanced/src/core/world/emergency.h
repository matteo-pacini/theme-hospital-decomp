#pragma once

#include "core/data/balance.h"
#include "core/world/world.h"

#include <array>
#include <cstdint>

namespace th
{

struct Date;

// Emergencies and earthquakes as far as the game schedules and offers them
// (src/emerg.cpp, src/quake.cpp): the level's Balance::emergency_control
// and quake_control entries picked in turn, at random turns within their
// months, and the emergencies offered to the player by a call. Accepting an
// emergency (its helicopter and patients) and the earthquakes themselves
// (generate_quake) are not ported.
struct Disasters {
    // The next emergency (gamevar.next_emergency_turn and on): the turn it
    // is due, its illness, patients, cure percentage to win and bonus per
    // patient; pending while one is scheduled.
    uint32_t next_emergency_turn = 0;
    uint8_t emergency_illness = 0;
    uint8_t emergency_patients = 0;
    uint8_t emergency_win_percent = 0;
    uint16_t emergency_bonus = 0;
    bool emergency_pending = false;
    // The emergency_control entries taken (gamevar.emergency_used).
    std::array<uint8_t, 20> emergency_used{};
    // The offers made to the player (gamevar.emergency_calls): the call, its
    // terms, and whether the slot is used; and the offers outstanding
    // (gamevar.emergencies_active).
    struct Offer {
        uint16_t call = 0;
        uint16_t bonus = 0;
        uint8_t used = 0;
        uint8_t illness = 0;
        uint8_t count = 0;
        uint8_t win_percent = 0;
    };
    std::array<Offer, 4> offers{};
    uint8_t emergencies_active = 0;
    // The next earthquake (gamevar.next_quake_turn and on) and the
    // quake_control entries taken.
    uint32_t next_quake_turn = 0;
    uint8_t quake_severity = 0;
    bool quake_pending = false;
    std::array<uint8_t, 16> quake_used{};
};

// setup_next_emergency (0xaf2c0): schedules the first emergency_control
// entry not yet taken that ends after the month of `date`: its turn (1500 a
// month, drawn from world.seed within its months), illness, patients (drawn
// between its Min and Max), and terms. Taken entries that end too soon are
// passed over; with none left, none is pending.
void setup_next_emergency(Disasters &d, World &world, const Balance &settings, const Date &date);

// setup_next_quake (0x43d88): likewise the next quake_control entry, its
// turn drawn from world.seed.
void setup_next_quake(Disasters &d, World &world, const Balance &settings, const Date &date);

// try_to_generate_emergency (0xaeec0), daily for hospital 0, `open` or not
// (allhosps.hospital_open): once the scheduled emergency is due
// (check_emergency_criteria: open, no epidemic call waiting, fewer than four
// offers outstanding), it is offered by a call (kind 2, expiring after 20
// days; setup_emergency_stats) if hospital 0 has discovered its illness,
// and the next one is scheduled. The emergency's helicopter, the countdowns
// of accepted ones and epidemics are not ported.
void try_to_generate_emergency(Disasters &d, World &world, const Balance &settings, const Date &date, bool open,
                               uint32_t game_turn);

} // namespace th
