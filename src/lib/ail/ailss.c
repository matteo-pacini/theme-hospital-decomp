#include "ail.h"

#define REALPTR(x) ((void *)(U32)((((U32)(x)) >> 16 << 4) + ((x) & 0xffff)))

/* abs() expanded inline; operand and result fixed to eax. */
S32 AIL_abs(S32 x);
#pragma aux AIL_abs = \
    "test eax,eax" \
    "jge L1" \
    "neg eax" \
    "L1:" \
    parm [eax] value [eax];

extern U8 pan_graph[128];

extern char D_00126c3c[];
extern char D_00126c68[];
extern char D_00126c90[];
extern char D_00126c98[];
extern char D_00126cb0[];
extern char D_00126cd4[];
extern char D_00126cf4[];
extern char D_00126d18[];
extern char D_00126d40[];
extern char D_00126d48[];
extern char D_00126d60[];
extern char D_00126d78[];
extern char D_00126d80[];
extern char D_00126da0[];

// Module state. The BSS layout follows symbol handles and name hashes, not
// declaration order (docs/MATCHING.md, "Static data layout"): the externs
// above put VDI in the same 25-symbol block as S..cnt, and the names hash in
// the original's order. Adding or removing a declaration before this point
// can split them again.
static S32 entry = 0;
static S32 locked = 0;
static VDI_CALL VDI;
static HSAMPLE S;
static S32 n;
static HDIGDRIVER dig;
static S32 current;
static U32 flags;
static S32 cnt;

#define DIG_BUFFER_SERVICE 0x0010

/* Timer callback: __pascal (callee pops), address-named. */
void fn_000dff00(U32 user);
#pragma aux (__pascal) fn_000dff00 "*_";
/* Driver destructor: __cdecl, address-named. */
void fn_000e0660(HDIGDRIVER dig);
#pragma aux (__cdecl) fn_000e0660 "*_";
void AILSS_end(void);

// FUNCTION: 0x000df970
void AILSS_start(void)
{
    if (!locked) {
        AIL_vmm_lock_range(AILSS_start, AILSS_end);
        AIL_vmm_lock(pan_graph, sizeof(pan_graph));
        AIL_vmm_lock(&entry, sizeof(entry));
        AIL_vmm_lock(&dig, sizeof(dig));
        AIL_vmm_lock(&VDI, sizeof(VDI));
        AIL_vmm_lock(&flags, sizeof(flags));
        AIL_vmm_lock(&cnt, sizeof(cnt));
        AIL_vmm_lock(&n, sizeof(n));
        AIL_vmm_lock(&current, sizeof(current));
        AIL_vmm_lock(&S, sizeof(S));
        AILSSA_VMM_lock();
        locked = 1;
    }
}

// FUNCTION: 0x000dfa30
void __cdecl SS_start_DIG_driver_playback(HDIGDRIVER dig)
{
    VDI_CALL VDI;

    if (!dig->playing) {
        VDI.DX = dig->hw_format;
        VDI.CX = dig->DMA_rate;
        AIL_call_driver(dig->drvr, DIG_START_P_CMD, &VDI, NULL);
        dig->playing = 1;
    }
}

// FUNCTION: 0x000dfa80
void __cdecl SS_stop_DIG_driver_playback(HDIGDRIVER dig)
{
    if (dig->playing) {
        AIL_call_driver(dig->drvr, DIG_STOP_P_REQ, NULL, NULL);
        dig->playing = 0;
    }
}

// FUNCTION: 0x000dfab0
void SS_stream_to_buffer(HSAMPLE S)
{
    S32 dest_offset;
    S32 amount;
    S32 next;

    if (S->status != SMP_PLAYING) {
        return;
    }
    dest_offset = 0;
    while (dest_offset < S->driver->build_size) {
        amount = S->len[S->current_buffer] - S->pos[S->current_buffer];
        if (S->loop_count != 1 && S->loop_end != -1
            && S->loop_end < S->len[S->current_buffer]) {
            amount = S->loop_end - S->pos[S->current_buffer];
        }
        if (amount <= 0) {
            if (S->EOB != NULL) {
                S->EOB(S);
            }
            if (S->loop_count != 1) {
                S->pos[S->current_buffer] = S->loop_start;
                if (S->loop_count != 0) {
                    S->loop_count--;
                }
                continue;
            }
            if (S->service_type == 2) {
                next = S->current_buffer ^ 1;
                if (!S->done[next]) {
                    if (S->len[next] == 0) {
                        S->starved = 1;
                        return;
                    }
                    if (S->pos[next] != 0) {
                        S->starved = 1;
                        return;
                    }
                    S->current_buffer = next;
                    continue;
                }
            }
            S->status = SMP_DONE;
            S->starved = 1;
            if (S->EOS != NULL) {
                S->EOS(S);
            }
            return;
        }
        if (S->SOB != NULL) {
            S->SOB(S);
        }
        dest_offset = AILSSA_merge(S, amount, dest_offset);
    }
}

