#include "ail.h"

#define MODULATION 1
#define DATA_MSB 6
#define PART_VOLUME 7
#define PANPOT 10
#define EXPRESSION 11
#define DATA_LSB 38
#define SUSTAIN 64
#define REVERB 91
#define CHORUS 93
#define RPN_LSB 100
#define RPN_MSB 101
#define CHAN_MUTE 107
#define CALLBACK_PFX 108
#define SEQ_BRANCH 109
#define CHAN_LOCK 110
#define CHAN_PROTECT 111
#define VOICE_PROTECT 112
#define PATCH_BANK_SEL 114
#define INDIRECT_C_PFX 115
#define FOR_LOOP 116
#define NEXT_LOOP 117
#define CLEAR_BEAT_BAR 118
#define CALLBACK_TRIG 119

#define MIN_LOCK_CHAN 1
#define MAX_LOCK_CHAN 8
#define PERCUSS_CHAN 9

#pragma pack(1)
typedef struct {
    char tag[4];
    U8 len_msb;
    U8 len_lsb;
    U8 len_pad[2];
    U16 n_entries;
    U16 timbre[1];
} TIMB_chunk;

typedef struct {
    U16 offset;
    U32 pos;
} RBRN_entry;

typedef struct {
    char tag[4];
    U32 len;
    S16 n_entries;
    RBRN_entry entry[1];
} RBRN_block;

typedef struct {
    char tag[4];
    U32 len;
    U16 n_entries;
    U8 patch;
    U8 bank;
} TIMB_request;
#pragma pack()

extern S32 locked;
extern char GTL_prefix[128];
extern S32 entry;
extern HSEQUENCE S;
extern S32 i;
extern S32 j;
extern S32 n;
extern S32 sequence_done;
extern S32 q;
extern S32 t;
extern S32 channel;
extern S32 status;
extern S32 type;
extern S32 len;
extern U8 *ptr;
extern S32 event;

extern TIMB_request D_00151634;
extern U8 D_0018cb6c[512];

extern char D_00126e6c[];
extern char D_00126e74[];
extern char D_00126e7c[];
extern char D_00126ea4[];
extern char D_00126ecc[];
extern char D_00126ed4[];
extern char D_00126eec[];
extern char D_00126f0c[];
extern char D_00126f38[];
extern char D_00126f60[];
extern char D_00126f68[];
extern char D_00126f80[];
extern char D_00126f98[];
extern char D_00126fa0[];
extern char D_00126fc0[];
extern char D_00126fd4[];
extern char D_00126ff0[];
extern char D_00126ff4[];
extern char D_00126ffc[];
extern char D_00127018[];
extern char D_0012704c[];
extern char D_00127064[];
extern char D_0012706c[];
extern char D_00127074[];
extern char D_0012707c[];

void fn_000e2ff0(void);
void fn_000e5320(void);
void __syscall XMI_SERVE(HSEQUENCE S);
void __cdecl XMI_destroy_MDI_driver(HMDIDRIVER mdi);

// FUNCTION: 0x000e1ed0
void AILXMIDI_start(void)
{
    if (!locked) {
        AIL_vmm_lock_range(AILXMIDI_start, fn_000e5320);
        AIL_vmm_lock(GTL_prefix, sizeof(GTL_prefix));
        AIL_vmm_lock(&entry, sizeof(entry));
        AIL_vmm_lock(&S, sizeof(S));
        AIL_vmm_lock(&i, sizeof(i));
        AIL_vmm_lock(&j, sizeof(j));
        AIL_vmm_lock(&n, sizeof(n));
        AIL_vmm_lock(&sequence_done, sizeof(sequence_done));
        AIL_vmm_lock(&q, sizeof(q));
        AIL_vmm_lock(&t, sizeof(t));
        AIL_vmm_lock(&channel, sizeof(channel));
        AIL_vmm_lock(&status, sizeof(status));
        AIL_vmm_lock(&type, sizeof(type));
        AIL_vmm_lock(&len, sizeof(len));
        AIL_vmm_lock(&ptr, sizeof(ptr));
        AIL_vmm_lock(&event, sizeof(event));
        locked = 1;
    }
}

// FUNCTION: 0x000e1ff0
S32 __cdecl XMI_message_size(S32 status)
{
    switch (status & 0xf0) {
    case 0x80:
    case 0x90:
    case 0xa0:
    case 0xb0:
    case 0xe0:
        return 3;
    case 0xc0:
    case 0xd0:
        return 2;
    }
    return 0;
}

// FUNCTION: 0x000e2050
void __cdecl XMI_flush_buffer(HMDIDRIVER mdi)
{
    VDI_CALL VDI;

    if (mdi->message_count > 0) {
        VDI.CX = mdi->message_count;
        AIL_call_driver(mdi->drvr, MDI_MIDI_XMIT, &VDI, NULL);
        mdi->message_count = 0;
        mdi->offset = 0;
    }
}

// FUNCTION: 0x000e20a0
void __cdecl XMI_MIDI_message(HMDIDRIVER mdi, S32 status, S32 d1, S32 d2)
{
    S32 size;
    S32 n;
    U8 b1;
    U8 st;
    U8 b2;

    n = 1;
    if (AIL_preference[MDI_DOUBLE_NOTE_OFF]) {
        if ((status & 0xf0) == 0x80 || ((status & 0xf0) == 0x90 && d2 == 0)) {
            n = 2;
        }
    }
    st = status;
    b1 = d1;
    b2 = d2;
    while (n--) {
        size = XMI_message_size(status);
        if (mdi->offset + size > sizeof(mdi->DST->MIDI_data)) {
            XMI_flush_buffer(mdi);
        }
        mdi->DST->MIDI_data[mdi->offset++] = st;
        mdi->DST->MIDI_data[mdi->offset++] = b1;
        if (size == 3) {
            mdi->DST->MIDI_data[mdi->offset++] = b2;
        }
        mdi->message_count++;
    }
}

// FUNCTION: 0x000e21a0
void __cdecl XMI_sysex_message(HMDIDRIVER mdi, U8 *message, U32 size)
{
    XMI_flush_buffer(mdi);
    memcpy(mdi->DST->MIDI_data, message,
           size > sizeof(mdi->DST->MIDI_data) ? sizeof(mdi->DST->MIDI_data) : size);
    mdi->message_count++;
    XMI_flush_buffer(mdi);
}

// FUNCTION: 0x000e2200
S32 __cdecl XMI_read_VLN(U8 **ptr)
{
    S32 value;
    S32 i;
    S32 b;

    value = 0;
    i = 4;
    do {
        b = **ptr;
        value = (value << 7) | (b & 0x7f);
        (*ptr)++;
        if (!(b & 0x80)) {
            break;
        }
    } while (--i);
    return value;
}

// FUNCTION: 0x000e2230
U32 __cdecl XMI_swap(U32 val)
{
    return ((val & 0x000000ff) << 24) + ((val & 0x0000ff00) << 8)
           + ((val & 0x00ff0000) >> 8) + ((val & 0xff000000) >> 24);
}

