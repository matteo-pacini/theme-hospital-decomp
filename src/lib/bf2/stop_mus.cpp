#include "snd.h"

// FUNCTION: 0xfb730
// NONMATCHING: constant stores go through registers instead of immediates
void StopMusic(void)
{
    if (MusicInstalled && MusicAble && MusicActive && SongCurrentlyPlaying) {
        if (DangerMusicFadeActive)
            AIL_release_timer_handle(DangerMusicFadeHandle);
        DangerMusicAble = 0;
        DangerMusicFadeActive = 0;
        CurrentDangerMusicFadeDirection = 1;
        DangerMusicVolume = 0;
        DangerMusicVolumeChange = 0xff;
        if (AIL_sequence_status(SongHandle) != 2) {
            AIL_stop_sequence(SongHandle);
            AIL_end_sequence(SongHandle);
        }
        SongCurrentlyPlaying = 0;
    }
}
