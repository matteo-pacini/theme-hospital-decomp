#include "snd.h"

extern "C" {
u32 MaxNumberOfSamples = 10;
u8 SoundAble = 1;
}
u8 SixteenBit = 1;
extern "C" {
u8 SoundActive = 1;
u8 AutoScanForSoundHardware = 1;
u8 StereoSound = 1;
volatile s8 CurrentSoundBank = -1;
s32 CurrentSoundMasterVolume = 127;
s32 SystemPlaybackRate = 22050;
}

/* Built without inline expansion: callees in this file stay calls. */
#pragma inline_depth(0)

// FUNCTION: 0xdc970
// NONMATCHING: MaxNumberOfSamples - 1 for end_sample_id is computed in ebx, not edx
void InitSound(void)
{
    AILIOParms io;
    char name[0x10];
    u8 ok = 0;
    u8 extra = 5;
    char *env;
    SampleInfo *smp;

    if (!SoundAble)
        return;
    sprintf(SoundProgressMessage, "BF3  - Init sound\n");
    SoundProgressLog(SoundProgressMessage);
    sprintf(SoundProgressMessage, "BF4  - Default sound type -  %d\n", SoundType);
    SoundProgressLog(SoundProgressMessage);
    if (SoundType == 0) {
        if (!MusicAble && AILStartupAlreadyInitiated) {
            AIL_shutdown();
            sprintf(SoundProgressMessage, "BF5  - No samples requested - AIL shutdown\n");
            SoundProgressLog(SoundProgressMessage);
        }
        SoundAble = 0;
        SoundActive = 0;
        sprintf(SoundProgressMessage, "BF6  - No samples requested  \n");
        SoundProgressLog(SoundProgressMessage);
        return;
    }
    if (!AILStartupAlreadyInitiated) {
        AIL_mem_use_malloc(LbMemoryAlloc);
        AIL_mem_use_free(LbMemoryFree);
        AIL_startup();
        AILStartupAlreadyInitiated = 1;
    }
    AIL_set_preference(9, 0);
    AIL_set_preference(0, 200);
    AIL_set_preference(8, 1);
    AIL_set_preference(1, SystemPlaybackRate);
    AIL_set_preference(3, 100);
    AIL_set_preference(4, MaxNumberOfSamples + extra);
    AIL_set_preference(5, 127);
    AIL_set_preference(6, 655);
    if (StereoSound)
        AIL_set_preference(7, 1);
    else
        AIL_set_preference(7, 0);
    if (!AutoScanForSoundHardware)
        AIL_set_preference(17, 0);
    env = getenv("BF_FORCE_SOUND_DRIVER");
    if (env) {
        sprintf(SoundProgressMessage, "BF8  - Force sound driver environment active\n");
        SoundProgressLog(SoundProgressMessage);
        sscanf(env, "%s %x %d %d %d", name, &io.IO, &io.IRQ, &io.DMA_8_bit, &io.DMA_16_bit);
        SoundDriver = AIL_install_DIG_driver_file(name, &io);
        if (SoundDriver == 0) {
            sprintf(SoundProgressMessage, "BF9  - MDSOUND environment driver installation - failed\n");
            SoundProgressLog(SoundProgressMessage);
            if (!MusicAble)
                AIL_shutdown();
            sprintf(SoundInstallChoice.driver_name, "none");
            SoundAble = 0;
            SoundActive = 0;
            return;
        }
        sprintf(SoundInstallChoice.driver_name, "%s", name);
        sprintf(SoundProgressMessage, "BF10 - MDSOUND environment driver installation - passed\n");
        SoundProgressLog(SoundProgressMessage);
        SoundInstallChoice.IO.IO = io.IO;
    } else {
        if (AIL_read_INI(&SoundInstallChoice, "DIG.INI")) {
            sprintf(SoundProgressMessage, "BF11 - Search for DIG.INI - passed   \n");
            SoundProgressLog(SoundProgressMessage);
            if (stricmp(SoundInstallChoice.driver_name, "None") == 0) {
                sprintf(SoundProgressMessage, "BF12 - user requests no sound in SETSOUND   \n");
                SoundProgressLog(SoundProgressMessage);
                if (!MusicAble)
                    AIL_shutdown();
                SoundAble = 0;
                SoundActive = 0;
                return;
            }
            init_error = AIL_install_DIG_INI(&SoundDriver);
            if (init_error == 0) {
                sprintf(SoundProgressMessage, "BF13 - DIG.INI driver installation - passed\n");
                SoundProgressLog(SoundProgressMessage);
                ok = 1;
            } else {
                sprintf(SoundInstallChoice.driver_name, "none");
                sprintf(SoundProgressMessage, "BF14 - DIG.INI driver installation - failed %d\n", init_error);
                SoundProgressLog(SoundProgressMessage);
            }
        } else {
            sprintf(SoundInstallChoice.driver_name, "none");
            sprintf(SoundProgressMessage, "BF15 - Search for DIG.INI - failed\n");
            SoundProgressLog(SoundProgressMessage);
        }
        if (!ok) {
            sprintf(SoundProgressMessage, "BF16 - all dig driver installation attempts failed - %s\n", AIL_error);
            SoundProgressLog(SoundProgressMessage);
            if (!MusicAble)
                AIL_shutdown();
            SoundAble = 0;
            SoundActive = 0;
            return;
        }
    }
    sprintf(SoundProgressMessage, "BF18 - determine sound type to be loaded\n");
    SoundProgressLog(SoundProgressMessage);
    if (DisableLoadSounds == 0) {
        DetermineSoundType();
    } else {
        sprintf(SoundProgressMessage, "BF19 - LoadSounds disabled\n");
        SoundProgressLog(SoundProgressMessage);
    }
    if (SoundType == 0) {
        sprintf(SoundProgressMessage, "BF20 - cannot allocate for digital samples\n");
        SoundProgressLog(SoundProgressMessage);
        sprintf(SoundInstallChoice.driver_name, "none");
        if (!MusicAble)
            AIL_shutdown();
        SoundAble = 0;
        SoundActive = 0;
        AIL_uninstall_DIG_driver(SoundDriver);
        return;
    }
    switch (SoundType) {
    case 0x66c:
        SixteenBit = 1;
        SampleRate = 44100;
        break;
    case 0x668:
        SixteenBit = 1;
        SampleRate = 44100;
        break;
    case 0x656:
        SixteenBit = 1;
        SampleRate = 22050;
        break;
    case 0x654:
        SixteenBit = 1;
        SampleRate = 22050;
        break;
    case 0x64b:
        SixteenBit = 1;
        SampleRate = 11025;
        break;
    case 0x64a:
        SixteenBit = 1;
        SampleRate = 11025;
        break;
    case 0x336:
        SixteenBit = 0;
        SampleRate = 22050;
        break;
    case 0x32b:
        SixteenBit = 0;
        SampleRate = 11025;
        break;
    case 0x320:
        SixteenBit = 0;
        SampleRate = 11025;
        break;
    case 0:
        SoundAble = 0;
        SoundActive = 0;
        return;
    }
    if (SoundAble) {
        end_sample_id = &sample_id[MaxNumberOfSamples - 1];
        for (smp = sample_id; smp <= end_sample_id; smp++) {
            smp->SampleHandle = AIL_allocate_sample_handle(SoundDriver);
            smp->FadeState = 0;
            smp->unknown17 = 0;
        }
        SoundInstalled = 1;
        sprintf(SoundProgressMessage, "BF24 - Init sound completed\n");
        SoundProgressLog(SoundProgressMessage);
        SetSoundMasterVolume(SoundMasterVolumeOverride ? SoundMasterVolumeOverride : CurrentSoundMasterVolume);
    }
}

