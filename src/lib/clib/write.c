#include "clib.h"

#define _BINARY 0x40
#define _APPEND 0x80
#define EBADF 4
#define EACCES 6

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
unsigned stackavail(void);
#pragma aux stackavail __modify __nomemory;
void __STKOVERFLOW(void);
void *__doalloca(size_t __size);
#pragma aux __doalloca = \
    "sub esp,eax" \
    __parm __nomemory [eax] __value [esp] __modify __exact __nomemory [esp];

// write: write len bytes from buffer to handle (text mode writes "\r\n" for '\n'); returns len or the bytes written (errno ENOSPC), or -1 with errno set.
// FUNCTION: 0xd1016
// NONMATCHING: same code, but the spilled locals get different stack slots
int write(int handle, const void *buffer, unsigned len)
{
    unsigned num_written;
    const char *p;
    unsigned buf_size;
    unsigned bytes_written;
    unsigned bytes_in_buf;
    unsigned iomode_flags;
    char *buf;
    tiny_ret_t rc;
    unsigned i;

    iomode_flags = __IOMode(handle);
    if (iomode_flags == 0) {
        __set_errno(EBADF);
        return -1;
    }
    if (!(iomode_flags & _WRITE)) {
        __set_errno(EACCES);
        return -1;
    }
    if (iomode_flags & _APPEND) {
        rc = _TinySeek(handle, 0L, SEEK_END);
        if (TINY_ERROR(rc)) {
            return __set_errno_dos(TINY_INFO(rc));
        }
    }
    if (iomode_flags & _BINARY) {
        rc = _TinyWrite(handle, buffer, len);
        if (TINY_ERROR(rc)) {
            return __set_errno_dos(TINY_INFO(rc));
        }
        if ((unsigned)rc != len) {
            __set_errno(ENOSPC);
            len = rc;
        }
        return len;
    }
    i = stackavail();
    if (i < 0xb0) {
        __STKOVERFLOW();
    }
    buf_size = 0x200;
    if (i < 0x230) {
        buf_size = 0x80;
    }
    buf = __doalloca((buf_size + 3) & ~3);
    bytes_written = 0;
    bytes_in_buf = 0;
    num_written = 0;
    p = buffer;
    while (bytes_written < len) {
        if ((int)*p == '\n') {
            buf[bytes_in_buf] = '\r';
            ++bytes_in_buf;
            if (bytes_in_buf == buf_size) {
                rc = _TinyWrite(handle, buf, buf_size);
                if (TINY_ERROR(rc)) {
                    return __set_errno_dos(TINY_INFO(rc));
                }
                bytes_in_buf = rc;
                if (rc != buf_size) {
                    __set_errno(ENOSPC);
                    return num_written + bytes_in_buf;
                }
                bytes_in_buf = 0;
                num_written = bytes_written;
            }
        }
        buf[bytes_in_buf] = *p++;
        ++bytes_in_buf;
        ++bytes_written;
        if (bytes_in_buf == buf_size) {
            rc = _TinyWrite(handle, buf, buf_size);
            if (TINY_ERROR(rc)) {
                return __set_errno_dos(TINY_INFO(rc));
            }
            bytes_in_buf = rc;
            if (rc != buf_size) {
                __set_errno(ENOSPC);
                return num_written + bytes_in_buf;
            }
            bytes_in_buf = 0;
            num_written = bytes_written;
        }
    }
    if (bytes_in_buf != 0) {
        rc = _TinyWrite(handle, buf, bytes_in_buf);
        if (TINY_ERROR(rc)) {
            return __set_errno_dos(TINY_INFO(rc));
        }
        if (rc != bytes_in_buf) {
            __set_errno(ENOSPC);
            return num_written + rc;
        }
    }
    return len;
}