// FUNCTION: 0x000dfbe0
// NONMATCHING: register allocation: pan and the loop counters get esi and
// hw_format edi where the target has them the other way round
void __cdecl SS_build_amplitude_tables(HSAMPLE S)
{
    S32 val;
    S32 vol;
    S32 pan;
    S32 *right;
    S32 right_pan;
    S32 *left;
    S32 i;
    S32 left_pan;

    if (S->volume > 127) {
        S->volume = 127;
    } else if (S->volume < 0) {
        S->volume = 0;
    }
    if (S->pan > 127) {
        S->pan = 127;
    } else if (S->pan < 0) {
        S->pan = 0;
    }
    vol = S->volume;
    vol = (vol * S->driver->master_volume) / 127;
    pan = S->pan;
    if (vol < 0) {
        vol = 0;
    }
    if (vol > 127) {
        vol = 127;
    }
    if (S->format == DIG_F_MONO_16 || S->format == DIG_F_STEREO_16) {
        S->vol_scale[0][0] = (pan_graph[127 - pan] * vol) / 127;
        S->vol_scale[1][0] = (pan_graph[pan] * vol) / 127;
        return;
    }
    if (vol) {
        vol++;
    }
    if (S->driver->hw_format == DIG_F_STEREO_8
        || S->driver->hw_format == DIG_F_STEREO_16
        || ((S->driver->hw_format == DIG_F_MONO_8
             || S->driver->hw_format == DIG_F_MONO_16)
            && (S->format == DIG_F_STEREO_8 || S->format == DIG_F_STEREO_16))) {
        left = S->vol_scale[0];
        right = S->vol_scale[1];
        left_pan = pan_graph[127 - pan];
        right_pan = pan_graph[pan];
        if (S->flags & DIG_PCM_SIGN) {
            for (i = 0; i < 32768; i += 256) {
                val = (i * vol) >> 7;
                *left++ = (val * left_pan) >> 7;
                *right++ = (val * right_pan) >> 7;
            }
            for (i = -32768; i < 0; i += 256) {
                val = (i * vol) >> 7;
                *left++ = (val * left_pan) >> 7;
                *right++ = (val * right_pan) >> 7;
            }
        } else {
            for (i = -32768; i < 32768; i += 256) {
                val = (i * vol) >> 7;
                *left++ = (val * left_pan) >> 7;
                *right++ = (val * right_pan) >> 7;
            }
        }
    } else {
        left = S->vol_scale[0];
        if (S->flags & DIG_PCM_SIGN) {
            for (i = 0; i < 32768; i += 256) {
                *left++ = (i * vol) >> 7;
            }
            for (i = -32768; i < 0; i += 256) {
                *left++ = (i * vol) >> 7;
            }
        } else {
            for (i = -32768; i < 32768; i += 256) {
                *left++ = (i * vol) >> 7;
            }
        }
    }
}

// FUNCTION: 0xdff00
void fn_000dff00(U32 user)
{
    if (entry) {
        return;
    }
    entry = 1;
    dig = (HDIGDRIVER)user;
    if (AIL_preference[DIG_SS_LOCK]) {
        flags = AIL_disable_interrupts();
    }
    if (!dig->playing) {
        if (AIL_preference[DIG_SS_LOCK]) {
            AIL_restore_interrupts(flags);
        }
        entry = 0;
        return;
    }
    current = *dig->buffer_flag;
    if (current == -1 || current == dig->last_buffer) {
        if (AIL_preference[DIG_SS_LOCK]) {
            AIL_restore_interrupts(flags);
        }
        entry = 0;
        return;
    }
    dig->last_buffer = current;
    AILSSA_flush_buffer(dig);
    cnt = 0;
    n = dig->n_samples;
    S = dig->samples;
    for (; n; n--, S++) {
        if (S->status == SMP_PLAYING) {
            cnt++;
            SS_stream_to_buffer(S);
        }
    }
    dig->n_active_samples = cnt;
    AILSSA_DMA_copy(dig, current ^ 1);
    if (dig->hw_mode_flags & DIG_BUFFER_SERVICE) {
        VDI.CX = 0;
        VDI.DX = current ^ 1;
        AIL_call_driver(dig->drvr, DIG_VSE, &VDI, NULL);
    }
    if (dig->n_active_samples) {
        dig->quiet = 0;
    } else if (dig->quiet++ == 2) {
        SS_stop_DIG_driver_playback(dig);
    }
    if (AIL_preference[DIG_SS_LOCK]) {
        AIL_restore_interrupts(flags);
    }
    entry = 0;
}

