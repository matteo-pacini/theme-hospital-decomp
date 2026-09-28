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

// FUNCTION: 0xbc13a
// NONMATCHING: mem_putc lies inside LbMemorySet_ in config/functions.txt, so
// the target names its address LbMemorySet_+0x23
int sprintf(char *dest, const char *format, ...)
{
    va_list args;
    int len;

    args[0] = (char *)&format + sizeof(format);
    len = __prtf(dest, format, args, mem_putc);
    dest[len] = '\0';
    return len;
}