// FUNCTION: 0xdd040
void DetermineSoundType(void)
{
    s32 ret;

    do {
        sprintf(SoundProgressMessage, "BF19 - %d", SoundType);
        SoundProgressLog(SoundProgressMessage);
        ret = AllocateSoundBankMemory(SoundType);
        if (ret <= 0) {
            if (ret == -1) {
                sprintf(SoundProgressMessage, " - no sound.dat\n");
                SoundProgressLog(SoundProgressMessage);
                SoundType = 0;
                return;
            }
            if (ret == 0) {
                sprintf(SoundProgressMessage, " - cannot allocate\n");
                SoundProgressLog(SoundProgressMessage);
            } else if (ret == -2) {
                sprintf(SoundProgressMessage, " - not present in sound.dat\n");
                SoundProgressLog(SoundProgressMessage);
            }
            switch (SoundType) {
            case 0x66c:
                SoundType = 0x668;
                break;
            case 0x668:
                SoundType = 0x656;
                break;
            case 0x656:
                SoundType = 0x654;
                break;
            case 0x654:
                SoundType = 0x64b;
                break;
            case 0x64b:
                SoundType = 0x64a;
                break;
            case 0x64a:
                SoundType = 0x336;
                break;
            case 0x336:
                SoundType = 0x32b;
                break;
            case 0x32b:
                SoundType = 0x320;
                break;
            case 0x320:
                SoundType = 0;
                return;
            }
        } else {
            sprintf(SoundProgressMessage, " - allocation successful\n");
            SoundProgressLog(SoundProgressMessage);
        }
    } while (ret <= 0);
}
