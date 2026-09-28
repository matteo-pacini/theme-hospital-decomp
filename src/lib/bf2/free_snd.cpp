#include "snd.h"

extern "C" {

// FUNCTION: 0xdc820
void fn_000dc820(void)
{
    if (!MusicInstalled)
        return;
    if (SongCurrentlyPlaying) {
        AIL_stop_sequence(SongHandle);
        AIL_end_sequence(SongHandle);
        SongCurrentlyPlaying = 0;
    }
    if (Awe32SoundfontLoaded == 1)
        FreeAwe32Soundfont();
    if (!SoundInstalled)
        AIL_shutdown();
    if (Music) {
        LbMemoryFree(Music);
        EndMusic = 0;
    }
    if (MusicData)
        LbMemoryFree(MusicData);
    MusicAble = 0;
    MusicActive = 0;
    MusicInstalled = 0;
}

// FUNCTION: 0xdc8d0
void fn_000dc8d0(void)
{
    if (!SoundInstalled)
        return;
    StopAllSamples();
    if (!MusicInstalled)
        AIL_shutdown();
    if (Sfx) {
        LbMemoryFree(Sfx);
        EndSfxs = 0;
    }
    if (SfxData)
        LbMemoryFree(SfxData);
    SoundAble = 0;
    SoundActive = 0;
    SoundInstalled = 0;
}

}
