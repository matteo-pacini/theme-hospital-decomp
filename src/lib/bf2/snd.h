#ifndef BF2_SND_H
#define BF2_SND_H

#include "bflib.h"

struct _SAMPLE;

struct SoundBankEntry {
    u8 unknown0[0x12];
    u32 data;
    u8 unknown16[0xa];
};

struct AILIOParms {
    s16 IO;
    s16 IRQ;
    s16 DMA_8_bit;
    s16 DMA_16_bit;
    s32 IO_reserved[4];
};

struct AILIni {
    char device_name[0x80];
    char driver_name[0x80];
    AILIOParms IO;
};

struct SampleInfo {
    u32 SampleHandle;
    s32 SampleVolume;
    u16 SamplePitch;
    u16 SamplePan;
    s16 FadeToVolume;
    u32 SourceID;
    s16 SampleNumber;
    u8 FadeState;
    u8 FadeStep;
    u8 FadeStopFlag;
    u8 unknown17;
    u8 unknown18;
    u32 ExtraData;
};

struct BankHead {
    u32 tab_offset;
    u32 data_offset;
    u32 tab_size;
    u32 data_size;
};

struct BankSizes {
    s32 data_size;
    s32 tab_size;
};

extern "C" {
extern u8 DebugAudio;
extern u8 SoundInstalled;
extern u8 SoundAble;
extern u8 SoundActive;
extern u8 MusicInstalled;
extern u8 MusicAble;
extern u8 MusicActive;
extern s32 CurrentMusicMasterVolume;
extern s32 MidiMasterVolumeOverride;
extern u32 MusicDriver;
extern u32 SongHandle;
extern u16 SongCurrentlyPlaying;
extern u8 DangerMusicFadeActive;
extern s32 DangerMusicFadeHandle;
extern u8 DangerMusicAble;
extern s8 CurrentDangerMusicFadeDirection;
extern u8 DangerMusicVolume;
extern u8 DangerMusicVolumeChange;

extern u16 NumberOfSamples;
extern SoundBankEntry *Sfx;
extern SoundBankEntry *EndSfxs;
extern u8 *SfxData;
extern u16 NumberOfSongs;
extern SoundBankEntry *Music;
extern SoundBankEntry *EndMusic;
extern u8 *MusicData;
extern u32 SoundDriver;
extern s32 SoundMasterVolumeOverride;
extern s32 D_00150824;
extern char SoundDataPath[];
extern char MusicDataPath[];
extern char MusicType[];
extern char SoundProgressMessage[];
extern char D_0018d4c4[];
extern char D_0018d5c4[];
extern BankSizes D_0018d474[];
extern BankSizes D_0018d5a4[];
extern char D_001282de[];
extern char D_001282eb[];
extern char D_00128477[];

extern char D_001263b4[];
extern char D_001263c7[];
extern char D_001263e8[];
extern char D_00126414[];
extern char D_00126433[];
extern char D_00126449[];
extern char D_00126477[];
extern char D_00126486[];
extern char D_001264bf[];
extern char D_001264c4[];
extern char D_001264c7[];
extern char D_00126500[];
extern char D_00126508[];
extern char D_0012652f[];
extern char D_00126534[];
extern char D_00126562[];
extern char D_0012658f[];
extern char D_00126594[];
extern char D_001265c4[];
extern char D_001265c9[];
extern char D_001265ed[];
extern char D_00126626[];
extern char D_00126650[];
extern char D_0012666c[];
extern char D_00126698[];
extern char D_0012669d[];
extern char D_001266ba[];
extern char D_001266c4[];
extern char D_001266d5[];
extern char D_001266e9[];
extern char D_00126706[];
extern char D_00126720[];
extern char D_00126733[];
extern char D_0012673a[];
extern char D_00126740[];
extern char D_00126747[];
extern char D_0012675c[];
extern char D_00126789[];
extern char D_00126798[];
extern char D_001267d1[];
extern char D_0012680a[];
extern char D_0012680d[];
extern char D_00126815[];
extern char D_00126839[];
extern char D_0012683e[];
extern char D_00126869[];
extern char D_00126896[];
extern char D_001268c3[];
extern char D_001268e7[];
extern char D_00126922[];
extern char D_0012692c[];
extern char D_0012692e[];
extern char D_00126939[];
extern char D_0012693b[];
extern char D_00126944[];
extern char D_00126946[];
extern char D_0012694e[];
extern char D_00126950[];
extern char D_0012695c[];
extern char D_0012695e[];
extern char D_0012696b[];
extern char D_0012696d[];
extern char D_00126978[];
extern char D_0012697a[];
extern char D_00126985[];
extern char D_00126987[];
extern char D_00126990[];
extern char D_00126992[];
extern char D_0012699e[];
extern char D_001269a0[];
extern char D_001269a2[];
extern char D_001269c5[];
extern char D_001269e4[];
extern char D_001269ef[];
extern char D_001269f1[];
extern char D_001269fe[];
extern char D_00126a00[];
extern char D_00126a0a[];
extern char D_00126a0c[];
extern char D_00126a18[];
extern char D_00126a1a[];
extern char D_00126a1c[];
extern char D_00126a1e[];
extern char D_00126a46[];
extern char D_00126a65[];
extern char D_00126a67[];
extern char D_00126a84[];
extern char D_00126a86[];
extern char D_00126aa6[];
extern char D_00126ab7[];
extern char D_00126acb[];
extern char D_00126ae8[];
extern char D_00126b02[];
extern char D_00126b1d[];
extern u8 AILStartupAlreadyInitiated;
extern u8 StereoSound;
extern u8 AutoScanForSoundHardware;
extern u8 DisableLoadSounds;
extern u8 DisableLoadMusic;
extern u8 UseCurrentAwe32Soundfont;
extern u8 Awe32SoundfontLoaded;
extern char CurrentAwe32SoundfontPrefix[];
extern u32 MaxNumberOfSamples;
extern s32 SystemPlaybackRate;
extern s32 SampleRate;
extern u16 SoundType;
extern s32 MidiTypeOverride;
extern char AIL_error[];
extern AILIni SoundInstallChoice;
extern AILIni MusicInstallChoice;
extern SampleInfo sample_id[];
extern SampleInfo *end_sample_id;
extern char D_0018c8b4[];

s32 LbFilePosition(s32 handle);
char *getenv(const char *name);
int sscanf(const char *s, const char *fmt, ...);
int stricmp(const char *a, const char *b);
void InitSound(void);
void InitMusic(void);
void DetermineSoundType(void);
void LoadAwe32Soundfont(char *prefix);
void FreeAwe32Soundfont(void);
s32 LbFileLength(char *path);
void UnpackM1(void *src, void *dst);
void SoundProgressLog(char *msg);
void StopMusic(void);
void SetMusicMasterVolume(s32 volume);
void StopSampleQueueList(void);
void LoadSounds(s32 volume);
s32 AllocateSoundBankMemory(u16 type);
s32 AllocateMusicBankMemory(void);

void __cdecl AIL_set_XMIDI_master_volume(u32 driver, s32 volume);
void __cdecl AIL_release_timer_handle(s32 timer);
u32 __cdecl AIL_sequence_status(u32 sequence);
void __cdecl AIL_end_sample(_SAMPLE *sample);
void __cdecl AIL_register_EOS_callback(_SAMPLE *sample, void *callback);
void __cdecl AIL_set_sample_user_data(_SAMPLE *sample, u32 index, s32 value);
void __cdecl AIL_set_digital_master_volume(u32 driver, s32 volume);
u32 __cdecl AIL_init_sequence(u32 sequence, u32 start, s32 track);
void __cdecl AIL_shutdown(void);
void __cdecl AIL_mem_use_malloc(void *(*fn)(u32));
void __cdecl AIL_mem_use_free(int (*fn)(void *));
void __cdecl AIL_startup_reg(void);
#define AIL_startup AIL_startup_reg
s32 __cdecl AIL_set_preference(u32 number, s32 value);
u32 __cdecl AIL_install_DIG_driver_file(char *filename, AILIOParms *io);
s32 __cdecl AIL_read_INI(AILIni *ini, char *filename);
s32 __cdecl AIL_install_DIG_INI(u32 *driver);
void __cdecl AIL_uninstall_DIG_driver(u32 driver);
u32 __cdecl AIL_allocate_sample_handle(u32 driver);
void __cdecl AIL_set_GTL_filename_prefix(char *prefix);
u32 __cdecl AIL_install_MDI_driver_file(char *filename, AILIOParms *io);
s32 __cdecl AIL_install_MDI_INI(u32 *driver);
void __cdecl AIL_uninstall_MDI_driver(u32 driver);
u32 __cdecl AIL_allocate_sequence_handle(u32 driver);
s32 __cdecl AIL_MDI_driver_type(u32 driver);
}

extern u8 sample_queue_handle_initiated;
extern u8 sample_queue_handle_stopped;
extern _SAMPLE *sample_queue_handle;
extern long largest_dat_size;
extern long largest_tab_size;
extern u32 init_error;
extern long music_allocated;
extern u8 SixteenBit;

#define RNC_LENGTH(p) ((((((p)[4] << 8) + (p)[5]) << 8) + (p)[6]) << 8) + (p)[7]

#endif