// FUNCTION: 0x000e0120
S32 __cdecl SS_configure_buffers(HDIGDRIVER dig)
{
    S32 i, min, max, hb_samples;
    S32 n, match, delta;
    S32 pref[4];
    U32 linear;
    S32 DMA_status;

    DMA_status = dig->playing;

    if (DMA_status) {
        for (i = 0; i < dig->n_samples; i++) {
            dig->samples[i].system_data[7] = dig->samples[i].status;
            if (dig->samples[i].status == SMP_PLAYING) {
                dig->samples[i].status = SMP_STOPPED;
            }
        }
        SS_stop_DIG_driver_playback(dig);
        AILSSA_flush_buffer(dig);
        AILSSA_DMA_copy(dig, 0);
        AILSSA_DMA_copy(dig, 1);
    }

    switch ((AIL_preference[DIG_USE_STEREO] << 1) | (AIL_preference[DIG_USE_16_BITS])) {
    case DIG_F_MONO_8:
        pref[0] = DIG_F_MONO_8;
        pref[1] = DIG_F_MONO_16;
        pref[2] = DIG_F_STEREO_8;
        pref[3] = DIG_F_STEREO_16;
        break;
    case DIG_F_MONO_16:
        pref[0] = DIG_F_MONO_16;
        pref[1] = DIG_F_MONO_8;
        pref[2] = DIG_F_STEREO_16;
        pref[3] = DIG_F_STEREO_8;
        break;
    case DIG_F_STEREO_8:
        pref[0] = DIG_F_STEREO_8;
        pref[1] = DIG_F_STEREO_16;
        pref[2] = DIG_F_MONO_8;
        pref[3] = DIG_F_MONO_16;
        break;
    case DIG_F_STEREO_16:
        pref[0] = DIG_F_STEREO_16;
        pref[1] = DIG_F_STEREO_8;
        pref[2] = DIG_F_MONO_16;
        pref[3] = DIG_F_MONO_8;
        break;
    }

    for (i = 0; i < 4; i++) {
        if (dig->DDT->format_supported[pref[i]]) {
            dig->hw_format = pref[i];
            break;
        }
    }

    dig->hw_mode_flags = dig->DDT->format_data[dig->hw_format].flags;

    switch (AIL_preference[DIG_HARDWARE_SAMPLE_RATE]) {
    case 0:
        dig->DMA_rate = dig->DDT->format_data[dig->hw_format].minimum_physical_sample_rate;
        break;
    case 1:
        dig->DMA_rate = dig->DDT->format_data[dig->hw_format].nominal_physical_sample_rate;
        break;
    case 2:
        dig->DMA_rate = dig->DDT->format_data[dig->hw_format].maximum_physical_sample_rate;
        break;
    default:
        n = AIL_preference[DIG_HARDWARE_SAMPLE_RATE];
        pref[0] = dig->DDT->format_data[dig->hw_format].minimum_physical_sample_rate;
        pref[1] = dig->DDT->format_data[dig->hw_format].nominal_physical_sample_rate;
        pref[2] = dig->DDT->format_data[dig->hw_format].maximum_physical_sample_rate;
        delta = 0x7fffffff;
        for (i = 0; i < 3; i++) {
            if (AIL_abs(n - pref[i]) <= delta) {
                delta = AIL_abs(n - pref[i]);
                match = i;
            }
        }
        dig->DMA_rate = pref[match];
        break;
    }

    switch (dig->hw_format) {
    case DIG_F_MONO_8:
        dig->channels_per_sample = 1;
        dig->bytes_per_channel = 1;
        break;
    case DIG_F_STEREO_8:
        dig->channels_per_sample = 2;
        dig->bytes_per_channel = 1;
        break;
    case DIG_F_MONO_16:
        dig->channels_per_sample = 1;
        dig->bytes_per_channel = 2;
        break;
    case DIG_F_STEREO_16:
        dig->channels_per_sample = 2;
        dig->bytes_per_channel = 2;
        break;
    }

    hb_samples = (dig->DMA_rate * AIL_preference[DIG_LATENCY]) / 1000;

    dig->half_buffer_size = hb_samples * dig->bytes_per_channel * dig->channels_per_sample;

    min = dig->DDT->format_data[dig->hw_format].minimum_DMA_half_buffer_size;
    max = dig->DDT->format_data[dig->hw_format].maximum_DMA_half_buffer_size;

    if (AIL_preference[AIL_ALLOW_VDM_EXECUTION]) {
        if (max > 2048) {
            max = 2048;
        }
        if (min > 2048) {
            strcpy(AIL_error, D_00126c3c);
            return 0;
        }
    }

    delta = 0x7fffffff;

    for (n = 8; n <= (AIL_preference[DIG_DMA_RESERVE] / 2); n <<= 1) {
        if (AIL_abs(n - dig->half_buffer_size) <= delta) {
            delta = AIL_abs(n - dig->half_buffer_size);
            match = n;
        }
    }

    dig->half_buffer_size = match;

    if (dig->half_buffer_size < min) {
        dig->half_buffer_size = min;
    }

    if (dig->half_buffer_size > max) {
        dig->half_buffer_size = max;
    }

    linear = (U32)dig->DMA_seg;

    if (AIL_preference[AIL_ALLOW_VDM_EXECUTION]) {
        linear >>= 12;
        linear += 4095;
        linear &= 0xff000;
        linear <<= 12;
    }

    dig->DST->DMA_buffer_A = linear;
    dig->DST->DMA_buffer_B = linear + dig->half_buffer_size;

    dig->DMA[0] = (void *)(((dig->DST->DMA_buffer_A >> 16) << 4)
                           + (dig->DST->DMA_buffer_A & 0xffff));
    dig->DMA[1] = (void *)(((dig->DST->DMA_buffer_B >> 16) << 4)
                           + (dig->DST->DMA_buffer_B & 0xffff));

    if (DMA_status) {
        for (i = 0; i < dig->n_samples; i++) {
            dig->samples[i].status = dig->samples[i].system_data[7];
        }
        AILSSA_flush_buffer(dig);
        AILSSA_DMA_copy(dig, 0);
        AILSSA_DMA_copy(dig, 1);
        SS_start_DIG_driver_playback(dig);
    }

    return 1;
}