// FUNCTION: 0x000e2270
U8 *__cdecl XMI_find_sequence(U8 *image, S32 sequence)
{
    U32 len;
    U8 *end;

    len = 0;
    do {
        image += len;
        if (memcmp(image, D_00126e6c, 4) && memcmp(image, D_00126e74, 4)) {
            return NULL;
        }
        len = XMI_swap(*(U32 *)(image + 4)) + 8;
    } while (memcmp(image + 8, D_00126e7c, 4));
    if (!memcmp(image, D_00126e6c, 4)) {
        if (sequence == 0) {
            return image;
        }
        return NULL;
    }
    end = image + len;
    image += 12;
    while (image < end) {
        if (!memcmp(image + 8, D_00126e7c, 4)) {
            if (sequence-- == 0) {
                return image;
            }
        }
        image += XMI_swap(*(U32 *)(image + 4)) + 8;
    }
    return NULL;
}

// FUNCTION: 0x000e2360
S32 __cdecl XMI_read_log(CTRL_LOG *log, S32 status, S32 data_1)
{
    S32 st;
    S32 ch;

    st = status & 0xf0;
    ch = status & 0x0f;
    switch (st) {
    case 0xc0:
        return log->program[ch];
    case 0xe0:
        return (log->pitch_h[ch] << 7) | log->pitch_l[ch];
    case 0xb0:
        switch (data_1) {
        case CHAN_LOCK:
            return log->c_lock[ch];
        case CHAN_PROTECT:
            return log->c_prot[ch];
        case CHAN_MUTE:
            return log->c_mute[ch];
        case VOICE_PROTECT:
            return log->c_v_prot[ch];
        case PATCH_BANK_SEL:
            return log->bank[ch];
        case INDIRECT_C_PFX:
            return log->indirect[ch];
        case CALLBACK_TRIG:
            return log->callback[ch];
        case MODULATION:
            return log->mod[ch];
        case PART_VOLUME:
            return log->vol[ch];
        case PANPOT:
            return log->pan[ch];
        case EXPRESSION:
            return log->exp[ch];
        case SUSTAIN:
            return log->sus[ch];
        case REVERB:
            return log->reverb[ch];
        case CHORUS:
            return log->chorus[ch];
        case DATA_MSB:
            return log->bend_range[ch];
        }
        return -1;
    }
    return -1;
}

// FUNCTION: 0x000e2520
void __cdecl XMI_write_log(CTRL_LOG *log, S32 status, S32 data_1, S32 data_2)
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
        case CHAN_MUTE:
            log->c_mute[ch] = (S8)data_2;
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

void __cdecl XMI_flush_channel_notes(HSEQUENCE S, S32 channel);

// FUNCTION: 0x000e26f0
// NONMATCHING: register allocation (the target keeps mdi on the stack and
// data_1/data_2 in ebp/edi)
void __cdecl XMI_send_channel_voice_message(HSEQUENCE S, S32 status, S32 data_1,
                                            S32 data_2, S32 ICA_enable)
{
    HMDIDRIVER mdi;
    S32 ch;
    S32 st;
    S32 phys;
    S32 i;

    mdi = S->driver;
    st = status & 0xf0;
    ch = status & 0x0f;
    phys = S->chan_map[ch];
    if (st == 0xb0 || st == 0xc0 || st == 0xe0) {
        XMI_write_log(&S->shadow, st | ch, data_1, data_2);
    }
    if (st == 0xb0) {
        if (ICA_enable && S->shadow.indirect[ch] != -1) {
            data_2 = S->shadow.indirect[ch];
            S->shadow.indirect[ch] = -1;
        }
        switch (data_1) {
        case INDIRECT_C_PFX:
            S->shadow.indirect[ch] = S->ICA[data_2];
            break;
        case CALLBACK_PFX:
            if (S->prefix_callback != NULL) {
                S->shadow.indirect[ch] = S->prefix_callback(S, ch, data_2);
            }
            break;
        case DATA_MSB:
            XMI_send_channel_voice_message(S, ch | 0xb0, RPN_LSB, 0, 0);
            XMI_send_channel_voice_message(S, ch | 0xb0, RPN_MSB, 0, 0);
            XMI_send_channel_voice_message(S, ch | 0xb0, DATA_LSB, 0, 0);
            break;
        case PART_VOLUME:
            data_2 = (S->volume * data_2 * mdi->master_volume) / (127 * 127);
            if (data_2 > 127) {
                data_2 = 127;
            }
            if (data_2 < 0) {
                data_2 = 0;
            }
            break;
        case CLEAR_BEAT_BAR:
            S->beat_count = 0;
            S->measure_count = 0;
            S->beat_fraction = 0;
            S->beat_fraction -= S->time_fraction;
            if (S->beat_callback != NULL) {
                S->beat_callback(mdi, S, 0, 0);
            }
            return;
        case CALLBACK_TRIG:
            if (S->trigger_callback != NULL) {
                S->trigger_callback(S, ch, data_2);
            }
            return;
        case FOR_LOOP:
            for (i = 0; i < FOR_NEST; i++) {
                if (S->FOR_loop_count[i] == -1) {
                    break;
                }
            }
            if (i == FOR_NEST) {
                return;
            }
            S->FOR_loop_count[i] = (S8)data_2;
            S->FOR_ptrs[i] = S->EVNT_ptr;
            return;
        case NEXT_LOOP:
            if (data_2 < 64) {
                return;
            }
            for (i = FOR_NEST - 1; i >= 0; i--) {
                if (S->FOR_loop_count[i] != -1) {
                    break;
                }
            }
            if (i == -1) {
                return;
            }
            if (S->FOR_loop_count[i] == 0) {
                S->EVNT_ptr = S->FOR_ptrs[i];
                return;
            }
            if (--S->FOR_loop_count[i]) {
                S->EVNT_ptr = S->FOR_ptrs[i];
                return;
            }
            S->FOR_loop_count[i] = -1;
            return;
        case SEQ_BRANCH:
            AIL_branch_index(S, data_2);
            return;
        case CHAN_PROTECT:
            if (mdi->lock[phys] == 1) {
                return;
            }
            if (data_2 < 64) {
                mdi->lock[phys] = 0;
            } else {
                mdi->lock[phys] = 2;
            }
            return;
        case CHAN_LOCK:
            if (data_2 >= 64) {
                if (mdi->lock[phys] == 1) {
                    return;
                }
                i = AIL_lock_channel(mdi);
                if (i == 0) {
                    return;
                }
                AIL_map_sequence_channel(S, ch + 1, i);
                mdi->locker[i - 1] = S;
            } else {
                if (mdi->lock[phys] != 1) {
                    return;
                }
                XMI_flush_channel_notes(S, ch);
                AIL_release_channel(mdi, phys + 1);
                AIL_map_sequence_channel(S, ch + 1, ch + 1);
            }
            return;
        }
    }
    if (mdi->lock[phys] == 1 && mdi->locker[phys] != S) {
        return;
    }
    if (st == 0x90) {
        mdi->notes[phys]++;
    } else if (st == 0x80) {
        mdi->notes[phys]--;
    }
    mdi->user[phys] = S;
    if (st == 0x90 && S->shadow.c_mute[ch] >= 64) {
        return;
    }
    if (mdi->event_trap != NULL && mdi->event_trap(mdi, S, st | phys, data_1, data_2)) {
        return;
    }
    XMI_MIDI_message(mdi, st | phys, data_1, data_2);
}

