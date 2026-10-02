#include "ail.h"

S32 AIL_started = 0;
static S32 locked = 0;
static char AIL_indent_str[] = "               "
    "\372\372\372\372\372\372\372\372\372\372\372\372\372";

#define START                                                            \
    AIL_indent++;                                                        \
    if ((AIL_debug && (AIL_indent == 1 || AIL_sys_debug)                 \
         && !AIL_API_background() && TIME_WRITE()) ? 1 : 0)

#define RESULT                                                           \
    if (AIL_debug && (AIL_indent == 1 || AIL_sys_debug)                  \
        && !AIL_API_background())

#define INDENT AIL_fwrite(AIL_debugfile, AIL_indent_str, AIL_indent + 14)

#define END AIL_indent--


void __pascal AILDEBUG_START(void);
void __pascal AILDEBUG_END(void);
#pragma aux (__pascal) AILDEBUG_END "AILDEBUG_END"

// Locks this module's code and the tracing state against paging, once.
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

// Writes string `str` to the MSS_DEBUG log.
// FUNCTION: 0x000c2550
void __pascal AIL_DEBUGOUTSTR(char const *str)
{
    AIL_fwrite(AIL_debugfile, str, strlen(str));
}

// Writes "Result = `value`" (decimal) to the MSS_DEBUG log.
// FUNCTION: 0x000c2580
void outresint(S32 value)
{
    char buf[64];
    S32 args[1];

    args[0] = value;
    AIL_sprintf(buf, "Result = %d\r\n", 1, args);
    AIL_DEBUGOUTSTR(buf);
}

// Writes "Result = 0x`value`" (hex) to the MSS_DEBUG log.
// FUNCTION: 0x000c25b0
void outreshex(U32 value)
{
    char buf[64];
    S32 args[1];

    args[0] = value;
    AIL_sprintf(buf, "Result = 0x%X\r\n", 1, args);
    AIL_DEBUGOUTSTR(buf);
}

// Writes `fmt` formatted with `a1` to the MSS_DEBUG log (128-byte buffer).
// FUNCTION: 0x000c25e0
void oprintf1(char const *fmt, S32 a1)
{
    char buf[128];
    S32 args[1];

    args[0] = a1;
    AIL_sprintf(buf, fmt, 1, args);
    AIL_DEBUGOUTSTR(buf);
}

// Writes `fmt` formatted with `a1` and `a2` to the MSS_DEBUG log (128-byte
// buffer).
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

// Writes `fmt` formatted with `a1` to `a3` to the MSS_DEBUG log (128-byte
// buffer).
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

// Writes `fmt` formatted with `a1` to `a4` to the MSS_DEBUG log (128-byte
// buffer).
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

// Writes `fmt` formatted with `a1` to `a5` to the MSS_DEBUG log (128-byte
// buffer).
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

// Starts a log line: the time since startup for a top-level call, blank padding
// for a nested one, then one marker character per nesting level below the top.
// Returns 1.
// FUNCTION: 0x000c2750
S32 __pascal TIME_WRITE(void)
{
    U32 elapsed;
    U16 hours, minutes, sec, ms;

    elapsed = AIL_ms_count() - AIL_starttime;
    hours = elapsed / 3600000;
    minutes = elapsed / 60000 % 60;
    sec = elapsed / 1000 % 60;
    ms = elapsed % 1000;
    if (AIL_indent == 1) {
        oprintf4("[%.02u:%.02u:%.02u.%.03u] ", hours, minutes, sec, ms);
    } else {
        AIL_DEBUGOUTSTR("               ");
    }
    if (AIL_indent > 1) {
        AIL_fwrite(AIL_debugfile, "\372\372\372\372\372\372\372\372\372\372\372\372\372\372\372\372", AIL_indent - 1);
    }
    return 1;
}

