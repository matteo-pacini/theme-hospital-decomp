#include "snd.h"

/* Built without inline expansion: callees in this file stay calls. */
#pragma inline_depth(0)

// FUNCTION: 0xdd210
void InitMusic(void)
{
    char prefix[0x90];
    char gtl[0x90];
    AILIOParms io;
    char name[0x10];
    u8 ok = 0;
    char *env;
    s32 type;

    if (!MusicAble)
        return;
    sprintf(SoundProgressMessage, D_00126720);
    SoundProgressLog(SoundProgressMessage);
    if (!AILStartupAlreadyInitiated) {
        AIL_mem_use_malloc(LbMemoryAlloc);
        AIL_mem_use_free(LbMemoryFree);
        AIL_startup();
        AILStartupAlreadyInitiated = 1;
    }
    AIL_set_preference(11, 120);
    AIL_set_preference(12, 1);
    AIL_set_preference(13, 127);
    AIL_set_preference(14, 1);
    AIL_set_preference(15, 0);
    AIL_set_preference(16, 12);
    if (!AutoScanForSoundHardware)
        AIL_set_preference(17, 0);
    sprintf(prefix, D_00126733);
    AIL_set_GTL_filename_prefix(prefix);
    sprintf(gtl, D_0012673a, prefix);
    sprintf(gtl, D_00126740, prefix);
    env = getenv(D_00126747);
    if (env) {
        sprintf(SoundProgressMessage, D_0012675c);
        SoundProgressLog(SoundProgressMessage);
        sscanf(env, D_00126789, name, &io.IO, &io.IRQ, &io.DMA_8_bit, &io.DMA_16_bit);
        MusicDriver = AIL_install_MDI_driver_file(name, &io);
        if (MusicDriver == 0) {
            sprintf(SoundProgressMessage, D_00126798);
            SoundProgressLog(SoundProgressMessage);
            if (!SoundAble)
                AIL_shutdown();
            MusicAble = 0;
            MusicActive = 0;
            return;
        }
        sprintf(SoundProgressMessage, D_001267d1);
        SoundProgressLog(SoundProgressMessage);
        sprintf(D_0018c8b4, D_0012680a, name);
    } else {
        if (AIL_read_INI(&MusicInstallChoice, D_0012680d)) {
            sprintf(SoundProgressMessage, D_00126815);
            SoundProgressLog(SoundProgressMessage);
            if (stricmp(D_0018c8b4, D_00126839) == 0) {
                sprintf(SoundProgressMessage, D_0012683e);
                SoundProgressLog(SoundProgressMessage);
                if (!SoundAble)
                    AIL_shutdown();
                MusicAble = 0;
                MusicActive = 0;
                return;
            }
            if (AIL_install_MDI_INI(&MusicDriver) == 0) {
                sprintf(SoundProgressMessage, D_00126869);
                SoundProgressLog(SoundProgressMessage);
                ok = 1;
            } else {
                sprintf(SoundProgressMessage, D_00126896);
                SoundProgressLog(SoundProgressMessage);
            }
        } else {
            sprintf(SoundProgressMessage, D_001268c3);
            SoundProgressLog(SoundProgressMessage);
        }
        if (!ok) {
            sprintf(SoundProgressMessage, D_001268e7, AIL_error);
            SoundProgressLog(SoundProgressMessage);
            if (!SoundAble)
                AIL_shutdown();
            MusicAble = 0;
            MusicActive = 0;
            return;
        }
    }
    SongHandle = AIL_allocate_sequence_handle(MusicDriver);
    if (MidiTypeOverride == 0) {
        if (stricmp(D_0018c8b4, D_00126922) == 0) {
            sprintf(MusicType, D_0012692c);
        } else if (stricmp(D_0018c8b4, D_0012692e) == 0) {
            sprintf(MusicType, D_00126939);
        } else if (stricmp(D_0018c8b4, D_0012693b) == 0) {
            sprintf(MusicType, D_00126944);
        } else if (stricmp(D_0018c8b4, D_00126946) == 0) {
            sprintf(MusicType, D_0012694e);
        } else if (stricmp(D_0018c8b4, D_00126950) == 0) {
            sprintf(MusicType, D_0012695c);
        } else if (stricmp(D_0018c8b4, D_0012695e) == 0) {
            sprintf(MusicType, D_0012696b);
        } else if (stricmp(D_0018c8b4, D_0012696d) == 0) {
            sprintf(MusicType, D_00126978);
        } else if (stricmp(D_0018c8b4, D_0012697a) == 0) {
            sprintf(MusicType, D_00126985);
        } else if (stricmp(D_0018c8b4, D_00126987) == 0) {
            sprintf(MusicType, D_00126990);
        } else if (stricmp(D_0018c8b4, D_00126992) == 0) {
            sprintf(MusicType, D_0012699e);
            MusicInstalled = 1;
            LoadAwe32Soundfont(CurrentAwe32SoundfontPrefix);
            MusicInstalled = 0;
            if (Awe32SoundfontLoaded == 0) {
                sprintf(MusicType, D_001269a0);
                sprintf(SoundProgressMessage, D_001269a2);
                SoundProgressLog(SoundProgressMessage);
            } else {
                sprintf(SoundProgressMessage, D_001269c5);
                SoundProgressLog(SoundProgressMessage);
            }
        } else if (stricmp(D_0018c8b4, D_001269e4) == 0) {
            sprintf(MusicType, D_001269ef);
        } else if (stricmp(D_0018c8b4, D_001269f1) == 0) {
            sprintf(MusicType, D_001269fe);
        } else if (stricmp(D_0018c8b4, D_00126a00) == 0) {
            sprintf(MusicType, D_00126a0a);
        } else if (stricmp(D_0018c8b4, D_00126a0c) == 0) {
            sprintf(MusicType, D_00126a18);
        } else if (AIL_MDI_driver_type(MusicDriver) == 0) {
            sprintf(MusicType, D_00126a1a);
        } else if (AIL_MDI_driver_type(MusicDriver) == 1 || AIL_MDI_driver_type(MusicDriver) == 2) {
            sprintf(MusicType, D_00126a1c);
        } else {
            AIL_uninstall_MDI_driver(MusicDriver);
            if (!SoundAble)
                AIL_shutdown();
            MusicAble = 0;
            MusicActive = 0;
            sprintf(SoundProgressMessage, D_00126a1e);
            SoundProgressLog(SoundProgressMessage);
            return;
        }
    }
    if (!DisableLoadMusic) {
        sprintf(SoundProgressMessage, D_00126a46, MusicType);
        SoundProgressLog(SoundProgressMessage);
        music_allocated = AllocateMusicBankMemory();
        if (MidiTypeOverride == 0 && stricmp(MusicType, D_00126a65) == 0 && music_allocated == -2) {
            sprintf(SoundProgressMessage, D_00126a67);
            SoundProgressLog(SoundProgressMessage);
            sprintf(MusicType, D_00126a84);
            sprintf(SoundProgressMessage, D_00126a86, MusicType);
            SoundProgressLog(SoundProgressMessage);
            music_allocated = AllocateMusicBankMemory();
        }
        if (music_allocated <= 0) {
            if (music_allocated == -1) {
                sprintf(SoundProgressMessage, D_00126aa6);
                SoundProgressLog(SoundProgressMessage);
            } else if (music_allocated == 0) {
                sprintf(SoundProgressMessage, D_00126ab7);
                SoundProgressLog(SoundProgressMessage);
            } else if (music_allocated == -2) {
                sprintf(SoundProgressMessage, D_00126acb);
                SoundProgressLog(SoundProgressMessage);
            }
            if (Awe32SoundfontLoaded == 1) {
                MusicInstalled = 1;
                FreeAwe32Soundfont();
                MusicInstalled = 0;
                Awe32SoundfontLoaded = 0;
            }
            AIL_uninstall_MDI_driver(MusicDriver);
            if (!SoundAble)
                AIL_shutdown();
            MusicAble = 0;
            MusicActive = 0;
            return;
        }
        sprintf(SoundProgressMessage, D_00126ae8);
    } else {
        sprintf(SoundProgressMessage, D_00126b02);
    }
    SoundProgressLog(SoundProgressMessage);
    sprintf(SoundProgressMessage, D_00126b1d);
    SoundProgressLog(SoundProgressMessage);
    MusicInstalled = 1;
    SetMusicMasterVolume(MidiMasterVolumeOverride ? MidiMasterVolumeOverride : CurrentMusicMasterVolume);
}

extern "C" {

int access(const char *path, int mode);

// FUNCTION: 0xdda20
int fn_000dda20(char *path)
{
    return access(path, 0) == 0;
}

}