// FUNCTION: 0x000e2b80
void __cdecl XMI_flush_note_queue(HSEQUENCE S)
{
    S32 i;
    S32 nmsgs;

    nmsgs = 0;
    for (i = 0; i < MAX_NOTES; i++) {
        if (S->note_chan[i] == -1) {
            continue;
        }
        XMI_send_channel_voice_message(S, S->note_chan[i] | 0x80, S->note_num[i], 0, 0);
        nmsgs++;
        S->note_chan[i] = -1;
    }
    S->note_count = 0;
    XMI_flush_buffer(S->driver);
    if (nmsgs && !AIL_background()) {
        AIL_delay(3);
    }
}

// FUNCTION: 0x000e2c10
void __cdecl XMI_flush_channel_notes(HSEQUENCE S, S32 channel)
{
    S32 i;

    for (i = 0; i < MAX_NOTES; i++) {
        if (S->note_chan[i] != channel) {
            continue;
        }
        XMI_send_channel_voice_message(S, S->note_chan[i] | 0x80, S->note_num[i], 0, 0);
        S->note_chan[i] = -1;
    }
    XMI_flush_buffer(S->driver);
}

// FUNCTION: 0x000e2c80
void __cdecl XMI_refresh_channel(HSEQUENCE S, S32 ch)
{
    if (S->shadow.bank[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xb0, PATCH_BANK_SEL, S->shadow.bank[ch], 0);
    }
    if (S->shadow.program[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xc0, S->shadow.program[ch], 0, 0);
    }
    if (S->shadow.pitch_h[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xe0, S->shadow.pitch_l[ch], S->shadow.pitch_h[ch], 0);
    }
    if (S->shadow.c_mute[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xb0, CHAN_MUTE, S->shadow.c_mute[ch], 0);
    }
    if (S->shadow.c_prot[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xb0, CHAN_PROTECT, S->shadow.c_prot[ch], 0);
    }
    if (S->shadow.c_v_prot[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xb0, VOICE_PROTECT, S->shadow.c_v_prot[ch], 0);
    }
    if (S->shadow.mod[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xb0, MODULATION, S->shadow.mod[ch], 0);
    }
    if (S->shadow.vol[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xb0, PART_VOLUME, S->shadow.vol[ch], 0);
    }
    if (S->shadow.pan[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xb0, PANPOT, S->shadow.pan[ch], 0);
    }
    if (S->shadow.exp[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xb0, EXPRESSION, S->shadow.exp[ch], 0);
    }
    if (S->shadow.sus[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xb0, SUSTAIN, S->shadow.sus[ch], 0);
    }
    if (S->shadow.reverb[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xb0, REVERB, S->shadow.reverb[ch], 0);
    }
    if (S->shadow.chorus[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xb0, CHORUS, S->shadow.chorus[ch], 0);
    }
    if (S->shadow.bend_range[ch] != -1) {
        XMI_send_channel_voice_message(S, ch | 0xb0, DATA_MSB, S->shadow.bend_range[ch], 0);
    }
}

// FUNCTION: 0x000e2ec0
void __cdecl XMI_init_sequence_state(HSEQUENCE S)
{
    S32 i;

    for (i = 0; i < NUM_CHANS; i++) {
        S->chan_map[i] = i;
    }
    memset(&S->shadow, -1, sizeof(S->shadow));
    for (i = 0; i < FOR_NEST; i++) {
        S->FOR_loop_count[i] = -1;
    }
    for (i = 0; i < MAX_NOTES; i++) {
        S->note_chan[i] = -1;
    }
    S->note_count = 0;
    S->interval_count = 0;
    S->beat_count = 0;
    S->measure_count = -1;
    S->beat_fraction = 0;
    S->time_fraction = 0;
    S->time_numerator = 4;
    S->time_per_beat = 8000000;
    S->interval_num = 0;
}

// FUNCTION: 0x000e2f90
void __cdecl XMI_rewind_sequence(HSEQUENCE S)
{
    XMI_init_sequence_state(S);
    S->EVNT_ptr = (U8 *)S->EVNT + 8;
}

// FUNCTION: 0x000e2fb0
void __syscall XMI_SERVE(HSEQUENCE S)
{
    S32 i;

    for (i = 0; i < NUM_CHANS; i++) {
        if (S->shadow.vol[i] != -1) {
            XMI_send_channel_voice_message(S, i | 0xb0, PART_VOLUME, S->shadow.vol[i], 0);
        }
    }
}

// FUNCTION: 0x000e36c0
// NONMATCHING: config/functions.txt sizes this function to also cover
// XMI_destroy_MDI_driver, which follows it
S32 __cdecl XMI_attempt_MDI_detection(HMDIDRIVER mdi, IO_PARMS *IO)
{
    IO_PARMS use;
    IO_PARMS *def;
    S32 i;

    use = *IO;
    if (mdi->drvr->VHDR->num_IO_configurations > 0) {
        def = (IO_PARMS *)(((mdi->drvr->VHDR->common_IO_configurations >> 16) << 4)
                           + (mdi->drvr->VHDR->common_IO_configurations & 0xffff));
        if (use.IO <= 0) {
            use.IO = def->IO;
        }
        if (use.IRQ <= 0) {
            use.IRQ = def->IRQ;
        }
        if (use.DMA_8_bit <= 0) {
            use.DMA_8_bit = def->DMA_8_bit;
        }
        if (use.DMA_16_bit <= 0) {
            use.DMA_16_bit = def->DMA_16_bit;
        }
        for (i = 0; i < 4; i++) {
            if (use.IO_reserved[i] <= 0) {
                use.IO_reserved[i] = def->IO_reserved[i];
            }
        }
    }
    mdi->drvr->VHDR->IO = use;
    return AIL_call_driver(mdi->drvr, DRV_VERIFY_IO, NULL, NULL);
}

// FUNCTION: 0x000e3790
void __cdecl XMI_destroy_MDI_driver(HMDIDRIVER mdi)
{
    S32 i;

    for (i = 0; i < mdi->n_sequences; i++) {
        AIL_end_sequence(&mdi->sequences[i]);
    }
    AIL_release_timer_handle(mdi->timer);
    AIL_mem_free_lock(mdi->sequences);
    AIL_mem_free_lock(mdi);
}

