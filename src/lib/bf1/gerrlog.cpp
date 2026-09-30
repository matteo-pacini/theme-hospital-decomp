#include "bflib.h"

typedef char *va_list[1];
#define va_start(ap, last) ((ap)[0] = (char *)&(last) + sizeof(last))
#define va_end(ap) ((ap)[0] = 0)

extern "C" {
extern u32 D_0018a580;
extern u8 D_0018a584[];

int fn_000bc504(int current_dir, char *directory, char *filename, char *buf, u32 buflen);
int LbLogSetup(void *log, char *filename, u32 flags);
int LbLogSetPrefix(void *log, char *title);
int LbLog(void *log, char *fmt, ...);
int LbLogClose(void *log);
int vsprintf(char *buf, char *fmt, va_list args);
char *strcat(char *dst, const char *src);
#pragma intrinsic(strcat)

// FUNCTION: 0x000cd1e0
int fn_000cd1e0(char *directory, char *filename, u8 append)
{
    char path[144];
    u32 flags;

    if (D_0018a580)
        return -1;
    if (fn_000bc504(1, directory, filename ? filename : "ERROR.LOG", path, 144) == -1)
        return -1;
    if (append)
        flags = 0x35;
    else
        flags = 0x36;
    if (LbLogSetup(D_0018a584, path, flags) != 1)
        return -1;
    LbLogSetPrefix(D_0018a584, "Error: ");
    D_0018a580 = 1;
    return 1;
}

// FUNCTION: 0x000cd294
int fn_000cd294(void)
{
    if (!D_0018a580)
        return -1;
    if (LbLogClose(D_0018a584) != 1)
        return -1;
    D_0018a580 = 0;
    return 1;
}

// FUNCTION: 0x000cd2cc
int fn_000cd2cc(char *fmt, ...)
{
    char buf[512];
    va_list args;

    if (!D_0018a580)
        return -1;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    strcat(buf, "\n");
    return LbLog(D_0018a584, buf);
}

}
