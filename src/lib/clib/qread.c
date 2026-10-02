#include "clib.h"

typedef int tiny_ret_t;

tiny_ret_t _TinyRead(int handle, void *buffer, unsigned len);
#pragma aux _TinyRead = \
    "mov ah,3fh" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    parm caller [ebx] [edx] [ecx] value [eax];

#define TINY_ERROR(rc) ((rc) < 0)
#define TINY_INFO(rc) ((unsigned short)(rc))

int __set_errno_dos(unsigned err);

// __qread: read up to len bytes from handle into buffer (DOS 3Fh); returns the count, or -1 with errno set.
// FUNCTION: 0xfc91c
int __qread(int handle, void *buffer, unsigned len)
{
    tiny_ret_t rc;

    rc = _TinyRead(handle, buffer, len);
    if (TINY_ERROR(rc))
        return __set_errno_dos(TINY_INFO(rc));
    return rc;
}
