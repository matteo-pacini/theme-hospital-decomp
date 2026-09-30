#include "bflib.h"

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
