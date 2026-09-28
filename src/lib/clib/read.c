#include "clib.h"

#define _BINARY 0x40
#define EBADF 4
#define EACCES 6

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

unsigned __IOMode(int handle);
int __set_errno_dos(unsigned err);

// FUNCTION: 0xd0f1b
// NONMATCHING: register and stack slot assignment of rc/total_len differ
int read(int handle, void *buf, unsigned len)
{
    unsigned read_len;
    unsigned total_len;
    unsigned reduce_idx;
    unsigned finish_idx;
    tiny_ret_t rc;
    unsigned iomode_flags;
    char *buffer = buf;
    char *p;
    char *end;

    iomode_flags = __IOMode(handle);
    if (iomode_flags == 0) {
        __set_errno(EBADF);
        return -1;
    }
    if (!(iomode_flags & _READ)) {
        __set_errno(EACCES);
        return -1;
    }
    if (iomode_flags & _BINARY) {
        rc = _TinyRead(handle, buffer, len);
        if (TINY_ERROR(rc)) {
            return __set_errno_dos(TINY_INFO(rc));
        }
        total_len = rc;
    } else {
        total_len = 0;
        read_len = len;
        do {
            rc = _TinyRead(handle, buffer, read_len);
            if (TINY_ERROR(rc)) {
                return __set_errno_dos(TINY_INFO(rc));
            }
            if (rc == 0)
                break;
            reduce_idx = 0;
            finish_idx = 0;
            end = buffer + rc;
            for (p = buffer; p < end; ++p, ++reduce_idx) {
                if ((int)*p == 0x1a) {
                    lseek(handle, ((long)reduce_idx - (long)rc) + 1L, SEEK_CUR);
                    return total_len;
                }
                if ((int)*p != '\r') {
                    buffer[finish_idx++] = *p;
                    ++total_len;
                }
            }
            read_len -= finish_idx;
            buffer += finish_idx;
            if (iomode_flags & _ISTTY)
                break;
        } while (read_len != 0);
    }
    return total_len;
}
