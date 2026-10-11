#pragma once

#include <cstdint>

namespace th
{

class AnimData;
struct Thing;
struct World;

// The status icons (ICON_* in include/enums.h), the game's icon_table index.
enum : int8_t {
    kIconHealth1 = 1,
    kIconHealth2 = 2,
    kIconHealth3 = 3,
    kIconHealth4 = 4,
    kIconHealth5 = 5,
    kIconEmergency = 6,
    kIconFedUp = 7,
    kIconUnhappy = 8,
    kIconLeaving = 9,
    kIconTired = 0xa,
    kIconToilet = 0xb,
    kIconThirsty = 0xc,
    kIconContagious = 0xd,
    kIconVaccineRequested = 0xe,
    kIconVaccineNurseSent = 0xf,
    kIconVaccinated = 0x10,
    kIconTooHot = 0x11,
    kIconTooCold = 0x12,
    kIconTooExpensive = 0x13,
    kIconGuessed = 0x14,
    kIconDiagnosed = 0x15,
    kIconKilled = 0x16,
    kIconCured = 0x17,
    kIconNoDiagnosis = 0x18,
    kIconCalling = 0x19,
    kIconQueue = 0x1a,
    kIconAwaitingDecision = 0x1b,
    kIconNeedsRepair = 0x1c,
};

// The status icons (ICON_* in include/enums.h) shown over a person's or a
// machine's head. The game's icon_table (src/icons.cpp): priority, the
// start[] animation, how long the icon lasts and how set_icon_value treats
// it (mode < 0 waits for peep_icon to announce it, 0 shows it for its time,
// > 0 keeps it until reset).
struct IconDef {
    uint8_t priority;
    uint16_t anim;
    uint16_t duration;
    int8_t mode;
};

extern const IconDef kIconTable[30];

// set_icon_value (0xb0300): shows status icon `icon` (an ICON_* index) over
// `peep`, unless a dead patient, or a more important icon (lower priority) is
// already showing.
void set_icon_value(const AnimData &anims, Thing &peep, int8_t icon);

// reset_icon_value (0xb0428): removes `peep`'s icon if it is `icon` (either
// sign).
void reset_icon_value(Thing &peep, int8_t icon);

// peep_icon (0xb0474): announces `peep`'s pending icon (stored negative) with
// a sound, making it show and starting its timer for announced kinds. Called
// as the mouse passes over a person.
void peep_icon(World &world, Thing &peep);

// update_icon_counter (0xb0540): animates `peep`'s icon and counts down its
// time, removing it when the time runs out; permanent icons just animate.
void update_icon_counter(const AnimData &anims, Thing &peep);

// clear_peep_icon (0xb05f0): removes `peep`'s icon.
void clear_peep_icon(Thing &peep);

} // namespace th
