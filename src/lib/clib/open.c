#include "clib.h"

#define _BINARY 0x40
#define _APPEND 0x80
#define EEXIST 7
#define S_IWRITE 0x0080
#define S_IREAD 0x0100
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01

typedef int tiny_ret_t;

tiny_ret_t _TinyOpen(const char *name, unsigned char mode);
#pragma aux _TinyOpen = \
    "mov ah,3dh" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    parm caller [edx] [al] value [eax];

tiny_ret_t _TinyCreate(const char *name, unsigned char attr);
#pragma aux _TinyCreate = \
    "mov ah,3ch" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    parm caller [edx] [ecx] value [eax];

tiny_ret_t _TinyClose(int handle);
#pragma aux _TinyClose = \
    "mov ah,3eh" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    parm caller [ebx] value [eax];

tiny_ret_t _TinyWrite(int handle, const void *buffer, unsigned len);
#pragma aux _TinyWrite = \
    "mov ah,40h" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    parm caller [ebx] [edx] [ecx] value [eax];

tiny_ret_t _TinyGetDeviceInfo(int handle);
#pragma aux _TinyGetDeviceInfo = \
    "mov al,0" \
    "mov ah,44h" \
    "int 21h" \
    "rcl edx,1" \
    "ror edx,1" \
    parm caller [ebx] value [edx] modify [eax ecx];

tiny_ret_t _TinySetDeviceInfo(int handle, unsigned char info);
#pragma aux _TinySetDeviceInfo = \
    "mov dh,0" \
    "mov al,1" \
    "mov ah,44h" \
    "int 21h" \
    "rcl edx,1" \
    "ror edx,1" \
    parm caller [ebx] [dl] value [edx] modify [eax ecx];

#define TIO_RAW_MODE 0x20

#define TINY_ERROR(rc) ((rc) < 0)
#define TINY_INFO(rc) ((unsigned short)(rc))

extern int __umaskval;
extern int _fmode;

unsigned __IOMode(int handle);
void __SetIOMode(int handle, unsigned value);
int __set_errno_dos(unsigned err);
int isatty(int handle);

// FUNCTION: 0xd0c66
int open(const char *name, int mode, ...)
{
    int permission;
    va_list args;

    args[0] = (char *)&mode + sizeof(mode);
    permission = ((int *)(args[0] += sizeof(int)))[-1];
    args[0] = 0;
    return sopen(name, mode, SH_COMPAT, permission);
}

// FUNCTION: 0xd0c8a
int sopen(const char *name, int mode, int shflag, ...)
{
    int rwmode;
    int handle;
    unsigned attr;
    int permission;
    unsigned iomode_flags;
    tiny_ret_t rc;
    char dummy;
    va_list args;

    while ((int)*name == ' ')
        ++name;
    rwmode = mode & (O_RDONLY | O_WRONLY | O_RDWR | O_NOINHERIT);
    rc = _TinyOpen(name, (unsigned char)rwmode | (unsigned char)shflag);
    handle = -1;
    if (!TINY_ERROR(rc))
        handle = TINY_INFO(rc);
    if ((mode & (O_WRONLY | O_RDWR)) && handle != -1 && !isatty(handle)) {
        if ((mode & O_EXCL) && (mode & O_CREAT)) {
            _TinyClose(handle);
            __set_errno(EEXIST);
            return -1;
        }
        if (mode & O_TRUNC) {
            rc = _TinyWrite(handle, &dummy, 0);
            if (TINY_ERROR(rc)) {
                _TinyClose(handle);
                return __set_errno_dos(TINY_INFO(rc));
            }
        }
    }
    if (handle == -1) {
        if (!(mode & O_CREAT) || (int)TINY_INFO(rc) != 2)
            return __set_errno_dos(TINY_INFO(rc));
        args[0] = (char *)&shflag + sizeof(shflag);
        permission = ((int *)(args[0] += sizeof(int)))[-1];
        if (permission == 0)
            permission = S_IREAD | S_IWRITE;
        permission &= ~__umaskval;
        attr = _A_NORMAL;
        if (!(permission & S_IWRITE))
            attr = _A_RDONLY;
        rc = _TinyCreate(name, attr);
        if (TINY_ERROR(rc))
            return __set_errno_dos(TINY_INFO(rc));
        handle = TINY_INFO(rc);
        if (shflag != 0) {
            rc = _TinyClose(handle);
            if (TINY_ERROR(rc))
                return __set_errno_dos(TINY_INFO(rc));
            rc = _TinyOpen(name, (unsigned char)rwmode | (unsigned char)shflag);
            if (TINY_ERROR(rc))
                return __set_errno_dos(TINY_INFO(rc));
            handle = TINY_INFO(rc);
        }
    }
    iomode_flags = __IOMode(handle) & ~(_READ | _WRITE | _APPEND | _BINARY);
    if (isatty(handle))
        iomode_flags |= _ISTTY;
    rwmode &= ~O_NOINHERIT;
    if (rwmode == O_RDWR)
        iomode_flags |= _READ | _WRITE;
    if (rwmode == O_RDONLY)
        iomode_flags |= _READ;
    if (rwmode == O_WRONLY)
        iomode_flags |= _WRITE;
    if (mode & O_APPEND)
        iomode_flags |= _APPEND;
    if (mode & (O_BINARY | O_TEXT)) {
        if (mode & O_BINARY)
            iomode_flags |= _BINARY;
    } else if (_fmode == O_BINARY) {
        iomode_flags |= _BINARY;
    }
    __SetIOMode(handle, iomode_flags);
    return handle;
}

// FUNCTION: 0xd0e73
int fn_000d0e73(int handle)
{
    unsigned iomode_flags;
    tiny_ret_t rc;

    iomode_flags = __IOMode(handle) | _BINARY;
    __SetIOMode(handle, iomode_flags);
    if (iomode_flags & _ISTTY) {
        rc = _TinyGetDeviceInfo(handle);
        if (TINY_ERROR(rc))
            return __set_errno_dos(TINY_INFO(rc));
        rc = _TinySetDeviceInfo(handle, (unsigned char)rc | TIO_RAW_MODE);
        if (TINY_ERROR(rc))
            return __set_errno_dos(TINY_INFO(rc));
    }
    return 0;
}
