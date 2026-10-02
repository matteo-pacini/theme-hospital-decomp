#include "clib.h"

#define _UNGET  0x0004
#define EINVAL  9

long tell(int handle);

// __update_buffer: move fp's read position by diff within its buffer if possible (returns 0); otherwise empty the buffer and return 1.
// FUNCTION: 0xfc7c1
static int __update_buffer(long diff, FILE *fp)
{
    fp->_flag &= ~_EOF;
    if (diff <= fp->_cnt && diff >= fp->_base - fp->_ptr) {
        fp->_ptr += diff;
        fp->_cnt -= diff;
        return 0;
    }
    fp->_ptr = fp->_base;
    fp->_cnt = 0;
    return 1;
}

// fseek: set fp's position to offset from origin (SEEK_SET/CUR/END); returns 0, or -1 on error.
// FUNCTION: 0xfc803
int fseek(FILE *fp, long offset, int origin)
{
    if (fp->_flag & (_WRITE | _UNGET)) {
        if (fp->_flag & _DIRTY) {
            if (__flush(fp)) {
                if (origin == SEEK_SET && offset < 0)
                    __set_errno(EINVAL);
                return -1;
            }
        } else {
            if (origin == SEEK_CUR)
                offset -= fp->_cnt;
            fp->_ptr = fp->_base;
            fp->_cnt = 0;
        }
        fp->_flag &= ~(_EOF | _UNGET);
        if (lseek(fp->_handle, offset, origin) == -1)
            return -1;
    } else {
        switch (origin) {
        case SEEK_CUR:
        {
            long ptr_delta = fp->_cnt;

            if (__update_buffer(offset, fp)) {
                offset -= ptr_delta;
                if (lseek(fp->_handle, offset, origin) == -1)
                    return -1;
            }
        }   break;
        case SEEK_SET:
        {
            long file_ptr = tell(fp->_handle);

            file_ptr -= fp->_cnt;
            if (__update_buffer(offset - file_ptr, fp)) {
                if (lseek(fp->_handle, offset, origin) == -1)
                    return -1;
            }
        }   break;
        case SEEK_END:
            fp->_flag &= ~_EOF;
            fp->_ptr = fp->_base;
            fp->_cnt = 0;
            if (lseek(fp->_handle, offset, origin) == -1)
                return -1;
            break;
        default:
            __set_errno(EINVAL);
            return -1;
        }
    }
    return 0;
}