// FUNCTION: 0x000e0560
S32 __cdecl SS_attempt_DIG_detection(HDIGDRIVER dig, IO_PARMS *IO)
{
    IO_PARMS use;
    IO_PARMS *def;
    VDI_CALL out;
    S32 i;

    use = *IO;
    if (dig->drvr->VHDR->num_IO_configurations > 0) {
        def = (IO_PARMS *)(((dig->drvr->VHDR->common_IO_configurations >> 16) << 4)
                           + (dig->drvr->VHDR->common_IO_configurations & 0xffff));
        if ((S32)use.IO < 0) {
            use.IO = def->IO;
        }
        if ((S32)use.IRQ < 0) {
            use.IRQ = def->IRQ;
        }
        if ((S32)use.DMA_8_bit < 0) {
            use.DMA_8_bit = def->DMA_8_bit;
        }
        if ((S32)use.DMA_16_bit < 0) {
            use.DMA_16_bit = def->DMA_16_bit;
        }
        for (i = 0; i < 4; i++) {
            if (use.IO_reserved[i] < 0) {
                use.IO_reserved[i] = def->IO_reserved[i];
            }
        }
    }
    dig->drvr->VHDR->IO = use;
    if (AIL_preference[DIG_ALLOW_16_BIT_DMA] == 0) {
        out.DX = 1;
        return AIL_call_driver(dig->drvr, DRV_VERIFY_IO, &out, NULL);
    } else {
        out.DX = 3;
        return AIL_call_driver(dig->drvr, DRV_VERIFY_IO, &out, NULL);
    }
}

// FUNCTION: 0xe0660
void fn_000e0660(HDIGDRIVER dig)
{
    if (dig->playing) {
        SS_stop_DIG_driver_playback(dig);
        AILSSA_flush_buffer(dig);
        AILSSA_DMA_copy(dig, 0);
        AILSSA_DMA_copy(dig, 1);
        AIL_delay(10);
    }
    AIL_release_timer_handle(dig->timer);
    AIL_mem_free_DOS(dig->DMA_buf, dig->DMA_seg, dig->DMA_sel);
    AIL_mem_free_lock(dig->samples);
    AIL_mem_free_lock(dig->build_buffer);
    AIL_mem_free_lock(dig);
}

