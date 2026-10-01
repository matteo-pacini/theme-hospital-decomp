#include "snd.h"

extern "C" {

// Starts the MIDI song at address (replacing the one playing) at volume
// (127: left at full).
// FUNCTION: 0x000ce1b0
void PlayMidiFromAddress(u8 volume, void *address)
{
    if (MusicInstalled && MusicAble && MusicActive) {
        if (SongCurrentlyPlaying) {
            if (AIL_sequence_status(SongHandle) != 2) {
                AIL_stop_sequence(SongHandle);
                AIL_end_sequence(SongHandle);
            }
            SongCurrentlyPlaying = 0;
        }
        AIL_init_sequence(SongHandle, address, 0);
        if (volume < 127)
            AIL_set_sequence_volume(SongHandle, volume, 0);
        AIL_start_sequence(SongHandle);
        CurrentTempo = 100;
        SongCurrentlyPlaying = 1;
    }
}

}
