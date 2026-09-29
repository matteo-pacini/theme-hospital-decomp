#ifndef AIL_H
#define AIL_H

/*
 * Miles Sound System (RAD Game Tools) internals, as linked into the game.
 * Public entry points use the __cdecl convention, so their symbols carry a
 * leading underscore (_AIL_...).
 */

typedef signed long S32;
typedef unsigned long U32;
typedef signed short S16;
typedef unsigned short U16;
typedef signed char S8;
typedef unsigned char U8;

#ifndef NULL
#define NULL 0
#endif

typedef struct _SAMPLE *HSAMPLE;
typedef struct _DIG_DRIVER *HDIGDRIVER;

typedef void (__pascal *AILSAMPLECB)(HSAMPLE sample);

typedef struct _SAMPLE {
    char tag[4];
    HDIGDRIVER driver;
    U32 status;
    void *start[2];
    U32 len[2];
    U32 pos[2];
    U32 done[2];
    S32 current_buffer;
    S32 last_buffer;
    S32 starved;
    S32 loop_count;
    S32 loop_start;
    S32 loop_end;
    S32 format;
    U32 flags;
    S32 playback_rate;
    S32 volume;
    S32 pan;
    S32 vol_scale[2][256];
    S32 service_type;
    AILSAMPLECB SOB;
    AILSAMPLECB EOB;
    AILSAMPLECB EOS;
    S32 user_data[8];
    S32 system_data[8];
} SAMPLE;

#define SMP_FREE 0x0001
#define SMP_DONE 0x0002
#define SMP_PLAYING 0x0004
#define SMP_STOPPED 0x0008

#define DIG_F_MONO_8 0
#define DIG_F_MONO_16 1
#define DIG_F_STEREO_8 2
#define DIG_F_STEREO_16 3

#define DIG_PCM_SIGN 0x0001

#define DIG_SERVICE_RATE 0
#define DIG_HARDWARE_SAMPLE_RATE 1
#define DIG_DMA_RESERVE 2
#define DIG_LATENCY 3
#define DIG_MIXER_CHANNELS 4
#define DIG_DEFAULT_VOLUME 5
#define DIG_RESAMPLING_TOLERANCE 6
#define DIG_USE_STEREO 7
#define DIG_USE_16_BITS 8
#define DIG_ALLOW_16_BIT_DMA 9
#define DIG_SS_LOCK 10
#define MDI_SERVICE_RATE 11
#define MDI_SEQUENCES 12
#define MDI_DEFAULT_VOLUME 13
#define MDI_QUANT_ADVANCE 14
#define MDI_ALLOW_LOOP_BRANCHING 15
#define MDI_DEFAULT_BEND_RANGE 16
#define AIL_SCAN_FOR_HARDWARE 17
#define AIL_ALLOW_VDM_EXECUTION 18
#define MDI_DOUBLE_NOTE_OFF 19

typedef U32 REALFAR;
typedef S32 HTIMER;
typedef struct _REDBOOK *HREDBOOK;

typedef struct {
    S16 IO;
    S16 IRQ;
    S16 DMA_8_bit;
    S16 DMA_16_bit;
    S32 IO_reserved[4];
} IO_PARMS;

typedef struct {
    char device_name[128];
    char driver_name[128];
    IO_PARMS IO;
} AIL_INI;

#pragma pack(1)
typedef struct {
    S8 ID[8];
    U32 driver_version;
    REALFAR common_IO_configurations;
    U16 num_IO_configurations;
    REALFAR environment_string;
    IO_PARMS IO;
    S16 service_rate;
    U16 busy;
    U16 driver_num;
    U16 this_ISR;
    REALFAR prev_ISR;
    S8 scratch[128];
    S8 dev_name[80];
} VDI_HDR;
#pragma pack()

typedef struct _AIL_DRIVER {
    REALFAR seg;
    U32 sel;
    void *buf;
    U32 size;
    VDI_HDR *VHDR;
    S32 type;
    S32 initialized;
    S32 PM_ISR;
    HTIMER server;
    void (__cdecl *destructor)(void *descriptor);
    void *descriptor;
} AIL_DRIVER;

typedef struct _VDI_CALL {
    S16 AX;
    S16 BX;
    S16 CX;
    S16 DX;
    S16 SI;
    S16 DI;
} VDI_CALL;

#define DIG_DRIVER 0
#define MDI_DRIVER 1

#define DRV_INIT 0x300
#define DRV_SERVE 0x302
#define DRV_PARSE_ENV 0x303
#define DRV_SHUTDOWN 0x306
#define DRV_GET_INFO 0x301
#define DRV_VERIFY_IO 0x304
#define DRV_INIT_DEV 0x305
#define DIG_START_P_CMD 0x401
#define DIG_STOP_P_REQ 0x402
#define DIG_VSE 0x405

#pragma pack(1)
typedef struct {
    U16 minimum_physical_sample_rate;
    U16 nominal_physical_sample_rate;
    U16 maximum_physical_sample_rate;
    U16 minimum_DMA_half_buffer_size;
    U16 maximum_DMA_half_buffer_size;
    U32 flags;
} DIG_MODE;

typedef struct {
    U8 format_supported[16];
    DIG_MODE format_data[16];
} DIG_DDT;

typedef struct {
    REALFAR DMA_buffer_A;
    REALFAR DMA_buffer_B;
    S16 active_buffer;
} DIG_DST;
#pragma pack()

typedef struct _DIG_DRIVER {
    char tag[4];
    AIL_DRIVER *drvr;
    DIG_DDT *DDT;
    DIG_DST *DST;
    HTIMER timer;
    S32 half_buffer_size;
    S32 DMA_rate;
    S32 hw_format;
    U32 hw_mode_flags;
    REALFAR DMA_seg;
    U32 DMA_sel;
    void *DMA_buf;
    void *DMA[2];
    S16 *buffer_flag;
    S32 last_buffer;
    S32 channels_per_sample;
    S32 bytes_per_channel;
    S32 channels_per_buffer;
    S32 samples_per_buffer;
    S32 build_size;
    S32 *build_buffer;
    S32 playing;
    S32 quiet;
    HSAMPLE samples;
    S32 n_samples;
    S32 n_active_samples;
    S32 master_volume;
    U8 reserved[0x20];
};

