#include "clib.h"

typedef int tiny_ret_t;

tiny_ret_t _TinyChDir(const char *path);
#pragma aux _TinyChDir = \
    "mov ah,3bh" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    parm caller [edx] value [eax];

#define TINY_ERROR(rc) ((rc) < 0)
#define TINY_INFO(rc) ((unsigned short)(rc))

int __set_errno_dos(unsigned err);

// FUNCTION: 0xec686
int chdir(const char *path)
{
    tiny_ret_t rc;

    rc = _TinyChDir(path);
    if (TINY_ERROR(rc))
        return __set_errno_dos(TINY_INFO(rc));
    return 0;
}
