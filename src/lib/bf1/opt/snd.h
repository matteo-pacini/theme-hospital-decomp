#ifndef OPT_SND_H
#define OPT_SND_H

#include "bflib.h"

/* One playing-sample slot; sample_id[] runs up to and including *end_sample_id. */
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

struct AudioInitOptions {
    char *SoundDataPath;
    char *MusicDataPath;
    char *SoundDriverPath;
    u16 SoundType;
    u16 InitRedbookAudio;
    u8 MaxSamples;
    u8 StereoCheck;
    u8 AutoScan;
    u8 DisableDangerMusic;
    u8 DisableLoadSounds;
    u8 DisableLoadMusic;
    u8 UseCurrentAwe32Soundfont;
    u8 UseMultiMediaExtensions;
    u8 unknown18;
    u8 unknown19;
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

u32 __cdecl AIL_sample_status(u32 sample);
void __cdecl AIL_end_sample(u32 sample);
void __cdecl AIL_init_sample(u32 sample);
u32 __cdecl AIL_set_sample_file(u32 sample, void *file, s32 block);
void __cdecl AIL_start_sample(u32 sample);
void __cdecl AIL_set_sample_volume(u32 sample, s32 volume);
void __cdecl AIL_set_sample_pan(u32 sample, s32 pan);
void __cdecl AIL_set_sample_playback_rate(u32 sample, s32 rate);
void __cdecl AIL_set_sample_loop_count(u32 sample, s32 count);
s32 __cdecl AIL_register_timer(void *callback);
void __cdecl AIL_set_timer_frequency(s32 timer, u32 hertz);
void __cdecl AIL_start_timer(s32 timer);
void __cdecl AIL_release_timer_handle(s32 timer);
u32 __cdecl AIL_sequence_status(u32 sequence);
u32 __cdecl AIL_init_sequence(u32 sequence, void *start, s32 track);
void __cdecl AIL_start_sequence(u32 sequence);
void __cdecl AIL_set_sequence_volume(u32 sequence, s32 volume, s32 ms);
char *__cdecl AIL_set_driver_directory(char *dir);
}

#endif