// FUNCTION: 0x000e06f0
HDIGDRIVER __cdecl SS_construct_DIG_driver(AIL_DRIVER *drvr, IO_PARMS *IO)
{
    IO_PARMS use;
    S32 i;
    S32 detected;
    VDI_CALL out;
    HDIGDRIVER dig;
    S32 DMA_bytes;
    U32 start_pg, end_pg;
    void *buf;
    U32 seg;
    U32 sel;
    U32 linear;

    AILSS_start();
    dig = AIL_mem_alloc_lock(sizeof(struct _DIG_DRIVER));
    if (dig == NULL) {
        strcpy(AIL_error, D_00126c68);
        return NULL;
    }
    memcpy(dig->tag, D_00126c90, 4);
    dig->drvr = drvr;
    if (dig->drvr->type != DIG_DRIVER) {
        strcpy(AIL_error, D_00126c98);
        AIL_mem_free_lock(dig);
        return NULL;
    }
    AIL_call_driver(dig->drvr, DRV_GET_INFO, NULL, &out);
    dig->DDT = (DIG_DDT *)(((U32)(U16)out.DX << 4) + (U32)(U16)out.AX);
    dig->DST = (DIG_DST *)(((U32)(U16)out.CX << 4) + (U32)(U16)out.BX);
    dig->buffer_flag = &dig->DST->active_buffer;
    dig->last_buffer = -1;
    dig->master_volume = 127;
    dig->playing = 0;
    dig->quiet = 0;
    dig->n_active_samples = 0;
    memset(&AIL_last_IO_attempt, -1, sizeof(AIL_last_IO_attempt));
    detected = 0;
    if (IO != NULL) {
        AIL_last_IO_attempt = *IO;
        if (SS_attempt_DIG_detection(dig, IO)) {
            detected = 1;
            use = *IO;
        }
    }
    if (!detected) {
        IO = AIL_get_IO_environment(dig->drvr);
        if (IO != NULL) {
            AIL_last_IO_attempt = *IO;
            if (SS_attempt_DIG_detection(dig, IO)) {
                detected = 1;
                use = *IO;
            }
        }
    }
    if ((!detected) && (AIL_preference[AIL_SCAN_FOR_HARDWARE] == 1)) {
        for (i = 0; i < dig->drvr->VHDR->num_IO_configurations; i++) {
            IO = &((IO_PARMS *)REALPTR(dig->drvr->VHDR->common_IO_configurations))[i];
            if (i == 0) {
                AIL_last_IO_attempt = *IO;
            }
            if (SS_attempt_DIG_detection(dig, IO)) {
                detected = 1;
                use = *IO;
                break;
            }
        }
    }
    if (detected) {
        AIL_last_IO_attempt = use;
    } else {
        strcpy(AIL_error, D_00126cb0);
        AIL_mem_free_lock(dig);
        return NULL;
    }
    if (AIL_preference[AIL_ALLOW_VDM_EXECUTION]) {
        DMA_bytes = 2 * 4096;
    } else {
        DMA_bytes = AIL_preference[DIG_DMA_RESERVE];
    }
    dig->DMA_buf = NULL;
    do {
        if (!AIL_mem_alloc_DOS((DMA_bytes + 15) / 16, &buf, &seg, &sel)) {
            strcpy(AIL_error, D_00126cd4);
            AIL_mem_free_lock(dig);
            return NULL;
        }
        if (dig->DMA_buf != NULL) {
            AIL_mem_free_DOS(dig->DMA_buf, dig->DMA_seg, dig->DMA_sel);
        }
        dig->DMA_buf = buf;
        dig->DMA_seg = seg;
        dig->DMA_sel = sel;
        linear = (U32)seg >> 12;
        start_pg = ((U32)linear) & 0xf0000L;
        end_pg = ((U32)((U8 *)linear + DMA_bytes - 1)) & 0xf0000L;
    } while (start_pg != end_pg);
    if (!SS_configure_buffers(dig)) {
        AIL_mem_free_DOS(dig->DMA_buf, dig->DMA_seg, dig->DMA_sel);
        AIL_mem_free_lock(dig);
        return NULL;
    }
    AIL_call_driver(dig->drvr, DRV_INIT_DEV, NULL, NULL);
    dig->drvr->initialized = 1;
    dig->samples_per_buffer = dig->half_buffer_size
                              / (dig->channels_per_sample * dig->bytes_per_channel);
    dig->channels_per_buffer = dig->half_buffer_size / dig->bytes_per_channel;
    dig->build_size = dig->channels_per_buffer * sizeof(S32);
    dig->build_buffer = AIL_mem_alloc_lock(dig->build_size);
    if (dig->build_buffer == NULL) {
        strcpy(AIL_error, D_00126cf4);
        if (drvr->PM_ISR != -1) {
            AIL_restore_USE16_ISR(drvr->PM_ISR);
        }
        AIL_call_driver(dig->drvr, DRV_SHUTDOWN, NULL, NULL);
        dig->drvr->initialized = 0;
        AIL_mem_free_DOS(dig->DMA_buf, dig->DMA_seg, dig->DMA_sel);
        AIL_mem_free_lock(dig);
        return NULL;
    }
    dig->n_samples = AIL_preference[DIG_MIXER_CHANNELS];
    dig->samples = AIL_mem_alloc_lock(sizeof(SAMPLE) * dig->n_samples);
    if (dig->samples == NULL) {
        strcpy(AIL_error, D_00126d18);
        if (drvr->PM_ISR != -1) {
            AIL_restore_USE16_ISR(drvr->PM_ISR);
        }
        AIL_call_driver(dig->drvr, DRV_SHUTDOWN, NULL, NULL);
        dig->drvr->initialized = 0;
        AIL_mem_free_DOS(dig->DMA_buf, dig->DMA_seg, dig->DMA_sel);
        AIL_mem_free_lock(dig->build_buffer);
        AIL_mem_free_lock(dig);
        return NULL;
    }
    for (i = 0; i < dig->n_samples; i++) {
        memcpy(dig->samples[i].tag, D_00126d40, 4);
        dig->samples[i].status = SMP_FREE;
        dig->samples[i].driver = dig;
    }
    dig->timer = AIL_register_timer((AILTIMERCB)fn_000dff00);
    if (dig->timer == -1) {
        strcpy(AIL_error, D_00126d48);
        if (drvr->PM_ISR != -1) {
            AIL_restore_USE16_ISR(drvr->PM_ISR);
        }
        AIL_call_driver(dig->drvr, DRV_SHUTDOWN, NULL, NULL);
        dig->drvr->initialized = 0;
        AIL_mem_free_DOS(dig->DMA_buf, dig->DMA_seg, dig->DMA_sel);
        AIL_mem_free_lock(dig->samples);
        AIL_mem_free_lock(dig->build_buffer);
        AIL_mem_free_lock(dig);
        return NULL;
    }
    AIL_set_timer_user(dig->timer, (U32)dig);
    AIL_set_timer_frequency(dig->timer, AIL_preference[DIG_SERVICE_RATE]);
    AIL_start_timer(dig->timer);
    dig->drvr->destructor = (void (__cdecl *)(void *))fn_000e0660;
    dig->drvr->descriptor = dig;
    AILSSA_flush_buffer(dig);
    AILSSA_DMA_copy(dig, 0);
    AILSSA_DMA_copy(dig, 1);
    return dig;
}

