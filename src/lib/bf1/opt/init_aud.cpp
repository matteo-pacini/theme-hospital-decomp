#include "snd.h"

extern u32 build_number;
extern u32 build_date;
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
extern char D_0012473c[];
extern char D_0012474b[];
extern char D_00124761[];
extern char D_00124776[];
extern char D_001247a4[];
extern char D_001247b8[];
extern char D_001247dd[];
extern char D_001247f4[];
extern char D_0012481a[];
extern char D_00124831[];
extern char D_00124857[];
extern char D_00124870[];
extern char D_00124898[];
extern char D_001248b6[];
extern char D_001248e1[];
extern char D_001248fe[];
extern char D_00124928[];
extern char D_00124944[];
extern char D_0012496d[];
extern char D_0012498d[];
extern char D_001249ba[];
extern char D_001249d7[];
extern char D_00124a04[];
extern char D_00124a20[];
extern char D_00124a4c[];
extern char D_00124a64[];
extern char D_00124a8d[];
extern char D_00124a9b[];
extern char D_00124ab8[];
extern char D_00124ad6[];
extern char D_00124b04[];
extern char D_00124b1f[];
extern char D_00124b4a[];
extern char D_00124b63[];
extern char D_00124b8c[];
extern char D_00124ba5[];
extern char D_00124bce[];
extern char D_00124be1[];
extern char D_00124c05[];
extern char D_00124c0b[];
extern char D_00124c11[];
extern char D_00124c21[];
extern char D_00124c31[];
extern char D_00124c41[];
extern char D_00124c51[];
extern char D_00124c6b[];

char *getenv(const char *name);
char *strcpy(char *dst, const char *src);
#pragma intrinsic(strcpy)
void InitSound(void);
void InitMusic(void);
void fn_000eb5b0(void);
void fn_000bbb50(void);
void fn_000bbb80(void);

// NONMATCHING: byte register choice for the StereoSound/MusicAble/SoundAble stores
// FUNCTION: 0x000cd840
void InitAudio(AudioInitOptions *ao)
{
    char *env;
    s32 value;

    if (getenv(D_0012473c))
        DebugAudio = 1;
    sprintf(SoundProgressMessage, D_0012474b, build_number, build_date);
    SoundProgressLog(SoundProgressMessage);
    if ((env = getenv(D_00124761)) != 0) {
        ao->MaxSamples = atoi(env);
        sprintf(SoundProgressMessage, D_00124776, ao->MaxSamples);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv(D_001247a4)) != 0) {
        ao->SoundType = atoi(env);
        sprintf(SoundProgressMessage, D_001247b8, ao->SoundType);
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv(D_001247dd)) {
        SoundAble = 0;
        sprintf(SoundProgressMessage, D_001247f4);
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv(D_0012481a)) {
        MusicAble = 0;
        sprintf(SoundProgressMessage, D_00124831);
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv(D_00124857)) {
        ao->unknown19 = 0;
        sprintf(SoundProgressMessage, D_00124870);
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv(D_00124898)) {
        ao->DisableDangerMusic = 1;
        sprintf(SoundProgressMessage, D_001248b6);
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv(D_001248e1)) {
        ao->DisableLoadSounds = 1;
        sprintf(SoundProgressMessage, D_001248fe);
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv(D_00124928)) {
        ao->DisableLoadMusic = 1;
        sprintf(SoundProgressMessage, D_00124944);
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv(D_0012496d)) {
        ao->UseCurrentAwe32Soundfont = 1;
        sprintf(SoundProgressMessage, D_0012498d);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv(D_001249ba)) != 0) {
        SoundMasterVolumeOverride = value = atoi(env);
        sprintf(SoundProgressMessage, D_001249d7, value);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv(D_00124a04)) != 0) {
        MidiMasterVolumeOverride = value = atoi(env);
        sprintf(SoundProgressMessage, D_00124a20, value);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv(D_00124a4c)) != 0) {
        RedbookVolumeOverride = value = atoi(env);
        sprintf(SoundProgressMessage, D_00124a64, value);
        SoundProgressLog(SoundProgressMessage);
    }
    if (getenv(D_00124a8d)) {
        ao->StereoCheck = 0;
        sprintf(SoundProgressMessage, D_00124a9b);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv(D_00124ab8)) != 0) {
        SystemPlaybackRate = value = atoi(env);
        sprintf(SoundProgressMessage, D_00124ad6, value);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv(D_00124b04)) != 0) {
        strcpy(ao->SoundDriverPath, env);
        sprintf(SoundProgressMessage, D_00124b1f, env);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv(D_00124b4a)) != 0) {
        strcpy(ao->SoundDataPath, env);
        sprintf(SoundProgressMessage, D_00124b63, env);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv(D_00124b8c)) != 0) {
        strcpy(ao->MusicDataPath, env);
        sprintf(SoundProgressMessage, D_00124ba5, env);
        SoundProgressLog(SoundProgressMessage);
    }
    if ((env = getenv(D_00124bce)) != 0) {
        MidiTypeOverride = 1;
        strcpy(MusicType, env);
        sprintf(SoundProgressMessage, D_00124be1, env);
        SoundProgressLog(SoundProgressMessage);
    }
    if (ao->SoundDriverPath)
        AIL_set_driver_directory(ao->SoundDriverPath);
    strcpy(MusicDataPath, D_00124c05);
    strcpy(SoundDataPath, D_00124c0b);
    if (ao->SoundDataPath)
        strcpy(SoundDataPath, ao->SoundDataPath);
    if (ao->MusicDataPath)
        strcpy(MusicDataPath, ao->MusicDataPath);
    MaxNumberOfSamples = ao->MaxSamples;
    SoundType = ao->SoundType;
    if (!ao->AutoScan)
        AutoScanForSoundHardware = ao->AutoScan;
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
    sprintf(SoundProgressMessage, D_00124c11, MusicAble);
    SoundProgressLog(SoundProgressMessage);
    sprintf(SoundProgressMessage, D_00124c21, SoundAble);
    SoundProgressLog(SoundProgressMessage);
    if (!(ao->InitRedbookAudio & 1) && !ForceMidiPlaybackOverride)
        MusicAble = 0;
    if (!(ao->InitRedbookAudio & 2) && !ForceSoundPlaybackOverride)
        SoundAble = 0;
    InitSound();
    InitMusic();
    fn_000eb5b0();
    if (ao->unknown18 == 1)
        fn_000bbb50();
    if (ao->unknown19 == 1)
        fn_000bbb80();
    sprintf(SoundProgressMessage, D_00124c31, MusicAble);
    SoundProgressLog(SoundProgressMessage);
    sprintf(SoundProgressMessage, D_00124c41, SoundAble);
    SoundProgressLog(SoundProgressMessage);
    sprintf(SoundProgressMessage, D_00124c51, D_0018c8b4);
    SoundProgressLog(SoundProgressMessage);
    sprintf(SoundProgressMessage, D_00124c6b, &SoundInstallChoice[0x80]);
    SoundProgressLog(SoundProgressMessage);
}

}
