#ifndef OPT_SND_H
#define OPT_SND_H

#include "bflib.h"

// AIL_sample_status and AIL_sequence_status results (Miles, as in ail.h).
#ifndef SMP_DONE
#define SMP_DONE 0x0002
#define SMP_PLAYING 0x0004
#define SEQ_DONE 0x0002
#endif

/* One playing-sample slot; sample_id[] runs up to and including *end_sample_id. */
/* The volatile fields are shared with the sample fade timer callback. */
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
    volatile u8 FadeStep;
    // 0x17: Zeroed by PlaySampleFromAddress; no code reads it.
    u8 unused_set_by_PlaySampleFromAddress;
    u8 unknown18;
    // Caller's tag (logged as "pri"); PlaySampleFromAddress type 3 reuses the
    // voice of the same source and tag.
    u32 ExtraData;
};

/*
 * InitAudio's options; SetupAudioOptionDefaults fills in the defaults.
 */
struct AudioInitOptions {
    // 0x0: Directory of the sound data.
    char *SoundDataPath;
    // 0x4: Directory of the music data.
    char *MusicDataPath;
    // 0x8: Directory passed to AIL_set_driver_directory.
    char *SoundDriverPath;
    // 0xc: Copied to SoundType (0x656 by default).
    u16 SoundType;
    // 0xe: Bit 0 allows music, bit 1 sound; a clear bit turns MusicAble or
    // SoundAble off unless overridden.
    u16 AbleFlags;
    // 0x10: Copied to MaxNumberOfSamples.
    u8 MaxSamples;
    // 0x11: 0 turns StereoSound off.
    u8 StereoCheck;
    // 0x12: 0 turns the hardware autoscan off.
    u8 AutoScan;
    // 0x13: 1 sets DisableDangerMusic.
    u8 DisableDangerMusic;
    // 0x14: 1 sets DisableLoadSounds.
    u8 DisableLoadSounds;
    // 0x15: 1 sets DisableLoadMusic.
    u8 DisableLoadMusic;
    // 0x16: 1 sets UseCurrentAwe32Soundfont.
    u8 UseCurrentAwe32Soundfont;
    // 0x17: 1 sets UseMultiMediaExtensions.
    u8 UseMultiMediaExtensions;
    // 0x18: 1 calls InitStreamedSound.
    u8 InitStreamedSound;
    // 0x19: 1 calls InitRedbook; BF_FORCE_REDBOOK_DISABLE clears it.
    u8 InitRedbookAudio;
};

extern "C" {
extern u8 SoundInstalled;
extern u8 SoundAble;
extern u8 SoundActive;
extern u8 StereoSound;
extern u8 MusicInstalled;
extern u8 MusicAble;
extern u8 MusicActive;
extern u16 NumberOfSamples;
extern SampleInfo sample_id[];
extern SampleInfo *end_sample_id;
extern u32 SongHandle;
extern u16 SongCurrentlyPlaying;
extern u16 CurrentTempo;
extern char SoundProgressMessage[];

void SoundProgressLog(char *msg);

u32 __cdecl AIL_sample_status(u32 S);
void __cdecl AIL_end_sample(u32 S);
void __cdecl AIL_init_sample(u32 S);
u32 __cdecl AIL_set_sample_file(u32 S, void *file_image, s32 block);
void __cdecl AIL_start_sample(u32 S);
void __cdecl AIL_set_sample_volume(u32 S, s32 volume);
void __cdecl AIL_set_sample_pan(u32 S, s32 pan);
void __cdecl AIL_set_sample_playback_rate(u32 S, s32 playback_rate);
void __cdecl AIL_set_sample_loop_count(u32 S, s32 loop_count);
s32 __cdecl AIL_register_timer(void *callback_fn);
void __cdecl AIL_set_timer_frequency(s32 timer, u32 hertz);
void __cdecl AIL_start_timer(s32 timer);
void __cdecl AIL_release_timer_handle(s32 timer);
u32 __cdecl AIL_sequence_status(u32 S);
u32 __cdecl AIL_init_sequence(u32 S, void *start, s32 sequence_num);
void __cdecl AIL_start_sequence(u32 S);
void __cdecl AIL_set_sequence_volume(u32 S, s32 volume, s32 milliseconds);
char *__cdecl AIL_set_driver_directory(char *dir);
}

#endif