// FUNCTION: 0x000e37f0
// NONMATCHING: the target names XMI_destroy_MDI_driver
// _XMI_attempt_MDI_detection+0xd0 (see above)
HMDIDRIVER __cdecl XMI_construct_MDI_driver(AIL_DRIVER *drvr, IO_PARMS *IO)
{
    IO_PARMS use;
    VDI_CALL out;
    S32 found;
    U32 offset;
    S32 i;
    char *env;
    char *str;
    S32 len;
    IO_PARMS *IO_parms;
    HMDIDRIVER mdi;

    AILXMIDI_start();
    mdi = AIL_mem_alloc_lock(sizeof(struct _MDI_DRIVER));
    if (mdi == NULL) {
        strcpy(AIL_error, D_00126ea4);
        return NULL;
    }
    memcpy(mdi->tag, D_00126ecc, 4);
    mdi->drvr = drvr;
    if (drvr->type != MDI_DRIVER) {
        strcpy(AIL_error, D_00126ed4);
        AIL_mem_free_lock(mdi);
        return NULL;
    }
    AIL_call_driver(drvr, DRV_GET_INFO, NULL, &out);
    mdi->DDT = (MDI_DDT *)(((U32)out.DX << 4) + (U32)out.AX);
    mdi->DST = (MDI_DST *)(((U32)out.CX << 4) + (U32)out.BX);
    env = (char *)(((mdi->DDT->library_environment >> 16) << 4)
                   + (mdi->DDT->library_environment & 0xffff));
    if (env != NULL && *env) {
        str = AIL_getenv(env);
        if (str != NULL && *str) {
            len = strlen(str);
            if (len > 127) {
                len = 127;
            }
            memcpy(mdi->DST->library_directory, str, len);
            mdi->DST->library_directory[len] = 0;
        }
    }
    str = (char *)(((mdi->DDT->GTL_suffix >> 16) << 4) + (mdi->DDT->GTL_suffix & 0xffff));
    if (str != NULL && *str) {
        strcpy(mdi->DST->GTL_filename, GTL_prefix);
        strcat(mdi->DST->GTL_filename, str);
    } else {
        mdi->DST->GTL_filename[0] = 0;
    }
    memset(&AIL_last_IO_attempt, -1, sizeof(AIL_last_IO_attempt));
    found = 0;
    if (IO != NULL) {
        AIL_last_IO_attempt = *IO;
        if (XMI_attempt_MDI_detection(mdi, IO)) {
            found = 1;
            use = *IO;
        }
    }
    if (!found) {
        IO_parms = AIL_get_IO_environment(mdi->drvr);
        if (IO_parms != NULL) {
            AIL_last_IO_attempt = *IO_parms;
            if (XMI_attempt_MDI_detection(mdi, IO_parms)) {
                found = 1;
                use = *IO_parms;
            }
        }
    }
    if (!found && AIL_preference[AIL_SCAN_FOR_HARDWARE] == 1) {
        for (i = 0, offset = 0; i < mdi->drvr->VHDR->num_IO_configurations;
             i++, offset += sizeof(IO_PARMS)) {
            IO_parms = (IO_PARMS *)(((mdi->drvr->VHDR->common_IO_configurations >> 16) << 4)
                                    + (mdi->drvr->VHDR->common_IO_configurations & 0xffff)
                                    + offset);
            if (IO == NULL) {
                AIL_last_IO_attempt = *IO_parms;
            }
            if (XMI_attempt_MDI_detection(mdi, IO_parms)) {
                found = 1;
                use = *IO_parms;
                break;
            }
        }
    }
    if (!found) {
        strcpy(AIL_error, D_00126eec);
        AIL_mem_free_lock(mdi);
        return NULL;
    }
    AIL_last_IO_attempt = use;
    AIL_call_driver(mdi->drvr, DRV_INIT_DEV, NULL, NULL);
    mdi->drvr->initialized = 1;
    AIL_call_driver(mdi->drvr, MDI_INIT_INS_MGR, NULL, &out);
    if (out.AX == 0) {
        strcpy(AIL_error, D_00126f0c);
        AIL_call_driver(mdi->drvr, DRV_SHUTDOWN, NULL, NULL);
        mdi->drvr->initialized = 0;
        AIL_mem_free_lock(mdi);
        return NULL;
    }
    mdi->n_sequences = AIL_preference[MDI_SEQUENCES];
    mdi->sequences = AIL_mem_alloc_lock(mdi->n_sequences * sizeof(SEQUENCE));
    if (mdi->sequences == NULL) {
        strcpy(AIL_error, D_00126f38);
        AIL_call_driver(mdi->drvr, DRV_SHUTDOWN, NULL, NULL);
        mdi->drvr->initialized = 0;
        AIL_mem_free_lock(mdi);
        return NULL;
    }
    for (i = 0; i < mdi->n_sequences; i++) {
        memcpy(mdi->sequences[i].tag, D_00126f60, 4);
        mdi->sequences[i].status = SEQ_FREE;
        mdi->sequences[i].driver = mdi;
    }
    mdi->interval_time = 1000000 / AIL_preference[MDI_SERVICE_RATE];
    mdi->event_trap = NULL;
    mdi->timbre_trap = NULL;
    mdi->message_count = 0;
    mdi->offset = 0;
    mdi->disable = 0;
    mdi->master_volume = 127;
    for (i = 0; i < NUM_CHANS; i++) {
        mdi->lock[i] = 0;
        mdi->locker[i] = NULL;
        mdi->owner[i] = NULL;
        mdi->user[i] = NULL;
        mdi->state[i] = 0;
        mdi->notes[i] = 0;
    }
    mdi->timer = AIL_register_timer((AILTIMERCB)fn_000e2ff0);
    if (mdi->timer == -1) {
        strcpy(AIL_error, D_00126f68);
        AIL_call_driver(mdi->drvr, DRV_SHUTDOWN, NULL, NULL);
        mdi->drvr->initialized = 0;
        AIL_mem_free_lock(mdi->sequences);
        AIL_mem_free_lock(mdi);
        return NULL;
    }
    AIL_set_timer_user(mdi->timer, (U32)mdi);
    mdi->drvr->destructor = (void (__cdecl *)(void *))XMI_destroy_MDI_driver;
    mdi->drvr->descriptor = mdi;
    for (i = 0; i < NUM_CHANS; i++) {
        XMI_MIDI_message(mdi, i | 0xb0, PATCH_BANK_SEL, 0);
        XMI_MIDI_message(mdi, i | 0xc0, 0, 0);
        XMI_MIDI_message(mdi, i | 0xe0, 0, 0x40);
        XMI_MIDI_message(mdi, i | 0xb0, VOICE_PROTECT, 0);
        XMI_MIDI_message(mdi, i | 0xb0, MODULATION, 0);
        XMI_MIDI_message(mdi, i | 0xb0, PART_VOLUME, AIL_preference[MDI_DEFAULT_VOLUME]);
        XMI_MIDI_message(mdi, i | 0xb0, PANPOT, 64);
        XMI_MIDI_message(mdi, i | 0xb0, EXPRESSION, 127);
        XMI_MIDI_message(mdi, i | 0xb0, SUSTAIN, 0);
        XMI_MIDI_message(mdi, i | 0xb0, REVERB, 40);
        XMI_MIDI_message(mdi, i | 0xb0, CHORUS, 0);
        XMI_MIDI_message(mdi, i | 0xb0, RPN_LSB, 0);
        XMI_MIDI_message(mdi, i | 0xb0, RPN_MSB, 0);
        XMI_MIDI_message(mdi, i | 0xb0, DATA_LSB, 0);
        XMI_MIDI_message(mdi, i | 0xb0, DATA_MSB, AIL_preference[MDI_DEFAULT_BEND_RANGE]);
        XMI_flush_buffer(mdi);
        if (!(i & 3)) {
            AIL_delay(3);
        }
    }
    AIL_set_timer_frequency(mdi->timer, AIL_preference[MDI_SERVICE_RATE]);
    AIL_start_timer(mdi->timer);
    return mdi;
}

