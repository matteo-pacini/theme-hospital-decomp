#include "bflib.h"

struct CDSpeedTable {
    s32 speed;
    s32 min_rate;
};

CDSpeedTable cd_speed_table[] = {
    { 1, 0x20800 },
    { 2, 0x46000 },
    { 3, 0x66800 },
    { 4, 0x8c000 },
    { 6, 0xd7000 },
    { 8, 0x122000 },
    { 12, 0x168000 },
    { 99, 0 },
};

// FUNCTION: 0x000bbbb0
s32 GetCDSpeed(char *path, void *buf, s32 size, u32 count)
{
    CDSpeedTable *entry;
    s32 best;
    int handle;
    s32 result;
    u32 start;
    u32 end;
    u32 elapsed;
    u32 blocks;
    u32 total;
    s32 rate;

    best = 0;
    result = -1;
    total = 0;
    handle = LbFileOpen(path, 2);
    if (handle != -1) {
        blocks = count / size;
        start = clock();
        result = 1;
        while (result > 0 && blocks != 0) {
            result = LbFileRead(handle, buf, size);
            if (result > 0)
                total += result;
            blocks--;
        }
        end = clock();
        elapsed = end - start;
        if (elapsed > 0) {
            elapsed /= 100;
            if (elapsed != 0)
                rate = total / elapsed;
            else
                rate = 99999999;
            for (entry = cd_speed_table; entry->speed != 99; entry++) {
                if (rate >= entry->min_rate)
                    best = entry->speed;
            }
            result = best;
        } else
            result = -1;
        LbFileClose(handle);
    }
    return result;
}
