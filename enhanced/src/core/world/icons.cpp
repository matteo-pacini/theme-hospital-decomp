#include "core/world/icons.h"

#include "core/data/anims.h"
#include "core/world/world.h"

#include <cstddef>
#include <cstdlib>

namespace th
{

// icon_table (src/icons.cpp), indices ICON_* in include/enums.h: priority,
// the start[] animation, the duration in turns and the mode (see IconDef).
// Entry 0 is the "no icon" sentinel, its priority the lowest.
const IconDef kIconTable[30] = {
    {255, 0, 0, 0},       {20, 2000, 65535, 1}, {19, 2001, 65535, 1}, {18, 2002, 65535, 1}, {17, 2003, 65535, 1},
    {16, 2004, 65535, 1}, {30, 1957, 65535, 1}, {11, 2026, 65535, 1}, {35, 2289, 120, 1},   {29, 2288, 120, 1},
    {30, 1995, 120, -1},  {55, 1998, 120, -1},  {60, 1993, 120, -1},  {10, 2287, 65535, 1}, {9, 2285, 65535, 1},
    {9, 2286, 65535, 1},  {8, 2283, 120, 1},    {60, 1994, 120, -1},  {60, 1997, 120, -1},  {15, 2009, 120, 0},
    {15, 2010, 120, 0},   {15, 2022, 120, 0},   {15, 2023, 120, 0},   {1, 2024, 120, 0},    {19, 2025, 120, 0},
    {15, 2027, 120, 0},   {13, 2284, 2, 0},     {14, 2503, 120, 1},   {50, 2282, 120, 1},   {0, 0, 0, 0},
};

// SFX_HLIGHTP2 (core/audio/sounds.h): a person's status icon appears.
constexpr uint8_t kIconSfxHighlight = 0x01;

// start[anim].frame for an icon, or 0 when the animation has no frame.
uint16_t icon_start_frame(const AnimData &anims, const IconDef &def)
{
    return def.anim < anims.starts.size() ? uint16_t(anims.starts[def.anim].frame) : 0;
}

void set_icon_value(const AnimData &anims, Thing &peep, int8_t icon)
{
    if (peep.type != kThingPatient || peep.health > 0) {
        if (kIconTable[icon].priority <= kIconTable[size_t(std::abs(int(peep.icon_id)))].priority) {
            peep.icon_frame = icon_start_frame(anims, kIconTable[icon]);
            if (kIconTable[icon].mode < 0) {
                peep.icon_id = int8_t(-icon);
                peep.icon_timer = int16_t(kIconTable[icon].duration);
            } else if (kIconTable[icon].mode == 0) {
                peep.icon_id = icon;
                peep.icon_timer = int16_t(kIconTable[icon].duration);
            } else if (kIconTable[icon].mode > 0) {
                peep.icon_id = icon;
                peep.icon_timer = int16_t(0xffff);
            }
        }
    }
}

void reset_icon_value(Thing &peep, int8_t icon)
{
    if (peep.icon_id == std::abs(int(icon))) {
        peep.icon_id = 0;
        peep.icon_timer = 0;
        peep.icon_frame = 0;
    }
}

void peep_icon(World &world, Thing &peep)
{
    if (peep.type != kThingPatient || peep.health > 0) {
        if (peep.icon_id < 0)
            world.building.sounds.push_back(kIconSfxHighlight);
        peep.icon_id = int8_t(std::abs(int(peep.icon_id)));
        if (kIconTable[peep.icon_id].anim > 0 && kIconTable[peep.icon_id].mode == -1)
            peep.icon_timer = int16_t(kIconTable[peep.icon_id].duration);
    }
}

void update_icon_counter(const AnimData &anims, Thing &peep)
{
    if (peep.icon_id > 0) {
        if (int16_t(peep.icon_timer) > 0) {
            peep.icon_timer--;
            if (peep.icon_frame < anims.frames.size())
                peep.icon_frame = uint16_t(anims.frames[peep.icon_frame].next);
        }
        if (int16_t(peep.icon_timer) == 0)
            reset_icon_value(peep, peep.icon_id);
        else if (int16_t(peep.icon_timer) < 0 && peep.icon_frame < anims.frames.size())
            peep.icon_frame = uint16_t(anims.frames[peep.icon_frame].next);
    }
}

void clear_peep_icon(Thing &peep)
{
    peep.icon_id = 0;
    peep.icon_timer = 0;
    peep.icon_frame = 0;
}

} // namespace th
