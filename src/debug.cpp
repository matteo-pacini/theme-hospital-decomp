#include "hosp.h"

typedef char *va_list[1];
#define va_start(ap, last) ((ap)[0] = (char *)&(last) + sizeof(last))
#define va_end(ap) ((ap)[0] = 0)

void note_in_error_log(char *text);
void write_to_player_log(char *text);

u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "fn_000cd716_"

extern "C" {


char *getenv(char *name);
int vsprintf(char *buf, char *fmt, va_list args);
typedef struct __iobuf FILE;
FILE *fopen(const char *name, const char *mode);
int fprintf(FILE *fp, const char *format, ...);
int fclose(FILE *fp);

}

// FUNCTION: 0x0008dcd0
u16 check_frame(u16 anim, Thing *peep)
{
    char buf[100];

    if (anim == 9999) {
        sprintf(buf, "Invalid Frame Lookup. Peep %d", (u16)(peep - thing));
        note_in_error_log(buf);
        return 0;
    }
    return anim;
}

// FUNCTION: 0x0008dd40
void nav_debug_mode(void)
{
    set_mouse_mode(MM_NAV_DEBUG);
    debug.nav_debug = 1;
}

// FUNCTION: 0x0008dd68
void write_to_player_log(char *text)
{
    char path[64];
    s32 handle;

    sprintf(path, "N:\\hosp\\log\\%s.txt", getenv("USERID"));
    handle = LbFileOpen(path, 1);
    if (handle != -1) {
        LbFileSeek(handle, 0, 2);
        LbFileWrite(handle, text, fn_000cd716(text));
        LbFileClose(handle);
    }
}

// FUNCTION: 0x0008dde4
void note_in_error_log(char *text)
{
    char buf[128];

    sprintf(buf, "NOTE: %s\n", text);
    write_to_player_log(buf);
}

// FUNCTION: 0x0008de24
extern "C" void fn_0008de24(void)
{
    if (resvars.f0 == SCREEN_MODE_13)
        resvars.f0 = SCREEN_MODE_1;
    if (!change_res(SCREEN_MODE_13))
        exit(1);
}

// FUNCTION: 0x0008de64
void debug_msg(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsprintf(debug.unknown116, fmt, args);
    va_end(args);
    debug.unknown1de = 200;
}

// FUNCTION: 0x0008dea0
extern "C" void fn_0008dea0(char *text)
{
    // Dead code: the compiler still emits its string literals, in this
    // order, and nothing else. The statements are reconstructed from those
    // literals; the extern names are placeholders, which dead code never
    // refers to (a static would take space in _BSS, a local in the frame).
    if (0) {
        extern char log_name[], log_path[];
        extern FILE *log_file;

        sprintf(log_path, "N:\\HOSP\\LOG\\%s.txt", log_name);
        log_file = fopen(log_path, "a");
        fprintf(log_file, "%s", text);
        fclose(log_file);
    }
}

// FUNCTION: 0x0008debc
extern "C" void fn_0008debc(char *text)
{
    char buf[132];

    if (0)
        sprintf(buf, text);
}
