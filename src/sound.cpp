#include "hosp.h"

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

#define ABS(x) ((x) < 0 ? -(x) : (x))

struct MidiFile {
    char name[0x1e];
    char title[0x1a];
};

struct HospMusic {
    // 0x0: Set to 1 by init_midi; no code reads it.
    s32 unused_set_by_init_midi;
    s16 volume;
};

#define isprint(c) (__IsTable[(u8)((c) + 1)] & 8)

struct AudioInitOptions {
    char *SoundDataPath;
    u32 MusicDataPath;
    char *SoundDriverPath;
    u16 SoundType;
    u16 AbleFlags;
    u8 MaxSamples;
    u8 StereoCheck;
    u8 AutoScan;
    u8 DisableDangerMusic;
    u8 DisableLoadSounds;
    u8 DisableLoadMusic;
    u8 UseCurrentAwe32Soundfont;
    u8 UseMultiMediaExtensions;
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

s32 init_sound_heap(u8 *mem, s32 size, char *path, s32 rate);
SampleInfo *play_sample_using_heap(u32 source, s16 sample, u32 volume, u32 pan, u32 pitch, s8 loop, u8 type);

extern "C" {

extern s32 sound_file_set;
extern s32 sound_heap_size;
extern u8 *heapptr;
extern u8 __IsTable[];
#pragma aux __IsTable "*"
void PlayMidiFromAddress(u8 volume, void *address);
int AIL_sequence_status(u32 handle, ...);
#pragma aux AIL_sequence_status "_*"

void SetupAudioOptionDefaults(AudioInitOptions *ao);
void InitAudio(AudioInitOptions *ao);
char GetSoundInstalled(void);
char GetSoundAble(void);
u8 GetMusicInstalled(void);
u8 GetMusicAble(void);
char IsSamplePlaying(s32 source_id, s32 number, void *handle);
int LbDataLoadAll(TbLoadFiles *files);
void StopSample(u32 source_id, s16 number);
void SetSamplePan(u32 source_id, s16 number, u16 pan, void *handle);
void SetSampleVolume(u32 source_id, s16 number, s16 volume, void *handle);
SampleInfo *GetFirstSampleInfoStructure(void);
SampleInfo *GetLastSampleInfoStructure(void);
}

// Starts the sound and music drivers and loads the sound bank for the
// language (or the minimal bank without sound), setting which of sound
// effects (statvar.sound_ok) and music (statvar.music_ok) are available and
// turning them on. Returns 0, or an error number for error_number.
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
    options.SoundDataPath = "Sound";
    options.SoundDriverPath = "Sound";
    options.AbleFlags = 3;
    options.AutoScan = 1;
    options.StereoCheck = 1;
    options.SoundType = 0x656;
    options.MaxSamples = 0xa;
    options.DisableLoadMusic = 1;
    options.DisableLoadSounds = 1;
    options.UseCurrentAwe32Soundfont = 1;
    options.UseMultiMediaExtensions = 1;
    statvar.sound_ok = 0;
    statvar.music_ok = 0;
    myplayer.ambients_on = 0;
    myplayer.sfx_on = 0;
    myplayer.music_on = 0;
    lib_strcpy(statvar.sound_path, options.SoundDataPath);
    InitAudio(&options);
    if (GetSoundInstalled() && GetSoundAble()) {
        LbDataLoad(&soundfiles[sound_file_set]);
        if (heapptr) {
            loaded = 1;
            sprintf(buf, "%s\\data\\sound-%1d.dat", options.SoundDataPath, myplayer.language);
            if (init_sound_heap(heapptr, sound_heap_size, buf, options.SoundType)) {
                window.zone0_x = 0;
                window.zone0_x1 = lbDisplay.physical_screen_width;
                window.zone0_y = 0;
                window.zone0_y1 = statvar.view_height;
                window.center0_x = (window.zone0_x + window.zone0_x1) / 2;
                window.center0_y = (window.zone0_y + window.zone0_y1) / 2;
                window.current_window = 0;
                statvar.sound_ok = 1;
            } else {
                return 6;
            }
        }
    }
    if ((installed = GetMusicInstalled()) != 0 && (able = GetMusicAble()) != 0)
        statvar.music_ok = 1;
    if (statvar.sound_ok) {
        myplayer.sfx_on = 1;
        myplayer.ambients_on = 1;
    }
    if (statvar.music_ok && LbDataLoadAll(midifiles) == 0)
        myplayer.music_on = 1;
    if (!loaded) {
        LbDataLoadAll(nosoundfiles);
        if (heapptr) {
            sprintf(buf, "%s\\data\\minheap.dat", options.SoundDataPath);
            if (!init_sound_heap(heapptr, 0x32000, buf, options.SoundType))
                return 7;
        } else {
            sub_error_number = 1;
            return 5;
        }
    }
    return 0;
}

