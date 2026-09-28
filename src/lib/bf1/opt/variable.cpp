#include "snd.h"

extern "C" {
extern u32 SoundDriver;

// FUNCTION: 0x000c23d0
u8 GetSoundAble(void)
{
    return SoundAble;
}

// FUNCTION: 0x000c23e0
u8 GetMusicAble(void)
{
    return MusicAble;
}

// FUNCTION: 0x000c2410
u8 GetSoundInstalled(void)
{
    return SoundInstalled;
}

// FUNCTION: 0x000c2420
u8 GetMusicInstalled(void)
{
    return MusicInstalled;
}

// FUNCTION: 0x000c2470
u32 GetStereoSound(void)
{
    return SoundDriver;
}

// FUNCTION: 0x000c2490
SampleInfo *GetNumberOfSongs(void)
{
    return sample_id;
}

// FUNCTION: 0x000c24a0
SampleInfo *GetCurrentSoundMasterVolume(void)
{
    return end_sample_id;
}

// FUNCTION: 0x000c24b0
u32 GetSongHandle(void)
{
    return SongHandle;
}

}
