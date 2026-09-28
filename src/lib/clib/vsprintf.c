#include "clib.h"

typedef struct {
    char *_dest;
    int _pad[3];
    int _output_count;
} SPECS;

void mem_putc(SPECS *specs, int op_char)
{
    *(specs->_dest++) = op_char;
    specs->_output_count++;
}

// FUNCTION: 0xce364
// NONMATCHING: mem_putc lies inside getenv_ in config/functions.txt, so the
// target names its address getenv_+0x61
int vsprintf(char *dest, const char *format, va_list arg)
{
    int len;

    len = __prtf(dest, format, arg, mem_putc);
    dest[len] = '\0';
    return len;
}