typedef struct _MDI_DRIVER *HMDIDRIVER;
typedef struct _SEQUENCE *HSEQUENCE;
typedef struct _WAVE_SYNTH *HWAVESYNTH;
typedef struct _AUDIO_TYPE *HAUDIO;

typedef void (__pascal *AILTIMERCB)(U32 user);
typedef S32 (__pascal *AILPREFIXCB)(HSEQUENCE seq, S32 log, S32 data);
typedef void (__pascal *AILTRIGGERCB)(HSEQUENCE seq, S32 log, S32 data);
typedef void (__pascal *AILSEQUENCECB)(HSEQUENCE seq);
typedef void (__pascal *AILBEATCB)(HMDIDRIVER mdi, HSEQUENCE seq, S32 beat,
                                   S32 measure);
typedef S32 (__pascal *AILEVENTCB)(HMDIDRIVER mdi, HSEQUENCE seq, S32 status,
                                   S32 data_1, S32 data_2);
typedef S32 (__pascal *AILTIMBRECB)(HMDIDRIVER mdi, S32 bank, S32 patch);

#define NUM_CHANS 16
#define MAX_NOTES 32
#define FOR_NEST 4

#define SEQ_FREE 0x0001
#define SEQ_DONE 0x0002
#define SEQ_PLAYING 0x0004
#define SEQ_STOPPED 0x0008

#define MDI_MIDI_XMIT 0x502
#define MDI_INIT_INS_MGR 0x501
#define MDI_INSTALL_T_SET 0x503
#define MDI_GET_T_STATUS 0x504
#define MDI_PROT_UNPROT_T 0x505

#pragma pack(1)
typedef struct {
    REALFAR library_environment;
    REALFAR GTL_suffix;
} MDI_DDT;

typedef struct {
    char library_directory[128];
    char GTL_filename[128];
    U8 MIDI_data[512];
} MDI_DST;
#pragma pack()

typedef struct {
    S32 program[NUM_CHANS];
    S32 pitch_l[NUM_CHANS];
    S32 pitch_h[NUM_CHANS];
    S32 c_lock[NUM_CHANS];
    S32 c_prot[NUM_CHANS];
    S32 c_mute[NUM_CHANS];
    S32 c_v_prot[NUM_CHANS];
    S32 bank[NUM_CHANS];
    S32 indirect[NUM_CHANS];
    S32 callback[NUM_CHANS];
    S32 mod[NUM_CHANS];
    S32 vol[NUM_CHANS];
    S32 pan[NUM_CHANS];
    S32 exp[NUM_CHANS];
    S32 sus[NUM_CHANS];
    S32 reverb[NUM_CHANS];
    S32 chorus[NUM_CHANS];
    S32 bend_range[NUM_CHANS];
} CTRL_LOG;

typedef struct _SEQUENCE {
    char tag[4];
    HMDIDRIVER driver;
    U32 status;
    void *TIMB;
    void *RBRN;
    void *EVNT;
    U8 *EVNT_ptr;
    U8 *ICA;
    AILPREFIXCB prefix_callback;
    AILTRIGGERCB trigger_callback;
    AILBEATCB beat_callback;
    AILSEQUENCECB EOS;
    S32 loop_count;
    S32 interval_count;
    S32 interval_num;
    S32 volume;
    S32 volume_target;
    S32 volume_accum;
    S32 volume_period;
    S32 tempo_percent;
    S32 tempo_target;
    S32 tempo_accum;
    S32 tempo_period;
    S32 tempo_error;
    S32 beat_count;
    S32 measure_count;
    S32 time_numerator;
    S32 time_fraction;
    S32 beat_fraction;
    S32 time_per_beat;
    U8 *FOR_ptrs[FOR_NEST];
    S32 FOR_loop_count[FOR_NEST];
    S32 chan_map[NUM_CHANS];
    CTRL_LOG shadow;
    S32 note_count;
    S32 note_chan[MAX_NOTES];
    S32 note_num[MAX_NOTES];
    S32 note_time[MAX_NOTES];
    S32 user_data[8];
    S32 system_data[8];
} SEQUENCE;

typedef struct _MDI_DRIVER {
    char tag[4];
    AIL_DRIVER *drvr;
    MDI_DDT *DDT;
    MDI_DST *DST;
    HTIMER timer;
    S32 interval_time;
    S32 disable;
    HSEQUENCE sequences;
    S32 n_sequences;
    S32 lock[NUM_CHANS];
    HSEQUENCE locker[NUM_CHANS];
    HSEQUENCE owner[NUM_CHANS];
    HSEQUENCE user[NUM_CHANS];
    S32 state[NUM_CHANS];
    S32 notes[NUM_CHANS];
    AILEVENTCB event_trap;
    AILTIMBRECB timbre_trap;
    S32 message_count;
    S32 offset;
    S32 master_volume;
    S32 system_data[8];
};

/* aildebug.c */
/* aildebug.c and ail.c each keep a static lock flag; both are named _locked. */
extern S32 AIL_started;
extern U32 AIL_indent;
extern S32 AIL_debug;
extern S32 AIL_sys_debug;
extern S32 AIL_starttime;
extern S32 AIL_debugfile;
extern char AIL_indent_str[];
extern void (__pascal *AIL_sprintf)(char *dest, char const *fmt, S32 nargs,
                                    S32 *args);

void __pascal AIL_DEBUGOUTSTR(char const *str);
S32 __pascal TIME_WRITE(void);
void outresint(S32 value);
void outreshex(U32 value);
void oprintf1(char const *fmt, S32 a1);
void oprintf2(char const *fmt, S32 a1, S32 a2);
void oprintf3(char const *fmt, S32 a1, S32 a2, S32 a3);
void oprintf4(char const *fmt, S32 a1, S32 a2, S32 a3, S32 a4);
void oprintf5(char const *fmt, S32 a1, S32 a2, S32 a3, S32 a4, S32 a5);