// FUNCTION: 0x000e0d30
HDIGDRIVER __cdecl AIL_API_install_DIG_driver_file(char *filename, IO_PARMS *IO)
{
    char fn[256];
    U32 *driver_image;
    AIL_DRIVER *drvr;
    HDIGDRIVER dig;

    driver_image = AIL_file_read(filename, FILE_READ_WITH_SIZE);
    if (driver_image == NULL) {
        if (AIL_driver_directory[0] == 0) {
        error:
            strcpy(AIL_error, D_00126d60);
            return NULL;
        }
        strcpy(fn, AIL_driver_directory);
        strcat(fn, filename);
        driver_image = AIL_file_read(fn, FILE_READ_WITH_SIZE);
        if (driver_image == NULL) {
            goto error;
        }
    }
    drvr = AIL_install_driver((U8 *)(driver_image + 1), driver_image[0]);
    AIL_mem_free_lock(driver_image);
    if (drvr == NULL) {
        return NULL;
    }
    dig = SS_construct_DIG_driver(drvr, IO);
    if (dig == NULL) {
        AIL_uninstall_driver(drvr);
    }
    return dig;
}

// FUNCTION: 0x000e0e10
HDIGDRIVER __cdecl AIL_API_install_DIG_driver_image(void *driver_image, U32 size, IO_PARMS *IO)
{
    AIL_DRIVER *drvr;
    HDIGDRIVER dig;

    drvr = AIL_install_driver(driver_image, size);
    if (drvr == NULL) {
        return NULL;
    }
    dig = SS_construct_DIG_driver(drvr, IO);
    if (dig == NULL) {
        AIL_uninstall_driver(drvr);
    }
    return dig;
}

// FUNCTION: 0x000e0e50
S32 __cdecl AIL_API_install_DIG_INI(HDIGDRIVER *dig)
{
    AIL_INI INI;

    if (!AIL_read_INI(&INI, D_00126d78)) {
        strcpy(AIL_error, D_00126d80);
        return 1;
    }
    *dig = AIL_install_DIG_driver_file(INI.driver_name, &INI.IO);
    if (*dig == NULL) {
        return 2;
    }
    return 0;
}

// FUNCTION: 0x000e0ed0
void __cdecl AIL_API_uninstall_DIG_driver(HDIGDRIVER dig)
{
    AIL_uninstall_driver(dig->drvr);
}

// FUNCTION: 0x000e0ef0
S32 __cdecl AIL_API_active_sample_count(HDIGDRIVER dig)
{
    S32 i;
    S32 n;

    n = 0;
    for (i = 0; i < dig->n_samples; i++) {
        if (dig->samples[i].status == SMP_PLAYING) {
            n++;
        }
    }
    return n;
}

// FUNCTION: 0x000e0f20
HSAMPLE __cdecl AIL_API_allocate_sample_handle(HDIGDRIVER dig)
{
    S32 i;
    HSAMPLE S;

    AIL_lock();
    for (i = 0; i < dig->n_samples; i++) {
        if (dig->samples[i].status == SMP_FREE) {
            break;
        }
    }
    if (i == dig->n_samples) {
        strcpy(AIL_error, D_00126da0);
        AIL_unlock();
        return NULL;
    }
    S = &dig->samples[i];
    AIL_init_sample(S);
    AIL_unlock();
    return S;
}

// FUNCTION: 0x000e0fa0
void __cdecl AIL_API_release_sample_handle(HSAMPLE S)
{
    if (S == NULL) {
        return;
    }
    S->status = SMP_FREE;
}

// FUNCTION: 0x000e0fb0
void __cdecl AIL_API_init_sample(HSAMPLE S)
{
    if (S == NULL) {
        return;
    }
    S->status = SMP_DONE;
    S->start[0] = NULL;
    S->len[0] = 0;
    S->pos[0] = 0;
    S->done[0] = 0;
    S->start[1] = NULL;
    S->len[1] = 0;
    S->pos[1] = 0;
    S->done[1] = 1;
    S->current_buffer = 0;
    S->last_buffer = -2;
    S->starved = 1;
    S->loop_count = 1;
    S->loop_start = 0;
    S->loop_end = -1;
    S->format = DIG_F_MONO_8;
    S->flags = 0;
    S->playback_rate = 11025;
    S->volume = AIL_preference[DIG_DEFAULT_VOLUME];
    if (S->driver->hw_format == DIG_F_MONO_8 || S->driver->hw_format == DIG_F_MONO_16) {
        S->pan = 0;
    } else {
        S->pan = 64;
    }
    S->SOB = NULL;
    S->EOB = NULL;
    S->EOS = NULL;
    S->service_type = 1;
    SS_build_amplitude_tables(S);
}

// FUNCTION: 0x000e10a0
U32 __cdecl AIL_API_sample_status(HSAMPLE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->status;
}

// FUNCTION: 0x000e10b0
void __cdecl AIL_API_set_sample_address(HSAMPLE S, void *start, U32 len)
{
    if (S == NULL) {
        return;
    }
    S->start[1] = NULL;
    S->len[1] = 0;
    S->start[0] = start;
    S->len[0] = len;
}

