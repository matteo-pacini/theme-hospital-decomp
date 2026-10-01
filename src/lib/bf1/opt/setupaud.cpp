#include "snd.h"

extern "C" {

// Fills ao with the default audio options ("Sound" paths, ten samples,
// sound and music on).
// FUNCTION: 0x000cd7f0
void SetupAudioOptionDefaults(AudioInitOptions *ao)
{
    ao->SoundDataPath = "Sound";
    ao->MusicDataPath = "Sound";
    ao->SoundDriverPath = "Sound";
    ao->InitRedbookAudio = 3;
    ao->AutoScan = 1;
    ao->StereoCheck = 1;
    ao->SoundType = 0x656;
    ao->MaxSamples = 10;
    ao->DisableDangerMusic = 0;
    ao->DisableLoadMusic = 0;
    ao->DisableLoadSounds = 0;
    ao->UseCurrentAwe32Soundfont = 0;
    ao->UseMultiMediaExtensions = 0;
    ao->unknown18 = 0;
    ao->unknown19 = 0;
}

}
