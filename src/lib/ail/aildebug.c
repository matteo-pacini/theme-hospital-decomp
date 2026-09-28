#include "ail.h"

#define START                                                            \
    AIL_indent++;                                                        \
    if ((AIL_debug && (AIL_indent == 1 || AIL_sys_debug)                 \
         && !AIL_API_background() && TIME_WRITE()) ? 1 : 0)

#define RESULT                                                           \
    if (AIL_debug && (AIL_indent == 1 || AIL_sys_debug)                  \
        && !AIL_API_background())

#define INDENT AIL_fwrite(AIL_debugfile, AIL_indent_str, AIL_indent + 14)

#define END AIL_indent--

extern char D_00122f74[];
extern char D_00122f84[];
extern char D_00122f94[];
extern char D_00122fb0[];
extern char D_00122fc0[];
extern char D_00122fd4[];
extern char D_00122fe8[];
extern char D_00123008[];
extern char D_001230d4[];
extern char D_001230f0[];
extern char D_00123100[];
extern char D_00123110[];
extern char D_00123120[];
extern char D_00123130[];
extern char D_00123140[];
extern char D_00123678[];
extern char D_001236b0[];
extern char D_00123cd0[];
extern char D_00123cf8[];
extern char D_00123024[];
extern char D_00123040[];
extern char D_00123060[];
extern char D_00123080[];
extern char D_0012309c[];
extern char D_001230c4[];
extern char D_00123150[];
extern char D_0012316c[];
extern char D_00123188[];
extern char D_001231a8[];
extern char D_001231cc[];
extern char D_001231ec[];
extern char D_00123208[];
extern char D_00123220[];
extern char D_0012323c[];
extern char D_00123254[];
extern char D_0012326c[];
extern char D_0012328c[];
extern char D_001232a8[];
extern char D_001232c8[];
extern char D_001232e8[];
extern char D_00123308[];
extern char D_00123324[];
extern char D_0012334c[];
extern char D_00123370[];
extern char D_00123394[];
extern char D_001233c0[];
extern char D_001233e4[];
extern char D_001233fc[];
extern char D_00123420[];
extern char D_00123448[];
extern char D_0012346c[];
extern char D_00123488[];
extern char D_001234a0[];
extern char D_001234bc[];
extern char D_001234d4[];
extern char D_001234fc[];
extern char D_00123520[];
extern char D_00123540[];
extern char D_00123568[];
extern char D_00123594[];
extern char D_001235b0[];
extern char D_001235d4[];
extern char D_001235f0[];
extern char D_00123608[];
extern char D_00123628[];
extern char D_00123654[];
extern char D_001236c8[];
extern char D_001236f8[];
extern char D_00123728[];
extern char D_00123748[];
extern char D_00123774[];
extern char D_001237a8[];
extern char D_001237cc[];
extern char D_001237e8[];
extern char D_00123810[];
extern char D_00123838[];
extern char D_00123860[];
extern char D_00123888[];
extern char D_001238b0[];
extern char D_001238d0[];
extern char D_001238f0[];
extern char D_0012390c[];
extern char D_00123934[];
extern char D_00123958[];
extern char D_00123980[];
extern char D_001239a4[];
extern char D_001239c8[];
extern char D_001239e4[];
extern char D_00123a00[];
extern char D_00123a1c[];
extern char D_00123a38[];
extern char D_00123a60[];
extern char D_00123a88[];
extern char D_00123ab0[];
extern char D_00123acc[];
extern char D_00123ae8[];
extern char D_00123b04[];
extern char D_00123b24[];
extern char D_00123b4c[];
extern char D_00123b6c[];
extern char D_00123b9c[];
extern char D_00123bb8[];
extern char D_00123bdc[];
extern char D_00123bfc[];
extern char D_00123c20[];
extern char D_00123c44[];
extern char D_00123c68[];
extern char D_00123c8c[];
extern char D_00123cb0[];
extern char D_00123d0c[];
extern char D_00123d28[];
extern char D_00123d54[];
extern char D_00123d80[];
extern char D_00123dac[];
extern char D_00123dd4[];
extern char D_00123e00[];
extern char D_00123e2c[];
extern char D_00123e58[];
extern char D_00123e7c[];
extern char D_00123ea0[];
extern char D_00123ebc[];
extern char D_00123edc[];
extern char D_00123f04[];
extern char D_00123f2c[];
extern char D_00123f68[];
extern char D_00123f8c[];
extern char D_00123fc0[];
extern char D_00123fe8[];
extern char D_00124000[];
extern char D_0012401c[];
extern char D_00124038[];
extern char D_00124058[];
extern char D_00124074[];
extern char D_00124090[];
extern char D_001240c4[];
extern char D_001240dc[];
extern char D_001240fc[];
extern char D_00124118[];
extern char D_0012413c[];
extern char D_00124158[];
extern char D_00124174[];
extern char D_00124190[];
extern char D_001241a8[];
extern char D_001241c0[];
extern char D_001241dc[];
extern char D_001241f8[];
extern char D_00124210[];
extern char D_0012422c[];

void __pascal AILDEBUG_START(void);
void __pascal AILDEBUG_END(void);
#pragma aux (__pascal) AILDEBUG_END "fn_000c82f0_"

// FUNCTION: 0x000c24c0
void __pascal AILDEBUG_START(void)
{
    if (!locked) {
        AIL_vmm_lock_range(AILDEBUG_START, AILDEBUG_END);
        AIL_vmm_lock(&AIL_indent, sizeof(AIL_indent));
        AIL_vmm_lock(&AIL_debug, sizeof(AIL_debug));
        AIL_vmm_lock(&AIL_sys_debug, sizeof(AIL_sys_debug));
        AIL_vmm_lock(&AIL_debugfile, sizeof(AIL_debugfile));
        AIL_vmm_lock(&AIL_started, sizeof(AIL_started));
        AIL_vmm_lock(&AIL_starttime, sizeof(AIL_starttime));
        locked = 1;
    }
}

// FUNCTION: 0x000c2550
void __pascal AIL_DEBUGOUTSTR(char const *str)
{
    AIL_fwrite(AIL_debugfile, str, strlen(str));
}

