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

}

// FUNCTION: 0x0008dcd0
u16 check_frame(u16 anim, Thing *w)
{
    char buf[100];

    if (anim == 9999) {
        sprintf(buf, "Invalid Frame Lookup. Peep %d", (u16)(w - thing));
        note_in_error_log(buf);
        return 0;
    }
    return anim;
}

// FUNCTION: 0x0008dd40
void nav_debug_mode(void)
{
    set_mouse_mode(0xd);
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
}

// FUNCTION: 0x0008debc
extern "C" void fn_0008debc(char *text)
{
    char buf[132];

    if (0)
        sprintf(buf, text);
}
