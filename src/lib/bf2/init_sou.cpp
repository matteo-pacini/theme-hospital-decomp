#include "snd.h"

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
    SampleInfo *p;

    if (!SoundAble)
        return;
    sprintf(SoundProgressMessage, D_001263b4);
    SoundProgressLog(SoundProgressMessage);
    sprintf(SoundProgressMessage, D_001263c7, SoundType);
    SoundProgressLog(SoundProgressMessage);
    if (SoundType == 0) {
        if (!MusicAble && AILStartupAlreadyInitiated) {
            AIL_shutdown();
            sprintf(SoundProgressMessage, D_001263e8);
            SoundProgressLog(SoundProgressMessage);
        }
        SoundAble = 0;
        SoundActive = 0;
        sprintf(SoundProgressMessage, D_00126414);
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
    env = getenv(D_00126433);
    if (env) {
        sprintf(SoundProgressMessage, D_00126449);
        SoundProgressLog(SoundProgressMessage);
        sscanf(env, D_00126477, name, &io.IO, &io.IRQ, &io.DMA_8_bit, &io.DMA_16_bit);
        SoundDriver = AIL_install_DIG_driver_file(name, &io);
        if (SoundDriver == 0) {
            sprintf(SoundProgressMessage, D_00126486);
            SoundProgressLog(SoundProgressMessage);
            if (!MusicAble)
                AIL_shutdown();
            sprintf(SoundInstallChoice.driver_name, D_001264bf);
            SoundAble = 0;
            SoundActive = 0;
            return;
        }
        sprintf(SoundInstallChoice.driver_name, D_001264c4, name);
        sprintf(SoundProgressMessage, D_001264c7);
        SoundProgressLog(SoundProgressMessage);
        SoundInstallChoice.IO.IO = io.IO;
    } else {
        if (AIL_read_INI(&SoundInstallChoice, D_00126500)) {
            sprintf(SoundProgressMessage, D_00126508);
            SoundProgressLog(SoundProgressMessage);
            if (stricmp(SoundInstallChoice.driver_name, D_0012652f) == 0) {
                sprintf(SoundProgressMessage, D_00126534);
                SoundProgressLog(SoundProgressMessage);
                if (!MusicAble)
                    AIL_shutdown();
                SoundAble = 0;
                SoundActive = 0;
                return;
            }
            init_error = AIL_install_DIG_INI(&SoundDriver);
            if (init_error == 0) {
                sprintf(SoundProgressMessage, D_00126562);
                SoundProgressLog(SoundProgressMessage);
                ok = 1;
            } else {
                sprintf(SoundInstallChoice.driver_name, D_0012658f);
                sprintf(SoundProgressMessage, D_00126594, init_error);
                SoundProgressLog(SoundProgressMessage);
            }
        } else {
            sprintf(SoundInstallChoice.driver_name, D_001265c4);
            sprintf(SoundProgressMessage, D_001265c9);
            SoundProgressLog(SoundProgressMessage);
        }
        if (!ok) {
            sprintf(SoundProgressMessage, D_001265ed, AIL_error);
            SoundProgressLog(SoundProgressMessage);
            if (!MusicAble)
                AIL_shutdown();
            SoundAble = 0;
            SoundActive = 0;
            return;
        }
    }
    sprintf(SoundProgressMessage, D_00126626);
    SoundProgressLog(SoundProgressMessage);
    if (DisableLoadSounds == 0) {
        DetermineSoundType();
    } else {
        sprintf(SoundProgressMessage, D_00126650);
        SoundProgressLog(SoundProgressMessage);
    }
    if (SoundType == 0) {
        sprintf(SoundProgressMessage, D_0012666c);
        SoundProgressLog(SoundProgressMessage);
        sprintf(SoundInstallChoice.driver_name, D_00126698);
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
        for (p = sample_id; p <= end_sample_id; p++) {
            p->SampleHandle = AIL_allocate_sample_handle(SoundDriver);
            p->FadeState = 0;
            p->unknown17 = 0;
        }
        SoundInstalled = 1;
        sprintf(SoundProgressMessage, D_0012669d);
        SoundProgressLog(SoundProgressMessage);
        LoadSounds(SoundMasterVolumeOverride ? SoundMasterVolumeOverride : D_00150824);
    }
}

// FUNCTION: 0xdd040
void DetermineSoundType(void)
{
    s32 ret;

    do {
        sprintf(SoundProgressMessage, D_001266ba, SoundType);
        SoundProgressLog(SoundProgressMessage);
        ret = AllocateSoundBankMemory(SoundType);
        if (ret <= 0) {
            if (ret == -1) {
                sprintf(SoundProgressMessage, D_001266c4);
                SoundProgressLog(SoundProgressMessage);
                SoundType = 0;
                return;
            }
            if (ret == 0) {
                sprintf(SoundProgressMessage, D_001266d5);
                SoundProgressLog(SoundProgressMessage);
            } else if (ret == -2) {
                sprintf(SoundProgressMessage, D_001266e9);
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
            sprintf(SoundProgressMessage, D_00126706);
            SoundProgressLog(SoundProgressMessage);
        }
    } while (ret <= 0);
}
