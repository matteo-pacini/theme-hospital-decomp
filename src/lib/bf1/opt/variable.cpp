#include "snd.h"

extern "C" {
extern u32 SoundDriver;
extern u16 NumberOfSongs;
extern s32 D_00150824;
extern s32 CurrentMusicMasterVolume;
extern u32 MusicDriver;

// FUNCTION: 0x000c23c0
u16 fn_000c23c0(void)
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
u8 fn_000c23f0(void)
{
    return SoundActive;
}

// FUNCTION: 0x000c2400
u8 fn_000c2400(void)
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
u8 fn_000c2430(void)
{
    return StereoSound;
}

// FUNCTION: 0x000c2440
u16 fn_000c2440(void)
{
    return NumberOfSongs;
}

// FUNCTION: 0x000c2450
s32 fn_000c2450(void)
{
    return D_00150824;
}

// FUNCTION: 0x000c2460
s32 fn_000c2460(void)
{
    return CurrentMusicMasterVolume;
}

// FUNCTION: 0x000c2470
u32 GetStereoSound(void)
{
    return SoundDriver;
}

// FUNCTION: 0x000c2480
u32 fn_000c2480(void)
{
    return MusicDriver;
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
