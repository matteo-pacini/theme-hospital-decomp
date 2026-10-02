#include "clib.h"

// __flush: write out fp's dirty buffer, or seek back over unread buffered input; resets the buffer; returns 0 or EOF.
// FUNCTION: 0xcf438
int __flush(FILE *fp)
{
    int len;
    long offset;
    int ret;
    unsigned char *ptr;
    unsigned amount;

    ret = 0;
    if (fp->_flag & _DIRTY) {
        fp->_flag &= ~_DIRTY;
        if ((fp->_flag & _WRITE) && fp->_base != NULL) {
            ptr = fp->_base;
            amount = fp->_cnt;
            while (amount != 0 && ret == 0) {
                len = __qwrite(fp->_handle, ptr, amount);
                if (len == -1) {
                    fp->_flag |= _SFERR;
                    ret = EOF;
                } else if (len == 0) {
                    __set_errno(ENOSPC);
                    fp->_flag |= _SFERR;
                    ret = EOF;
                }
                ptr += len;
                amount -= len;
            }
        }
    } else if (fp->_base != NULL) {
        fp->_flag &= ~_EOF;
        if (!(fp->_flag & _ISTTY)) {
            offset = fp->_cnt;
            if (offset != 0) {
                offset = lseek(fp->_handle, -offset, SEEK_CUR);
            }
            if (offset == -1) {
                fp->_flag |= _SFERR;
                ret = EOF;
            }
        }
    }
    fp->_ptr = fp->_base;
    fp->_cnt = 0;
    return ret;
}