// FUNCTION: 0x000e3eb0
HMDIDRIVER __cdecl AIL_API_install_MDI_driver_file(char *filename, IO_PARMS *IO)
{
    char fn[256];
    char old_prefix[128];
    U32 *driver_image;
    AIL_DRIVER *drvr;
    HMDIDRIVER mdi;

    old_prefix[0] = 0;
    mdi = NULL;
    driver_image = AIL_file_read(filename, FILE_READ_WITH_SIZE);
    if (driver_image == NULL) {
        if (AIL_driver_directory[0]) {
            strcpy(fn, AIL_driver_directory);
            strcat(fn, filename);
            strcpy(old_prefix, GTL_prefix);
            strcpy(GTL_prefix, AIL_driver_directory);
            strcat(GTL_prefix, old_prefix);
            driver_image = AIL_file_read(fn, FILE_READ_WITH_SIZE);
            if (driver_image != NULL) {
                goto install;
            }
        }
        strcpy(AIL_error, D_00126f80);
        goto done;
    }
install:
    drvr = AIL_install_driver((U8 *)(driver_image + 1), driver_image[0]);
    AIL_mem_free_lock(driver_image);
    if (drvr != NULL) {
        mdi = XMI_construct_MDI_driver(drvr, IO);
        if (mdi == NULL) {
            AIL_uninstall_driver(drvr);
        }
    }
done:
    if (old_prefix[0]) {
        strcpy(GTL_prefix, old_prefix);
    }
    return mdi;
}

// FUNCTION: 0x000e4000
HMDIDRIVER __cdecl AIL_API_install_MDI_driver_image(void *driver_image, U32 size, IO_PARMS *IO)
{
    AIL_DRIVER *drvr;
    HMDIDRIVER mdi;

    drvr = AIL_install_driver(driver_image, size);
    if (drvr == NULL) {
        return NULL;
    }
    mdi = XMI_construct_MDI_driver(drvr, IO);
    if (mdi == NULL) {
        AIL_uninstall_driver(drvr);
    }
    return mdi;
}

// FUNCTION: 0x000e4040
S32 __cdecl AIL_API_install_MDI_INI(HMDIDRIVER *mdi)
{
    AIL_INI INI;

    if (!AIL_read_INI(&INI, D_00126f98)) {
        strcpy(AIL_error, D_00126fa0);
        return 1;
    }
    *mdi = AIL_install_MDI_driver_file(INI.driver_name, &INI.IO);
    if (*mdi == NULL) {
        return 2;
    }
    return 0;
}

// FUNCTION: 0x000e40c0
void __cdecl AIL_API_uninstall_MDI_driver(HMDIDRIVER mdi)
{
    AIL_uninstall_driver(mdi->drvr);
}

// FUNCTION: 0x000e40e0
S32 __cdecl AIL_API_MDI_driver_type(HMDIDRIVER mdi)
{
    char *name;
    VDI_HDR *VHDR;

    VHDR = mdi->drvr->VHDR;
    if (VHDR->driver_version >= 0x112) {
        name = (char *)VHDR->dev_name;
        if (!stricmp(name, D_00126fc0)) {
            return 3;
        }
        if (!stricmp(name, D_00126fd4)) {
            return 3;
        }
    }
    name = (char *)(((mdi->DDT->GTL_suffix >> 16) << 4) + (mdi->DDT->GTL_suffix & 0xffff));
    if (name == NULL || !*name) {
        return 0;
    }
    if (!stricmp(name, D_00126ff0)) {
        if (mdi->drvr->size > 0x5000) {
            return 0;
        }
        return 1;
    }
    if (!stricmp(name, D_00126ff4)) {
        return 2;
    }
    return 0;
}

// FUNCTION: 0x000e4240
void __cdecl AIL_API_set_GTL_filename_prefix(char *prefix)
{
    S32 i;

    strcpy(GTL_prefix, prefix);
    for (i = strlen(GTL_prefix) - 1; i; i--) {
        if (GTL_prefix[i] == '\\') {
            if (i == strlen(GTL_prefix) - 1) {
                GTL_prefix[i] = 0;
            }
            return;
        }
        if (GTL_prefix[i] == '.') {
            GTL_prefix[i] = 0;
            return;
        }
    }
}

// FUNCTION: 0x000e42c0
HSEQUENCE __cdecl AIL_API_allocate_sequence_handle(HMDIDRIVER mdi)
{
    S32 i;
    HSEQUENCE S;

    AIL_lock();
    for (i = 0; i < mdi->n_sequences; i++) {
        if (mdi->sequences[i].status == SEQ_FREE) {
            break;
        }
    }
    if (i == mdi->n_sequences) {
        strcpy(AIL_error, D_00126ffc);
        AIL_unlock();
        return NULL;
    }
    S = &mdi->sequences[i];
    S->status = SEQ_DONE;
    XMI_init_sequence_state(S);
    S->EOS = NULL;
    AIL_unlock();
    return S;
}

// FUNCTION: 0x000e4360
void __cdecl AIL_API_release_sequence_handle(HSEQUENCE S)
{
    if (S == NULL) {
        return;
    }
    AIL_stop_sequence(S);
    S->status = SEQ_FREE;
}

// FUNCTION: 0x000e4380
void set_timbre_error(S32 bank, S32 patch)
{
    S32 args[2];

    args[0] = bank;
    args[1] = patch;
    AIL_sprintf(AIL_error, D_00127018, 2, args);
}

