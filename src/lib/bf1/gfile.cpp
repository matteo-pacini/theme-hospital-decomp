#include "bflib.h"

extern "C" {

int strncmp(const char *a, const char *b, u32 n);
int UnpackM1(void *src, void *dst);
int LbFileLength(char *path);
int LbFilePosition(int handle);
void fn_000cd2cc(char *s, ...);
int fn_000d0460(char *buf, u32 size);
unsigned strlen(const char *s);
char *strcpy(char *dst, const char *src);
char *strcat(char *dst, const char *src);
void *memcpy(void *dst, const void *src, u32 n);
#pragma intrinsic(strlen, strcpy, strcat, memcpy)
int fn_000bc75c(s8 *a, s8 *b, u32 len, int ignore_case);

// FUNCTION: 0x000bc170
int LbFileLengthRnc(char *path)
{
    int handle;
    int length;
    u8 header[8];
    char rnc[5];

    length = -1;
    rnc[0] = 'R';
    rnc[1] = 'N';
    rnc[2] = 'C';
    rnc[3] = 1;
    rnc[4] = 0;
    handle = LbFileOpen(path, 2);
    if (handle == -1)
        return -1;
    if (LbFileRead(handle, header, 8) == -1) {
        LbFileClose(handle);
        return -1;
    }
    LbFileClose(handle);
    if (strncmp((char *)header, rnc, 4) == 0) {
        length = header[4];
        length <<= 8;
        length += header[5];
        length <<= 8;
        length += header[6];
        length <<= 8;
        length += header[7];
    } else {
        length = LbFileLength(path);
    }
    return length;
}

// FUNCTION: 0x000bc23c
int LbFileLoadAt(char *path, void *buf)
{
    int handle;
    int result;
    int length;

    length = LbFileLengthRnc(path);
    if (length != -1) {
        handle = LbFileOpen(path, 2);
        if (handle != -1) {
            if (LbFileRead(handle, buf, length) != -1) {
                LbFileClose(handle);
                result = UnpackM1(buf, buf);
                if (result < 0) {
                    fn_000cd2cc("ERROR decompressing %s\n", path);
                    return -1;
                }
                if (result == 0)
                    return length;
                return result;
            }
        }
    }
    return -1;
}

// FUNCTION: 0x000bc2dc
int LbFileSaveAt(char *path, void *buf, u32 size)
{
    int result;
    int handle;

    handle = LbFileOpen(path, 0);
    if (handle != -1) {
        result = LbFileWrite(handle, buf, size);
        LbFileClose(handle);
        return result;
    }
    return -1;
}

// FUNCTION: 0x000bc328
int fn_000bc328(int handle, char *string, u8 *buf, u32 buflen, u32 flags)
{
    int start;
    int i;
    int offset;
    int found;
    int len;
    int count;
    int got;
    s8 search[130];

    if (buflen < 256)
        return -3;
    len = strlen(string);
    if (len > 128)
        return -4;
    start = LbFilePosition(handle);
    if (start == -1)
        return -1;
    found = -2;
    if (flags & 2) {
        search[0] = '\n';
        strcpy((char *)search + 1, string);
        len++;
    } else {
        strcpy((char *)search, string);
    }
    offset = 0;
    flags &= 1;
    count = LbFileRead(handle, buf, buflen);
    while (found == -2 && count >= len) {
        for (i = 0; i <= count - len; i++) {
            if (fn_000bc75c((s8 *)buf + i, search, len, flags)) {
                found = start + offset + i;
                if (search[0] == '\n')
                    found++;
                break;
            }
        }
        if (found == -2) {
            memcpy(buf, buf + i, count - i);
            offset += i;
            count -= i;
            got = LbFileRead(handle, buf + count, buflen - count);
            if (got == -1)
                return -1;
            count += got;
        }
    }
    if (LbFileSeek(handle, start, 0) != -1)
        return found;
    return -1;
}

// FUNCTION: 0x000bc504
int fn_000bc504(int current_dir, char *directory, char *filename, char *buf, u32 buflen)
{
    u32 len;

    len = 0;
    if (current_dir) {
        if (fn_000d0460(buf, buflen - 2) == -1)
            return -1;
        len = strlen(buf);
        buf[len] = '\\';
        ++len;
        buf[len] = 0;
    }
    if (directory) {
        len += strlen(directory);
        if (buflen - 2 <= len)
            return -1;
        strcat(buf, directory);
        if (directory[len - 1] != '\\') {
            buf[len] = '\\';
            ++len;
            buf[len] = 0;
        }
    }
    len += strlen(filename);
    if (len >= buflen)
        return -1;
    strcat(buf, filename);
    return 1;
}

// FUNCTION: 0x000bc63c
int fn_000bc63c(char *src, char *dst)
{
    u8 buf[1024];
    u32 chunk;
    u32 left;
    int in;
    int out;

    left = LbFileLength(src);
    if (left == -1)
        return -1;
    in = LbFileOpen(src, 2);
    if (in == -1)
        return -1;
    out = LbFileOpen(dst, 0);
    if (out == -1) {
        LbFileClose(in);
        return -1;
    }
    while (left) {
        chunk = left > 1024 ? 1024 : left;
        if (LbFileRead(in, buf, chunk) != chunk) {
            LbFileClose(in);
            LbFileClose(out);
            return -1;
        }
        if (LbFileWrite(out, buf, chunk) != chunk) {
            LbFileClose(in);
            LbFileClose(out);
            return -1;
        }
        left -= chunk;
    }
    LbFileClose(in);
    LbFileClose(out);
    return 1;
}

// FUNCTION: 0x000bc75c
int fn_000bc75c(s8 *a, s8 *b, u32 len, int ignore_case)
{
    u32 i;

    if (!ignore_case)
        return strncmp((char *)a, (char *)b, len) == 0;
    for (i = 0; i < len; i++) {
        if (toupper(*a++) != toupper(*b++))
            return 0;
    }
    return 1;
}

}
