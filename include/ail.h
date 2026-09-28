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

typedef struct _SAMPLE {
    U8 unknown0[0x4c];
    S32 playback_rate;
    S32 volume;
} SAMPLE, *HSAMPLE;

typedef U32 REALFAR;
typedef S32 HTIMER;
typedef U32 HREDBOOK;

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

typedef struct _AIL_DRIVER AIL_DRIVER;
typedef struct _VDI_CALL VDI_CALL;
typedef struct _DIG_DRIVER *HDIGDRIVER;
typedef struct _MDI_DRIVER *HMDIDRIVER;
typedef struct _SEQUENCE *HSEQUENCE;
typedef struct _WAVE_SYNTH *HWAVESYNTH;
typedef struct _AUDIO_TYPE *HAUDIO;

typedef void (*AILTIMERCB)(U32 user);
typedef void (*AILSAMPLECB)(HSAMPLE sample);
typedef S32 (*AILPREFIXCB)(HSEQUENCE seq, S32 log, S32 data);
typedef void (*AILTRIGGERCB)(HSEQUENCE seq, S32 log, S32 data);
typedef void (*AILSEQUENCECB)(HSEQUENCE seq);
typedef void (*AILBEATCB)(HMDIDRIVER mdi, HSEQUENCE seq, S32 beat, S32 measure);
typedef S32 (*AILEVENTCB)(HMDIDRIVER mdi, HSEQUENCE seq, S32 status, S32 data_1,
                          S32 data_2);
typedef S32 (*AILTIMBRECB)(HMDIDRIVER mdi, S32 bank, S32 patch);

/* aildebug.c */
extern S32 locked;
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

/* C library */
unsigned strlen(char const *s);
#pragma aux strlen = \
    "mov ecx,-1" \
    "xor eax,eax" \
    "repne scasb" \
    "not ecx" \
    "dec ecx" \
    parm [edi] value [ecx] modify exact [eax ecx edi];

/* aila.asm */
void __cdecl AIL_API_lock(void);
void __cdecl AIL_API_unlock(void);
U32 __cdecl AIL_API_disable_interrupts(void);
void __cdecl AIL_API_restore_interrupts(U32 FD_register);
/* config/functions.txt names AIL_API_restore_interrupts _AIL_switch_stack. */
#pragma aux (__cdecl) AIL_API_restore_interrupts "_AIL_switch_stack";
U32 __cdecl AIL_ms_count(void);
S32 __cdecl AIL_API_background(void);
S32 __cdecl AIL_fwrite(S32 file, void const *buf, U32 len);
void __cdecl AIL_fclose(S32 file);

/* memfile.c */
S32 __cdecl AIL_vmm_lock_range(void *start, void *end);
S32 __cdecl AIL_vmm_unlock_range(void *start, void *end);
S32 __cdecl AIL_vmm_lock(void *p, U32 size);

/* ail.c */
void __cdecl AIL_API_shutdown(void);
S32 __cdecl AIL_API_set_driver_directory(char const *dir);
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

#pragma aux (__cdecl) AIL_API_register_EOF_callback "fn_000e1e80_";
#pragma aux (__cdecl) AIL_API_send_sysex_message "AILXMIDI_end_";
#pragma aux (__cdecl) AIL_API_redbook_track "rbpos_";
#pragma aux (__cdecl) AIL_API_quick_shutdown "fn_000e6e50_";
#pragma aux (__cdecl) AIL_API_quick_halt "fn_000e7330_";
#pragma aux (__cdecl) AIL_API_quick_status "fn_000e7410_";
#pragma aux (__cdecl) AIL_API_quick_load_and_play "fn_000e7380_";

#endif
