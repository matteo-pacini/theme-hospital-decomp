#include "bflib.h"

struct stat {
    s32 st_dev;
    u32 st_ino;
    u16 st_mode;
    s16 st_nlink;
    s16 st_uid;
    s16 st_gid;
    s32 st_rdev;
    s32 st_size;
    s32 st_atime;
    s32 st_mtime;
    s32 st_ctime;
    u32 st_btime;
    u32 st_attr;
    u32 st_archivedID;
    u32 st_updatedID;
    u16 st_inheritedRightsMask;
    u8 st_originatingNameSpace;
    u8 st_name[13];
    u8 st_pad[4];
};

extern "C" {
char *getcwd(char *buf, u32 size);
int access(const char *path, int mode);
int stat(const char *path, struct stat *buf);
int rmdir(const char *path);
unsigned strlen(const char *s);
char *strcpy(char *dst, const char *src);
#pragma intrinsic(strlen, strcpy)

// FUNCTION: 0x000d0460
int fn_000d0460(char *buf, u32 size)
{
    s32 len;

    if (getcwd(buf, size)) {
        if (buf[1] == ':')
            strcpy(buf, buf + 2);
        len = strlen(buf);
        if (len > 0) {
            if (buf[len - 1] == '\\')
                buf[len - 1] = 0;
        }
        return 1;
    }
    return -1;
}

// NONMATCHING: target range also covers the next function (0xd0538)
// FUNCTION: 0x000d04f0
int fn_000d04f0(char *path)
{
    struct stat st;

    if (access(path, 0) == 0 && stat(path, &st) != -1)
        return (st.st_mode & 0xf000) == 0x4000;
    return 0;
}

int fn_000d0538(char *path)
{
    if (rmdir(path) == 0)
        return 1;
    return -1;
}

}
