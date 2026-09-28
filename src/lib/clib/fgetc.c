#include "clib.h"

#define _UNGET  0x0004
#define _BINARY 0x0040
#define _IOLBF  0x0200
#define _IONBF  0x0400
#define EBADF   4
#define STDIN_FILENO 0

void __ioalloc(FILE *fp);
int getche(void);
int __qread(int handle, void *buffer, unsigned len);
int __fill_buffer(FILE *fp);
int __filbuf(FILE *fp);

// FUNCTION: 0xfbfb3
int fgetc(FILE *fp)
{
    int c;

    if ((fp->_flag & _READ) == 0) {
        __set_errno(EBADF);
        fp->_flag |= _SFERR;
        c = EOF;
    } else {
        fp->_cnt--;
        if (fp->_cnt < 0) {
            c = __filbuf(fp);
        } else {
            c = *fp->_ptr & 0xff;
            fp->_ptr++;
        }
    }
    if (!(fp->_flag & _BINARY)) {
        if (c == '\r') {
            fp->_cnt--;
            if (fp->_cnt < 0) {
                c = __filbuf(fp);
            } else {
                c = *fp->_ptr & 0xff;
                fp->_ptr++;
            }
        }
        if (c == 0x1a) {
            fp->_flag |= _EOF;
            c = EOF;
        }
    }
    return c;
}

// FUNCTION: 0xfc03c
int __filbuf(FILE *fp)
{
    if (__fill_buffer(fp) == 0)
        return EOF;
    fp->_cnt--;
    fp->_ptr++;
    return *(fp->_ptr - 1);
}

// FUNCTION: 0xfc06b
int __fill_buffer(FILE *fp)
{
    int c;

    if (fp->_base == NULL)
        __ioalloc(fp);
    if (fp->_flag & _ISTTY) {
        if (fp->_flag & (_IONBF | _IOLBF))
            __flushall(_ISTTY);
    }
    fp->_flag &= ~_UNGET;
    fp->_ptr = fp->_base;
    if ((fp->_flag & (_IONBF | _ISTTY)) == (_IONBF | _ISTTY) && fp->_handle == STDIN_FILENO) {
        fp->_cnt = 0;
        c = getche();
        if (c != EOF) {
            *fp->_ptr = c;
            fp->_cnt = 1;
        }
    } else {
        fp->_cnt = __qread(fp->_handle, fp->_ptr, (fp->_flag & _IONBF) ? 1 : fp->_bufsize);
    }
    if (fp->_cnt <= 0) {
        if (fp->_cnt == 0) {
            fp->_flag |= _EOF;
        } else {
            fp->_flag |= _SFERR;
            fp->_cnt = 0;
        }
    }
    return fp->_cnt;
}