// Per turn: plays the next queued announcement once the last has finished,
// and every 2000 turns queues a random one. Only with ambient sounds on and
// the game running.
// FUNCTION: 0x00088bdc
void do_ambients(void)
{
    u8 i;

    if (myplayer.ambients_on != 0 && !gamevar.rat_fest_level && !gamevar.paused) {
        if (othervar.ambient_playing) {
            if (!IsSamplePlaying(0, 0, othervar.ambient_sample->SampleHandle))
                othervar.ambient_playing = 0;
        } else if (AmbientQueue[0].sound) {
            othervar.ambient_sample = play_sample_using_heap(SNDSRC_AMBIENT, AmbientQueue[0].sound + 500,
                                                             myplayer.ambient_volume, 0x40, 0x64, 0, 2);
            if (othervar.ambient_sample)
                othervar.ambient_playing = AmbientQueue[0].sound;
            for (i = 0; i < 3; i++)
                AmbientQueue[i] = AmbientQueue[i + 1];
            AmbientQueue[i].sound = AmbientQueue[i].priority = 0;
            othervar.ambient_timer = 0;
        }
        othervar.ambient_timer++;
        if (othervar.ambient_timer > 2000) {
            play_ambient(saferand() % 34 + 0x89);
            othervar.ambient_timer = 0;
        }
    }
}

// Empties the announcement queue.
// FUNCTION: 0x00088d44
void init_ambients(void)
{
    u8 i;

    for (i = 0; i < 4; i++) {
        AmbientQueue[i].sound = 0;
        AmbientQueue[i].priority = 0;
    }
}

// Queues announcement `sound` (an Ambients index) if ambient sounds are on
// and the hospital has a receptionist; when the four slots are full it
// replaces the last queued entry of lower priority, if any (never the
// first).
// FUNCTION: 0x00088d90
void play_ambient(u8 sound)
{
    u8 i;
    u8 slot;

    if (myplayer.ambients_on != 0 && Ambients[sound].sound && myhosp->staff_by_kind[STAFF_RECEPTIONIST] != 0 &&
        !gamevar.paused) {
        for (i = 0; i < 4; i++) {
            if (!AmbientQueue[i].sound) {
                AmbientQueue[i] = Ambients[sound];
                return;
            }
        }
        if (i == 4) {
            slot = 0;
            for (i = 1; i < 4; i++) {
                if (AmbientQueue[i].priority < Ambients[sound].priority)
                    slot = i;
            }
        }
        if (slot)
            AmbientQueue[slot] = Ambients[sound];
    }
}

