#include "ail.h"

typedef struct {
    U32 size;
    U32 count;
    U8 *ptr;
    U8 data[256];
} INI_BUFFER;

// Module state, in the order the original lays it out.
static S32 entry = 0;
static S32 locked = 0;
IO_PARMS AIL_last_IO_attempt;
static U32 flags;


void AIL_start(void);
void AIL_end(void);
#pragma aux AIL_end "AIL_end_"
void __pascal AIL_DRIVER_SERVER(U32 user);
#pragma aux (__pascal) AIL_DRIVER_SERVER "AIL_DRIVER_SERVER"

// Locks this module's code (AIL_start up to AIL_end) and the driver table,
// preferences, error text and module state against paging, once; AIL_end undoes
// it.
// FUNCTION: 0x000debc0
void AIL_start(void)
{
    if (!locked) {
        AIL_vmm_lock_range(AIL_start, AIL_end);
        AIL_vmm_lock(AIL_driver, sizeof(AIL_driver));
        AIL_vmm_lock(AIL_preference, sizeof(AIL_preference));
        AIL_vmm_lock(AIL_error, sizeof(AIL_error));
        AIL_vmm_lock(&AIL_last_IO_attempt, sizeof(AIL_last_IO_attempt));
        AIL_vmm_lock(&entry, sizeof(entry));
        AIL_vmm_lock(&flags, sizeof(flags));
        locked = 1;
    }
}

// Timer callback of an installed driver with a service rate: `user` is its
// AIL_DRIVER. Sends it DRV_SERVE with interrupts disabled; a call arriving
// while one is still running is dropped.
// FUNCTION: 0x000dec50
void __pascal AIL_DRIVER_SERVER(U32 user)
{
    if (!entry) {
        entry = 1;
        flags = AIL_disable_interrupts();
        AIL_call_driver((AIL_DRIVER *)user, DRV_SERVE, NULL, NULL);
        AIL_restore_interrupts(flags);
        entry = 0;
    }
}

// Parses `str` as an integer in `base` (2..16, digits case-insensitive). Spaces
// are skipped and each '-' flips the sign; parsing stops at the first other
// character, and `*end` (if not NULL) is set to it. Returns the signed value.
// FUNCTION: 0x000decb0
S32 __cdecl ASC_val(char *str, S32 base, char **end)
{
    U32 i;
    S32 j;
    S32 total;
    S32 sign;
    char ch;

    total = 0;
    sign = 1;
    for (i = 0; i < strlen(str); i++) {
        ch = str[i];
        if (ch >= 'a' && ch <= 'z') {
            ch -= 'a' - 'A';
        }
        if (ch == '-') {
            sign = -sign;
            continue;
        }
        if (ch == ' ') {
            continue;
        }
        for (j = 0; j < base; j++) {
            if (ch == "0123456789ABCDEF"[j]) {
                total = total * base + j;
                break;
            }
        }
        if (j == base) {
            break;
        }
    }
    if (end != NULL) {
        *end = str + i;
    }
    return total * sign;
}

// Reads one line of `file` through `buf` into `dest`, dropping '\r' and the
// terminating '\n', and NUL-terminates it. Stops after `maxlen` characters (the
// '\n' counts), so `dest` needs `maxlen` + 1 bytes. Returns the number of
// characters read other than '\r', 0 at end of file.
// FUNCTION: 0x000ded50
U32 AIL_fgets(S32 file, INI_BUFFER *buf, char *dest, U32 maxlen)
{
    U32 n = 0;

    for (;;) {
        if (n >= maxlen)
            break;
        if (buf->count == 0) {
            buf->ptr = buf->data;
            buf->count = AIL_fread(file, buf->data, buf->size - 12);
            if (buf->count == 0)
                break;
        }
        buf->count--;
        if (*buf->ptr != '\r') {
            n++;
            if (*buf->ptr == '\n') {
                buf->ptr++;
                break;
            }
            *dest++ = *buf->ptr;
        }
        buf->ptr++;
    }
    *dest = 0;
    return n;
}