// FUNCTION: 0x000c2580
void outresint(S32 value)
{
    char buf[64];
    S32 args[1];

    args[0] = value;
    AIL_sprintf(buf, D_00122f74, 1, args);
    AIL_DEBUGOUTSTR(buf);
}

// FUNCTION: 0x000c25b0
void outreshex(U32 value)
{
    char buf[64];
    S32 args[1];

    args[0] = value;
    AIL_sprintf(buf, D_00122f84, 1, args);
    AIL_DEBUGOUTSTR(buf);
}

// FUNCTION: 0x000c25e0
void oprintf1(char const *fmt, S32 a1)
{
    char buf[128];
    S32 args[1];

    args[0] = a1;
    AIL_sprintf(buf, fmt, 1, args);
    AIL_DEBUGOUTSTR(buf);
}

// FUNCTION: 0x000c2620
void oprintf2(char const *fmt, S32 a1, S32 a2)
{
    char buf[128];
    S32 args[2];

    args[0] = a1;
    args[1] = a2;
    AIL_sprintf(buf, fmt, 2, args);
    AIL_DEBUGOUTSTR(buf);
}

// FUNCTION: 0x000c2660
void oprintf3(char const *fmt, S32 a1, S32 a2, S32 a3)
{
    char buf[128];
    S32 args[3];

    args[0] = a1;
    args[1] = a2;
    args[2] = a3;
    AIL_sprintf(buf, fmt, 3, args);
    AIL_DEBUGOUTSTR(buf);
}

// FUNCTION: 0x000c26a0
void oprintf4(char const *fmt, S32 a1, S32 a2, S32 a3, S32 a4)
{
    char buf[128];
    S32 args[4];

    args[0] = a1;
    args[1] = a2;
    args[2] = a3;
    args[3] = a4;
    AIL_sprintf(buf, fmt, 4, args);
    AIL_DEBUGOUTSTR(buf);
}

// FUNCTION: 0x000c26f0
void oprintf5(char const *fmt, S32 a1, S32 a2, S32 a3, S32 a4, S32 a5)
{
    char buf[128];
    S32 args[5];

    args[0] = a1;
    args[1] = a2;
    args[2] = a3;
    args[3] = a4;
    args[4] = a5;
    AIL_sprintf(buf, fmt, 5, args);
    AIL_DEBUGOUTSTR(buf);
}

// FUNCTION: 0x000c2750
S32 __pascal TIME_WRITE(void)
{
    U32 t;
    U16 h, m, sec, ms;

    t = AIL_ms_count() - AIL_starttime;
    h = t / 3600000;
    m = t / 60000 % 60;
    sec = t / 1000 % 60;
    ms = t % 1000;
    if (AIL_indent == 1) {
        oprintf4(D_00122f94, h, m, sec, ms);
    } else {
        AIL_DEBUGOUTSTR(D_00122fb0);
    }
    if (AIL_indent > 1) {
        AIL_fwrite(AIL_debugfile, D_00122fc0, AIL_indent - 1);
    }
    return 1;
}

// FUNCTION: 0x000c2810
void __cdecl AIL_shutdown(void)
{
    if (AIL_started) {
        START
        {
            AIL_DEBUGOUTSTR(D_00122fd4);
        }
        AIL_API_shutdown();
        RESULT
        {
            AIL_fclose(AIL_debugfile);
        }
        END;
        AIL_started = 0;
    }
}

