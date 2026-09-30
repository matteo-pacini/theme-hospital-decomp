#include "bflib.h"

static char *_defaultModifyDataLoadFilename(TbLoadFiles *files);

static char *(*modify_data_load_filename_function)(TbLoadFiles *files) = _defaultModifyDataLoadFilename;

void LbDebugString(char *fmt, ...);
int LbAssertMessage(const char *expr, const char *file, unsigned long line);

extern "C" {

void *LbMemoryAllocLow(u32 size);
void fn_000cd2cc(char *s, ...);
void int3(void);
#pragma aux int3 = 0xcc;

// FUNCTION: 0xbccf0
void LbDataLoadSetModifyFilenameFunction(char *(*fn)(TbLoadFiles *files))
{
    modify_data_load_filename_function = fn;
}

}

// FUNCTION: 0xbcd08
static char *_defaultModifyDataLoadFilename(TbLoadFiles *files)
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
    fname = modify_data_load_filename_function(files);
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
            LbDebugString("ERROR: Allocation %s.\n", t);
            {
                char msg[150];
                sprintf(msg, "LbDataLoadAll() : Memory alloaction error before loading file: %s", t);
                if (LbAssertMessage(msg, "gdata.cpp", 102))
                    int3();
            }
            count++;
        } else if (ret == -101) {
            LbDebugString("ERROR: File %s.\n", t);
            {
                char msg[150];
                sprintf(msg, "LbDataLoadAll() : Error loading file: %s", t);
                if (LbAssertMessage(msg, "gdata.cpp", 110))
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
