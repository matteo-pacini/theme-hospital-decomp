#include "ail.h"

#define MODULATION 1
#define DATA_MSB 6
#define PART_VOLUME 7
#define PANPOT 10
#define EXPRESSION 11
#define SUSTAIN 64
#define REVERB 91
#define CHORUS 93
#define CHAN_LOCK 110
#define CHAN_PROTECT 111
#define VOICE_PROTECT 112
#define PATCH_BANK_SEL 114
#define INDIRECT_C_PFX 115
#define CALLBACK_TRIG 119

#define MAX_W_VOICES 16

typedef struct {
    S32 bank;
    S32 patch;
    S32 root_key;
    U32 file_offset;
    U32 size;
    S32 format;
    U32 flags;
    S32 playback_rate;
} WAVE_ENTRY;

typedef struct _WAVE_SYNTH {
    HMDIDRIVER mdi;
    HDIGDRIVER dig;
    WAVE_ENTRY *library;
    AILEVENTCB prev_event_fn;
    AILTIMBRECB prev_timb_fn;
    CTRL_LOG controls;
    WAVE_ENTRY *wave[NUM_CHANS];
    HSAMPLE S[MAX_W_VOICES];
    S32 n_voices;
    S32 chan[MAX_W_VOICES];
    S32 note[MAX_W_VOICES];
    S32 root[MAX_W_VOICES];
    S32 rate[MAX_W_VOICES];
    S32 vel[MAX_W_VOICES];
    U32 time[MAX_W_VOICES];
    U32 event;
} WAVE_SYNTH;

static S32 locked = 0;
/* In the code segment, where the original's table sits between two functions. */
#pragma data_seg("_TEXT", "CODE")
// Data, not a function: the pitch of each MIDI note in Hz, rounded and an
// octave high (440 at note 57) except entry 0. Only ratios are used.
// FUNCTION: 0xe5430
S32 note_freq[128] = {
    8, 17, 18, 19, 20, 21, 23, 24,
    26, 27, 29, 30, 32, 34, 36, 38,
    41, 43, 46, 49, 51, 55, 58, 61,
    65, 69, 73, 77, 82, 87, 92, 98,
    103, 110, 116, 123, 130, 138, 146, 155,
    164, 174, 185, 196, 207, 220, 233, 247,
    261, 277, 293, 311, 329, 349, 370, 392,
    415, 440, 466, 493, 523, 554, 587, 622,
    659, 698, 740, 784, 830, 880, 932, 987,
    1046, 1108, 1174, 1244, 1318, 1396, 1480, 1568,
    1661, 1760, 1864, 1975, 2093, 2217, 2349, 2488,
    2636, 2793, 2960, 3136, 3322, 3520, 3729, 3951,
    4186, 4435, 4699, 4977, 5273, 5587, 5920, 6272,
    6644, 7040, 7459, 7902, 8372, 8870, 9398, 9955,
    10547, 11174, 11840, 12544, 13289, 14080, 14918, 15804,
    16745, 17740, 18796, 19910, 21094, 22348, 23680, 25088,
};
#pragma data_seg()
#pragma aux note_freq "fn_000e5430_";


/* Timbre and MIDI event traps: __pascal (callee pops). */
S32 XDIG_TIMB_TRAP(HMDIDRIVER mdi, S32 bank, S32 patch);
#pragma aux (__pascal) XDIG_TIMB_TRAP "*";
S32 XDIG_MIDI_TRAP(HMDIDRIVER mdi, HSEQUENCE seq, S32 status, S32 data_1, S32 data_2);
#pragma aux (__pascal) XDIG_MIDI_TRAP "*";
void AILXDIG_end(void);

static void __cdecl XMI_write_log(CTRL_LOG *log, S32 status, S32 data_1, S32 data_2);

void XDIG_set_pitch(HWAVESYNTH W, S32 voice);

// Locks this module's code against paging, once.
// FUNCTION: 0x000e5630
void AILXDIG_start(void)
{
    if (!locked) {
        AIL_vmm_lock_range(AILXDIG_start, AILXDIG_end);
        locked = 1;
    }
}

