#include "clib.h"

long tell(int handle);

// ftell: current position of fp, accounting for buffered data; -1 on error.
// FUNCTION: 0xfbacb
long ftell(FILE *fp)
{
    long pos;

    pos = tell(fp->_handle);
    if (pos == -1)
        return -1;
    if (fp->_cnt != 0) {
        if (fp->_flag & _DIRTY) {
            pos += fp->_cnt;
        } else {
            pos -= fp->_cnt;
        }
    }
    return pos;
}
