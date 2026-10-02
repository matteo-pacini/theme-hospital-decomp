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

U8 pan_graph[128] = {
    0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e,
    0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e,
    0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e,
    0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e, 0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
};

/* Timer callback: __pascal (callee pops). */
void SS_SERVE(U32 user);
#pragma aux (__pascal) SS_SERVE "SS_SERVE";
/* Driver destructor: __cdecl. */
void SS_destroy_DIG_driver(HDIGDRIVER dig);
#pragma aux (__cdecl) SS_destroy_DIG_driver "_SS_destroy_DIG_driver";
void AILSS_end(void);

// Module state. The BSS layout follows symbol handles and name hashes, not
// declaration order (docs/MATCHING.md, "Static data layout"): the
// declarations above put VDI in the same 25-symbol block as S..cnt, and the
// names hash in the original's order. Adding or removing a declaration
// before this point can split them again.
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

// Locks this module's code, pan table and state, and the mixer's (AILSSA),
// against paging, once.
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

// Starts DMA playback on `dig` at its hardware format and rate, if not already
// playing.
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

// Stops DMA playback on `dig`, if playing.
// FUNCTION: 0x000dfa80
void __cdecl SS_stop_DIG_driver_playback(HDIGDRIVER dig)
{
    if (dig->playing) {
        AIL_call_driver(dig->drvr, DIG_STOP_P_REQ, NULL, NULL);
        dig->playing = 0;
    }
}

// Mixes playing sample `S` into its driver's build buffer until the buffer is
// full. Handles loops (loop_count 0 = forever, loop_end -1 = end of data),
// calls SOB before each piece and EOB each time the loop end or the end of the
// data is reached, and for a
// double-buffered sample moves on to the other buffer if it is loaded and
// unplayed, else flags the sample starved. When the data runs out the sample is
// done and EOS is called.
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

// Clamps `S`'s volume and pan to 0-127 and rebuilds its scaling from them and
// the driver's master volume: for 16-bit data just a left and right factor, for
// 8-bit data 256-entry tables mapping each sample byte to a scaled value, left
// and right ones when either the sample or the output is stereo, otherwise a
// single table.
// FUNCTION: 0x000dfbe0
// pan doubles as every loop counter, and the mono tables are filled through
// right: the register assignment depends on those variables being shared.
void __cdecl SS_build_amplitude_tables(HSAMPLE S)
{
    S32 pan;
    S32 val;
    S32 vol;
    S32 *right;
    S32 right_pan;
    S32 *left;
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
            for (pan = 0; pan < 32768; pan += 256) {
                val = (pan * vol) >> 7;
                *left++ = (val * left_pan) >> 7;
                *right++ = (val * right_pan) >> 7;
            }
            for (pan = -32768; pan < 0; pan += 256) {
                val = (pan * vol) >> 7;
                *left++ = (val * left_pan) >> 7;
                *right++ = (val * right_pan) >> 7;
            }
        } else {
            for (pan = -32768; pan < 32768; pan += 256) {
                val = (pan * vol) >> 7;
                *left++ = (val * left_pan) >> 7;
                *right++ = (val * right_pan) >> 7;
            }
        }
    } else {
        right = S->vol_scale[0];
        if (S->flags & DIG_PCM_SIGN) {
            for (pan = 0; pan < 32768; pan += 256) {
                *right++ = (pan * vol) >> 7;
            }
            for (pan = -32768; pan < 0; pan += 256) {
                *right++ = (pan * vol) >> 7;
            }
        } else {
            for (pan = -32768; pan < 32768; pan += 256) {
                *right++ = (pan * vol) >> 7;
            }
        }
    }
}

// Service timer callback of digital driver `user` (an HDIGDRIVER). Once the
// driver has switched DMA halves, mixes every playing sample and copies the
// result into the idle half (and notifies the driver if it wants that); stops
// DMA after three silent services. Reentrant calls return at once; with
// DIG_SS_LOCK set it runs with interrupts disabled.
// FUNCTION: 0xdff00
void SS_SERVE(U32 user)
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

// Chooses `dig`'s hardware format (the supported one nearest the DIG_USE_STEREO
// and DIG_USE_16_BITS preferences), DMA rate (from DIG_HARDWARE_SAMPLE_RATE: 0
// minimum, 1 nominal, 2 maximum, else the closest of the three) and DMA
// half-buffer size (DIG_LATENCY milliseconds of output rounded to the nearest
// power of two from 8 to half of DIG_DMA_RESERVE, then clamped to the driver's
// limits), and places the two halves in the DMA buffer. Playing
// samples are suspended around the change. Returns 1, or 0 with AIL_error set
// if the driver's minimum buffer is too large for VDM execution.
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
            strcpy(AIL_error, "Minimum DMA buffer size too large for VDM\n");
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

// Tries `dig`'s driver with I/O settings `IO`, negative fields taken from the
// driver's first standard configuration, by storing them in its header and
// sending DRV_VERIFY_IO. Returns the driver's answer, nonzero if the hardware
// is there.
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

