#include "snd.h"

extern "C" {

// Fades the playing song to volume (0-127) over rate milliseconds.
// FUNCTION: 0x000ce290
void FadeMusic(u16 rate, u8 volume)
{
    if (MusicInstalled && MusicAble && MusicActive && SongCurrentlyPlaying && volume <= 127) {
        if (AIL_sequence_status(SongHandle) != SEQ_DONE)
            AIL_set_sequence_volume(SongHandle, volume, rate);
    }
}

}
