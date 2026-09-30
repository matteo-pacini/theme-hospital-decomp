#include "clib.h"

typedef struct {
    char *_dest;
    int _pad[3];
    int _output_count;
} SPECS;

static void mem_putc(SPECS *specs, char op_char);

// FUNCTION: 0xce351
static void mem_putc(SPECS *specs, char op_char)
{
    *(specs->_dest++) = op_char;
    specs->_output_count++;
}

// FUNCTION: 0xce364
int vsprintf(char *dest, const char *format, va_list arg)
{
    int len;

    len = __prtf(dest, format, arg, mem_putc);
    dest[len] = '\0';
    return len;
}
