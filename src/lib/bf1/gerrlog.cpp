#include "bflib.h"

typedef char *va_list[1];
#define va_start(ap, last) ((ap)[0] = (char *)&(last) + sizeof(last))
#define va_end(ap) ((ap)[0] = 0)

extern "C" {
extern u32 error_log_initialised;
extern u8 error_log[];

int LbFileMakeFullPath(int current_dir, char *directory, char *filename, char *buf, u32 buflen);
int LbLogSetup(void *log, char *filename, u32 flags);
int LbLogSetPrefix(void *log, char *prefix);
int LbLog(void *log, char *fmt, ...);
int LbLogClose(void *log);
int vsprintf(char *dest, char *format, va_list arg);
char *strcat(char *dst, const char *src);
#pragma intrinsic(strcat)

// Opens the error log file (filename, default ERROR.LOG, in directory),
// appending to it if append is set. Returns 1, or -1 if already open or on
// failure.
// FUNCTION: 0x000cd1e0
int LbErrorLogSetup(char *directory, char *filename, u8 append)
{
    char path[144];
    u32 flags;

    if (error_log_initialised)
        return -1;
    if (LbFileMakeFullPath(1, directory, filename ? filename : "ERROR.LOG", path, 144) == -1)
        return -1;
    if (append)
        flags = 0x35;
    else
        flags = 0x36;
    if (LbLogSetup(error_log, path, flags) != 1)
        return -1;
    LbLogSetPrefix(error_log, "Error: ");
    error_log_initialised = 1;
    return 1;
}

// Closes the error log. Returns 1, or -1 if it was not open or failed to
// close.
// FUNCTION: 0x000cd294
int LbErrorLogReset(void)
{
    if (!error_log_initialised)
        return -1;
    if (LbLogClose(error_log) != 1)
        return -1;
    error_log_initialised = 0;
    return 1;
}

// Formats a printf-style line for the error log and passes it to LbLog,
// which is a stub in this build (nothing is written). Returns LbLog's result
// (-1), or -1 if the log is not open.
// FUNCTION: 0x000cd2cc
int LbErrorLog(char *fmt, ...)
{
    char buf[512];
    va_list args;

    if (!error_log_initialised)
        return -1;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    strcat(buf, "\n");
    return LbLog(error_log, buf);
}

}
