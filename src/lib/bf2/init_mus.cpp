#include "snd.h"

extern "C" {
s32 CurrentMusicMasterVolume = 127;
u8 MusicAble = 1;
u8 MusicActive = 1;
s8 CurrentDangerMusicFadeDirection = 1;
u8 DangerMusicVolumeChange = 255;
}

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
    sprintf(SoundProgressMessage, "BF25 - Init Music\n");
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
    sprintf(prefix, "SAMPLE");
    AIL_set_GTL_filename_prefix(prefix);
    sprintf(gtl, "%s.ad", prefix);
    sprintf(gtl, "%s.opl", prefix);
    env = getenv("BF_FORCE_MIDI_DRIVER");
    if (env) {
        sprintf(SoundProgressMessage, "BF29 - Force midi driver environment active\n");
        SoundProgressLog(SoundProgressMessage);
        sscanf(env, "%s %x %d %d %d", name, &io.IO, &io.IRQ, &io.DMA_8_bit, &io.DMA_16_bit);
        MusicDriver = AIL_install_MDI_driver_file(name, &io);
        if (MusicDriver == 0) {
            sprintf(SoundProgressMessage, "BF30 - MDMUSIC environment driver installation - failed\n");
            SoundProgressLog(SoundProgressMessage);
            if (!SoundAble)
                AIL_shutdown();
            MusicAble = 0;
            MusicActive = 0;
            return;
        }
        sprintf(SoundProgressMessage, "BF31 - MDMUSIC environment driver installation - passed\n");
        SoundProgressLog(SoundProgressMessage);
        sprintf(D_0018c8b4, "%s", name);
    } else {
        if (AIL_read_INI(&MusicInstallChoice, "MDI.INI")) {
            sprintf(SoundProgressMessage, "BF32 - Search for MDI.INI - passed\n");
            SoundProgressLog(SoundProgressMessage);
            if (stricmp(D_0018c8b4, "None") == 0) {
                sprintf(SoundProgressMessage, "BF33 - user requests no music in SETSOUND\n");
                SoundProgressLog(SoundProgressMessage);
                if (!SoundAble)
                    AIL_shutdown();
                MusicAble = 0;
                MusicActive = 0;
                return;
            }
            if (AIL_install_MDI_INI(&MusicDriver) == 0) {
                sprintf(SoundProgressMessage, "BF34 - MDI.INI driver installation - passed\n");
                SoundProgressLog(SoundProgressMessage);
                ok = 1;
            } else {
                sprintf(SoundProgressMessage, "BF35 - MDI.INI driver installation - failed\n");
                SoundProgressLog(SoundProgressMessage);
            }
        } else {
            sprintf(SoundProgressMessage, "BF36 - Search for MDI.INI - failed\n");
            SoundProgressLog(SoundProgressMessage);
        }
        if (!ok) {
            sprintf(SoundProgressMessage, "BF37 - all music driver installation attempts failed - %s\n", AIL_error);
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
        if (stricmp(D_0018c8b4, "ADLIB.MDI") == 0) {
            sprintf(MusicType, "f");
        } else if (stricmp(D_0018c8b4, "ADLIBG.MDI") == 0) {
            sprintf(MusicType, "f");
        } else if (stricmp(D_0018c8b4, "OPL3.MDI") == 0) {
            sprintf(MusicType, "f");
        } else if (stricmp(D_0018c8b4, "PAS.MDI") == 0) {
            sprintf(MusicType, "f");
        } else if (stricmp(D_0018c8b4, "PASPLUS.MDI") == 0) {
            sprintf(MusicType, "f");
        } else if (stricmp(D_0018c8b4, "SBLASTER.MDI") == 0) {
            sprintf(MusicType, "f");
        } else if (stricmp(D_0018c8b4, "SBPRO1.MDI") == 0) {
            sprintf(MusicType, "f");
        } else if (stricmp(D_0018c8b4, "SBPRO2.MDI") == 0) {
            sprintf(MusicType, "f");
        } else if (stricmp(D_0018c8b4, "ESFM.MDI") == 0) {
            sprintf(MusicType, "f");
        } else if (stricmp(D_0018c8b4, "SBAWE32.MDI") == 0) {
            sprintf(MusicType, "w");
            MusicInstalled = 1;
            LoadAwe32Soundfont(CurrentAwe32SoundfontPrefix);
            MusicInstalled = 0;
            if (Awe32SoundfontLoaded == 0) {
                sprintf(MusicType, "g");
                sprintf(SoundProgressMessage, "BF38 - awe32 soundfont not loaded\n");
                SoundProgressLog(SoundProgressMessage);
            } else {
                sprintf(SoundProgressMessage, "BF38 - awe32 soundfont loaded\n");
                SoundProgressLog(SoundProgressMessage);
            }
        } else if (stricmp(D_0018c8b4, "MPU401.MDI") == 0) {
            sprintf(MusicType, "g");
        } else if (stricmp(D_0018c8b4, "SNDSCAPE.MDI") == 0) {
            sprintf(MusicType, "g");
        } else if (stricmp(D_0018c8b4, "ULTRA.MDI") == 0) {
            sprintf(MusicType, "g");
        } else if (stricmp(D_0018c8b4, "MT32MPU.MDI") == 0) {
            sprintf(MusicType, "r");
        } else if (AIL_MDI_driver_type(MusicDriver) == 0) {
            sprintf(MusicType, "g");
        } else if (AIL_MDI_driver_type(MusicDriver) == 1 || AIL_MDI_driver_type(MusicDriver) == 2) {
            sprintf(MusicType, "f");
        } else {
            AIL_uninstall_MDI_driver(MusicDriver);
            if (!SoundAble)
                AIL_shutdown();
            MusicAble = 0;
            MusicActive = 0;
            sprintf(SoundProgressMessage, "BF39 - music driver type not supported\n");
            SoundProgressLog(SoundProgressMessage);
            return;
        }
    }
    if (!DisableLoadMusic) {
        sprintf(SoundProgressMessage, "BF40 - music%s bank allocation", MusicType);
        SoundProgressLog(SoundProgressMessage);
        music_allocated = AllocateMusicBankMemory();
        if (MidiTypeOverride == 0 && stricmp(MusicType, "w") == 0 && music_allocated == -2) {
            sprintf(SoundProgressMessage, " - not present in music.dat\n");
            SoundProgressLog(SoundProgressMessage);
            sprintf(MusicType, "g");
            sprintf(SoundProgressMessage, "BF40A - music%s bank allocation", MusicType);
            SoundProgressLog(SoundProgressMessage);
            music_allocated = AllocateMusicBankMemory();
        }
        if (music_allocated <= 0) {
            if (music_allocated == -1) {
                sprintf(SoundProgressMessage, " - no music.dat\n");
                SoundProgressLog(SoundProgressMessage);
            } else if (music_allocated == 0) {
                sprintf(SoundProgressMessage, " - cannot allocate\n");
                SoundProgressLog(SoundProgressMessage);
            } else if (music_allocated == -2) {
                sprintf(SoundProgressMessage, " - not present in music.dat\n");
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
        sprintf(SoundProgressMessage, " - allocation successful\n");
    } else {
        sprintf(SoundProgressMessage, "BF40 - LoadMusic Disabled\n");
    }
    SoundProgressLog(SoundProgressMessage);
    sprintf(SoundProgressMessage, "BF41 - Init music completed\n");
    SoundProgressLog(SoundProgressMessage);
    MusicInstalled = 1;
    SetMusicMasterVolume(MidiMasterVolumeOverride ? MidiMasterVolumeOverride : CurrentMusicMasterVolume);
}

extern "C" int access(const char *path, int mode);

// FUNCTION: 0xdda20
int fm_instrument_file_exists(u8 *path)
{
    return access((char *)path, 0) == 0;
}