// Records a program change, pitch bend or tracked controller (`status`,
// `data_1`, `data_2`) in channel state `log`; other events are ignored.
// FUNCTION: 0x000e5660
static void __cdecl XMI_write_log(CTRL_LOG *log, S32 status, S32 data_1, S32 data_2)
{
    S32 st;
    S32 ch;

    st = status & 0xf0;
    ch = status & 0x0f;
    switch (st) {
    case 0xc0:
        log->program[ch] = (S8)data_1;
        break;
    case 0xe0:
        log->pitch_l[ch] = (S8)data_1;
        log->pitch_h[ch] = (S8)data_2;
        break;
    case 0xb0:
        switch (data_1) {
        case CHAN_LOCK:
            log->c_lock[ch] = (S8)data_2;
            break;
        case CHAN_PROTECT:
            log->c_prot[ch] = (S8)data_2;
            break;
        case VOICE_PROTECT:
            log->c_v_prot[ch] = (S8)data_2;
            break;
        case PATCH_BANK_SEL:
            log->bank[ch] = (S8)data_2;
            break;
        case INDIRECT_C_PFX:
            log->indirect[ch] = (S8)data_2;
            break;
        case CALLBACK_TRIG:
            log->callback[ch] = (S8)data_2;
            break;
        case MODULATION:
            log->mod[ch] = (S8)data_2;
            break;
        case PART_VOLUME:
            log->vol[ch] = (S8)data_2;
            break;
        case PANPOT:
            log->pan[ch] = (S8)data_2;
            break;
        case EXPRESSION:
            log->exp[ch] = (S8)data_2;
            break;
        case SUSTAIN:
            log->sus[ch] = (S8)data_2;
            break;
        case REVERB:
            log->reverb[ch] = (S8)data_2;
            break;
        case CHORUS:
            log->chorus[ch] = (S8)data_2;
            break;
        case DATA_MSB:
            log->bend_range[ch] = (S8)data_2;
            break;
        }
        break;
    }
}

// Returns the entry for `bank` and `patch` in wave library `list` (ended by
// bank -1, at most 512 entries), or NULL.
// FUNCTION: 0x000e5820
WAVE_ENTRY *WVL_search(WAVE_ENTRY *list, S32 bank, S32 patch)
{
    S32 i;

    for (i = 0; i < 512; i++) {
        if (list->bank == -1) {
            break;
        }
        if (list->bank == bank && list->patch == patch) {
            return list;
        }
        list++;
    }
    return NULL;
}

// Sets voice `voice`'s sample volume from its channel's volume, expression and
// the note velocity, and its pan from the channel's pan.
// FUNCTION: 0x000e5850
void XDIG_set_volume(HWAVESYNTH W, S32 voice)
{
    S32 ch;
    S32 vol;

    ch = W->chan[voice];
    vol = (W->controls.vol[ch] * W->controls.exp[ch]) / 127;
    vol = (W->vel[voice] * vol) / 127;
    AIL_set_sample_volume(W->S[voice], vol);
    AIL_set_sample_pan(W->S[voice], W->controls.pan[ch]);
}

// Sets voice `voice`'s playback rate for its note, transposed from the wave's
// root key (not on the drum channel 10) and bent by the channel's pitch bend
// over its bend range.
// FUNCTION: 0x000e58d0
void XDIG_set_pitch(HWAVESYNTH W, S32 voice)
{
    S32 ch;
    S32 root;
    S32 rate;
    S32 note;
    S32 pitch;
    S32 dir;
    S32 bend_note;
    S32 root_freq;
    S32 base;
    S32 freq;

    ch = W->chan[voice];
    root = W->root[voice];
    rate = W->rate[voice];
    note = W->note[voice];
    pitch = W->controls.pitch_l[ch] | (W->controls.pitch_h[ch] << 7);
    if (pitch > 0x2000) {
        dir = 1;
        pitch -= 0x1fff;
    } else if (pitch < 0x2000) {
        pitch = 0x2000 - pitch;
        dir = -1;
    } else {
        pitch = 0;
        dir = 0;
    }
    bend_note = dir * W->controls.bend_range[ch] + note;
    if (bend_note < 0) {
        bend_note = 0;
    }
    if (bend_note > 127) {
        bend_note = 127;
    }
    root_freq = note_freq[root];
    base = note_freq[note];
    freq = base + ((note_freq[bend_note] - base) * pitch) / 8192;
    if (ch != 9) {
        rate = ((rate * base) / root_freq) * freq / base;
    } else {
        rate = (rate * freq) / base;
    }
    AIL_set_sample_playback_rate(W->S[voice], rate);
}

