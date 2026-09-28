#include "clib.h"

#define _IOFBF 0x0100
#define _IONBF 0x0400

typedef struct {
    FILE *_dest;
    char _pad[12];
    int _output_count;
} SPECS;

int fputc(int c, FILE *fp);
void __ioalloc(FILE *fp);

// FUNCTION: 0x000eb229
void file_putc(SPECS *specs, int op_char)
{
    fputc(op_char, specs->_dest);
    specs->_output_count++;
}

// FUNCTION: 0xeb23a
// NONMATCHING: file_putc lies inside smkmalloc_ in config/functions.txt
// (target names it smkmalloc_+0x29); the _IONBF/_IOFBF updates store the
// flag byte twice instead of once
int __fprtf(FILE *fp, const char *format, va_list arg)
{
    int not_buffered;
    int amount_written;
    unsigned oflag;

    oflag = fp->_flag & (_SFERR | _EOF);
    fp->_flag &= ~(_SFERR | _EOF);
    if (fp->_base == NULL)
        __ioalloc(fp);
    not_buffered = 0;
    if (fp->_flag & _IONBF) {
        not_buffered = 1;
        fp->_flag &= ~(_IONBF | _IOFBF);
        fp->_flag |= _IOFBF;
    }
    amount_written = __prtf(fp, format, arg, file_putc);
    if (not_buffered) {
        fp->_flag &= ~(_IONBF | _IOFBF);
        fp->_flag |= _IONBF;
        __flush(fp);
    }
    if (fp->_flag & _SFERR)
        amount_written = -1;
    fp->_flag |= oflag;
    return amount_written;
}
