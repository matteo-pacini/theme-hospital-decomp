#include "bflib.h"

struct CDSpeedTable {
    s32 speed;
    s32 min_rate;
};

extern CDSpeedTable cd_speed_table[];

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

extern "C" {

// FUNCTION: 0x000bbce0
s32 fn_000bbce0(s8 *data, s32 len)
{
    s8 *p;
    s8 *end;
    s32 n;
    s8 c;

    p = data;
    end = data + len;
    n = 0;
    c = *p++;
    while (p < end) {
        if (*p != c) {
            if (c == 0)
                n++;
            else
                n += 2;
            c = *p;
        }
        p++;
    }
    if (c == 0)
        n++;
    else
        n += 2;
    return n;
}

// FUNCTION: 0x000bbd70
s32 fn_000bbd70(s8 *data, s32 len)
{
    s8 *buf;
    s8 *out;
    s8 *p;
    s8 *end;
    s32 size;
    s8 run;
    s8 c;

    p = data;
    end = data + len;
    c = *p++;
    run = 1;
    size = fn_000bbce0(data, len);
    buf = (s8 *)LbMemoryAlloc(size);
    if (buf == 0)
        return len;
    out = buf;
    while (p < end) {
        if (*p != c || run == 0x7f) {
            if (c == 0)
                *out++ = -run;
            else {
                *out++ = run;
                *out++ = c;
            }
            run = 0;
            c = *p;
        }
        p++;
        run++;
    }
    if (c == 0)
        *out++ = -run;
    else {
        *out++ = run;
        *out++ = c;
    }
    size = out - buf;
    if (size < len)
        fn_000c21e6(data, buf, size);
    else
        size = len;
    LbMemoryFree(buf);
    return size;
}

// FUNCTION: 0x000bbea0
s32 fn_000bbea0(s8 *data, s32 len)
{
    s8 *p;
    s8 *end;
    s32 n;

    p = data;
    end = data + len;
    n = 0;
    while (p < end) {
        if (*p < 0)
            n -= *p;
        else {
            n += *p;
            p++;
        }
        p++;
    }
    return n;
}

// FUNCTION: 0x000bbf10
s32 fn_000bbf10(s8 *data, s32 len)
{
    s8 *buf;
    s8 *out;
    s32 size;
    s8 *p;
    s8 *end;

    p = data;
    end = data + len;
    size = fn_000bbea0(data, len);
    buf = (s8 *)LbMemoryAlloc(size);
    if (buf == 0)
        return size;
    out = buf;
    while (p < end) {
        if (*p < 0) {
            memset(out, 0, -*p);
            out += -*p++;
        } else {
            memset(out, p[1], *p);
            out += *p;
            p += 2;
        }
    }
    fn_000c21e6(data, buf, size);
    LbMemoryFree(buf);
    return size;
}

}
