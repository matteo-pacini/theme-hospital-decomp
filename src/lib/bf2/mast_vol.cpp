#include "snd.h"

// Sets the music master volume (0-127), unless overridden by the
// environment.
// FUNCTION: 0xefed0
void SetMusicMasterVolume(s32 volume)
{
    if (MusicAble && MusicInstalled && volume != CurrentMusicMasterVolume && volume <= 127 && volume >= 0
        && !MidiMasterVolumeOverride) {
        AIL_set_XMIDI_master_volume(MusicDriver, volume);
        CurrentMusicMasterVolume = volume;
    }
}

// Sets the sound effects master volume (0-127), unless overridden by the
// environment.
// FUNCTION: 0xeff20
void SetSoundMasterVolume(s32 volume)
{
    if (SoundAble && SoundInstalled && volume != CurrentSoundMasterVolume && volume <= 127 && volume >= 0
        && !SoundMasterVolumeOverride) {
        AIL_set_digital_master_volume(SoundDriver, volume);
        CurrentSoundMasterVolume = volume;
    }
}
