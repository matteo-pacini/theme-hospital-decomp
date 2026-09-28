#include "hosp.h"

struct ambient {
    u8 sound;
    u8 priority;
};

struct SampleInfo {
    void *SampleHandle;
    s32 SampleVolume;
    u16 SamplePitch;
    u16 SamplePan;
    u16 FadeToVolume;
    u32 SourceID;
    s16 SampleNumber;
    u8 unknown14[9];
};

struct PitchGroup {
    u8 count;
    u8 values[5];
};

struct SampleGroup {
    u8 count;
    u8 samples[5];
};

struct QuakeState {
    u8 unknown0[2];
    u8 state;
    u8 magnitude;
    u8 unknown4;
};

#define ABS(x) ((x) < 0 ? -(x) : (x))

struct MidiFile {
    char name[0x1e];
    char title[0x1a];
};

struct HospMusic {
    s32 unknown0;
    s16 volume;
};

#define isprint(c) (__IsTable[(u8)((c) + 1)] & 8)

struct AudioInitOptions {
    char *unknown0;
    u32 unknown4;
    char *unknown8;
    u16 unknownc;
    u16 unknowne;
    u8 unknown10;
    u8 unknown11;
    u8 unknown12;
    u8 unknown13;
    u8 unknown14;
    u8 unknown15;
    u8 unknown16;
    u8 unknown17;
    u32 unknown18;
};

extern ambient AmbientQueue[];
extern PitchGroup pitch_group[];
extern MidiFile midi_files[];
extern HospMusic music;

void load_midi(void);
u8 is_midi_playing(void);
void play_last_midi(void);
char *read_line_from_buf(char *line, s32 max, char *src);
extern SampleGroup sample_group[];

s32 init_sound_heap(u8 *heap, s32 size, char *path, s32 count);
SampleInfo *play_sample_using_heap(u32 a, s16 b, u32 c, u32 d, u32 e, s8 f, u8 g);

extern "C" {

extern char D_00120794[], D_0012079a[], D_001207a0[], D_001207b6[];
extern s32 D_00156433;
extern s32 D_0015642f;
extern u8 *D_00158334;
extern u8 D_001547e5;
extern TbLoadFiles D_0013ecb4[], D_0013edbc[], D_0013ed64[];
extern u8 D_001553b4;
extern u8 D_00155dfa;
extern u16 D_00155dfb;
extern SampleInfo *D_00155dfd;
extern ambient D_0014a468[];
extern u8 D_0015f3f8, D_0015f3f7;
extern u16 D_00155e2a;
extern u8 D_00155e2c, D_00155e32;
extern u16 D_00155e43;
extern QuakeState D_0015532d;
extern u8 D_00155d71, D_00155d72, D_00155e07, D_00155e08, D_0015642d;
extern char D_001207ca[], D_001207d5[], D_001207e0[], D_001207eb[], D_001207f6[];
extern char D_00120801[], D_00120812[], D_0012081d[], D_00120820[];
extern u8 __IsTable[];
#pragma aux __IsTable "*"
void PlayMidiFromAddress(u8 volume, void *data);
int AIL_sequence_status(u32 handle, ...);
#pragma aux AIL_sequence_status "_*"

void SetupAudioOptionDefaults(AudioInitOptions *options);
void InitAudio(AudioInitOptions *options);
char GetSoundInstalled(void);
char GetSoundAble(void);
u8 GetMusicInstalled(void);
u8 GetMusicAble(void);
char IsSamplePlaying(s32 a, s32 b, void *handle);
int fn_000bce44(TbLoadFiles *files);
void StopSample(u32 source, s16 sample);
void SetSamplePan(u32 source, s16 sample, u16 pan, void *handle);
void SetSampleVolume(u32 source, s16 sample, s16 volume, void *handle);
SampleInfo *GetNumberOfSongs(void);
SampleInfo *GetCurrentSoundMasterVolume(void);

}

