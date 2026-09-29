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

void *malloc(U32 size);
void free(void *ptr);
/* config/functions.txt has malloc and free under wrong demo names. */
#pragma aux malloc "_nmalloc_";
#pragma aux free "_nfree_";

/* __pascal (callee pops) with address names. */
void *fn_000efba0(U32 size);
void fn_000efbb0(void *ptr);
int AIL_INT386(int intno, union REGS *in, union REGS *out);
#pragma aux (__pascal) fn_000efba0 "*_";
#pragma aux (__pascal) fn_000efbb0 "*_";
#pragma aux (__pascal) AIL_INT386 "fn_000efbd0_";
void __pascal AIL_SPRINTF(char *dest, char const *fmt, S32 nargs, S32 *args);
void AILSR_end(void);

// FUNCTION: 0x000efb60
void AILSR_start(void)
{
    if (!locked) {
        AIL_vmm_lock_range(AILSR_start, AILSR_end);
        locked = 1;
        AILDEBUG_START();
    }
}

// FUNCTION: 0xefba0
void *fn_000efba0(U32 size)
{
    void *ptr;

    ptr = malloc(size);
    return ptr;
}

// FUNCTION: 0xefbb0
void fn_000efbb0(void *ptr)
{
    free(ptr);
}

// FUNCTION: 0x000efbc0
// NONMATCHING: the target copies getenv's result to edx and back
char *__pascal AIL_GETENV(char const *name)
{
    char *value;

    value = getenv(name);
    return value;
}

// FUNCTION: 0xefbd0
int AIL_INT386(int intno, union REGS *in, union REGS *out)
{
    return int386(intno, in, out);
}

// FUNCTION: 0x000efc00
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
// NONMATCHING: the target copies localtime's result to esi and back
void __cdecl AIL_startup_reg(void)
{
    time_t t;
    char *debug;

    if (AIL_started) {
        return;
    }
    AIL_mem_alloc = (void *(__pascal *)(U32))fn_000efba0;
    AIL_mem_free = (void (__pascal *)(void *))fn_000efbb0;
    AIL_getenv = AIL_GETENV;
    AIL_int386 = (int (__pascal *)(int, union REGS *, union REGS *))AIL_INT386;
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

// FUNCTION: 0x000efe30
void AILSR_end(void)
{
    if (locked) {
        AIL_vmm_unlock_range(AILSR_start, AILSR_end);
        locked = 0;
    }
}
