#include "bflib.h"

extern "C" {
extern char *(*D_0014d5e4)(TbLoadFiles *files);

void *LbMemoryAllocLow(u32 size);

// NONMATCHING: register choice (target loads operands into registers before compares)
// FUNCTION: 0x000bcd18
int LbDataLoadAll(TbLoadFiles *files)
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
