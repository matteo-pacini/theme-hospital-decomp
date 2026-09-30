#include "snd.h"

extern "C" {
extern u32 SoundDriver;
extern u16 NumberOfSongs;
extern s32 CurrentSoundMasterVolume;
extern s32 CurrentMusicMasterVolume;
extern u32 MusicDriver;

// FUNCTION: 0x000c23c0
u16 GetNumberOfSamples(void)
{
    return NumberOfSamples;
}

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

// FUNCTION: 0x000c23f0
u8 GetSoundActive(void)
{
    return SoundActive;
}

// FUNCTION: 0x000c2400
u8 GetMusicActive(void)
{
    return MusicActive;
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

// FUNCTION: 0x000c2430
u8 GetStereoSound(void)
{
    return StereoSound;
}

// FUNCTION: 0x000c2440
u16 GetNumberOfSongs(void)
{
    return NumberOfSongs;
}

// FUNCTION: 0x000c2450
s32 GetCurrentSoundMasterVolume(void)
{
    return CurrentSoundMasterVolume;
}

// FUNCTION: 0x000c2460
s32 GetCurrentMusicMasterVolume(void)
{
    return CurrentMusicMasterVolume;
}

// FUNCTION: 0x000c2470
u32 GetSoundDriver(void)
{
    return SoundDriver;
}

// FUNCTION: 0x000c2480
u32 GetMusicDriver(void)
{
    return MusicDriver;
}

// FUNCTION: 0x000c2490
SampleInfo *GetFirstSampleInfoStructure(void)
{
    return sample_id;
}

// FUNCTION: 0x000c24a0
SampleInfo *GetLastSampleInfoStructure(void)
{
    return end_sample_id;
}

// FUNCTION: 0x000c24b0
u32 GetSongHandle(void)
{
    return SongHandle;
}

}