// FUNCTION: 0x00088980
u8 setup_sound(void)
{
    char buf[144];
    AudioInitOptions options;
    u8 loaded;
    int installed;
    int able;

    loaded = 0;
    SetupAudioOptionDefaults(&options);
    options.unknown0 = D_00120794;
    options.unknown8 = D_0012079a;
    options.unknowne = 3;
    options.unknown12 = 1;
    options.unknown11 = 1;
    options.unknownc = 0x656;
    options.unknown10 = 0xa;
    options.unknown15 = 1;
    options.unknown14 = 1;
    options.unknown16 = 1;
    options.unknown17 = 1;
    statvar.fe4 = 0;
    statvar.unknowne5 = 0;
    myplayer.f10 = 0;
    myplayer.ff = 0;
    myplayer.unknown11 = 0;
    LbDataLoadSetModifyFilenameFunction(statvar.fd0, options.unknown0);
    InitAudio(&options);
    if (GetSoundInstalled() && GetSoundAble()) {
        LbDataLoadAll(&D_0013ecb4[D_00156433]);
        if (D_00158334) {
            loaded = 1;
            sprintf(buf, D_001207a0, options.unknown0, D_001547e5);
            if (init_sound_heap(D_00158334, D_0015642f, buf, options.unknownc)) {
                window.unknown30 = 0;
                window.unknown48 = lbDisplay.unknown18;
                window.unknown3c = 0;
                window.unknown54 = statvar.unknown4;
                window.unknown90 = (window.unknown30 + window.unknown48) / 2;
                window.unknown9c = (window.unknown3c + window.unknown54) / 2;
                window.unknown2eb = 0;
                statvar.fe4 = 1;
            } else {
                return 6;
            }
        }
    }
    if ((installed = GetMusicInstalled()) != 0 && (able = GetMusicAble()) != 0)
        statvar.unknowne5 = 1;
    if (statvar.fe4) {
        myplayer.ff = 1;
        myplayer.f10 = 1;
    }
    if (statvar.unknowne5 && fn_000bce44(D_0013edbc) == 0)
        myplayer.unknown11 = 1;
    if (!loaded) {
        fn_000bce44(D_0013ed64);
        if (D_00158334) {
            sprintf(buf, D_001207b6, options.unknown0);
            if (!init_sound_heap(D_00158334, 0x32000, buf, options.unknownc))
                return 7;
        } else {
            sub_error_number = 1;
            return 5;
        }
    }
    return 0;
}

// FUNCTION: 0x00088bdc
void do_ambients(void)
{
    u8 i;

    if (myplayer.f10 != 0 && !D_001553b4 && !gamevar.unknown21) {
        if (D_00155dfa) {
            if (!IsSamplePlaying(0, 0, D_00155dfd->SampleHandle))
                D_00155dfa = 0;
        } else if (AmbientQueue[0].sound) {
            D_00155dfd = play_sample_using_heap(0xb, AmbientQueue[0].sound + 500, myplayer.fe, 0x40, 0x64, 0, 2);
            if (D_00155dfd)
                D_00155dfa = AmbientQueue[0].sound;
            for (i = 0; i < 3; i++)
                AmbientQueue[i] = AmbientQueue[i + 1];
            AmbientQueue[i].sound = AmbientQueue[i].priority = 0;
            D_00155dfb = 0;
        }
        D_00155dfb++;
        if (D_00155dfb > 2000) {
            play_ambient(saferand() % 34 + 0x89);
            D_00155dfb = 0;
        }
    }
}

// FUNCTION: 0x00088d44
void init_ambients(void)
{
    u8 i;

    for (i = 0; i < 4; i++) {
        AmbientQueue[i].sound = 0;
        AmbientQueue[i].priority = 0;
    }
}

// FUNCTION: 0x00088d90
void play_ambient(u8 sound)
{
    u8 i;
    u8 slot;

    if (myplayer.f10 != 0 && D_0014a468[sound].sound && myhosp->fc9e[3] != 0 && !gamevar.unknown21) {
        for (i = 0; i < 4; i++) {
            if (!AmbientQueue[i].sound) {
                AmbientQueue[i] = D_0014a468[sound];
                return;
            }
        }
        if (i == 4) {
            slot = 0;
            for (i = 1; i < 4; i++) {
                if (AmbientQueue[i].priority < D_0014a468[sound].priority)
                    slot = i;
            }
        }
        if (slot)
            AmbientQueue[slot] = D_0014a468[sound];
    }
}