// Plays sound effect sfx (SFX_*) for sample source `source`, choosing the
// sample and pitch by its sfx_details entry (or stopping it). (x, y) is a
// screen position for its volume and pan, -1 for full volume in the centre.
// Does nothing with sound effects off.
// FUNCTION: 0x00088ebc
void play_sound(u32 source, u16 sfx, s32 x, s32 y)
{
    u32 pitch;
    u16 sample;
    s32 pan;
    s32 volume;
    s32 half;

    if (myplayer.sfx_on && statvar.sound_ok) {
        if (sfx < 0xf2) {
            switch (sfx_details[sfx].type) {
            case SFXT_NONE:
                break;
            case SFXT_SAMPLE:
                pitch = 100;
                sample = sfx_details[sfx].sample;
                break;
            case SFXT_STOP:
                StopSample(source, sfx_details[sfx].sample);
                return;
            case SFXT_RANDOM_PITCH:
                pitch =
                    pitch_group[sfx_details[sfx].group].values[saferand() % pitch_group[sfx_details[sfx].group].count];
                sample = sfx_details[sfx].sample;
                break;
            case SFXT_RANDOM_SAMPLE:
                pitch = 100;
                sample = sample_group[sfx_details[sfx].group]
                             .samples[saferand() % sample_group[sfx_details[sfx].group].count];
                break;
            case SFXT_PITCH_RANGE:
                pitch = pitch_group[sfx_details[sfx].group].values[0] +
                        saferand() % (pitch_group[sfx_details[sfx].group].values[1] -
                                      pitch_group[sfx_details[sfx].group].values[0]);
                sample = sfx_details[sfx].sample;
                break;
            }
            if (x == -1 || y == -1) {
                pan = 0x40;
                volume = myplayer.sfx_volume;
            } else {
                half = lbDisplay.physical_screen_width / 2;
                volume = (half - ABS(x - half)) * myplayer.sfx_volume / half;
                pan = x * 0x7f / lbDisplay.physical_screen_width;
                if (volume < 0)
                    volume = 0;
                else if (volume > myplayer.sfx_volume)
                    volume = myplayer.sfx_volume;
                if (pan < 0)
                    pan = 0;
                else if (pan > 0x7f)
                    pan = 0x7f;
            }
            play_sample_using_heap(source, sample, volume, pan, pitch, sfx_details[sfx].loop,
                                   sfx_details[sfx].play_type);
        }
    }
}

// Per frame: adjusts the volume and pan of samples whose source is a thing
// to its position on screen, stopping those whose thing is off screen or
// whose source is not a thing.
// FUNCTION: 0x00089204
void manage_sound(void)
{
    SampleInfo *sample;
    Thing *source;
    s32 volume;
    s32 pan;
    s32 half;

    for (sample = GetFirstSampleInfoStructure(); sample <= GetLastSampleInfoStructure(); sample++) {
        if (IsSamplePlaying(0, 0, sample->SampleHandle)) {
            if (sample->SourceID > 0xe) {
                if ((Thing *)sample->SourceID > thing && (Thing *)sample->SourceID < end_things) {
                    source = (Thing *)sample->SourceID;
                    give_screen_xy_of_cell_at_column_row(window.current_window, source->x.tile, source->y.tile);
                    info.cell_x += resvars.tile_width / 2;
                    info.cell_y += resvars.tile_height / 2;
                    if (sample->SourceID == SNDSRC_CLOCK) {
                        if (myhosp->emergency_timer_s == 0)
                            StopSample(sample->SourceID, sample->SampleNumber);
                    } else if (info.cell_x > 0 && info.cell_x < lbDisplay.physical_screen_width && info.cell_y > 0 &&
                               info.cell_y < lbDisplay.physical_screen_height) {
                        half = lbDisplay.physical_screen_width / 2;
                        volume = (half - ABS(info.cell_x - half)) * myplayer.sfx_volume / half;
                        pan = info.cell_x * 0x7f / lbDisplay.physical_screen_width;
                        if (volume < 0)
                            volume = 0;
                        else if (volume > myplayer.sfx_volume)
                            volume = myplayer.sfx_volume;
                        if (pan < 0)
                            pan = 0;
                        else if (pan > 0x7f)
                            pan = 0x7f;
                        if (sample->SamplePan != pan)
                            SetSamplePan(0, 0, pan, sample->SampleHandle);
                        if (sample->SampleVolume != volume)
                            SetSampleVolume(0, 0, volume, sample->SampleHandle);
                    } else {
                        StopSample(sample->SourceID, sample->SampleNumber);
                    }
                } else {
                    StopSample(sample->SourceID, sample->SampleNumber);
                }
            }
        }
    }
}

