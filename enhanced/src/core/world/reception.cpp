#include "core/world/reception.h"

namespace th
{

// Stubs until queues are ported: no patient queues, so none is called.

uint16_t reception_queue_head(const World &, uint16_t)
{
    return 0;
}

uint16_t reception_queue_next(const World &, uint16_t)
{
    return 0;
}

bool reception_emergency(const World &, uint16_t)
{
    return false;
}

int16_t all_queue_capacity(const World &, uint16_t, bool, uint8_t)
{
    return 0;
}

uint16_t call_person_to_room(World &, const AnimData &, const PersonComplexes &, uint16_t, uint16_t, uint16_t, uint16_t,
                             bool, uint32_t)
{
    return 0;
}

uint8_t call_person_again(World &, const AnimData &, const PersonComplexes &, uint16_t, uint16_t, uint16_t, uint32_t)
{
    return 0;
}

bool forget_about_my_slave(World &, const AnimData &, const PersonComplexes &, uint16_t, uint16_t, uint32_t)
{
    return true;
}

void interrupt_person(World &, const AnimData &, const PersonComplexes &, uint16_t, uint32_t)
{
}

bool reception_cure_available(const World &, uint16_t)
{
    return false;
}

void reception_serves_patient(World &, uint16_t, uint16_t)
{
}

} // namespace th