// NONMATCHING: register choice, switch temporary slot
// FUNCTION: 0x00088ebc
void play_sound(u32 source, u16 sfx, s32 x, s32 y)
{
    s32 sample;
    u32 pitch;
    s32 pan;
    s32 volume;
    s32 half;

    if (myplayer.ff && statvar.fe4) {
        if (sfx < 0xf2) {
            switch (sfx_details[sfx].type) {
            case 0:
                break;
            case 1:
                pitch = 100;
                sample = sfx_details[sfx].sample;
                break;
            case 2:
                StopSample(source, sfx_details[sfx].sample);
                return;
            case 3:
                pitch = pitch_group[sfx_details[sfx].group].values[saferand() % pitch_group[sfx_details[sfx].group].count];
                sample = sfx_details[sfx].sample;
                break;
            case 4:
                pitch = 100;
                sample = sample_group[sfx_details[sfx].group].samples[saferand() % sample_group[sfx_details[sfx].group].count];
                break;
            case 5:
                pitch = saferand() % (pitch_group[sfx_details[sfx].group].values[1] - pitch_group[sfx_details[sfx].group].values[0]) + pitch_group[sfx_details[sfx].group].values[0];
                sample = sfx_details[sfx].sample;
                break;
            }
            if (x == -1 || y == -1) {
                pan = 0x40;
                volume = myplayer.fd;
            } else {
                half = lbDisplay.unknown18 / 2;
                volume = (half - ABS(x - half)) * myplayer.fd / half;
                pan = x * 0x7f / lbDisplay.unknown18;
                if (volume < 0)
                    volume = 0;
                else if (volume > myplayer.fd)
                    volume = myplayer.fd;
                if (pan < 0)
                    pan = 0;
                else if (pan > 0x7f)
                    pan = 0x7f;
            }
            play_sample_using_heap(source, sample, volume, pan, pitch, sfx_details[sfx].f4, sfx_details[sfx].fb);
        }
    }
}

// FUNCTION: 0x00089204
void manage_sound(void)
{
    SampleInfo *s;
    Thing *w;
    s32 volume;
    s32 pan;
    s32 half;

    for (s = GetNumberOfSongs(); s <= GetCurrentSoundMasterVolume(); s++) {
        if (IsSamplePlaying(0, 0, s->SampleHandle)) {
            if (s->SourceID > 0xe) {
            if ((Thing *)s->SourceID > thing && (Thing *)s->SourceID < end_things) {
                w = (Thing *)s->SourceID;
                give_screen_xy_of_cell_at_column_row(window.unknown2eb, w->x.tile, w->y.tile);
                info.unknown54 += D_0015f3f8 / 2;
                info.unknown58 += D_0015f3f7 / 2;
                if (s->SourceID == 0xd) {
                    if (myhosp->f1d9s == 0)
                        StopSample(s->SourceID, s->SampleNumber);
                } else if (info.unknown54 > 0 && info.unknown54 < lbDisplay.unknown18 && info.unknown58 > 0 && info.unknown58 < lbDisplay.unknown1c) {
                    half = lbDisplay.unknown18 / 2;
                    volume = (half - ABS(info.unknown54 - half)) * myplayer.fd / half;
                    pan = info.unknown54 * 0x7f / lbDisplay.unknown18;
                    if (volume < 0)
                        volume = 0;
                    else if (volume > myplayer.fd)
                        volume = myplayer.fd;
                    if (pan < 0)
                        pan = 0;
                    else if (pan > 0x7f)
                        pan = 0x7f;
                    if (s->SamplePan != pan)
                        SetSamplePan(0, 0, pan, s->SampleHandle);
                    if (s->SampleVolume != volume)
                        SetSampleVolume(0, 0, volume, s->SampleHandle);
                } else {
                    StopSample(s->SourceID, s->SampleNumber);
                }
            } else {
                StopSample(s->SourceID, s->SampleNumber);
            }
            }
        }
    }
}

