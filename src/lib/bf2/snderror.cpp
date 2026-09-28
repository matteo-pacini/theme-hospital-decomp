#include "snd.h"

// FUNCTION: 0xeb590
void SoundProgressLog(char *msg)
{
    if (DebugAudio)
        printf(msg);
}