// FUNCTION: 0x000e43b0
// NONMATCHING: register allocation around the tempo/volume initialisation and
// the timbre loop
S32 __cdecl AIL_API_init_sequence(HSEQUENCE S, void *start, S32 sequence_num)
{
    VDI_CALL out;
    U8 *image;
    U8 *end;
    TIMB_chunk *timb;
    U32 i;
    U32 j;
    U32 len;
    U32 t;

    if (S == NULL) {
        return 0;
    }
    S->status = SEQ_DONE;
    image = XMI_find_sequence(start, sequence_num);
    if (image == NULL) {
        strcpy(AIL_error, D_0012704c);
        return 0;
    }
    len = XMI_swap(*(U32 *)(image + 4)) + 8;
    end = image + len;
    S->TIMB = NULL;
    S->RBRN = NULL;
    S->EVNT = NULL;
    image += 12;
    while (image < end) {
        if (!memcmp(image, D_00127064, 4)) {
            S->TIMB = image;
        }
        if (!memcmp(image, D_0012706c, 4)) {
            S->RBRN = image;
        }
        if (!memcmp(image, D_00127074, 4)) {
            S->EVNT = image;
        }
        image += XMI_swap(*(U32 *)(image + 4)) + 8;
    }
    if (S->EVNT == NULL) {
        strcpy(AIL_error, D_0012704c);
        return 0;
    }
    S->ICA = NULL;
    S->prefix_callback = NULL;
    S->trigger_callback = NULL;
    S->beat_callback = NULL;
    S->EOS = NULL;
    S->loop_count = 1;
    XMI_rewind_sequence(S);
    S->volume_period = 0;
    S->volume_accum = 0;
    S->tempo_percent = 100;
    S->tempo_target = 100;
    S->tempo_period = 0;
    S->tempo_accum = 0;
    S->tempo_error = 0;
    S->volume = S->volume_target = AIL_preference[MDI_DEFAULT_VOLUME];
    if (S->TIMB == NULL) {
        return 1;
    }
    len = XMI_swap(*(U32 *)((U8 *)S->TIMB + 4)) + 8;
    if (len > sizeof(D_0018cb6c)) {
        len = sizeof(D_0018cb6c);
    } else {
        len = XMI_swap(*(U32 *)((U8 *)S->TIMB + 4)) + 8;
    }
    memcpy(D_0018cb6c, S->TIMB, len);
    timb = (TIMB_chunk *)D_0018cb6c;
    if (S->driver->timbre_trap != NULL) {
        i = 0;
        while (i < timb->n_entries) {
            t = timb->timbre[i];
            if (!S->driver->timbre_trap(S->driver, (t & 0xff00) >> 8, t & 0xff)) {
                i++;
                continue;
            }
            for (j = i + 1; j < timb->n_entries; j++) {
                timb->timbre[j - 1] = timb->timbre[j];
            }
            timb->n_entries--;
            if (timb->len_lsb < 2) {
                timb->len_lsb -= 2;
                timb->len_msb--;
            } else {
                timb->len_lsb -= 2;
            }
        }
    }
    if (timb->n_entries == 0) {
        return 1;
    }
    if (AIL_background()) {
        strcpy(AIL_error, D_0012707c);
        return -1;
    }
    S->driver->disable++;
    XMI_flush_buffer(S->driver);
    memcpy(S->driver->DST->MIDI_data, timb, sizeof(S->driver->DST->MIDI_data));
    AIL_call_driver(S->driver->drvr, MDI_INSTALL_T_SET, NULL, &out);
    S->driver->disable--;
    if (out.AX == 0) {
        set_timbre_error((S16)out.BX >> 8, (S16)(out.BX & 0xff));
        return -1;
    }
    return 1;
}

// FUNCTION: 0x000e4740
void __cdecl AIL_API_start_sequence(HSEQUENCE S)
{
    if (S == NULL) {
        return;
    }
    if (S->status == SEQ_FREE) {
        return;
    }
    AIL_stop_sequence(S);
    XMI_rewind_sequence(S);
    S->status = SEQ_PLAYING;
}

// FUNCTION: 0x000e4770
void __cdecl AIL_API_stop_sequence(HSEQUENCE S)
{
    HMDIDRIVER mdi;
    S32 i;
    S32 ch;

    if (S == NULL) {
        return;
    }
    if (S->status != SEQ_PLAYING) {
        return;
    }
    S->status = SEQ_STOPPED;
    XMI_flush_note_queue(S);
    mdi = S->driver;
    for (i = 0; i < NUM_CHANS; i++) {
        ch = S->chan_map[i];
        if (S->shadow.sus[i] >= 64) {
            XMI_MIDI_message(mdi, ch | 0xb0, SUSTAIN, 0);
        }
        if (S->shadow.c_prot[i] >= 64) {
            mdi->lock[ch] = 0;
        }
        if (S->shadow.c_v_prot[i] >= 64) {
            XMI_MIDI_message(mdi, ch | 0xb0, VOICE_PROTECT, 0);
        }
        if (S->shadow.c_lock[i] >= 64) {
            AIL_release_channel(mdi, ch + 1);
        }
    }
}

// FUNCTION: 0x000e4830
void __cdecl AIL_API_resume_sequence(HSEQUENCE S)
{
    HMDIDRIVER mdi;
    S32 i;
    S32 ch;

    if (S == NULL) {
        return;
    }
    if (S->status != SEQ_STOPPED) {
        return;
    }
    mdi = S->driver;
    for (i = 0; i < NUM_CHANS; i++) {
        if (S->shadow.c_lock[i] >= 64) {
            ch = AIL_lock_channel(mdi) - 1;
            if (ch == -1) {
                ch = i;
            }
            S->chan_map[i] = ch;
        }
    }
    for (i = 0; i < NUM_CHANS; i++) {
        XMI_refresh_channel(S, i);
    }
    S->status = SEQ_PLAYING;
}

// FUNCTION: 0x000e48b0
void __cdecl AIL_API_end_sequence(HSEQUENCE S)
{
    if (S == NULL) {
        return;
    }
    if (S->status == SEQ_FREE || S->status == SEQ_DONE) {
        return;
    }
    AIL_stop_sequence(S);
    S->status = SEQ_DONE;
    if (S->EOS != NULL) {
        S->EOS(S);
    }
}

// FUNCTION: 0x000e48f0
void __cdecl AIL_API_set_sequence_loop_count(HSEQUENCE S, S32 loop_count)
{
    if (S == NULL) {
        return;
    }
    S->loop_count = loop_count;
}

// FUNCTION: 0x000e4900
// NONMATCHING: the difference is computed in eax and copied to ecx
void __cdecl AIL_API_set_sequence_tempo(HSEQUENCE S, S32 tempo, S32 milliseconds)
{
    S32 diff;

    if (S == NULL) {
        return;
    }
    S->driver->disable++;
    S->tempo_target = tempo;
    if (S->tempo_percent != S->tempo_target) {
        if (milliseconds == 0) {
            S->tempo_percent = S->tempo_target;
        } else {
            milliseconds *= 1000;
            diff = S->tempo_percent - S->tempo_target;
            if (diff < 0) {
                diff = -diff;
            }
            S->tempo_accum = 0;
            S->tempo_period = milliseconds / diff;
        }
    }
    S->driver->disable--;
}

// FUNCTION: 0x000e4970
// NONMATCHING: the difference is computed in eax and copied to ecx
void __cdecl AIL_API_set_sequence_volume(HSEQUENCE S, S32 volume, S32 milliseconds)
{
    S32 diff;

    if (S == NULL) {
        return;
    }
    S->driver->disable++;
    S->volume_target = volume;
    if (S->volume == S->volume_target) {
        S->driver->disable--;
        return;
    }
    if (milliseconds == 0) {
        S->volume = S->volume_target;
    } else {
        milliseconds *= 1000;
        diff = S->volume - S->volume_target;
        if (diff < 0) {
            diff = -diff;
        }
        S->volume_accum = 0;
        S->volume_period = milliseconds / diff;
    }
    XMI_SERVE(S);
    S->driver->disable--;
}

// FUNCTION: 0x000e4a00
U32 __cdecl AIL_API_sequence_status(HSEQUENCE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->status;
}

// FUNCTION: 0x000e4a10
S32 __cdecl AIL_API_sequence_loop_count(HSEQUENCE S)
{
    if (S == NULL) {
        return -1;
    }
    return S->loop_count;
}

