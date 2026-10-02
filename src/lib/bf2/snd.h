#ifndef BF2_SND_H
#define BF2_SND_H

#include "bflib.h"

// AIL_sample_status and AIL_sequence_status results (Miles, as in ail.h).
#ifndef SMP_DONE
#define SMP_DONE 0x0002
#define SMP_PLAYING 0x0004
#define SEQ_DONE 0x0002
#endif

struct _SAMPLE;

struct SoundBankEntry {
    u8 unknown0[0x12];
    // Offset into the bank's data until format_music or format_sounds makes
    // it a pointer.
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

/* The volatile fields are also written by the sample fade timer callback. */
struct SampleInfo {
    u32 SampleHandle;
    volatile s32 SampleVolume;
    // Playback rate as a percentage of the sample's own.
    u16 SamplePitch;
    u16 SamplePan;
    volatile s16 FadeToVolume;
    u32 SourceID;
    s16 SampleNumber;
    // 0 not fading, 1 fading up, 2 fading down.
    volatile u8 FadeState;
    // Nonzero: end the sample once the fade reaches FadeToVolume.
    volatile u8 FadeStopFlag;
    // Volume steps per 50 ms fade tick (1-5).
    u8 FadeStep;
    // 0x17: Zeroed by InitSound; no code reads it.
    u8 unused_set_by_InitSound;
    u8 unknown18;
    // Caller's tag (see PlaySampleFromAddress).
    u32 ExtraData;
};

struct BankHead {
    u32 tab_offset;
    // -1: the file has no bank of this type.
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
extern s32 CurrentSoundMasterVolume;
extern char SoundDataPath[];
extern char MusicDataPath[];
extern char MusicType[];
extern char SoundProgressMessage[];
extern char full_sound_data_path[];
extern char full_music_data_path[];
extern BankSizes sound_bank_size_info[];
extern BankSizes music_bank_size_info[];

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
extern char MusicDriverName[];

s32 LbFilePosition(s32 handle);
char *getenv(const char *name);
int sscanf(const char *s, const char *format, ...);
int stricmp(const char *s, const char *t);
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
void SetSoundMasterVolume(s32 volume);
s32 AllocateSoundBankMemory(u16 type);
s32 AllocateMusicBankMemory(void);

void __cdecl AIL_set_XMIDI_master_volume(u32 mdi, s32 master_volume);
void __cdecl AIL_release_timer_handle(s32 timer);
u32 __cdecl AIL_sequence_status(u32 S);
void __cdecl AIL_end_sample(_SAMPLE *S);
void __cdecl AIL_register_EOS_callback(_SAMPLE *S, void *EOS);
void __cdecl AIL_set_sample_user_data(_SAMPLE *S, u32 index, s32 value);
void __cdecl AIL_set_digital_master_volume(u32 dig, s32 master_volume);
u32 __cdecl AIL_init_sequence(u32 S, u32 start, s32 sequence_num);
void __cdecl AIL_shutdown(void);
void __cdecl AIL_mem_use_malloc(void *(*fn)(u32));
void __cdecl AIL_mem_use_free(int (*fn)(void *));
void __cdecl AIL_startup_reg(void);
#define AIL_startup AIL_startup_reg
s32 __cdecl AIL_set_preference(u32 number, s32 value);
u32 __cdecl AIL_install_DIG_driver_file(char *filename, AILIOParms *IO);
s32 __cdecl AIL_read_INI(AILIni *INI, char *filename);
s32 __cdecl AIL_install_DIG_INI(u32 *dig);
void __cdecl AIL_uninstall_DIG_driver(u32 dig);
u32 __cdecl AIL_allocate_sample_handle(u32 dig);
void __cdecl AIL_set_GTL_filename_prefix(char *prefix);
u32 __cdecl AIL_install_MDI_driver_file(char *filename, AILIOParms *IO);
s32 __cdecl AIL_install_MDI_INI(u32 *mdi);
void __cdecl AIL_uninstall_MDI_driver(u32 mdi);
u32 __cdecl AIL_allocate_sequence_handle(u32 mdi);
s32 __cdecl AIL_MDI_driver_type(u32 mdi);
}

extern u8 sample_queue_handle_initiated;
extern u8 sample_queue_handle_stopped;
extern _SAMPLE *sample_queue_handle;
extern long largest_dat_size;
extern long largest_tab_size;
extern u32 init_error;
extern long music_allocated;
extern u8 SixteenBit;

#define RNC_LENGTH(p) (((((((p)[5] + ((p)[4] << 8)) << 8) + (p)[6]) << 8) + (p)[7]))

#endif
