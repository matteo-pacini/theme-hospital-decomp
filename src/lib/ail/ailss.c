#include "ail.h"

// FUNCTION: 0x000e1110
S32 __cdecl AIL_API_sample_playback_rate(HSAMPLE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->playback_rate;
}

// FUNCTION: 0x000e1130
S32 __cdecl AIL_API_sample_volume(HSAMPLE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->volume;
}
