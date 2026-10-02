#include "clib.h"

#define _INITIALIZED 0x4000
#define NUM_STD_STREAMS 5

extern unsigned __NFiles;
extern unsigned *_iomode;

int isatty(int handle);

// __IOMode: I/O mode flags of handle (0 if out of range); first use of a standard handle sets _ISTTY from isatty.
// FUNCTION: 0xed1bc
unsigned __IOMode(int handle)
{
    if ((unsigned)handle >= __NFiles)
        return 0;
    if (handle <= NUM_STD_STREAMS) {
        if (!(_iomode[handle] & _INITIALIZED)) {
            _iomode[handle] |= _INITIALIZED;
            if (isatty(handle))
                _iomode[handle] |= _ISTTY;
        }
    }
    return _iomode[handle];
}

// __SetIOMode: set handle's I/O mode flags to value (marked initialised).
// FUNCTION: 0xed211
// NONMATCHING: the target scales the index in a register (shl edx,2) where
// this C compiles to a scaled address ([edx+eax*4])
void __SetIOMode(int handle, unsigned value)
{
    _iomode[handle] = value | _INITIALIZED;
}