// Reads driver settings from the .INI file `filename`, looked up as given and
// then in the driver directory: DRIVER, DEVICE, IO_ADDR (hex), IRQ, DMA_8_bit
// and DMA_16_bit, one "key value" per line, ';' starting a comment. IO fields
// not given are -1. Returns 1 and fills `INI`; 0 if the file is missing or
// lacks DRIVER or DEVICE (then AIL_error says so).
// FUNCTION: 0x000dedd0
S32 __cdecl AIL_API_read_INI(AIL_INI *INI, char *filename)
{
    AIL_INI work;
    INI_BUFFER buf;
    char path[256];
    char line[80];
    S32 file;
    S32 i;
    char *key;
    char *key_end;
    char *value;

    memset(&work, 0, sizeof(work));
    memset(&work.IO, -1, sizeof(work.IO));
    file = AIL_fopen(filename, 0);
    if (file == -1) {
        if (AIL_driver_directory[0] == 0) {
            return 0;
        }
        strcpy(path, AIL_driver_directory);
        strcat(path, filename);
        file = AIL_fopen(path, 0);
        if (file == -1) {
            return 0;
        }
    }
    buf.size = sizeof(buf);
    buf.count = 0;
    while (AIL_fgets(file, &buf, line, sizeof(line))) {
        for (i = strlen(line) - 1; i >= 0; i--) {
            if (line[i] != ' ') {
                break;
            }
            line[i] = 0;
        }
        for (i = 0; i < strlen(line); i++) {
            if (line[i] != ' ') {
                break;
            }
        }
        key = &line[i];
        for (; i < strlen(line); i++) {
            if (line[i] == ' ') {
                break;
            }
        }
        key_end = &line[i];
        for (; i < strlen(line); i++) {
            if (line[i] != ' ') {
                break;
            }
        }
        value = &line[i];
        if (i >= strlen(line)) {
            continue;
        }
        *key_end = 0;
        if (key[0] == ';') {
            continue;
        }
        if (!strnicmp(key, "DRIVER", 7)) {
            strcpy(work.driver_name, value);
        } else if (!strnicmp(key, "DEVICE", 7)) {
            strcpy(work.device_name, value);
        } else if (!strnicmp(key, "IO_ADDR", 8)) {
            work.IO.IO = ASC_val(value, 16, NULL);
        } else if (!strnicmp(key, "IRQ", 4)) {
            work.IO.IRQ = ASC_val(value, 10, NULL);
        } else if (!strnicmp(key, "DMA_8_bit", 10)) {
            work.IO.DMA_8_bit = ASC_val(value, 10, NULL);
        } else if (!strnicmp(key, "DMA_16_bit", 11)) {
            work.IO.DMA_16_bit = ASC_val(value, 10, NULL);
        }
    }
    AIL_fclose(file);
    if (strlen(work.driver_name) == 0 || strlen(work.device_name) == 0) {
        strcpy(AIL_error, "Corrupted .INI file\n");
        return 0;
    }
    *INI = work;
    return 1;
}

// Sets preference `number` (DIG_..., MDI_..., AIL_...) to `value`; returns its
// previous value.
// FUNCTION: 0x000df270
S32 __cdecl AIL_API_set_preference(U32 number, S32 value)
{
    S32 old;

    old = AIL_preference[number];
    AIL_preference[number] = value;
    return old;
}

// Returns the text describing the last error.
// FUNCTION: 0x000df290
char *__cdecl AIL_last_error(void)
{
    return AIL_error;
}

