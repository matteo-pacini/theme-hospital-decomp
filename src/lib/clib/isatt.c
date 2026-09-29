#include "clib.h"

typedef int tiny_ret_t;

tiny_ret_t _TinyGetDeviceInfo(int handle);
#pragma aux _TinyGetDeviceInfo = \
    "mov al,0" \
    "mov ah,44h" \
    "int 21h" \
    "rcl edx,1" \
    "ror edx,1" \
    parm caller [ebx] value [edx] modify [eax ecx];

#define TIO_CTL_DEVICE 0x80

// FUNCTION: 0xed25b
// NONMATCHING: config/functions.txt extends isatty_ over stack-check code
// (0xed282..0xed29d); the code itself matches
int isatty(int handle)
{
    tiny_ret_t rc;
    unsigned short info;

    rc = _TinyGetDeviceInfo(handle);
    info = (unsigned short)rc & 0x00ff;
    info &= TIO_CTL_DEVICE;
    return (int)info != 0;
}
