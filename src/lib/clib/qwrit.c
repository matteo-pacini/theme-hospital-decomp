#include "clib.h"

#define _APPEND 0x80

typedef int tiny_ret_t;

tiny_ret_t _TinyWrite(int handle, const void *buffer, unsigned len);
#pragma aux _TinyWrite = \
    "mov ah,40h" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    parm caller [ebx] [edx] [ecx] value [eax];

tiny_ret_t _TinySeek(int handle, unsigned long offset, int method);
#pragma aux _TinySeek = \
    "mov ah,42h" \
    "mov ecx,edx" \
    "shr ecx,16" \
    "int 21h" \
    "rcl dx,1" \
    "ror dx,1" \
    "shl edx,16" \
    "mov dx,ax" \
    parm caller [ebx] [edx] [al] value [edx] modify [ecx];

#define TINY_ERROR(rc) ((rc) < 0)
#define TINY_INFO(rc) ((unsigned short)(rc))

unsigned __IOMode(int handle);
int __set_errno_dos(unsigned err);

// __qwrite: write len bytes to handle (seeking to the end first in append mode); returns bytes written (errno ENOSPC if short), or -1.
// FUNCTION: 0xec4b8
int __qwrite(int handle, const void *buffer, unsigned len)
{
    tiny_ret_t rc;
    unsigned written;

    if (__IOMode(handle) & _APPEND) {
        rc = _TinySeek(handle, 0L, SEEK_END);
        if (TINY_ERROR(rc))
            return __set_errno_dos(TINY_INFO(rc));
    }
    rc = _TinyWrite(handle, buffer, len);
    written = rc;
    if (TINY_ERROR(rc))
        return __set_errno_dos(TINY_INFO(rc));
    if (written != len)
        __set_errno(ENOSPC);
    return written;
}