// Loads a .DIG or .MDI driver image of `n_bytes` into DOS memory, gives it one
// of the 16 driver handles, hooks real-mode INT 66h to it, sends DRV_INIT and,
// if the driver asks for one, starts a timer at its service rate. Returns the
// new driver, or NULL with AIL_error set.
// FUNCTION: 0x000df2a0
AIL_DRIVER *__cdecl AIL_API_install_driver(U8 *driver_image, U32 n_bytes)
{
    AIL_DRIVER *drvr;
    U32 i;

    drvr = AIL_mem_alloc_lock(sizeof(AIL_DRIVER));
    if (drvr == NULL) {
        strcpy(AIL_error, "Insufficient memory for driver descriptor\n");
        return NULL;
    }
    drvr->size = n_bytes;
    if (!AIL_mem_alloc_DOS((n_bytes + 15) / 16, &drvr->buf, &drvr->seg,
                           &drvr->sel)) {
        strcpy(AIL_error, "Insufficient low memory\n");
        AIL_mem_free_lock(drvr);
        return NULL;
    }
    memcpy(drvr->buf, driver_image, n_bytes);
    drvr->VHDR = drvr->buf;
    if (!strnicmp(drvr->VHDR->ID, "AIL3DIG", 7)) {
        drvr->type = DIG_DRIVER;
    } else if (!strnicmp(drvr->VHDR->ID, "AIL3MDI", 7)) {
        drvr->type = MDI_DRIVER;
    } else {
        strcpy(AIL_error, "Invalid driver type\n");
        AIL_mem_free_DOS(drvr->buf, drvr->seg, drvr->sel);
        AIL_mem_free_lock(drvr);
        return NULL;
    }
    for (i = 0; i < 16; i++) {
        if (AIL_driver[i] == NULL) {
            AIL_driver[i] = drvr;
            drvr->VHDR->driver_num = i;
            break;
        }
    }
    if (i == 16) {
        strcpy(AIL_error, "Out of driver handles\n");
        AIL_mem_free_DOS(drvr->buf, drvr->seg, drvr->sel);
        AIL_mem_free_lock(drvr);
        return NULL;
    }
    drvr->VHDR->busy = 0;
    drvr->initialized = 0;
    drvr->PM_ISR = -1;
    drvr->VHDR->prev_ISR = AIL_get_real_vect(0x66);
    AIL_set_real_vect(0x66, drvr->seg + drvr->VHDR->this_ISR);
    drvr->destructor = NULL;
    drvr->descriptor = NULL;
    AIL_call_driver(drvr, DRV_INIT, NULL, NULL);
    if (drvr->VHDR->service_rate <= 0) {
        drvr->server = -1;
    } else {
        drvr->server = AIL_register_timer(AIL_DRIVER_SERVER);
        if (drvr->server == -1) {
            strcpy(AIL_error, "Out of timer handles\n");
            AIL_set_real_vect(0x66, drvr->VHDR->prev_ISR);
            AIL_mem_free_DOS(drvr->buf, drvr->seg, drvr->sel);
            AIL_mem_free_lock(drvr);
            return NULL;
        }
        AIL_set_timer_user(drvr->server, (U32)drvr);
        AIL_set_timer_frequency(drvr->server, drvr->VHDR->service_rate);
        AIL_start_timer(drvr->server);
    }
    return drvr;
}

// Releases driver `drvr`: runs its destructor, stops its timer, shuts it down
// (DRV_SHUTDOWN) if initialized, restores INT 66h and frees its memory and
// handle.
// FUNCTION: 0x000df580
void __cdecl AIL_API_uninstall_driver(AIL_DRIVER *drvr)
{
    S32 i;

    if (drvr->destructor != NULL) {
        drvr->destructor(drvr->descriptor);
    }
    if (drvr->server != -1) {
        AIL_release_timer_handle(drvr->server);
    }
    if (drvr->initialized) {
        if (drvr->PM_ISR != -1) {
            AIL_restore_USE16_ISR(drvr->PM_ISR);
        }
        AIL_call_driver(drvr, DRV_SHUTDOWN, NULL, NULL);
    }
    AIL_set_real_vect(0x66, drvr->VHDR->prev_ISR);
    AIL_mem_free_DOS(drvr->buf, drvr->seg, drvr->sel);
    AIL_mem_free_lock(drvr);
    for (i = 0; i < 16; i++) {
        if (AIL_driver[i] == drvr) {
            AIL_driver[i] = NULL;
        }
    }
}

