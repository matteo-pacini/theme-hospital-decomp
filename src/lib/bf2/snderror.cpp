#include "snd.h"

// Prints msg to the console when audio debugging (BF_DEBUG_AUDIO) is on.
// FUNCTION: 0xeb590
void SoundProgressLog(char *msg)
{
    if (DebugAudio)
        printf(msg);
}