// FUNCTION: 0x000e10e0
void __cdecl AIL_API_set_sample_type(HSAMPLE S, S32 format, U32 flags)
{
    if (S == NULL) {
        return;
    }
    if (format != S->format || flags != S->flags) {
        S->format = format;
        S->flags = flags;
        SS_build_amplitude_tables(S);
    }
}

// FUNCTION: 0x000e1110
S32 __cdecl AIL_API_sample_playback_rate(HSAMPLE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->playback_rate;
}

// FUNCTION: 0x000e1120
void __cdecl AIL_API_set_sample_playback_rate(HSAMPLE S, S32 playback_rate)
{
    if (S == NULL) {
        return;
    }
    S->playback_rate = playback_rate;
}

// FUNCTION: 0x000e1130
S32 __cdecl AIL_API_sample_volume(HSAMPLE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->volume;
}

// FUNCTION: 0x000e1140
void __cdecl AIL_API_set_sample_volume(HSAMPLE S, S32 volume)
{
    if (S == NULL) {
        return;
    }
    if (volume != S->volume) {
        S->volume = volume;
        SS_build_amplitude_tables(S);
    }
}

// FUNCTION: 0x000e1160
S32 __cdecl AIL_API_sample_pan(HSAMPLE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->pan;
}

// FUNCTION: 0x000e1170
void __cdecl AIL_API_set_sample_pan(HSAMPLE S, S32 pan)
{
    if (S == NULL) {
        return;
    }
    if (pan != S->pan) {
        S->pan = pan;
        SS_build_amplitude_tables(S);
    }
}

// FUNCTION: 0x000e1190
U32 __cdecl AIL_API_sample_position(HSAMPLE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->pos[S->current_buffer];
}

// FUNCTION: 0x000e11b0
void __cdecl AIL_API_set_sample_position(HSAMPLE S, U32 pos)
{
    if (S == NULL) {
        return;
    }
    S->pos[S->current_buffer] = pos;
}

// FUNCTION: 0x000e11d0
S32 __cdecl AIL_API_sample_loop_count(HSAMPLE S)
{
    if (S == NULL) {
        return -1;
    }
    return S->loop_count;
}

// FUNCTION: 0x000e11f0
void __cdecl AIL_API_set_sample_loop_count(HSAMPLE S, S32 loop_count)
{
    if (S == NULL) {
        return;
    }
    S->loop_count = loop_count;
}

// FUNCTION: 0x000e1200
void __cdecl AIL_API_set_sample_loop_block(HSAMPLE S, S32 loop_start_offset, S32 loop_end_offset)
{
    if (S == NULL) {
        return;
    }
    S->loop_start = loop_start_offset;
    S->loop_end = loop_end_offset;
}

// FUNCTION: 0x000e1220
void __cdecl AIL_API_start_sample(HSAMPLE S)
{
    if (S == NULL) {
        return;
    }
    if (S->status == SMP_FREE) {
        return;
    }
    if (S->len[S->current_buffer] == 0 || S->start[S->current_buffer] == NULL) {
        return;
    }
    S->pos[S->current_buffer] = 0;
    S->service_type = 1;
    S->status = SMP_PLAYING;
    SS_start_DIG_driver_playback(S->driver);
}

// FUNCTION: 0x000e1270
void __cdecl AIL_API_stop_sample(HSAMPLE S)
{
    if (S == NULL) {
        return;
    }
    if (S->status != SMP_PLAYING) {
        return;
    }
    S->status = SMP_STOPPED;
}

// FUNCTION: 0x000e1290
void __cdecl AIL_API_resume_sample(HSAMPLE S)
{
    if (S == NULL) {
        return;
    }
    if (S->status == SMP_STOPPED) {
        S->status = SMP_PLAYING;
        SS_start_DIG_driver_playback(S->driver);
    }
}

// FUNCTION: 0x000e12c0
void __cdecl AIL_API_end_sample(HSAMPLE S)
{
    if (S == NULL) {
        return;
    }
    if (S->status == SMP_FREE || S->status == SMP_DONE) {
        return;
    }
    S->status = SMP_DONE;
    S->starved = 1;
    if (S->EOB != NULL) {
        S->EOB(S);
    }
    if (S->EOS != NULL) {
        S->EOS(S);
    }
}

// FUNCTION: 0x000e1310
AILSAMPLECB __cdecl AIL_API_register_SOB_callback(HSAMPLE S, AILSAMPLECB SOB)
{
    AILSAMPLECB old;

    if (S == NULL) {
        return NULL;
    }
    old = S->SOB;
    S->SOB = SOB;
    return old;
}

// FUNCTION: 0x000e1330
AILSAMPLECB __cdecl AIL_API_register_EOB_callback(HSAMPLE S, AILSAMPLECB EOB)
{
    AILSAMPLECB old;

    if (S == NULL) {
        return NULL;
    }
    old = S->EOB;
    S->EOB = EOB;
    return old;
}

// FUNCTION: 0x000e1350
AILSAMPLECB __cdecl AIL_API_register_EOS_callback(HSAMPLE S, AILSAMPLECB EOS)
{
    AILSAMPLECB old;

    if (S == NULL) {
        return NULL;
    }
    old = S->EOS;
    S->EOS = EOS;
    return old;
}