// FUNCTION: 0x000e4a30
S32 __cdecl AIL_API_sequence_tempo(HSEQUENCE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->tempo_percent;
}

// FUNCTION: 0x000e4a40
S32 __cdecl AIL_API_sequence_volume(HSEQUENCE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->volume;
}

// FUNCTION: 0x000e4a50
void __cdecl AIL_API_set_XMIDI_master_volume(HMDIDRIVER mdi, S32 master_volume)
{
    S32 i;
    HSEQUENCE S;

    if (mdi->master_volume == master_volume) {
        return;
    }
    mdi->master_volume = master_volume;
    mdi->disable++;
    for (i = mdi->n_sequences, S = mdi->sequences; i; i--, S++) {
        if (S->status == SEQ_PLAYING) {
            XMI_SERVE(S);
        }
    }
    mdi->disable--;
}

// FUNCTION: 0x000e4aa0
S32 __cdecl AIL_API_XMIDI_master_volume(HMDIDRIVER mdi)
{
    return mdi->master_volume;
}

// FUNCTION: 0x000e4ab0
S32 __cdecl AIL_API_timbre_status(HMDIDRIVER mdi, S32 bank, S32 patch)
{
    VDI_CALL in;

    in.CX = (bank << 8) | patch;
    return AIL_call_driver(mdi->drvr, MDI_GET_T_STATUS, &in, NULL);
}

// FUNCTION: 0x000e4af0
S32 __cdecl AIL_API_install_timbre(HMDIDRIVER mdi, S32 bank, S32 patch)
{
    VDI_CALL out;

    if (mdi->timbre_trap != NULL && mdi->timbre_trap(mdi, bank, patch)) {
        return 1;
    }
    D_00151634.patch = patch;
    D_00151634.bank = bank;
    mdi->disable++;
    XMI_flush_buffer(mdi);
    memcpy(mdi->DST->MIDI_data, &D_00151634, sizeof(D_00151634));
    AIL_call_driver(mdi->drvr, MDI_INSTALL_T_SET, NULL, &out);
    mdi->disable--;
    if (out.AX == 0) {
        set_timbre_error((S16)out.BX >> 8, (S16)(out.BX & 0xff));
    }
    return (S16)out.AX;
}

// FUNCTION: 0x000e4bb0
void __cdecl AIL_API_protect_timbre(HMDIDRIVER mdi, S32 bank, S32 patch)
{
    VDI_CALL in;

    in.CX = (bank << 8) | patch;
    in.DX = 1;
    AIL_call_driver(mdi->drvr, MDI_PROT_UNPROT_T, &in, NULL);
}

// FUNCTION: 0x000e4bf0
void __cdecl AIL_API_unprotect_timbre(HMDIDRIVER mdi, S32 bank, S32 patch)
{
    VDI_CALL in;

    in.CX = (bank << 8) | patch;
    in.DX = 0;
    AIL_call_driver(mdi->drvr, MDI_PROT_UNPROT_T, &in, NULL);
}

// FUNCTION: 0x000e4c30
S32 __cdecl AIL_API_active_sequence_count(HMDIDRIVER mdi)
{
    S32 i;
    S32 n;

    n = 0;
    for (i = 0; i < mdi->n_sequences; i++) {
        if (mdi->sequences[i].status == SEQ_PLAYING) {
            n++;
        }
    }
    return n;
}

// FUNCTION: 0x000e4c60
S32 __cdecl AIL_API_controller_value(HSEQUENCE S, S32 channel, S32 controller_num)
{
    if (S == NULL) {
        return -1;
    }
    return XMI_read_log(&S->shadow, (channel - 1) | 0xb0, controller_num);
}

// FUNCTION: 0x000e4c90
S32 __cdecl AIL_API_channel_notes(HSEQUENCE S, S32 channel)
{
    S32 i;
    S32 n;

    if (S == NULL) {
        return 0;
    }
    n = 0;
    for (i = 0; i < NUM_CHANS; i++) {
        if (S->note_chan[i] == channel - 1) {
            n++;
        }
    }
    return n;
}

// FUNCTION: 0x000e4cd0
void __cdecl AIL_API_sequence_position(HSEQUENCE S, S32 *beat, S32 *measure)
{
    S32 b;
    S32 m;
    S32 f;
    S32 i;

    if (S == NULL) {
        return;
    }
    S->driver->disable++;
    b = S->beat_count;
    m = S->measure_count;
    f = S->beat_fraction;
    for (i = 0; i < AIL_preference[MDI_QUANT_ADVANCE]; i++) {
        f += S->time_fraction;
        if (f >= S->time_per_beat) {
            f -= S->time_per_beat;
            if (++b >= S->time_numerator) {
                m++;
                b = 0;
            }
        }
    }
    if (m < 0) {
        m = 0;
    }
    if (measure != NULL) {
        *measure = m;
    }
    if (beat != NULL) {
        *beat = b;
    }
    S->driver->disable--;
}

// FUNCTION: 0x000e4d50
// NONMATCHING: the target clears the loop index with mov eax,edx / xor eax,edx
// and walks the RBRN entries through a pointer
void __cdecl AIL_API_branch_index(HSEQUENCE S, U32 marker)
{
    RBRN_block *RBRN;
    S32 i;
    S32 n;

    if (S == NULL) {
        return;
    }
    RBRN = S->RBRN;
    if (RBRN == NULL) {
        return;
    }
    n = RBRN->n_entries;
    for (i = 0; i < n; i++) {
        if (RBRN->entry[i].offset == marker) {
            break;
        }
    }
    if (i == n) {
        return;
    }
    S->EVNT_ptr = (U8 *)S->EVNT + 8 + RBRN->entry[i].pos;
    S->interval_count = 0;
    if (!AIL_preference[MDI_ALLOW_LOOP_BRANCHING]) {
        for (i = 0; i < FOR_NEST; i++) {
            S->FOR_loop_count[i] = -1;
        }
    }
}

// FUNCTION: 0x000e4de0
AILPREFIXCB __cdecl AIL_API_register_prefix_callback(HSEQUENCE S, AILPREFIXCB callback)
{
    AILPREFIXCB old;

    if (S == NULL) {
        return NULL;
    }
    old = S->prefix_callback;
    S->prefix_callback = callback;
    return old;
}

// FUNCTION: 0x000e4e00
AILTRIGGERCB __cdecl AIL_API_register_trigger_callback(HSEQUENCE S, AILTRIGGERCB callback)
{
    AILTRIGGERCB old;

    if (S == NULL) {
        return NULL;
    }
    old = S->trigger_callback;
    S->trigger_callback = callback;
    return old;
}

// FUNCTION: 0x000e4e20
AILSEQUENCECB __cdecl AIL_API_register_sequence_callback(HSEQUENCE S, AILSEQUENCECB callback)
{
    AILSEQUENCECB old;

    if (S == NULL) {
        return NULL;
    }
    old = S->EOS;
    S->EOS = callback;
    return old;
}

