#include "snd.h"

extern "C" {

// FUNCTION: 0x000ce290
void FadeMusic(u16 rate, u8 volume)
{
    if (MusicInstalled && MusicAble && MusicActive && SongCurrentlyPlaying && volume <= 127) {
        if (AIL_sequence_status(SongHandle) != 2)
            AIL_set_sequence_volume(SongHandle, volume, rate);
    }
}

}
