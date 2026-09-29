#include "clib.h"

#define _IOLBF  0x0200
#define _IONBF  0x0400

void __ioalloc(FILE *fp);
int fputc(int c, FILE *fp);

// NONMATCHING: ours stores the flags byte between the and and the or (10.6
// keeps the dead store)
// FUNCTION: 0xfdfb4
int fputs(const char *s, FILE *fp)
{
    const char *start;
    int c;
    int not_buffered;
    int rc;

    if (fp->_base == NULL)
        __ioalloc(fp);
    not_buffered = 0;
    if (fp->_flag & _IONBF) {
        not_buffered = 1;
        fp->_flag &= ~_IONBF & ~_IOLBF;
        fp->_flag |= _IOLBF;
    }
    rc = 0;
    start = s;
    while (c = *s) {
        s++;
        if (fputc(c, fp) == EOF) {
            rc = EOF;
            break;
        }
    }
    if (not_buffered) {
        fp->_flag &= ~(_IONBF | _IOLBF);
        fp->_flag |= _IONBF;
        if (rc == 0)
            rc = __flush(fp);
    }
    if (rc == 0)
        rc = s - start;
    return rc;
}
