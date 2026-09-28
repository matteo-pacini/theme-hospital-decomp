#include "snd.h"

extern "C" {
extern char D_00124728[];
extern char D_0012472e[];
extern char D_00124734[];

// FUNCTION: 0x000cd7f0
void SetupAudioOptionDefaults(AudioInitOptions *ao)
{
    ao->SoundDataPath = D_00124728;
    ao->MusicDataPath = D_0012472e;
    ao->SoundDriverPath = D_00124734;
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
