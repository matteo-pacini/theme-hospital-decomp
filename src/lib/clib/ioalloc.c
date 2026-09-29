#include "clib.h"

#define _BIGBUF 0x0008
#define _IOFBF  0x0100
#define _IOLBF  0x0200
#define _IONBF  0x0400
#define BUFSIZ  4096

void *malloc(size_t size);
#pragma aux malloc "W?init$n()v"

void __chktty(FILE *fp);

// FUNCTION: 0xf320e
void __ioalloc(FILE *fp)
{
    __chktty(fp);
    if (fp->_bufsize == 0) {
        if (fp->_flag & _IOLBF) {
            fp->_bufsize = 134;
        } else if (fp->_flag & _IONBF) {
            fp->_bufsize = 1;
        } else {
            fp->_bufsize = BUFSIZ;
        }
    }
    fp->_base = malloc(fp->_bufsize);
    if (fp->_base == NULL) {
        fp->_flag &= ~(_IONBF | _IOLBF | _IOFBF);
        fp->_flag |= _IONBF;
        fp->_bufsize = 1;
        fp->_base = &fp->_ungotten;
    } else {
        fp->_flag |= _BIGBUF;
    }
    fp->_ptr = fp->_base;
    fp->_cnt = 0;
}