// Per turn: keeps the crowd sound looping at a volume set by the people on
// screen (othervar.visible_people), plays random background samples, the emergency
// clock (louder as time runs out) and the earthquake rumble.
// FUNCTION: 0x00089468
void manage_background_sounds(void)
{
    s32 sample;
    s32 volume;

    if (myplayer.sfx_on && statvar.sound_ok && !gamevar.paused) {
        volume = othervar.visible_people * myplayer.sfx_volume / 20;
        if (volume < 0)
            volume = 0;
        else if (volume > myplayer.sfx_volume)
            volume = myplayer.sfx_volume;
        play_sample_using_heap(0xc, 200, volume, 0x40, 100, -1, 2);
        if (othervar.crowd_volume != volume) {
            SetSampleVolume(0xc, 200, volume, 0);
            othervar.crowd_volume = volume;
        }
        othervar.ambient_noise_timer++;
        if (othervar.ambient_noise_timer > 400) {
            sample = saferand() % 30 + 0x10e;
            play_sample_using_heap(SNDSRC_GENERAL, sample, volume, 0x40, 100, 0, 2);
            othervar.ambient_noise_timer = 0;
        }
        if (myhosp->emergency_timer) {
            volume = myplayer.sfx_volume / 2;
            if (myhosp->emergency_timer_s < 0x21)
                volume += (0x21 - myhosp->emergency_timer_s) * myplayer.sfx_volume / 0x42;
            if (volume < 0)
                volume = 0;
            else if (volume > myplayer.sfx_volume)
                volume = myplayer.sfx_volume;
            play_sample_using_heap(SNDSRC_CLOCK, 0xa9, volume, 0x40, 100, -1, 2);
            if (othervar.emergency_clock_volume != volume) {
                SetSampleVolume(SNDSRC_CLOCK, 0xa9, volume, 0);
                othervar.emergency_clock_volume = volume;
            }
        }
        if (gamevar.quake_strength)
            play_sample_using_heap(0xc, 0x48, myplayer.sfx_volume, 0x40, 100, -1, 2);
    }
}

// Resets the jukebox: all ten tracks on, track 8 current, and loads the
// track list.
// FUNCTION: 0x000896b0
void init_midi(void)
{
    u8 i;

    music.unused_set_by_init_midi = 1;
    music.volume = myplayer.music_volume;
    for (i = 0; i < 10; i++)
        myplayer.track_enabled[i] = 1;
    othervar.midi_playing = 1;
    othervar.midi_track = 8;
    load_midi();
}

// If no track is playing, plays the next enabled track (or repeats the
// current one when looping), clearing othervar.midi_playing if none can be loaded.
// FUNCTION: 0x00089718
void play_next_midi(void)
{
    char buf[100];
    u8 i;

    othervar.midi_playing = 1;
    if (!is_midi_playing()) {
        if (othervar.loop_track) {
            if (myplayer.track_enabled[othervar.midi_track]) {
                sprintf(buf, "%s\\midi/%s", statvar.sound_path, midi_files[othervar.midi_track].name);
                if (LbFileLoadAt(buf, midi_buffer) != -1) {
                    PlayMidiFromAddress(myplayer.music_volume, midi_buffer);
                    othervar.music_playing = 1;
                } else {
                    othervar.midi_playing = 0;
                }
            }
        } else {
            for (i = 0; i < statvar.midi_count; i++) {
                othervar.midi_track++;
                if (othervar.midi_track > statvar.midi_count)
                    othervar.midi_track = 0;
                if (myplayer.track_enabled[othervar.midi_track]) {
                    sprintf(buf, "%s\\midi/%s", statvar.sound_path, midi_files[othervar.midi_track].name);
                    if (LbFileLoadAt(buf, midi_buffer) != -1) {
                        PlayMidiFromAddress(myplayer.music_volume, midi_buffer);
                        othervar.music_playing = 1;
                        break;
                    }
                }
            }
            if (i == statvar.midi_count)
                othervar.midi_playing = 0;
        }
    }
}

// Plays music track `track`, or the next enabled one if it is disabled or
// cannot be loaded. Does nothing with music off or unavailable.
// FUNCTION: 0x0008987c
void play_midi(u8 track)
{
    char buf[100];

    AIL_end_sequence(GetSongHandle());
    if (myplayer.music_on && statvar.music_ok && othervar.midi_playing) {
        if (myplayer.track_enabled[track]) {
            sprintf(buf, "%s\\midi/%s", statvar.sound_path, midi_files[track].name);
            if (LbFileLoadAt(buf, midi_buffer) != -1) {
                PlayMidiFromAddress(myplayer.music_volume, midi_buffer);
                othervar.midi_track = track;
                othervar.music_playing = 1;
            } else {
                play_next_midi();
            }
        } else {
            play_next_midi();
        }
    }
}

