#include "bflib.h"

extern "C" {
extern char D_00121de4[];

int strncmp(const char *a, const char *b, u32 n);
int UnpackM1(void *src, void *dst);
int LbFileLength(char *path);
void fn_000cd2cc(char *s, ...);

// NONMATCHING: stack slots of path/handle swapped
// FUNCTION: 0x000bc170
int LbFileLengthRnc(char *path)
{
    char rnc[5];
    u8 header[8];
    int length;
    int handle;

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
                    fn_000cd2cc(D_00121de4, path);
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

}
