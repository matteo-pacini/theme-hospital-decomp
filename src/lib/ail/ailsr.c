#include "ail.h"

struct tm;
typedef long time_t;

char *getenv(char const *name);
time_t time(time_t *t);
struct tm *localtime(time_t const *t);
char *asctime(struct tm const *tm);
int int386(int intno, union REGS *in, union REGS *out);
int sprintf(char *buf, char const *fmt, ...);

extern S32 locked;
extern char D_0018d454[32];

extern char D_00128018[];
extern char D_00128024[];
extern char D_00128034[];
extern char D_00128088[];
extern char D_001280c4[];
extern char D_001280d4[];
extern char D_0012812c[];

void __cdecl AIL_API_startup(void);
void AILDEBUG_START(void);
#pragma aux AILDEBUG_START "AILDEBUG_START";

void *__pascal fn_000efba0(U32 size);
void __pascal fn_000efbb0(void *ptr);
int __pascal AIL_INT386(int intno, union REGS *in, union REGS *out);
void __pascal AIL_SPRINTF(char *dest, char const *fmt, S32 nargs, S32 *args);
void AILSR_end(void);

// FUNCTION: 0x000efb60
// NONMATCHING: the target names AILSR_end _AIL_startup_reg+0x170 (see below)
void AILSR_start(void)
{
    if (!locked) {
        AIL_vmm_lock_range(AILSR_start, AILSR_end);
        locked = 1;
        AILDEBUG_START();
    }
}

// FUNCTION: 0x000efbc0
// NONMATCHING: config/functions.txt sizes this function to also cover
// AIL_INT386 and AIL_SPRINTF, which follow it
char *__pascal AIL_GETENV(char const *name)
{
    char *value;

    value = getenv(name);
    return value;
}

int __pascal AIL_INT386(int intno, union REGS *in, union REGS *out)
{
    return int386(intno, in, out);
}

void __pascal AIL_SPRINTF(char *dest, char const *fmt, S32 nargs, S32 *args)
{
    switch (nargs) {
    case 0:
        sprintf(dest, fmt);
        break;
    case 1:
        sprintf(dest, fmt, args[0]);
        break;
    case 2:
        sprintf(dest, fmt, args[0], args[1]);
        break;
    case 3:
        sprintf(dest, fmt, args[0], args[1], args[2]);
        break;
    case 4:
        sprintf(dest, fmt, args[0], args[1], args[2], args[3]);
        break;
    case 5:
        sprintf(dest, fmt, args[0], args[1], args[2], args[3], args[4]);
        break;
    }
}

// FUNCTION: 0x000efcc0
// NONMATCHING: config/functions.txt sizes this function to also cover
// AILSR_end and two C library helpers that follow it
void __cdecl AIL_startup_reg(void)
{
    time_t t;
    char *debug;

    if (AIL_started) {
        return;
    }
    AIL_mem_alloc = fn_000efba0;
    AIL_mem_free = fn_000efbb0;
    AIL_getenv = AIL_GETENV;
    AIL_int386 = AIL_INT386;
    AIL_sprintf = AIL_SPRINTF;
    AILSR_start();
    AIL_debug = 0;
    AIL_sys_debug = 0;
    debug = AIL_GETENV(D_00128018);
    if (debug == NULL) {
        AIL_API_startup();
        AIL_started = 1;
        return;
    }
    if (AIL_GETENV(D_00128024) != NULL) {
        AIL_sys_debug = 1;
    }
    AIL_debugfile = AIL_fappend(debug);
    if (AIL_debugfile == -1) {
        AIL_API_startup();
        AIL_started = 1;
        return;
    }
    time(&t);
    strcpy(D_0018d454, asctime(localtime(&t)));
    D_0018d454[24] = 0;
    AIL_DEBUGOUTSTR(D_00128034);
    AIL_DEBUGOUTSTR(D_00128088);
    AIL_DEBUGOUTSTR(D_001280c4);
    AIL_DEBUGOUTSTR(D_0018d454);
    AIL_DEBUGOUTSTR(D_001280d4);
    AIL_API_startup();
    AIL_starttime = AIL_ms_count();
    AIL_debug = 1;
    AIL_indent = 1;
    TIME_WRITE();
    AIL_indent = 0;
    AIL_DEBUGOUTSTR(D_0012812c);
    AIL_started = 1;
}

void AILSR_end(void)
{
    if (locked) {
        AIL_vmm_unlock_range(AILSR_start, AILSR_end);
        locked = 0;
    }
}
