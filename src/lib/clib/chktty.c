#include "clib.h"

#define _IOFBF  0x0100
#define _IOLBF  0x0200
#define _IONBF  0x0400

int isatty(int handle);

// __chktty: on first use, mark fp as a TTY if its handle is one, and make it line-buffered unless a buffering mode is already set.
// FUNCTION: 0xfb7df
void __chktty(FILE *fp)
{
    if ((fp->_flag & _ISTTY) == 0) {
        if (isatty(fp->_handle)) {
            fp->_flag |= _ISTTY;
            if ((fp->_flag & (_IONBF | _IOLBF | _IOFBF)) == 0)
                fp->_flag |= _IOLBF;
        }
    }
}