// FUNCTION: 0x000c28c0
char *__cdecl AIL_set_driver_directory(char *dir)
{
    char *result;

    START
    {
        oprintf1(D_00122fe8, (S32)dir);
    }
    result = AIL_API_set_driver_directory(dir);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c2980
S32 __cdecl AIL_set_preference(U32 number, S32 value)
{
    S32 result;

    START
    {
        oprintf2(D_00123008, number, value);
    }
    result = AIL_API_set_preference(number, value);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c2a50
void __cdecl AIL_lock(void)
{
    AIL_API_lock();
}

// FUNCTION: 0x000c2a60
void __cdecl AIL_unlock(void)
{
    AIL_API_unlock();
}

// FUNCTION: 0x000c2a70
REALFAR __cdecl AIL_get_real_vect(U32 vectnum)
{
    REALFAR result;

    START
    {
        oprintf1(D_00123024, vectnum);
    }
    result = AIL_API_get_real_vect(vectnum);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c2b30
void __cdecl AIL_set_real_vect(U32 vectnum, REALFAR real_ptr)
{
    START
    {
        oprintf2(D_00123040, vectnum, (S32)real_ptr);
    }
    AIL_API_set_real_vect(vectnum, real_ptr);
    END;
}

// FUNCTION: 0x000c2bb0
void __cdecl AIL_set_USE16_ISR(S32 IRQ, REALFAR real_base, U32 ISR_offset)
{
    START
    {
        oprintf3(D_00123060, IRQ, (S32)real_base, ISR_offset);
    }
    AIL_API_set_USE16_ISR(IRQ, real_base, ISR_offset);
    END;
}

// FUNCTION: 0x000c2c40
void __cdecl AIL_restore_USE16_ISR(S32 IRQ)
{
    START
    {
        oprintf1(D_00123080, IRQ);
    }
    AIL_API_restore_USE16_ISR(IRQ);
    END;
}

// FUNCTION: 0x000c2cc0
U32 __cdecl AIL_disable_interrupts(void)
{
    return AIL_API_disable_interrupts();
}

// FUNCTION: 0x000c2cd0
void __cdecl AIL_restore_interrupts(U32 FD_register)
{
    AIL_API_restore_interrupts(FD_register);
}

// FUNCTION: 0x000c2ce0
S32 __cdecl AIL_call_driver(AIL_DRIVER *drvr, S32 fn, VDI_CALL *in, VDI_CALL *out)
{
    S32 result;

    START
    {
        oprintf4(D_0012309c, (S32)drvr, fn, (S32)in, (S32)out);
    }
    result = AIL_API_call_driver(drvr, fn, in, out);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c2dc0
void __cdecl AIL_delay(S32 intervals)
{
    START
    {
        oprintf1(D_001230c4, intervals);
    }
    AIL_API_delay(intervals);
    END;
}

// FUNCTION: 0x000c2e40
S32 __cdecl AIL_background(void)
{
    return AIL_API_background();
}

// FUNCTION: 0x000c2e50
S32 __cdecl AIL_read_INI(AIL_INI *INI, char *filename)
{
    S32 result;

    START
    {
        oprintf2(D_001230d4, (S32)INI, (S32)filename);
    }
    result = AIL_API_read_INI(INI, filename);
    if (result) {
        RESULT
        {
            INDENT;
            oprintf1(D_001230f0, (S32)INI->driver_name);
            INDENT;
            oprintf1(D_00123100, (S32)INI->device_name);
            INDENT;
            oprintf1(D_00123110, INI->IO.IO);
            INDENT;
            oprintf1(D_00123120, INI->IO.IRQ);
            INDENT;
            oprintf1(D_00123130, INI->IO.DMA_8_bit);
            INDENT;
            oprintf1(D_00123140, INI->IO.DMA_16_bit);
        }
    } else {
        RESULT
        {
            INDENT;
            outresint(result);
        }
    }
    END;
    return result;
}

// FUNCTION: 0x000c3070
HTIMER __cdecl AIL_register_timer(AILTIMERCB callback_fn)
{
    HTIMER result;

    START
    {
        oprintf1(D_00123150, (S32)callback_fn);
    }
    result = AIL_API_register_timer(callback_fn);
    RESULT
    {
        INDENT;
        outresint((S32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c3130
U32 __cdecl AIL_set_timer_user(HTIMER timer, U32 user)
{
    U32 result;

    START
    {
        oprintf2(D_0012316c, (S32)timer, user);
    }
    result = AIL_API_set_timer_user(timer, user);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c3200
void __cdecl AIL_set_timer_period(HTIMER timer, U32 microseconds)
{
    START
    {
        oprintf2(D_00123188, (S32)timer, microseconds);
    }
    AIL_API_set_timer_period(timer, microseconds);
    END;
}

// FUNCTION: 0x000c3280
void __cdecl AIL_set_timer_frequency(HTIMER timer, U32 hertz)
{
    START
    {
        oprintf2(D_001231a8, (S32)timer, hertz);
    }
    AIL_API_set_timer_frequency(timer, hertz);
    END;
}

// FUNCTION: 0x000c3300
void __cdecl AIL_set_timer_divisor(HTIMER timer, U32 PIT_divisor)
{
    START
    {
        oprintf2(D_001231cc, (S32)timer, PIT_divisor);
    }
    AIL_API_set_timer_divisor(timer, PIT_divisor);
    END;
}

// FUNCTION: 0x000c3380
S32 __cdecl AIL_interrupt_divisor(void)
{
    S32 result;

    START
    {
        AIL_DEBUGOUTSTR(D_001231ec);
    }
    result = AIL_API_interrupt_divisor();
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c3440
void __cdecl AIL_start_timer(HTIMER timer)
{
    START
    {
        oprintf1(D_00123208, (S32)timer);
    }
    AIL_API_start_timer(timer);
    END;
}

// FUNCTION: 0x000c34c0
void __cdecl AIL_start_all_timers(void)
{
    START
    {
        AIL_DEBUGOUTSTR(D_00123220);
    }
    AIL_API_start_all_timers();
    END;
}

// FUNCTION: 0x000c3520
void __cdecl AIL_stop_timer(HTIMER timer)
{
    START
    {
        oprintf1(D_0012323c, (S32)timer);
    }
    AIL_API_stop_timer(timer);
    END;
}

// FUNCTION: 0x000c35a0
void __cdecl AIL_stop_all_timers(void)
{
    START
    {
        AIL_DEBUGOUTSTR(D_00123254);
    }
    AIL_API_stop_all_timers();
    END;
}

// FUNCTION: 0x000c3600
void __cdecl AIL_release_timer_handle(HTIMER timer)
{
    START
    {
        oprintf1(D_0012326c, (S32)timer);
    }
    AIL_API_release_timer_handle(timer);
    END;
}

// FUNCTION: 0x000c3680
void __cdecl AIL_release_all_timers(void)
{
    START
    {
        AIL_DEBUGOUTSTR(D_0012328c);
    }
    AIL_API_release_all_timers();
    END;
}

// FUNCTION: 0x000c36e0
IO_PARMS *__cdecl AIL_get_IO_environment(AIL_DRIVER *drvr)
{
    IO_PARMS * result;

    START
    {
        oprintf1(D_001232a8, (S32)drvr);
    }
    result = AIL_API_get_IO_environment(drvr);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c37a0
AIL_DRIVER *__cdecl AIL_install_driver(U8 *driver_image, U32 n_bytes)
{
    AIL_DRIVER * result;

    START
    {
        oprintf2(D_001232c8, (S32)driver_image, n_bytes);
    }
    result = AIL_API_install_driver(driver_image, n_bytes);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c3870
void __cdecl AIL_uninstall_driver(AIL_DRIVER *drvr)
{
    START
    {
        oprintf1(D_001232e8, (S32)drvr);
    }
    AIL_API_uninstall_driver(drvr);
    END;
}

// FUNCTION: 0x000c38f0
S32 __cdecl AIL_install_DIG_INI(HDIGDRIVER *dig)
{
    S32 result;

    START
    {
        oprintf1(D_00123308, (S32)dig);
    }
    result = AIL_API_install_DIG_INI(dig);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c39b0
HDIGDRIVER __cdecl AIL_install_DIG_driver_file(char *filename, IO_PARMS *IO)
{
    HDIGDRIVER result;

    START
    {
        oprintf2(D_00123324, (S32)filename, (S32)IO);
    }
    result = AIL_API_install_DIG_driver_file(filename, IO);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c3a80
void __cdecl AIL_uninstall_DIG_driver(HDIGDRIVER dig)
{
    START
    {
        oprintf1(D_0012334c, (S32)dig);
    }
    AIL_API_uninstall_DIG_driver(dig);
    END;
}

// FUNCTION: 0x000c3b00
HSAMPLE __cdecl AIL_allocate_sample_handle(HDIGDRIVER dig)
{
    HSAMPLE result;

    START
    {
        oprintf1(D_00123370, (S32)dig);
    }
    result = AIL_API_allocate_sample_handle(dig);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c3bc0
HSAMPLE __cdecl AIL_allocate_file_sample(HDIGDRIVER dig, void *file_image, S32 block)
{
    HSAMPLE result;

    START
    {
        oprintf3(D_00123394, (S32)dig, (S32)file_image, block);
    }
    result = AIL_API_allocate_file_sample(dig, file_image, block);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c3c90
void __cdecl AIL_release_sample_handle(HSAMPLE S)
{
    START
    {
        oprintf1(D_001233c0, (S32)S);
    }
    AIL_API_release_sample_handle(S);
    END;
}

// FUNCTION: 0x000c3d10
void __cdecl AIL_init_sample(HSAMPLE S)
{
    START
    {
        oprintf1(D_001233e4, (S32)S);
    }
    AIL_API_init_sample(S);
    END;
}

// FUNCTION: 0x000c3d90
S32 __cdecl AIL_set_sample_file(HSAMPLE S, void *file_image, S32 block)
{
    S32 result;

    START
    {
        oprintf3(D_001233fc, (S32)S, (S32)file_image, block);
    }
    result = AIL_API_set_sample_file(S, file_image, block);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c3e60
void __cdecl AIL_set_sample_address(HSAMPLE S, void *start, U32 len)
{
    START
    {
        oprintf3(D_00123420, (S32)S, (S32)start, len);
    }
    AIL_API_set_sample_address(S, start, len);
    END;
}

// FUNCTION: 0x000c3ef0
void __cdecl AIL_set_sample_type(HSAMPLE S, S32 format, U32 flags)
{
    START
    {
        oprintf3(D_00123448, (S32)S, format, flags);
    }
    AIL_API_set_sample_type(S, format, flags);
    END;
}

// FUNCTION: 0x000c3f80
void __cdecl AIL_start_sample(HSAMPLE S)
{
    START
    {
        oprintf1(D_0012346c, (S32)S);
    }
    AIL_API_start_sample(S);
    END;
}

// FUNCTION: 0x000c4000
void __cdecl AIL_stop_sample(HSAMPLE S)
{
    START
    {
        oprintf1(D_00123488, (S32)S);
    }
    AIL_API_stop_sample(S);
    END;
}

// FUNCTION: 0x000c4080
void __cdecl AIL_resume_sample(HSAMPLE S)
{
    START
    {
        oprintf1(D_001234a0, (S32)S);
    }
    AIL_API_resume_sample(S);
    END;
}

// FUNCTION: 0x000c4100
void __cdecl AIL_end_sample(HSAMPLE S)
{
    START
    {
        oprintf1(D_001234bc, (S32)S);
    }
    AIL_API_end_sample(S);
    END;
}

// FUNCTION: 0x000c4180
void __cdecl AIL_set_sample_playback_rate(HSAMPLE S, S32 playback_rate)
{
    START
    {
        oprintf2(D_001234d4, (S32)S, playback_rate);
    }
    AIL_API_set_sample_playback_rate(S, playback_rate);
    END;
}

// FUNCTION: 0x000c4200
void __cdecl AIL_set_sample_volume(HSAMPLE S, S32 volume)
{
    START
    {
        oprintf2(D_001234fc, (S32)S, volume);
    }
    AIL_API_set_sample_volume(S, volume);
    END;
}

// FUNCTION: 0x000c4280
void __cdecl AIL_set_sample_pan(HSAMPLE S, S32 pan)
{
    START
    {
        oprintf2(D_00123520, (S32)S, pan);
    }
    AIL_API_set_sample_pan(S, pan);
    END;
}

// FUNCTION: 0x000c4300
void __cdecl AIL_set_sample_loop_count(HSAMPLE S, S32 loop_count)
{
    START
    {
        oprintf2(D_00123540, (S32)S, loop_count);
    }
    AIL_API_set_sample_loop_count(S, loop_count);
    END;
}

// FUNCTION: 0x000c4380
void __cdecl AIL_set_sample_loop_block(HSAMPLE S, S32 loop_start_offset, S32 loop_end_offset)
{
    START
    {
        oprintf3(D_00123568, (S32)S, loop_start_offset, loop_end_offset);
    }
    AIL_API_set_sample_loop_block(S, loop_start_offset, loop_end_offset);
    END;
}

// FUNCTION: 0x000c4410
U32 __cdecl AIL_sample_status(HSAMPLE S)
{
    U32 result;

    START
    {
        oprintf1(D_00123594, (S32)S);
    }
    result = AIL_API_sample_status(S);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c44d0
S32 __cdecl AIL_sample_playback_rate(HSAMPLE S)
{
    S32 result;

    START
    {
        oprintf1(D_001235b0, (S32)S);
    }
    result = AIL_API_sample_playback_rate(S);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c4590
S32 __cdecl AIL_sample_volume(HSAMPLE S)
{
    S32 result;

    START
    {
        oprintf1(D_001235d4, (S32)S);
    }
    result = AIL_API_sample_volume(S);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c4650
S32 __cdecl AIL_sample_pan(HSAMPLE S)
{
    S32 result;

    START
    {
        oprintf1(D_001235f0, (S32)S);
    }
    result = AIL_API_sample_pan(S);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c4710
S32 __cdecl AIL_sample_loop_count(HSAMPLE S)
{
    S32 result;

    START
    {
        oprintf1(D_00123608, (S32)S);
    }
    result = AIL_API_sample_loop_count(S);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c47d0
void __cdecl AIL_set_digital_master_volume(HDIGDRIVER dig, S32 master_volume)
{
    START
    {
        oprintf2(D_00123628, (S32)dig, master_volume);
    }
    AIL_API_set_digital_master_volume(dig, master_volume);
    END;
}

// FUNCTION: 0x000c4850
S32 __cdecl AIL_digital_master_volume(HDIGDRIVER dig)
{
    S32 result;

    START
    {
        oprintf1(D_00123654, (S32)dig);
    }
    result = AIL_API_digital_master_volume(dig);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c4910
void __cdecl AIL_digital_configuration(HDIGDRIVER dig, S32 *rate, S32 *format,
                                       char *string)
{
    START
    {
        oprintf4(D_00123678, (S32)dig, (S32)rate, (S32)format, (S32)string);
    }
    AIL_API_digital_configuration(dig, rate, format, string);
    RESULT
    {
        INDENT;
        oprintf3(D_001236b0, *rate, *format, *string);
    }
    END;
}

// FUNCTION: 0x000c49f0
HDIGDRIVER __cdecl AIL_install_DIG_driver_image(void *driver_image, U32 size, IO_PARMS *IO)
{
    HDIGDRIVER result;

    START
    {
        oprintf3(D_001236c8, (S32)driver_image, size, (S32)IO);
    }
    result = AIL_API_install_DIG_driver_image(driver_image, size, IO);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c4ac0
S32 __cdecl AIL_minimum_sample_buffer_size(HDIGDRIVER dig, S32 playback_rate, S32 format)
{
    S32 result;

    START
    {
        oprintf3(D_001236f8, (S32)dig, playback_rate, format);
    }
    result = AIL_API_minimum_sample_buffer_size(dig, playback_rate, format);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c4b90
S32 __cdecl AIL_sample_buffer_ready(HSAMPLE S)
{
    S32 result;

    START
    {
        oprintf1(D_00123728, (S32)S);
    }
    result = AIL_API_sample_buffer_ready(S);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c4c50
void __cdecl AIL_load_sample_buffer(HSAMPLE S, U32 buff_num, void *buffer, U32 len)
{
    START
    {
        oprintf4(D_00123748, (S32)S, buff_num, (S32)buffer, len);
    }
    AIL_API_load_sample_buffer(S, buff_num, buffer, len);
    END;
}

// FUNCTION: 0x000c4ce0
S32 __cdecl AIL_sample_buffer_info(HSAMPLE S, U32 *pos0, U32 *len0, U32 *pos1, U32 *len1)
{
    S32 result;

    START
    {
        oprintf5(D_00123774, (S32)S, (S32)pos0, (S32)len0, (S32)pos1, (S32)len1);
    }
    result = AIL_API_sample_buffer_info(S, pos0, len0, pos1, len1);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c4dd0
void __cdecl AIL_set_sample_position(HSAMPLE S, U32 pos)
{
    START
    {
        oprintf2(D_001237a8, (S32)S, pos);
    }
    AIL_API_set_sample_position(S, pos);
    END;
}

// FUNCTION: 0x000c4e50
U32 __cdecl AIL_sample_position(HSAMPLE S)
{
    U32 result;

    START
    {
        oprintf1(D_001237cc, (S32)S);
    }
    result = AIL_API_sample_position(S);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c4f10
AILSAMPLECB __cdecl AIL_register_SOB_callback(HSAMPLE S, AILSAMPLECB SOB)
{
    AILSAMPLECB result;

    START
    {
        oprintf2(D_001237e8, (S32)S, (S32)SOB);
    }
    result = AIL_API_register_SOB_callback(S, SOB);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c4fe0
AILSAMPLECB __cdecl AIL_register_EOB_callback(HSAMPLE S, AILSAMPLECB EOB)
{
    AILSAMPLECB result;

    START
    {
        oprintf2(D_00123810, (S32)S, (S32)EOB);
    }
    result = AIL_API_register_EOB_callback(S, EOB);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c50b0
AILSAMPLECB __cdecl AIL_register_EOS_callback(HSAMPLE S, AILSAMPLECB EOS)
{
    AILSAMPLECB result;

    START
    {
        oprintf2(D_00123838, (S32)S, (S32)EOS);
    }
    result = AIL_API_register_EOS_callback(S, EOS);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c5180
AILSAMPLECB __cdecl AIL_register_EOF_callback(HSAMPLE S, AILSAMPLECB EOFILE)
{
    AILSAMPLECB result;

    START
    {
        oprintf2(D_00123860, (S32)S, (S32)EOFILE);
    }
    result = AIL_API_register_EOF_callback(S, EOFILE);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c5250
void __cdecl AIL_set_sample_user_data(HSAMPLE S, U32 index, S32 value)
{
    START
    {
        oprintf3(D_00123888, (S32)S, index, value);
    }
    AIL_API_set_sample_user_data(S, index, value);
    END;
}

// FUNCTION: 0x000c52e0
S32 __cdecl AIL_sample_user_data(HSAMPLE S, U32 index)
{
    S32 result;

    START
    {
        oprintf2(D_001238b0, (S32)S, index);
    }
    result = AIL_API_sample_user_data(S, index);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c53b0
S32 __cdecl AIL_active_sample_count(HDIGDRIVER dig)
{
    S32 result;

    START
    {
        oprintf1(D_001238d0, (S32)dig);
    }
    result = AIL_API_active_sample_count(dig);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c5470
S32 __cdecl AIL_install_MDI_INI(HMDIDRIVER *mdi)
{
    S32 result;

    START
    {
        oprintf1(D_001238f0, (S32)mdi);
    }
    result = AIL_API_install_MDI_INI(mdi);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c5530
HMDIDRIVER __cdecl AIL_install_MDI_driver_file(char *filename, IO_PARMS *IO)
{
    HMDIDRIVER result;

    START
    {
        oprintf2(D_0012390c, (S32)filename, (S32)IO);
    }
    result = AIL_API_install_MDI_driver_file(filename, IO);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c5600
void __cdecl AIL_uninstall_MDI_driver(HMDIDRIVER mdi)
{
    START
    {
        oprintf1(D_00123934, (S32)mdi);
    }
    AIL_API_uninstall_MDI_driver(mdi);
    END;
}

// FUNCTION: 0x000c5680
HSEQUENCE __cdecl AIL_allocate_sequence_handle(HMDIDRIVER mdi)
{
    HSEQUENCE result;

    START
    {
        oprintf1(D_00123958, (S32)mdi);
    }
    result = AIL_API_allocate_sequence_handle(mdi);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c5740
void __cdecl AIL_release_sequence_handle(HSEQUENCE S)
{
    START
    {
        oprintf1(D_00123980, (S32)S);
    }
    AIL_API_release_sequence_handle(S);
    END;
}

// FUNCTION: 0x000c57c0
S32 __cdecl AIL_init_sequence(HSEQUENCE S, void *start, S32 sequence_num)
{
    S32 result;

    START
    {
        oprintf3(D_001239a4, (S32)S, (S32)start, sequence_num);
    }
    result = AIL_API_init_sequence(S, start, sequence_num);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c5890
void __cdecl AIL_start_sequence(HSEQUENCE S)
{
    START
    {
        oprintf1(D_001239c8, (S32)S);
    }
    AIL_API_start_sequence(S);
    END;
}

// FUNCTION: 0x000c5910
void __cdecl AIL_stop_sequence(HSEQUENCE S)
{
    START
    {
        oprintf1(D_001239e4, (S32)S);
    }
    AIL_API_stop_sequence(S);
    END;
}

// FUNCTION: 0x000c5990
void __cdecl AIL_resume_sequence(HSEQUENCE S)
{
    START
    {
        oprintf1(D_00123a00, (S32)S);
    }
    AIL_API_resume_sequence(S);
    END;
}

// FUNCTION: 0x000c5a10
void __cdecl AIL_end_sequence(HSEQUENCE S)
{
    START
    {
        oprintf1(D_00123a1c, (S32)S);
    }
    AIL_API_end_sequence(S);
    END;
}

// FUNCTION: 0x000c5a90
void __cdecl AIL_set_sequence_tempo(HSEQUENCE S, S32 tempo, S32 milliseconds)
{
    START
    {
        oprintf3(D_00123a38, (S32)S, tempo, milliseconds);
    }
    AIL_API_set_sequence_tempo(S, tempo, milliseconds);
    END;
}

// FUNCTION: 0x000c5b20
void __cdecl AIL_set_sequence_volume(HSEQUENCE S, S32 volume, S32 milliseconds)
{
    START
    {
        oprintf3(D_00123a60, (S32)S, volume, milliseconds);
    }
    AIL_API_set_sequence_volume(S, volume, milliseconds);
    END;
}

// FUNCTION: 0x000c5bb0
void __cdecl AIL_set_sequence_loop_count(HSEQUENCE S, S32 loop_count)
{
    START
    {
        oprintf2(D_00123a88, (S32)S, loop_count);
    }
    AIL_API_set_sequence_loop_count(S, loop_count);
    END;
}

// FUNCTION: 0x000c5c30
U32 __cdecl AIL_sequence_status(HSEQUENCE S)
{
    U32 result;

    START
    {
        oprintf1(D_00123ab0, (S32)S);
    }
    result = AIL_API_sequence_status(S);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c5cf0
S32 __cdecl AIL_sequence_tempo(HSEQUENCE S)
{
    S32 result;

    START
    {
        oprintf1(D_00123acc, (S32)S);
    }
    result = AIL_API_sequence_tempo(S);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c5db0
S32 __cdecl AIL_sequence_volume(HSEQUENCE S)
{
    S32 result;

    START
    {
        oprintf1(D_00123ae8, (S32)S);
    }
    result = AIL_API_sequence_volume(S);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c5e70
S32 __cdecl AIL_sequence_loop_count(HSEQUENCE S)
{
    S32 result;

    START
    {
        oprintf1(D_00123b04, (S32)S);
    }
    result = AIL_API_sequence_loop_count(S);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c5f30
void __cdecl AIL_set_XMIDI_master_volume(HMDIDRIVER mdi, S32 master_volume)
{
    START
    {
        oprintf2(D_00123b24, (S32)mdi, master_volume);
    }
    AIL_API_set_XMIDI_master_volume(mdi, master_volume);
    END;
}

// FUNCTION: 0x000c5fb0
S32 __cdecl AIL_XMIDI_master_volume(HMDIDRIVER mdi)
{
    S32 result;

    START
    {
        oprintf1(D_00123b4c, (S32)mdi);
    }
    result = AIL_API_XMIDI_master_volume(mdi);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c6070
HMDIDRIVER __cdecl AIL_install_MDI_driver_image(void *driver_image, U32 size, IO_PARMS *IO)
{
    HMDIDRIVER result;

    START
    {
        oprintf3(D_00123b6c, (S32)driver_image, size, (S32)IO);
    }
    result = AIL_API_install_MDI_driver_image(driver_image, size, IO);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c6140
S32 __cdecl AIL_MDI_driver_type(HMDIDRIVER mdi)
{
    S32 result;

    START
    {
        oprintf1(D_00123b9c, (S32)mdi);
    }
    result = AIL_API_MDI_driver_type(mdi);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c6200
void __cdecl AIL_set_GTL_filename_prefix(char *prefix)
{
    START
    {
        oprintf1(D_00123bb8, (S32)prefix);
    }
    AIL_API_set_GTL_filename_prefix(prefix);
    END;
}

// FUNCTION: 0x000c6280
S32 __cdecl AIL_timbre_status(HMDIDRIVER mdi, S32 bank, S32 patch)
{
    S32 result;

    START
    {
        oprintf3(D_00123bdc, (S32)mdi, bank, patch);
    }
    result = AIL_API_timbre_status(mdi, bank, patch);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c6350
S32 __cdecl AIL_install_timbre(HMDIDRIVER mdi, S32 bank, S32 patch)
{
    S32 result;

    START
    {
        oprintf3(D_00123bfc, (S32)mdi, bank, patch);
    }
    result = AIL_API_install_timbre(mdi, bank, patch);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c6420
void __cdecl AIL_protect_timbre(HMDIDRIVER mdi, S32 bank, S32 patch)
{
    START
    {
        oprintf3(D_00123c20, (S32)mdi, bank, patch);
    }
    AIL_API_protect_timbre(mdi, bank, patch);
    END;
}

// FUNCTION: 0x000c64b0
void __cdecl AIL_unprotect_timbre(HMDIDRIVER mdi, S32 bank, S32 patch)
{
    START
    {
        oprintf3(D_00123c44, (S32)mdi, bank, patch);
    }
    AIL_API_unprotect_timbre(mdi, bank, patch);
    END;
}

// FUNCTION: 0x000c6540
S32 __cdecl AIL_active_sequence_count(HMDIDRIVER mdi)
{
    S32 result;

    START
    {
        oprintf1(D_00123c68, (S32)mdi);
    }
    result = AIL_API_active_sequence_count(mdi);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c6600
S32 __cdecl AIL_controller_value(HSEQUENCE S, S32 channel, S32 controller_num)
{
    S32 result;

    START
    {
        oprintf3(D_00123c8c, (S32)S, channel, controller_num);
    }
    result = AIL_API_controller_value(S, channel, controller_num);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c66d0
S32 __cdecl AIL_channel_notes(HSEQUENCE S, S32 channel)
{
    S32 result;

    START
    {
        oprintf2(D_00123cb0, (S32)S, channel);
    }
    result = AIL_API_channel_notes(S, channel);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c67a0
void __cdecl AIL_sequence_position(HSEQUENCE S, S32 *beat, S32 *measure)
{
    START
    {
        oprintf3(D_00123cd0, (S32)S, (S32)beat, (S32)measure);
    }
    AIL_API_sequence_position(S, beat, measure);
    RESULT
    {
        INDENT;
        oprintf2(D_00123cf8, *measure + 1, *beat + 1);
    }
    END;
}

// FUNCTION: 0x000c6870
void __cdecl AIL_branch_index(HSEQUENCE S, U32 marker)
{
    START
    {
        oprintf2(D_00123d0c, (S32)S, marker);
    }
    AIL_API_branch_index(S, marker);
    END;
}

// FUNCTION: 0x000c68f0
AILPREFIXCB __cdecl AIL_register_prefix_callback(HSEQUENCE S, AILPREFIXCB callback)
{
    AILPREFIXCB result;

    START
    {
        oprintf2(D_00123d28, (S32)S, (S32)callback);
    }
    result = AIL_API_register_prefix_callback(S, callback);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c69c0
AILTRIGGERCB __cdecl AIL_register_trigger_callback(HSEQUENCE S, AILTRIGGERCB callback)
{
    AILTRIGGERCB result;

    START
    {
        oprintf2(D_00123d54, (S32)S, (S32)callback);
    }
    result = AIL_API_register_trigger_callback(S, callback);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c6a90
AILSEQUENCECB __cdecl AIL_register_sequence_callback(HSEQUENCE S, AILSEQUENCECB callback)
{
    AILSEQUENCECB result;

    START
    {
        oprintf2(D_00123d80, (S32)S, (S32)callback);
    }
    result = AIL_API_register_sequence_callback(S, callback);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c6b60
AILBEATCB __cdecl AIL_register_beat_callback(HSEQUENCE S, AILBEATCB callback)
{
    AILBEATCB result;

    START
    {
        oprintf2(D_00123dac, (S32)S, (S32)callback);
    }
    result = AIL_API_register_beat_callback(S, callback);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c6c30
AILEVENTCB __cdecl AIL_register_event_callback(HMDIDRIVER mdi, AILEVENTCB callback)
{
    AILEVENTCB result;

    START
    {
        oprintf2(D_00123dd4, (S32)mdi, (S32)callback);
    }
    result = AIL_API_register_event_callback(mdi, callback);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c6d00
AILTIMBRECB __cdecl AIL_register_timbre_callback(HMDIDRIVER mdi, AILTIMBRECB callback)
{
    AILTIMBRECB result;

    START
    {
        oprintf2(D_00123e00, (S32)mdi, (S32)callback);
    }
    result = AIL_API_register_timbre_callback(mdi, callback);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c6dd0
void __cdecl AIL_set_sequence_user_data(HSEQUENCE S, U32 index, S32 value)
{
    START
    {
        oprintf3(D_00123e2c, (S32)S, index, value);
    }
    AIL_API_set_sequence_user_data(S, index, value);
    END;
}

// FUNCTION: 0x000c6e60
S32 __cdecl AIL_sequence_user_data(HSEQUENCE S, U32 index)
{
    S32 result;

    START
    {
        oprintf2(D_00123e58, (S32)S, index);
    }
    result = AIL_API_sequence_user_data(S, index);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c6f30
void __cdecl AIL_register_ICA_array(HSEQUENCE S, U8 *array)
{
    START
    {
        oprintf2(D_00123e7c, (S32)S, (S32)array);
    }
    AIL_API_register_ICA_array(S, array);
    END;
}

// FUNCTION: 0x000c6fb0
S32 __cdecl AIL_lock_channel(HMDIDRIVER mdi)
{
    S32 result;

    START
    {
        oprintf1(D_00123ea0, (S32)mdi);
    }
    result = AIL_API_lock_channel(mdi);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c7070
void __cdecl AIL_release_channel(HMDIDRIVER mdi, S32 channel)
{
    START
    {
        oprintf2(D_00123ebc, (S32)mdi, channel);
    }
    AIL_API_release_channel(mdi, channel);
    END;
}

// FUNCTION: 0x000c70f0
void __cdecl AIL_map_sequence_channel(HSEQUENCE S, S32 seq_channel, S32 new_channel)
{
    START
    {
        oprintf3(D_00123edc, (S32)S, seq_channel, new_channel);
    }
    AIL_API_map_sequence_channel(S, seq_channel, new_channel);
    END;
}

// FUNCTION: 0x000c7180
S32 __cdecl AIL_true_sequence_channel(HSEQUENCE S, S32 seq_channel)
{
    S32 result;

    START
    {
        oprintf2(D_00123f04, (S32)S, seq_channel);
    }
    result = AIL_API_true_sequence_channel(S, seq_channel);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c7250
void __cdecl AIL_send_channel_voice_message(HMDIDRIVER mdi, HSEQUENCE S, S32 status, S32 data_1, S32 data_2)
{
    START
    {
        oprintf5(D_00123f2c, (S32)mdi, (S32)S, status, data_1, data_2);
    }
    AIL_API_send_channel_voice_message(mdi, S, status, data_1, data_2);
    END;
}

// FUNCTION: 0x000c72f0
void __cdecl AIL_send_sysex_message(HMDIDRIVER mdi, void *buffer)
{
    START
    {
        oprintf2(D_00123f68, (S32)mdi, (S32)buffer);
    }
    AIL_API_send_sysex_message(mdi, buffer);
    END;
}

// FUNCTION: 0x000c7370
HWAVESYNTH __cdecl AIL_create_wave_synthesizer(HDIGDRIVER dig, HMDIDRIVER mdi, void *wave_lib, S32 polyphony)
{
    HWAVESYNTH result;

    START
    {
        oprintf4(D_00123f8c, (S32)dig, (S32)mdi, (S32)wave_lib, polyphony);
    }
    result = AIL_API_create_wave_synthesizer(dig, mdi, wave_lib, polyphony);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c7450
void __cdecl AIL_destroy_wave_synthesizer(HWAVESYNTH W)
{
    START
    {
        oprintf1(D_00123fc0, (S32)W);
    }
    AIL_API_destroy_wave_synthesizer(W);
    END;
}

// FUNCTION: 0x000c74d0
HREDBOOK __cdecl AIL_redbook_open(U32 which)
{
    HREDBOOK result;

    START
    {
        oprintf1(D_00123fe8, which);
    }
    result = AIL_API_redbook_open(which);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c7590
void __cdecl AIL_redbook_close(HREDBOOK hand)
{
    START
    {
        oprintf1(D_00124000, (S32)hand);
    }
    AIL_API_redbook_close(hand);
    END;
}

// FUNCTION: 0x000c7610
void __cdecl AIL_redbook_eject(HREDBOOK hand)
{
    START
    {
        oprintf1(D_0012401c, (S32)hand);
    }
    AIL_API_redbook_eject(hand);
    END;
}

// FUNCTION: 0x000c7690
void __cdecl AIL_redbook_retract(HREDBOOK hand)
{
    START
    {
        oprintf1(D_00124038, (S32)hand);
    }
    AIL_API_redbook_retract(hand);
    END;
}

// FUNCTION: 0x000c7710
U32 __cdecl AIL_redbook_status(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1(D_00124058, (S32)hand);
    }
    result = AIL_API_redbook_status(hand);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c77d0
U32 __cdecl AIL_redbook_tracks(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1(D_00124074, (S32)hand);
    }
    result = AIL_API_redbook_tracks(hand);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c7890
void __cdecl AIL_redbook_track_info(HREDBOOK hand, U32 tracknum, U32 *startmsec, U32 *endmsec)
{
    START
    {
        oprintf4(D_00124090, (S32)hand, tracknum, (S32)startmsec, (S32)endmsec);
    }
    AIL_API_redbook_track_info(hand, tracknum, startmsec, endmsec);
    END;
}

// FUNCTION: 0x000c7920
U32 __cdecl AIL_redbook_id(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1(D_001240c4, (S32)hand);
    }
    result = AIL_API_redbook_id(hand);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c79e0
U32 __cdecl AIL_redbook_position(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1(D_001240dc, (S32)hand);
    }
    result = AIL_API_redbook_position(hand);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c7aa0
U32 __cdecl AIL_redbook_track(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1(D_001240fc, (S32)hand);
    }
    result = AIL_API_redbook_track(hand);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c7b60
U32 __cdecl AIL_redbook_play(HREDBOOK hand, U32 startmsec, U32 endmsec)
{
    U32 result;

    START
    {
        oprintf3(D_00124118, (S32)hand, startmsec, endmsec);
    }
    result = AIL_API_redbook_play(hand, startmsec, endmsec);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c7c30
U32 __cdecl AIL_redbook_stop(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1(D_0012413c, (S32)hand);
    }
    result = AIL_API_redbook_stop(hand);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c7cf0
U32 __cdecl AIL_redbook_pause(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1(D_00124158, (S32)hand);
    }
    result = AIL_API_redbook_pause(hand);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c7db0
U32 __cdecl AIL_redbook_resume(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1(D_00124174, (S32)hand);
    }
    result = AIL_API_redbook_resume(hand);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c7e70
void __cdecl AIL_quick_shutdown(void)
{
    START
    {
        AIL_DEBUGOUTSTR(D_00124190);
    }
    AIL_API_quick_shutdown();
    END;
}

// FUNCTION: 0x000c7ed0
HAUDIO __cdecl AIL_quick_load(char *filename)
{
    HAUDIO result;

    START
    {
        oprintf1(D_001241a8, (S32)filename);
    }
    result = AIL_API_quick_load(filename);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c7f90
void __cdecl AIL_quick_unload(HAUDIO audio)
{
    START
    {
        oprintf1(D_001241c0, (S32)audio);
    }
    AIL_API_quick_unload(audio);
    END;
}

// FUNCTION: 0x000c8010
S32 __cdecl AIL_quick_play(HAUDIO audio, U32 loop_count)
{
    S32 result;

    START
    {
        oprintf2(D_001241dc, (S32)audio, loop_count);
    }
    result = AIL_API_quick_play(audio, loop_count);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c80e0
void __cdecl AIL_quick_halt(HAUDIO audio)
{
    START
    {
        oprintf1(D_001241f8, (S32)audio);
    }
    AIL_API_quick_halt(audio);
    END;
}

// FUNCTION: 0x000c8160
S32 __cdecl AIL_quick_status(HAUDIO audio)
{
    S32 result;

    START
    {
        oprintf1(D_00124210, (S32)audio);
    }
    result = AIL_API_quick_status(audio);
    RESULT
    {
        INDENT;
        outresint(result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c8220
HAUDIO __cdecl AIL_quick_load_and_play(char *filename, U32 loop_count, S32 wait_request)
{
    HAUDIO result;

    START
    {
        oprintf3(D_0012422c, (S32)filename, loop_count, wait_request);
    }
    result = AIL_API_quick_load_and_play(filename, loop_count, wait_request);
    RESULT
    {
        INDENT;
        outreshex((U32)result);
    }
    END;
    return result;
}

// FUNCTION: 0x000c82f0
void __pascal AILDEBUG_END(void)
{
    if (locked) {
        AIL_vmm_unlock_range(AILDEBUG_START, AILDEBUG_END);
        locked = 0;
    }
}