// FUNCTION: 0x000e1370
void __cdecl AIL_API_set_sample_user_data(HSAMPLE S, U32 index, S32 value)
{
    if (S == NULL) {
        return;
    }
    S->user_data[index] = value;
}

// FUNCTION: 0x000e1390
S32 __cdecl AIL_API_sample_user_data(HSAMPLE S, U32 index)
{
    if (S == NULL) {
        return 0;
    }
    return S->user_data[index];
}

// FUNCTION: 0x000e13b0
void __cdecl AIL_API_set_digital_master_volume(HDIGDRIVER dig, S32 master_volume)
{
    S32 i;

    dig->master_volume = master_volume;
    for (i = 0; i < dig->n_samples; i++) {
        if (dig->samples[i].status != SMP_FREE) {
            SS_build_amplitude_tables(&dig->samples[i]);
        }
    }
}

// FUNCTION: 0x000e13f0
S32 __cdecl AIL_API_digital_master_volume(HDIGDRIVER dig)
{
    return dig->master_volume;
}

// FUNCTION: 0x000e1410
S32 __cdecl AIL_API_minimum_sample_buffer_size(HDIGDRIVER dig, S32 playback_rate, S32 format)
{
    S32 app_bytes_per_sample;
    S32 n;

    switch (format) {
    case DIG_F_MONO_8:
        app_bytes_per_sample = 1;
        break;
    case DIG_F_MONO_16:
        app_bytes_per_sample = 2;
        break;
    case DIG_F_STEREO_8:
        app_bytes_per_sample = 2;
        break;
    case DIG_F_STEREO_16:
        app_bytes_per_sample = 4;
        break;
    }
    n = app_bytes_per_sample * dig->half_buffer_size
        / (dig->channels_per_sample * dig->bytes_per_channel);
    n = n * playback_rate / dig->DMA_rate;
    n += n * AIL_preference[DIG_RESAMPLING_TOLERANCE] / 32768;
    if (playback_rate != dig->DMA_rate && playback_rate * 2 != dig->DMA_rate
        && playback_rate * 4 != dig->DMA_rate) {
        n += 4;
    }
    n = (n + 255) & ~255;
    return n;
}

// FUNCTION: 0x000e14b0
void __cdecl AIL_API_load_sample_buffer(HSAMPLE S, U32 buff_num, void *buffer, U32 len)
{
    if (S == NULL) {
        return;
    }
    S->done[buff_num] = (len == 0);
    S->pos[buff_num] = 0;
    S->len[buff_num] = len;
    S->start[buff_num] = buffer;
    S->starved = 0;
    if (len != 0 && S->status != SMP_PLAYING) {
        S->status = SMP_PLAYING;
        S->service_type = 2;
        SS_start_DIG_driver_playback(S->driver);
    }
}

// FUNCTION: 0x000e1520
S32 __cdecl AIL_API_sample_buffer_ready(HSAMPLE S)
{
    if (S == NULL) {
        return -1;
    }
    switch (S->last_buffer) {
    case -2:
        S->done[1] = 0;
        S->last_buffer = -1;
        return 0;
    case -1:
        S->last_buffer = S->current_buffer;
        return 1;
    }
    if (S->last_buffer == S->current_buffer) {
        return -1;
    }
    S->last_buffer = S->current_buffer;
    return S->current_buffer ^ 1;
}

// FUNCTION: 0x000e1580
void __cdecl AIL_API_digital_configuration(HDIGDRIVER dig, S32 *rate, S32 *format, char *string)
{
    if (rate != NULL) {
        *rate = dig->DMA_rate;
    }
    if (format != NULL) {
        *format = dig->hw_format;
    }
    if (string != NULL) {
        strcpy(string, (char *)dig->drvr->VHDR->dev_name);
    }
}

// FUNCTION: 0x000e15d0
S32 __cdecl AIL_API_sample_buffer_info(HSAMPLE S, U32 *pos0, U32 *len0, U32 *pos1, U32 *len1)
{
    if (pos0 != NULL) {
        *pos0 = S->pos[0];
    }
    if (pos1 != NULL) {
        *pos1 = S->pos[1];
    }
    if (len0 != NULL) {
        *len0 = S->len[0];
    }
    if (len1 != NULL) {
        *len1 = S->len[1];
    }
    return S->starved;
}

// FUNCTION: 0xe1620
void AILSS_end(void)
{
    if (locked) {
        AIL_vmm_unlock_range(AILSS_start, AILSS_end);
        AIL_vmm_unlock(pan_graph, sizeof(pan_graph));
        AIL_vmm_unlock(&entry, sizeof(entry));
        AIL_vmm_unlock(&dig, sizeof(dig));
        AIL_vmm_unlock(&VDI, sizeof(VDI));
        AIL_vmm_unlock(&flags, sizeof(flags));
        AIL_vmm_unlock(&cnt, sizeof(cnt));
        AIL_vmm_unlock(&n, sizeof(n));
        AIL_vmm_unlock(&current, sizeof(current));
        AIL_vmm_unlock(&S, sizeof(S));
        locked = 0;
    }
}