// Timbre callback on the MIDI driver: returns 1 if the wave library has
// `bank`/`patch`, so that timbre is not loaded into the MIDI driver.
// FUNCTION: 0xe59e0
S32 XDIG_TIMB_TRAP(HMDIDRIVER mdi, S32 bank, S32 patch)
{
    HWAVESYNTH W = (HWAVESYNTH)mdi->system_data[0];

    return WVL_search(W->library, bank, patch) != NULL;
}

// Event callback on the MIDI driver: plays note-ons with a wave in the library
// as digital samples (drum channel 10 from bank 127, the note as patch),
// stealing the oldest voice when all are busy, and ends them on note-off
// (except drums) or all-notes-off; volume, pan, expression, bend range and
// pitch bend changes update the playing voices. Returns 1 if it handled a note
// (so the driver does not play it), else 0.
// FUNCTION: 0xe5a10
S32 XDIG_MIDI_TRAP(HMDIDRIVER mdi, HSEQUENCE seq, S32 status, S32 data_1, S32 data_2)
{
    HWAVESYNTH W;
    S32 st;
    S32 ch;
    S32 i;
    S32 j;
    U32 oldest;
    WAVE_ENTRY *wave;

    st = status & 0xf0;
    ch = status & 0x0f;
    W = (HWAVESYNTH)mdi->system_data[0];
    if (st == 0xb0 || st == 0xc0 || st == 0xe0) {
        XMI_write_log(&W->controls, status, data_1, data_2);
    }
    if (st == 0x90 && data_2 == 0) {
        st = 0x80;
    }
    switch (st) {
    case 0xb0:
        switch (data_1) {
        case PART_VOLUME:
        case PANPOT:
        case EXPRESSION:
            for (i = 0; i < W->n_voices; i++) {
                if (W->chan[i] == ch && AIL_sample_status(W->S[i]) == SMP_PLAYING) {
                    XDIG_set_volume(W, i);
                }
            }
            break;
        case DATA_MSB:
            for (i = 0; i < W->n_voices; i++) {
                if (W->chan[i] == ch && AIL_sample_status(W->S[i]) == SMP_PLAYING) {
                    XDIG_set_pitch(W, i);
                }
            }
            break;
        case 123:
            for (i = 0; i < W->n_voices; i++) {
                if (W->chan[i] == ch && AIL_sample_status(W->S[i]) == SMP_PLAYING) {
                    AIL_end_sample(W->S[i]);
                }
            }
            break;
        }
        break;
    case 0xc0:
        W->wave[ch] = WVL_search(W->library, W->controls.bank[ch], data_1);
        break;
    case 0xe0:
        for (i = 0; i < W->n_voices; i++) {
            if (W->chan[i] == ch && AIL_sample_status(W->S[i]) == SMP_PLAYING) {
                XDIG_set_pitch(W, i);
            }
        }
        break;
    case 0x90:
        if (ch == 9) {
            W->wave[ch] = WVL_search(W->library, 127, data_1);
        }
        if (W->wave[ch] == NULL) {
            break;
        }
        for (i = 0; i < W->n_voices; i++) {
            if (AIL_sample_status(W->S[i]) == SMP_DONE) {
                break;
            }
        }
        if (i == W->n_voices) {
            oldest = 0xffffffff;
            for (j = 0; j < W->n_voices; j++) {
                if (W->time[j] <= oldest) {
                    oldest = W->time[j];
                    i = j;
                }
            }
            AIL_end_sample(W->S[i]);
        }
        W->chan[i] = ch;
        W->note[i] = data_1;
        W->root[i] = W->wave[ch]->root_key;
        W->rate[i] = W->wave[ch]->playback_rate;
        W->vel[i] = data_2;
        W->time[i] = W->event++;
        AIL_init_sample(W->S[i]);
        AIL_set_sample_type(W->S[i], W->wave[ch]->format, W->wave[ch]->flags);
        AIL_set_sample_address(W->S[i], (U8 *)W->library + W->wave[ch]->file_offset, W->wave[ch]->size);
        XDIG_set_pitch(W, i);
        XDIG_set_volume(W, i);
        AIL_start_sample(W->S[i]);
        return 1;
    case 0x80:
        if (ch == 9) {
            break;
        }
        for (i = 0; i < W->n_voices; i++) {
            if (W->chan[i] == ch && W->note[i] == data_1 && AIL_sample_status(W->S[i]) == SMP_PLAYING) {
                AIL_end_sample(W->S[i]);
                return 1;
            }
        }
        break;
    }
    return 0;
}

