#include "clib.h"

#define ENOENT 1
#define ENOMEM 5
#define _MAX_PATH 144

typedef int tiny_ret_t;

tiny_ret_t _TinyGetCWDir(char *buf, unsigned char drive);
#pragma aux _TinyGetCWDir = \
    "mov ah,47h" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    parm caller [esi] [dl] value [eax];

unsigned char _TinyGetCurrDrive(void);
#pragma aux _TinyGetCurrDrive = \
    "mov ah,19h" \
    "int 21h" \
    value [al] modify [eax];

#define TINY_ERROR(rc) ((rc) < 0)

void *malloc(size_t size);
/* config/functions.txt has malloc under a wrong demo name. */
#pragma aux malloc "_nmalloc_"
char *strncpy(char *dst, const char *src, size_t n);

// FUNCTION: 0xec6c8
char *getcwd(char *buf, size_t size)
{
    tiny_ret_t rc;
    char path[_MAX_PATH];
    size_t len;

    rc = _TinyGetCWDir(&path[3], 0);
    if (TINY_ERROR(rc)) {
        __set_errno(ENOENT);
        return NULL;
    }
    path[0] = _TinyGetCurrDrive() + 'A';
    path[1] = ':';
    path[2] = '\\';
    len = strlen(path) + 1;
    if (buf == NULL) {
        if ((buf = malloc(len)) == NULL) {
            __set_errno(ENOMEM);
            return NULL;
        }
        size = len;
    }
    return strncpy(buf, path, size);
}
