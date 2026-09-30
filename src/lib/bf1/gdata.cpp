#include "bflib.h"

extern "C" {
extern char *(*D_0014d5e4)(TbLoadFiles *files);

void *LbMemoryAllocLow(u32 size);
void fn_000cd2cc(char *s, ...);
void fn_000d080c(char *fmt, ...);
int fn_000d0848(char *expr, char *file, int line);
void int3(void);
#pragma aux int3 = 0xcc;

// FUNCTION: 0xbccf0
void LbDataLoadSetModifyFilenameFunction(char *(*fn)(TbLoadFiles *files))
{
    D_0014d5e4 = fn;
}

}

// FUNCTION: 0xbcd08
// Not static, as the original is: only D_0014d5e4's initialiser refers to it, and
// that data is not part of this unit, so a static copy would not be emitted.
char *_defaultModifyDataLoadFilename(TbLoadFiles *files)
{
    return files->FName;
}

extern "C" {

// NONMATCHING: register choice (target loads operands into registers before compares)
// FUNCTION: 0x000bcd18
int LbDataLoad(TbLoadFiles *files)
{
    void *(*alloc)(u32);
    char *fname;

    if (files->Flags & 1)
        alloc = LbMemoryAllocLow;
    else
        alloc = LbMemoryAlloc;
    LbDataFree(files);
    fname = D_0014d5e4(files);
    if (*fname == '*') {
        *files->Start = (u8 *)alloc(files->SLength);
        if (*files->Start == 0)
            return -100;
    } else {
        files->SLength = LbFileLengthRnc(fname);
        if (files->SLength <= 0)
            return -101;
        *files->Start = (u8 *)alloc(files->SLength + 512);
        if (*files->Start == 0)
            return -100;
        if (LbFileLoadAt(fname, *files->Start) != files->SLength) {
            *files->Start = 0;
            *files->SEnd = 0;
            files->SLength = 0;
            return -101;
        }
    }
    if (files->SEnd)
        *files->SEnd = *files->Start + files->SLength;
    return 1;
}

// FUNCTION: 0x000bce44
int LbDataLoadAll(TbLoadFiles *files)
{
    u16 count;
    int ret;
    TbLoadFiles *t;

    count = 0;
    fn_000cd2cc("LbDataLoadAll");
    for (t = files; t->Start; t++)
        LbDataFree(t);
    for (t = files; t->Start; t++) {
        ret = LbDataLoad(t);
        if (ret == -100) {
            fn_000d080c("ERROR: Allocation %s.\n", t);
            {
                char msg[150];
                sprintf(msg, "LbDataLoadAll() : Memory alloaction error before loading file: %s", t);
                if (fn_000d0848(msg, "gdata.cpp", 102))
                    int3();
            }
            count++;
        } else if (ret == -101) {
            fn_000d080c("ERROR: File %s.\n", t);
            {
                char msg[150];
                sprintf(msg, "LbDataLoadAll() : Error loading file: %s", t);
                if (fn_000d0848(msg, "gdata.cpp", 110))
                    int3();
            }
            count++;
        }
    }
    return count;
}

// FUNCTION: 0x000bcf5c
int LbDataFree(TbLoadFiles *files)
{
    if (*files->Start) {
        LbMemoryFree(*files->Start);
        *files->Start = 0;
    }
    return 1;
}

// FUNCTION: 0x000bcf90
int LbDataFreeAll(TbLoadFiles *files)
{
    while (files->Start) {
        LbDataFree(files);
        files++;
    }
    return 1;
}

}
