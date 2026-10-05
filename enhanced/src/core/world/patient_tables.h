#pragma once

#include <array>
#include <cstdint>

namespace th
{

// The game's tables for making patients (make_a_peep, effect_illness,
// add_contagiousness; src/peepmake.cpp, src/examine.cpp), copied from
// src/tables.cpp and src/array.cpp.

// generate_person[] (PeepType in include/hosp.h): a kind of visitor, its
// heads and, per head, its kVariations entry.
struct PeepType {
    uint8_t kind;                   // Thing.subtype
    uint8_t count;                  // heads used
    std::array<uint8_t, 5> head;    // Thing.layers[0]
    std::array<uint8_t, 5> palette; // kVariations index, per head
    uint8_t weight;
};

constexpr std::array<PeepType, 5> kGeneratePerson = {{
    {0, 3, {1, 2, 3, 0, 0}, {0, 0, 0, 0, 0}, 33},
    {0, 1, {4, 0, 0, 0, 0}, {2, 0, 0, 0, 0}, 33},
    {0, 1, {5, 0, 0, 0, 0}, {3, 0, 0, 0, 0}, 33},
    {1, 3, {1, 2, 3, 0, 0}, {1, 1, 1, 0, 0}, 50},
    {1, 1, {4, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, 50},
}};

// variations[] (PeepPalette): the choices for Thing.layers[1..3]. The game's
// layer3 is 11 bytes long; past count3 they are never read and are left out.
struct PeepPalette {
    uint8_t count1;
    std::array<uint8_t, 5> layer1;
    uint8_t count2;
    std::array<uint8_t, 5> layer2;
    uint8_t count3;
    std::array<uint8_t, 5> layer3;
};

constexpr std::array<PeepPalette, 4> kVariations = {{
    {4, {0, 1, 2, 3, 0}, 4, {0, 1, 2, 0, 0}, 4, {0, 1, 2, 3, 0}},
    {4, {0, 1, 2, 3, 0}, 4, {0, 0, 0, 0, 0}, 4, {0, 0, 0, 0, 0}},
    {4, {0, 1, 2, 3, 0}, 4, {0, 1, 0, 0, 0}, 4, {0, 1, 2, 3, 0}},
    {4, {0, 1, 2, 3, 0}, 4, {0, 1, 2, 0, 0}, 4, {0, 1, 2, 3, 0}},
}};

// suitable_illness[illness][i]: whether generate_person[i] may have visual
// illness `illness` (2-15).
constexpr std::array<std::array<uint8_t, 5>, 16> kSuitableIllness = {{
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0},
    {1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0},
    {1, 0, 0, 0, 0},
    {1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0},
    {1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1},
    {0, 0, 0, 1, 1},
    {1, 1, 1, 1, 1},
}};

// change_person_who[change][subtype]: the patient kind a kind becomes
// (transform_peep_who), -1 for none.
constexpr std::array<std::array<int8_t, 15>, 15> kChangePersonWho = {{
    {4, 5, -1, 4, -1, -1, -1, 4, -1, -1, -1, -1, -1, -1, -1},
    {2, 11, -1, 2, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1},
    {6, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, 1, -1, -1},
    {3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1},
    {8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1},
    {9, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, -1, -1, -1, -1},
    {13, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1},
}};

// expertise[].not_contagious (src/array.cpp; not a balance key): nonzero
// for illnesses never made contagious.
constexpr std::array<uint8_t, 47> kNotContagious = {
    1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// towns[] fields that are not balance keys (src/tables.cpp): a new patient
// is insured with a 1 in insurance_odds chance; special_kind is 1 for the
// rat levels (no patients), 2 for the training level.
struct TownExtra {
    uint16_t insurance_odds;
    uint8_t special_kind;
};

constexpr std::array<TownExtra, 16> kTownExtras = {{
    {3, 0},
    {3, 0},
    {3, 0},
    {10, 0},
    {5, 0},
    {5, 2},
    {5, 0},
    {5, 0},
    {5, 0},
    {5, 0},
    {5, 0},
    {5, 0},
    {5, 0},
    {5, 1},
    {5, 1},
    {5, 0},
}};

// gbv.peep_speed and gbv.peep_anim_speed (src/array.cpp; not balance keys):
// a new patient's Thing.speed and Thing.anim_speed.
constexpr uint8_t kPeepSpeed = 0x20;
constexpr uint8_t kPeepAnimSpeed = 1;

} // namespace th
