#include "clib.h"

#define EBADF 4

#define _BINARY 0x0040
#define _IOLBF  0x0200
#define _IONBF  0x0400

void __ioalloc(FILE *fp);

// fputc: write character c to fp (text mode writes "\r\n" for '\n'); returns c, or EOF on error.
// FUNCTION: 0xf316a
int fputc(int c, FILE *fp)
{
    int flags;

    if (!(fp->_flag & _WRITE)) {
        __set_errno(EBADF);
        fp->_flag |= _SFERR;
        return EOF;
    }
    if (fp->_base == NULL)
        __ioalloc(fp);
    flags = _IONBF;
    if (c == '\n') {
        flags = _IONBF | _IOLBF;
        if (!(fp->_flag & _BINARY)) {
            fp->_flag |= _DIRTY;
            *(char *)fp->_ptr = '\r';
            ++fp->_ptr;
            ++fp->_cnt;
            if (fp->_cnt == fp->_bufsize) {
                if (__flush(fp))
                    return EOF;
            }
        }
    }
    fp->_flag |= _DIRTY;
    *(char *)fp->_ptr = c;
    ++fp->_ptr;
    ++fp->_cnt;
    if ((fp->_flag & flags) || fp->_cnt == fp->_bufsize) {
        if (__flush(fp))
            return EOF;
    }
    return (unsigned char)c;
}
