// Private to the rooms*.cpp files: what the parts of rooms.cpp share.
#pragma once

#include "core/world/game.h"
#include "core/world/world.h"

namespace th
{

inline namespace rooms_detail
{

// Illnesses past the cures: the diagnosis rooms' treatments start here.
constexpr uint8_t kFirstDiagnosis = 0x24;

// Thing.person_flags bits (the game's PF_*) besides core/world/queue.h's.
constexpr uint32_t kPfWrongGuess = 0x800;

// The player call kinds about patients: a missing treatment room (4) and an
// unfinished diagnosis (8).
constexpr uint16_t kCallNoTreatment = 4;
constexpr uint16_t kCallNoDiagnosis = 8;
// The call announcing an illness discovered.
constexpr uint16_t kCallNewIllness = 7;

// The game's diag_register (src/tables.cpp): per illness, the diagnosis
// progress each diagnosis room (from treatment 0x24 on) adds; a room that
// takes some away also costs 5 health.
constexpr std::array<std::array<int16_t, 8>, 36> kDiagRegister = {{
    {},
    {},
    {400, 0, 0, 0, 0, 0, 200},
    {200, 0, 0, 0, 0, 0, 400},
    {0, 0, 0, 0, 0, 0, 200, 400},
    {0, 400, 0, -200},
    {-200, 400, 0, -400},
    {400, 0, 200, 0, 0, 0, 0, -200},
    {200, 400, 0, 200, 200, 0, 0, -400},
    {0, 0, 200, 400},
    {0, 400, 0, 0, 0, 0, 200, -200},
    {0, 400},
    {0, 0, 0, 0, 0, 400},
    {400, 0, 0, 0, 0, 0, 200},
    {},
    {0, 0, 0, 0, 0, 0, 400},
    {0, 0, 0, 0, 0, 400},
    {0, 0, 0, 0, 400},
    {0, 0, 0, 400},
    {0, 0, 0, 200, 400},
    {0, 0, 400, 0, 200},
    {400, 0, 0, 0, 200},
    {0, 0, 0, 0, 0, 0, 0, 400},
    {0, 200, 0, 0, 0, 0, 0, 400},
    {0, 0, 400},
    {400, 0, 0, 0, 0, 0, 200},
    {0, 0, 0, 0, 0, 0, 0, 400},
    {0, 400},
    {0, 0, 0, 0, 0, 0, 400},
    {0, 0, 0, 0, 0, 400, 200},
    {200, 0, 0, 400},
    {0, 0, -400, 0, 0, 0, 0, 400},
    {0, 0, 0, 0, 0, 0, 400},
    {0, 0, 0, 0, 400},
    {0, 0, 0, 200, 400},
    {400},
}};

inline uint16_t cell_of(const Thing &t)
{
    return uint16_t(t.x.tile + (t.y.tile << 7));
}

// The player's settings, or the game's starting ones before
// start_rooms_in_use.
inline const HospitalSettings &hospital_of(const World &world)
{
    static const HospitalSettings statics;
    return world.hospital ? *world.hospital : statics;
}

inline const PersonComplexes &person_complexes_of(const World &world)
{
    static const PersonComplexes none;
    return world.person_complexes ? *world.person_complexes : none;
}

// rooms.cpp
void turned_away(World &world, const Thing &patient);
int surgeon_count(const World &world, uint8_t owner);

// rooms_diagnosis.cpp
bool treatment_already_done(const Thing &patient, uint8_t treatment);
void best_guess_treatment(World &world, Thing &p);

} // namespace rooms_detail

} // namespace th