// Shuts the library down (uninstalling every driver and releasing all timers)
// and closes the MSS_DEBUG log; does nothing unless AIL_startup ran. Logged
// when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c2810
void __cdecl AIL_shutdown(void)
{
    if (AIL_started) {
        START
        {
            AIL_DEBUGOUTSTR("AIL_shutdown()\r\n");
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

// Sets the directory searched for drivers and .INI files to `dir`, adding a
// trailing '\' unless it ends in '\' or ':'; NULL clears it. Returns the stored
// directory. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c28c0
char *__cdecl AIL_set_driver_directory(char *dir)
{
    char *result;

    START
    {
        oprintf1("AIL_set_driver_directory(%s)\r\n", (S32)dir);
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

// Sets preference `number` (DIG_..., MDI_..., AIL_...) to `value`; returns its
// previous value. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c2980
S32 __cdecl AIL_set_preference(U32 number, S32 value)
{
    S32 result;

    START
    {
        oprintf2("AIL_set_preference(%d,%d)\r\n", number, value);
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

// Raises the timer lock count. While it is above zero the timer interrupt still
// counts due periods but defers the callbacks until AIL_unlock.
// FUNCTION: 0x000c2a50
void __cdecl AIL_lock(void)
{
    AIL_API_lock();
}

// Lowers the timer lock count raised by AIL_lock.
// FUNCTION: 0x000c2a60
void __cdecl AIL_unlock(void)
{
    AIL_API_unlock();
}

// Returns real-mode interrupt vector `vectnum` as a segment:offset pair
// (segment in the high word). Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c2a70
REALFAR __cdecl AIL_get_real_vect(U32 vectnum)
{
    REALFAR result;

    START
    {
        oprintf1("AIL_get_real_vect(0x%X)\r\n", vectnum);
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

// Sets real-mode interrupt vector `vectnum` to the segment:offset pair
// `real_ptr` (segment in the high word). Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c2b30
void __cdecl AIL_set_real_vect(U32 vectnum, REALFAR real_ptr)
{
    START
    {
        oprintf2("AIL_set_real_vect(0x%X,0x%X)\r\n", vectnum, (S32)real_ptr);
    }
    AIL_API_set_real_vect(vectnum, real_ptr);
    END;
}

// Routes hardware interrupt `IRQ` to the 16-bit driver ISR at `ISR_offset` in
// real-mode segment `real_base`; one at a time. Logged when MSS_DEBUG tracing
// is on.
// FUNCTION: 0x000c2bb0
void __cdecl AIL_set_USE16_ISR(S32 IRQ, REALFAR real_base, U32 ISR_offset)
{
    START
    {
        oprintf3("AIL_set_USE16_ISR(%d,0x%X,%u)\r\n", IRQ, (S32)real_base, ISR_offset);
    }
    AIL_API_set_USE16_ISR(IRQ, real_base, ISR_offset);
    END;
}

// Restores the protected-mode vector of `IRQ` saved by AIL_set_USE16_ISR, if
// that is the IRQ currently installed. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c2c40
void __cdecl AIL_restore_USE16_ISR(S32 IRQ)
{
    START
    {
        oprintf1("AIL_restore_USE16_ISR(%d)\r\n", IRQ);
    }
    AIL_API_restore_USE16_ISR(IRQ);
    END;
}

// Disables interrupts; returns the EFLAGS value from before, for
// AIL_restore_interrupts.
// FUNCTION: 0x000c2cc0
U32 __cdecl AIL_disable_interrupts(void)
{
    return AIL_API_disable_interrupts();
}

// Restores EFLAGS (and so the interrupt flag) from `FD_register`, as returned
// by AIL_disable_interrupts.
// FUNCTION: 0x000c2cd0
void __cdecl AIL_restore_interrupts(U32 FD_register)
{
    AIL_API_restore_interrupts(FD_register);
}

// Calls driver `drvr` function `fn` through INT 66h with registers from `in`,
// returning them in `out` (either may be NULL). Returns AX. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c2ce0
S32 __cdecl AIL_call_driver(AIL_DRIVER *drvr, S32 fn, VDI_CALL *in, VDI_CALL *out)
{
    S32 result;

    START
    {
        oprintf4("AIL_call_driver(0x%X,0x%X,0x%X,0x%X)\r\n", (S32)drvr, fn, (S32)in, (S32)out);
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

// Busy-waits for `intervals` ticks of 16 ms (about 1/60 second). Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c2dc0
void __cdecl AIL_delay(S32 intervals)
{
    START
    {
        oprintf1("AIL_delay(%d)\r\n", intervals);
    }
    AIL_API_delay(intervals);
    END;
}

// Returns nonzero while called from inside the timer interrupt.
// FUNCTION: 0x000c2e40
S32 __cdecl AIL_background(void)
{
    return AIL_API_background();
}

// Reads driver name, device name and I/O settings from .INI file `filename`
// into `INI`. Returns 1, or 0 if missing or incomplete. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c2e50
S32 __cdecl AIL_read_INI(AIL_INI *INI, char *filename)
{
    S32 result;

    START
    {
        oprintf2("AIL_API_read_INI(0x%X,%s)\r\n", (S32)INI, (S32)filename);
    }
    result = AIL_API_read_INI(INI, filename);
    if (result) {
        RESULT
        {
            INDENT;
            oprintf1("Driver = %s\r\n", (S32)INI->driver_name);
            INDENT;
            oprintf1("Device = %s\r\n", (S32)INI->device_name);
            INDENT;
            oprintf1("IO     = %X\r\n", INI->IO.IO);
            INDENT;
            oprintf1("IRQ    = %d\r\n", INI->IO.IRQ);
            INDENT;
            oprintf1("DMA_8  = %d\r\n", INI->IO.DMA_8_bit);
            INDENT;
            oprintf1("DMA_16 = %d\r\n", INI->IO.DMA_16_bit);
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

// Allocates a stopped timer with `callback_fn` as its callback. Returns its
// handle, or -1 if all 15 are in use. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3070
HTIMER __cdecl AIL_register_timer(AILTIMERCB callback_fn)
{
    HTIMER result;

    START
    {
        oprintf1("AIL_register_timer(0x%X)\r\n", (S32)callback_fn);
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

// Sets the value passed to timer `timer`'s callback to `user`; returns the
// previous value. Handle -1 is ignored. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3130
U32 __cdecl AIL_set_timer_user(HTIMER timer, U32 user)
{
    U32 result;

    START
    {
        oprintf2("AIL_set_timer_user(%u,%u)\r\n", (S32)timer, user);
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

// Sets timer `timer`'s period to `microseconds`, restarting its count, and
// reprograms the PIT for the shortest period in use. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c3200
void __cdecl AIL_set_timer_period(HTIMER timer, U32 microseconds)
{
    START
    {
        oprintf2("AIL_set_timer_period(%u,%u)\r\n", (S32)timer, microseconds);
    }
    AIL_API_set_timer_period(timer, microseconds);
    END;
}

// Sets timer `timer` to fire `hertz` times a second (period 1000000 / `hertz`
// microseconds). Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3280
void __cdecl AIL_set_timer_frequency(HTIMER timer, U32 hertz)
{
    START
    {
        oprintf2("AIL_set_timer_frequency(%u,%u)\r\n", (S32)timer, hertz);
    }
    AIL_API_set_timer_frequency(timer, hertz);
    END;
}

// Sets timer `timer`'s period to that of PIT divisor `PIT_divisor` (0 gives the
// BIOS rate, 54925 microseconds). Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3300
void __cdecl AIL_set_timer_divisor(HTIMER timer, U32 PIT_divisor)
{
    START
    {
        oprintf2("AIL_set_timer_divisor(%u,%u)\r\n", (S32)timer, PIT_divisor);
    }
    AIL_API_set_timer_divisor(timer, PIT_divisor);
    END;
}

// Returns the divisor the PIT is currently programmed with (0 = 65536). Logged
// when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3380
S32 __cdecl AIL_interrupt_divisor(void)
{
    S32 result;

    START
    {
        AIL_DEBUGOUTSTR("AIL_interrupt_divisor()\r\n");
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

// Starts timer `timer` if it is allocated and stopped; -1 is ignored. Logged
// when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3440
void __cdecl AIL_start_timer(HTIMER timer)
{
    START
    {
        oprintf1("AIL_start_timer(%u)\r\n", (S32)timer);
    }
    AIL_API_start_timer(timer);
    END;
}

// Starts every allocated, stopped user timer. Logged when MSS_DEBUG tracing is
// on.
// FUNCTION: 0x000c34c0
void __cdecl AIL_start_all_timers(void)
{
    START
    {
        AIL_DEBUGOUTSTR("AIL_start_all_timers()\r\n");
    }
    AIL_API_start_all_timers();
    END;
}

// Stops timer `timer` if it is running; -1 is ignored. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c3520
void __cdecl AIL_stop_timer(HTIMER timer)
{
    START
    {
        oprintf1("AIL_stop_timer(%u)\r\n", (S32)timer);
    }
    AIL_API_stop_timer(timer);
    END;
}

// Stops every running user timer. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c35a0
void __cdecl AIL_stop_all_timers(void)
{
    START
    {
        AIL_DEBUGOUTSTR("AIL_stop_all_timers()\r\n");
    }
    AIL_API_stop_all_timers();
    END;
}

// Frees timer `timer`; -1 is ignored. The PIT rate is not recomputed. Logged
// when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3600
void __cdecl AIL_release_timer_handle(HTIMER timer)
{
    START
    {
        oprintf1("AIL_release_timer_handle(%u)\r\n", (S32)timer);
    }
    AIL_API_release_timer_handle(timer);
    END;
}

// Frees the 15 user timers. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3680
void __cdecl AIL_release_all_timers(void)
{
    START
    {
        AIL_DEBUGOUTSTR("AIL_release_all_timers()\r\n");
    }
    AIL_API_release_all_timers();
    END;
}

// Returns `drvr`'s I/O settings parsed from the environment variable it names
// (static buffer), or NULL. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c36e0
IO_PARMS *__cdecl AIL_get_IO_environment(AIL_DRIVER *drvr)
{
    IO_PARMS * result;

    START
    {
        oprintf1("AIL_get_IO_environment(0x%X)\r\n", (S32)drvr);
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

// Installs the `n_bytes`-byte .DIG or .MDI driver image `driver_image`. Returns
// the driver, or NULL with AIL_error set. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c37a0
AIL_DRIVER *__cdecl AIL_install_driver(U8 *driver_image, U32 n_bytes)
{
    AIL_DRIVER * result;

    START
    {
        oprintf2("AIL_install_driver(0x%X,%u)\r\n", (S32)driver_image, n_bytes);
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

// Releases driver `drvr`: runs its destructor, stops its timer, shuts it down
// (DRV_SHUTDOWN) if initialized, restores INT 66h and frees its memory and
// handle. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3870
void __cdecl AIL_uninstall_driver(AIL_DRIVER *drvr)
{
    START
    {
        oprintf1("AIL_uninstall_driver(0x%X)\r\n", (S32)drvr);
    }
    AIL_API_uninstall_driver(drvr);
    END;
}

// Installs the digital driver and I/O settings named in DIG.INI, storing it in
// `*dig`. Returns 0 on success, 1 if DIG.INI cannot be read, 2 if the install
// fails. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c38f0
S32 __cdecl AIL_install_DIG_INI(HDIGDRIVER *dig)
{
    S32 result;

    START
    {
        oprintf1("AIL_install_DIG_INI(0x%X)\r\n", (S32)dig);
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

// Loads .DIG driver file `filename` (also tried in the driver directory) and
// installs it with I/O settings `IO` (NULL to detect). Returns NULL on failure.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c39b0
HDIGDRIVER __cdecl AIL_install_DIG_driver_file(char *filename, IO_PARMS *IO)
{
    HDIGDRIVER result;

    START
    {
        oprintf2("AIL_install_DIG_driver_file(%s,0x%X)\r\n", (S32)filename, (S32)IO);
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

// Uninstalls digital driver `dig` and frees everything it owns. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3a80
void __cdecl AIL_uninstall_DIG_driver(HDIGDRIVER dig)
{
    START
    {
        oprintf1("AIL_uninstall_DIG_driver(0x%X)\r\n", (S32)dig);
    }
    AIL_API_uninstall_DIG_driver(dig);
    END;
}

// Returns a free sample of `dig`, initialised with AIL_init_sample, or NULL
// with AIL_error set if all are in use. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3b00
HSAMPLE __cdecl AIL_allocate_sample_handle(HDIGDRIVER dig)
{
    HSAMPLE result;

    START
    {
        oprintf1("AIL_allocate_sample_handle(0x%X)\r\n", (S32)dig);
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

// Allocates a sample on `dig` playing VOC or WAV file image `file_image` (VOC
// marker `block`, -1 for all), released when it ends. NULL on failure. Logged
// when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3bc0
HSAMPLE __cdecl AIL_allocate_file_sample(HDIGDRIVER dig, void *file_image, S32 block)
{
    HSAMPLE result;

    START
    {
        oprintf3("AIL_allocate_file_sample(0x%X,0x%X,%d)\r\n", (S32)dig, (S32)file_image, block);
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

// Returns sample `S` to its driver's pool, silencing it. NULL is ignored.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3c90
void __cdecl AIL_release_sample_handle(HSAMPLE S)
{
    START
    {
        oprintf1("AIL_release_sample_handle(0x%X)\r\n", (S32)S);
    }
    AIL_API_release_sample_handle(S);
    END;
}

// Resets sample `S` to defaults: done, no data, played once, 8-bit mono at
// 11025 Hz, default volume, pan 64 (0 on mono hardware), no callbacks. Logged
// when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3d10
void __cdecl AIL_init_sample(HSAMPLE S)
{
    START
    {
        oprintf1("AIL_init_sample(0x%X)\r\n", (S32)S);
    }
    AIL_API_init_sample(S);
    END;
}

// Sets sample `S` up from VOC or WAV file image `file_image` (VOC marker
// `block`, -1 for all). Returns 1, or 0 on failure. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c3d90
S32 __cdecl AIL_set_sample_file(HSAMPLE S, void *file_image, S32 block)
{
    S32 result;

    START
    {
        oprintf3("AIL_set_sample_file(0x%X,0x%X,%d)\r\n", (S32)S, (S32)file_image, block);
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

// Makes the `len` bytes at `start` `S`'s (single-buffered) data. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3e60
void __cdecl AIL_set_sample_address(HSAMPLE S, void *start, U32 len)
{
    START
    {
        oprintf3("AIL_set_sample_address(0x%X,0x%X,%u)\r\n", (S32)S, (S32)start, len);
    }
    AIL_API_set_sample_address(S, start, len);
    END;
}

// Sets `S`'s data format (DIG_F_...) and `flags` (DIG_PCM_SIGN), rebuilding its
// scaling if they changed. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3ef0
void __cdecl AIL_set_sample_type(HSAMPLE S, S32 format, U32 flags)
{
    START
    {
        oprintf3("AIL_set_sample_type(0x%X,%d,%u)\r\n", (S32)S, format, flags);
    }
    AIL_API_set_sample_type(S, format, flags);
    END;
}

// Plays `S` from the start of its current buffer, starting DMA if needed.
// Ignored if `S` is free or has no data. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c3f80
void __cdecl AIL_start_sample(HSAMPLE S)
{
    START
    {
        oprintf1("AIL_start_sample(0x%X)\r\n", (S32)S);
    }
    AIL_API_start_sample(S);
    END;
}

// Pauses `S` if it is playing. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4000
void __cdecl AIL_stop_sample(HSAMPLE S)
{
    START
    {
        oprintf1("AIL_stop_sample(0x%X)\r\n", (S32)S);
    }
    AIL_API_stop_sample(S);
    END;
}

// Resumes `S` if it is paused. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4080
void __cdecl AIL_resume_sample(HSAMPLE S)
{
    START
    {
        oprintf1("AIL_resume_sample(0x%X)\r\n", (S32)S);
    }
    AIL_API_resume_sample(S);
    END;
}

// Ends `S` now unless it is free or already done: marks it done and calls its
// EOB and EOS callbacks. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4100
void __cdecl AIL_end_sample(HSAMPLE S)
{
    START
    {
        oprintf1("AIL_end_sample(0x%X)\r\n", (S32)S);
    }
    AIL_API_end_sample(S);
    END;
}

// Sets `S`'s playback rate to `playback_rate` Hz. Logged when MSS_DEBUG tracing
// is on.
// FUNCTION: 0x000c4180
void __cdecl AIL_set_sample_playback_rate(HSAMPLE S, S32 playback_rate)
{
    START
    {
        oprintf2("AIL_set_sample_playback_rate(0x%X,%d)\r\n", (S32)S, playback_rate);
    }
    AIL_API_set_sample_playback_rate(S, playback_rate);
    END;
}

// Sets `S`'s volume (0-127) and rebuilds its scaling. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c4200
void __cdecl AIL_set_sample_volume(HSAMPLE S, S32 volume)
{
    START
    {
        oprintf2("AIL_set_sample_volume(0x%X,%d)\r\n", (S32)S, volume);
    }
    AIL_API_set_sample_volume(S, volume);
    END;
}

// Sets `S`'s pan (0 left - 127 right) and rebuilds its scaling. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4280
void __cdecl AIL_set_sample_pan(HSAMPLE S, S32 pan)
{
    START
    {
        oprintf2("AIL_set_sample_pan(0x%X,%d)\r\n", (S32)S, pan);
    }
    AIL_API_set_sample_pan(S, pan);
    END;
}

// Sets how many times `S` plays to `loop_count` (0 = forever). Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4300
void __cdecl AIL_set_sample_loop_count(HSAMPLE S, S32 loop_count)
{
    START
    {
        oprintf2("AIL_set_sample_loop_count(0x%X,%d)\r\n", (S32)S, loop_count);
    }
    AIL_API_set_sample_loop_count(S, loop_count);
    END;
}

// Sets the byte offsets each repeat of `S` starts and ends at (end -1 = end of
// data). Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4380
void __cdecl AIL_set_sample_loop_block(HSAMPLE S, S32 loop_start_offset, S32 loop_end_offset)
{
    START
    {
        oprintf3("AIL_set_sample_loop_block(0x%lX,%ld,%ld)\r\n", (S32)S, loop_start_offset, loop_end_offset);
    }
    AIL_API_set_sample_loop_block(S, loop_start_offset, loop_end_offset);
    END;
}

// Returns `S`'s SMP_... status, 0 for NULL. Logged when MSS_DEBUG tracing is
// on.
// FUNCTION: 0x000c4410
U32 __cdecl AIL_sample_status(HSAMPLE S)
{
    U32 result;

    START
    {
        oprintf1("AIL_sample_status(0x%X)\r\n", (S32)S);
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

// Returns `S`'s playback rate in Hz, 0 for NULL. Logged when MSS_DEBUG tracing
// is on.
// FUNCTION: 0x000c44d0
S32 __cdecl AIL_sample_playback_rate(HSAMPLE S)
{
    S32 result;

    START
    {
        oprintf1("AIL_sample_playback_rate(0x%X)\r\n", (S32)S);
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

// Returns `S`'s volume (0-127), 0 for NULL. Logged when MSS_DEBUG tracing is
// on.
// FUNCTION: 0x000c4590
S32 __cdecl AIL_sample_volume(HSAMPLE S)
{
    S32 result;

    START
    {
        oprintf1("AIL_sample_volume(0x%X)\r\n", (S32)S);
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

// Returns `S`'s pan (0 left - 127 right), 0 for NULL. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c4650
S32 __cdecl AIL_sample_pan(HSAMPLE S)
{
    S32 result;

    START
    {
        oprintf1("AIL_sample_pan(0x%X)\r\n", (S32)S);
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

// Returns how many more times `S` plays (0 = forever), -1 for NULL. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4710
S32 __cdecl AIL_sample_loop_count(HSAMPLE S)
{
    S32 result;

    START
    {
        oprintf1("AIL_sample_loop_count(0x%X)\r\n", (S32)S);
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

// Sets `dig`'s master volume (0-127) and rebuilds the scaling of every
// allocated sample. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c47d0
void __cdecl AIL_set_digital_master_volume(HDIGDRIVER dig, S32 master_volume)
{
    START
    {
        oprintf2("AIL_set_digital_master_volume(0x%X,%d)\r\n", (S32)dig, master_volume);
    }
    AIL_API_set_digital_master_volume(dig, master_volume);
    END;
}

// Returns `dig`'s master volume. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4850
S32 __cdecl AIL_digital_master_volume(HDIGDRIVER dig)
{
    S32 result;

    START
    {
        oprintf1("AIL_digital_master_volume(0x%X)\r\n", (S32)dig);
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

// Stores `dig`'s DMA rate in `*rate`, hardware format in `*format` and device
// name in `string`; NULL ones are skipped. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4910
void __cdecl AIL_digital_configuration(HDIGDRIVER dig, S32 *rate, S32 *format,
                                       char *string)
{
    START
    {
        oprintf4("AIL_digital_configuration(0x%lX,0x%lX,0x%lX,0x%lX)\r\n", (S32)dig, (S32)rate, (S32)format, (S32)string);
    }
    AIL_API_digital_configuration(dig, rate, format, string);
    RESULT
    {
        INDENT;
        oprintf3("Result = %ld, %lX, %s\r\n", *rate, *format, *string);
    }
    END;
}

// Installs the .DIG driver image `driver_image` of `size` bytes and sets it up
// with I/O settings `IO` (NULL to detect). Returns the driver, or NULL with
// AIL_error set. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c49f0
HDIGDRIVER __cdecl AIL_install_DIG_driver_image(void *driver_image, U32 size, IO_PARMS *IO)
{
    HDIGDRIVER result;

    START
    {
        oprintf3("AIL_install_DIG_driver_image(0x%X,%u,0x%X)\r\n", (S32)driver_image, size, (S32)IO);
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

// Returns the smallest buffer, in bytes, for AIL_load_sample_buffer data at
// `playback_rate` Hz in `format` to cover one DMA half-buffer. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4ac0
S32 __cdecl AIL_minimum_sample_buffer_size(HDIGDRIVER dig, S32 playback_rate, S32 format)
{
    S32 result;

    START
    {
        oprintf3("AIL_minimum_sample_buffer_size(0x%X,%d,%d)\r\n", (S32)dig, playback_rate, format);
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

// Returns which buffer of streaming sample `S` to load next (0 or 1), or -1 if
// none is needed yet (also for NULL). The first two calls after AIL_init_sample
// return 0 and then 1. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4b90
S32 __cdecl AIL_sample_buffer_ready(HSAMPLE S)
{
    S32 result;

    START
    {
        oprintf1("AIL_sample_buffer_ready(0x%X)\r\n", (S32)S);
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

// Loads double buffer `buff_num` (0 or 1) of `S` with the `len` bytes at
// `buffer`, `len` 0 meaning no more data, and starts `S` playing in streaming
// mode if it is not already. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4c50
void __cdecl AIL_load_sample_buffer(HSAMPLE S, U32 buff_num, void *buffer, U32 len)
{
    START
    {
        oprintf4("AIL_load_sample_buffer(0x%X,%u,0x%X,%u)\r\n", (S32)S, buff_num, (S32)buffer, len);
    }
    AIL_API_load_sample_buffer(S, buff_num, buffer, len);
    END;
}

// Stores the positions and lengths of `S`'s two buffers through the non-NULL
// pointers. Returns 1 if `S` has run out of data (starved), else 0. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4ce0
S32 __cdecl AIL_sample_buffer_info(HSAMPLE S, U32 *pos0, U32 *len0, U32 *pos1, U32 *len1)
{
    S32 result;

    START
    {
        oprintf5("AIL_sample_buffer_info(0x%X,0x%X,0x%X,0x%X,0x%X)\r\n", (S32)S, (S32)pos0, (S32)len0, (S32)pos1, (S32)len1);
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

// Moves playback in `S`'s current buffer to byte offset `pos`. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4dd0
void __cdecl AIL_set_sample_position(HSAMPLE S, U32 pos)
{
    START
    {
        oprintf2("AIL_set_sample_position(0x%X,%u)\r\n", (S32)S, pos);
    }
    AIL_API_set_sample_position(S, pos);
    END;
}

// Returns the byte offset of playback in `S`'s current buffer, 0 for NULL.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4e50
U32 __cdecl AIL_sample_position(HSAMPLE S)
{
    U32 result;

    START
    {
        oprintf1("AIL_sample_position(0x%X)\r\n", (S32)S);
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

// Sets `S`'s start-of-block callback; returns the previous one (NULL for a NULL
// `S`). Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4f10
AILSAMPLECB __cdecl AIL_register_SOB_callback(HSAMPLE S, AILSAMPLECB SOB)
{
    AILSAMPLECB result;

    START
    {
        oprintf2("AIL_register_SOB_callback(0x%X,0x%X)\r\n", (S32)S, (S32)SOB);
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

// Sets `S`'s end-of-buffer callback; returns the previous one (NULL for a NULL
// `S`). Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c4fe0
AILSAMPLECB __cdecl AIL_register_EOB_callback(HSAMPLE S, AILSAMPLECB EOB)
{
    AILSAMPLECB result;

    START
    {
        oprintf2("AIL_register_EOB_callback(0x%X,0x%X)\r\n", (S32)S, (S32)EOB);
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

// Sets `S`'s end-of-sample callback; returns the previous one (NULL for a NULL
// `S`). Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c50b0
AILSAMPLECB __cdecl AIL_register_EOS_callback(HSAMPLE S, AILSAMPLECB EOS)
{
    AILSAMPLECB result;

    START
    {
        oprintf2("AIL_register_EOS_callback(0x%X,0x%X)\r\n", (S32)S, (S32)EOS);
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

// Sets the callback run when the file behind sample `S` ends; returns the
// previous one (NULL for a NULL `S`). Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5180
AILSAMPLECB __cdecl AIL_register_EOF_callback(HSAMPLE S, AILSAMPLECB EOFILE)
{
    AILSAMPLECB result;

    START
    {
        oprintf2("AIL_register_EOF_callback(0x%X,0x%X)\r\n", (S32)S, (S32)EOFILE);
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

// Stores `value` in `S`'s user data slot `index` (0-7, unchecked). Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5250
void __cdecl AIL_set_sample_user_data(HSAMPLE S, U32 index, S32 value)
{
    START
    {
        oprintf3("AIL_set_sample_user_data(0x%X,%u,%d)\r\n", (S32)S, index, value);
    }
    AIL_API_set_sample_user_data(S, index, value);
    END;
}

// Returns `S`'s user data slot `index` (0-7, unchecked), 0 for NULL. Logged
// when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c52e0
S32 __cdecl AIL_sample_user_data(HSAMPLE S, U32 index)
{
    S32 result;

    START
    {
        oprintf2("AIL_sample_user_data(0x%X,%u)\r\n", (S32)S, index);
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

// Returns the number of `dig`'s samples that are playing. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c53b0
S32 __cdecl AIL_active_sample_count(HDIGDRIVER dig)
{
    S32 result;

    START
    {
        oprintf1("AIL_active_sample_count(0x%X)\r\n", (S32)dig);
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

// Installs the MIDI driver and I/O settings named in MDI.INI, storing it in
// `*mdi`. Returns 0 on success, 1 if MDI.INI cannot be read, 2 if the install
// fails. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5470
S32 __cdecl AIL_install_MDI_INI(HMDIDRIVER *mdi)
{
    S32 result;

    START
    {
        oprintf1("AIL_install_MDI_INI(0x%X)\r\n", (S32)mdi);
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

// Loads .MDI driver file `filename` (also tried in the driver directory) and
// installs it with I/O settings `IO` (NULL to detect). Returns NULL on failure.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5530
HMDIDRIVER __cdecl AIL_install_MDI_driver_file(char *filename, IO_PARMS *IO)
{
    HMDIDRIVER result;

    START
    {
        oprintf2("AIL_install_MDI_driver_file(%s,0x%X)\r\n", (S32)filename, (S32)IO);
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

// Uninstalls MIDI driver `mdi` and frees everything it owns. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5600
void __cdecl AIL_uninstall_MDI_driver(HMDIDRIVER mdi)
{
    START
    {
        oprintf1("AIL_uninstall_MDI_driver(0x%X)\r\n", (S32)mdi);
    }
    AIL_API_uninstall_MDI_driver(mdi);
    END;
}

// Returns a free sequence of `mdi`, reset and with no end-of-sequence callback,
// or NULL with AIL_error set if all are in use. Logged when MSS_DEBUG tracing
// is on.
// FUNCTION: 0x000c5680
HSEQUENCE __cdecl AIL_allocate_sequence_handle(HMDIDRIVER mdi)
{
    HSEQUENCE result;

    START
    {
        oprintf1("AIL_allocate_sequence_handle(0x%X)\r\n", (S32)mdi);
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

// Stops sequence `S` and returns it to its driver's pool. NULL is ignored.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5740
void __cdecl AIL_release_sequence_handle(HSEQUENCE S)
{
    START
    {
        oprintf1("AIL_release_sequence_handle(0x%X)\r\n", (S32)S);
    }
    AIL_API_release_sequence_handle(S);
    END;
}

// Prepares `S` to play sequence `sequence_num` of XMIDI image `start`, loading
// the timbres it needs. Returns 1, 0 for an invalid sequence, -1 if timbres
// failed. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c57c0
S32 __cdecl AIL_init_sequence(HSEQUENCE S, void *start, S32 sequence_num)
{
    S32 result;

    START
    {
        oprintf3("AIL_init_sequence(0x%X,0x%X,%d)\r\n", (S32)S, (S32)start, sequence_num);
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

// Plays `S` from the start. Ignored for a free sequence. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c5890
void __cdecl AIL_start_sequence(HSEQUENCE S)
{
    START
    {
        oprintf1("AIL_start_sequence(0x%X)\r\n", (S32)S);
    }
    AIL_API_start_sequence(S);
    END;
}

// Pauses `S` if it is playing: ends its notes, releases sustain, voice
// protection and channel protection, and gives back channels it locked. Logged
// when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5910
void __cdecl AIL_stop_sequence(HSEQUENCE S)
{
    START
    {
        oprintf1("AIL_stop_sequence(0x%X)\r\n", (S32)S);
    }
    AIL_API_stop_sequence(S);
    END;
}

// Resumes `S` if it is paused: locks channels again where it held them and
// resends every channel's state. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5990
void __cdecl AIL_resume_sequence(HSEQUENCE S)
{
    START
    {
        oprintf1("AIL_resume_sequence(0x%X)\r\n", (S32)S);
    }
    AIL_API_resume_sequence(S);
    END;
}

// Ends `S` now unless it is free or already done, calling its end-of-sequence
// callback. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5a10
void __cdecl AIL_end_sequence(HSEQUENCE S)
{
    START
    {
        oprintf1("AIL_end_sequence(0x%X)\r\n", (S32)S);
    }
    AIL_API_end_sequence(S);
    END;
}

// Moves `S`'s tempo to `tempo` percent over `milliseconds` (0 = at once).
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5a90
void __cdecl AIL_set_sequence_tempo(HSEQUENCE S, S32 tempo, S32 milliseconds)
{
    START
    {
        oprintf3("AIL_set_sequence_tempo(0x%X,%d,%d)\r\n", (S32)S, tempo, milliseconds);
    }
    AIL_API_set_sequence_tempo(S, tempo, milliseconds);
    END;
}

// Moves `S`'s volume to `volume` (0-127) over `milliseconds` (0 = at once).
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5b20
void __cdecl AIL_set_sequence_volume(HSEQUENCE S, S32 volume, S32 milliseconds)
{
    START
    {
        oprintf3("AIL_set_sequence_volume(0x%X,%d,%d)\r\n", (S32)S, volume, milliseconds);
    }
    AIL_API_set_sequence_volume(S, volume, milliseconds);
    END;
}

// Sets how many times `S` plays to `loop_count` (0 = forever). Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5bb0
void __cdecl AIL_set_sequence_loop_count(HSEQUENCE S, S32 loop_count)
{
    START
    {
        oprintf2("AIL_set_sequence_loop_count(0x%X,%d)\r\n", (S32)S, loop_count);
    }
    AIL_API_set_sequence_loop_count(S, loop_count);
    END;
}

// Returns `S`'s SEQ_... status, 0 for NULL. Logged when MSS_DEBUG tracing is
// on.
// FUNCTION: 0x000c5c30
U32 __cdecl AIL_sequence_status(HSEQUENCE S)
{
    U32 result;

    START
    {
        oprintf1("AIL_sequence_status(0x%X)\r\n", (S32)S);
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

// Returns `S`'s current tempo in percent, 0 for NULL. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c5cf0
S32 __cdecl AIL_sequence_tempo(HSEQUENCE S)
{
    S32 result;

    START
    {
        oprintf1("AIL_sequence_tempo(0x%X)\r\n", (S32)S);
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

// Returns `S`'s current volume, 0 for NULL. Logged when MSS_DEBUG tracing is
// on.
// FUNCTION: 0x000c5db0
S32 __cdecl AIL_sequence_volume(HSEQUENCE S)
{
    S32 result;

    START
    {
        oprintf1("AIL_sequence_volume(0x%X)\r\n", (S32)S);
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

// Returns how many more times `S` plays (0 = forever), -1 for NULL. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5e70
S32 __cdecl AIL_sequence_loop_count(HSEQUENCE S)
{
    S32 result;

    START
    {
        oprintf1("AIL_sequence_loop_count(0x%X)\r\n", (S32)S);
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

// Sets `mdi`'s master volume (0-127) and resends the volume of every playing
// sequence. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5f30
void __cdecl AIL_set_XMIDI_master_volume(HMDIDRIVER mdi, S32 master_volume)
{
    START
    {
        oprintf2("AIL_set_XMIDI_master_volume(0x%X,%d)\r\n", (S32)mdi, master_volume);
    }
    AIL_API_set_XMIDI_master_volume(mdi, master_volume);
    END;
}

// Returns `mdi`'s master volume. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c5fb0
S32 __cdecl AIL_XMIDI_master_volume(HMDIDRIVER mdi)
{
    S32 result;

    START
    {
        oprintf1("AIL_XMIDI_master_volume(0x%X)\r\n", (S32)mdi);
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

// Installs the .MDI driver image `driver_image` of `size` bytes and sets it up
// with I/O settings `IO` (NULL to detect). Returns the driver, or NULL with
// AIL_error set. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c6070
HMDIDRIVER __cdecl AIL_install_MDI_driver_image(void *driver_image, U32 size, IO_PARMS *IO)
{
    HMDIDRIVER result;

    START
    {
        oprintf3("AIL_install_MDI_driver_image(0x%X,%u,0x%X)\r\n", (S32)driver_image, size, (S32)IO);
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

// Returns the kind of synthesizer `mdi` drives: 0 General MIDI, 1 OPL2, 2 OPL3,
// 3 Tandy or PC speaker. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c6140
S32 __cdecl AIL_MDI_driver_type(HMDIDRIVER mdi)
{
    S32 result;

    START
    {
        oprintf1("AIL_MDI_driver_type(0x%X)\r\n", (S32)mdi);
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

// Sets the GTL file name prefix to `prefix`, dropping a trailing '\' or a file
// extension. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c6200
void __cdecl AIL_set_GTL_filename_prefix(char *prefix)
{
    START
    {
        oprintf1("AIL_set_GTL_filename_prefix(%s)\r\n", (S32)prefix);
    }
    AIL_API_set_GTL_filename_prefix(prefix);
    END;
}

// Returns the driver's status for timbre `bank`/`patch` (MDI_GET_T_STATUS),
// nonzero if it is installed. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c6280
S32 __cdecl AIL_timbre_status(HMDIDRIVER mdi, S32 bank, S32 patch)
{
    S32 result;

    START
    {
        oprintf3("AIL_timbre_status(0x%X,%d,%d)\r\n", (S32)mdi, bank, patch);
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

// Has `mdi`'s driver load timbre `bank`/`patch`, unless the timbre callback
// supplies it. Returns nonzero on success; 0 with AIL_error set otherwise.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c6350
S32 __cdecl AIL_install_timbre(HMDIDRIVER mdi, S32 bank, S32 patch)
{
    S32 result;

    START
    {
        oprintf3("AIL_install_timbre(0x%X,%d,%d)\r\n", (S32)mdi, bank, patch);
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

// Protects timbre `bank`/`patch` in `mdi`'s driver from being swapped out.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c6420
void __cdecl AIL_protect_timbre(HMDIDRIVER mdi, S32 bank, S32 patch)
{
    START
    {
        oprintf3("AIL_protect_timbre(0x%X,%d,%d)\r\n", (S32)mdi, bank, patch);
    }
    AIL_API_protect_timbre(mdi, bank, patch);
    END;
}

// Lets `mdi`'s driver swap out timbre `bank`/`patch` again. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c64b0
void __cdecl AIL_unprotect_timbre(HMDIDRIVER mdi, S32 bank, S32 patch)
{
    START
    {
        oprintf3("AIL_unprotect_timbre(0x%X,%d,%d)\r\n", (S32)mdi, bank, patch);
    }
    AIL_API_unprotect_timbre(mdi, bank, patch);
    END;
}

// Returns the number of `mdi`'s sequences that are playing. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c6540
S32 __cdecl AIL_active_sequence_count(HMDIDRIVER mdi)
{
    S32 result;

    START
    {
        oprintf1("AIL_active_sequence_count(0x%X)\r\n", (S32)mdi);
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

// Returns the value `S` last set for controller `controller_num` on `channel`
// (1-16), -1 if none or for NULL. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c6600
S32 __cdecl AIL_controller_value(HSEQUENCE S, S32 channel, S32 controller_num)
{
    S32 result;

    START
    {
        oprintf3("AIL_controller_value(0x%X,%d,%d)\r\n", (S32)S, channel, controller_num);
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

// Returns the number of notes `S` has playing on `channel` (1-16), counting
// only the first 16 entries of its note queue; 0 for NULL. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c66d0
S32 __cdecl AIL_channel_notes(HSEQUENCE S, S32 channel)
{
    S32 result;

    START
    {
        oprintf2("AIL_channel_notes(0x%X,%d)\r\n", (S32)S, channel);
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

// Stores `S`'s current beat and measure (MDI_QUANT_ADVANCE intervals ahead) in
// `*beat` and `*measure`; NULL ones are skipped. Logged when MSS_DEBUG tracing
// is on.
// FUNCTION: 0x000c67a0
void __cdecl AIL_sequence_position(HSEQUENCE S, S32 *beat, S32 *measure)
{
    START
    {
        oprintf3("AIL_sequence_position(0x%X,0x%X,0x%X)\r\n", (S32)S, (S32)beat, (S32)measure);
    }
    AIL_API_sequence_position(S, beat, measure);
    RESULT
    {
        INDENT;
        oprintf2("Result = %d:%d\r\n", *measure + 1, *beat + 1);
    }
    END;
}

// Jumps `S` to branch point `marker` of its RBRN chunk, if present. FOR loops
// are cancelled unless MDI_ALLOW_LOOP_BRANCHING is set. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c6870
void __cdecl AIL_branch_index(HSEQUENCE S, U32 marker)
{
    START
    {
        oprintf2("AIL_branch_index(0x%X,%u)\r\n", (S32)S, marker);
    }
    AIL_API_branch_index(S, marker);
    END;
}

// Sets `S`'s callback-prefix controller callback; returns the previous one.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c68f0
AILPREFIXCB __cdecl AIL_register_prefix_callback(HSEQUENCE S, AILPREFIXCB callback)
{
    AILPREFIXCB result;

    START
    {
        oprintf2("AIL_register_prefix_callback(0x%X,0x%X)\r\n", (S32)S, (S32)callback);
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

// Sets `S`'s callback-trigger controller callback; returns the previous one.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c69c0
AILTRIGGERCB __cdecl AIL_register_trigger_callback(HSEQUENCE S, AILTRIGGERCB callback)
{
    AILTRIGGERCB result;

    START
    {
        oprintf2("AIL_register_trigger_callback(0x%X,0x%X)\r\n", (S32)S, (S32)callback);
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

// Sets `S`'s end-of-sequence callback; returns the previous one. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c6a90
AILSEQUENCECB __cdecl AIL_register_sequence_callback(HSEQUENCE S, AILSEQUENCECB callback)
{
    AILSEQUENCECB result;

    START
    {
        oprintf2("AIL_register_sequence_callback(0x%X,0x%X)\r\n", (S32)S, (S32)callback);
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

// Sets `S`'s beat/bar callback; returns the previous one. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c6b60
AILBEATCB __cdecl AIL_register_beat_callback(HSEQUENCE S, AILBEATCB callback)
{
    AILBEATCB result;

    START
    {
        oprintf2("AIL_register_beat_callback(0x%X,0x%X)\r\n", (S32)S, (S32)callback);
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

// Sets the callback that sees each MIDI event `mdi` is about to send and can
// claim it by returning nonzero; returns the previous one. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c6c30
AILEVENTCB __cdecl AIL_register_event_callback(HMDIDRIVER mdi, AILEVENTCB callback)
{
    AILEVENTCB result;

    START
    {
        oprintf2("AIL_register_event_callback(0x%X,0x%X)\r\n", (S32)mdi, (S32)callback);
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

// Sets the callback that can supply timbres for `mdi` (by returning nonzero)
// instead of the driver; returns the previous one. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c6d00
AILTIMBRECB __cdecl AIL_register_timbre_callback(HMDIDRIVER mdi, AILTIMBRECB callback)
{
    AILTIMBRECB result;

    START
    {
        oprintf2("AIL_register_timbre_callback(0x%X,0x%X)\r\n", (S32)mdi, (S32)callback);
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

// Stores `value` in `S`'s user data slot `index` (0-7, unchecked). Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c6dd0
void __cdecl AIL_set_sequence_user_data(HSEQUENCE S, U32 index, S32 value)
{
    START
    {
        oprintf3("AIL_set_sequence_user_data(0x%X,%u,%d)\r\n", (S32)S, index, value);
    }
    AIL_API_set_sequence_user_data(S, index, value);
    END;
}

// Returns `S`'s user data slot `index` (0-7, unchecked), 0 for NULL. Logged
// when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c6e60
S32 __cdecl AIL_sequence_user_data(HSEQUENCE S, U32 index)
{
    S32 result;

    START
    {
        oprintf2("AIL_sequence_user_data(0x%X,%u)\r\n", (S32)S, index);
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

// Sets the array the indirect controller prefix of `S` reads values from.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c6f30
void __cdecl AIL_register_ICA_array(HSEQUENCE S, U8 *array)
{
    START
    {
        oprintf2("AIL_register_ICA_array(0x%X,0x%X)\r\n", (S32)S, (S32)array);
    }
    AIL_API_register_ICA_array(S, array);
    END;
}

// Locks a physical channel (2-9) of `mdi` for the application and silences it.
// Returns its number (1-based), or 0 if none is free. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c6fb0
S32 __cdecl AIL_lock_channel(HMDIDRIVER mdi)
{
    S32 result;

    START
    {
        oprintf1("AIL_lock_channel(0x%X)\r\n", (S32)mdi);
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

// Releases physical channel `channel` (1-based) locked by AIL_lock_channel:
// silences it and restores its previous owner's settings. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c7070
void __cdecl AIL_release_channel(HMDIDRIVER mdi, S32 channel)
{
    START
    {
        oprintf2("AIL_release_channel(0x%X,%d)\r\n", (S32)mdi, channel);
    }
    AIL_API_release_channel(mdi, channel);
    END;
}

// Routes `S`'s logical channel `seq_channel` to physical channel `new_channel`
// (both 1-based). Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c70f0
void __cdecl AIL_map_sequence_channel(HSEQUENCE S, S32 seq_channel, S32 new_channel)
{
    START
    {
        oprintf3("AIL_map_sequence_channel(0x%X,%d,%d)\r\n", (S32)S, seq_channel, new_channel);
    }
    AIL_API_map_sequence_channel(S, seq_channel, new_channel);
    END;
}

// Returns the physical channel (1-based) `S`'s logical channel `seq_channel` is
// routed to, 0 for NULL. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7180
S32 __cdecl AIL_true_sequence_channel(HSEQUENCE S, S32 seq_channel)
{
    S32 result;

    START
    {
        oprintf2("AIL_true_sequence_channel(0x%X,%d)\r\n", (S32)S, seq_channel);
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

// Sends a channel message now: through sequence `S`'s mapping and logic if `S`
// is given, otherwise straight to `mdi` (volume scaled by the master volume,
// the event callback consulted). Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7250
void __cdecl AIL_send_channel_voice_message(HMDIDRIVER mdi, HSEQUENCE S, S32 status, S32 data_1, S32 data_2)
{
    START
    {
        oprintf5("AIL_send_channel_voice_message(0x%X,0x%X,0x%X,0x%X,0x%X)\r\n", (S32)mdi, (S32)S, status, data_1, data_2);
    }
    AIL_API_send_channel_voice_message(mdi, S, status, data_1, data_2);
    END;
}

// Sends the system exclusive message at `buffer` (status byte, length, data) to
// `mdi`'s driver now. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c72f0
void __cdecl AIL_send_sysex_message(HMDIDRIVER mdi, void *buffer)
{
    START
    {
        oprintf2("AIL_send_sysex_message(0x%X,0x%X)\r\n", (S32)mdi, (S32)buffer);
    }
    AIL_API_send_sysex_message(mdi, buffer);
    END;
}

// Plays `mdi`'s notes found in wave library `wave_lib` as samples on `dig`, up
// to `polyphony` (at most 16) at once. Returns NULL on failure. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7370
HWAVESYNTH __cdecl AIL_create_wave_synthesizer(HDIGDRIVER dig, HMDIDRIVER mdi, void *wave_lib, S32 polyphony)
{
    HWAVESYNTH result;

    START
    {
        oprintf4("AIL_create_wave_synthesizer(0x%X,0x%X,0x%X,%d)\r\n", (S32)dig, (S32)mdi, (S32)wave_lib, polyphony);
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

// Releases wave synthesizer `W`'s samples, restores the MIDI driver's previous
// event and timbre callbacks and frees it. NULL is ignored. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7450
void __cdecl AIL_destroy_wave_synthesizer(HWAVESYNTH W)
{
    START
    {
        oprintf1("AIL_destroy_wave_synthesizer(0x%X)\r\n", (S32)W);
    }
    AIL_API_destroy_wave_synthesizer(W);
    END;
}

// Opens CD-ROM drive `which` (0 first) for Red Book audio. Returns its handle,
// or NULL if there is no such drive or memory runs out. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c74d0
HREDBOOK __cdecl AIL_redbook_open(U32 which)
{
    HREDBOOK result;

    START
    {
        oprintf1("AIL_redbook_open(%ld)\r\n", which);
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

// Closes Red Book handle `hand`; NULL is ignored. Logged when MSS_DEBUG tracing
// is on.
// FUNCTION: 0x000c7590
void __cdecl AIL_redbook_close(HREDBOOK hand)
{
    START
    {
        oprintf1("AIL_redbook_close(0x%lX)\r\n", (S32)hand);
    }
    AIL_API_redbook_close(hand);
    END;
}

// Ejects the disc in `hand`'s drive and cancels any pause; NULL is ignored.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7610
void __cdecl AIL_redbook_eject(HREDBOOK hand)
{
    START
    {
        oprintf1("AIL_redbook_eject(0x%lX)\r\n", (S32)hand);
    }
    AIL_API_redbook_eject(hand);
    END;
}

// Closes the tray of `hand`'s drive and cancels any pause; NULL is ignored.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7690
void __cdecl AIL_redbook_retract(HREDBOOK hand)
{
    START
    {
        oprintf1("AIL_redbook_retract(0x%lX)\r\n", (S32)hand);
    }
    AIL_API_redbook_retract(hand);
    END;
}

// Returns `hand`'s state: REDBOOK_PLAYING, REDBOOK_PAUSED, REDBOOK_STOPPED or
// REDBOOK_ERROR (also for NULL or no disc). Logged when MSS_DEBUG tracing is
// on.
// FUNCTION: 0x000c7710
U32 __cdecl AIL_redbook_status(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1("AIL_redbook_status(0x%lX)\r\n", (S32)hand);
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

// Returns the number of audio tracks on `hand`'s disc, 0 on error. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c77d0
U32 __cdecl AIL_redbook_tracks(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1("AIL_redbook_tracks(0x%lX)\r\n", (S32)hand);
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

// Stores track `tracknum`'s (1-based) start and end in ms in `*startmsec` and
// `*endmsec` (either may be NULL); both get 0 for an invalid track or an error.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7890
void __cdecl AIL_redbook_track_info(HREDBOOK hand, U32 tracknum, U32 *startmsec, U32 *endmsec)
{
    START
    {
        oprintf4("AIL_redbook_track_info(0x%lX, %ld, 0x%lX, 0x%lX)\r\n", (S32)hand, tracknum, (S32)startmsec, (S32)endmsec);
    }
    AIL_API_redbook_track_info(hand, tracknum, startmsec, endmsec);
    END;
}

// Returns an ID for `hand`'s disc hashed from its track count and track times,
// or 0 on error. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7920
U32 __cdecl AIL_redbook_id(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1("AIL_redbook_id(0x%lX)\r\n", (S32)hand);
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

// Returns the current play position in ms while playing, otherwise 0. Logged
// when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c79e0
U32 __cdecl AIL_redbook_position(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1("AIL_redbook_position(0x%lX)\r\n", (S32)hand);
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

// Returns the track being played, or 0 when not playing. Logged when MSS_DEBUG
// tracing is on.
// FUNCTION: 0x000c7aa0
U32 __cdecl AIL_redbook_track(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1("AIL_redbook_track(0x%lX)\r\n", (S32)hand);
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

// Stops any play or pause, then plays from `startmsec` to `endmsec` unless both
// are 0. Returns the new status. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7b60
U32 __cdecl AIL_redbook_play(HREDBOOK hand, U32 startmsec, U32 endmsec)
{
    U32 result;

    START
    {
        oprintf3("AIL_redbook_play(0x%lX, %ld, %ld)\r\n", (S32)hand, startmsec, endmsec);
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

// Stops play and cancels any pause. Returns the new status. Logged when
// MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7c30
U32 __cdecl AIL_redbook_stop(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1("AIL_redbook_stop(0x%lX)\r\n", (S32)hand);
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

// Pauses play, remembering the position for AIL_redbook_resume. Returns the new
// status. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7cf0
U32 __cdecl AIL_redbook_pause(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1("AIL_redbook_pause(0x%lX)\r\n", (S32)hand);
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

// Resumes a paused disc from where it paused up to the end of the last play.
// Returns the new status. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7db0
U32 __cdecl AIL_redbook_resume(HREDBOOK hand)
{
    U32 result;

    START
    {
        oprintf1("AIL_redbook_resume(0x%lX)\r\n", (S32)hand);
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

// Unloads every Quick API file and shuts the library down; does nothing if no
// driver was installed. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7e70
void __cdecl AIL_quick_shutdown(void)
{
    START
    {
        AIL_DEBUGOUTSTR("AIL_quick_shutdown()\r\n");
    }
    AIL_API_quick_shutdown();
    END;
}

// Loads VOC, WAV or XMIDI file `filename` for the Quick API. Returns its
// HAUDIO, or NULL with AIL_error set. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7ed0
HAUDIO __cdecl AIL_quick_load(char *filename)
{
    HAUDIO result;

    START
    {
        oprintf1("AIL_quick_load(%s)\r\n", (S32)filename);
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

// Stops `audio` if it is playing and frees it and its data, unlinking it from
// the loaded list. NULL is ignored. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c7f90
void __cdecl AIL_quick_unload(HAUDIO audio)
{
    START
    {
        oprintf1("AIL_quick_unload(0x%X)\r\n", (S32)audio);
    }
    AIL_API_quick_unload(audio);
    END;
}

// Plays `audio` from the start (restarting it if playing) `loop_count` times, 0
// meaning forever, on a newly allocated sequence or sample handle. Returns 1 if
// it started, 0 otherwise. Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c8010
S32 __cdecl AIL_quick_play(HAUDIO audio, U32 loop_count)
{
    S32 result;

    START
    {
        oprintf2("AIL_quick_play(0x%X,%d)\r\n", (S32)audio, loop_count);
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

// Stops `audio` if it is playing and releases its handle. NULL is ignored.
// Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c80e0
void __cdecl AIL_quick_halt(HAUDIO audio)
{
    START
    {
        oprintf1("AIL_quick_halt(0x%X)\r\n", (S32)audio);
    }
    AIL_API_quick_halt(audio);
    END;
}

// Returns `audio`'s state: QSTAT_LOADED, QSTAT_PLAYING or QSTAT_DONE (also for
// NULL). Logged when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c8160
S32 __cdecl AIL_quick_status(HAUDIO audio)
{
    S32 result;

    START
    {
        oprintf1("AIL_quick_status(0x%X)\r\n", (S32)audio);
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

// Loads and plays file `filename` `loop_count` times, waiting for the end if
// `wait_request` is set and it is finite. Returns the HAUDIO, or NULL. Logged
// when MSS_DEBUG tracing is on.
// FUNCTION: 0x000c8220
HAUDIO __cdecl AIL_quick_load_and_play(char *filename, U32 loop_count, S32 wait_request)
{
    HAUDIO result;

    START
    {
        oprintf3("AIL_quick_load_and_play(%s,%d,%d)\r\n", (S32)filename, loop_count, wait_request);
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

// Unlocks this module's code. Its address also marks the end of the locked code
// range.
// FUNCTION: 0x000c82f0
void __pascal AILDEBUG_END(void)
{
    if (locked) {
        AIL_vmm_unlock_range(AILDEBUG_START, AILDEBUG_END);
        locked = 0;
    }
}