/* aildebug.c: public entry points */
void __cdecl AIL_shutdown(void);
char *__cdecl AIL_set_driver_directory(char *dir);
void __cdecl AIL_lock(void);
void __cdecl AIL_unlock(void);
U32 __cdecl AIL_disable_interrupts(void);
void __cdecl AIL_restore_interrupts(U32 FD_register);
S32 __cdecl AIL_background(void);
S32 __cdecl AIL_read_INI(AIL_INI *INI, char *filename);
void __cdecl AIL_digital_configuration(HDIGDRIVER dig, S32 *rate, S32 *format,
                                       char *string);
void __cdecl AIL_sequence_position(HSEQUENCE S, S32 *beat, S32 *measure);
S32 __cdecl AIL_set_preference(U32 number, S32 value);
REALFAR __cdecl AIL_get_real_vect(U32 vectnum);
void __cdecl AIL_set_real_vect(U32 vectnum, REALFAR real_ptr);
void __cdecl AIL_set_USE16_ISR(S32 IRQ, REALFAR real_base, U32 ISR_offset);
void __cdecl AIL_restore_USE16_ISR(S32 IRQ);
S32 __cdecl AIL_call_driver(AIL_DRIVER *drvr, S32 fn, VDI_CALL *in, VDI_CALL *out);
void __cdecl AIL_delay(S32 intervals);
HTIMER __cdecl AIL_register_timer(AILTIMERCB callback_fn);
U32 __cdecl AIL_set_timer_user(HTIMER timer, U32 user);
void __cdecl AIL_set_timer_period(HTIMER timer, U32 microseconds);
void __cdecl AIL_set_timer_frequency(HTIMER timer, U32 hertz);
void __cdecl AIL_set_timer_divisor(HTIMER timer, U32 PIT_divisor);
S32 __cdecl AIL_interrupt_divisor(void);
void __cdecl AIL_start_timer(HTIMER timer);
void __cdecl AIL_start_all_timers(void);
void __cdecl AIL_stop_timer(HTIMER timer);
void __cdecl AIL_stop_all_timers(void);
void __cdecl AIL_release_timer_handle(HTIMER timer);
void __cdecl AIL_release_all_timers(void);
IO_PARMS *__cdecl AIL_get_IO_environment(AIL_DRIVER *drvr);
AIL_DRIVER *__cdecl AIL_install_driver(U8 *driver_image, U32 n_bytes);
void __cdecl AIL_uninstall_driver(AIL_DRIVER *drvr);
S32 __cdecl AIL_install_DIG_INI(HDIGDRIVER *dig);
HDIGDRIVER __cdecl AIL_install_DIG_driver_file(char *filename, IO_PARMS *IO);
void __cdecl AIL_uninstall_DIG_driver(HDIGDRIVER dig);
HSAMPLE __cdecl AIL_allocate_sample_handle(HDIGDRIVER dig);
HSAMPLE __cdecl AIL_allocate_file_sample(HDIGDRIVER dig, void *file_image, S32 block);
void __cdecl AIL_release_sample_handle(HSAMPLE S);
void __cdecl AIL_init_sample(HSAMPLE S);
S32 __cdecl AIL_set_sample_file(HSAMPLE S, void *file_image, S32 block);
void __cdecl AIL_set_sample_address(HSAMPLE S, void *start, U32 len);
void __cdecl AIL_set_sample_type(HSAMPLE S, S32 format, U32 flags);
void __cdecl AIL_start_sample(HSAMPLE S);
void __cdecl AIL_stop_sample(HSAMPLE S);
void __cdecl AIL_resume_sample(HSAMPLE S);
void __cdecl AIL_end_sample(HSAMPLE S);
void __cdecl AIL_set_sample_playback_rate(HSAMPLE S, S32 playback_rate);
void __cdecl AIL_set_sample_volume(HSAMPLE S, S32 volume);
void __cdecl AIL_set_sample_pan(HSAMPLE S, S32 pan);
void __cdecl AIL_set_sample_loop_count(HSAMPLE S, S32 loop_count);
void __cdecl AIL_set_sample_loop_block(HSAMPLE S, S32 loop_start_offset, S32 loop_end_offset);
U32 __cdecl AIL_sample_status(HSAMPLE S);
S32 __cdecl AIL_sample_playback_rate(HSAMPLE S);
S32 __cdecl AIL_sample_volume(HSAMPLE S);
S32 __cdecl AIL_sample_pan(HSAMPLE S);
S32 __cdecl AIL_sample_loop_count(HSAMPLE S);
void __cdecl AIL_set_digital_master_volume(HDIGDRIVER dig, S32 master_volume);
S32 __cdecl AIL_digital_master_volume(HDIGDRIVER dig);
HDIGDRIVER __cdecl AIL_install_DIG_driver_image(void *driver_image, U32 size, IO_PARMS *IO);
S32 __cdecl AIL_minimum_sample_buffer_size(HDIGDRIVER dig, S32 playback_rate, S32 format);
S32 __cdecl AIL_sample_buffer_ready(HSAMPLE S);
void __cdecl AIL_load_sample_buffer(HSAMPLE S, U32 buff_num, void *buffer, U32 len);
S32 __cdecl AIL_sample_buffer_info(HSAMPLE S, U32 *pos0, U32 *len0, U32 *pos1, U32 *len1);
void __cdecl AIL_set_sample_position(HSAMPLE S, U32 pos);
U32 __cdecl AIL_sample_position(HSAMPLE S);
AILSAMPLECB __cdecl AIL_register_SOB_callback(HSAMPLE S, AILSAMPLECB SOB);
AILSAMPLECB __cdecl AIL_register_EOB_callback(HSAMPLE S, AILSAMPLECB EOB);
AILSAMPLECB __cdecl AIL_register_EOS_callback(HSAMPLE S, AILSAMPLECB EOS);
AILSAMPLECB __cdecl AIL_register_EOF_callback(HSAMPLE S, AILSAMPLECB EOFILE);
void __cdecl AIL_set_sample_user_data(HSAMPLE S, U32 index, S32 value);
S32 __cdecl AIL_sample_user_data(HSAMPLE S, U32 index);
S32 __cdecl AIL_active_sample_count(HDIGDRIVER dig);
S32 __cdecl AIL_install_MDI_INI(HMDIDRIVER *mdi);
HMDIDRIVER __cdecl AIL_install_MDI_driver_file(char *filename, IO_PARMS *IO);
void __cdecl AIL_uninstall_MDI_driver(HMDIDRIVER mdi);
HSEQUENCE __cdecl AIL_allocate_sequence_handle(HMDIDRIVER mdi);
void __cdecl AIL_release_sequence_handle(HSEQUENCE S);
S32 __cdecl AIL_init_sequence(HSEQUENCE S, void *start, S32 sequence_num);
void __cdecl AIL_start_sequence(HSEQUENCE S);
void __cdecl AIL_stop_sequence(HSEQUENCE S);
void __cdecl AIL_resume_sequence(HSEQUENCE S);
void __cdecl AIL_end_sequence(HSEQUENCE S);
void __cdecl AIL_set_sequence_tempo(HSEQUENCE S, S32 tempo, S32 milliseconds);
void __cdecl AIL_set_sequence_volume(HSEQUENCE S, S32 volume, S32 milliseconds);
void __cdecl AIL_set_sequence_loop_count(HSEQUENCE S, S32 loop_count);
U32 __cdecl AIL_sequence_status(HSEQUENCE S);
S32 __cdecl AIL_sequence_tempo(HSEQUENCE S);
S32 __cdecl AIL_sequence_volume(HSEQUENCE S);
S32 __cdecl AIL_sequence_loop_count(HSEQUENCE S);
void __cdecl AIL_set_XMIDI_master_volume(HMDIDRIVER mdi, S32 master_volume);
S32 __cdecl AIL_XMIDI_master_volume(HMDIDRIVER mdi);
HMDIDRIVER __cdecl AIL_install_MDI_driver_image(void *driver_image, U32 size, IO_PARMS *IO);
S32 __cdecl AIL_MDI_driver_type(HMDIDRIVER mdi);
void __cdecl AIL_set_GTL_filename_prefix(char *prefix);
S32 __cdecl AIL_timbre_status(HMDIDRIVER mdi, S32 bank, S32 patch);
S32 __cdecl AIL_install_timbre(HMDIDRIVER mdi, S32 bank, S32 patch);
void __cdecl AIL_protect_timbre(HMDIDRIVER mdi, S32 bank, S32 patch);
void __cdecl AIL_unprotect_timbre(HMDIDRIVER mdi, S32 bank, S32 patch);
S32 __cdecl AIL_active_sequence_count(HMDIDRIVER mdi);
S32 __cdecl AIL_controller_value(HSEQUENCE S, S32 channel, S32 controller_num);
S32 __cdecl AIL_channel_notes(HSEQUENCE S, S32 channel);
void __cdecl AIL_branch_index(HSEQUENCE S, U32 marker);
AILPREFIXCB __cdecl AIL_register_prefix_callback(HSEQUENCE S, AILPREFIXCB callback);
AILTRIGGERCB __cdecl AIL_register_trigger_callback(HSEQUENCE S, AILTRIGGERCB callback);
AILSEQUENCECB __cdecl AIL_register_sequence_callback(HSEQUENCE S, AILSEQUENCECB callback);
AILBEATCB __cdecl AIL_register_beat_callback(HSEQUENCE S, AILBEATCB callback);
AILEVENTCB __cdecl AIL_register_event_callback(HMDIDRIVER mdi, AILEVENTCB callback);
AILTIMBRECB __cdecl AIL_register_timbre_callback(HMDIDRIVER mdi, AILTIMBRECB callback);
void __cdecl AIL_set_sequence_user_data(HSEQUENCE S, U32 index, S32 value);
S32 __cdecl AIL_sequence_user_data(HSEQUENCE S, U32 index);
void __cdecl AIL_register_ICA_array(HSEQUENCE S, U8 *array);
S32 __cdecl AIL_lock_channel(HMDIDRIVER mdi);
void __cdecl AIL_release_channel(HMDIDRIVER mdi, S32 channel);
void __cdecl AIL_map_sequence_channel(HSEQUENCE S, S32 seq_channel, S32 new_channel);
S32 __cdecl AIL_true_sequence_channel(HSEQUENCE S, S32 seq_channel);
void __cdecl AIL_send_channel_voice_message(HMDIDRIVER mdi, HSEQUENCE S, S32 status, S32 data_1, S32 data_2);
void __cdecl AIL_send_sysex_message(HMDIDRIVER mdi, void *buffer);
HWAVESYNTH __cdecl AIL_create_wave_synthesizer(HDIGDRIVER dig, HMDIDRIVER mdi, void *wave_lib, S32 polyphony);
void __cdecl AIL_destroy_wave_synthesizer(HWAVESYNTH W);
HREDBOOK __cdecl AIL_redbook_open(U32 which);
void __cdecl AIL_redbook_close(HREDBOOK hand);
void __cdecl AIL_redbook_eject(HREDBOOK hand);
void __cdecl AIL_redbook_retract(HREDBOOK hand);
U32 __cdecl AIL_redbook_status(HREDBOOK hand);
U32 __cdecl AIL_redbook_tracks(HREDBOOK hand);
void __cdecl AIL_redbook_track_info(HREDBOOK hand, U32 tracknum, U32 *startmsec, U32 *endmsec);
U32 __cdecl AIL_redbook_id(HREDBOOK hand);
U32 __cdecl AIL_redbook_position(HREDBOOK hand);
U32 __cdecl AIL_redbook_track(HREDBOOK hand);
U32 __cdecl AIL_redbook_play(HREDBOOK hand, U32 startmsec, U32 endmsec);
U32 __cdecl AIL_redbook_stop(HREDBOOK hand);
U32 __cdecl AIL_redbook_pause(HREDBOOK hand);
U32 __cdecl AIL_redbook_resume(HREDBOOK hand);
void __cdecl AIL_quick_shutdown(void);
HAUDIO __cdecl AIL_quick_load(char *filename);
void __cdecl AIL_quick_unload(HAUDIO audio);
S32 __cdecl AIL_quick_play(HAUDIO audio, U32 loop_count);
void __cdecl AIL_quick_halt(HAUDIO audio);
S32 __cdecl AIL_quick_status(HAUDIO audio);
HAUDIO __cdecl AIL_quick_load_and_play(char *filename, U32 loop_count, S32 wait_request);

