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

extern S32 locked;
extern S32 note_freq[128];
#pragma aux note_freq "fn_000e5430_";

extern char D_00127090[];
extern char D_001270bc[];

void fn_000e59e0(void);
void fn_000e5a10(void);
void fn_000e6090(void);

void __cdecl XDIG_write_log(CTRL_LOG *log, S32 status, S32 data_1, S32 data_2);
#pragma aux (__cdecl) XDIG_write_log "fn_000e5660_";

void XDIG_MIDI_TRAP(HWAVESYNTH W, S32 voice);
#pragma aux XDIG_MIDI_TRAP "XDIG_MIDI_TRAP";

// FUNCTION: 0x000e5630
void AILXDIG_start(void)
{
    if (!locked) {
        AIL_vmm_lock_range(AILXDIG_start, fn_000e6090);
        locked = 1;
    }
}

// FUNCTION: 0x000e5660
void __cdecl XDIG_write_log(CTRL_LOG *log, S32 status, S32 data_1, S32 data_2)
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

// FUNCTION: 0x000e58d0
void XDIG_MIDI_TRAP(HWAVESYNTH W, S32 voice)
{
    S32 ch;
    S32 root;
    S32 rate;
    S32 note;
    S32 pitch;
    S32 dir;
    S32 n;
    S32 root_freq;
    S32 base;
    S32 f;

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
    n = dir * W->controls.bend_range[ch] + note;
    if (n < 0) {
        n = 0;
    }
    if (n > 127) {
        n = 127;
    }
    root_freq = note_freq[root];
    base = note_freq[note];
    f = base + ((note_freq[n] - base) * pitch) / 8192;
    if (ch != 9) {
        rate = ((rate * base) / root_freq) * f / base;
    } else {
        rate = (rate * f) / base;
    }
    AIL_set_sample_playback_rate(W->S[voice], rate);
}

// FUNCTION: 0x000e5e30
// NONMATCHING: the target loads dig before storing W->mdi
HWAVESYNTH __cdecl AIL_API_create_wave_synthesizer(HDIGDRIVER dig, HMDIDRIVER mdi,
                                                   void *wave_lib, S32 polyphony)
{
    HWAVESYNTH W;
    S32 i;

    AILXDIG_start();
    W = AIL_mem_alloc_lock(sizeof(WAVE_SYNTH));
    if (W == NULL) {
        strcpy(AIL_error, D_00127090);
        return NULL;
    }
    W->mdi = mdi;
    W->dig = dig;
    W->library = wave_lib;
    W->prev_event_fn = AIL_register_event_callback(mdi, (AILEVENTCB)fn_000e5a10);
    W->prev_timb_fn = AIL_register_timbre_callback(mdi, (AILTIMBRECB)fn_000e59e0);
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
            strcpy(AIL_error, D_001270bc);
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
