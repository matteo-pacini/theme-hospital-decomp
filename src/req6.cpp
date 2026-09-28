#include "hosp.h"

extern "C" {

extern u8 D_00155e07, D_00155e08;

}

// FUNCTION: 0x0008ee10
void stop_midi(void)
{
    u32 song;

    song = GetSongHandle();
    D_00155e07 = 0;
    AIL_end_sequence(song);
    D_00155e08 = 0;
}