// Destructor of a digital driver: stops playback over silenced DMA buffers,
// then frees `dig`'s timer, DMA buffer, samples, build buffer and descriptor.
// FUNCTION: 0xe0660
void SS_destroy_DIG_driver(HDIGDRIVER dig)
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

// Builds the HDIGDRIVER for installed .DIG driver `drvr`: finds the hardware
// with `IO`, else the driver's environment variable, else (with
// AIL_SCAN_FOR_HARDWARE) each standard configuration, recording the attempt in
// AIL_last_IO_attempt; allocates a DMA buffer that does not cross a 64K page,
// configures it, initialises the device, allocates the build buffer and
// DIG_MIXER_CHANNELS samples, and starts the service timer at DIG_SERVICE_RATE.
// Returns it, or NULL with AIL_error set.
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
        strcpy(AIL_error, "Could not allocate memory for driver\n");
        return NULL;
    }
    memcpy(dig->tag, "HDIG", 4);
    dig->drvr = drvr;
    if (dig->drvr->type != DIG_DRIVER) {
        strcpy(AIL_error, ".DIG driver required\n");
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
        strcpy(AIL_error, "Digital sound hardware not found\n");
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
            strcpy(AIL_error, "Could not allocate DMA buffers\n");
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
        strcpy(AIL_error, "Could not allocate build buffer\n");
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
        strcpy(AIL_error, "Could not allocate SAMPLE structures\n");
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
        memcpy(dig->samples[i].tag, "HSAM", 4);
        dig->samples[i].status = SMP_FREE;
        dig->samples[i].driver = dig;
    }
    dig->timer = AIL_register_timer((AILTIMERCB)SS_SERVE);
    if (dig->timer == -1) {
        strcpy(AIL_error, "Out of timer handles\n");
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
    dig->drvr->destructor = (void (__cdecl *)(void *))SS_destroy_DIG_driver;
    dig->drvr->descriptor = dig;
    AILSSA_flush_buffer(dig);
    AILSSA_DMA_copy(dig, 0);
    AILSSA_DMA_copy(dig, 1);
    return dig;
}

// Loads the .DIG driver file `filename` (also looked up in the driver
// directory), installs it and sets it up with I/O settings `IO` (NULL to
// detect). Returns the driver, or NULL with AIL_error set.
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
            strcpy(AIL_error, "Driver file not found\n");
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

// Installs the .DIG driver image `driver_image` of `size` bytes and sets it up
// with I/O settings `IO` (NULL to detect). Returns the driver, or NULL with
// AIL_error set.
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

// Installs the digital driver and I/O settings named in DIG.INI, storing it in
// `*dig`. Returns 0 on success, 1 if DIG.INI cannot be read, 2 if the install
// fails.
// FUNCTION: 0x000e0e50
S32 __cdecl AIL_API_install_DIG_INI(HDIGDRIVER *dig)
{
    AIL_INI INI;

    if (!AIL_read_INI(&INI, "DIG.INI")) {
        strcpy(AIL_error, "Unable to open file DIG.INI\n");
        return 1;
    }
    *dig = AIL_install_DIG_driver_file(INI.driver_name, &INI.IO);
    if (*dig == NULL) {
        return 2;
    }
    return 0;
}

// Uninstalls digital driver `dig` and frees everything it owns.
// FUNCTION: 0x000e0ed0
void __cdecl AIL_API_uninstall_DIG_driver(HDIGDRIVER dig)
{
    AIL_uninstall_driver(dig->drvr);
}

// Returns the number of `dig`'s samples that are playing.
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

// Returns a free sample of `dig`, initialised with AIL_init_sample, or NULL
// with AIL_error set if all are in use.
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
        strcpy(AIL_error, "Out of sample handles\n");
        AIL_unlock();
        return NULL;
    }
    S = &dig->samples[i];
    AIL_init_sample(S);
    AIL_unlock();
    return S;
}

// Returns sample `S` to its driver's pool, silencing it. NULL is ignored.
// FUNCTION: 0x000e0fa0
void __cdecl AIL_API_release_sample_handle(HSAMPLE S)
{
    if (S == NULL) {
        return;
    }
    S->status = SMP_FREE;
}

// Resets sample `S` to defaults: done, no data, played once, 8-bit mono at
// 11025 Hz, default volume, pan 64 (0 on mono hardware), no callbacks.
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

// Returns `S`'s SMP_... status, 0 for NULL.
// FUNCTION: 0x000e10a0
U32 __cdecl AIL_API_sample_status(HSAMPLE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->status;
}

// Makes the `len` bytes at `start` `S`'s (single-buffered) data.
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

// Sets `S`'s data format (DIG_F_...) and `flags` (DIG_PCM_SIGN), rebuilding its
// scaling if they changed.
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

// Returns `S`'s playback rate in Hz, 0 for NULL.
// FUNCTION: 0x000e1110
S32 __cdecl AIL_API_sample_playback_rate(HSAMPLE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->playback_rate;
}

