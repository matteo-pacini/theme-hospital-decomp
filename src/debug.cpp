#include "hosp.h"

typedef char *va_list[1];
#define va_start(ap, last) ((ap)[0] = (char *)&(last) + sizeof(last))
#define va_end(ap) ((ap)[0] = 0)

void note_in_error_log(char *text);
void write_to_player_log(char *text);

u32 lib_strlen(char *s);
#pragma aux lib_strlen "strlen_"

extern "C" {

char *getenv(char *name);
int vsprintf(char *dest, char *format, va_list arg);
typedef struct __iobuf FILE;
FILE *fopen(const char *name, const char *access_mode);
int fprintf(FILE *fp, const char *format, ...);
int fclose(FILE *fp);
}

// Returns anim unchanged, unless it is 9999 (no animation for peep): then logs
// it and returns 0.
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

// Enters navigation debug mode (MM_NAV_DEBUG).
// FUNCTION: 0x0008dd40
void nav_debug_mode(void)
{
    set_mouse_mode(MM_NAV_DEBUG);
    debug.nav_debug = 1;
}

// Appends text to the developer's log file (N:\hosp\log\<USERID>.txt), if it
// can be opened.
// FUNCTION: 0x0008dd68
void write_to_player_log(char *text)
{
    char path[64];
    s32 handle;

    sprintf(path, "N:\\hosp\\log\\%s.txt", getenv("USERID"));
    handle = LbFileOpen(path, 1);
    if (handle != -1) {
        LbFileSeek(handle, 0, 2);
        LbFileWrite(handle, text, lib_strlen(text));
        LbFileClose(handle);
    }
}

// Appends "NOTE: text" as a line to the developer's log.
// FUNCTION: 0x0008dde4
void note_in_error_log(char *text)
{
    char buf[128];

    sprintf(buf, "NOTE: %s\n", text);
    write_to_player_log(buf);
}

// Switches to screen mode SCREEN_MODE_13 (high resolution), first setting
// resvars.screen_mode to SCREEN_MODE_1 if it was SCREEN_MODE_13. Exits if the mode
// cannot be set.
// FUNCTION: 0x0008de24
extern "C" void switch_to_hires(void)
{
    if (resvars.screen_mode == SCREEN_MODE_13)
        resvars.screen_mode = SCREEN_MODE_1;
    if (!change_res(SCREEN_MODE_13))
        exit(1);
}

// Formats a debug message into debug.msg_text and shows it for 200 frames
// (debug.msg_timer).
// FUNCTION: 0x0008de64
void debug_msg(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsprintf(debug.msg_text, fmt, args);
    va_end(args);
    debug.msg_timer = 200;
}

// Does nothing (a disabled log writer).
// FUNCTION: 0x0008dea0
extern "C" void write_network_log(char *text)
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

// Does nothing (a disabled message formatter).
// FUNCTION: 0x0008debc
extern "C" void net_debug_msg(char *text)
{
    char buf[132];

    if (0)
        sprintf(buf, text);
}
