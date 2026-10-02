#include "clib.h"

#define _BINARY 0x40
#define EBADF 4
#define EINVAL 9

unsigned __IOMode(int handle);
void __SetIOMode(int handle, unsigned value);

// setmode: set handle (and its stream) to O_BINARY or O_TEXT; returns the previous mode, or -1 with errno set.
// FUNCTION: 0xd0bb5
int setmode(int handle, int mode)
{
    unsigned iomode_flags;
    unsigned old_mode;
    __stream_link *link;
    FILE *fp;

    iomode_flags = __IOMode(handle);
    if (iomode_flags == 0) {
        __set_errno(EBADF);
        return -1;
    }
    old_mode = (iomode_flags & _BINARY) ? O_BINARY : O_TEXT;
    if (mode != old_mode) {
        if (mode == O_BINARY || mode == O_TEXT) {
            iomode_flags &= ~_BINARY;
            if (mode == O_BINARY) {
                iomode_flags |= _BINARY;
            }
            __SetIOMode(handle, iomode_flags);
            for (link = __OpenStreams; link != NULL; link = link->next) {
                fp = link->stream;
                if (fp->_flag != 0) {
                    if (fp->_handle == handle) {
                        fp->_flag &= ~_BINARY;
                        if (mode == O_BINARY) {
                            fp->_flag |= _BINARY;
                        }
                        break;
                    }
                }
            }
        } else {
            __set_errno(EINVAL);
            return -1;
        }
    }
    return old_mode;
}
