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

// read: read up to len bytes from handle into buf (text mode drops CR and stops at Ctrl-Z); returns bytes stored, or -1 with errno set.
// FUNCTION: 0xd0f1b
// NONMATCHING: esi and edi are swapped (the original keeps total_len in esi),
// and buf is copied to ebp before the __IOMode call rather than after
int read(int handle, void *buf, unsigned len)
{
    unsigned read_len;
    unsigned total_len;
    unsigned amount_read;
    unsigned reduce_idx;
    unsigned finish_idx;
    tiny_ret_t rc;
    unsigned iomode_flags;
    char *buffer = buf;

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
        total_len = rc;
        if (TINY_ERROR(rc)) {
            return __set_errno_dos(TINY_INFO(rc));
        }
    } else {
        total_len = 0;
        read_len = len;
        do {
            rc = _TinyRead(handle, buffer, read_len);
            amount_read = rc;
            if (TINY_ERROR(rc)) {
                return __set_errno_dos(TINY_INFO(rc));
            }
            if (amount_read == 0)
                break;
            reduce_idx = 0;
            finish_idx = 0;
            for (; reduce_idx < amount_read; ++reduce_idx) {
                if ((int)buffer[reduce_idx] == 0x1a) {
                    lseek(handle, ((long)reduce_idx - (long)amount_read) + 1L, SEEK_CUR);
                    return total_len;
                }
                if ((int)buffer[reduce_idx] != '\r') {
                    buffer[finish_idx++] = buffer[reduce_idx];
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