/* C library */
unsigned strlen(char const *s);
/*
 * Inline strlen as the original compiler expanded it; Watcom 10.6's own
 * strlen intrinsic loads the -1 with "sub ecx,ecx; dec ecx" instead.
 */
#pragma aux strlen = \
    "mov ecx,-1" \
    "xor eax,eax" \
    "repne scasb" \
    "not ecx" \
    "dec ecx" \
    parm [edi] value [ecx] modify exact [eax ecx edi];

/* Watcom <i86.h> register block for int386(). */
struct DWORDREGS {
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int esi;
    unsigned int edi;
    unsigned int cflag;
};

union REGS {
    struct DWORDREGS x;
};

/* As for strlen: the original compiler's inline memset, not Watcom 10.6's. */
void memset(void *dst, int c, unsigned n);
#pragma aux memset = \
    "mov ah,al" \
    "mov bx,ax" \
    "shl eax,16" \
    "mov ax,bx" \
    "mov bl,cl" \
    "shr ecx,2" \
    "and bl,3" \
    "rep stosd" \
    "mov cl,bl" \
    "rep stosb" \
    parm [edi] [al] [ecx] modify exact [eax ebx ecx edx edi];

/* As for strlen: the original compiler's inline expansions. */
char *strcpy(char *dst, char const *src);
#pragma aux strcpy = \
    0x89 0xfa /* mov edx,edi */ \
    0x8a 0x06 /* L1: mov al,[esi] */ \
    0x46 /* inc esi */ \
    0x88 0x07 /* mov [edi],al */ \
    0x47 /* inc edi */ \
    0x3c 0x00 /* cmp al,0 */ \
    0x75 0xf6 /* jne L1 */ \
    parm [edi] [esi] value [edx] modify exact [eax edx esi edi];

