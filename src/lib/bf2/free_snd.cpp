#include "snd.h"

extern "C" {

// Stops the song and shuts down music: frees the AWE32 soundfont and the
// music bank, and the Miles library if sound effects are not installed.
// FUNCTION: 0xdc820
void FreeMusic(void)
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

// Stops all samples and shuts down sound effects: frees the sample bank,
// and the Miles library if music is not installed.
// FUNCTION: 0xdc8d0
void FreeSound(void)
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
