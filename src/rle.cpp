#include "bflib.h"

extern "C" {

// Returns the size data[0..len) would have run-length encoded (see
// rle_compress).
// FUNCTION: 0x000bbce0
s32 rle_compressed_size(s8 *data, s32 len)
{
    s8 *src;
    s8 *end;
    s32 size;
    s8 value;

    src = data;
    end = data + len;
    size = 0;
    value = *src++;
    while (src < end) {
        if (*src != value) {
            if (value == 0)
                size++;
            else
                size += 2;
            value = *src;
        }
        src++;
    }
    if (value == 0)
        size++;
    else
        size += 2;
    return size;
}

// Run-length encodes data[0..len) in place: a run of n zero bytes becomes
// the byte -n, any other run of n bytes the pair n, value (runs of at most
// 127). Returns the new size, or len (data unchanged) if encoding would not
// shrink it or memory ran out.
// FUNCTION: 0x000bbd70
s32 rle_compress(s8 *data, s32 len)
{
    s8 *buf;
    s8 *out;
    s8 *src;
    s8 *end;
    s32 size;
    s8 run;
    s8 value;

    src = data;
    end = data + len;
    value = *src++;
    run = 1;
    size = rle_compressed_size(data, len);
    buf = (s8 *)LbMemoryAlloc(size);
    if (buf == 0)
        return len;
    out = buf;
    while (src < end) {
        if (*src != value || run == 0x7f) {
            if (value == 0)
                *out++ = -run;
            else {
                *out++ = run;
                *out++ = value;
            }
            run = 0;
            value = *src;
        }
        src++;
        run++;
    }
    if (value == 0)
        *out++ = -run;
    else {
        *out++ = run;
        *out++ = value;
    }
    size = out - buf;
    if (size < len)
        lib_memcpy(data, buf, size);
    else
        size = len;
    LbMemoryFree(buf);
    return size;
}

// Returns the decoded size of len bytes of run-length data (see
// rle_compress).
// FUNCTION: 0x000bbea0
s32 rle_decompressed_size(s8 *data, s32 len)
{
    s8 *src;
    s8 *end;
    s32 size;

    src = data;
    end = data + len;
    size = 0;
    while (src < end) {
        if (*src < 0)
            size -= *src;
        else {
            size += *src;
            src++;
        }
        src++;
    }
    return size;
}

// Decodes len bytes of run-length data in place; data must have room for
// the decoded size, which is returned. If memory runs out, data is left
// encoded but the decoded size is still returned.
// FUNCTION: 0x000bbf10
s32 rle_decompress(s8 *data, s32 len)
{
    s8 *buf;
    s8 *out;
    s32 size;
    s8 *src;
    s8 *end;

    src = data;
    end = data + len;
    size = rle_decompressed_size(data, len);
    buf = (s8 *)LbMemoryAlloc(size);
    if (buf == 0)
        return size;
    out = buf;
    while (src < end) {
        if (*src < 0) {
            memset(out, 0, -*src);
            out += -*src++;
        } else {
            memset(out, src[1], *src);
            out += *src;
            src += 2;
        }
    }
    lib_memcpy(data, buf, size);
    LbMemoryFree(buf);
    return size;
}
}