// Sets `S`'s playback rate to `playback_rate` Hz.
// FUNCTION: 0x000e1120
void __cdecl AIL_API_set_sample_playback_rate(HSAMPLE S, S32 playback_rate)
{
    if (S == NULL) {
        return;
    }
    S->playback_rate = playback_rate;
}

// Returns `S`'s volume (0-127), 0 for NULL.
// FUNCTION: 0x000e1130
S32 __cdecl AIL_API_sample_volume(HSAMPLE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->volume;
}

// Sets `S`'s volume (0-127), rebuilding its scaling if it changed.
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

// Returns `S`'s pan (0 left - 127 right), 0 for NULL.
// FUNCTION: 0x000e1160
S32 __cdecl AIL_API_sample_pan(HSAMPLE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->pan;
}

// Sets `S`'s pan (0 left - 127 right), rebuilding its scaling if it changed.
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

// Returns the byte offset of playback in `S`'s current buffer, 0 for NULL.
// FUNCTION: 0x000e1190
U32 __cdecl AIL_API_sample_position(HSAMPLE S)
{
    if (S == NULL) {
        return 0;
    }
    return S->pos[S->current_buffer];
}

// Moves playback in `S`'s current buffer to byte offset `pos`.
// FUNCTION: 0x000e11b0
void __cdecl AIL_API_set_sample_position(HSAMPLE S, U32 pos)
{
    if (S == NULL) {
        return;
    }
    S->pos[S->current_buffer] = pos;
}

// Returns how many more times `S` plays (0 = forever), -1 for NULL.
// FUNCTION: 0x000e11d0
S32 __cdecl AIL_API_sample_loop_count(HSAMPLE S)
{
    if (S == NULL) {
        return -1;
    }
    return S->loop_count;
}

// Sets how many times `S` plays to `loop_count` (0 = forever).
// FUNCTION: 0x000e11f0
void __cdecl AIL_API_set_sample_loop_count(HSAMPLE S, S32 loop_count)
{
    if (S == NULL) {
        return;
    }
    S->loop_count = loop_count;
}

// Sets the byte offsets each repeat of `S` starts and ends at (end -1 = end of
// data).
// FUNCTION: 0x000e1200
void __cdecl AIL_API_set_sample_loop_block(HSAMPLE S, S32 loop_start_offset, S32 loop_end_offset)
{
    if (S == NULL) {
        return;
    }
    S->loop_start = loop_start_offset;
    S->loop_end = loop_end_offset;
}

// Plays `S` from the start of its current buffer, starting DMA if needed.
// Ignored if `S` is free or has no data.
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

// Pauses `S` if it is playing.
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

// Resumes `S` if it is paused.
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

// Ends `S` now unless it is free or already done: marks it done and calls its
// EOB and EOS callbacks.
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

// Sets `S`'s start-of-block callback; returns the previous one (NULL for a NULL
// `S`).
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

// Sets `S`'s end-of-buffer callback; returns the previous one (NULL for a NULL
// `S`).
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

// Sets `S`'s end-of-sample callback; returns the previous one (NULL for a NULL
// `S`).
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

// Stores `value` in `S`'s user data slot `index` (0-7, unchecked).
// FUNCTION: 0x000e1370
void __cdecl AIL_API_set_sample_user_data(HSAMPLE S, U32 index, S32 value)
{
    if (S == NULL) {
        return;
    }
    S->user_data[index] = value;
}

// Returns `S`'s user data slot `index` (0-7, unchecked), 0 for NULL.
// FUNCTION: 0x000e1390
S32 __cdecl AIL_API_sample_user_data(HSAMPLE S, U32 index)
{
    if (S == NULL) {
        return 0;
    }
    return S->user_data[index];
}

// Sets `dig`'s master volume (0-127) and rebuilds the scaling of every
// allocated sample.
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

// Returns `dig`'s master volume.
// FUNCTION: 0x000e13f0
S32 __cdecl AIL_API_digital_master_volume(HDIGDRIVER dig)
{
    return dig->master_volume;
}

// Returns the smallest buffer size, in bytes, for AIL_load_sample_buffer data
// at `playback_rate` Hz in `format` to cover one DMA half-buffer: allowing for
// DIG_RESAMPLING_TOLERANCE and, unless the rate is the DMA rate, a half or a
// quarter of it, 4 more bytes, rounded up to 256.
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

// Loads double buffer `buff_num` (0 or 1) of `S` with the `len` bytes at
// `buffer`, `len` 0 meaning no more data, and starts `S` playing in streaming
// mode if it is not already.
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

// Returns which buffer of streaming sample `S` to load next (0 or 1), or -1 if
// none is needed yet (also for NULL). The first two calls after AIL_init_sample
// return 0 and then 1.
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

// Stores `dig`'s DMA rate in `*rate`, hardware format in `*format` and device
// name in `string`; NULL ones are skipped.
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

// Stores the positions and lengths of `S`'s two buffers through the non-NULL
// pointers. Returns 1 if `S` has run out of data (starved), else 0.
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

// Unlocks what AILSS_start locked, except the mixer. Its address also marks the
// end of the locked code range.
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