char *strcat(char *dst, char const *src);
#pragma aux strcat = \
    0xfc /* cld */ \
    0xb9 0xff 0xff 0xff 0xff /* mov ecx,-1 */ \
    0x89 0xfa /* mov edx,edi */ \
    0x31 0xc0 /* xor eax,eax */ \
    0xf2 0xae /* repne scasb */ \
    0x4f /* dec edi */ \
    0xac /* L1: lodsb */ \
    0xaa /* stosb */ \
    0xa8 0xff /* test al,0ffh */ \
    0x75 0xfa /* jne L1 */ \
    parm [edi] [esi] value [edx] modify exact [eax ecx edx esi edi];

void memcpy(void *dst, void const *src, unsigned n);
#pragma aux memcpy = \
    0x88 0xcb /* mov bl,cl */ \
    0xc1 0xe9 0x02 /* shr ecx,2 */ \
    0x80 0xe3 0x03 /* and bl,3 */ \
    0xf3 0xa5 /* rep movsd */ \
    0x88 0xd9 /* mov cl,bl */ \
    0xf3 0xa4 /* rep movsb */ \
    parm [edi] [esi] [ecx] modify exact [eax ebx ecx esi edi];

/*
 * Case-insensitive compare of at most n characters, expanded inline by the
 * original compiler.
 */
char strnicmp(char const *s1, char const *s2, unsigned n);
#pragma aux strnicmp = \
    0x8a 0x06 /* L1: mov al,[esi] */ \
    0x8a 0x27 /* mov ah,[edi] */ \
    0x3c 0x61 /* cmp al,'a' */ \
    0x72 0x06 /* jb L2 */ \
    0x3c 0x7a /* cmp al,'z' */ \
    0x77 0x02 /* ja L2 */ \
    0x2c 0x20 /* sub al,20h */ \
    0x80 0xfc 0x61 /* L2: cmp ah,'a' */ \
    0x72 0x08 /* jb L3 */ \
    0x80 0xfc 0x7a /* cmp ah,'z' */ \
    0x77 0x03 /* ja L3 */ \
    0x80 0xec 0x20 /* sub ah,20h */ \
    0x38 0xe0 /* L3: cmp al,ah */ \
    0x75 0x0b /* jne L4 */ \
    0x3c 0x00 /* cmp al,0 */ \
    0x74 0x07 /* je L4 */ \
    0x49 /* dec ecx */ \
    0x74 0x04 /* je L4 */ \
    0x46 /* inc esi */ \
    0x47 /* inc edi */ \
    0xeb 0xd6 /* jmp L1 */ \
    0x0f 0x95 0xc0 /* L4: setne al */ \
    0x76 0x02 /* jbe L5 */ \
    0xf6 0xd8 /* neg al */ \
    parm [edi] [esi] [ecx] value [al] modify exact [eax ecx esi edi];

/* memfile.c */
extern void *(__pascal *AIL_mem_alloc)(U32 size);
extern void (__pascal *AIL_mem_free)(void *ptr);
extern int (__pascal *AIL_int386)(int intno, union REGS *in, union REGS *out);

void *__cdecl AIL_mem_alloc_lock(U32 size);
void __cdecl AIL_mem_free_lock(void *ptr);
S32 __cdecl AIL_mem_alloc_DOS(U32 n_paras, void **protected_ptr,
                              U32 *segment_far_ptr, U32 *selector);