// FUNCTION: 0x000e4e40
AILBEATCB __cdecl AIL_API_register_beat_callback(HSEQUENCE S, AILBEATCB callback)
{
    AILBEATCB old;

    if (S == NULL) {
        return NULL;
    }
    old = S->beat_callback;
    S->beat_callback = callback;
    return old;
}

// FUNCTION: 0x000e4e60
AILEVENTCB __cdecl AIL_API_register_event_callback(HMDIDRIVER mdi, AILEVENTCB callback)
{
    AILEVENTCB old;

    old = mdi->event_trap;
    mdi->event_trap = callback;
    return old;
}

// FUNCTION: 0x000e4e80
AILTIMBRECB __cdecl AIL_API_register_timbre_callback(HMDIDRIVER mdi, AILTIMBRECB callback)
{
    AILTIMBRECB old;

    old = mdi->timbre_trap;
    mdi->timbre_trap = callback;
    return old;
}

// FUNCTION: 0x000e4ea0
void __cdecl AIL_API_set_sequence_user_data(HSEQUENCE S, U32 index, S32 value)
{
    if (S == NULL) {
        return;
    }
    S->user_data[index] = value;
}

// FUNCTION: 0x000e4ec0
S32 __cdecl AIL_API_sequence_user_data(HSEQUENCE S, U32 index)
{
    if (S == NULL) {
        return 0;
    }
    return S->user_data[index];
}

// FUNCTION: 0x000e4ee0
void __cdecl AIL_API_register_ICA_array(HSEQUENCE S, U8 *array)
{
    if (S == NULL) {
        return;
    }
    S->ICA = array;
}

// FUNCTION: 0x000e4ef0
// NONMATCHING: eax/ebx swapped in the channel search loops
S32 __cdecl AIL_API_lock_channel(HMDIDRIVER mdi)
{
    S32 i;
    S32 j;
    S32 best;
    S32 min_notes;
    HSEQUENCE S;

    mdi->disable++;
    best = -1;
    min_notes = 0x7fffffff;
    for (i = MAX_LOCK_CHAN; i >= MIN_LOCK_CHAN; i--) {
        if (i == PERCUSS_CHAN) {
            continue;
        }
        if (mdi->lock[i] == 1 || mdi->lock[i] == 2) {
            continue;
        }
        if (mdi->notes[i] < min_notes) {
            min_notes = mdi->notes[i];
            best = i;
        }
    }
    if (best == -1) {
        for (i = MAX_LOCK_CHAN; i >= MIN_LOCK_CHAN; i--) {
            if (i == PERCUSS_CHAN) {
                continue;
            }
            if (mdi->lock[i] == 1) {
                continue;
            }
            if (mdi->notes[i] < min_notes) {
                min_notes = mdi->notes[i];
                best = i;
            }
        }
    }
    if (best == -1) {
        mdi->disable--;
        return 0;
    }
    XMI_MIDI_message(mdi, best | 0xb0, SUSTAIN, 0);
    for (j = mdi->n_sequences, S = mdi->sequences; j; j--, S++) {
        if (S->status == SEQ_FREE) {
            continue;
        }
        for (i = 0; i < MAX_NOTES; i++) {
            if (S->note_chan[i] == -1) {
                continue;
            }
            if (S->chan_map[S->note_chan[i]] != best) {
                continue;
            }
            XMI_send_channel_voice_message(S, S->note_chan[i] | 0x80, S->note_num[i], 0, 0);
            S->note_chan[i] = -1;
        }
    }
    mdi->locker[best] = NULL;
    mdi->state[best] = mdi->lock[best];
    mdi->lock[best] = 1;
    mdi->owner[best] = mdi->user[best];
    mdi->disable--;
    return best + 1;
}

// FUNCTION: 0x000e5080
// NONMATCHING: the target loads the owner's status into edi before comparing
void __cdecl AIL_API_release_channel(HMDIDRIVER mdi, S32 channel)
{
    S32 i;
    S32 j;
    HSEQUENCE S;

    channel--;
    if (mdi->lock[channel] != 1) {
        return;
    }
    mdi->disable++;
    mdi->lock[channel] = mdi->state[channel];
    mdi->user[channel] = mdi->owner[channel];
    XMI_MIDI_message(mdi, channel | 0xb0, SUSTAIN, 0);
    for (j = mdi->n_sequences, S = mdi->sequences; j; j--, S++) {
        if (S->status == SEQ_FREE) {
            continue;
        }
        for (i = 0; i < MAX_NOTES; i++) {
            if (S->note_chan[i] == -1) {
                continue;
            }
            if (S->chan_map[S->note_chan[i]] != channel) {
                continue;
            }
            XMI_send_channel_voice_message(S, S->note_chan[i] | 0x80, S->note_num[i], 0, 0);
            S->note_chan[i] = -1;
        }
    }
    S = mdi->owner[channel];
    if (S != NULL && S->status != SEQ_FREE) {
        XMI_refresh_channel(S, channel);
    }
    mdi->disable--;
}

// FUNCTION: 0x000e51a0
void __cdecl AIL_API_map_sequence_channel(HSEQUENCE S, S32 seq_channel, S32 new_channel)
{
    if (S == NULL) {
        return;
    }
    S->chan_map[seq_channel - 1] = new_channel - 1;
    if (S->driver->lock[new_channel - 1] == 1 && S->driver->locker[new_channel - 1] != S) {
        S->driver->locker[new_channel - 1] = S;
    }
}

// FUNCTION: 0x000e51e0
S32 __cdecl AIL_API_true_sequence_channel(HSEQUENCE S, S32 seq_channel)
{
    if (S == NULL) {
        return 0;
    }
    return S->chan_map[seq_channel - 1] + 1;
}

// FUNCTION: 0x000e5200
// NONMATCHING: register allocation of the parameters
void __cdecl AIL_API_send_channel_voice_message(HMDIDRIVER mdi, HSEQUENCE S, S32 status,
                                                S32 data_1, S32 data_2)
{
    if (mdi == NULL) {
        if (S == NULL) {
            return;
        }
        mdi = S->driver;
    }
    mdi->disable++;
    if (S == NULL) {
        if ((status & 0xf0) == 0xb0 && data_1 == PART_VOLUME) {
            data_2 = (data_2 * mdi->master_volume) / 127;
            if (data_2 > 127) {
                data_2 = 127;
            }
            if (data_2 < 0) {
                data_2 = 0;
            }
        }
        if (mdi->event_trap == NULL || !mdi->event_trap(mdi, NULL, status, data_1, data_2)) {
            XMI_MIDI_message(mdi, status, data_1, data_2);
        }
    } else {
        XMI_send_channel_voice_message(S, status, data_1, data_2, 0);
    }
    XMI_flush_buffer(mdi);
    mdi->disable--;
}

// FUNCTION: 0x000e52d0
void __cdecl AIL_API_send_sysex_message(HMDIDRIVER mdi, void *buffer)
{
    U8 *ptr;
    S32 len;

    mdi->disable++;
    ptr = (U8 *)buffer + 1;
    len = XMI_read_VLN(&ptr);
    XMI_sysex_message(mdi, buffer, len + (ptr - (U8 *)buffer));
    mdi->disable--;
}
