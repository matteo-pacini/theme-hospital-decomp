#include "clib.h"

typedef struct {
    char *_dest;
    int _pad[3];
    int _output_count;
} SPECS;

static void mem_putc(SPECS *specs, char op_char);

// FUNCTION: 0xbc127
static void mem_putc(SPECS *specs, char op_char)
{
    *(specs->_dest++) = op_char;
    specs->_output_count++;
}

// FUNCTION: 0xbc13a
int sprintf(char *dest, const char *format, ...)
{
    va_list args;
    int len;

    args[0] = (char *)&format + sizeof(format);
    len = __prtf(dest, format, args, mem_putc);
    dest[len] = '\0';
    return len;
}