void __cdecl AIL_mem_free_DOS(void *protected_ptr, U32 segment_far_ptr,
                              U32 selector);
void *__cdecl AIL_file_read(char *filename, void *dest);

#define FILE_READ_WITH_SIZE ((void *)-1)

/* Case-sensitive compare of n bytes, expanded inline by the original compiler. */
char memcmp(void const *s1, void const *s2, unsigned n);
#pragma aux memcmp = \
    0xfc /* cld */ \
    0xf3 0xa6 /* repe cmpsb */ \
    0x0f 0x95 0xc0 /* setne al */ \
    0x76 0x02 /* jbe L1 */ \
    0xf6 0xd8 /* neg al */ \
    parm [edi] [esi] [ecx] value [al] modify exact [eax ecx esi edi];

/* Case-insensitive compare, expanded inline by the original compiler. */
char stricmp(char const *s1, char const *s2);
#pragma aux stricmp = \
    0x8a 0x06 /* L1: mov al,[esi] */ \
    0x8a 0x27 /* mov ah,[edi] */ \
    0x3c 0x61 /* cmp al,'a' */ \
    0x72 0x06 /* jb L2 */ \
    0x3c 0x7a /* cmp al,'z' */ \
    0x77 0x02 /* ja L2 */ \
    0x2c 0x20 /* sub al,20h */ \
    0x80 0xfc 0x61 /* L2: cmp ah,'a' */ \
    0x72 0x08 /* jb L3 */ \
    0x80 0xfc 0x7a /* cmp ah,'z' */ \
    0x77 0x03 /* ja L3 */ \
    0x80 0xec 0x20 /* sub ah,20h */ \
    0x38 0xe0 /* L3: cmp al,ah */ \
    0x75 0x08 /* jne L4 */ \
    0x3c 0x00 /* cmp al,0 */ \
    0x74 0x04 /* je L4 */ \
    0x46 /* inc esi */ \
    0x47 /* inc edi */ \
    0xeb 0xd9 /* jmp L1 */ \
    0x0f 0x95 0xc0 /* L4: setne al */ \
    0x76 0x02 /* jbe L5 */ \
    0xf6 0xd8 /* neg al */ \
    parm [edi] [esi] value [al] modify exact [eax esi edi];

/* ailssa.asm */
void __cdecl AILSSA_VMM_lock(void);
S32 __cdecl AILSSA_merge(HSAMPLE S, S32 amount, S32 dest_offset);
void __cdecl AILSSA_flush_buffer(HDIGDRIVER dig);
void __cdecl AILSSA_DMA_copy(HDIGDRIVER dig, S32 buffer);

/* aila.asm */
void __cdecl AIL_API_lock(void);
void __cdecl AIL_API_unlock(void);
U32 __cdecl AIL_API_disable_interrupts(void);
void __cdecl AIL_API_restore_interrupts(U32 FD_register);
U32 __cdecl AIL_ms_count(void);
S32 __cdecl AIL_API_background(void);
void __cdecl AILA_startup(void);
void __cdecl AILA_shutdown(void);
S32 __cdecl AIL_fopen(char const *filename, U32 access);
S32 __cdecl AIL_fcreate(char const *filename);
S32 __cdecl AIL_fappend(char const *filename);
U32 __cdecl AIL_fread(S32 file, void *buf, U32 len);
U32 __cdecl AIL_fwrite(S32 file, void const *buf, U32 len);
S32 __cdecl AIL_fseekbegin(S32 file, S32 offset);
S32 __cdecl AIL_fseekend(S32 file, S32 offset);
void __cdecl AIL_fclose(S32 file);

S32 __cdecl AIL_vmm_lock_range(void *p1, void *p2);
S32 __cdecl AIL_vmm_unlock_range(void *p1, void *p2);
S32 __cdecl AIL_vmm_lock(void *start, U32 size);
S32 __cdecl AIL_vmm_unlock(void *start, U32 size);

/* ail.c */
extern AIL_DRIVER *AIL_driver[16];
extern S32 AIL_preference[20];
extern char AIL_error[256];
extern IO_PARMS AIL_last_IO_attempt;
extern char AIL_driver_directory[256];
extern char *(__pascal *AIL_getenv)(char const *name);

S32 __cdecl ASC_val(char *str, S32 base, char **end);
void __cdecl AIL_API_shutdown(void);
char *__cdecl AIL_API_set_driver_directory(char *dir);
S32 __cdecl AIL_API_read_INI(AIL_INI *INI, char *filename);