// Despite its name, plays the previous track, not the last one (the mirror of
// play_next_midi): if no track is playing, plays the previous enabled track
// (or repeats the current one when looping), clearing othervar.midi_playing
// if none can be loaded.
// FUNCTION: 0x00089940
void play_last_midi(void)
{
    char buf[100];
    u8 i;

    othervar.midi_playing = 1;
    if (!is_midi_playing()) {
        if (othervar.loop_track) {
            if (myplayer.track_enabled[othervar.midi_track]) {
                sprintf(buf, "%s\\midi/%s", statvar.sound_path, midi_files[othervar.midi_track].name);
                if (LbFileLoadAt(buf, midi_buffer) != -1) {
                    PlayMidiFromAddress(myplayer.music_volume, midi_buffer);
                    othervar.music_playing = 1;
                } else {
                    othervar.midi_playing = 0;
                }
            }
        } else {
            for (i = 0; i < statvar.midi_count; i++) {
                if (othervar.midi_track == 0)
                    othervar.midi_track = statvar.midi_count;
                else
                    othervar.midi_track--;
                if (myplayer.track_enabled[othervar.midi_track]) {
                    sprintf(buf, "%s\\midi/%s", statvar.sound_path, midi_files[othervar.midi_track].name);
                    if (LbFileLoadAt(buf, midi_buffer) != -1) {
                        PlayMidiFromAddress(myplayer.music_volume, midi_buffer);
                        othervar.music_playing = 1;
                        break;
                    }
                }
            }
            if (i == statvar.midi_count)
                othervar.midi_playing = 0;
        }
    }
}

// Returns 0 if music should be playing but the current song has finished,
// else 1 (always 1 while the game is paused).
// FUNCTION: 0x00089aa8
u8 is_midi_playing(void)
{
    if (gamevar.paused)
        return 1;
    if (myplayer.music_on && statvar.music_ok) {
        if (othervar.midi_playing) {
            if (othervar.music_playing) {
                if (AIL_sequence_status(GetSongHandle()) != 4)
                    return 0;
            }
        }
    }
    return 1;
}

// Reads the jukebox's track list (file name and title pairs) from
// midi/midi.txt, keeping up to ten tracks whose files exist; statvar.midi_count
// is their count. On failure, or with no tracks, it turns off the sound
// effects flags (statvar.sound_ok, myplayer.sfx_on), not the music ones; a
// failure to read the file also clears othervar.midi_playing.
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
    sprintf(name, "%s\\midi/midi.txt", statvar.sound_path);
    len = LbFileLengthRnc(name);
    if (len == -1) {
        othervar.midi_playing = 0;
        statvar.sound_ok = 0;
        myplayer.sfx_on = 0;
        return;
    }
    buf = (char *)LbMemoryAlloc(len + 1);
    if (buf == 0) {
        othervar.midi_playing = 0;
        statvar.sound_ok = 0;
        myplayer.sfx_on = 0;
        return;
    }
    if (LbFileLoadAt(name, buf) == -1) {
        othervar.midi_playing = 0;
        statvar.sound_ok = 0;
        myplayer.sfx_on = 0;
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
        sprintf(path, "%s\\midi/%s", statvar.sound_path, line);
        handle = LbFileOpen(path, 2);
        if (handle != -1) {
            LbFileClose(handle);
            sprintf(midi_files[i].name, "%s", line);
            read_line_from_buf(line, 0x80, buf);
            for (j = 0; j < 26; j++) {
                if (!isprint(line[j])) {
                    line[j] = 0;
                    break;
                }
            }
            line[0x19] = 0;
            sprintf(midi_files[i].title, "%s", line);
            LbMemorySet(line, 0, 0x80);
            i++;
            if (i >= 10)
                break;
        }
    }
    statvar.midi_count = i;
    if (statvar.midi_count == 0) {
        statvar.sound_ok = 0;
        myplayer.sfx_on = 0;
    }
    read_line_from_buf(0, 1, 0);
    LbMemoryFree(buf);
}