// Creates a wave synthesizer that plays `mdi`'s notes found in wave library
// `wave_lib` as samples on `dig`, with up to `polyphony` voices (at most 16),
// by hooking `mdi`'s event and timbre callbacks. Returns it, or NULL with
// AIL_error set if memory or sample handles run out.
// FUNCTION: 0x000e5e30
HWAVESYNTH __cdecl AIL_API_create_wave_synthesizer(HDIGDRIVER dig, HMDIDRIVER mdi,
                                                   void *wave_lib, S32 polyphony)
{
    HWAVESYNTH W;
    S32 i;

    AILXDIG_start();
    W = AIL_mem_alloc_lock(sizeof(WAVE_SYNTH));
    if (W == NULL) {
        strcpy(AIL_error, "Insufficient memory for HWAVE descriptor\n");
        return NULL;
    }
    W->dig = dig;
    W->mdi = mdi;
    W->library = wave_lib;
    W->prev_event_fn = AIL_register_event_callback(mdi, (AILEVENTCB)XDIG_MIDI_TRAP);
    W->prev_timb_fn = AIL_register_timbre_callback(mdi, (AILTIMBRECB)XDIG_TIMB_TRAP);
    mdi->system_data[0] = (S32)W;
    for (i = 0; i < NUM_CHANS; i++) {
        W->controls.pitch_h[i] = 0x40;
        W->controls.pan[i] = 64;
        W->controls.program[i] = 0;
        W->controls.pitch_l[i] = 0;
        W->controls.bank[i] = 0;
        W->controls.vol[i] = 127;
        W->controls.exp[i] = 127;
        W->controls.bend_range[i] = AIL_preference[MDI_DEFAULT_BEND_RANGE];
    }
    for (W->n_voices = 0;
         W->n_voices < (polyphony > MAX_W_VOICES ? MAX_W_VOICES : polyphony);
         W->n_voices++) {
        W->S[W->n_voices] = AIL_allocate_sample_handle(W->dig);
        if (W->S[W->n_voices] == NULL) {
            strcpy(AIL_error, "Could not allocate enough SAMPLE handles\n");
            for (--W->n_voices; W->n_voices >= 0; W->n_voices--) {
                AIL_release_sample_handle(W->S[W->n_voices]);
            }
            AIL_mem_free_lock(W);
            return NULL;
        }
    }
    for (i = 0; i < NUM_CHANS; i++) {
        W->wave[i] = NULL;
    }
    for (i = 0; i < W->n_voices; i++) {
        W->chan[i] = -1;
    }
    W->event = 0;
    return W;
}

// Releases wave synthesizer `W`'s samples, restores the MIDI driver's previous
// event and timbre callbacks and frees it. NULL is ignored.
// FUNCTION: 0x000e6020
void __cdecl AIL_API_destroy_wave_synthesizer(HWAVESYNTH W)
{
    S32 i;

    if (W == NULL) {
        return;
    }
    for (i = 0; i < W->n_voices; i++) {
        AIL_release_sample_handle(W->S[i]);
    }
    AIL_register_event_callback(W->mdi, W->prev_event_fn);
    AIL_register_timbre_callback(W->mdi, W->prev_timb_fn);
    AIL_mem_free_lock(W);
}

// Unlocks this module's code. Its address also marks the end of the locked code
// range.
// FUNCTION: 0xe6090
void AILXDIG_end(void)
{
    if (locked) {
        AIL_vmm_unlock_range(AILXDIG_start, AILXDIG_end);
        locked = 0;
    }
}