/* Implemented elsewhere in the library. */
void __cdecl AIL_API_digital_configuration(HDIGDRIVER dig, S32 *rate, S32 *format, char *string);
void __cdecl AIL_API_sequence_position(HSEQUENCE S, S32 *beat, S32 *measure);
S32 __cdecl AIL_API_set_preference(U32 number, S32 value);
REALFAR __cdecl AIL_API_get_real_vect(U32 vectnum);
void __cdecl AIL_API_set_real_vect(U32 vectnum, REALFAR real_ptr);
void __cdecl AIL_API_set_USE16_ISR(S32 IRQ, REALFAR real_base, U32 ISR_offset);
void __cdecl AIL_API_restore_USE16_ISR(S32 IRQ);
S32 __cdecl AIL_API_call_driver(AIL_DRIVER *drvr, S32 fn, VDI_CALL *in, VDI_CALL *out);
void __cdecl AIL_API_delay(S32 intervals);
HTIMER __cdecl AIL_API_register_timer(AILTIMERCB callback_fn);
U32 __cdecl AIL_API_set_timer_user(HTIMER timer, U32 user);
void __cdecl AIL_API_set_timer_period(HTIMER timer, U32 microseconds);
void __cdecl AIL_API_set_timer_frequency(HTIMER timer, U32 hertz);
void __cdecl AIL_API_set_timer_divisor(HTIMER timer, U32 PIT_divisor);
S32 __cdecl AIL_API_interrupt_divisor(void);
void __cdecl AIL_API_start_timer(HTIMER timer);
void __cdecl AIL_API_start_all_timers(void);
void __cdecl AIL_API_stop_timer(HTIMER timer);
void __cdecl AIL_API_stop_all_timers(void);
void __cdecl AIL_API_release_timer_handle(HTIMER timer);
void __cdecl AIL_API_release_all_timers(void);
IO_PARMS *__cdecl AIL_API_get_IO_environment(AIL_DRIVER *drvr);
AIL_DRIVER *__cdecl AIL_API_install_driver(U8 *driver_image, U32 n_bytes);
void __cdecl AIL_API_uninstall_driver(AIL_DRIVER *drvr);
S32 __cdecl AIL_API_install_DIG_INI(HDIGDRIVER *dig);
HDIGDRIVER __cdecl AIL_API_install_DIG_driver_file(char *filename, IO_PARMS *IO);
void __cdecl AIL_API_uninstall_DIG_driver(HDIGDRIVER dig);
HSAMPLE __cdecl AIL_API_allocate_sample_handle(HDIGDRIVER dig);
HSAMPLE __cdecl AIL_API_allocate_file_sample(HDIGDRIVER dig, void *file_image, S32 block);
void __cdecl AIL_API_release_sample_handle(HSAMPLE S);
void __cdecl AIL_API_init_sample(HSAMPLE S);
S32 __cdecl AIL_API_set_sample_file(HSAMPLE S, void *file_image, S32 block);
void __cdecl AIL_API_set_sample_address(HSAMPLE S, void *start, U32 len);
void __cdecl AIL_API_set_sample_type(HSAMPLE S, S32 format, U32 flags);
void __cdecl AIL_API_start_sample(HSAMPLE S);
void __cdecl AIL_API_stop_sample(HSAMPLE S);
void __cdecl AIL_API_resume_sample(HSAMPLE S);
void __cdecl AIL_API_end_sample(HSAMPLE S);
void __cdecl AIL_API_set_sample_playback_rate(HSAMPLE S, S32 playback_rate);
void __cdecl AIL_API_set_sample_volume(HSAMPLE S, S32 volume);
void __cdecl AIL_API_set_sample_pan(HSAMPLE S, S32 pan);
void __cdecl AIL_API_set_sample_loop_count(HSAMPLE S, S32 loop_count);
void __cdecl AIL_API_set_sample_loop_block(HSAMPLE S, S32 loop_start_offset, S32 loop_end_offset);
U32 __cdecl AIL_API_sample_status(HSAMPLE S);
S32 __cdecl AIL_API_sample_playback_rate(HSAMPLE S);
S32 __cdecl AIL_API_sample_volume(HSAMPLE S);
S32 __cdecl AIL_API_sample_pan(HSAMPLE S);
S32 __cdecl AIL_API_sample_loop_count(HSAMPLE S);
void __cdecl AIL_API_set_digital_master_volume(HDIGDRIVER dig, S32 master_volume);
S32 __cdecl AIL_API_digital_master_volume(HDIGDRIVER dig);
HDIGDRIVER __cdecl AIL_API_install_DIG_driver_image(void *driver_image, U32 size, IO_PARMS *IO);
S32 __cdecl AIL_API_minimum_sample_buffer_size(HDIGDRIVER dig, S32 playback_rate, S32 format);
S32 __cdecl AIL_API_sample_buffer_ready(HSAMPLE S);
void __cdecl AIL_API_load_sample_buffer(HSAMPLE S, U32 buff_num, void *buffer, U32 len);
S32 __cdecl AIL_API_sample_buffer_info(HSAMPLE S, U32 *pos0, U32 *len0, U32 *pos1, U32 *len1);
void __cdecl AIL_API_set_sample_position(HSAMPLE S, U32 pos);
U32 __cdecl AIL_API_sample_position(HSAMPLE S);
AILSAMPLECB __cdecl AIL_API_register_SOB_callback(HSAMPLE S, AILSAMPLECB SOB);
AILSAMPLECB __cdecl AIL_API_register_EOB_callback(HSAMPLE S, AILSAMPLECB EOB);
AILSAMPLECB __cdecl AIL_API_register_EOS_callback(HSAMPLE S, AILSAMPLECB EOS);
AILSAMPLECB __cdecl AIL_API_register_EOF_callback(HSAMPLE S, AILSAMPLECB EOFILE);
void __cdecl AIL_API_set_sample_user_data(HSAMPLE S, U32 index, S32 value);
S32 __cdecl AIL_API_sample_user_data(HSAMPLE S, U32 index);
S32 __cdecl AIL_API_active_sample_count(HDIGDRIVER dig);
S32 __cdecl AIL_API_install_MDI_INI(HMDIDRIVER *mdi);
HMDIDRIVER __cdecl AIL_API_install_MDI_driver_file(char *filename, IO_PARMS *IO);
void __cdecl AIL_API_uninstall_MDI_driver(HMDIDRIVER mdi);
HSEQUENCE __cdecl AIL_API_allocate_sequence_handle(HMDIDRIVER mdi);
void __cdecl AIL_API_release_sequence_handle(HSEQUENCE S);
S32 __cdecl AIL_API_init_sequence(HSEQUENCE S, void *start, S32 sequence_num);
void __cdecl AIL_API_start_sequence(HSEQUENCE S);
void __cdecl AIL_API_stop_sequence(HSEQUENCE S);
void __cdecl AIL_API_resume_sequence(HSEQUENCE S);
void __cdecl AIL_API_end_sequence(HSEQUENCE S);
void __cdecl AIL_API_set_sequence_tempo(HSEQUENCE S, S32 tempo, S32 milliseconds);
void __cdecl AIL_API_set_sequence_volume(HSEQUENCE S, S32 volume, S32 milliseconds);
void __cdecl AIL_API_set_sequence_loop_count(HSEQUENCE S, S32 loop_count);
U32 __cdecl AIL_API_sequence_status(HSEQUENCE S);
S32 __cdecl AIL_API_sequence_tempo(HSEQUENCE S);
S32 __cdecl AIL_API_sequence_volume(HSEQUENCE S);
S32 __cdecl AIL_API_sequence_loop_count(HSEQUENCE S);
void __cdecl AIL_API_set_XMIDI_master_volume(HMDIDRIVER mdi, S32 master_volume);
S32 __cdecl AIL_API_XMIDI_master_volume(HMDIDRIVER mdi);
HMDIDRIVER __cdecl AIL_API_install_MDI_driver_image(void *driver_image, U32 size, IO_PARMS *IO);
S32 __cdecl AIL_API_MDI_driver_type(HMDIDRIVER mdi);
void __cdecl AIL_API_set_GTL_filename_prefix(char *prefix);
S32 __cdecl AIL_API_timbre_status(HMDIDRIVER mdi, S32 bank, S32 patch);
S32 __cdecl AIL_API_install_timbre(HMDIDRIVER mdi, S32 bank, S32 patch);
void __cdecl AIL_API_protect_timbre(HMDIDRIVER mdi, S32 bank, S32 patch);
void __cdecl AIL_API_unprotect_timbre(HMDIDRIVER mdi, S32 bank, S32 patch);
S32 __cdecl AIL_API_active_sequence_count(HMDIDRIVER mdi);
S32 __cdecl AIL_API_controller_value(HSEQUENCE S, S32 channel, S32 controller_num);
S32 __cdecl AIL_API_channel_notes(HSEQUENCE S, S32 channel);
void __cdecl AIL_API_branch_index(HSEQUENCE S, U32 marker);
AILPREFIXCB __cdecl AIL_API_register_prefix_callback(HSEQUENCE S, AILPREFIXCB callback);
AILTRIGGERCB __cdecl AIL_API_register_trigger_callback(HSEQUENCE S, AILTRIGGERCB callback);
AILSEQUENCECB __cdecl AIL_API_register_sequence_callback(HSEQUENCE S, AILSEQUENCECB callback);
AILBEATCB __cdecl AIL_API_register_beat_callback(HSEQUENCE S, AILBEATCB callback);
AILEVENTCB __cdecl AIL_API_register_event_callback(HMDIDRIVER mdi, AILEVENTCB callback);
AILTIMBRECB __cdecl AIL_API_register_timbre_callback(HMDIDRIVER mdi, AILTIMBRECB callback);
void __cdecl AIL_API_set_sequence_user_data(HSEQUENCE S, U32 index, S32 value);
S32 __cdecl AIL_API_sequence_user_data(HSEQUENCE S, U32 index);
void __cdecl AIL_API_register_ICA_array(HSEQUENCE S, U8 *array);
S32 __cdecl AIL_API_lock_channel(HMDIDRIVER mdi);
void __cdecl AIL_API_release_channel(HMDIDRIVER mdi, S32 channel);
void __cdecl AIL_API_map_sequence_channel(HSEQUENCE S, S32 seq_channel, S32 new_channel);
S32 __cdecl AIL_API_true_sequence_channel(HSEQUENCE S, S32 seq_channel);
void __cdecl AIL_API_send_channel_voice_message(HMDIDRIVER mdi, HSEQUENCE S, S32 status, S32 data_1, S32 data_2);
void __cdecl AIL_API_send_sysex_message(HMDIDRIVER mdi, void *buffer);
HWAVESYNTH __cdecl AIL_API_create_wave_synthesizer(HDIGDRIVER dig, HMDIDRIVER mdi, void *wave_lib, S32 polyphony);
void __cdecl AIL_API_destroy_wave_synthesizer(HWAVESYNTH W);
HREDBOOK __cdecl AIL_API_redbook_open(U32 which);
void __cdecl AIL_API_redbook_close(HREDBOOK hand);
void __cdecl AIL_API_redbook_eject(HREDBOOK hand);
void __cdecl AIL_API_redbook_retract(HREDBOOK hand);
U32 __cdecl AIL_API_redbook_status(HREDBOOK hand);
U32 __cdecl AIL_API_redbook_tracks(HREDBOOK hand);
void __cdecl AIL_API_redbook_track_info(HREDBOOK hand, U32 tracknum, U32 *startmsec, U32 *endmsec);
U32 __cdecl AIL_API_redbook_id(HREDBOOK hand);
U32 __cdecl AIL_API_redbook_position(HREDBOOK hand);
U32 __cdecl AIL_API_redbook_track(HREDBOOK hand);
U32 __cdecl AIL_API_redbook_play(HREDBOOK hand, U32 startmsec, U32 endmsec);
U32 __cdecl AIL_API_redbook_stop(HREDBOOK hand);
U32 __cdecl AIL_API_redbook_pause(HREDBOOK hand);
U32 __cdecl AIL_API_redbook_resume(HREDBOOK hand);
void __cdecl AIL_API_quick_shutdown(void);
HAUDIO __cdecl AIL_API_quick_load(char *filename);
void __cdecl AIL_API_quick_unload(HAUDIO audio);
S32 __cdecl AIL_API_quick_play(HAUDIO audio, U32 loop_count);
void __cdecl AIL_API_quick_halt(HAUDIO audio);
S32 __cdecl AIL_API_quick_status(HAUDIO audio);
HAUDIO __cdecl AIL_API_quick_load_and_play(char *filename, U32 loop_count, S32 wait_request);

#pragma aux (__cdecl) AIL_API_register_EOF_callback "_AIL_API_register_EOF_callback";
#pragma aux (__cdecl) AIL_API_send_sysex_message "_AIL_API_send_sysex_message";
#pragma aux (__cdecl) AIL_API_quick_shutdown "fn_000e6e50_";
#pragma aux (__cdecl) AIL_API_quick_halt "fn_000e7330_";
#pragma aux (__cdecl) AIL_API_quick_status "_AIL_API_quick_status";
#pragma aux (__cdecl) AIL_API_quick_load_and_play "fn_000e7380_";

#endif
