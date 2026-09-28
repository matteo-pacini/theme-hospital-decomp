#include "clib.h"

#define _INITIALIZED 0x4000
#define NUM_STD_STREAMS 5

extern unsigned __NFiles;
extern unsigned *_iomode;

int isatty(int handle);

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

// FUNCTION: 0xed211
// NONMATCHING: the index is scaled into a register instead of the address
void __SetIOMode(int handle, unsigned value)
{
    _iomode[handle] = value | _INITIALIZED;
}
