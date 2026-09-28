#include "clib.h"

#define EBADF 4

typedef int tiny_ret_t;

tiny_ret_t _TinyClose(int handle);
#pragma aux _TinyClose = \
    "mov ah,3eh" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    parm caller [ebx] value [eax];

#define TINY_ERROR(rc) ((rc) < 0)

void __SetIOMode(int handle, unsigned value);

// FUNCTION: 0xed226
int __close(int handle)
{
    tiny_ret_t rc;

    rc = _TinyClose(handle);
    if (!TINY_ERROR(rc)) {
        __SetIOMode(handle, 0);
        return 0;
    }
    __set_errno(EBADF);
    return -1;
}