// Gets driver `drvr`'s I/O settings from the environment variable its header
// names, parsed by the driver (DRV_PARSE_ENV). Returns them in a static buffer
// overwritten by the next call, or NULL if the driver names no variable, it is
// not set or the driver rejects it.
// FUNCTION: 0x000df630
IO_PARMS *__cdecl AIL_API_get_IO_environment(AIL_DRIVER *drvr)
{
    static IO_PARMS IO;
    char *env;
    S32 len;
    U32 name;

    name = (drvr->VHDR->environment_string & 0xffff)
           + (drvr->VHDR->environment_string >> 16) * 16;
    if (name == 0) {
        return NULL;
    }
    env = AIL_getenv((char *)name);
    if (env == NULL) {
        return NULL;
    }
    len = strlen(env);
    if (len > 127) {
        len = 127;
    }
    memcpy(drvr->VHDR->scratch, env, len);
    drvr->VHDR->scratch[len] = 0;
    if (AIL_call_driver(drvr, DRV_PARSE_ENV, NULL, NULL) == -1) {
        return NULL;
    }
    IO = drvr->VHDR->IO;
    return &IO;
}

// Initialises the library: locks its memory, sets every preference to its
// default, clears the driver table and error text, and starts the timer system.
// FUNCTION: 0x000df6e0
void __cdecl AIL_API_startup(void)
{
    S32 i;

    AIL_start();
    AIL_set_preference(0, 200);
    AIL_set_preference(1, 1);
    AIL_set_preference(2, 0x8000);
    AIL_set_preference(3, 100);
    AIL_set_preference(4, 16);
    AIL_set_preference(5, 100);
    AIL_set_preference(6, 655);
    AIL_set_preference(7, 0);
    AIL_set_preference(8, 0);
    AIL_set_preference(9, 1);
    AIL_set_preference(10, 0);
    AIL_set_preference(11, 120);
    AIL_set_preference(12, 8);
    AIL_set_preference(13, 127);
    AIL_set_preference(14, 1);
    AIL_set_preference(15, 0);
    AIL_set_preference(16, 2);
    AIL_set_preference(17, 1);
    AIL_set_preference(18, 1);
    AIL_set_preference(19, 0);
    for (i = 0; i < 16; i++) {
        AIL_driver[i] = NULL;
    }
    AIL_error[0] = 0;
    AILA_startup();
}

// Uninstalls every driver (last handle first), releases all timers and shuts
// the timer system down.
// FUNCTION: 0x000df800
void __cdecl AIL_API_shutdown(void)
{
    S32 i;
    AIL_DRIVER *drvr;

    for (i = 15; i >= 0; i--) {
        drvr = AIL_driver[i];
        if (drvr != NULL) {
            AIL_uninstall_driver(drvr);
        }
    }
    AIL_release_all_timers();
    AILA_shutdown();
}

// Sets the directory searched for drivers and .INI files to `dir`, adding a
// trailing '\' unless it ends in '\' or ':'; NULL clears it. Returns the stored
// directory.
// FUNCTION: 0x000df850
char *__cdecl AIL_API_set_driver_directory(char *dir)
{
    U32 len;

    if (dir == NULL) {
        AIL_driver_directory[0] = 0;
    } else {
        strcpy(AIL_driver_directory, dir);
        len = strlen(AIL_driver_directory);
        if (len != 0 && AIL_driver_directory[len - 1] != '\\'
            && AIL_driver_directory[len - 1] != ':') {
            AIL_driver_directory[len] = '\\';
            AIL_driver_directory[len + 1] = 0;
        }
    }
    return AIL_driver_directory;
}

// Busy-waits for `intervals` ticks of 16 ms (about 1/60 second).
// FUNCTION: 0x000df8c0
void __cdecl AIL_API_delay(S32 intervals)
{
    U32 end;

    end = AIL_ms_count() + intervals * 16;
    while (AIL_ms_count() < end) {
    }
}

// Unlocks what AIL_start locked. Its address also marks the end of the locked
// code range.
// FUNCTION: 0x000df8e0
void AIL_end(void)
{
    if (locked) {
        AIL_vmm_unlock_range(AIL_start, AIL_end);
        AIL_vmm_unlock(AIL_driver, sizeof(AIL_driver));
        AIL_vmm_unlock(AIL_preference, sizeof(AIL_preference));
        AIL_vmm_unlock(AIL_error, sizeof(AIL_error));
        AIL_vmm_unlock(&AIL_last_IO_attempt, sizeof(AIL_last_IO_attempt));
        AIL_vmm_unlock(&entry, sizeof(entry));
        AIL_vmm_unlock(&flags, sizeof(flags));
        locked = 0;
    }
}
