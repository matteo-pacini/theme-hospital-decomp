#include "snd.h"

extern "C" {
extern u32 SoundDriver;
extern u16 NumberOfSongs;
extern s32 CurrentSoundMasterVolume;
extern s32 CurrentMusicMasterVolume;
extern u32 MusicDriver;

// Returns the number of samples in the loaded sound bank.
// FUNCTION: 0x000c23c0
u16 GetNumberOfSamples(void)
{
    return NumberOfSamples;
}

// Returns nonzero if sound effects may be used (not disabled).
// FUNCTION: 0x000c23d0
u8 GetSoundAble(void)
{
    return SoundAble;
}

// Returns nonzero if music may be used (not disabled).
// FUNCTION: 0x000c23e0
u8 GetMusicAble(void)
{
    return MusicAble;
}

// Returns nonzero if sound effects are switched on.
// FUNCTION: 0x000c23f0
u8 GetSoundActive(void)
{
    return SoundActive;
}

// Returns nonzero if music is switched on.
// FUNCTION: 0x000c2400
u8 GetMusicActive(void)
{
    return MusicActive;
}

// Returns nonzero if a sound effects driver is installed.
// FUNCTION: 0x000c2410
u8 GetSoundInstalled(void)
{
    return SoundInstalled;
}

// Returns nonzero if a music driver is installed.
// FUNCTION: 0x000c2420
u8 GetMusicInstalled(void)
{
    return MusicInstalled;
}

// Returns nonzero if sound is played in stereo.
// FUNCTION: 0x000c2430
u8 GetStereoSound(void)
{
    return StereoSound;
}

// Returns the number of songs loaded.
// FUNCTION: 0x000c2440
u16 GetNumberOfSongs(void)
{
    return NumberOfSongs;
}

// Returns the current sound effects master volume.
// FUNCTION: 0x000c2450
s32 GetCurrentSoundMasterVolume(void)
{
    return CurrentSoundMasterVolume;
}

// Returns the current music master volume.
// FUNCTION: 0x000c2460
s32 GetCurrentMusicMasterVolume(void)
{
    return CurrentMusicMasterVolume;
}

// Returns the Miles digital driver handle.
// FUNCTION: 0x000c2470
u32 GetSoundDriver(void)
{
    return SoundDriver;
}

// Returns the Miles MIDI driver handle.
// FUNCTION: 0x000c2480
u32 GetMusicDriver(void)
{
    return MusicDriver;
}

// Returns the first entry of the sample voice table.
// FUNCTION: 0x000c2490
SampleInfo *GetFirstSampleInfoStructure(void)
{
    return sample_id;
}

// Returns the last entry of the sample voice table (inclusive).
// FUNCTION: 0x000c24a0
SampleInfo *GetLastSampleInfoStructure(void)
{
    return end_sample_id;
}

// Returns the Miles sequence handle songs play on.
// FUNCTION: 0x000c24b0
u32 GetSongHandle(void)
{
    return SongHandle;
}

}