// NONMATCHING: operand load order in the first multiply, register choice
// FUNCTION: 0x00089468
void manage_background_sounds(void)
{
    s32 sample;
    s32 volume;

    if (myplayer.ff && statvar.fe4 && !gamevar.unknown21) {
        volume = D_00155e2a * myplayer.fd / 20;
        if (volume < 0)
            volume = 0;
        else if (volume > myplayer.fd)
            volume = myplayer.fd;
        play_sample_using_heap(0xc, 200, volume, 0x40, 100, -1, 2);
        if (D_00155e2c != volume) {
            SetSampleVolume(0xc, 200, volume, 0);
            D_00155e2c = volume;
        }
        D_00155e43++;
        if (D_00155e43 > 400) {
            sample = saferand() % 30 + 0x10e;
            play_sample_using_heap(0xa, sample, volume, 0x40, 100, 0, 2);
            D_00155e43 = 0;
        }
        if (myhosp->f1d9) {
            volume = myplayer.fd / 2;
            if (myhosp->f1d9s < 0x21)
                volume += (0x21 - myhosp->f1d9s) * myplayer.fd / 0x42;
            if (volume < 0)
                volume = 0;
            else if (volume > myplayer.fd)
                volume = myplayer.fd;
            play_sample_using_heap(0xd, 0xa9, volume, 0x40, 100, -1, 2);
            if (D_00155e32 != volume) {
                SetSampleVolume(0xd, 0xa9, volume, 0);
                D_00155e32 = volume;
            }
        }
        if (D_0015532d.magnitude)
            play_sample_using_heap(0xc, 0x48, myplayer.fd, 0x40, 100, -1, 2);
    }
}

// FUNCTION: 0x000896b0
void init_midi(void)
{
    u8 i;

    music.unknown0 = 1;
    music.volume = myplayer.fc;
    for (i = 0; i < 10; i++)
        myplayer.unknown12[i] = 1;
    D_00155d71 = 1;
    D_00155d72 = 8;
    load_midi();
}

// FUNCTION: 0x00089718
void play_next_midi(void)
{
    char buf[100];
    u8 i;

    D_00155d71 = 1;
    if (!is_midi_playing()) {
        if (D_00155e08) {
            if (myplayer.unknown12[D_00155d72]) {
                sprintf(buf, D_001207ca, statvar.fd0, midi_files[D_00155d72].name);
                if (LbFileLoadAt(buf, midi_buffer) != -1) {
                    PlayMidiFromAddress(myplayer.fc, midi_buffer);
                    D_00155e07 = 1;
                } else {
                    D_00155d71 = 0;
                }
            }
        } else {
            for (i = 0; i < D_0015642d; i++) {
                D_00155d72++;
                if (D_00155d72 > D_0015642d)
                    D_00155d72 = 0;
                if (myplayer.unknown12[D_00155d72]) {
                    sprintf(buf, D_001207d5, statvar.fd0, midi_files[D_00155d72].name);
                    if (LbFileLoadAt(buf, midi_buffer) != -1) {
                        PlayMidiFromAddress(myplayer.fc, midi_buffer);
                        D_00155e07 = 1;
                        break;
                    }
                }
            }
            if (i == D_0015642d)
                D_00155d71 = 0;
        }
    }
}

// FUNCTION: 0x0008987c
void play_midi(u8 track)
{
    char buf[100];

    AIL_end_sequence(GetSongHandle());
    if (myplayer.unknown11 && statvar.unknowne5 && D_00155d71) {
        if (myplayer.unknown12[track]) {
            sprintf(buf, D_001207e0, statvar.fd0, midi_files[track].name);
            if (LbFileLoadAt(buf, midi_buffer) != -1) {
                PlayMidiFromAddress(myplayer.fc, midi_buffer);
                D_00155d72 = track;
                D_00155e07 = 1;
            } else {
                play_next_midi();
            }
        } else {
            play_next_midi();
        }
    }
}

