#include "snd.h"

u32 build_number = 8;
u32 build_date = 50297;
extern long ForceSoundPlaybackOverride;
extern long ForceMidiPlaybackOverride;

extern "C" {
extern u8 DebugAudio;
extern u32 MaxNumberOfSamples;
extern u8 AutoScanForSoundHardware;
extern s32 SystemPlaybackRate;
extern char MusicDataPath[];
extern char SoundDataPath[];
extern s32 MidiMasterVolumeOverride;
extern s32 SoundMasterVolumeOverride;
extern s32 RedbookVolumeOverride;
extern s32 MidiTypeOverride;
extern u16 SoundType;
extern u8 UseMultiMediaExtensions;
extern u8 DisableLoadSounds;
extern u8 UseCurrentAwe32Soundfont;
extern char MusicType[];
extern u8 DisableLoadMusic;
extern u8 DisableDangerMusic;
extern char SoundInstallChoice[];
extern char D_0018c8b4[];

char *getenv(const char *name);
char *strcpy(char *dst, const char *src);
#pragma intrinsic(strcpy)
void InitSound(void);
void InitMusic(void);
void InitAllBullfrogSoundTimers(void);
void fn_000bbb50(void);
void fn_000bbb80(void);

// FUNCTION: 0x000cd840
void InitAudio(AudioInitOptions *ao)
{
    char *env;
    s32 value;

    if (getenv("BF_DEBUG_AUDIO"))
        DebugAudio = 1;
    sprintf(SoundProgressMessage, "BUILD %d compiled %d\n", build_number, build_date);
    SoundProgressLog(SoundProgressMessage);
    if ((env = getenv("BF_FORCE_MAX_SAMPLES")) != 0) {
        ao->MaxSamples = atoi(env);
        sprintf(SoundProgressMessage, "System override - ForceMaxNumberOfSamples %d\n", ao->MaxSamples);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv("BF_FORCE_SOUND_TYPE")) != 0) {
        ao->SoundType = atoi(env);
        sprintf(SoundProgressMessage, "System override - ForceSoundType %d\n", ao->SoundType);
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv("BF_FORCE_SOUND_DISABLE")) {
        SoundAble = 0;
        sprintf(SoundProgressMessage, "System override - ForceSoundDisable \n");
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv("BF_FORCE_MUSIC_DISABLE")) {
        MusicAble = 0;
        sprintf(SoundProgressMessage, "System override - ForceMusicDisable \n");
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv("BF_FORCE_REDBOOK_DISABLE")) {
        ao->unknown19 = 0;
        sprintf(SoundProgressMessage, "System override - ForceRedbookDisable \n");
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv("BF_FORCE_DANGER_MUSIC_DISABLE")) {
        ao->DisableDangerMusic = 1;
        sprintf(SoundProgressMessage, "System override - ForceDangerMusicDisable\n");
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv("BF_FORCE_LOAD_SOUNDS_DISABLE")) {
        ao->DisableLoadSounds = 1;
        sprintf(SoundProgressMessage, "System override - ForceLoadSoundsDisable\n");
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv("BF_FORCE_LOAD_MUSIC_DISABLE")) {
        ao->DisableLoadMusic = 1;
        sprintf(SoundProgressMessage, "System override - ForceLoadMusicDisable\n");
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv("BF_FORCE_SOUNDFONT_LOAD_DISABLE")) {
        ao->UseCurrentAwe32Soundfont = 1;
        sprintf(SoundProgressMessage, "System override - ForceSoundfontLoadDisable\n");
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv("BF_FORCE_SOUND_MASTER_VOLUME")) != 0) {
        SoundMasterVolumeOverride = value = atoi(env);
        sprintf(SoundProgressMessage, "System override - ForceSoundMasterVolume %d\n", value);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv("BF_FORCE_MIDI_MASTER_VOLUME")) != 0) {
        MidiMasterVolumeOverride = value = atoi(env);
        sprintf(SoundProgressMessage, "System override - ForceMidiMasterVolume %d\n", value);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv("BF_FORCE_REDBOOK_VOLUME")) != 0) {
        RedbookVolumeOverride = value = atoi(env);
        sprintf(SoundProgressMessage, "System override - ForceRedbookVolume %d\n", value);
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv("BF_FORCE_MONO")) {
        ao->StereoCheck = 0;
        sprintf(SoundProgressMessage, "System override - ForceMono\n");
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv("BF_FORCE_SYSTEM_PLAYBACK_RATE")) != 0) {
        SystemPlaybackRate = value = atoi(env);
        sprintf(SoundProgressMessage, "System override - ForceSystemPlaybackRate %d\n", value);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv("BF_FORCE_SOUND_DRIVER_PATH")) != 0) {
        strcpy(ao->SoundDriverPath, env);
        sprintf(SoundProgressMessage, "System override - ForceSoundDriverPath %s\n", env);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv("BF_FORCE_SOUND_DATA_PATH")) != 0) {
        strcpy(ao->SoundDataPath, env);
        sprintf(SoundProgressMessage, "System override - ForceSoundDataPath %s\n", env);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv("BF_FORCE_MUSIC_DATA_PATH")) != 0) {
        strcpy(ao->MusicDataPath, env);
        sprintf(SoundProgressMessage, "System override - ForceMusicDataPath %s\n", env);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv("BF_FORCE_MIDI_TYPE")) != 0) {
        MidiTypeOverride = 1;
        strcpy(MusicType, env);
        sprintf(SoundProgressMessage, "System override - ForceMidiType %s\n", env);
        SoundProgressLog(SoundProgressMessage);
    }
    if (ao->SoundDriverPath)
        AIL_set_driver_directory(ao->SoundDriverPath);
    strcpy(MusicDataPath, "sound");
    strcpy(SoundDataPath, "sound");
    if (ao->SoundDataPath)
        strcpy(SoundDataPath, ao->SoundDataPath);
    if (ao->MusicDataPath)
        strcpy(MusicDataPath, ao->MusicDataPath);
    MaxNumberOfSamples = ao->MaxSamples;
    SoundType = ao->SoundType;
    if (!ao->AutoScan)
        AutoScanForSoundHardware = 0;
    if (!ao->StereoCheck)
        StereoSound = 0;
    if (ao->DisableDangerMusic == 1)
        DisableDangerMusic = 1;
    if (ao->DisableLoadSounds == 1)
        DisableLoadSounds = 1;
    if (ao->DisableLoadMusic == 1)
        DisableLoadMusic = 1;
    if (ao->UseCurrentAwe32Soundfont == 1)
        UseCurrentAwe32Soundfont = 1;
    if (ao->UseMultiMediaExtensions == 1)
        UseMultiMediaExtensions = 1;
    sprintf(SoundProgressMessage, "BF1  - MA   %d\n", MusicAble);
    SoundProgressLog(SoundProgressMessage);
    sprintf(SoundProgressMessage, "BF2  - SA   %d\n", SoundAble);
    SoundProgressLog(SoundProgressMessage);
    if (!(ao->InitRedbookAudio & 1) && !ForceMidiPlaybackOverride)
        MusicAble = 0;
    if (!(ao->InitRedbookAudio & 2) && !ForceSoundPlaybackOverride)
        SoundAble = 0;
    InitSound();
    InitMusic();
    InitAllBullfrogSoundTimers();
    if (ao->unknown18 == 1)
        fn_000bbb50();
    if (ao->unknown19 == 1)
        fn_000bbb80();
    sprintf(SoundProgressMessage, "BF54 - MA   %d\n", MusicAble);
    SoundProgressLog(SoundProgressMessage);
    sprintf(SoundProgressMessage, "BF55 - SA   %d\n", SoundAble);
    SoundProgressLog(SoundProgressMessage);
    sprintf(SoundProgressMessage, "BF56 - music driver = %s\n", D_0018c8b4);
    SoundProgressLog(SoundProgressMessage);
    sprintf(SoundProgressMessage, "BF57 - sound driver = %s\n", &SoundInstallChoice[0x80]);
    SoundProgressLog(SoundProgressMessage);
}

}
