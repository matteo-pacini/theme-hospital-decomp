#include "bflib.h"

struct TbLog {
    char filename[0x91];
    char prefix[0x21];
    u32 flags;
    u32 initialised;
    u32 created;
    u32 suspended;
};

extern "C" {
int LbLog(TbLog *log, char *fmt, ...);
int LbLogClose(TbLog *log);
int LbFileDelete(char *path);
unsigned strlen(const char *s);
char *strcpy(char *dst, const char *src);
#pragma intrinsic(strlen, strcpy)
}

// Writing to a log is stubbed out in this build: always returns -1.
// FUNCTION: 0xeb3ac
int LbLog(TbLog *log, char *fmt, ...)
{
    char buf[0x20];

    // A stub: the unreachable code below only reproduces the registers the
    // target's prologue saves.
    return -1;
    strcpy(buf, fmt);
    LbFileWrite(1, buf, strlen(buf));
    return 1;
}

// Closes log, clearing its settings. Returns 1, or -1 if it was not set up.
// FUNCTION: 0xeb3c8
int LbLogClose(TbLog *log)
{
    if (!log->initialised)
        return -1;
    memset(log->filename, 0, sizeof(log->filename));
    memset(log->prefix, 0, sizeof(log->prefix));
    log->flags = 0;
    log->initialised = 0;
    log->suspended = 0;
    log->created = 0;
    return 1;
}

extern "C" {

// Sets up log for file filename (at most 0x90 characters); flags must
// select exactly one of bits 0 and 1 (the open mode). Returns 1, or -1.
// FUNCTION: 0xeb2c0
int LbLogSetup(TbLog *log, char *filename, u32 flags)
{
    log->initialised = 0;
    memset(log->filename, 0, sizeof(log->filename));
    memset(log->prefix, 0, sizeof(log->prefix));
    log->flags = 0;
    log->created = 0;
    log->suspended = 0;
    if ((flags & 3) == 0 || (flags & 3) == 3)
        return -1;
    if (strlen(filename) > 0x90)
        return -1;
    strcpy(log->filename, filename);
    log->flags = flags;
    log->initialised = 1;
    return 1;
}

// Sets log's line prefix (at most 32 characters; 0 for none). Returns 1,
// or -1.
// FUNCTION: 0xeb448
int LbLogSetPrefix(TbLog *log, char *prefix)
{
    if (!log->initialised)
        return -1;
    if (prefix == 0)
        memset(log->prefix, 0, sizeof(log->prefix));
    else {
        if (strlen(prefix) > 0x20)
            return -1;
        strcpy(log->prefix, prefix);
    }
    return 1;
}

// Deletes log's file. Returns 1, or -1.
// FUNCTION: 0xeb4dc
int LbLogDelete(TbLog *log)
{
    if (!log->initialised)
        return -1;
    if (LbFileDelete(log->filename) != 1)
        return -1;
    log->created = 0;
    return 1;
}

// Suspends logging to log. Returns 1, or -1 if it is not set up.
// FUNCTION: 0xeb528
int LbLogSuspend(TbLog *log)
{
    if (!log->initialised)
        return -1;
    log->suspended = 1;
    return 1;
}

// Resumes logging to log. Returns 1, or -1 if it is not set up.
// FUNCTION: 0xeb55c
int LbLogRestart(TbLog *log)
{
    if (!log->initialised)
        return -1;
    log->suspended = 0;
    return 1;
}

}