// FUNCTION: 0x00089940
void play_last_midi(void)
{
    char buf[100];
    u8 i;

    D_00155d71 = 1;
    if (!is_midi_playing()) {
        if (D_00155e08) {
            if (myplayer.unknown12[D_00155d72]) {
                sprintf(buf, D_001207eb, statvar.fd0, midi_files[D_00155d72].name);
                if (LbFileLoadAt(buf, midi_buffer) != -1) {
                    PlayMidiFromAddress(myplayer.fc, midi_buffer);
                    D_00155e07 = 1;
                } else {
                    D_00155d71 = 0;
                }
            }
        } else {
            for (i = 0; i < D_0015642d; i++) {
                if (D_00155d72 == 0)
                    D_00155d72 = D_0015642d;
                else
                    D_00155d72--;
                if (myplayer.unknown12[D_00155d72]) {
                    sprintf(buf, D_001207f6, statvar.fd0, midi_files[D_00155d72].name);
                    if (LbFileLoadAt(buf, midi_buffer) != -1) {
                        PlayMidiFromAddress(myplayer.fc, midi_buffer);
                        D_00155e07 = 1;
                        break;
                    }
                }
            }
            if (i == D_0015642d)
                D_00155d71 = 0;
        }
    }
}

// FUNCTION: 0x00089aa8
u8 is_midi_playing(void)
{
    if (gamevar.unknown21)
        return 1;
    if (myplayer.unknown11 && statvar.unknowne5) {
        if (D_00155d71) {
            if (D_00155e07) {
                if (AIL_sequence_status(GetSongHandle()) != 4)
                    return 0;
            }
        }
    }
    return 1;
}

// FUNCTION: 0x00089b14
void load_midi(void)
{
    char line[132];
    char path[100];
    char name[32];
    s32 handle;
    s32 len;
    char *buf;
    s8 j;
    s8 i;

    len = 0;
    buf = 0;
    sprintf(name, D_00120801, statvar.fd0);
    len = LbFileLengthRnc(name);
    if (len == -1) {
        D_00155d71 = 0;
        statvar.fe4 = 0;
        myplayer.ff = 0;
        return;
    }
    buf = (char *)LbMemoryAlloc(len + 1);
    if (buf == 0) {
        D_00155d71 = 0;
        statvar.fe4 = 0;
        myplayer.ff = 0;
        return;
    }
    if (LbFileLoadAt(name, buf) == -1) {
        D_00155d71 = 0;
        statvar.fe4 = 0;
        myplayer.ff = 0;
        LbMemoryFree(buf);
        return;
    }
    buf[len] = 0xff;
    LbMemorySet(line, 0, 0x80);
    read_line_from_buf(0, 0, buf);
    for (i = 0; i < 10; i++)
        LbMemorySet(&midi_files[i], 0, 0x1e);
    i = 0;
    for (;;) {
        if (!read_line_from_buf(line, 0x80, buf))
            break;
        for (j = 0; j < 30; j++) {
            if (!isprint(line[j])) {
                line[j] = 0;
                break;
            }
        }
        line[0x1d] = 0;
        sprintf(path, D_00120812, statvar.fd0, line);
        handle = LbFileOpen(path, 2);
        if (handle != -1) {
            LbFileClose(handle);
            sprintf(midi_files[i].name, D_0012081d, line);
            read_line_from_buf(line, 0x80, buf);
            for (j = 0; j < 26; j++) {
                if (!isprint(line[j])) {
                    line[j] = 0;
                    break;
                }
            }
            line[0x19] = 0;
            sprintf(midi_files[i].title, D_00120820, line);
            LbMemorySet(line, 0, 0x80);
            i++;
            if (i >= 10)
                break;
        }
    }
    D_0015642d = i;
    if (D_0015642d == 0) {
        statvar.fe4 = 0;
        myplayer.ff = 0;
    }
    read_line_from_buf(0, 1, 0);
    LbMemoryFree(buf);
}
