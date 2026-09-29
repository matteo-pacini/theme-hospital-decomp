#include "hosp.h"

extern "C" {


}

// FUNCTION: 0x0008ee10
void stop_midi(void)
{
    u32 song;

    song = GetSongHandle();
    othervar.fa4f = 0;
    AIL_end_sequence(song);
    othervar.fa50 = 0;
}
