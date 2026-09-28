#include "bflib.h"

struct TbLog {
    char filename[0x91];
    char prefix[0x21];
    u32 flags;
    u32 initialised;
    u32 suspended;
    u32 created;
};

extern "C" {
int LbLog(TbLog *log, char *fmt, ...);
int LbLogClose(TbLog *log);
}

// FUNCTION: 0xeb3ac
// NONMATCHING: target saves ebx-edi although the body uses none of them
int LbLog(TbLog *log, char *fmt, ...)
{
    char buf[0x20];

    return -1;
}

// FUNCTION: 0xeb3c8
int LbLogClose(TbLog *log)
{
    if (!log->initialised)
        return -1;
    memset(log->filename, 0, sizeof(log->filename));
    memset(log->prefix, 0, sizeof(log->prefix));
    log->flags = 0;
    log->initialised = 0;
    log->created = 0;
    log->suspended = 0;
    return 1;
}
