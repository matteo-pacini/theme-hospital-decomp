#include "clib.h"

typedef struct {
    char *_dest;
    int _pad[3];
    int _output_count;
} SPECS;

static void mem_putc(SPECS *specs, char op_char);

// mem_putc: __prtf output callback; append op_char to the destination string and count it.
// FUNCTION: 0xbc127
static void mem_putc(SPECS *specs, char op_char)
{
    *(specs->_dest++) = op_char;
    specs->_output_count++;
}

// sprintf: formatted output to string dest (NUL-terminated); returns the length.
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
